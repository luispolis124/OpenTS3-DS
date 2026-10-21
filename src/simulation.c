#include "simulation.h"

#include <string.h>

/* ===========================================================================
 * Parâmetros de projeto (placeholders ajustáveis; não são do jogo original)
 * ======================================================================== */

#define NEED_THRESHOLD            350    /* abaixo disso a necessidade vira prioridade */
#define NEED_THRESHOLD_STORM      650    /* pets sob tempestade procuram o dono antes */
#define NEED_FULL                 950    /* uso termina ao atingir */
#define NAV_BUDGET_PER_TICK       96u    /* expansões de A* por tick */
#define NAV_WAIT_MAX_TICKS        150u   /* 5 s esperando o pathfinding */
#define STUCK_MAX_TICKS           90u    /* 3 s andando sem sair do lugar */
#define FAIL_BACKOFF_TICKS        30u
#define FAILSAFE_FAILS            3u
#define FAILSAFE_GAIN             200
#define WANDER_RADIUS             6
#define WANDER_TRIES              8u
#define BOWL_LOW                  25u
#define BOWL_FULL                 100u
#define CHORE_MIN_NEED            450    /* Sim só faz tarefa doméstica se estiver bem */
#define SEEK_RETRY_MAX            3u
#define AFFECTION_MINUTES         5u
#define AFFECTION_GAIN_PER_MIN    90
#define CARETAKER_FUN_PER_MIN     20
#define RESERVE_SLACK_MINUTES     10u
#define SPEED_DIAG_Q8             181u   /* 0,707 em ponto fixo 8 bits */

typedef struct ObjectDef {
    u8  need;                          /* NeedId que satisfaz */
    u8  users;                         /* bit0 = Sim, bit1 = pet */
    u16 max_minutes;
    s16 gain_per_minute;
    u8  consumes;                      /* unidades de 'state' gastas por minuto */
} ObjectDef;

#define USER_SIM 0x01u
#define USER_PET 0x02u

static const ObjectDef kObj[OBJ_KIND_COUNT] = {
    /* NONE     */ { 0,             0,        0,   0, 0 },
    /* FRIDGE   */ { NEED_HUNGER,   USER_SIM, 20,  50, 0 },
    /* BED      */ { NEED_ENERGY,   USER_SIM, 360,  4, 0 },
    /* TOILET   */ { NEED_BLADDER,  USER_SIM, 5,  200, 0 },
    /* SHOWER   */ { NEED_HYGIENE,  USER_SIM, 10, 100, 0 },
    /* TV       */ { NEED_FUN,      USER_SIM, 60,  25, 0 },
    /* PET_BOWL */ { NEED_HUNGER,   USER_PET, 10,  80, 8 },
    /* PET_BED  */ { NEED_ENERGY,   USER_PET, 240,  8, 0 }
};

/* velocidade em tiles (20.12) por tick: Sim, cão, gato */
static const s32 kSpeedSim = 0x0140;
static const s32 kSpeedDog = 0x01C0;
static const s32 kSpeedCat = 0x0180;

/* ===========================================================================
 * Utilidades
 * ======================================================================== */

static u32 RngNext(WorldState* w)
{
    u32 x = w->p.rng;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    w->p.rng = x;
    return x;
}

static u32 Rnd(WorldState* w, u32 n)
{
    return n ? (u32)(((u64)RngNext(w) * n) >> 32) : 0u;
}

static BOOL Live(const WorldState* w, u32 slot)
{
    return w->p.entities.e[slot].id != ENTITY_ID_NONE;
}

static u32 SlotOf(EntityId id)
{
    return ENTITY_ID_INDEX(id) - 1u;
}

static u8 ProfileOf(const Entity* e)
{
    return e->kind == ENTITY_KIND_PET ? NAV_PROFILE_PET : NAV_PROFILE_SIM;
}

static s32 SpeedOf(const Entity* e)
{
    if (e->kind == ENTITY_KIND_PET) {
        return e->species == PET_SPECIES_DOG ? kSpeedDog : kSpeedCat;
    }
    return kSpeedSim;
}

static s32 TileX(const Entity* e) { return FX_TILE(e->pos.x); }
static s32 TileY(const Entity* e) { return FX_TILE(e->pos.y); }

