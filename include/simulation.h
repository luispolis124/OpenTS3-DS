#ifndef OTS3_SIMULATION_H
#define OTS3_SIMULATION_H

#include "config.h"
#include "entity.h"
#include "nav.h"
#include "lifecycle.h"
#include "weather.h"

OTS3_EXTERN_C_BEGIN

/*
 * simulation.h - o mundo e o laço de simulação (Sims, pets, clima, ciclo de vida).
 *
 * WorldState = WorldPersist (vai para o save; sem ponteiros; tamanho fixo)
 *            + WorldRuntime (caminhos, ações, reservas, filas; nunca é salvo).
 *
 * PASSO FIXO
 *   A simulação avança em ticks de OTS3_STEP_US (30 Hz). O estado depois de N ticks é
 *   uma função só de N e das entradas, NÃO de como os quadros reais foram divididos:
 *   por isso o NDS (60 fps, dt irregular) e a PC (qualquer taxa) produzem a mesma história.
 *   Simulation_Update(world, float) é a porta de entrada pedida; ela converte o dt para
 *   microssegundos inteiros e chama Simulation_UpdateUs, que é o caminho preferido no NDS
 *   (o ARM9 não tem FPU: ler os timers de hardware e chamar UpdateUs evita ponto flutuante).
 *
 * SEM TRAVAMENTOS
 *   Toda espera tem limite: pathfinding (timeout), uso de objeto (minutos), reservas
 *   (expiram e são ignoradas se o dono sumiu), movimento (detector de "andando parado").
 *   Falhas seguidas acionam o "failsafe": a necessidade é parcialmente restaurada e a
 *   entidade, se estiver em tile bloqueado, é recolocada num tile livre. Um Sim ou pet
 *   nunca fica preso para sempre numa decisão impossível.
 */

typedef struct SimClock {
    u32 tick;                      /* ticks totais */
    u32 day;                       /* dia de jogo (alimenta calendário e envelhecimento) */
    u16 minute;                    /* minuto do dia, 0..1439 */
    u8  speed;                     /* 0 = pausa, 1..OTS3_MAX_SPEED */
    u8  minute_ticks;              /* ticks dentro do minuto atual */
} SimClock;

typedef struct Household {
    char name[OTS3_HOUSEHOLD_NAME_LEN];
    s32  funds;
    u8   active;
    u8   pad[3];
} Household;

enum ObjectKind {
    OBJ_NONE = 0,
    OBJ_FRIDGE,                    /* Sim: fome */
    OBJ_BED,                       /* Sim: energia */
    OBJ_TOILET,                    /* Sim: bexiga */
    OBJ_SHOWER,                    /* Sim: higiene */
    OBJ_TV,                        /* Sim: diversão */
    OBJ_PET_BOWL,                  /* pet: fome; 'state' = comida (0..100); Sims reabastecem */
    OBJ_PET_BED,                   /* pet: energia */
    OBJ_KIND_COUNT
};
#define OBJECT_STATE_MAX 100u

typedef struct WorldObject {
    u8 kind;                       /* ObjectKind */
    u8 state;                      /* 0..OBJECT_STATE_MAX (comida da tigela; 0 nos demais) */
    u8 tx, ty;
} WorldObject;

/* ------------------------------------------------------- estado persistente -- */

typedef struct WorldPersist {
    SimClock     clock;
    u32          rng;              /* xorshift32, nunca 0 */
    Household    households[OTS3_MAX_HOUSEHOLDS];
    EntityTable  entities;
    NavGrid      grid;
    u16          object_count;
    u8           spawn_x, spawn_y; /* onde nascem corpos novos */
    WorldObject  objects[OTS3_MAX_OBJECTS];
    WeatherState weather;
    Lifecycle    life;
} WorldPersist;

/* --------------------------------------------------- estado transitório -- */

typedef struct ObjReservation {
    EntityId owner;                /* 0 = livre */
    u32      until_tick;           /* expira (comparação com sinal: tolera overflow) */
} ObjReservation;

typedef struct WorldStats {
    u32 failsafes;                 /* vezes que o failsafe agiu */
    u32 nav_failures;              /* A* sem caminho */
    u32 nav_timeouts;              /* esperou o A* além do limite */
    u32 stuck_events;              /* "andando parado" detectado */
    u32 dropped_us;                /* tempo de quadro descartado (dt gigante / espiral) */
} WorldStats;

typedef struct WorldRuntime {
    u32            acc_us;         /* tempo real acumulado, menor que OTS3_STEP_US após cada update */
    EntityRuntime  ent[OTS3_MAX_ENTITIES];
    ObjReservation res[OTS3_MAX_OBJECTS];
    NavSystem      nav;
    LifeEventQueue events;
    LifeHooks      hooks;
    WorldStats     stats;
} WorldRuntime;

typedef struct WorldState {
    WorldPersist p;
    WorldRuntime rt;
} WorldState;

/* --------------------------------------------------------------- draw list -- */

enum ModelId { MODEL_SIM = 0, MODEL_PET_DOG, MODEL_PET_CAT, MODEL_COUNT };

typedef struct DrawItem {
    EntityId id;
    Vec2Fx   pos;
    u16      yaw;
    u8       model;                /* ModelId */
    u8       stage;                /* LifeStage (escala/variante do modelo) */
    u8       action;               /* ActionKind (animação) */
    u8       move;                 /* MoveState */
    u8       pad[2];
} DrawItem;

typedef struct DrawList {
    u16      count;
    u16      pad;
    DrawItem item[OTS3_MAX_ENTITIES];
} DrawList;

/* ------------------------------------------------------------------- API -- */

void Simulation_Init(WorldState* w, u32 seed);

BOOL     Simulation_AddHousehold(WorldState* w, const char* name, u8* out_index);
/* Corpo de Sim sem registro de pessoa (use Simulation_AddPerson para o ciclo de vida). */
EntityId Simulation_SpawnSim(WorldState* w, u8 household, const char* name, Vec2Fx pos);
EntityId Simulation_SpawnPet(WorldState* w, u8 species, u8 household, EntityId owner,
                             const char* name, Vec2Fx pos);
BOOL     Simulation_Despawn(WorldState* w, EntityId id);
/* Cria a pessoa (genealogia/idade) e, se pedido, o corpo. */
LifeResult Simulation_AddPerson(WorldState* w, const LifePersonInit* init, PersonId* out);
BOOL     Simulation_AddObject(WorldState* w, u8 kind, u32 tx, u32 ty, u8* out_index);

/* Lota de demonstração/teste: 24x16, duas salas, porta, porta de pet, objetos. */
void Simulation_BuildDemoLot(WorldState* w);

void Simulation_SetSpeed(WorldState* w, u8 speed);

/* Um tick fixo exato. */
OTS3_HOT void Simulation_Step(WorldState* w);
/* Avança pelo tempo real decorrido (microssegundos). Devolve os ticks executados. */
u32  Simulation_UpdateUs(WorldState* w, u32 delta_us);
/* Porta de entrada em segundos (float). NaN, infinito e valores <= 0 são ignorados. */
void Simulation_Update(WorldState* world, float deltaTime);

/* Invariantes do estado persistente, inclusive referências entre entidades. */
BOOL Simulation_ValidatePersist(const WorldPersist* p);
/* Valida, copia e zera o runtime (todos recomeçam a decidir). Em erro, nada muda. */
BOOL Simulation_ApplyPersist(WorldState* w, const WorldPersist* p);

void Simulation_BuildDrawList(const WorldState* w, DrawList* out);

OTS3_EXTERN_C_END

#endif /* OTS3_SIMULATION_H */
