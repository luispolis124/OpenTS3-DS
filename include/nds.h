#ifndef OPENTS3DS_NDS_H
#define OPENTS3DS_NDS_H

/*
 * nds.h - header global do projeto. Inclua este em todo .c.
 *
 * Ele reúne types.h + memory_map.h e acrescenta o que não está lá:
 *   1. atributos de compilador e de seção (ligados ao linkscript.ld)
 *   2. vídeo 2D (BG afim, janelas, blend, captura) e campos dos registradores
 *   3. 3D (portas de comando da geometry engine)
 *   4. DMA (bits de controle + funções inline)
 *   5. timers, energia, SPI (power-management/toque), som
 *   6. números das chamadas SWI do BIOS (para reconhecer no desmontado)
 *   7. helpers pequenos
 *
 * Registradores e bits seguem a documentação pública do hardware (GBATEK);
 * confira nela antes de depender de um bit específico. Marcações [ARM9]/[ARM7]/[ambos]
 * indicam qual CPU enxerga o registrador.
 *
 * Nenhum nome daqui repete um de memory_map.h (que já define DISPCNT, DISPSTAT,
 * BGCNT, POWCNT1/2, VRAMCNT, IME/IE/IF, timers/DMA (endereços), teclas, IPC, SPICNT,
 * SOUNDCNT, RTC).
 */

#include "types.h"
#include "memory_map.h"

/* ======================================================================== */
/* 1. Atributos de compilador e de seção                                    */
/* ======================================================================== */

#if defined(__GNUC__)
#  define ARM_CODE        __attribute__((target("arm")))
#  define THUMB_CODE      __attribute__((target("thumb")))
#  define NORETURN        __attribute__((noreturn))
#  define UNUSED          __attribute__((unused))
#  define PACKED          __attribute__((packed))
#  define ALIGN(n)        __attribute__((aligned(n)))
#  define NOINLINE        __attribute__((noinline))
#  define SECTION(name)   __attribute__((section(name)))
#else
/* TODO: mapear para pragmas do compilador original (Metrowerks) quando o
 * toolchain de matching for definido. Vazio por enquanto para não quebrar o parse. */
#  define ARM_CODE
#  define THUMB_CODE
#  define NORETURN
#  define UNUSED
#  define PACKED
#  define ALIGN(n)
#  define NOINLINE
#  define SECTION(name)
#endif

/* Nomes de seção definidos em linkscript.ld. */
#if defined(ARM9)
#  define ITCM_CODE       SECTION(".itcm")       /* código/dados constantes na ITCM (32 KB) */
#  define ITCM_DATA       SECTION(".itcm")
#  define ITCM_BSS        SECTION(".itcm_bss")
#  define DTCM_DATA       SECTION(".dtcm")       /* dados na DTCM (16 KB) */
#  define DTCM_BSS        SECTION(".dtcm_bss")
#elif defined(ARM7)
#  define WRAM_CODE       SECTION(".wram")       /* autoload 0: WRAM do ARM7 */
#  define WRAM_DATA       SECTION(".wram")
#  define WRAM_BSS        SECTION(".wram_bss")
#  define HIGH_CODE       SECTION(".high")       /* autoload 1: topo da RAM principal */
#  define HIGH_DATA       SECTION(".high")
#  define HIGH_BSS        SECTION(".high_bss")
#endif

/* ======================================================================== */
/* 2. Vídeo 2D                                                              */
/* ======================================================================== */

/* DISPCNT (REG_DISPCNT / REG_DISPCNT_B): campos além dos já em memory_map.h */
#define DISPCNT_MODE(n)         ((u32)(n) & 7u)          /* modo de BG 0..6 */
#define DISPCNT_BG0_3D          (1u << 3)                /* [A] BG0 mostra a saída 3D */
#define DISPCNT_OBJ_1D_MAP      (1u << 4)                /* tiles de OBJ em mapeamento 1D */
#define DISPCNT_WIN0_ON         (1u << 13)
#define DISPCNT_WIN1_ON         (1u << 14)
#define DISPCNT_OBJWIN_ON       (1u << 15)
#define DISPCNT_DISPLAY_SHIFT   16                       /* bits 16-17: modo de exibição */
#define DISPCNT_DISPLAY_OFF     (0u << DISPCNT_DISPLAY_SHIFT)
#define DISPCNT_DISPLAY_GFX     (1u << DISPCNT_DISPLAY_SHIFT)
#define DISPCNT_DISPLAY_VRAM    (2u << DISPCNT_DISPLAY_SHIFT)  /* [A] */
#define DISPCNT_DISPLAY_MMEM    (3u << DISPCNT_DISPLAY_SHIFT)  /* [A] FIFO da RAM principal */