static s32 Clamp32(s32 v, s32 lo, s32 hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static s32 Abs32(s32 v) { return v < 0 ? -v : v; }

static BOOL InGrid(const NavGrid* g, s32 x, s32 y)
{
    return x >= 0 && y >= 0 && x < (s32)g->w && y < (s32)g->h;
}

/* ===========================================================================
 * Objetos e reservas
 * ======================================================================== */

static BOOL ObjFree(const WorldState* w, u32 oi, EntityId who)
{
    const ObjReservation* r = &w->rt.res[oi];

    if (r->owner == ENTITY_ID_NONE || r->owner == who) {
        return TRUE;
    }
    if (EntityTable_GetConst(&w->p.entities, r->owner) == NULL) {
        return TRUE;                                   /* dono sumiu: reserva órfã */
    }
    return (s32)(w->p.clock.tick - r->until_tick) >= 0;   /* expirou */
}

static void ObjReserve(WorldState* w, u32 oi, EntityId who)
{
    u32 minutes = (u32)kObj[w->p.objects[oi].kind].max_minutes + RESERVE_SLACK_MINUTES;

    w->rt.res[oi].owner = who;
    w->rt.res[oi].until_tick = w->p.clock.tick + minutes * OTS3_TICKS_PER_GAME_MINUTE + 300u;
}

static void ReleaseObj(WorldState* w, u32 slot)
{
    EntityRuntime* r = &w->rt.ent[slot];
    const Entity* e = &w->p.entities.e[slot];

    if (r->obj < w->p.object_count && w->rt.res[r->obj].owner == e->id) {
        w->rt.res[r->obj].owner = ENTITY_ID_NONE;
    }
    r->obj = OBJ_NONE_INDEX;
}

/* objeto livre, adequado à necessidade e ao tipo de usuário, o mais próximo */
static u32 FindObject(const WorldState* w, const Entity* e, u8 need)
{
    u32 oi, best = OBJ_NONE_INDEX, best_d = 0xFFFFFFFFu;
    u8 user_bit = (e->kind == ENTITY_KIND_PET) ? USER_PET : USER_SIM;

    for (oi = 0; oi < w->p.object_count; oi++) {
        const WorldObject* o = &w->p.objects[oi];
        u32 d;

        if (o->kind == OBJ_NONE || kObj[o->kind].need != need || !(kObj[o->kind].users & user_bit)) {
            continue;
        }
        if (kObj[o->kind].consumes && o->state == 0) {
            continue;                                  /* tigela vazia */
        }
        if (!ObjFree(w, oi, e->id)) {
            continue;
        }
        d = (u32)Abs32((s32)o->tx - TileX(e)) + (u32)Abs32((s32)o->ty - TileY(e));
        if (d < best_d) {
            best_d = d;
            best = oi;
        }
    }
    return best;
}

/* tigela baixa que alguém da família precisa ver reabastecida */
static u32 FindLowBowl(const WorldState* w, const Entity* sim)
{
    u32 oi, i, best = OBJ_NONE_INDEX, best_d = 0xFFFFFFFFu;
    BOOL has_pet = FALSE;

    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        const Entity* p = &w->p.entities.e[i];
        if (p->id != ENTITY_ID_NONE && p->kind == ENTITY_KIND_PET && p->household == sim->household) {
            has_pet = TRUE;
            break;
        }
    }
    if (!has_pet) {
        return OBJ_NONE_INDEX;
    }
    for (oi = 0; oi < w->p.object_count; oi++) {
        const WorldObject* o = &w->p.objects[oi];
        u32 d;
        if (o->kind != OBJ_PET_BOWL || o->state > BOWL_LOW || !ObjFree(w, oi, sim->id)) {
            continue;
        }
        d = (u32)Abs32((s32)o->tx - TileX(sim)) + (u32)Abs32((s32)o->ty - TileY(sim));
        if (d < best_d) {
            best_d = d;
            best = oi;
        }
    }
    return best;
}

/* ===========================================================================
 * Ações
 * ======================================================================== */

static void StartTravel(WorldState* w, u32 slot, u8 action, u32 gx, u32 gy)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];
    s32 sx = Clamp32(TileX(e), 0, (s32)w->p.grid.w - 1);
    s32 sy = Clamp32(TileY(e), 0, (s32)w->p.grid.h - 1);

    r->action = action;
    r->goal = NAV_TILE(gx, gy);
    r->move_state = MOVE_WAIT_PATH;
    r->nav_wait = 0;
    r->path_pos = 0;
    r->path.len = 0;
    r->path.partial = 0;
    r->stuck_ticks = 0;
    r->submit_pending = Nav_Submit(&w->rt.nav, &w->p.grid, e->id, ProfileOf(e),
                                   (u32)sx, (u32)sy, gx, gy) ? 0u : 1u;
}

static void Teleport(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    const NavGrid* g = &w->p.grid;
    u8 profile = ProfileOf(e);
    u32 i, n = (u32)g->w * g->h;

    if (InGrid(g, TileX(e), TileY(e)) && Nav_Passable(g, profile, TileX(e), TileY(e))) {
        return;
    }
    for (i = 0; i < 16u; i++) {                        /* tenta tiles aleatórios... */
        s32 x = (s32)Rnd(w, g->w), y = (s32)Rnd(w, g->h);
        if (Nav_Passable(g, profile, x, y)) {
            e->pos.x = FX_TILE_CENTER(x);
            e->pos.y = FX_TILE_CENTER(y);
            return;
        }
    }
    for (i = 0; i < n; i++) {                          /* ...e varre a grade como último recurso */
        s32 x = (s32)(i % g->w), y = (s32)(i / g->w);
        if (Nav_Passable(g, profile, x, y)) {
            e->pos.x = FX_TILE_CENTER(x);
            e->pos.y = FX_TILE_CENTER(y);
            return;
        }
    }
}

static void Failsafe(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];

    w->rt.stats.failsafes++;
    if (Entity_NeedApplies(e, r->need)) {
        Entity_AddNeed(e, r->need, FAILSAFE_GAIN);     /* nunca morre de fome por um bug de rota */
    }
    r->fail_count = 0;
    Teleport(w, slot);
}

static void AbortAction(WorldState* w, u32 slot)
{
    EntityRuntime* r = &w->rt.ent[slot];

    ReleaseObj(w, slot);
    Nav_Cancel(&w->rt.nav, w->p.entities.e[slot].id);
    r->move_state = MOVE_IDLE;
    r->action = ACT_NONE;
    r->submit_pending = 0;
}

