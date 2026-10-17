#ifndef OTS3_ENTITY_H
#define OTS3_ENTITY_H

#include "config.h"
#include "nav.h"

OTS3_EXTERN_C_BEGIN

/*
 * entity.h - Sims e pets (cães e gatos) no mesmo pool fixo.
 *
 * Duas structs separadas, de propósito:
 *   Entity         estado PERSISTENTE (vai para o save). Sem ponteiros, tamanho fixo.
 *   EntityRuntime  estado TRANSITÓRIO (caminho, ação atual, temporizadores). Nunca é
 *                  salvo: depois de um load as entidades simplesmente "decidem de novo".
 *
 * Referências entre entidades são EntityId = (geração << 16) | índice. Um id de entidade
 * já removida nunca volta a ser válido (a geração muda), então um "ponteiro velho" no
 * save ou numa tarefa pendente vira NULL em vez de acessar a memória de outra entidade.
 *
 * Este header descreve o motor NOVO. O espelho hipotético do layout original do jogo
 * (decompilação) fica em include/orig_entity.h; não inclua os dois no mesmo arquivo.
 */

typedef struct Vec2Fx { fx32 x, y; } Vec2Fx;           /* 20.12; 1.0 = 1 tile */

#define FX_TILE(f)           ((s32)((f) >> FX32_SHIFT))
#define FX_FROM_TILE(t)      ((fx32)(((s32)(t)) << FX32_SHIFT))
#define FX_TILE_CENTER(t)    ((fx32)((((s32)(t)) << FX32_SHIFT) + (FX32_ONE / 2)))

/* ------------------------------------------------------------------ ids -- */

typedef u32 EntityId;
#define ENTITY_ID_NONE       0u
#define ENTITY_ID_INDEX(id)  ((u32)(id) & 0xFFFFu)     /* 1..OTS3_MAX_ENTITIES */
#define ENTITY_ID_GEN(id)    ((u32)(id) >> 16)         /* 1..65535 (nunca 0) */
#define ENTITY_ID_MAKE(idx, gen) ((EntityId)(((u32)(gen) << 16) | (u32)(idx)))

/* ---------------------------------------------------------------- enums -- */

enum EntityKind {
    ENTITY_KIND_NONE = 0,
    ENTITY_KIND_SIM  = 1,
    ENTITY_KIND_PET  = 2,
    ENTITY_KIND_COUNT
};

enum PetSpecies {
    PET_SPECIES_NONE = 0,       /* Sims */
    PET_SPECIES_DOG  = 1,
    PET_SPECIES_CAT  = 2,
    PET_SPECIES_COUNT
};

/* Necessidades. Sims usam: fome, energia, bexiga, higiene, diversão.
 * Pets usam: fome, energia, afeto. As que não se aplicam ficam travadas em NEED_MAX. */
enum NeedId {
    NEED_HUNGER = 0,
    NEED_ENERGY,
    NEED_AFFECTION,
    NEED_BLADDER,
    NEED_HYGIENE,
    NEED_FUN,
    NEED_COUNT
};
#define NEED_MAX             1000
#define NEED_REM_DEN         60u       /* resto do decaimento: taxa/h dividida em 60 minutos */

enum MoveState {
    MOVE_IDLE = 0,              /* parado */
    MOVE_WAIT_PATH,             /* esperando o pathfinding */
    MOVE_WALKING,
    MOVE_BLOCKED                /* caminho invalidado (grade mudou) */
};

enum ActionKind {
    ACT_NONE = 0,
    ACT_WANDER,                 /* passeio aleatório */
    ACT_GOTO_OBJECT,            /* indo até um objeto que satisfaz a necessidade */
    ACT_USE_OBJECT,             /* usando o objeto (come, dorme...) */
    ACT_SEEK_OWNER,             /* pet indo até o dono */
    ACT_RECEIVE_AFFECTION,      /* pet sendo acarinhado pelo dono */
    ACT_GOTO_BOWL,              /* Sim indo reabastecer a tigela */
    ACT_COUNT
};

