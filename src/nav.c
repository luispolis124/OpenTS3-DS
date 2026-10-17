#include "nav.h"

#include <string.h>

#define NAV_REQ_FREE    0u
#define NAV_REQ_QUEUED  1u
#define NAV_REQ_ACTIVE  2u
#define NAV_REQ_DONE    3u
#define NAV_NO_ACTIVE   0xFFu
#define NAV_G_UNSEEN    0xFFFFu
#define NAV_G_LIMIT     60000u

/* N, NE, E, SE, S, SW, W, NW (índices ímpares = diagonais) */
static const s8 kDx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
static const s8 kDy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

BOOL Nav_Passable(const NavGrid* g, u8 profile, s32 x, s32 y)
{
    u8 f;

    if (x < 0 || y < 0 || x >= (s32)g->w || y >= (s32)g->h) {
        return FALSE;
    }
    f = g->flags[(u32)y * g->w + (u32)x];
    if (f & NAV_SOLID) {
        return FALSE;
    }
    if (profile == NAV_PROFILE_PET) {
        return (f & NAV_NO_PET) ? FALSE : TRUE;
    }
    return (f & NAV_NO_SIM) ? FALSE : TRUE;
}

BOOL NavGrid_Validate(const NavGrid* g)
{
    u32 i, n;

    if (g->w == 0 || g->h == 0 || g->w > NAV_MAX_W || g->h > NAV_MAX_H) {
        return FALSE;
    }
    n = (u32)g->w * g->h;
    for (i = 0; i < n; i++) {
        if (g->flags[i] & ~NAV_FLAGS_MASK) {
            return FALSE;
        }
    }
    return TRUE;
}

void Nav_Init(NavSystem* nav)
{
    memset(nav, 0, sizeof(*nav));
    nav->active = NAV_NO_ACTIVE;
}

/* ------------------------------------------------------------------ heap -- */

static BOOL HeapPush(NavSystem* n, u32 v)
{
    u32 i;

    if (n->heap_n >= NAV_HEAP_CAP) {
        return FALSE;
    }
    i = n->heap_n++;
    while (i > 0) {
        u32 p = (i - 1u) >> 1;
        if (n->heap[p] <= v) {
            break;
        }
        n->heap[i] = n->heap[p];
        i = p;
    }
    n->heap[i] = v;
    return TRUE;
}

static u32 HeapPop(NavSystem* n)
{
    u32 top = n->heap[0];
    u32 v = n->heap[--n->heap_n];
    u32 i = 0;

    for (;;) {
        u32 c = 2u * i + 1u;
        if (c >= n->heap_n) {
            break;
        }
        if (c + 1u < n->heap_n && n->heap[c + 1u] < n->heap[c]) {
            c++;
        }
        if (v <= n->heap[c]) {
            break;
        }
        n->heap[i] = n->heap[c];
        i = c;
    }
    if (n->heap_n > 0) {
        n->heap[i] = v;
    }
    return top;
}

/* heurística octile: 10 por passo reto, 14 por diagonal (= 10 + 4) */
static u32 Heur(u32 ax, u32 ay, u32 bx, u32 by)
{
    u32 dx = ax > bx ? ax - bx : bx - ax;
    u32 dy = ay > by ? ay - by : by - ay;
    return dx > dy ? 10u * dx + 4u * dy : 10u * dy + 4u * dx;
}

/* ------------------------------------------------------------- requests -- */

