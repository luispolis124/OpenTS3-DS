#include "lifecycle.h"
#include "bytes.h"

#include <string.h>

/* ===========================================================================
 * Parâmetros de projeto (placeholders ajustáveis; NÃO são do jogo original)
 * ======================================================================== */

/* Duração, em dias de jogo, de cada fase (exceto idoso), por tempo de vida. */
static const u8 kStageLen[LIFE_SPAN_COUNT][LIFE_STAGE_ELDER] = {
    /* curto  */ { 2,  4,  7,  7, 10, 10 },
    /* normal */ { 3,  7, 14, 14, 28, 28 },
    /* longo  */ { 5, 12, 28, 28, 56, 56 }
};
/* Idoso: dias de "graça" sem risco; e dias até a morte garantida. */
static const u8 kElderGrace[LIFE_SPAN_COUNT] = {  4,  7, 14 };
static const u8 kElderMax[LIFE_SPAN_COUNT]   = { 14, 28, 56 };
/* Aumento diário da chance (permille) depois da graça = 1000 / (max - graça). */
static const u8 kElderRise[LIFE_SPAN_COUNT]  = { 100, 47, 23 };

/* Herança */
#define GENE_MUTATION_PERMILLE   30u
#define TRAIT_BOTH_PERMILLE      850u
#define TRAIT_ONE_PERMILLE       450u
#define TRAIT_NONE_PERMILLE       40u

#define LIFE_BABY_NAME "Baby"

/* ===========================================================================
 * Utilidades internas
 * ======================================================================== */

static u32 RngNext(Lifecycle* l)
{
    u32 x = l->rng;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    l->rng = x;
    return x;
}

static u32 RngRange(Lifecycle* l, u32 n)
{
    return n ? (u32)(((u64)RngNext(l) * n) >> 32) : 0u;
}

/* índice do slot do id, ou LIFE_MAX_PEOPLE se o id não resolve (livre/podado/inválido) */
static u32 PersonIdx(const Lifecycle* l, PersonId id)
{
    u32 idx = PERSON_ID_INDEX(id);

    if (id == PERSON_ID_NONE || idx == 0 || idx > LIFE_MAX_PEOPLE) {
        return LIFE_MAX_PEOPLE;
    }
    return (l->people[idx - 1u].id == id) ? idx - 1u : LIFE_MAX_PEOPLE;
}

Person* Life_Get(Lifecycle* l, PersonId id)
{
    u32 i = PersonIdx(l, id);
    return i < LIFE_MAX_PEOPLE ? &l->people[i] : NULL;
}

const Person* Life_GetConst(const Lifecycle* l, PersonId id)
{
    u32 i = PersonIdx(l, id);
    return i < LIFE_MAX_PEOPLE ? &l->people[i] : NULL;
}

static BOOL IsAlive(const Person* p)
{
    return p->id != PERSON_ID_NONE && (p->flags & LIFE_P_ALIVE) != 0;
}

static BOOL HouseholdValid(u8 h)
{
    return h < OTS3_MAX_HOUSEHOLDS;
}

static void CopyName(char* dst, const char* src)
{
    u32 i = 0;

    memset(dst, 0, OTS3_NAME_LEN);
    if (src) {
        while (i < OTS3_NAME_LEN - 1u && src[i] != '\0') {
            dst[i] = src[i];
            i++;
        }
    }
}

static void Emit(LifeContext* c, u8 type, u8 arg, PersonId a, PersonId b, EntityId ent)
{
    LifeEventQueue* q;
    LifeEvent* e;

    if (!c || !c->events) {
        return;
    }
    q = c->events;
    if (q->count >= LIFE_EVENT_CAP) {
        if (q->dropped < 0xFFFFu) {
            q->dropped++;
        }
        return;                                    /* cheia: descarta o NOVO */
    }
    e = &q->ev[(q->head + q->count) % LIFE_EVENT_CAP];
    e->type = type; e->arg = arg; e->person = a; e->other = b; e->pad = 0; e->entity = ent;
    q->count++;
}

void LifeEvents_Init(LifeEventQueue* q)
{
    memset(q, 0, sizeof(*q));
}

BOOL LifeEvents_Pop(LifeEventQueue* q, LifeEvent* out)
{
    if (q->count == 0) {
        return FALSE;
    }
    *out = q->ev[q->head];
    q->head = (u8)((q->head + 1u) % LIFE_EVENT_CAP);
    q->count--;
    return TRUE;
}

/* ===========================================================================
 * Idade e fases
 * ======================================================================== */

static u8 ClampSpan(u8 span)
{
    return span < LIFE_SPAN_COUNT ? span : (u8)LIFE_SPAN_NORMAL;
}

u32 Life_StageStartDays(u8 lifespan, u8 stage)
{
    u32 s, sum = 0;

    lifespan = ClampSpan(lifespan);
    if (stage > LIFE_STAGE_ELDER) {
        stage = LIFE_STAGE_ELDER;
    }
    for (s = 0; s < stage; s++) {
        sum += kStageLen[lifespan][s];
    }
    return sum;
}

u8 Life_StageOfAge(u8 lifespan, u32 age_days)
{
    u32 s, end = 0;

    lifespan = ClampSpan(lifespan);
    for (s = 0; s < LIFE_STAGE_ELDER; s++) {
        end += kStageLen[lifespan][s];
        if (age_days < end) {
            return (u8)s;
        }
    }
    return LIFE_STAGE_ELDER;
}

u32 Life_AgeDays(const Person* p, u32 today)
{
    s32 age = (s32)today - p->birth_day;       /* |today|,|birth| <= 2^29: sem estouro */

    return age < 0 ? 0u : (u32)age;
}

u8 Life_StageOf(const Lifecycle* l, const Person* p, u32 today)
{
    return Life_StageOfAge(l->lifespan, Life_AgeDays(p, today));
}