static void Fail(WorldState* w, u32 slot)
{
    EntityRuntime* r = &w->rt.ent[slot];

    AbortAction(w, slot);
    r->backoff = (u16)FAIL_BACKOFF_TICKS;
    r->fail_count++;
    if (r->fail_count >= FAILSAFE_FAILS) {
        Failsafe(w, slot);
    }
}

static void EndUse(WorldState* w, u32 slot)
{
    EntityRuntime* r = &w->rt.ent[slot];

    AbortAction(w, slot);
    r->backoff = 15;
    r->fail_count = 0;
    r->use_left = 0;
}

/* dono se existir; senão o primeiro Sim da família que possa andar */
static const Entity* FindCaretaker(const WorldState* w, const Entity* pet)
{
    const Entity* o = EntityTable_GetConst(&w->p.entities, pet->owner);
    u32 i;

    if (o) {
        return o;
    }
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        const Entity* c = &w->p.entities.e[i];
        if (c->id != ENTITY_ID_NONE && c->kind == ENTITY_KIND_SIM && c->household == pet->household &&
            (Life_StageCaps(c->age_stage) & LIFE_CAP_WALK)) {
            return c;
        }
    }
    return NULL;
}

static BOOL Adjacent(const Entity* a, const Entity* b)
{
    return Abs32(TileX(a) - TileX(b)) <= 1 && Abs32(TileY(a) - TileY(b)) <= 1;
}

static BOOL StartSeekCaretaker(WorldState* w, u32 slot)
{
    const Entity* e = &w->p.entities.e[slot];
    const Entity* c = FindCaretaker(w, e);
    EntityRuntime* r = &w->rt.ent[slot];

    if (!c || !InGrid(&w->p.grid, TileX(c), TileY(c))) {
        return FALSE;
    }
    r->need = NEED_AFFECTION;
    r->retry = 0;
    StartTravel(w, slot, ACT_SEEK_OWNER, (u32)TileX(c), (u32)TileY(c));
    return TRUE;
}

static void Wander(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];
    u32 i;

    for (i = 0; i < WANDER_TRIES; i++) {
        s32 x = TileX(e) + (s32)Rnd(w, 2u * WANDER_RADIUS + 1u) - WANDER_RADIUS;
        s32 y = TileY(e) + (s32)Rnd(w, 2u * WANDER_RADIUS + 1u) - WANDER_RADIUS;
        if ((x != TileX(e) || y != TileY(e)) && Nav_Passable(&w->p.grid, ProfileOf(e), x, y)) {
            r->need = NEED_COUNT;                      /* passeio não é uma necessidade */
            StartTravel(w, slot, ACT_WANDER, (u32)x, (u32)y);
            return;
        }
    }
    r->backoff = 60;
}

static void Decide(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];
    u32 caps = Life_StageCaps(e->age_stage);
    u8 need = NEED_COUNT;
    BOOL urgent = Entity_MostUrgentNeed(e, NEED_THRESHOLD, &need);

    if (!urgent && e->kind == ENTITY_KIND_PET && Weather_IsStormy(&w->p.weather) &&
        e->need[NEED_AFFECTION] < NEED_THRESHOLD_STORM) {
        need = NEED_AFFECTION;                         /* medo de tempestade */
        urgent = TRUE;
    }
    if (urgent) {
        if (need == NEED_AFFECTION) {
            if (StartSeekCaretaker(w, slot)) {
                return;
            }
        } else if (caps & LIFE_CAP_USE_OBJECTS) {
            u32 oi = FindObject(w, e, need);
            if (oi != OBJ_NONE_INDEX) {
                ObjReserve(w, oi, e->id);
                r->need = need;
                r->obj = (u8)oi;
                StartTravel(w, slot, ACT_GOTO_OBJECT, w->p.objects[oi].tx, w->p.objects[oi].ty);
                return;
            }
        }
    }
    if (e->kind == ENTITY_KIND_SIM && (caps & LIFE_CAP_USE_OBJECTS)) {
        BOOL healthy = TRUE;
        u32 k;
        for (k = 0; k < NEED_COUNT; k++) {
            if (Entity_NeedApplies(e, (u8)k) && e->need[k] < CHORE_MIN_NEED) {
                healthy = FALSE;
            }
        }
        if (healthy) {
            u32 oi = FindLowBowl(w, e);
            if (oi != OBJ_NONE_INDEX) {
                ObjReserve(w, oi, e->id);
                r->need = NEED_COUNT;
                r->obj = (u8)oi;
                StartTravel(w, slot, ACT_GOTO_BOWL, w->p.objects[oi].tx, w->p.objects[oi].ty);
                return;
            }
        }
    }
    Wander(w, slot);
}

