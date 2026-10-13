#ifndef OPENTS3DS_TYPES_H
#define OPENTS3DS_TYPES_H

/*
 * Tipos básicos. Não usa <stdint.h> de propósito: o compilador original do SDK
 * (Metrowerks) e vários toolchains de DS não garantem esse header.
 * Assume ILP32 (ARM9/ARM7): int e long têm 32 bits.
 */

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef volatile u8        vu8;
typedef volatile u16       vu16;
typedef volatile u32       vu32;
typedef volatile s8        vs8;
typedef volatile s16       vs16;
typedef volatile s32       vs32;

typedef int                BOOL;
#define TRUE               1
#define FALSE              0

#ifndef NULL
#define NULL               ((void *)0)
#endif

/* Ponto fixo, formato padrão do hardware 3D do DS: 20.12 (32 bits) e 4.12 (16 bits). */
typedef s32 fx32;
typedef s16 fx16;
#define FX32_SHIFT         12
#define FX32_ONE           (1 << FX32_SHIFT)
#define FX32(x)            ((fx32)((x) * (1 << FX32_SHIFT)))
#define FX32_TO_INT(x)     ((x) >> FX32_SHIFT)

/* Asserção em tempo de compilação (C99): use para travar offsets/tamanhos confirmados. */
#define STATIC_ASSERT(name, cond) typedef char static_assert_##name[(cond) ? 1 : -1]

#endif /* OPENTS3DS_TYPES_H */