u32 Life_DeathChancePermille(u8 lifespan, u32 age_days)
{
    u32 start, d, chance;

    lifespan = ClampSpan(lifespan);
    start = Life_StageStartDays(lifespan, LIFE_STAGE_ELDER);
    if (age_days < start + kElderGrace[lifespan]) {
        return 0;
    }
    d = age_days - start;
    if (d >= kElderMax[lifespan]) {
        return 1000;                            /* limite duro: morte garantida */
    }
    chance = (d - kElderGrace[lifespan]) * kElderRise[lifespan];
    return chance > 999u ? 999u : chance;
}

u32 Life_StageCaps(u8 stage)
{
    switch (stage) {
    case LIFE_STAGE_BABY:        return 0;
    case LIFE_STAGE_TODDLER:     return LIFE_CAP_WALK;
    case LIFE_STAGE_CHILD:       return LIFE_CAP_WALK | LIFE_CAP_USE_OBJECTS;
    case LIFE_STAGE_TEEN:        return LIFE_CAP_WALK | LIFE_CAP_USE_OBJECTS | LIFE_CAP_CARRY_BABY;
    case LIFE_STAGE_YOUNG_ADULT:
    case LIFE_STAGE_ADULT:       return LIFE_CAP_WALK | LIFE_CAP_USE_OBJECTS | LIFE_CAP_WORK |
                                        LIFE_CAP_PARTNER | LIFE_CAP_CARRY_BABY;
    case LIFE_STAGE_ELDER:       return LIFE_CAP_WALK | LIFE_CAP_USE_OBJECTS |
                                        LIFE_CAP_PARTNER | LIFE_CAP_CARRY_BABY;
    default:                     return 0;
    }
}

/* ===========================================================================
 * Tabela de pessoas: criação, poda, remoção
 * ======================================================================== */

void Lifecycle_Init(Lifecycle* l, u32 seed, u32 today)
{
    u32 i;

    memset(l, 0, sizeof(*l));
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        l->gen[i] = 1;
    }
    l->lifespan = LIFE_SPAN_NORMAL;
    l->settings = LIFE_SET_AGING | LIFE_SET_NATURAL_DEATH;
    l->last_day = today > LIFE_MAX_DAY ? LIFE_MAX_DAY : today;
    l->rng = seed ? seed : 0x9E3779B9u;
}

void Life_SetLifespan(Lifecycle* l, u8 lifespan)
{
    l->lifespan = ClampSpan(lifespan);
}

void Life_SetSettings(Lifecycle* l, u8 settings)
{
    l->settings = (u8)(settings & LIFE_SET_MASK);
}

static void FreeSlot(Lifecycle* l, u32 i)
{
    memset(&l->people[i], 0, sizeof(Person));
    l->gen[i] = (u8)(l->gen[i] + 1u);
    if (l->gen[i] == 0) {
        l->gen[i] = 1;                          /* geração 0 é reservada */
    }
}

/* Libera UMA pessoa morta que não seja ancestral (até 3 gerações acima) de ninguém vivo.
 * Escolhe a de morte mais antiga. FALSE se não há candidata. */
static BOOL PruneOne(Lifecycle* l)
{
    u8 lvl[LIFE_MAX_PEOPLE];
    u32 i, k, pass, victim = LIFE_MAX_PEOPLE;

    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        lvl[i] = IsAlive(&l->people[i]) ? 4u : 0u;
    }
    for (pass = 0; pass < 3; pass++) {          /* vivo=4, pai=3, avô=2, bisavô=1 (protegidos) */
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
            if (lvl[i] <= 1u) {
                continue;
            }
            for (k = 0; k < 2; k++) {
                u32 j = PersonIdx(l, l->people[i].parent[k]);
                if (j < LIFE_MAX_PEOPLE && lvl[j] < lvl[i] - 1u) {
                    lvl[j] = (u8)(lvl[i] - 1u);
                }
            }
        }
    }
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        const Person* p = &l->people[i];
        if (p->id == PERSON_ID_NONE || IsAlive(p) || lvl[i] != 0) {
            continue;
        }
        if (victim == LIFE_MAX_PEOPLE || p->death_day < l->people[victim].death_day) {
            victim = i;
        }
    }
    if (victim == LIFE_MAX_PEOPLE) {
        return FALSE;
    }
    FreeSlot(l, victim);
    return TRUE;
}

/* slot livre (podando se preciso) ou LIFE_MAX_PEOPLE */
static u32 AllocPerson(Lifecycle* l)
{
    u32 pass, i;

    for (pass = 0; pass < 2; pass++) {
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
            if (l->people[i].id == PERSON_ID_NONE) {
                return i;
            }
        }
        if (!PruneOne(l)) {
            break;
        }
    }
    return LIFE_MAX_PEOPLE;
}

/* vivos da família + gestações em curso (cada uma vai virar mais um) */
static u32 HouseholdLoad(const Lifecycle* l, u8 household)
{
    u32 i, n = 0;

    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        const Person* p = &l->people[i];
        if (IsAlive(p) && p->household == household) {
            n += 1u + (p->preg_left ? 1u : 0u);
        }
    }
    return n;
}