static void OnArrive(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];
    u32 oi = r->obj;

    switch (r->action) {
    case ACT_WANDER:
        r->action = ACT_NONE;
        r->move_state = MOVE_IDLE;
        r->backoff = (u16)(30u + Rnd(w, 60));
        r->fail_count = 0;
        break;
    case ACT_GOTO_OBJECT:
        if (oi < w->p.object_count && w->rt.res[oi].owner == e->id &&
            TileX(e) == (s32)w->p.objects[oi].tx && TileY(e) == (s32)w->p.objects[oi].ty &&
            w->p.objects[oi].kind != OBJ_NONE) {
            r->action = ACT_USE_OBJECT;
            r->move_state = MOVE_IDLE;
            r->use_left = kObj[w->p.objects[oi].kind].max_minutes;
        } else {
            Fail(w, slot);                             /* reserva perdida ou objeto removido */
        }
        break;
    case ACT_GOTO_BOWL:
        if (oi < w->p.object_count && w->p.objects[oi].kind == OBJ_PET_BOWL &&
            TileX(e) == (s32)w->p.objects[oi].tx && TileY(e) == (s32)w->p.objects[oi].ty) {
            w->p.objects[oi].state = (u8)BOWL_FULL;
            EndUse(w, slot);
        } else {
            Fail(w, slot);
        }
        break;
    case ACT_SEEK_OWNER: {
        const Entity* c = FindCaretaker(w, e);
        if (c && Adjacent(e, c)) {
            r->action = ACT_RECEIVE_AFFECTION;
            r->move_state = MOVE_IDLE;
            r->use_left = (u16)AFFECTION_MINUTES;
        } else if (c && r->retry < SEEK_RETRY_MAX && InGrid(&w->p.grid, TileX(c), TileY(c))) {
            r->retry++;
            StartTravel(w, slot, ACT_SEEK_OWNER, (u32)TileX(c), (u32)TileY(c));
        } else {
            Fail(w, slot);
        }
        break;
    }
    default:
        Fail(w, slot);
        break;
    }
}

static void TravelThink(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];

    switch (r->move_state) {
    case MOVE_WAIT_PATH: {
        NavResult res = NAV_RESULT_NONE;

        if (r->submit_pending) {                       /* fila do A* estava cheia */
            s32 sx = Clamp32(TileX(e), 0, (s32)w->p.grid.w - 1);
            s32 sy = Clamp32(TileY(e), 0, (s32)w->p.grid.h - 1);
            if (Nav_Submit(&w->rt.nav, &w->p.grid, e->id, ProfileOf(e), (u32)sx, (u32)sy,
                           NAV_TILE_X(r->goal), NAV_TILE_Y(r->goal))) {
                r->submit_pending = 0;
            }
        } else {
            res = Nav_Poll(&w->rt.nav, e->id, &r->path);
        }
        if (res == NAV_RESULT_FOUND) {
            r->path_pos = 0;
            r->nav_wait = 0;
            r->stuck_ticks = 0;
            r->move_state = (r->path.len == 0) ? MOVE_IDLE : MOVE_WALKING;
            return;
        }
        if (res == NAV_RESULT_FAILED) {
            w->rt.stats.nav_failures++;
            Fail(w, slot);
            return;
        }
        if (res == NAV_RESULT_NONE && !r->submit_pending) {
            r->submit_pending = 1;                     /* pedido perdido: refaz */
        }
        if (++r->nav_wait > NAV_WAIT_MAX_TICKS) {
            w->rt.stats.nav_timeouts++;
            Fail(w, slot);
        }
        break;
    }
    case MOVE_BLOCKED:
        Fail(w, slot);
        break;
    case MOVE_WALKING:
        if (r->action == ACT_SEEK_OWNER) {             /* chegou perto do dono: não precisa ir até o fim */
            const Entity* c = FindCaretaker(w, e);
            if (c && Adjacent(e, c)) {
                r->move_state = MOVE_IDLE;
                OnArrive(w, slot);
            }
        }
        break;
    case MOVE_IDLE:
    default:
        OnArrive(w, slot);
        break;
    }
}

static void Think(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];

    if (!(Life_StageCaps(e->age_stage) & LIFE_CAP_WALK)) {
        if (r->action != ACT_NONE) {
            AbortAction(w, slot);                      /* bebê não faz nada sozinho */
        }
        return;
    }
    switch (r->action) {
    case ACT_NONE:
        if (r->backoff > 0) {
            r->backoff--;
        } else {
            Decide(w, slot);
        }
        break;
    case ACT_WANDER:
    case ACT_GOTO_OBJECT:
    case ACT_SEEK_OWNER:
    case ACT_GOTO_BOWL:
        TravelThink(w, slot);
        break;
    case ACT_USE_OBJECT:
        if (r->obj >= w->p.object_count || w->p.objects[r->obj].kind == OBJ_NONE ||
            w->rt.res[r->obj].owner != e->id) {
            EndUse(w, slot);                           /* objeto removido ou reserva roubada */
        }
        break;
    case ACT_RECEIVE_AFFECTION: {
        const Entity* c = FindCaretaker(w, e);
        if (!c || !Adjacent(e, c)) {
            EndUse(w, slot);
        }
        break;
    }
    default:
        AbortAction(w, slot);
        break;
    }
}

/* ===========================================================================
 * Movimento
 * ======================================================================== */

static const u16 kYaw[9] = {         /* indexado por (sinal_y + 1) * 3 + (sinal_x + 1) */
    0xE000, 0x0000, 0x2000,
    0xC000, 0xFFFF, 0x4000,
    0xA000, 0x8000, 0x6000
};

static s32 Sign32(s32 v) { return v > 0 ? 1 : (v < 0 ? -1 : 0); }

static void PathEnd(WorldState* w, u32 slot)
{
    EntityRuntime* r = &w->rt.ent[slot];
    Entity* e = &w->p.entities.e[slot];

    if (r->path.partial) {                             /* caminho truncado: pede o resto */
        s32 sx = Clamp32(TileX(e), 0, (s32)w->p.grid.w - 1);
        s32 sy = Clamp32(TileY(e), 0, (s32)w->p.grid.h - 1);
        r->path.partial = 0;
        r->path.len = 0;
        r->path_pos = 0;
        r->move_state = MOVE_WAIT_PATH;
        r->nav_wait = 0;
        r->submit_pending = Nav_Submit(&w->rt.nav, &w->p.grid, e->id, ProfileOf(e), (u32)sx, (u32)sy,
                                       NAV_TILE_X(r->goal), NAV_TILE_Y(r->goal)) ? 0u : 1u;
    } else {
        r->move_state = MOVE_IDLE;
    }
}

