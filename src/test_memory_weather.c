#include "test_common.h"
#include "memory.h"
#include "weather.h"

static u64 gArenaMem[256];           /* 2 KB, alinhado a 8 */

int main(void)
{
    Arena a;
    VramBudget v;
    void *p, *q;
    u32 mark, i, season, d, kind_count[WEATHER_COUNT];
    WeatherState w;
    Calendar cal;
    u8 raw[64];

    /* ---------------- Arena ---------------- */
    CHECK(!Arena_Init(&a, NULL, 100));
    CHECK(!Arena_Init(&a, gArenaMem, 0));
    CHECK(!Arena_Init(&a, ((u8*)gArenaMem) + 4, 100));            /* desalinhado */
    CHECK(Arena_Init(&a, gArenaMem, sizeof(gArenaMem)));
    CHECK_EQ(Arena_Free(&a), 2048);
    p = Arena_Alloc(&a, 3, 1);
    q = Arena_Alloc(&a, 4, 4);
    CHECK(p != NULL && q != NULL && (((unsigned long)q) & 3u) == 0);
    CHECK_EQ((u8*)q - (u8*)p, 4);                                  /* padding de alinhamento */
    CHECK(Arena_Alloc(&a, 8, 3) == NULL);                          /* alinhamento inválido */
    CHECK(Arena_Alloc(&a, 8, 16) == NULL);
    CHECK(Arena_Alloc(&a, 8, 0) == NULL);
    CHECK(Arena_Alloc(&a, 0xFFFFFFFFu, 4) == NULL);                /* sem dar a volta no u32 */
    CHECK(Arena_Alloc(&a, 0xFFFFFFF0u, 8) == NULL);
    CHECK(Arena_Alloc(&a, 2048, 1) == NULL);                       /* não cabe */
    mark = Arena_BackMark(&a);
    p = Arena_AllocBack(&a, 100, 8);
    CHECK(p != NULL && (((unsigned long)p) & 7u) == 0);
    CHECK(Arena_AllocBack(&a, 0xFFFFFFFFu, 4) == NULL);
    CHECK(Arena_AllocBack(&a, 5000, 4) == NULL);
    q = Arena_Alloc(&a, 1945, 1);                                  /* 8 + 1945 > 1948: frente e fundo nunca se cruzam */
    CHECK(q == NULL);
    Arena_BackRelease(&a, mark);
    CHECK_EQ(Arena_Free(&a), 2048 - 8);
    Arena_BackRelease(&a, 5);                                      /* marca inválida: ignorada */
    CHECK_EQ(Arena_Free(&a), 2048 - 8);
    CHECK(Arena_Alloc(&a, 2048 - 8, 1) != NULL);                   /* agora cabe exatamente */
    CHECK_EQ(Arena_Free(&a), 0);
    CHECK(Arena_Alloc(&a, 1, 1) == NULL);
    CHECK(a.high_front == 2048);
    Arena_Reset(&a);
    CHECK_EQ(Arena_Free(&a), 2048);

    /* ---------------- VRAM ---------------- */
    Vram_Init(&v, TRUE);
    CHECK_EQ(v.total_limit, 656u * 1024u);
    CHECK(Vram_Reserve(&v, VRAM_POOL_PET, 90u * 1024u));
    CHECK(!Vram_Reserve(&v, VRAM_POOL_PET, 7u * 1024u));           /* estoura a partição (96 KB) */
    CHECK_EQ(v.used, 90u * 1024u);                                 /* recusa não altera nada */
    CHECK(Vram_Reserve(&v, VRAM_POOL_PET, 6u * 1024u));
    CHECK_EQ(Vram_Free(&v, VRAM_POOL_PET), 0);
    CHECK(Vram_Reserve(&v, VRAM_POOL_WORLD, 400u * 1024u));
    CHECK(Vram_Reserve(&v, VRAM_POOL_UI, 96u * 1024u));
    CHECK(Vram_Reserve(&v, VRAM_POOL_RESERVED, 64u * 1024u));
    CHECK_EQ(v.used, 656u * 1024u);                                 /* as quatro partições somam os 656 KB */
    CHECK(!Vram_Reserve(&v, VRAM_POOL_UI, 1));
    CHECK(!Vram_Reserve(&v, 9, 1));
    CHECK(!Vram_Release(&v, VRAM_POOL_PET, 96u * 1024u + 1u));      /* devolver demais é recusado */
    CHECK(Vram_Release(&v, VRAM_POOL_PET, 96u * 1024u));
    CHECK_EQ(v.pool_used[VRAM_POOL_PET], 0);
    CHECK(!Vram_Release(&v, VRAM_POOL_PET, 1));
    CHECK_EQ(v.high, 656u * 1024u);
    Vram_Init(&v, FALSE);                                           /* PC: só registra */
    CHECK(Vram_Reserve(&v, VRAM_POOL_PET, 500u * 1024u * 1024u));
    CHECK(!Vram_Reserve(&v, VRAM_POOL_PET, 0xFFFFFFFFu));           /* mas o contador não dá a volta */

    /* ---------------- calendário ---------------- */
    Calendar_FromDay(0, &cal);
    CHECK(cal.year == 0 && cal.season == SEASON_SPRING && cal.day_of_season == 0 && cal.weekday == 0);
    Calendar_FromDay(WEATHER_DAYS_PER_SEASON, &cal);
    CHECK(cal.season == SEASON_SUMMER && cal.day_of_season == 0);
    Calendar_FromDay(3u * WEATHER_DAYS_PER_SEASON + 2u, &cal);
    CHECK(cal.season == SEASON_WINTER && cal.day_of_season == 2);
    Calendar_FromDay(WEATHER_DAYS_PER_YEAR * 5u + 9u, &cal);
    CHECK(cal.year == 5 && cal.season == SEASON_SUMMER && cal.day_of_season == 2);
    Calendar_FromDay(0xFFFFFFFFu, &cal);                            /* extremos não quebram */
    CHECK(cal.season < SEASON_COUNT && cal.day_of_season < WEATHER_DAYS_PER_SEASON && cal.weekday < 7);

    /* ---------------- clima ---------------- */
    for (season = 0; season < SEASON_COUNT; season++) {
        memset(kind_count, 0, sizeof(kind_count));
        Weather_Init(&w, 1234u + season, (u8)season);
        CHECK(Weather_Validate(&w));
        for (i = 0; i < 400000u; i++) {                              /* ~277 dias de jogo */
            u32 before = w.minutes_left;
            Weather_OnMinute(&w, (u8)season);
            if (i % 1000u == 0) CHECK(Weather_Validate(&w));
            CHECK(w.minutes_left <= WEATHER_MAX_MINUTES);
            if (w.minutes_left > before) kind_count[w.kind]++;       /* contou uma troca */
        }
        d = 0;
        for (i = 0; i < WEATHER_COUNT; i++) d += kind_count[i];
        CHECK(d > 1000u);
        if (season == SEASON_SUMMER || season == SEASON_SPRING || season == SEASON_AUTUMN) CHECK_EQ(kind_count[WEATHER_SNOW] * (season == SEASON_AUTUMN ? 0u : 1u), 0);
        if (season == SEASON_WINTER) { CHECK_EQ(kind_count[WEATHER_STORM], 0); CHECK_EQ(kind_count[WEATHER_HEAT], 0); CHECK(kind_count[WEATHER_SNOW] > d / 4u); }
        if (season == SEASON_SUMMER) { CHECK(kind_count[WEATHER_HEAT] > d / 12u); CHECK_EQ(kind_count[WEATHER_SNOW], 0); }
    }
    /* determinismo: mesma semente, mesma história */
    {
        WeatherState x, y;
        Weather_Init(&x, 99, SEASON_SPRING); Weather_Init(&y, 99, SEASON_SPRING);
        for (i = 0; i < 50000u; i++) {
            Weather_OnMinute(&x, (u8)((i / 10000u) % 4u)); Weather_OnMinute(&y, (u8)((i / 10000u) % 4u));
            CHECK(x.rng == y.rng && x.kind == y.kind && x.minutes_left == y.minutes_left);
        }
    }
    /* tempo impossível na estação troca imediatamente */
    Weather_Init(&w, 5, SEASON_WINTER);
    w.kind = WEATHER_SNOW; w.intensity = 2; w.minutes_left = 300;
    Weather_OnMinute(&w, SEASON_SUMMER);
    CHECK(w.kind != WEATHER_SNOW && w.minutes_left >= WEATHER_MIN_MINUTES);
    /* temperatura: ciclo diário e efeito do tempo */
    Weather_Init(&w, 5, SEASON_SUMMER);
    w.kind = WEATHER_CLEAR; w.intensity = 0;
    CHECK_EQ(Weather_Temperature10(&w, SEASON_SUMMER, 900), 260 + 50);
    CHECK_EQ(Weather_Temperature10(&w, SEASON_SUMMER, 180), 260 - 50);
    CHECK(Weather_Temperature10(&w, SEASON_SUMMER, 900) > Weather_Temperature10(&w, SEASON_WINTER, 900));
    w.kind = WEATHER_HEAT; w.intensity = 2;
    CHECK_EQ(Weather_Temperature10(&w, SEASON_SUMMER, 900), 260 + 50 + 80);
    CHECK_EQ(Weather_Temperature10(&w, SEASON_SUMMER, 900 + OTS3_MINUTES_PER_DAY), Weather_Temperature10(&w, SEASON_SUMMER, 900));
    CHECK(Weather_Temperature10(&w, 77, 0) == Weather_Temperature10(&w, SEASON_SPRING, 0));
    /* decaimento extra: exato e só onde deve */
    w.kind = WEATHER_HEAT;
    for (i = 0, d = 0; i < OTS3_MINUTES_PER_DAY; i++) d += Weather_ExtraNeedDecay(&w, NEED_ENERGY, i);
    CHECK_EQ(d, 144);
    CHECK_EQ(Weather_ExtraNeedDecay(&w, NEED_HUNGER, 0), 0);
    w.kind = WEATHER_SNOW;
    for (i = 0, d = 0; i < OTS3_MINUTES_PER_DAY; i++) d += Weather_ExtraNeedDecay(&w, NEED_HUNGER, i);
    CHECK_EQ(d, 120);
    w.kind = WEATHER_RAIN; CHECK_EQ(Weather_ExtraNeedDecay(&w, NEED_ENERGY, 0), 0);
    CHECK(Weather_IsStormy(&w) == FALSE);
    w.kind = WEATHER_STORM; CHECK(Weather_IsStormy(&w));
    /* validação */
    memset(raw, 0, sizeof(raw));
    w.rng = 1; w.kind = 0; w.intensity = 0; w.minutes_left = 10; CHECK(Weather_Validate(&w));
    w.rng = 0; CHECK(!Weather_Validate(&w)); w.rng = 1;
    w.kind = WEATHER_COUNT; CHECK(!Weather_Validate(&w)); w.kind = 0;
    w.intensity = 4; CHECK(!Weather_Validate(&w)); w.intensity = 1; CHECK(!Weather_Validate(&w)); /* clear com intensidade */
    w.intensity = 0; w.minutes_left = 361; CHECK(!Weather_Validate(&w));

    return Test_Finish("memory+clima");
}
