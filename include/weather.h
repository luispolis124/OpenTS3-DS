#ifndef OTS3_WEATHER_H
#define OTS3_WEATHER_H

#include "config.h"
#include "entity.h"      /* NeedId */

OTS3_EXTERN_C_BEGIN

/*
 * weather.h - calendário, estações e clima.
 *
 *  - O calendário é DERIVADO do dia de jogo (sem estado próprio que possa dessincronizar).
 *  - O clima é uma máquina de estados com RNG próprio (xorshift32); troca de tempo em
 *    múltiplos de 30 minutos; a probabilidade de cada tempo depende da estação.
 *  - Tudo em inteiros: a mesma semente produz o mesmo clima no NDS e na PC.
 * Os números (dias por estação, pesos, temperaturas) são parâmetros de projeto.
 */

#define WEATHER_DAYS_PER_SEASON  7u
#define WEATHER_DAYS_PER_YEAR    (4u * WEATHER_DAYS_PER_SEASON)
#define WEATHER_MIN_MINUTES      120u
#define WEATHER_MAX_MINUTES      360u

enum Season { SEASON_SPRING = 0, SEASON_SUMMER, SEASON_AUTUMN, SEASON_WINTER, SEASON_COUNT };

enum WeatherKind {
    WEATHER_CLEAR = 0,
    WEATHER_CLOUDY,
    WEATHER_RAIN,
    WEATHER_STORM,
    WEATHER_SNOW,
    WEATHER_HEAT,
    WEATHER_COUNT
};

typedef struct Calendar {
    u32 year;
    u8  season;
    u8  day_of_season;       /* 0..WEATHER_DAYS_PER_SEASON-1 */
    u8  weekday;             /* 0..6 */
    u8  pad;
} Calendar;

typedef struct WeatherState {
    u32 rng;                 /* nunca 0 */
    u16 minutes_left;        /* até a próxima troca */
    u8  kind;                /* WeatherKind */
    u8  intensity;           /* 0 (limpo) .. 3 */
} WeatherState;

void Calendar_FromDay(u32 day, Calendar* out);
void Weather_Init(WeatherState* w, u32 seed, u8 season);
/* Um minuto de jogo. Troca o tempo quando acaba a duração ou quando o tempo atual
 * é impossível na estação (ex.: neve no verão). */
void Weather_OnMinute(WeatherState* w, u8 season);
/* Temperatura em décimos de grau Celsius (ex.: 215 = 21,5 C). */
s16  Weather_Temperature10(const WeatherState* w, u8 season, u32 minute_of_day);
/* 1 se, neste minuto, a necessidade 'need' sofre 1 ponto extra de decaimento. */
u32  Weather_ExtraNeedDecay(const WeatherState* w, u8 need, u32 minute_of_day);
BOOL Weather_IsStormy(const WeatherState* w);
BOOL Weather_Validate(const WeatherState* w);

OTS3_EXTERN_C_END

#endif /* OTS3_WEATHER_H */