static void MoveStep(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];
    u16 tile;
    s32 tx, ty, cx, cy, dx, dy, sp, mx, my;
    Vec2Fx before;

    if (r->move_state != MOVE_WALKING) {
        return;
    }
    if (r->path_pos >= r->path.len) {
        PathEnd(w, slot);
        return;
    }
    tile = r->path.tiles[r->path_pos];
    tx = (s32)NAV_TILE_X(tile);
    ty = (s32)NAV_TILE_Y(tile);
    if (!Nav_Passable(&w->p.grid, ProfileOf(e), tx, ty)) {
        r->move_state = MOVE_BLOCKED;                  /* a lota mudou no meio do caminho */
        return;
    }
    cx = FX_TILE_CENTER(tx);
    cy = FX_TILE_CENTER(ty);
    dx = cx - e->pos.x;
    dy = cy - e->pos.y;
    sp = SpeedOf(e);
    if (dx != 0 && dy != 0) {
        sp = (sp * (s32)SPEED_DIAG_Q8) >> 8;           /* diagonal não é mais rápida */
    }
    before = e->pos;
    mx = Clamp32(dx, -sp, sp);
    my = Clamp32(dy, -sp, sp);
    e->pos.x += mx;
    e->pos.y += my;
    if (mx != 0 || my != 0) {
        u16 yaw = kYaw[(Sign32(my) + 1) * 3 + (Sign32(mx) + 1)];
        if (yaw != 0xFFFFu) {
            e->yaw = yaw;
        }
    }
    if (e->pos.x == before.x && e->pos.y == before.y) {
        if (++r->stuck_ticks > STUCK_MAX_TICKS) {
            w->rt.stats.stuck_events++;
            r->move_state = MOVE_BLOCKED;
        }
        return;
    }
    r->stuck_ticks = 0;
    if (e->pos.x == cx && e->pos.y == cy) {
        r->path_pos++;
        if (r->path_pos >= r->path.len) {
            PathEnd(w, slot);
        }
    }
}

/* ===========================================================================
 * Minuto e dia de jogo
 * ======================================================================== */

static void UseMinute(WorldState* w, u32 slot)
{
    Entity* e = &w->p.entities.e[slot];
    EntityRuntime* r = &w->rt.ent[slot];

    if (r->action == ACT_USE_OBJECT && r->obj < w->p.object_count) {
        WorldObject* o = &w->p.objects[r->obj];
        const ObjectDef* d = &kObj[o->kind];
        BOOL done;

        Entity_AddNeed(e, d->need, d->gain_per_minute);
        if (d->consumes) {
            o->state = (u8)(o->state > d->consumes ? o->state - d->consumes : 0u);
        }
        if (r->use_left > 0) {
            r->use_left--;
        }
        done = r->use_left == 0 || e->need[d->need] >= NEED_FULL || (d->consumes && o->state == 0);
        if (done) {
            EndUse(w, slot);
        }
    } else if (r->action == ACT_RECEIVE_AFFECTION) {
        const Entity* c = FindCaretaker(w, e);
        Entity_AddNeed(e, NEED_AFFECTION, AFFECTION_GAIN_PER_MIN);
        if (c && c->kind == ENTITY_KIND_SIM) {
            Entity* cm = EntityTable_Get(&w->p.entities, c->id);
            if (cm) {
                Entity_AddNeed(cm, NEED_FUN, CARETAKER_FUN_PER_MIN);   /* carinho também faz bem ao Sim */
            }
        }
        if (r->use_left > 0) {
            r->use_left--;
        }
        if (r->use_left == 0 || e->need[NEED_AFFECTION] >= NEED_FULL) {
            EndUse(w, slot);
        }
    }
}

static EntityId HookSpawn(void* user, u8 household, const char* name, Vec2Fx pos)
{
    return Simulation_SpawnSim((WorldState*)user, household, name, pos);
}

static void HookDespawn(void* user, EntityId id)
{
    (void)Simulation_Despawn((WorldState*)user, id);
}

static void MakeLifeContext(WorldState* w, LifeContext* ctx)
{
    ctx->ents = &w->p.entities;
    ctx->events = &w->rt.events;
    ctx->hooks = &w->rt.hooks;
    ctx->spawn_pos.x = FX_TILE_CENTER(w->p.spawn_x);
    ctx->spawn_pos.y = FX_TILE_CENTER(w->p.spawn_y);
}

