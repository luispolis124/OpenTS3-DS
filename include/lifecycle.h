#ifndef OTS3_LIFECYCLE_H
#define OTS3_LIFECYCLE_H

#include "config.h"
#include "entity.h"

OTS3_EXTERN_C_BEGIN

/*
 * lifecycle.h - envelhecimento, família e árvore genealógica dos Sims.
 *
 * MODELO
 *   Person   registro PERSISTENTE de uma pessoa (viva ou morta). A genealogia mora
 *            aqui e não na Entity: uma Entity existe só enquanto o Sim está no mundo,
 *            enquanto a Person continua existindo depois que o Sim morre ou sai da lota.
 *   Entity   o corpo no mundo. A Person aponta para ela por EntityId (referência
 *            "mole": resolvida com EntityTable_GetConst, nunca confiada às cegas).
 *
 * IDADE é DERIVADA, não armazenada: idade = dia_atual - birth_day. Não há contador de
 * idade para ficar fora de sincronia; pular dias (carregar um save antigo, pausa longa)
 * não "perde" envelhecimento. Para congelar a idade (envelhecimento desligado), o
 * birth_day é empurrado um dia por dia processado.
 *
 * SEGURANÇA
 *   - sem malloc, sem ponteiros no estado persistente, tudo em tabelas de tamanho fixo;
 *   - PersonId = (geração << 8) | (slot + 1): um id de pessoa removida (poda de
 *     ancestrais antigos) nunca resolve para outra pessoa que reutilize o slot;
 *   - a árvore é um DAG por construção: todo pai nasce ESTRITAMENTE antes do filho
 *     (regra aplicada na criação e conferida em Lifecycle_Validate), então não existem
 *     ciclos e toda travessia é limitada;
 *   - Lifecycle_Deserialize nunca confia no buffer: limites verificados, contagens
 *     validadas, invariantes conferidas antes de qualquer uso.
 *
 * INTEGRAÇÃO COM A SIMULAÇÃO (ver LifeContext / LifeHooks)
 *   Uma vez por dia de jogo:   Life_Update(&life, &ctx, clock.day);
 *   Depois, drene os eventos (nascimento, aniversário, morte...) para a interface.
 *   Para criar/remover Entities com a limpeza de runtime da simulação (reservas de
 *   objetos, pedidos de pathfinding), passe LifeHooks que chamem Simulation_SpawnSim /
 *   Simulation_Despawn. Sem hooks, o módulo opera direto na EntityTable (útil em testes).
 *
 * VALORES de duração de cada fase, chance de morte, gravidez etc. são PARÂMETROS DE
 * PROJETO (placeholders ajustáveis em lifecycle.c), não valores do jogo original.
 */

/* ------------------------------------------------------------- capacidades -- */

#define LIFE_MAX_PEOPLE           96u    /* pessoas guardadas (vivas + ancestrais) */
#define LIFE_MAX_HOUSEHOLD_SIMS   8u     /* vivos por família (inclui gestações em curso) */
#define LIFE_PREGNANCY_DAYS       3u
#define LIFE_MAX_CATCHUP_DAYS     32u    /* dias processados dia a dia por Life_Update */
#define LIFE_MAX_DAY              (1u << 29)
#define LIFE_EVENT_CAP            16u
#define LIFE_TREE_MAX             32u
#define LIFE_NO_HOUSEHOLD         0xFFu
#define LIFE_SERIAL_VERSION       1u
#define LIFE_SERIAL_MAX           4608u  /* tamanho máximo do buffer serializado */

/* ---------------------------------------------------------------- tipos -- */

typedef u16 PersonId;
#define PERSON_ID_NONE            0u
#define PERSON_ID_INDEX(id)       ((u32)(id) & 0xFFu)            /* 1..LIFE_MAX_PEOPLE */
#define PERSON_ID_GEN(id)         ((u32)(id) >> 8)               /* 1..255 */
#define PERSON_ID_MAKE(idx, gen)  ((PersonId)(((u32)(gen) << 8) | (u32)(idx)))

enum LifeStage {
    LIFE_STAGE_BABY = 0,
    LIFE_STAGE_TODDLER,
    LIFE_STAGE_CHILD,
    LIFE_STAGE_TEEN,
    LIFE_STAGE_YOUNG_ADULT,
    LIFE_STAGE_ADULT,
    LIFE_STAGE_ELDER,
    LIFE_STAGE_COUNT
};
/* Entity.age_stage recebe exatamente estes valores. */