LifeResult Life_AddPerson(Lifecycle* l, const LifePersonInit* init, u32 today, PersonId* out)
{
    u32 slot, k;
    s32 birth;
    Person* p;

    if (!l || !init || today > LIFE_MAX_DAY || init->age_days > LIFE_MAX_DAY ||
        init->gender >= LIFE_GENDER_COUNT) {
        return LIFE_ERR_ARG;
    }
    if (init->household != LIFE_NO_HOUSEHOLD && !HouseholdValid(init->household)) {
        return LIFE_ERR_ARG;
    }
    if (init->wants_entity && !HouseholdValid(init->household)) {
        return LIFE_ERR_ARG;
    }
    birth = (s32)today - (s32)init->age_days;
    for (k = 0; k < 2; k++) {
        const Person* par;
        if (init->parent[k] == PERSON_ID_NONE) {
            continue;
        }
        par = Life_GetConst(l, init->parent[k]);
        if (!par) {
            return LIFE_ERR_ARG;
        }
        if (par->birth_day >= birth) {
            return LIFE_ERR_RULE;               /* pai precisa ter nascido ESTRITAMENTE antes */
        }
    }
    if (init->parent[0] != PERSON_ID_NONE && init->parent[0] == init->parent[1]) {
        return LIFE_ERR_ARG;
    }
    if (HouseholdValid(init->household) && HouseholdLoad(l, init->household) >= LIFE_MAX_HOUSEHOLD_SIMS) {
        return LIFE_ERR_RULE;
    }
    slot = AllocPerson(l);
    if (slot >= LIFE_MAX_PEOPLE) {
        return LIFE_ERR_FULL;
    }
    p = &l->people[slot];
    memset(p, 0, sizeof(*p));
    p->id = PERSON_ID_MAKE(slot + 1u, l->gen[slot]);
    p->birth_day = birth;
    p->parent[0] = init->parent[0];
    p->parent[1] = init->parent[1];
    p->genes = init->genes ? init->genes : RngNext(l);
    p->traits = init->traits;
    p->household = init->household;
    p->gender = init->gender;
    p->flags = (u8)(LIFE_P_ALIVE | (init->can_carry ? LIFE_P_CAN_CARRY : 0u) |
                    (init->no_aging ? LIFE_P_NO_AGING : 0u) |
                    (init->wants_entity ? LIFE_P_WANTS_ENTITY : 0u));
    CopyName(p->name, init->name);
    if (out) {
        *out = p->id;
    }
    return LIFE_OK;
}

LifeResult Life_SetName(Lifecycle* l, PersonId id, const char* name)
{
    Person* p = Life_Get(l, id);

    if (!p) {
        return LIFE_ERR_ARG;
    }
    CopyName(p->name, name);
    return LIFE_OK;
}

LifeResult Life_SetWantsEntity(Lifecycle* l, PersonId id, BOOL wants)
{
    Person* p = Life_Get(l, id);

    if (!p || !IsAlive(p) || (wants && !HouseholdValid(p->household))) {
        return p ? LIFE_ERR_STATE : LIFE_ERR_ARG;
    }
    p->flags = (u8)(wants ? (p->flags | LIFE_P_WANTS_ENTITY) : (p->flags & ~LIFE_P_WANTS_ENTITY));
    return LIFE_OK;
}

/* ===========================================================================
 * Corpos (Entities)
 * ======================================================================== */

static EntityId SpawnBody(LifeContext* c, const Person* p, Vec2Fx pos)
{
    if (c->hooks && c->hooks->spawn_sim) {
        return c->hooks->spawn_sim(c->hooks->user, p->household, p->name, pos);
    }
    return EntityTable_Spawn(c->ents, ENTITY_KIND_SIM, PET_SPECIES_NONE, p->household,
                             ENTITY_ID_NONE, p->name, pos);
}

static void DespawnBody(LifeContext* c, EntityId id)
{
    if (c->hooks && c->hooks->despawn) {
        c->hooks->despawn(c->hooks->user, id);
    } else {
        (void)EntityTable_Despawn(c->ents, id);
    }
}

/* Cria o corpo de 'p' e liga as duas pontas. FALSE se não coube. */
static BOOL AttachBody(Lifecycle* l, LifeContext* c, Person* p, Vec2Fx pos, u32 today)
{
    EntityId id = SpawnBody(c, p, pos);
    Entity* e;
    u32 i;

    if (id == ENTITY_ID_NONE) {
        return FALSE;
    }
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {      /* nunca dois donos para o mesmo corpo */
        if (&l->people[i] != p && l->people[i].entity == id) {
            l->people[i].entity = ENTITY_ID_NONE;
        }
    }
    p->entity = id;
    e = EntityTable_Get(c->ents, id);
    if (e) {
        e->age_stage = Life_StageOf(l, p, today);
    }
    Emit(c, LIFE_EVENT_ENTITY_SPAWNED, 0, p->id, PERSON_ID_NONE, id);
    return TRUE;
}

/* pets do morto passam ao herdeiro (primeiro membro vivo da família, criança ou mais velho) */
static void ReassignPets(Lifecycle* l, LifeContext* c, const Person* dead)
{
    EntityId heir = ENTITY_ID_NONE;
    u32 i;

    if (HouseholdValid(dead->household)) {
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
            const Person* q = &l->people[i];
            const Entity* e;
            if (q == dead || !IsAlive(q) || q->household != dead->household || q->entity == ENTITY_ID_NONE) {
                continue;
            }
            e = EntityTable_GetConst(c->ents, q->entity);
            if (e && e->kind == ENTITY_KIND_SIM && e->age_stage >= LIFE_STAGE_CHILD) {
                heir = q->entity;
                break;
            }
        }
    }
    for (i = 0; i < OTS3_MAX_ENTITIES; i++) {
        Entity* e = &c->ents->e[i];
        if (e->id != ENTITY_ID_NONE && e->kind == ENTITY_KIND_PET && e->owner == dead->entity) {
            e->owner = heir;
        }
    }
}

/* ===========================================================================
 * Morte
 * ======================================================================== */

static void KillInternal(Lifecycle* l, LifeContext* c, Person* p, u8 cause, u32 today)
{
    Person* partner = Life_Get(l, p->partner);
    EntityId body = p->entity;
    PersonId partner_id = p->partner;

    if (partner) {
        partner->partner = PERSON_ID_NONE;
    }
    if (body != ENTITY_ID_NONE && EntityTable_GetConst(c->ents, body)) {
        ReassignPets(l, c, p);
        DespawnBody(c, body);
    }
    p->flags = (u8)(p->flags & ~(LIFE_P_ALIVE | LIFE_P_WANTS_ENTITY | LIFE_P_DEFERRED));
    p->death_day = (s32)today;
    p->cause = cause;
    p->preg_left = 0;
    p->preg_other = PERSON_ID_NONE;
    p->partner = PERSON_ID_NONE;
    p->entity = ENTITY_ID_NONE;
    Emit(c, LIFE_EVENT_DEATH, cause, p->id, partner_id, body);
}

