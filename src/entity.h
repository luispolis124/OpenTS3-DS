#ifndef OPENTS3DS_ENTITY_H
#define OPENTS3DS_ENTITY_H

#include "types.h"

/*
 * entity.h - ESQUELETO PROVISÓRIO das entidades do jogo.
 *
 * Nada aqui foi confirmado contra o binário: nomes, tipos, ordem dos campos e
 * tamanhos são hipóteses de partida, escolhidas pelo que um jogo desse gênero
 * costuma precisar. Elas só viram fato conforme você decompilar o código que usa
 * essas estruturas.
 *
 * Fluxo sugerido para ir confirmando:
 *   1. No Ghidra, ache o laço principal que percorre os objetos do jogo e descubra
 *      o tamanho da struct (pelo stride do laço ou pelo argumento do alocador).
 *   2. Crie a struct no Ghidra com o mesmo layout e aplique nos parâmetros das funções.
 *   3. Aqui, troque cada campo "?" pelo nome/tipo real e trave o offset:
 *          STATIC_ASSERT(entity_pos, offsetof(Entity, pos) == 0x0C);
 *      (inclua <stddef.h> no .c que usar offsetof).
 *
 * Pista que pode importar: as tabelas de overlay trazem listas "static init"
 * (ex.: overlay 0 tem 1 inicializador). Isso é típico de C++ com construtores
 * globais. Se o jogo for mesmo C++, Entity provavelmente é uma classe com vtable
 * (o ponteiro "vtable" abaixo já assume isso) e este header vira uma descrição
 * de layout, não de código-fonte original.
 */

/* ----- Vetores em ponto fixo (matemática de 3D do DS) --------------------- */
typedef struct Vec2Fx { fx32 x, y; } Vec2Fx;
typedef struct Vec3Fx { fx32 x, y, z; } Vec3Fx;

/* Ângulos do DS usam 16 bits: 0x10000 = 360 graus. */
typedef u16 Angle16;

/* ----- Tipos de entidade (hipótese; confirmar com a tabela de despacho) ---- */
typedef enum EntityType {
    ENTITY_TYPE_NONE = 0,
    ENTITY_TYPE_SIM,        /* personagem controlável/NPC */
    ENTITY_TYPE_OBJECT,     /* móvel/objeto interativo */
    ENTITY_TYPE_EFFECT,     /* partículas, ícones sobre a cabeça, etc. */
    ENTITY_TYPE_COUNT
} EntityType;

/* ----- Flags (bits ainda desconhecidos) ------------------------------------ */
enum {
    ENTITY_FLAG_ACTIVE   = 1 << 0,  /* ? */
    ENTITY_FLAG_VISIBLE  = 1 << 1,  /* ? */
    ENTITY_FLAG_PENDING_DESTROY = 1 << 2  /* ? */
};

typedef struct Entity Entity;

/* Tabela de métodos virtuais (hipótese). Os ponteiros reais apontarão para
 * funções em overlays: não chame-os sem checar se o overlay está carregado. */
typedef struct EntityVTable {
    void (*init)(Entity *self);
    void (*update)(Entity *self);
    void (*draw)(Entity *self);
    void (*destroy)(Entity *self);
} EntityVTable;

struct Entity {
    const EntityVTable *vtable;  /* ? provisório */
    u16 type;                    /* EntityType */
    u16 flags;                   /* ENTITY_FLAG_* */
    u32 id;                      /* ? identificador único */

    Vec3Fx  pos;                 /* posição no mundo (20.12) */
    Angle16 rot_y;               /* ? rotação em torno do eixo vertical */
    Angle16 pad_rot;

    Entity *prev;                /* ? lista duplamente encadeada de entidades vivas */
    Entity *next;

    void *model;                 /* ? handle do modelo 3D (os assets estão nos .package) */
    void *anim;                  /* ? estado de animação */
    u32 state;                   /* ? estado atual da máquina de estados */
    u32 state_timer;             /* ? */
    void *user;                  /* ? dados específicos do tipo (Sim, objeto...) */
};

/* ----- Lista de entidades -------------------------------------------------- */
typedef struct EntityList {
    Entity *head;
    Entity *tail;
    u32 count;
} EntityList;

/* ----- API provisória (nomes não existem no binário; renomeie ao identificar) -- */
void Entity_Init(Entity *e, EntityType type, const EntityVTable *vtable);
void Entity_Update(Entity *e);
void Entity_Draw(Entity *e);
void Entity_Destroy(Entity *e);

void EntityList_Init(EntityList *list);
void EntityList_Add(EntityList *list, Entity *e);
void EntityList_Remove(EntityList *list, Entity *e);
void EntityList_UpdateAll(EntityList *list);
void EntityList_DrawAll(EntityList *list);

#endif /* OPENTS3DS_ENTITY_H */
