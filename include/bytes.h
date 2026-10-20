#ifndef OTS3_BYTES_H
#define OTS3_BYTES_H

#include "config.h"

#include <string.h>

OTS3_EXTERN_C_BEGIN

/*
 * bytes.h - escrita/leitura sequencial em buffer contínuo, com limites verificados.
 *
 * Substitui o "memcpy de struct" por serialização campo a campo:
 *   - ordem de bytes fixa (little-endian), igual no NDS e na PC;
 *   - sem dependência de padding, alinhamento ou tamanho de ponteiro;
 *   - toda operação confere 'n <= cap - pos' ANTES de copiar (a soma pos + n nunca é
 *     calculada, então não há estouro de u32); o erro é "pegajoso": depois da primeira
 *     falha, escritas viram no-ops e leituras devolvem 0.
 * O chamador checa BW_Ok()/BR_Ok() uma vez no fim.
 */

typedef struct ByteWriter {
    u8* p;
    u32 cap;
    u32 pos;
    u8  err;
} ByteWriter;

typedef struct ByteReader {
    const u8* p;
    u32 size;
    u32 pos;
    u8  err;
} ByteReader;

static inline void BW_Init(ByteWriter* w, void* buf, u32 cap)
{
    w->p = (u8*)buf; w->cap = cap; w->pos = 0; w->err = 0;
}

static inline BOOL BW_Ok(const ByteWriter* w) { return w->err == 0; }

static inline void BW_Bytes(ByteWriter* w, const void* src, u32 n)
{
    if (w->err || n > w->cap - w->pos) {
        w->err = 1;
        return;
    }
    if (n) {
        memcpy(w->p + w->pos, src, n);
    }
    w->pos += n;
}

static inline void BW_U8(ByteWriter* w, u32 v)
{
    u8 b = (u8)v;
    BW_Bytes(w, &b, 1);
}

static inline void BW_U16(ByteWriter* w, u32 v)
{
    u8 b[2];
    b[0] = (u8)v; b[1] = (u8)(v >> 8);
    BW_Bytes(w, b, 2);
}

static inline void BW_U32(ByteWriter* w, u32 v)
{
    u8 b[4];
    b[0] = (u8)v; b[1] = (u8)(v >> 8); b[2] = (u8)(v >> 16); b[3] = (u8)(v >> 24);
    BW_Bytes(w, b, 4);
}

static inline void BW_S32(ByteWriter* w, s32 v) { BW_U32(w, (u32)v); }

static inline void BR_Init(ByteReader* r, const void* buf, u32 size)
{
    r->p = (const u8*)buf; r->size = size; r->pos = 0; r->err = 0;
}

static inline BOOL BR_Ok(const ByteReader* r) { return r->err == 0; }
static inline u32  BR_Remaining(const ByteReader* r) { return r->size - r->pos; }

static inline void BR_Bytes(ByteReader* r, void* dst, u32 n)
{
    if (r->err || n > r->size - r->pos) {
        r->err = 1;
        if (n) {
            memset(dst, 0, n);                    /* nunca deixa lixo no destino */
        }
        return;
    }
    if (n) {
        memcpy(dst, r->p + r->pos, n);
    }
    r->pos += n;
}

static inline u32 BR_U8(ByteReader* r)
{
    u8 b = 0;
    BR_Bytes(r, &b, 1);
    return b;
}

static inline u32 BR_U16(ByteReader* r)
{
    u8 b[2] = { 0, 0 };
    BR_Bytes(r, b, 2);
    return (u32)b[0] | ((u32)b[1] << 8);
}

static inline u32 BR_U32(ByteReader* r)
{
    u8 b[4] = { 0, 0, 0, 0 };
    BR_Bytes(r, b, 4);
    return (u32)b[0] | ((u32)b[1] << 8) | ((u32)b[2] << 16) | ((u32)b[3] << 24);
}

static inline s32 BR_S32(ByteReader* r) { return (s32)BR_U32(r); }

OTS3_EXTERN_C_END

#endif /* OTS3_BYTES_H */