LifeResult Life_Kill(Lifecycle* l, LifeContext* ctx, PersonId id, u8 cause, u32 today)
{
    Person* p;

    if (!l || !ctx || !ctx->ents || today > LIFE_MAX_DAY ||
        cause == LIFE_CAUSE_NONE || cause >= LIFE_CAUSE_COUNT) {
        return LIFE_ERR_ARG;
    }
    p = Life_Get(l, id);
    if (!p) {
        return LIFE_ERR_ARG;
    }
    if (!IsAlive(p)) {
        return LIFE_ERR_STATE;
    }
    KillInternal(l, ctx, p, cause, today);
    return LIFE_OK;
}

/* ===========================================================================
 * Parentesco
 * ======================================================================== */

/* pais RESOLVIDOS de 'id' (pai podado vira desconhecido) */
static void ParentsOf(const Lifecycle* l, PersonId id, PersonId out[2])
{
    u32 i = PersonIdx(l, id), k;

    out[0] = out[1] = PERSON_ID_NONE;
    if (i >= LIFE_MAX_PEOPLE) {
        return;
    }
    for (k = 0; k < 2; k++) {
        PersonId par = l->people[i].parent[k];
        out[k] = (PersonIdx(l, par) < LIFE_MAX_PEOPLE) ? par : PERSON_ID_NONE;
    }
}

static u32 SharedParents(const PersonId a[2], const PersonId b[2])
{
    u32 i, n = 0;

    for (i = 0; i < 2; i++) {
        if (a[i] != PERSON_ID_NONE && (a[i] == b[0] || a[i] == b[1])) {
            n++;
        }
    }
    return n;
}

static BOOL Siblingish(const Lifecycle* l, PersonId x, PersonId y)
{
    PersonId px[2], py[2];

    if (x == y) {
        return FALSE;
    }
    ParentsOf(l, x, px);
    ParentsOf(l, y, py);
    return SharedParents(px, py) >= 1u;
}

#define LIFE_ANCESTOR_DEPTH 8u

/* 'anc' é ancestral de 'desc' até LIFE_ANCESTOR_DEPTH gerações? (busca limitada) */
static BOOL IsAncestor(const Lifecycle* l, PersonId anc, PersonId desc)
{
    u8 seen[LIFE_MAX_PEOPLE];
    u8 queue[LIFE_MAX_PEOPLE];
    u8 depth[LIFE_MAX_PEOPLE];
    u32 head = 0, tail = 0, start = PersonIdx(l, desc);

    if (start >= LIFE_MAX_PEOPLE || PersonIdx(l, anc) >= LIFE_MAX_PEOPLE) {
        return FALSE;
    }
    memset(seen, 0, sizeof(seen));
    seen[start] = 1; queue[tail] = (u8)start; depth[tail] = 0; tail++;
    while (head < tail) {
        u32 cur = queue[head], d = depth[head], k;
        head++;
        if (d >= LIFE_ANCESTOR_DEPTH) {
            continue;
        }
        for (k = 0; k < 2; k++) {
            PersonId par = l->people[cur].parent[k];
            u32 j = PersonIdx(l, par);
            if (j >= LIFE_MAX_PEOPLE || seen[j]) {
                continue;
            }
            if (par == anc) {
                return TRUE;
            }
            seen[j] = 1; queue[tail] = (u8)j; depth[tail] = (u8)(d + 1u); tail++;
        }
    }
    return FALSE;
}

u8 Life_Relationship(const Lifecycle* l, PersonId a, PersonId b)
{
    PersonId pa[2], pb[2], gpa[2], gpb[2];
    u32 ia = PersonIdx(l, a), ib = PersonIdx(l, b), s, i, j;

    if (ia >= LIFE_MAX_PEOPLE || ib >= LIFE_MAX_PEOPLE) {
        return LIFE_REL_UNKNOWN;
    }
    if (a == b) {
        return LIFE_REL_SELF;
    }
    ParentsOf(l, a, pa);
    ParentsOf(l, b, pb);
    if (b == pa[0] || b == pa[1]) {
        return LIFE_REL_PARENT;
    }
    if (a == pb[0] || a == pb[1]) {
        return LIFE_REL_CHILD;
    }
    s = SharedParents(pa, pb);
    if (s >= 2u) {
        return LIFE_REL_SIBLING;
    }
    if (s == 1u) {
        return LIFE_REL_HALF_SIBLING;
    }
    for (i = 0; i < 2; i++) {                   /* avós */
        if (pa[i] != PERSON_ID_NONE) {
            ParentsOf(l, pa[i], gpa);
            if (b == gpa[0] || b == gpa[1]) {
                return LIFE_REL_GRANDPARENT;
            }
        }
        if (pb[i] != PERSON_ID_NONE) {
            ParentsOf(l, pb[i], gpb);
            if (a == gpb[0] || a == gpb[1]) {
                return LIFE_REL_GRANDCHILD;
            }
        }
    }
    for (i = 0; i < 2; i++) {                   /* tios, sobrinhos, primos */
        if (pa[i] != PERSON_ID_NONE && Siblingish(l, pa[i], b)) {
            return LIFE_REL_AUNT_UNCLE;
        }
        if (pb[i] != PERSON_ID_NONE && Siblingish(l, pb[i], a)) {
            return LIFE_REL_NIECE_NEPHEW;
        }
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            if (pa[i] != PERSON_ID_NONE && pb[j] != PERSON_ID_NONE && Siblingish(l, pa[i], pb[j])) {
                return LIFE_REL_COUSIN;
            }
        }
    }
    if (IsAncestor(l, b, a)) {
        return LIFE_REL_ANCESTOR;
    }
    if (IsAncestor(l, a, b)) {
        return LIFE_REL_DESCENDANT;
    }
    if (l->people[ia].partner == b) {
        return LIFE_REL_PARTNER;
    }
    return LIFE_REL_UNRELATED;
}