/* BGxCNT (REG_BGCNT(n)) */
#define BGCNT_PRIORITY(n)       ((u16)(n) & 3u)
#define BGCNT_CHAR_BASE(n)      (((u16)(n) & 0xFu) << 2)     /* x16 KB */
#define BGCNT_MOSAIC            (1u << 6)
#define BGCNT_256COLOR          (1u << 7)
#define BGCNT_SCREEN_BASE(n)    (((u16)(n) & 0x1Fu) << 8)    /* x2 KB */
#define BGCNT_WRAP              (1u << 13)                   /* BG2/BG3 afim: repete */
#define BGCNT_SIZE(n)           (((u16)(n) & 3u) << 14)

/* BG2/BG3 afim (ponto fixo 8.8 para PA..PD, 20.8 para X/Y) */
#define REG_BG2PA               HW_REG16(0x04000020)
#define REG_BG2PB               HW_REG16(0x04000022)
#define REG_BG2PC               HW_REG16(0x04000024)
#define REG_BG2PD               HW_REG16(0x04000026)
#define REG_BG2X                HW_REG32(0x04000028)
#define REG_BG2Y                HW_REG32(0x0400002C)
#define REG_BG3PA               HW_REG16(0x04000030)
#define REG_BG3PB               HW_REG16(0x04000032)
#define REG_BG3PC               HW_REG16(0x04000034)
#define REG_BG3PD               HW_REG16(0x04000036)
#define REG_BG3X                HW_REG32(0x04000038)
#define REG_BG3Y                HW_REG32(0x0400003C)

/* Janelas, mosaico, blend */
#define REG_WIN0H               HW_REG16(0x04000040)
#define REG_WIN1H               HW_REG16(0x04000042)
#define REG_WIN0V               HW_REG16(0x04000044)
#define REG_WIN1V               HW_REG16(0x04000046)
#define REG_WININ               HW_REG16(0x04000048)
#define REG_WINOUT              HW_REG16(0x0400004A)
#define REG_MOSAIC              HW_REG16(0x0400004C)
#define REG_BLDCNT              HW_REG16(0x04000050)
#define REG_BLDALPHA            HW_REG16(0x04000052)
#define REG_BLDY                HW_REG16(0x04000054)

/* Captura de tela [ARM9, engine A] */
#define REG_DISPCAPCNT          HW_REG32(0x04000064)
#define REG_DISP_MMEM_FIFO      HW_REG32(0x04000068)

/* MASTER_BRIGHT: bits 14-15 = modo */
#define MASTER_BRIGHT_UP        (1u << 14)
#define MASTER_BRIGHT_DOWN      (2u << 14)

/* ======================================================================== */
/* 3. 3D [ARM9]                                                             */
/* ======================================================================== */

#define REG_GXFIFO              HW_REG32(0x04000400)
/* Cada comando tem uma porta própria em 0x04000400 + 4*cmd. */
#define GX_CMD_PORT(cmd)        HW_REG32(0x04000400 + 4 * (cmd))

#define GXCMD_MTX_MODE          0x10
#define GXCMD_MTX_PUSH          0x11
#define GXCMD_MTX_POP           0x12
#define GXCMD_MTX_STORE         0x13
#define GXCMD_MTX_RESTORE       0x14
#define GXCMD_MTX_IDENTITY      0x15
#define GXCMD_MTX_LOAD_4x4      0x16
#define GXCMD_MTX_LOAD_4x3      0x17
#define GXCMD_MTX_MULT_4x4      0x18
#define GXCMD_MTX_MULT_4x3      0x19
#define GXCMD_MTX_MULT_3x3      0x1A
#define GXCMD_MTX_SCALE         0x1B
#define GXCMD_MTX_TRANS         0x1C
#define GXCMD_COLOR             0x20
#define GXCMD_NORMAL            0x21
#define GXCMD_TEXCOORD          0x22
#define GXCMD_VTX_16            0x23
#define GXCMD_VTX_10            0x24
#define GXCMD_VTX_XY            0x25
#define GXCMD_VTX_XZ            0x26
#define GXCMD_VTX_YZ            0x27
#define GXCMD_VTX_DIFF          0x28
#define GXCMD_POLYGON_ATTR      0x29
#define GXCMD_TEXIMAGE_PARAM    0x2A
#define GXCMD_PLTT_BASE         0x2B
#define GXCMD_DIF_AMB           0x30
#define GXCMD_SPE_EMI           0x31
#define GXCMD_LIGHT_VECTOR      0x32
#define GXCMD_LIGHT_COLOR       0x33
#define GXCMD_SHININESS         0x34
#define GXCMD_BEGIN_VTXS        0x40
#define GXCMD_END_VTXS          0x41
#define GXCMD_SWAP_BUFFERS      0x50
#define GXCMD_VIEWPORT          0x60
#define GXCMD_BOX_TEST          0x70
#define GXCMD_POS_TEST          0x71
#define GXCMD_VEC_TEST          0x72