enum LifeSpan { LIFE_SPAN_SHORT = 0, LIFE_SPAN_NORMAL, LIFE_SPAN_LONG, LIFE_SPAN_COUNT };

enum LifeGender { LIFE_GENDER_FEMALE = 0, LIFE_GENDER_MALE, LIFE_GENDER_OTHER, LIFE_GENDER_COUNT };

enum LifeCause {
    LIFE_CAUSE_NONE = 0,         /* vivo */
    LIFE_CAUSE_OLD_AGE,
    LIFE_CAUSE_STARVATION,
    LIFE_CAUSE_ACCIDENT,
    LIFE_CAUSE_OTHER,
    LIFE_CAUSE_COUNT
};

/* Person.flags */
#define LIFE_P_ALIVE         0x01u
#define LIFE_P_CAN_CARRY     0x02u   /* pode gestar (independe do rótulo de gênero) */
#define LIFE_P_NO_AGING      0x04u   /* não envelhece (NPC fixo, regra de história) */
#define LIFE_P_WANTS_ENTITY  0x08u   /* deve ter corpo no mundo (membro da lota) */
#define LIFE_P_DEFERRED      0x10u   /* nascimento adiado já foi notificado */
#define LIFE_P_FLAGS_MASK    0x1Fu

/* Lifecycle.settings */
#define LIFE_SET_AGING          0x01u
#define LIFE_SET_NATURAL_DEATH  0x02u
#define LIFE_SET_MASK           0x03u

typedef struct Person {
    s32      birth_day;          /* pode ser negativo (Sim criado já adulto) */
    s32      death_day;          /* só para mortos; 0 enquanto vivo */
    EntityId entity;             /* referência mole ao corpo; 0 = sem corpo */
    u32      genes;              /* 8 genes de 4 bits (aparência/herança) */
    PersonId id;                 /* 0 = slot livre */
    PersonId parent[2];          /* pais (0 = desconhecido); sem restrição de gênero */
    PersonId partner;            /* parceiro atual (vínculo sempre mútuo) */
    PersonId preg_other;         /* outro genitor da gestação em curso */
    u16      traits;             /* 16 traços herdáveis */
    u8       household;          /* índice da família ou LIFE_NO_HOUSEHOLD */
    u8       flags;              /* LIFE_P_* */
    u8       gender;             /* LifeGender (informativo; nenhuma regra depende dele) */
    u8       cause;              /* LifeCause */
    u8       preg_left;          /* dias de gestação restantes (0 = não gestante) */
    u8       pad;
    char     name[OTS3_NAME_LEN];
} Person;

OTS3_STATIC_ASSERT(person_size_stable, sizeof(Person) == 48);

typedef struct Lifecycle {
    Person people[LIFE_MAX_PEOPLE];
    u8     gen[LIFE_MAX_PEOPLE];     /* geração de cada slot (nunca 0) */
    u8     lifespan;                 /* LifeSpan */
    u8     settings;                 /* LIFE_SET_* */
    u8     pad[2];
    u32    last_day;                 /* último dia já processado */
    u32    rng;                      /* RNG próprio (xorshift32), nunca 0 */
} Lifecycle;

/* ---------------------------------------------------------------- eventos -- */

enum LifeEventType {
    LIFE_EVENT_NONE = 0,
    LIFE_EVENT_BIRTH,            /* person = bebê, other = genitor que gestou */
    LIFE_EVENT_BIRTH_DEFERRED,   /* tabela cheia: tentará de novo no dia seguinte */
    LIFE_EVENT_STAGE_CHANGE,     /* "aniversário": arg = novo LifeStage */
    LIFE_EVENT_DEATH,            /* arg = LifeCause; entity = corpo (já inválido) */
    LIFE_EVENT_PREGNANCY,        /* person = gestante, other = outro genitor */
    LIFE_EVENT_PARTNER,          /* person e other passaram a ser parceiros */
    LIFE_EVENT_BREAKUP,
    LIFE_EVENT_ENTITY_SPAWNED    /* corpo criado para uma pessoa */
};

typedef struct LifeEvent {
    u8       type;
    u8       arg;
    PersonId person;
    PersonId other;
    u16      pad;
    EntityId entity;
} LifeEvent;