BOOL Life_IsCloseRelative(const Lifecycle* l, PersonId a, PersonId b)
{
    switch (Life_Relationship(l, a, b)) {
    case LIFE_REL_PARENT: case LIFE_REL_CHILD: case LIFE_REL_SIBLING: case LIFE_REL_HALF_SIBLING:
    case LIFE_REL_GRANDPARENT: case LIFE_REL_GRANDCHILD: case LIFE_REL_AUNT_UNCLE:
    case LIFE_REL_NIECE_NEPHEW: case LIFE_REL_ANCESTOR: case LIFE_REL_DESCENDANT:
        return TRUE;
    default:
        return FALSE;
    }
}

u32 Life_GetChildren(const Lifecycle* l, PersonId parent, PersonId* out, u32 cap)
{
    u32 i, n = 0;

    if (PersonIdx(l, parent) >= LIFE_MAX_PEOPLE || !out || cap == 0) {
        return 0;
    }
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        const Person* c = &l->people[i];
        u32 pos;

        if (c->id == PERSON_ID_NONE || (c->parent[0] != parent && c->parent[1] != parent)) {
            continue;
        }
        pos = n;                                 /* inserção ordenada por nascimento (estável) */
        while (pos > 0 && Life_GetConst(l, out[pos - 1u])->birth_day > c->birth_day) {
            pos--;
        }
        if (pos >= cap) {
            continue;
        }
        if (n < cap) {
            n++;
        }
        {
            u32 m;
            for (m = n - 1u; m > pos; m--) {
                out[m] = out[m - 1u];
            }
        }
        out[pos] = c->id;
    }
    return n;
}

u32 Life_GetHousehold(const Lifecycle* l, u8 household, BOOL only_alive, PersonId* out, u32 cap)
{
    u32 i, n = 0;

    for (i = 0; i < LIFE_MAX_PEOPLE && n < cap; i++) {
        const Person* p = &l->people[i];
        if (p->id != PERSON_ID_NONE && p->household == household && (!only_alive || IsAlive(p))) {
            out[n++] = p->id;
        }
    }
    return n;
}

u32 Life_CountAlive(const Lifecycle* l)
{
    u32 i, n = 0;

    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        if (IsAlive(&l->people[i])) {
            n++;
        }
    }
    return n;
}

u32 Life_BuildTree(const Lifecycle* l, PersonId root, u8 direction, u8 max_depth, LifeTree* out)
{
    u32 head = 0;

    memset(out, 0, sizeof(*out));
    if (PersonIdx(l, root) >= LIFE_MAX_PEOPLE) {
        return 0;
    }
    out->node[0].id = root; out->node[0].parent_node = -1; out->node[0].depth = 0;
    out->count = 1;
    while (head < out->count) {
        PersonId kids[LIFE_TREE_MAX];
        u32 n, k;
        LifeTreeNode cur = out->node[head];

        if (cur.depth < max_depth) {
            if (direction == LIFE_TREE_ANCESTORS) {
                PersonId par[2];
                ParentsOf(l, cur.id, par);
                n = 2;
                for (k = 0; k < n; k++) { kids[k] = par[k]; }
            } else {
                n = Life_GetChildren(l, cur.id, kids, LIFE_TREE_MAX);
            }
            for (k = 0; k < n; k++) {
                if (kids[k] == PERSON_ID_NONE) {
                    continue;
                }
                if (out->count >= LIFE_TREE_MAX) {
                    out->truncated = 1;
                    break;
                }
                out->node[out->count].id = kids[k];
                out->node[out->count].parent_node = (s8)head;
                out->node[out->count].depth = (u8)(cur.depth + 1u);
                out->count++;
            }
        }
        head++;
    }
    return out->count;
}

/* ===========================================================================
 * Parceria
 * ======================================================================== */

BOOL Life_CanPartner(const Lifecycle* l, PersonId a, PersonId b, u32 today)
{
    const Person* pa = Life_GetConst(l, a);
    const Person* pb = Life_GetConst(l, b);

    if (!pa || !pb || a == b || !IsAlive(pa) || !IsAlive(pb)) {
        return FALSE;
    }
    if (pa->partner != PERSON_ID_NONE || pb->partner != PERSON_ID_NONE) {
        return FALSE;
    }
    if (!(Life_StageCaps(Life_StageOf(l, pa, today)) & LIFE_CAP_PARTNER) ||
        !(Life_StageCaps(Life_StageOf(l, pb, today)) & LIFE_CAP_PARTNER)) {
        return FALSE;
    }
    return !Life_IsCloseRelative(l, a, b);
}

LifeResult Life_SetPartner(Lifecycle* l, LifeContext* ctx, PersonId a, PersonId b, u32 today)
{
    Person* pa;
    Person* pb;

    if (!l || today > LIFE_MAX_DAY) {
        return LIFE_ERR_ARG;
    }
    pa = Life_Get(l, a);
    pb = Life_Get(l, b);
    if (!pa || !pb || a == b) {
        return LIFE_ERR_ARG;
    }
    if (!Life_CanPartner(l, a, b, today)) {
        return (IsAlive(pa) && IsAlive(pb)) ? LIFE_ERR_RULE : LIFE_ERR_STATE;
    }
    pa->partner = b;
    pb->partner = a;
    Emit(ctx, LIFE_EVENT_PARTNER, 0, a, b, ENTITY_ID_NONE);
    return LIFE_OK;
}

LifeResult Life_ClearPartner(Lifecycle* l, LifeContext* ctx, PersonId a)
{
    Person* pa = Life_Get(l, a);
    Person* pb;
    PersonId b;

    if (!pa) {
        return LIFE_ERR_ARG;
    }
    if (pa->partner == PERSON_ID_NONE) {
        return LIFE_ERR_STATE;
    }
    b = pa->partner;
    pb = Life_Get(l, b);
    pa->partner = PERSON_ID_NONE;
    if (pb) {
        pb->partner = PERSON_ID_NONE;
    }
    Emit(ctx, LIFE_EVENT_BREAKUP, 0, a, b, ENTITY_ID_NONE);
    return LIFE_OK;
}

/* ===========================================================================
 * Gravidez e nascimento
 * ======================================================================== */