/* ======================================================================== */
/* 4. DMA [ambos; bits abaixo valem para o ARM9]                            */
/* ======================================================================== */

#define DMA_COUNT_MASK          0x001FFFFFu   /* ARM9: 21 bits (ARM7: 14 bits em DMA0-2, 16 em DMA3) */
#define DMA_DST_INC             (0u << 21)
#define DMA_DST_DEC             (1u << 21)
#define DMA_DST_FIXED           (2u << 21)
#define DMA_DST_RELOAD          (3u << 21)
#define DMA_SRC_INC             (0u << 23)
#define DMA_SRC_DEC             (1u << 23)
#define DMA_SRC_FIXED           (2u << 23)
#define DMA_REPEAT              (1u << 25)
#define DMA_16BIT               (0u << 26)
#define DMA_32BIT               (1u << 26)
#define DMA_START_NOW           (0u << 27)
#define DMA_START_VBLANK        (1u << 27)
#define DMA_START_HBLANK        (2u << 27)
#define DMA_START_DISPLAY       (3u << 27)
#define DMA_START_MMEM          (4u << 27)
#define DMA_START_CARD          (5u << 27)
#define DMA_START_GXFIFO        (7u << 27)    /* [ARM9] */
#define DMA_IRQ                 (1u << 30)
#define DMA_ENABLE              (1u << 31)

/* Cópia imediata de 'count' palavras de 32 bits (canal 'ch'); bloqueia até terminar. */
static inline void DMA_Copy32(u32 ch, const void *src, void *dst, u32 count)
{
    REG_DMA_CNT(ch) = 0;
    REG_DMA_SRC(ch) = (u32)(unsigned long)src;
    REG_DMA_DST(ch) = (u32)(unsigned long)dst;
    REG_DMA_CNT(ch) = DMA_ENABLE | DMA_32BIT | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC |
                      (count & DMA_COUNT_MASK);
    while (REG_DMA_CNT(ch) & DMA_ENABLE) {
    }
}

/* Cópia imediata de 'count' meias-palavras de 16 bits. */
static inline void DMA_Copy16(u32 ch, const void *src, void *dst, u32 count)
{
    REG_DMA_CNT(ch) = 0;
    REG_DMA_SRC(ch) = (u32)(unsigned long)src;
    REG_DMA_DST(ch) = (u32)(unsigned long)dst;
    REG_DMA_CNT(ch) = DMA_ENABLE | DMA_16BIT | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC |
                      (count & DMA_COUNT_MASK);
    while (REG_DMA_CNT(ch) & DMA_ENABLE) {
    }
}

/* Preenche 'count' palavras de 32 bits com 'value' (origem fixa). */
static inline void DMA_Fill32(u32 ch, u32 value, void *dst, u32 count)
{
    vu32 v = value;

    REG_DMA_CNT(ch) = 0;
    REG_DMA_SRC(ch) = (u32)(unsigned long)&v;
    REG_DMA_DST(ch) = (u32)(unsigned long)dst;
    REG_DMA_CNT(ch) = DMA_ENABLE | DMA_32BIT | DMA_START_NOW | DMA_SRC_FIXED | DMA_DST_INC |
                      (count & DMA_COUNT_MASK);
    while (REG_DMA_CNT(ch) & DMA_ENABLE) {
    }
}

/* ======================================================================== */
/* 5. Timers, energia, SPI, som                                             */
/* ======================================================================== */

/* TMxCNT_H (REG_TM_CTRL(n)) */
#define TM_PRESCALER_1          0u
#define TM_PRESCALER_64         1u
#define TM_PRESCALER_256        2u
#define TM_PRESCALER_1024       3u
#define TM_COUNT_UP             (1u << 2)    /* conta overflow do timer anterior */
#define TM_IRQ                  (1u << 6)
#define TM_ENABLE               (1u << 7)