/* Fila circular de capacidade fixa. Cheia => o evento NOVO é descartado e contado. */
typedef struct LifeEventQueue {
    LifeEvent ev[LIFE_EVENT_CAP];
    u8  head;
    u8  count;
    u16 dropped;
} LifeEventQueue;

void LifeEvents_Init(LifeEventQueue* q);
BOOL LifeEvents_Pop(LifeEventQueue* q, LifeEvent* out);

/* ----------------------------------------------------------------- contexto -- */

/* Criação/remoção de corpos. Opcional: NULL => opera direto na EntityTable. */
typedef struct LifeHooks {
    void* user;
    EntityId (*spawn_sim)(void* user, u8 household, const char* name, Vec2Fx pos);
    void     (*despawn)(void* user, EntityId id);
} LifeHooks;

typedef struct LifeContext {
    EntityTable*    ents;        /* obrigatório */
    LifeEventQueue* events;      /* opcional */
    const LifeHooks* hooks;      /* opcional */
    Vec2Fx          spawn_pos;   /* onde nascem corpos sem referência melhor */
} LifeContext;

/* ----------------------------------------------------------------- resultados -- */

typedef enum LifeResult {
    LIFE_OK = 0,
    LIFE_ERR_ARG,                /* argumento inválido / id inexistente */
    LIFE_ERR_FULL,               /* tabela cheia e nada podável */
    LIFE_ERR_RULE,               /* viola uma regra (parentesco, idade, limite) */
    LIFE_ERR_STATE,              /* estado não permite (morto, já gestante...) */
    LIFE_ERR_CORRUPT             /* dados inválidos (save) */
} LifeResult;

enum LifeRelation {              /* "o que B é para A" */
    LIFE_REL_SELF = 0,
    LIFE_REL_PARENT,
    LIFE_REL_CHILD,
    LIFE_REL_SIBLING,            /* mesmos dois pais */
    LIFE_REL_HALF_SIBLING,       /* um pai em comum */
    LIFE_REL_GRANDPARENT,
    LIFE_REL_GRANDCHILD,
    LIFE_REL_AUNT_UNCLE,
    LIFE_REL_NIECE_NEPHEW,
    LIFE_REL_COUSIN,
    LIFE_REL_ANCESTOR,           /* bisavô e acima */
    LIFE_REL_DESCENDANT,
    LIFE_REL_PARTNER,
    LIFE_REL_UNRELATED,
    LIFE_REL_UNKNOWN             /* id inválido */
};

/* ------------------------------------------------------------------ criação -- */

typedef struct LifePersonInit {
    const char* name;
    u32      age_days;           /* idade inicial (use Life_StageStartDays para "entrar" numa fase) */
    PersonId parent[2];          /* opcionais; precisam existir e ser mais velhos */
    u32      genes;              /* 0 => sorteado */
    u16      traits;
    u8       gender;
    u8       household;          /* LIFE_NO_HOUSEHOLD se não pertence a nenhuma */
    u8       can_carry;          /* 0/1 */
    u8       no_aging;           /* 0/1 */
    u8       wants_entity;       /* 0/1 (exige household válido) */
} LifePersonInit;

void Lifecycle_Init(Lifecycle* l, u32 seed, u32 today);

/* Cria uma pessoa (fundador, adotado, convidado). Adoção = pais zerados. */
LifeResult Life_AddPerson(Lifecycle* l, const LifePersonInit* init, u32 today, PersonId* out);

Person*       Life_Get(Lifecycle* l, PersonId id);                 /* NULL se inválido/podado */
const Person* Life_GetConst(const Lifecycle* l, PersonId id);
LifeResult    Life_SetName(Lifecycle* l, PersonId id, const char* name);
LifeResult    Life_SetWantsEntity(Lifecycle* l, PersonId id, BOOL wants);
void          Life_SetLifespan(Lifecycle* l, u8 lifespan);
void          Life_SetSettings(Lifecycle* l, u8 settings);

/* ------------------------------------------------------------------- idade -- */

u32  Life_AgeDays(const Person* p, u32 today);
u8   Life_StageOfAge(u8 lifespan, u32 age_days);
u8   Life_StageOf(const Lifecycle* l, const Person* p, u32 today);
u32  Life_StageStartDays(u8 lifespan, u8 stage);
/* Chance de morte natural no dia em que a pessoa completa 'age_days' (permille).
 * 0 antes do período de graça; 1000 no limite máximo (morte garantida). */