LifeResult Life_StartPregnancy(Lifecycle* l, LifeContext* ctx, PersonId carrier,
                               PersonId other, u32 today)
{
    Person* c;
    const Person* o;
    u8 stage;

    if (!l || today > LIFE_MAX_DAY) {
        return LIFE_ERR_ARG;
    }
    c = Life_Get(l, carrier);
    o = Life_GetConst(l, other);
    if (!c || !o || carrier == other) {
        return LIFE_ERR_ARG;
    }
    if (!IsAlive(c) || !IsAlive(o) || c->preg_left != 0) {
        return LIFE_ERR_STATE;
    }
    stage = Life_StageOf(l, c, today);
    if (!(c->flags & LIFE_P_CAN_CARRY) ||
        (stage != LIFE_STAGE_YOUNG_ADULT && stage != LIFE_STAGE_ADULT) ||
        !(Life_StageCaps(Life_StageOf(l, o, today)) & LIFE_CAP_PARTNER) ||
        c->partner != other || o->partner != carrier) {
        return LIFE_ERR_RULE;
    }
    if (HouseholdValid(c->household) && HouseholdLoad(l, c->household) >= LIFE_MAX_HOUSEHOLD_SIMS) {
        return LIFE_ERR_RULE;
    }
    c->preg_left = (u8)LIFE_PREGNANCY_DAYS;
    c->preg_other = other;
    Emit(ctx, LIFE_EVENT_PREGNANCY, 0, carrier, other, ENTITY_ID_NONE);
    return LIFE_OK;
}

static u32 InheritGenes(Lifecycle* l, const Person* a, const Person* b)
{
    u32 g = 0, k;

    for (k = 0; k < 8; k++) {
        u32 shift = k * 4u, nib;
        if (RngRange(l, 1000) < GENE_MUTATION_PERMILLE) {
            nib = RngRange(l, 16);
        } else if (b == NULL || (RngNext(l) & 1u) == 0) {
            nib = (a->genes >> shift) & 0xFu;
        } else {
            nib = (b->genes >> shift) & 0xFu;
        }
        g |= nib << shift;
    }
    return g;
}

static u16 InheritTraits(Lifecycle* l, const Person* a, const Person* b)
{
    u32 t = 0, k;

    for (k = 0; k < 16; k++) {
        u32 have = ((a->traits >> k) & 1u) + (b ? ((b->traits >> k) & 1u) : 0u);
        u32 chance = have == 2u ? TRAIT_BOTH_PERMILLE : (have == 1u ? TRAIT_ONE_PERMILLE : TRAIT_NONE_PERMILLE);
        if (RngRange(l, 1000) < chance) {
            t |= 1u << k;
        }
    }
    return (u16)t;
}

/* FALSE = adiado (sem espaço); a gestação continua e tenta de novo no dia seguinte. */
static BOOL DoBirth(Lifecycle* l, LifeContext* c, u32 carrier_slot, u32 today)
{
    u32 slot = AllocPerson(l);
    Person* mom = &l->people[carrier_slot];      /* gestante está viva => nunca é podada */
    const Person* other;
    Person* b;
    Vec2Fx pos = c->spawn_pos;

    if (slot >= LIFE_MAX_PEOPLE) {
        return FALSE;
    }
    other = Life_GetConst(l, mom->preg_other);   /* genitor morto continua sendo genitor */
    b = &l->people[slot];
    memset(b, 0, sizeof(*b));
    b->id = PERSON_ID_MAKE(slot + 1u, l->gen[slot]);
    b->birth_day = (s32)today;
    b->parent[0] = mom->id;
    b->parent[1] = other ? other->id : PERSON_ID_NONE;
    b->genes = InheritGenes(l, mom, other);
    b->traits = InheritTraits(l, mom, other);
    b->gender = (u8)(RngRange(l, 2) ? LIFE_GENDER_MALE : LIFE_GENDER_FEMALE);
    b->household = mom->household;
    b->flags = (u8)(LIFE_P_ALIVE | (b->gender == LIFE_GENDER_FEMALE ? LIFE_P_CAN_CARRY : 0u) |
                    (mom->flags & LIFE_P_WANTS_ENTITY));
    CopyName(b->name, LIFE_BABY_NAME);
    Emit(c, LIFE_EVENT_BIRTH, 0, b->id, mom->id, ENTITY_ID_NONE);
    if (b->flags & LIFE_P_WANTS_ENTITY) {
        const Entity* me = EntityTable_GetConst(c->ents, mom->entity);
        if (me) {
            pos = me->pos;
        }
        (void)AttachBody(l, c, b, pos, today);   /* se não couber, o Life_Update tenta depois */
    }
    return TRUE;
}

/* ===========================================================================
 * Atualização diária
 * ======================================================================== */

static BOOL Aging(const Lifecycle* l, const Person* p)
{
    return (l->settings & LIFE_SET_AGING) && !(p->flags & LIFE_P_NO_AGING);
}

static void SyncEntityStage(const Lifecycle* l, LifeContext* c, const Person* p, u32 day)
{
    Entity* e;

    if (p->entity == ENTITY_ID_NONE) {
        return;
    }
    e = EntityTable_Get(c->ents, p->entity);
    if (e) {
        e->age_stage = Life_StageOf(l, p, day);
    }
}

u32 Life_MaterializeBodies(Lifecycle* l, LifeContext* ctx, u32 today)
{
    u32 i, n = 0;

    if (!l || !ctx || !ctx->ents) {
        return 0;
    }
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {      /* corpos pendentes (início do jogo, nascimento sem espaço) */
        Person* p = &l->people[i];
        if (IsAlive(p) && (p->flags & LIFE_P_WANTS_ENTITY) && p->entity == ENTITY_ID_NONE) {
            if (!AttachBody(l, ctx, p, ctx->spawn_pos, today)) {
                break;                           /* tabela de entidades cheia: tenta depois */
            }
            n++;
        }
    }
    return n;
}

