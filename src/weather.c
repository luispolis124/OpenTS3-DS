#include "weather.h"
#include "entity.h"

/* Chance (permille, soma 1000) de cada tempo ao sortear, por estação. */
static const u16 kWeight[SEASON_COUNT][WEATHER_COUNT] = {
    /*          clear cloudy rain storm snow heat */
    /* spring */ { 400, 250, 250,  70,   0,  30 },
    /* summer */ { 500, 150, 100, 100,   0, 150 },
    /* autumn */ { 300, 300, 300,  70,  30,   0 },
    /* winter */ { 250, 300,  50,   0, 400,   0 }
};
static const s16 kSeasonBase10[SEASON_COUNT] = { 160, 260, 140, 20 };
static const s16 kWeatherOffset10[WEATHER_COUNT] = { 0, -10, -30, -40, -60, 80 };

static u32 Next(WeatherState* w)
{
    u32 x = w->rng;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    w->rng = x;
    return x;
}

static u32 Range(WeatherState* w, u32 n)
{
    return (u32)(((u64)Next(w) * n) >> 32);
}

void Calendar_FromDay(u32 day, Calendar* out)
{
    u32 in_year = day % WEATHER_DAYS_PER_YEAR;

    out->year = day / WEATHER_DAYS_PER_YEAR;
    out->season = (u8)(in_year / WEATHER_DAYS_PER_SEASON);
    out->day_of_season = (u8)(in_year % WEATHER_DAYS_PER_SEASON);
    out->weekday = (u8)(day % 7u);
    out->pad = 0;
}

static void Pick(WeatherState* w, u8 season)
{
    u32 roll, acc = 0, k;

    if (season >= SEASON_COUNT) {
        season = SEASON_SPRING;
    }
    roll = Range(w, 1000);
    w->kind = WEATHER_CLEAR;
    for (k = 0; k < WEATHER_COUNT; k++) {
        acc += kWeight[season][k];
        if (roll < acc) {
            w->kind = (u8)k;
            break;
        }
    }
    w->intensity = (w->kind == WEATHER_CLEAR) ? 0u : (u8)(1u + Range(w, 3));
    w->minutes_left = (u16)(WEATHER_MIN_MINUTES + 30u * Range(w, 9));
}

void Weather_Init(WeatherState* w, u32 seed, u8 season)
{
    w->rng = seed ? seed : 0x9E3779B9u;
    Pick(w, season);
}

void Weather_OnMinute(WeatherState* w, u8 season)
{
    if (season >= SEASON_COUNT) {
        season = SEASON_SPRING;
    }
    if (w->kind >= WEATHER_COUNT || kWeight[season][w->kind] == 0) {
        w->minutes_left = 0;                      /* tempo impossível nesta estação: troca já */
    } else if (w->minutes_left > 0) {
        w->minutes_left--;
    }
    if (w->minutes_left == 0) {
        Pick(w, season);
    }
}

s16 Weather_Temperature10(const WeatherState* w, u8 season, u32 minute_of_day)
{
    u32 m = minute_of_day % OTS3_MINUTES_PER_DAY;
    u32 d = m > 900u ? m - 900u : 900u - m;       /* distância do pico das 15:00 */
    s32 curve, t;

    if (season >= SEASON_COUNT) {
        season = SEASON_SPRING;
    }
    if (d > 720u) {
        d = OTS3_MINUTES_PER_DAY - d;
    }
    curve = 50 - (s32)((100u * d) / 720u);        /* +5,0 C às 15:00, -5,0 C às 03:00 */
    t = kSeasonBase10[season] + curve + (w->kind < WEATHER_COUNT ? kWeatherOffset10[w->kind] : 0);
    return (s16)t;
}

u32 Weather_ExtraNeedDecay(const WeatherState* w, u8 need, u32 minute_of_day)
{
    if (w->kind == WEATHER_HEAT && need == NEED_ENERGY) {
        return (minute_of_day % 10u) == 0 ? 1u : 0u;      /* +6 pontos/h */
    }
    if (w->kind == WEATHER_SNOW && need == NEED_HUNGER) {
        return (minute_of_day % 12u) == 0 ? 1u : 0u;      /* +5 pontos/h */
    }
    return 0;
}

BOOL Weather_IsStormy(const WeatherState* w)
{
    return w->kind == WEATHER_STORM;
}

BOOL Weather_Validate(const WeatherState* w)
{
    return w->rng != 0 && w->kind < WEATHER_COUNT && w->intensity <= 3u &&
           w->minutes_left <= WEATHER_MAX_MINUTES &&
           (w->kind != WEATHER_CLEAR || w->intensity == 0);
}