static void OnMinute(WorldState* w)
{
    SimClock* c = &w->p.clock;
    Calendar cal;
    u32 slot, k;

    c->minute++;
    if (c->minute >= OTS3_MINUTES_PER_DAY) {
        LifeContext ctx;
        c->minute = 0;
        if (c->day < LIFE_MAX_DAY) {
            c->day++;
        }
        MakeLifeContext(w, &ctx);
        (void)Life_Update(&w->p.life, &ctx, c->day);   /* envelhecimento, gestação, morte natural */
    }
    Calendar_FromDay(c->day, &cal);
    Weather_OnMinute(&w->p.weather, cal.season);

    for (slot = 0; slot < OTS3_MAX_ENTITIES; slot++) {
        Entity* e = &w->p.entities.e[slot];

        if (e->id == ENTITY_ID_NONE) {
            continue;
        }
        if (Life_StageCaps(e->age_stage) & LIFE_CAP_USE_OBJECTS) {   /* bebês são cuidados pela família */
            Entity_DecayNeedsMinute(e);
            for (k = 0; k < NEED_COUNT; k++) {
                if (Entity_NeedApplies(e, (u8)k) && Weather_ExtraNeedDecay(&w->p.weather, (u8)k, c->minute)) {
                    Entity_AddNeed(e, (u8)k, -1);
                }
            }
        }
        UseMinute(w, slot);
    }
}

OTS3_HOT void Simulation_Step(WorldState* w)
{
    SimClock* c = &w->p.clock;
    u32 slot;

    c->tick++;
    c->minute_ticks++;
    if (c->minute_ticks >= OTS3_TICKS_PER_GAME_MINUTE) {
        c->minute_ticks = 0;
        OnMinute(w);
    }
    for (slot = 0; slot < OTS3_MAX_ENTITIES; slot++) {
        if (Live(w, slot)) {
            Think(w, slot);
        }
    }
    Nav_Step(&w->rt.nav, &w->p.grid, NAV_BUDGET_PER_TICK);
    for (slot = 0; slot < OTS3_MAX_ENTITIES; slot++) {
        if (Live(w, slot)) {
            MoveStep(w, slot);
        }
    }
}

/* ===========================================================================
 * Tempo real -> ticks
 * ======================================================================== */

u32 Simulation_UpdateUs(WorldState* w, u32 delta_us)
{
    u32 speed = w->p.clock.speed, steps, i;

    if (delta_us > OTS3_MAX_DELTA_US) {
        w->rt.stats.dropped_us += delta_us - OTS3_MAX_DELTA_US;
        delta_us = OTS3_MAX_DELTA_US;
    }
    if (speed == 0 || speed > OTS3_MAX_SPEED) {
        return 0;                                      /* pausa: o tempo acumulado fica como está */
    }
    w->rt.acc_us += delta_us * speed;                  /* <= 250000 * 3: não estoura */
    steps = 0;
    while (w->rt.acc_us >= OTS3_STEP_US) {
        w->rt.acc_us -= OTS3_STEP_US;
        steps++;
    }
    if (steps > OTS3_MAX_STEPS_PER_UPDATE) {
        w->rt.stats.dropped_us += (steps - OTS3_MAX_STEPS_PER_UPDATE) * OTS3_STEP_US;
        steps = OTS3_MAX_STEPS_PER_UPDATE;
    }
    for (i = 0; i < steps; i++) {
        Simulation_Step(w);
    }
    return steps;
}

void Simulation_Update(WorldState* world, float deltaTime)
{
    u32 us;

    if (!(deltaTime > 0.0f)) {                         /* também descarta NaN */
        return;
    }
    if (deltaTime > 0.25f) {                           /* inclui +infinito */
        deltaTime = 0.25f;
    }
    us = (u32)(deltaTime * 1000000.0f + 0.5f);
    (void)Simulation_UpdateUs(world, us);
}

void Simulation_SetSpeed(WorldState* w, u8 speed)
{
    w->p.clock.speed = (u8)(speed > OTS3_MAX_SPEED ? OTS3_MAX_SPEED : speed);
}

/* ===========================================================================
 * Criação e remoção
 * ======================================================================== */

static void ResetRuntime(WorldState* w)
{
    u32 i;

    Nav_Init(&w->rt.nav);
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        EntityRuntime_Reset(&w->rt.ent[i]);
    }
    memset(w->rt.res, 0, sizeof(w->rt.res));
    LifeEvents_Init(&w->rt.events);
    memset(&w->rt.stats, 0, sizeof(w->rt.stats));
    w->rt.acc_us = 0;
    w->rt.hooks.user = w;
    w->rt.hooks.spawn_sim = HookSpawn;
    w->rt.hooks.despawn = HookDespawn;
}

void Simulation_Init(WorldState* w, u32 seed)
{
    memset(w, 0, sizeof(*w));
    EntityTable_Init(&w->p.entities);
    w->p.rng = seed ? seed : 0x9E3779B9u;
    w->p.clock.speed = 1;
    w->p.grid.w = 16;
    w->p.grid.h = 16;
    w->p.spawn_x = 1;
    w->p.spawn_y = 1;
    Lifecycle_Init(&w->p.life, w->p.rng ^ 0xA5A5A5A5u, 0);
    Weather_Init(&w->p.weather, w->p.rng ^ 0x5A5A5A5Au, SEASON_SPRING);
    ResetRuntime(w);
}