static void ProcessDay(Lifecycle* l, LifeContext* c, u32 day)
{
    u32 i;

    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        Person* p = &l->people[i];
        u8 stage;

        if (!IsAlive(p)) {
            continue;
        }
        if (p->entity != ENTITY_ID_NONE && !EntityTable_GetConst(c->ents, p->entity)) {
            p->entity = ENTITY_ID_NONE;          /* corpo sumiu: desfaz o vínculo velho */
        }
        if (p->birth_day >= (s32)day) {
            continue;                            /* nasceu hoje */
        }
        if (!Aging(l, p)) {
            p->birth_day += 1;                   /* idade congelada */
        } else {
            u32 age = Life_AgeDays(p, day);
            u8 before = Life_StageOfAge(l->lifespan, age - 1u);
            u8 now = Life_StageOfAge(l->lifespan, age);
            if (now != before) {
                Emit(c, LIFE_EVENT_STAGE_CHANGE, now, p->id, PERSON_ID_NONE, p->entity);
            }
        }
        SyncEntityStage(l, c, p, day);
        stage = Life_StageOf(l, p, day);

        if (p->preg_left > 1u) {
            p->preg_left--;
        } else if (p->preg_left == 1u) {
            if (DoBirth(l, c, i, day)) {
                p->preg_left = 0;
                p->preg_other = PERSON_ID_NONE;
                p->flags = (u8)(p->flags & ~LIFE_P_DEFERRED);
            } else if (!(p->flags & LIFE_P_DEFERRED)) {
                p->flags |= LIFE_P_DEFERRED;
                Emit(c, LIFE_EVENT_BIRTH_DEFERRED, 0, p->id, p->preg_other, ENTITY_ID_NONE);
            }
        }
        if (Aging(l, p) && (l->settings & LIFE_SET_NATURAL_DEATH) && stage == LIFE_STAGE_ELDER) {
            u32 chance = Life_DeathChancePermille(l->lifespan, Life_AgeDays(p, day));
            if (chance >= 1000u || RngRange(l, 1000) < chance) {
                KillInternal(l, c, p, LIFE_CAUSE_OLD_AGE, day);
            }
        }
    }
    (void)Life_MaterializeBodies(l, c, day);
}

u32 Life_Update(Lifecycle* l, LifeContext* ctx, u32 today)
{
    u32 processed = 0, gap;

    if (!l || !ctx || !ctx->ents || today > LIFE_MAX_DAY || today <= l->last_day) {
        return 0;
    }
    gap = today - l->last_day;
    if (gap > LIFE_MAX_CATCHUP_DAYS) {
        u32 skipped = gap - LIFE_MAX_CATCHUP_DAYS, i;
        for (i = 0; i < LIFE_MAX_PEOPLE; i++) {  /* quem tem idade congelada continua congelado */
            if (IsAlive(&l->people[i]) && !Aging(l, &l->people[i])) {
                l->people[i].birth_day += (s32)skipped;
            }
        }
        l->last_day = today - LIFE_MAX_CATCHUP_DAYS;
    }
    while (l->last_day < today) {
        l->last_day++;
        ProcessDay(l, ctx, l->last_day);
        processed++;
    }
    return processed;
}

/* ===========================================================================
 * Validação
 * ======================================================================== */

static BOOL DayInRange(s32 d)
{
    return d >= -(s32)LIFE_MAX_DAY && d <= (s32)LIFE_MAX_DAY;
}

