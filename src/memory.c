#include "memory.h"

#include <string.h>

BOOL Arena_Init(Arena* a, void* mem, u32 cap)
{
    if (!a || !mem || cap == 0 || (((unsigned long)mem) & 7u) != 0) {
        return FALSE;
    }
    a->base = (u8*)mem;
    a->cap = cap;
    a->front = 0;
    a->back = cap;
    a->high_front = 0;
    a->high_back = 0;
    return TRUE;
}

static BOOL AlignOk(u32 align)
{
    return align >= 1u && align <= 8u && (align & (align - 1u)) == 0;
}

void* Arena_Alloc(Arena* a, u32 size, u32 align)
{
    u64 start, end;

    if (!a || !a->base || !AlignOk(align)) {
        return NULL;
    }
    start = ((u64)a->front + align - 1u) & ~((u64)align - 1u);
    end = start + size;
    if (end > a->back) {
        return NULL;
    }
    a->front = (u32)end;
    if (a->front > a->high_front) {
        a->high_front = a->front;
    }
    return a->base + start;
}

void* Arena_AllocBack(Arena* a, u32 size, u32 align)
{
    u64 start;

    if (!a || !a->base || !AlignOk(align) || size > a->back) {
        return NULL;
    }
    start = ((u64)a->back - size) & ~((u64)align - 1u);
    if (start < a->front) {
        return NULL;
    }
    a->back = (u32)start;
    if (a->cap - a->back > a->high_back) {
        a->high_back = a->cap - a->back;
    }
    return a->base + start;
}

u32 Arena_BackMark(const Arena* a)
{
    return a->back;
}

void Arena_BackRelease(Arena* a, u32 mark)
{
    if (mark >= a->back && mark <= a->cap) {
        a->back = mark;
    }
}

u32 Arena_Free(const Arena* a)
{
    return a->back - a->front;
}

void Arena_Reset(Arena* a)
{
    a->front = 0;
    a->back = a->cap;
}

/* ------------------------------------------------------------------ VRAM -- */

void Vram_Init(VramBudget* v, BOOL enforce)
{
    memset(v, 0, sizeof(*v));
    v->total_limit = OTS3_VRAM_TOTAL_BYTES;
    v->pool_limit[VRAM_POOL_WORLD] = OTS3_VRAM_BUDGET_WORLD;
    v->pool_limit[VRAM_POOL_UI] = OTS3_VRAM_BUDGET_UI;
    v->pool_limit[VRAM_POOL_PET] = OTS3_VRAM_BUDGET_PET;
    v->pool_limit[VRAM_POOL_RESERVED] = OTS3_VRAM_BUDGET_RESERVED;
    v->enforce = enforce ? 1u : 0u;
}

BOOL Vram_Reserve(VramBudget* v, u8 pool, u32 bytes)
{
    if (pool >= VRAM_POOL_COUNT) {
        return FALSE;
    }
    if (v->enforce &&
        ((u64)v->used + bytes > v->total_limit ||
         (u64)v->pool_used[pool] + bytes > v->pool_limit[pool])) {
        return FALSE;
    }
    if ((u64)v->used + bytes > 0xFFFFFFFFu || (u64)v->pool_used[pool] + bytes > 0xFFFFFFFFu) {
        return FALSE;                               /* contadores nunca dão a volta */
    }
    v->pool_used[pool] += bytes;
    v->used += bytes;
    if (v->used > v->high) {
        v->high = v->used;
    }
    return TRUE;
}

BOOL Vram_Release(VramBudget* v, u8 pool, u32 bytes)
{
    if (pool >= VRAM_POOL_COUNT || bytes > v->pool_used[pool]) {
        return FALSE;
    }
    v->pool_used[pool] -= bytes;
    v->used -= bytes;
    return TRUE;
}

u32 Vram_Free(const VramBudget* v, u8 pool)
{
    u32 by_pool, by_total;

    if (pool >= VRAM_POOL_COUNT) {
        return 0;
    }
    by_pool = v->pool_limit[pool] - (v->pool_used[pool] < v->pool_limit[pool] ? v->pool_used[pool] : v->pool_limit[pool]);
    by_total = v->total_limit - (v->used < v->total_limit ? v->used : v->total_limit);
    return by_pool < by_total ? by_pool : by_total;
}