BOOL Nav_Submit(NavSystem* nav, const NavGrid* g, u32 who, u8 profile,
                u32 sx, u32 sy, u32 gx, u32 gy)
{
    u32 i, slot = NAV_QUEUE;
    NavRequest* r;

    if (who == 0 || sx >= g->w || sy >= g->h || gx >= g->w || gy >= g->h) {
        return FALSE;
    }
    for (i = 0; i < NAV_QUEUE; i++) {          /* um pedido por dono */
        if (nav->req[i].state != NAV_REQ_FREE && nav->req[i].who == who) {
            slot = i;
            break;
        }
    }
    if (slot == NAV_QUEUE) {
        for (i = 0; i < NAV_QUEUE; i++) {
            if (nav->req[i].state == NAV_REQ_FREE) {
                slot = i;
                break;
            }
        }
    }
    if (slot == NAV_QUEUE) {
        return FALSE;                           /* fila cheia: o chamador tenta depois */
    }
    if (nav->active == slot) {
        nav->active = NAV_NO_ACTIVE;            /* abandona a busca em andamento */
    }
    r = &nav->req[slot];
    memset(r, 0, sizeof(*r));
    r->who = who;
    r->seq = nav->next_seq++;
    r->start = NAV_TILE(sx, sy);
    r->goal = NAV_TILE(gx, gy);
    r->profile = (profile == NAV_PROFILE_PET) ? NAV_PROFILE_PET : NAV_PROFILE_SIM;
    r->state = NAV_REQ_QUEUED;
    return TRUE;
}

void Nav_Cancel(NavSystem* nav, u32 who)
{
    u32 i;

    for (i = 0; i < NAV_QUEUE; i++) {
        if (nav->req[i].state != NAV_REQ_FREE && nav->req[i].who == who) {
            if (nav->active == i) {
                nav->active = NAV_NO_ACTIVE;
            }
            nav->req[i].state = NAV_REQ_FREE;
            nav->req[i].who = 0;
        }
    }
}

NavResult Nav_Poll(NavSystem* nav, u32 who, NavPath* out)
{
    u32 i;

    for (i = 0; i < NAV_QUEUE; i++) {
        NavRequest* r = &nav->req[i];
        if (r->state == NAV_REQ_FREE || r->who != who) {
            continue;
        }
        if (r->state != NAV_REQ_DONE) {
            return NAV_RESULT_PENDING;
        }
        if (r->result == NAV_RESULT_FOUND && out) {
            *out = r->path;
        }
        {
            NavResult res = (NavResult)r->result;
            r->state = NAV_REQ_FREE;
            r->who = 0;
            return res;
        }
    }
    return NAV_RESULT_NONE;
}

/* ---------------------------------------------------------------- search -- */

static void Finish(NavSystem* nav, u32 idx, NavResult res)
{
    nav->req[idx].state = NAV_REQ_DONE;
    nav->req[idx].result = (u8)res;
    nav->active = NAV_NO_ACTIVE;
}

/* Monta o caminho (sem o tile inicial) seguindo os 'parent' do objetivo ao início.
 * Guarda os PRIMEIROS NAV_MAX_PATH passos; se houver mais, marca 'partial'. */
static void BuildPath(NavSystem* nav, const NavGrid* g, NavRequest* r)
{
    s32 cx = (s32)NAV_TILE_X(r->goal), cy = (s32)NAV_TILE_Y(r->goal);
    s32 sx = (s32)NAV_TILE_X(r->start), sy = (s32)NAV_TILE_Y(r->start);
    u32 total = 0, idx, pass;

    for (pass = 0; pass < 2; pass++) {
        s32 x = cx, y = cy;
        u32 guard = 0;

        idx = total;
        while ((x != sx || y != sy) && guard++ <= NAV_MAX_TILES) {
            u8 d = nav->parent[(u32)y * g->w + (u32)x];
            if (pass == 1) {
                idx--;
                if (idx < NAV_MAX_PATH) {
                    r->path.tiles[idx] = NAV_TILE(x, y);
                }
            } else {
                total++;
            }
            x -= kDx[d & 7u];
            y -= kDy[d & 7u];
        }
    }
    r->path.len = (u8)(total > NAV_MAX_PATH ? NAV_MAX_PATH : total);
    r->path.partial = (u8)(total > NAV_MAX_PATH ? 1u : 0u);
}

