#ifndef OTS3_NAV_H
#define OTS3_NAV_H

#include "config.h"

OTS3_EXTERN_C_BEGIN

/*
 * nav.h - pathfinding determinístico por A* em grade, com orçamento por quadro.
 *
 *  - 8 direções (custo 10 / 14), sem "cortar quina" em diagonais;
 *  - heurística octile (consistente) => o caminho é ótimo;
 *  - ordem de vizinhos e desempate do heap fixos => resultado idêntico no NDS e na PC;
 *  - busca incremental: Nav_Step(budget) expande no máximo 'budget' nós por chamada;
 *  - tudo estático e limitado: fila de NAV_QUEUE pedidos, no máximo NAV_MAX_EXPANSIONS
 *    expansões por busca, heap com capacidade fixa (estouro => FALHA, nunca corrupção).
 *
 * Perfis: Sims e pets enxergam a grade de forma diferente (porta de pet: só pets
 * passam; cerca baixa: só Sims passam).
 */

#define NAV_MAX_W            48u
#define NAV_MAX_H            48u
#define NAV_MAX_TILES        (NAV_MAX_W * NAV_MAX_H)
#define NAV_MAX_PATH         64u          /* passos guardados por caminho */
#define NAV_QUEUE            8u
#define NAV_MAX_EXPANSIONS   (NAV_MAX_TILES + 16u)
#define NAV_HEAP_CAP         6144u

/* flags de tile */
#define NAV_SOLID            0x01u        /* ninguém passa */
#define NAV_NO_PET           0x02u        /* pets não passam */
#define NAV_NO_SIM           0x04u        /* Sims não passam (porta de pet) */
#define NAV_FLAGS_MASK       0x07u

#define NAV_PROFILE_SIM      0u
#define NAV_PROFILE_PET      1u

/* tile empacotado: (y << 8) | x */
#define NAV_TILE(x, y)       ((u16)((((u32)(y)) << 8) | ((u32)(x) & 0xFFu)))
#define NAV_TILE_X(t)        ((u32)(t) & 0xFFu)
#define NAV_TILE_Y(t)        ((u32)(t) >> 8)

typedef struct NavGrid {
    u8 w, h;
    u8 flags[NAV_MAX_TILES];
} NavGrid;

typedef struct NavPath {
    u8  len;                       /* passos válidos em tiles[] (o início NÃO está incluso) */
    u8  partial;                   /* 1 = caminho maior que NAV_MAX_PATH: pedir de novo no fim */
    u16 tiles[NAV_MAX_PATH];
} NavPath;

typedef enum NavResult {
    NAV_RESULT_NONE = 0,           /* nenhum pedido deste dono */
    NAV_RESULT_PENDING,
    NAV_RESULT_FOUND,
    NAV_RESULT_FAILED
} NavResult;

typedef struct NavRequest {
    u32 who;                       /* chave do dono (EntityId) */
    u32 seq;                       /* ordem de chegada (FIFO) */
    u16 start, goal;
    u8  profile;
    u8  state;                     /* NAV_REQ_* (interno) */
    u8  result;                    /* NavResult quando DONE */
    u8  pad;
    NavPath path;
} NavRequest;

typedef struct NavSystem {
    NavRequest req[NAV_QUEUE];
    u32  next_seq;
    u8   active;                   /* índice do pedido em busca, 0xFF = nenhum */
    u8   pad0[3];
    /* estado da busca ativa */
    u32  expansions;
    u32  heap_n;
    u16  g[NAV_MAX_TILES];
    u8   parent[NAV_MAX_TILES];    /* direção de chegada (0..7) */
    u8   closed[(NAV_MAX_TILES + 7u) / 8u];
    u32  heap[NAV_HEAP_CAP];
} NavSystem;

void Nav_Init(NavSystem* nav);

/* Valida a grade (dimensões e flags). */
BOOL NavGrid_Validate(const NavGrid* g);

/* Enfileira um pedido (substitui o pedido anterior do mesmo dono).
 * FALSE se a fila está cheia ou as coordenadas são inválidas. */
BOOL Nav_Submit(NavSystem* nav, const NavGrid* g, u32 who, u8 profile,
                u32 sx, u32 sy, u32 gx, u32 gy);

void Nav_Cancel(NavSystem* nav, u32 who);

/* Expande até 'budget' nós. Chame uma vez por tick. */
OTS3_HOT void Nav_Step(NavSystem* nav, const NavGrid* g, u32 budget);

/* PENDING enquanto busca; FOUND/FAILED entregam o resultado UMA vez e liberam o pedido. */
NavResult Nav_Poll(NavSystem* nav, u32 who, NavPath* out);

/* Tile é transitável para o perfil? (fora da grade => FALSE) */
BOOL Nav_Passable(const NavGrid* g, u8 profile, s32 x, s32 y);

OTS3_EXTERN_C_END

#endif /* OTS3_NAV_H */