u32  Life_DeathChancePermille(u8 lifespan, u32 age_days);

/* Capacidades por fase, para a IA/UI não tratar um bebê como adulto. */
#define LIFE_CAP_WALK        0x01u
#define LIFE_CAP_USE_OBJECTS 0x02u
#define LIFE_CAP_WORK        0x04u
#define LIFE_CAP_PARTNER     0x08u
#define LIFE_CAP_CARRY_BABY  0x10u
u32  Life_StageCaps(u8 stage);

/* ------------------------------------------------------------------ família -- */

BOOL       Life_CanPartner(const Lifecycle* l, PersonId a, PersonId b, u32 today);
LifeResult Life_SetPartner(Lifecycle* l, LifeContext* ctx, PersonId a, PersonId b, u32 today);
LifeResult Life_ClearPartner(Lifecycle* l, LifeContext* ctx, PersonId a);
/* A gestante deve ser parceira de 'other'. Respeita o limite de vivos da família. */
LifeResult Life_StartPregnancy(Lifecycle* l, LifeContext* ctx, PersonId carrier,
                               PersonId other, u32 today);
/* Mata (causas externas: fome, acidente). Remove o corpo, reatribui os pets do morto
 * ao herdeiro, desfaz o vínculo de parceria. Idempotente em mortos (LIFE_ERR_STATE). */
LifeResult Life_Kill(Lifecycle* l, LifeContext* ctx, PersonId id, u8 cause, u32 today);

/* Processa os dias entre last_day e 'today': envelhecimento, aniversários, gestação,
 * nascimentos, morte natural e criação de corpos pendentes. Devolve dias processados.
 * 'today' menor que last_day (relógio voltou/corrompido) não faz nada. */
u32 Life_Update(Lifecycle* l, LifeContext* ctx, u32 today);
/* Cria agora os corpos das pessoas vivas com LIFE_P_WANTS_ENTITY que ainda não têm um
 * (use depois de Life_AddPerson, no início do jogo). Devolve quantos criou. */
u32 Life_MaterializeBodies(Lifecycle* l, LifeContext* ctx, u32 today);

/* --------------------------------------------------------------- consultas -- */

u8   Life_Relationship(const Lifecycle* l, PersonId a, PersonId b);   /* LifeRelation */
BOOL Life_IsCloseRelative(const Lifecycle* l, PersonId a, PersonId b);
u32  Life_GetChildren(const Lifecycle* l, PersonId parent, PersonId* out, u32 cap);  /* por data de nascimento */
u32  Life_GetHousehold(const Lifecycle* l, u8 household, BOOL only_alive, PersonId* out, u32 cap);
u32  Life_CountAlive(const Lifecycle* l);

enum { LIFE_TREE_ANCESTORS = 0, LIFE_TREE_DESCENDANTS = 1 };

typedef struct LifeTreeNode {
    PersonId id;
    s8       parent_node;        /* índice do nó de onde veio (-1 = raiz) */
    u8       depth;              /* 0 = raiz */
} LifeTreeNode;

typedef struct LifeTree {
    u8 count;
    u8 truncated;                /* 1 se LIFE_TREE_MAX cortou a árvore */
    LifeTreeNode node[LIFE_TREE_MAX];
} LifeTree;

/* Árvore em largura a partir de 'root', até 'max_depth' gerações. Sempre termina. */
u32  Life_BuildTree(const Lifecycle* l, PersonId root, u8 direction, u8 max_depth, LifeTree* out);

/* ---------------------------------------------------------- validação / save -- */

/* Todas as invariantes estruturais (ver lifecycle.c). Não exige EntityTable: os
 * vínculos com corpos são "moles" e revalidados no uso. */
BOOL       Lifecycle_Validate(const Lifecycle* l);
BOOL       Lifecycle_Serialize(const Lifecycle* l, void* buf, u32 cap, u32* out_len);
/* Decodifica para um staging, valida e só então copia para 'out'. Em erro, 'out' não muda. */
LifeResult Lifecycle_Deserialize(Lifecycle* out, const void* buf, u32 len);

OTS3_EXTERN_C_END

#endif /* OTS3_LIFECYCLE_H */
