#include "test_common.h"
#include "nav.h"

static NavSystem gNav;
static NavGrid gGrid;

/* ---- referência: Dijkstra O(T^2) com as MESMAS regras do A* ---- */
static const int RDX[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int RDY[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

static int RefCost(const NavGrid* g, int profile, int sx, int sy, int gx, int gy)
{
    static int dist[NAV_MAX_TILES];
    static char done[NAV_MAX_TILES];
    int n = g->w * g->h, i, it;
    for (i = 0; i < n; i++) { dist[i] = 1 << 30; done[i] = 0; }
    dist[sy * g->w + sx] = 0;
    for (it = 0; it < n; it++) {
        int best = -1;
        for (i = 0; i < n; i++) if (!done[i] && (best < 0 || dist[i] < dist[best])) best = i;
        if (best < 0 || dist[best] >= (1 << 30)) break;
        done[best] = 1;
        {
            int x = best % g->w, y = best / g->w, d;
            for (d = 0; d < 8; d++) {
                int nx = x + RDX[d], ny = y + RDY[d];
                if (!Nav_Passable(g, (u8)profile, nx, ny)) continue;
                if ((d & 1) && (!Nav_Passable(g, (u8)profile, nx, y) || !Nav_Passable(g, (u8)profile, x, ny))) continue;
                if (dist[best] + ((d & 1) ? 14 : 10) < dist[ny * g->w + nx])
                    dist[ny * g->w + nx] = dist[best] + ((d & 1) ? 14 : 10);
            }
        }
    }
    return dist[gy * g->w + gx] >= (1 << 30) ? -1 : dist[gy * g->w + gx];
}

/* custo do caminho entregue pelo A*; -1 se algum passo é inválido */
static int PathCost(const NavGrid* g, int profile, int sx, int sy, const NavPath* p)
{
    int cx = sx, cy = sy, cost = 0, i;
    for (i = 0; i < p->len; i++) {
        int nx = (int)NAV_TILE_X(p->tiles[i]), ny = (int)NAV_TILE_Y(p->tiles[i]);
        int dx = nx - cx, dy = ny - cy;
        if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || (dx == 0 && dy == 0)) return -1;
        if (!Nav_Passable(g, (u8)profile, nx, ny)) return -1;
        if (dx && dy && (!Nav_Passable(g, (u8)profile, nx, cy) || !Nav_Passable(g, (u8)profile, cx, ny))) return -1;
        cost += (dx && dy) ? 14 : 10;
        cx = nx; cy = ny;
    }
    return cost;
}

/* roda uma busca completa com o orçamento dado */
static NavResult Solve(const NavGrid* g, int profile, int sx, int sy, int gx, int gy, u32 budget, NavPath* out, u32* calls)
{
    NavResult r;
    u32 guard = 0;
    Nav_Init(&gNav);
    if (!Nav_Submit(&gNav, g, 7u, (u8)profile, (u32)sx, (u32)sy, (u32)gx, (u32)gy)) return NAV_RESULT_NONE;
    *calls = 0;
    do {
        Nav_Step(&gNav, g, budget);
        (*calls)++;
        r = Nav_Poll(&gNav, 7u, out);
    } while (r == NAV_RESULT_PENDING && guard++ < 100000u);
    return r;
}

static void RandomGrid(NavGrid* g, int w, int h, int density)
{
    int i;
    memset(g, 0, sizeof(*g));
    g->w = (u8)w; g->h = (u8)h;
    for (i = 0; i < w * h; i++) {
        unsigned r = Test_Rand() % 100u;
        if ((int)r < density) g->flags[i] = NAV_SOLID;
        else if (r > 96u) g->flags[i] = NAV_NO_SIM;
        else if (r > 93u) g->flags[i] = NAV_NO_PET;
    }
}

int main(void)
{
    int iter, found = 0, failed = 0;
    NavPath p, p2;
    u32 calls, calls2;

    /* 1) otimalidade contra Dijkstra em grades aleatórias, os dois perfis */
    for (iter = 0; iter < 300; iter++) {
        int w = 5 + (int)(Test_Rand() % 20u), h = 5 + (int)(Test_Rand() % 20u);
        int profile = iter & 1, sx, sy, gx, gy, ref;
        NavResult r;
        RandomGrid(&gGrid, w, h, 15 + (int)(Test_Rand() % 30u));
        sx = (int)(Test_Rand() % (unsigned)w); sy = (int)(Test_Rand() % (unsigned)h);
        gx = (int)(Test_Rand() % (unsigned)w); gy = (int)(Test_Rand() % (unsigned)h);
        gGrid.flags[sy * w + sx] = 0;                      /* início livre */
        ref = Nav_Passable(&gGrid, (u8)profile, gx, gy) ? RefCost(&gGrid, profile, sx, sy, gx, gy) : -1;
        r = Solve(&gGrid, profile, sx, sy, gx, gy, 50u, &p, &calls);
        if (ref < 0) { CHECK(r == NAV_RESULT_FAILED); failed++; }
        else {
            CHECK(r == NAV_RESULT_FOUND);
            if (r == NAV_RESULT_FOUND) {
                found++;
                CHECK_EQ(PathCost(&gGrid, profile, sx, sy, &p), ref);
                CHECK(p.partial == 0);
                if (p.len) CHECK_EQ(p.tiles[p.len - 1], NAV_TILE(gx, gy));
            }
        }
    }
    CHECK(found > 50 && failed > 5);                        /* o teste exercitou os dois casos */

    /* 2) fatiar o orçamento não muda o resultado */
    for (iter = 0; iter < 60; iter++) {
        NavResult a, b;
        RandomGrid(&gGrid, 30, 30, 25);
        gGrid.flags[0] = 0; gGrid.flags[29 * 30 + 29] = 0;
        a = Solve(&gGrid, NAV_PROFILE_SIM, 0, 0, 29, 29, 1u, &p, &calls);
        b = Solve(&gGrid, NAV_PROFILE_SIM, 0, 0, 29, 29, 100000u, &p2, &calls2);
        CHECK(a == b);
        if (a == NAV_RESULT_FOUND && b == NAV_RESULT_FOUND) {
            CHECK_EQ(p.len, p2.len);
            CHECK(memcmp(p.tiles, p2.tiles, (size_t)p.len * 2u) == 0);
            CHECK(calls >= calls2);
        }
    }

    /* 3) casos especiais */
    memset(&gGrid, 0, sizeof(gGrid)); gGrid.w = 10; gGrid.h = 10;
    CHECK(Solve(&gGrid, 0, 3, 3, 3, 3, 10u, &p, &calls) == NAV_RESULT_FOUND && p.len == 0);   /* início = fim */
    gGrid.flags[5 * 10 + 5] = NAV_SOLID;
    CHECK(Solve(&gGrid, 0, 0, 0, 5, 5, 10u, &p, &calls) == NAV_RESULT_FAILED);              /* objetivo sólido */
    {   /* parede total com porta de pet em y=4: só o pet atravessa */
        int y;
        memset(&gGrid, 0, sizeof(gGrid)); gGrid.w = 10; gGrid.h = 10;
        for (y = 0; y < 10; y++) gGrid.flags[y * 10 + 5] = (y == 4) ? NAV_NO_SIM : NAV_SOLID;
        CHECK(Solve(&gGrid, NAV_PROFILE_SIM, 1, 4, 8, 4, 100u, &p, &calls) == NAV_RESULT_FAILED);
        CHECK(Solve(&gGrid, NAV_PROFILE_PET, 1, 4, 8, 4, 100u, &p, &calls) == NAV_RESULT_FOUND);
        CHECK_EQ(PathCost(&gGrid, NAV_PROFILE_PET, 1, 4, &p), 70);
    }
    {   /* diagonal não corta quina: dois sólidos em "X" bloqueiam a passagem diagonal */
        memset(&gGrid, 0, sizeof(gGrid)); gGrid.w = 4; gGrid.h = 4;
        gGrid.flags[0 * 4 + 1] = NAV_SOLID; gGrid.flags[1 * 4 + 0] = NAV_SOLID;
        CHECK(Solve(&gGrid, 0, 0, 0, 3, 3, 100u, &p, &calls) == NAV_RESULT_FAILED);
    }
    {   /* caminho longo é truncado e marcado partial */
        int x, y;
        memset(&gGrid, 0, sizeof(gGrid)); gGrid.w = 48; gGrid.h = 48;
        for (y = 1; y < 47; y += 2) for (x = 0; x < 48; x++) gGrid.flags[y * 48 + x] = NAV_SOLID;
        for (y = 1; y < 47; y += 4) gGrid.flags[y * 48 + 47] = 0;        /* serpentina */
        for (y = 3; y < 47; y += 4) gGrid.flags[y * 48 + 0] = 0;
        CHECK(Solve(&gGrid, 0, 0, 0, 47, 47, 100u, &p, &calls) == NAV_RESULT_FOUND);
        CHECK(p.partial == 1 && p.len == NAV_MAX_PATH);
    }

    /* 4) fila: capacidade, substituição por dono, cancelamento, FIFO */
    {
        u32 i;
        memset(&gGrid, 0, sizeof(gGrid)); gGrid.w = 20; gGrid.h = 20;
        Nav_Init(&gNav);
        for (i = 1; i <= NAV_QUEUE; i++) CHECK(Nav_Submit(&gNav, &gGrid, i, 0, 0, 0, 19, 19));
        CHECK(!Nav_Submit(&gNav, &gGrid, 99u, 0, 0, 0, 19, 19));                  /* cheia */
        CHECK(Nav_Submit(&gNav, &gGrid, 3u, 0, 1, 1, 5, 5));                      /* mesmo dono: substitui */
        CHECK(Nav_Poll(&gNav, 3u, &p) == NAV_RESULT_PENDING);
        Nav_Cancel(&gNav, 3u);
        CHECK(Nav_Poll(&gNav, 3u, &p) == NAV_RESULT_NONE);
        CHECK(Nav_Submit(&gNav, &gGrid, 99u, 0, 0, 0, 19, 19));                   /* vaga liberada */
        CHECK(!Nav_Submit(&gNav, &gGrid, 50u, 0, 0, 0, 20, 0));                   /* fora da grade */
        CHECK(!Nav_Submit(&gNav, &gGrid, 0u, 0, 0, 0, 1, 1));                     /* dono inválido */
        Nav_Step(&gNav, &gGrid, 1u);                                              /* só o 1º (FIFO) começou */
        CHECK(Nav_Poll(&gNav, 1u, &p) == NAV_RESULT_PENDING);
        Nav_Step(&gNav, &gGrid, 100000u);
        for (i = 1; i <= NAV_QUEUE; i++) {
            NavResult r = Nav_Poll(&gNav, i == 3u ? 99u : i, &p);
            CHECK(r == NAV_RESULT_FOUND || (i == 3u && r == NAV_RESULT_FOUND));
        }
        CHECK(Nav_Poll(&gNav, 1u, &p) == NAV_RESULT_NONE);                        /* entrega única */
    }

    /* 5) validação da grade */
    memset(&gGrid, 0, sizeof(gGrid)); gGrid.w = 10; gGrid.h = 10;
    CHECK(NavGrid_Validate(&gGrid));
    gGrid.flags[55] = 0x80; CHECK(!NavGrid_Validate(&gGrid)); gGrid.flags[55] = 0;
    gGrid.w = 0; CHECK(!NavGrid_Validate(&gGrid));
    gGrid.w = 49; CHECK(!NavGrid_Validate(&gGrid));

    return Test_Finish("nav");
}