static BOOL StartNext(NavSystem* nav, const NavGrid* g)
{
    u32 i, best = NAV_QUEUE;
    NavRequest* r;
    u32 sx, sy, gx, gy;

    for (i = 0; i < NAV_QUEUE; i++) {           /* FIFO por 'seq' (com sinal: tolera overflow) */
        if (nav->req[i].state != NAV_REQ_QUEUED) {
            continue;
        }
        if (best == NAV_QUEUE || (s32)(nav->req[i].seq - nav->req[best].seq) < 0) {
            best = i;
        }
    }
    if (best == NAV_QUEUE) {
        return FALSE;
    }
    r = &nav->req[best];
    r->state = NAV_REQ_ACTIVE;
    nav->active = (u8)best;
    sx = NAV_TILE_X(r->start); sy = NAV_TILE_Y(r->start);
    gx = NAV_TILE_X(r->goal);  gy = NAV_TILE_Y(r->goal);

    if (sx >= g->w || sy >= g->h || gx >= g->w || gy >= g->h ||
        !Nav_Passable(g, r->profile, (s32)gx, (s32)gy)) {
        Finish(nav, best, NAV_RESULT_FAILED);   /* objetivo bloqueado ou grade mudou */
        return TRUE;
    }
    if (sx == gx && sy == gy) {
        r->path.len = 0;
        r->path.partial = 0;
        Finish(nav, best, NAV_RESULT_FOUND);
        return TRUE;
    }
    memset(nav->g, 0xFF, sizeof(nav->g));
    memset(nav->closed, 0, sizeof(nav->closed));
    nav->heap_n = 0;
    nav->expansions = 0;
    nav->g[sy * g->w + sx] = 0;
    (void)HeapPush(nav, (Heur(sx, sy, gx, gy) << 16) | NAV_TILE(sx, sy));
    return TRUE;
}

OTS3_HOT void Nav_Step(NavSystem* nav, const NavGrid* g, u32 budget)
{
    while (budget > 0) {
        NavRequest* r;
        u32 gx, gy, d;

        if (nav->active == NAV_NO_ACTIVE) {
            if (!StartNext(nav, g)) {
                return;
            }
            continue;                           /* pode ter terminado na hora */
        }
        r = &nav->req[nav->active];
        gx = NAV_TILE_X(r->goal);
        gy = NAV_TILE_Y(r->goal);

        if (nav->heap_n == 0 || nav->expansions >= NAV_MAX_EXPANSIONS) {
            Finish(nav, nav->active, NAV_RESULT_FAILED);   /* sem caminho / limite */
            continue;
        }
        budget--;
        {
            u32 key = HeapPop(nav);
            u32 x = key & 0xFFu, y = (key >> 8) & 0xFFu;   /* chave carrega o tile empacotado */
            u32 node = y * g->w + x;                       /* sem divisão por variável */

            if (nav->closed[node >> 3] & (1u << (node & 7u))) {
                continue;                              /* entrada velha do heap */
            }
            nav->closed[node >> 3] |= (u8)(1u << (node & 7u));
            nav->expansions++;

            if (x == gx && y == gy) {
                BuildPath(nav, g, r);
                Finish(nav, nav->active, NAV_RESULT_FOUND);
                continue;
            }
            for (d = 0; d < 8u; d++) {
                s32 nx = (s32)x + kDx[d], ny = (s32)y + kDy[d];
                u32 ng, nidx, f;

                if (!Nav_Passable(g, r->profile, nx, ny)) {
                    continue;
                }
                if (d & 1u) {                          /* diagonal: sem cortar quinas */
                    if (!Nav_Passable(g, r->profile, (s32)x + kDx[d], (s32)y) ||
                        !Nav_Passable(g, r->profile, (s32)x, (s32)y + kDy[d])) {
                        continue;
                    }
                }
                nidx = (u32)ny * g->w + (u32)nx;
                if (nav->closed[nidx >> 3] & (1u << (nidx & 7u))) {
                    continue;
                }
                ng = (u32)nav->g[node] + ((d & 1u) ? 14u : 10u);
                if (ng >= NAV_G_LIMIT || ng >= nav->g[nidx]) {
                    continue;
                }
                nav->g[nidx] = (u16)ng;
                nav->parent[nidx] = (u8)d;
                f = ng + Heur((u32)nx, (u32)ny, gx, gy);
                if (f > 0xFFFFu || !HeapPush(nav, (f << 16) | NAV_TILE(nx, ny))) {
                    Finish(nav, nav->active, NAV_RESULT_FAILED);   /* heap cheio */
                    break;
                }
            }
        }
    }
}