BOOL Lifecycle_Validate(const Lifecycle* l)
{
    u32 i, j, k;

    if (l->lifespan >= LIFE_SPAN_COUNT || (l->settings & ~LIFE_SET_MASK) || l->rng == 0 ||
        l->last_day > LIFE_MAX_DAY) {
        return FALSE;
    }
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        const Person* p = &l->people[i];

        if (l->gen[i] == 0) {
            return FALSE;
        }
        if (p->id == PERSON_ID_NONE) {
            continue;
        }
        if (PERSON_ID_INDEX(p->id) != i + 1u || PERSON_ID_GEN(p->id) != l->gen[i]) {
            return FALSE;
        }
        if (memchr(p->name, '\0', sizeof(p->name)) == NULL || p->pad != 0 ||
            (p->flags & ~LIFE_P_FLAGS_MASK) || p->gender >= LIFE_GENDER_COUNT ||
            p->cause >= LIFE_CAUSE_COUNT) {
            return FALSE;
        }
        if (p->household != LIFE_NO_HOUSEHOLD && !HouseholdValid(p->household)) {
            return FALSE;
        }
        if (!DayInRange(p->birth_day) || !DayInRange(p->death_day)) {
            return FALSE;
        }
        if ((p->flags & LIFE_P_WANTS_ENTITY) && !HouseholdValid(p->household)) {
            return FALSE;
        }
        if (IsAlive(p)) {
            if (p->cause != LIFE_CAUSE_NONE || p->death_day != 0) {
                return FALSE;
            }
        } else {
            if (p->cause == LIFE_CAUSE_NONE || p->death_day < p->birth_day || p->preg_left != 0 ||
                p->preg_other != PERSON_ID_NONE || p->partner != PERSON_ID_NONE ||
                p->entity != ENTITY_ID_NONE || (p->flags & (LIFE_P_WANTS_ENTITY | LIFE_P_DEFERRED))) {
                return FALSE;
            }
        }
        for (k = 0; k < 2; k++) {                /* pais: índice válido; se resolve, nasceu antes */
            PersonId par = p->parent[k];
            if (par == PERSON_ID_NONE) {
                continue;
            }
            if (PERSON_ID_INDEX(par) == 0 || PERSON_ID_INDEX(par) > LIFE_MAX_PEOPLE ||
                PERSON_ID_GEN(par) == 0) {
                return FALSE;
            }
            j = PersonIdx(l, par);
            if (j < LIFE_MAX_PEOPLE && l->people[j].birth_day >= p->birth_day) {
                return FALSE;                    /* garante o DAG: sem ciclos nem auto-referência */
            }
        }
        if (p->parent[0] != PERSON_ID_NONE && p->parent[0] == p->parent[1]) {
            return FALSE;
        }
        if (p->partner != PERSON_ID_NONE) {
            j = PersonIdx(l, p->partner);
            if (j >= LIFE_MAX_PEOPLE || j == i || !IsAlive(&l->people[j]) ||
                l->people[j].partner != p->id) {
                return FALSE;                    /* vínculo sempre mútuo e entre vivos */
            }
        }
        if (p->preg_left > LIFE_PREGNANCY_DAYS ||
            (p->preg_left > 0 && (!IsAlive(p) || !(p->flags & LIFE_P_CAN_CARRY)))) {
            return FALSE;
        }
        if (p->preg_other != PERSON_ID_NONE &&
            (p->preg_left == 0 || PERSON_ID_INDEX(p->preg_other) == 0 ||
             PERSON_ID_INDEX(p->preg_other) > LIFE_MAX_PEOPLE)) {
            return FALSE;
        }
        if ((p->flags & LIFE_P_DEFERRED) && p->preg_left == 0) {
            return FALSE;
        }
        if (p->entity != ENTITY_ID_NONE) {       /* nunca dois donos para o mesmo corpo */
            for (j = i + 1u; j < LIFE_MAX_PEOPLE; j++) {
                if (l->people[j].id != PERSON_ID_NONE && l->people[j].entity == p->entity) {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

/* ===========================================================================
 * Serialização (campo a campo, little-endian, com limites)
 * ======================================================================== */

OTS3_STATIC_ASSERT(life_serial_max_fits,
    12u + LIFE_MAX_PEOPLE + LIFE_MAX_PEOPLE * 46u <= LIFE_SERIAL_MAX);

static void WritePerson(ByteWriter* w, u32 slot, const Person* p)
{
    BW_U8(w, slot);
    BW_S32(w, p->birth_day);
    BW_S32(w, p->death_day);
    BW_U32(w, p->entity);
    BW_U32(w, p->genes);
    BW_U16(w, p->id);
    BW_U16(w, p->parent[0]);
    BW_U16(w, p->parent[1]);
    BW_U16(w, p->partner);
    BW_U16(w, p->preg_other);
    BW_U16(w, p->traits);
    BW_U8(w, p->household);
    BW_U8(w, p->flags);
    BW_U8(w, p->gender);
    BW_U8(w, p->cause);
    BW_U8(w, p->preg_left);
    BW_Bytes(w, p->name, OTS3_NAME_LEN);
}

static void ReadPerson(ByteReader* r, Person* p)
{
    p->birth_day = BR_S32(r);
    p->death_day = BR_S32(r);
    p->entity = BR_U32(r);
    p->genes = BR_U32(r);
    p->id = (PersonId)BR_U16(r);
    p->parent[0] = (PersonId)BR_U16(r);
    p->parent[1] = (PersonId)BR_U16(r);
    p->partner = (PersonId)BR_U16(r);
    p->preg_other = (PersonId)BR_U16(r);
    p->traits = (u16)BR_U16(r);
    p->household = (u8)BR_U8(r);
    p->flags = (u8)BR_U8(r);
    p->gender = (u8)BR_U8(r);
    p->cause = (u8)BR_U8(r);
    p->preg_left = (u8)BR_U8(r);
    BR_Bytes(r, p->name, OTS3_NAME_LEN);
}

BOOL Lifecycle_Serialize(const Lifecycle* l, void* buf, u32 cap, u32* out_len)
{
    ByteWriter w;
    u32 i, count = 0;

    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        if (l->people[i].id != PERSON_ID_NONE) {
            count++;
        }
    }
    BW_Init(&w, buf, cap);
    BW_U8(&w, LIFE_SERIAL_VERSION);
    BW_U8(&w, l->lifespan);
    BW_U8(&w, l->settings);
    BW_U8(&w, count);
    BW_U32(&w, l->last_day);
    BW_U32(&w, l->rng);
    BW_Bytes(&w, l->gen, LIFE_MAX_PEOPLE);
    for (i = 0; i < LIFE_MAX_PEOPLE; i++) {
        if (l->people[i].id != PERSON_ID_NONE) {
            WritePerson(&w, i, &l->people[i]);
        }
    }
    if (!BW_Ok(&w)) {
        return FALSE;
    }
    if (out_len) {
        *out_len = w.pos;
    }
    return TRUE;
}

/* staging estático: evita 4,7 KB de pilha (a DTCM do NDS tem 16 KB). Não reentrante. */
static Lifecycle sStaging;

LifeResult Lifecycle_Deserialize(Lifecycle* out, const void* buf, u32 len)
{
    ByteReader r;
    u32 count, i, prev = 0, have_prev = 0;
    Lifecycle* s = &sStaging;

    if (!out || !buf) {
        return LIFE_ERR_ARG;
    }
    BR_Init(&r, buf, len);
    memset(s, 0, sizeof(*s));
    if (BR_U8(&r) != LIFE_SERIAL_VERSION) {
        return LIFE_ERR_CORRUPT;
    }
    s->lifespan = (u8)BR_U8(&r);
    s->settings = (u8)BR_U8(&r);
    count = BR_U8(&r);
    s->last_day = BR_U32(&r);
    s->rng = BR_U32(&r);
    BR_Bytes(&r, s->gen, LIFE_MAX_PEOPLE);
    if (!BR_Ok(&r) || count > LIFE_MAX_PEOPLE) {
        return LIFE_ERR_CORRUPT;
    }
    for (i = 0; i < count; i++) {
        u32 slot = BR_U8(&r);
        if (!BR_Ok(&r) || slot >= LIFE_MAX_PEOPLE || (have_prev && slot <= prev)) {
            return LIFE_ERR_CORRUPT;             /* slots estritamente crescentes: sem duplicatas */
        }
        ReadPerson(&r, &s->people[slot]);
        if (!BR_Ok(&r) || s->people[slot].id == PERSON_ID_NONE) {
            return LIFE_ERR_CORRUPT;
        }
        prev = slot;
        have_prev = 1;
    }
    if (!BR_Ok(&r) || BR_Remaining(&r) != 0 || !Lifecycle_Validate(s)) {
        return LIFE_ERR_CORRUPT;
    }
    memcpy(out, s, sizeof(*out));
    return LIFE_OK;
}