/* Máscaras compostas de energia [ARM9] (bits individuais em memory_map.h) */
#define POWCNT1_ALL_2D_3D       (POWCNT1_LCD | POWCNT1_2D_A | POWCNT1_2D_B | \
                                 POWCNT1_3D_RENDER | POWCNT1_3D_GEOMETRY)

/* SPI [ARM7]: seleção de dispositivo em REG_SPICNT, bits 8-9 */
#define SPICNT_BAUD_4MHZ        0u
#define SPICNT_BAUD_2MHZ        1u
#define SPICNT_BAUD_1MHZ        2u
#define SPICNT_BAUD_512KHZ      3u
#define SPICNT_DEV_POWER        (0u << 8)    /* chip de gerenciamento de energia */
#define SPICNT_DEV_FIRMWARE     (1u << 8)
#define SPICNT_DEV_TOUCH        (2u << 8)    /* touchscreen/ADC */
#define SPICNT_IRQ              (1u << 14)

/* Chip de energia (via SPI, registrador 0): backlights, desligar */
#define PM_CTRL_SOUND_AMP       (1u << 0)
#define PM_CTRL_SOUND_MUTE      (1u << 1)
#define PM_CTRL_BACKLIGHT_LOW   (1u << 2)    /* tela inferior */
#define PM_CTRL_BACKLIGHT_UP    (1u << 3)    /* tela superior */
#define PM_CTRL_SYSTEM_OFF      (1u << 6)

/* Som [ARM7]: registradores por canal em REG_SOUND_CH_CNT(n) + offset */
#define SOUND_CH_OFS_CNT        0x0          /* u32: volume, pan, formato, repeat, enable (bit 31) */
#define SOUND_CH_OFS_SRC        0x4          /* u32: endereço da amostra */
#define SOUND_CH_OFS_TIMER      0x8          /* u16 */
#define SOUND_CH_OFS_LOOP       0xA          /* u16: ponto de loop */
#define SOUND_CH_OFS_LEN        0xC          /* u32 */
#define REG_SOUND_CH(n, ofs)    HW_REG32(0x04000400 + 0x10 * (n) + (ofs))
#define REG_SOUNDBIAS           HW_REG16(0x04000504)
#define SOUND_CH_ENABLE         (1u << 31)

/* ======================================================================== */
/* 6. Chamadas SWI do BIOS (números; use para reconhecer no desmontado)     */
/* ======================================================================== */
/* No ARM o número vem em "swi N << 16"; no Thumb, "swi N". */

#define BIOS_SWI_INTR_WAIT          0x04
#define BIOS_SWI_VBLANK_INTR_WAIT   0x05
#define BIOS_SWI_HALT               0x06
#define BIOS_SWI_DIV                0x09
#define BIOS_SWI_CPU_SET            0x0B
#define BIOS_SWI_CPU_FAST_SET       0x0C
#define BIOS_SWI_SQRT               0x0D
#define BIOS_SWI_GET_CRC16          0x0E
#define BIOS_SWI_LZ77_UNCOMP_WRAM   0x11
#define BIOS_SWI_LZ77_UNCOMP_VRAM   0x12
#define BIOS_SWI_HUFF_UNCOMP        0x13
#define BIOS_SWI_RL_UNCOMP_WRAM     0x14
#define BIOS_SWI_RL_UNCOMP_VRAM     0x15

/* ======================================================================== */
/* 7. Helpers                                                               */
/* ======================================================================== */

#define ARRAY_COUNT(a)          ((u32)(sizeof(a) / sizeof((a)[0])))
#define ALIGN_UP(x, a)          (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a)        ((x) & ~((a) - 1))
#define BIT(n)                  (1u << (n))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))

/* Desliga IRQs e devolve o IME anterior (para IRQ_Restore). */
static inline u32 IRQ_Disable(void)
{
    u32 old = REG_IME;

    REG_IME = 0;
    return old;
}

static inline void IRQ_Restore(u32 old)
{
    REG_IME = old;
}

/* Teclas pressionadas (REG_KEYINPUT é ativo em 0): devolve máscara com 1 = pressionada. */
static inline u32 Keys_Held(void)
{
    return (~REG_KEYINPUT) & 0x03FFu;
}

#endif /* OPENTS3DS_NDS_H */