BOOL Simulation_AddHousehold(WorldState* w, const char* name, u8* out_index)
{
    u32 i, k = 0;

    for (i = 0; i < OTS3_MAX_HOUSEHOLDS; i++) {
        Household* h = &w->p.households[i];
        if (!h->active) {
            memset(h, 0, sizeof(*h));
            while (name && k < OTS3_HOUSEHOLD_NAME_LEN - 1u && name[k] != '\0') {
                h->name[k] = name[k];
                k++;
            }
            h->funds = 20000;
            h->active = 1;
            if (out_index) {
                *out_index = (u8)i;
            }
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL HouseholdActive(const WorldState* w, u8 h)
{
    return h < OTS3_MAX_HOUSEHOLDS && w->p.households[h].active;
}

EntityId Simulation_SpawnSim(WorldState* w, u8 household, const char* name, Vec2Fx pos)
{
    EntityId id;
    Entity* e;

    if (!HouseholdActive(w, household) || !InGrid(&w->p.grid, FX_TILE(pos.x), FX_TILE(pos.y)) ||
        pos.x < 0 || pos.y < 0) {
        return ENTITY_ID_NONE;
    }
    id = EntityTable_Spawn(&w->p.entities, ENTITY_KIND_SIM, PET_SPECIES_NONE, household,
                           ENTITY_ID_NONE, name, pos);
    if (id == ENTITY_ID_NONE) {
        return ENTITY_ID_NONE;
    }
    e = EntityTable_Get(&w->p.entities, id);
    e->age_stage = LIFE_STAGE_ADULT;                   /* o ciclo de vida corrige se houver Person */
    EntityRuntime_Reset(&w->rt.ent[SlotOf(id)]);
    return id;
}

EntityId Simulation_SpawnPet(WorldState* w, u8 species, u8 household, EntityId owner,
                             const char* name, Vec2Fx pos)
{
    EntityId id;
    Entity* e;

    if (species != PET_SPECIES_DOG && species != PET_SPECIES_CAT) {
        return ENTITY_ID_NONE;
    }
    if (!HouseholdActive(w, household) || pos.x < 0 || pos.y < 0 ||
        !InGrid(&w->p.grid, FX_TILE(pos.x), FX_TILE(pos.y))) {
        return ENTITY_ID_NONE;
    }
    if (owner != ENTITY_ID_NONE) {
        const Entity* o = EntityTable_GetConst(&w->p.entities, owner);
        if (!o || o->kind != ENTITY_KIND_SIM || o->household != household) {
            return ENTITY_ID_NONE;
        }
    }
    id = EntityTable_Spawn(&w->p.entities, ENTITY_KIND_PET, species, household, owner, name, pos);
    if (id == ENTITY_ID_NONE) {
        return ENTITY_ID_NONE;
    }
    e = EntityTable_Get(&w->p.entities, id);
    e->age_stage = LIFE_STAGE_ADULT;                   /* pets não envelhecem (limitação documentada) */
    EntityRuntime_Reset(&w->rt.ent[SlotOf(id)]);
    return id;
}

BOOL Simulation_Despawn(WorldState* w, EntityId id)
{
    Entity* e = EntityTable_Get(&w->p.entities, id);
    u32 slot, i;

    if (!e) {
        return FALSE;
    }
    slot = SlotOf(id);
    ReleaseObj(w, slot);
    Nav_Cancel(&w->rt.nav, id);
    for (i = 0; i < OTS3_MAX_OBJECTS; i++) {           /* nenhuma reserva sobrevive ao dono */
        if (w->rt.res[i].owner == id) {
            w->rt.res[i].owner = ENTITY_ID_NONE;
        }
    }
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        Entity* p = &w->p.entities.e[i];
        if (p->id != ENTITY_ID_NONE && p->owner == id) {
            p->owner = ENTITY_ID_NONE;                 /* nunca deixa um id velho como dono */
        }
    }
    EntityRuntime_Reset(&w->rt.ent[slot]);
    return EntityTable_Despawn(&w->p.entities, id);
}

LifeResult Simulation_AddPerson(WorldState* w, const LifePersonInit* init, PersonId* out)
{
    LifeResult r;

    if (init && init->household != LIFE_NO_HOUSEHOLD && !HouseholdActive(w, init->household)) {
        return LIFE_ERR_ARG;
    }
    r = Life_AddPerson(&w->p.life, init, w->p.clock.day, out);
    if (r == LIFE_OK && init->wants_entity) {
        LifeContext ctx;
        MakeLifeContext(w, &ctx);
        (void)Life_MaterializeBodies(&w->p.life, &ctx, w->p.clock.day);
    }
    return r;
}

BOOL Simulation_AddObject(WorldState* w, u8 kind, u32 tx, u32 ty, u8* out_index)
{
    WorldObject* o;

    if (kind == OBJ_NONE || kind >= OBJ_KIND_COUNT || w->p.object_count >= OTS3_MAX_OBJECTS ||
        !InGrid(&w->p.grid, (s32)tx, (s32)ty) || (w->p.grid.flags[ty * w->p.grid.w + tx] & NAV_SOLID)) {
        return FALSE;
    }
    o = &w->p.objects[w->p.object_count];
    o->kind = kind;
    o->state = (kind == OBJ_PET_BOWL) ? (u8)BOWL_FULL : 0u;
    o->tx = (u8)tx;
    o->ty = (u8)ty;
    if (out_index) {
        *out_index = (u8)w->p.object_count;
    }
    w->p.object_count++;
    return TRUE;
}

void Simulation_BuildDemoLot(WorldState* w)
{
    u32 x, y;
    NavGrid* g = &w->p.grid;

    memset(g, 0, sizeof(*g));
    g->w = 24;
    g->h = 16;
    for (x = 0; x < 24; x++) {
        g->flags[0 * 24 + x] = NAV_SOLID;
        g->flags[15 * 24 + x] = NAV_SOLID;
    }
    for (y = 0; y < 16; y++) {
        g->flags[y * 24 + 0] = NAV_SOLID;
        g->flags[y * 24 + 23] = NAV_SOLID;
        g->flags[y * 24 + 12] = (y == 8) ? 0u : NAV_SOLID;           /* porta da sala */
    }
    g->flags[3 * 24 + 12] = NAV_NO_SIM;                                /* porta de pet (só pets passam) */
    memset(w->p.objects, 0, sizeof(w->p.objects));
    w->p.object_count = 0;
    w->p.spawn_x = 3;
    w->p.spawn_y = 8;
    (void)Simulation_AddObject(w, OBJ_FRIDGE, 2, 2, NULL);
    (void)Simulation_AddObject(w, OBJ_TV, 4, 12, NULL);
    (void)Simulation_AddObject(w, OBJ_TOILET, 10, 2, NULL);
    (void)Simulation_AddObject(w, OBJ_SHOWER, 10, 13, NULL);
    (void)Simulation_AddObject(w, OBJ_PET_BOWL, 2, 13, NULL);
    (void)Simulation_AddObject(w, OBJ_BED, 20, 3, NULL);
    (void)Simulation_AddObject(w, OBJ_PET_BED, 21, 13, NULL);
    memset(w->rt.res, 0, sizeof(w->rt.res));
}

/* ===========================================================================
 * Validação / aplicação
 * ======================================================================== */

BOOL Simulation_ValidatePersist(const WorldPersist* p)
{
    u32 i, n;
    const NavGrid* g = &p->grid;

    if (p->clock.minute >= OTS3_MINUTES_PER_DAY || p->clock.speed > OTS3_MAX_SPEED ||
        p->clock.minute_ticks >= OTS3_TICKS_PER_GAME_MINUTE || p->clock.day > LIFE_MAX_DAY ||
        p->rng == 0) {
        return FALSE;
    }
    for (i = 0; i < OTS3_MAX_HOUSEHOLDS; i++) {
        const Household* h = &p->households[i];
        if (h->active > 1u || memchr(h->name, '\0', sizeof(h->name)) == NULL ||
            h->pad[0] || h->pad[1] || h->pad[2]) {
            return FALSE;
        }
    }
    if (!NavGrid_Validate(g)) {
        return FALSE;
    }
    n = (u32)g->w * g->h;
    for (i = n; i < NAV_MAX_TILES; i++) {              /* forma canônica: o resto da grade é zero */
        if (g->flags[i] != 0) {
            return FALSE;
        }
    }
    if (!Nav_Passable(g, NAV_PROFILE_SIM, p->spawn_x, p->spawn_y)) {
        return FALSE;
    }
    if (p->object_count > OTS3_MAX_OBJECTS) {
        return FALSE;
    }
    for (i = 0; i < OTS3_MAX_OBJECTS; i++) {
        const WorldObject* o = &p->objects[i];
        if (i >= p->object_count) {
            if (o->kind != 0 || o->state != 0 || o->tx != 0 || o->ty != 0) {
                return FALSE;
            }
            continue;
        }
        if (o->kind == OBJ_NONE || o->kind >= OBJ_KIND_COUNT || o->state > OBJECT_STATE_MAX ||
            (o->kind != OBJ_PET_BOWL && o->state != 0) || !InGrid(g, o->tx, o->ty) ||
            (g->flags[(u32)o->ty * g->w + o->tx] & NAV_SOLID)) {
            return FALSE;
        }
    }
    if (!EntityTable_Validate(&p->entities)) {
        return FALSE;
    }
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        const Entity* e = &p->entities.e[i];
        if (e->id == ENTITY_ID_NONE) {
            continue;
        }
        if (e->household >= OTS3_MAX_HOUSEHOLDS || !p->households[e->household].active ||
            e->pos.x < 0 || e->pos.y < 0 || !InGrid(g, FX_TILE(e->pos.x), FX_TILE(e->pos.y))) {
            return FALSE;
        }
        if (e->kind == ENTITY_KIND_PET && e->owner != ENTITY_ID_NONE) {
            const Entity* o = EntityTable_GetConst(&p->entities, e->owner);
            if (!o || o->kind != ENTITY_KIND_SIM || o->household != e->household) {
                return FALSE;                          /* dono tem que existir (id velho não vale) */
            }
        }
    }
    return Weather_Validate(&p->weather) && Lifecycle_Validate(&p->life);
}

BOOL Simulation_ApplyPersist(WorldState* w, const WorldPersist* p)
{
    if (!Simulation_ValidatePersist(p)) {
        return FALSE;
    }
    if (&w->p != p) {
        memcpy(&w->p, p, sizeof(w->p));
    }
    ResetRuntime(w);
    return TRUE;
}

void Simulation_BuildDrawList(const WorldState* w, DrawList* out)
{
    u32 i;

    memset(out, 0, sizeof(*out));
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        const Entity* e = &w->p.entities.e[i];
        DrawItem* d;

        if (e->id == ENTITY_ID_NONE) {
            continue;
        }
        d = &out->item[out->count++];
        d->id = e->id;
        d->pos = e->pos;
        d->yaw = e->yaw;
        d->model = (e->kind == ENTITY_KIND_PET)
                       ? (u8)(e->species == PET_SPECIES_DOG ? MODEL_PET_DOG : MODEL_PET_CAT)
                       : (u8)MODEL_SIM;
        d->stage = e->age_stage;
        d->action = w->rt.ent[i].action;
        d->move = w->rt.ent[i].move_state;
    }
}