#define ENTITY_FLAG_ACTIVE   0x01u
#define ENTITY_FLAGS_MASK    0x01u

/* ------------------------------------------------------ estado persistente -- */

typedef struct Entity {
    EntityId id;                       /* 0 = slot livre */
    EntityId owner;                    /* pets: dono (Sim); Sims: 0 */
    Vec2Fx   pos;
    u16      yaw;                      /* 0x10000 = 360 graus */
    u8       kind;                     /* EntityKind */
    u8       species;                  /* PetSpecies (pets) */
    u8       household;                /* índice da família */
    u8       flags;
    s16      need[NEED_COUNT];         /* 0..NEED_MAX */
    u8       need_rem[NEED_COUNT];     /* acumulador do decaimento, 0..59 */
    u8       age_stage;
    char     name[OTS3_NAME_LEN];
} Entity;

OTS3_STATIC_ASSERT(entity_size_stable, sizeof(Entity) == 56);

typedef struct EntityTable {
    Entity e[OTS3_MAX_ENTITIES];
    u16    gen[OTS3_MAX_ENTITIES];     /* geração atual de cada slot (nunca 0) */
} EntityTable;

/* ----------------------------------------------------- estado transitório -- */

#define OBJ_NONE_INDEX       0xFFu

typedef struct EntityRuntime {
    NavPath path;
    u16  goal;                         /* tile de destino (NAV_TILE) */
    u16  backoff;                      /* ticks sem decidir (após falha/uso) */
    u16  nav_wait;                     /* ticks esperando o pathfinding */
    u16  wander_pause;                 /* ticks parado depois de passear */
    u16  use_left;                     /* minutos de jogo de uso restantes */
    u8   move_state;                   /* MoveState */
    u8   action;                       /* ActionKind */
    u8   need;                         /* NeedId que motivou a ação */
    u8   path_pos;
    u8   obj;                          /* objeto reservado (OBJ_NONE_INDEX = nenhum) */
    u8   fail_count;                   /* falhas seguidas (aciona o "failsafe") */
    u8   retry;                        /* repetições do SEEK_OWNER */
    u8   submit_pending;               /* pedido ao nav ainda não aceito (fila cheia) */
} EntityRuntime;

/* ------------------------------------------------------------ tabela -- */

void     EntityTable_Init(EntityTable* t);
EntityId EntityTable_Spawn(EntityTable* t, u8 kind, u8 species, u8 household,
                           EntityId owner, const char* name, Vec2Fx pos);
BOOL     EntityTable_Despawn(EntityTable* t, EntityId id);
Entity*  EntityTable_Get(EntityTable* t, EntityId id);                 /* NULL se inválido/velho */
const Entity* EntityTable_GetConst(const EntityTable* t, EntityId id);
u32      EntityTable_Count(const EntityTable* t);
/* Estrutura da tabela (ids coerentes com os slots, gerações != 0, enums válidos,
 * necessidades dentro da faixa, nomes terminados em NUL). Referências entre
 * entidades e posições são conferidas em Simulation_ValidatePersist. */
BOOL     EntityTable_Validate(const EntityTable* t);

void EntityRuntime_Reset(EntityRuntime* r);

/* ------------------------------------------------------- necessidades -- */

void Entity_SetName(Entity* e, const char* name);
void Entity_InitNeeds(Entity* e);
BOOL Entity_NeedApplies(const Entity* e, u8 need);
/* Um minuto de jogo de decaimento (aritmética inteira exata, sem deriva). */
OTS3_HOT void Entity_DecayNeedsMinute(Entity* e);
void Entity_AddNeed(Entity* e, u8 need, s32 delta);                    /* satura em 0..NEED_MAX */
/* A necessidade aplicável mais baixa que esteja abaixo de 'threshold' (empate: menor id). */
BOOL Entity_MostUrgentNeed(const Entity* e, s16 threshold, u8* out_need);

const char* PetSpecies_Name(u8 species);

OTS3_EXTERN_C_END

#endif /* OTS3_ENTITY_H */
