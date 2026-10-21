#ifndef OTS3_MEMORY_H
#define OTS3_MEMORY_H

#include "config.h"

OTS3_EXTERN_C_BEGIN

/*
 * memory.h - gerenciador de memória SEM heap.
 *
 *  Arena       região fornecida pelo chamador (array estático). Aloca "para frente"
 *              (dados que ficam) e "para trás" (temporários de carga, liberados por
 *              marca). Nunca chama malloc; falta de espaço devolve NULL, não corrompe.
 *              Toda aritmética usa u64: pedidos gigantes não dão a volta no u32.
 *  VramBudget  contabilidade da VRAM (656 KB no NDS) por partição. Não toca no
 *              hardware: serve para recusar assets que não cabem ANTES de carregá-los.
 *              Na PC (enforce = FALSE) só registra o uso.
 */

typedef struct Arena {
    u8* base;
    u32 cap;
    u32 front;                 /* próximo byte livre, alocação "para frente" */
    u32 back;                  /* início da região de temporários (cresce para baixo) */
    u32 high_front;            /* maior 'front' já atingido */
    u32 high_back;             /* maior uso da região de temporários */
} Arena;

/* 'mem' deve estar alinhado a 8 bytes. FALSE se nulo/desalinhado/cap == 0. */
BOOL  Arena_Init(Arena* a, void* mem, u32 cap);
/* 'align' potência de dois, 1..8. NULL se não couber. */
void* Arena_Alloc(Arena* a, u32 size, u32 align);
void* Arena_AllocBack(Arena* a, u32 size, u32 align);
u32   Arena_BackMark(const Arena* a);
void  Arena_BackRelease(Arena* a, u32 mark);          /* libera tudo alocado desde a marca */
u32   Arena_Free(const Arena* a);
void  Arena_Reset(Arena* a);                          /* descarta tudo (só se nada mais usa) */

enum VramPool {
    VRAM_POOL_WORLD = 0,
    VRAM_POOL_UI,
    VRAM_POOL_PET,
    VRAM_POOL_RESERVED,
    VRAM_POOL_COUNT
};

typedef struct VramBudget {
    u32 total_limit;
    u32 pool_limit[VRAM_POOL_COUNT];
    u32 pool_used[VRAM_POOL_COUNT];
    u32 used;
    u32 high;
    u8  enforce;
    u8  pad[3];
} VramBudget;

void Vram_Init(VramBudget* v, BOOL enforce);
BOOL Vram_Reserve(VramBudget* v, u8 pool, u32 bytes);    /* FALSE: não cabe (nada muda) */
BOOL Vram_Release(VramBudget* v, u8 pool, u32 bytes);    /* FALSE: devolveu mais do que reservou */
u32  Vram_Free(const VramBudget* v, u8 pool);

OTS3_EXTERN_C_END

#endif /* OTS3_MEMORY_H */
