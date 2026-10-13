#ifndef OPENTS3DS_MEMORY_MAP_H
#define OPENTS3DS_MEMORY_MAP_H

#include "types.h"

/*
 * memory_map.h - mapa de memória e registradores do Nintendo DS / DSi.
 *
 * Seções:
 *   1. Mapa de endereços do hardware (comum a todo jogo de DS).
 *   2. Layout desta ROM (The Sims 3, USA, VS3E), extraído do próprio binário por
 *      tools/extract_code.py -> asm/memory_layout.txt. Se algum valor divergir de lá,
 *      o layout.txt é a fonte da verdade.
 *   3. Registradores de hardware. Marcados [ARM9], [ARM7] ou [ambos].
 *
 * Os registradores e bits da seção 3 seguem a documentação pública do hardware (GBATEK).
 * Confira contra ela antes de depender de um bit específico.
 */

/* ======================================================================== */
/* 1. Mapa de endereços do hardware                                         */
/* ======================================================================== */

#define HW_MAIN_RAM             0x02000000   /* RAM principal, vista pelos dois CPUs */
#define HW_MAIN_RAM_SIZE_NTR    0x00400000   /*   4 MB no DS / DS Lite                */
#define HW_MAIN_RAM_SIZE_TWL    0x01000000   /*  16 MB no modo DSi                    */

#define HW_SHARED_WRAM          0x03000000   /* 32 KB, repartida entre ARM9/ARM7 (WRAMCNT) */
#define HW_SHARED_WRAM_SIZE     0x00008000
#define HW_ARM7_WRAM            0x03800000   /* 64 KB, só ARM7 */
#define HW_ARM7_WRAM_SIZE       0x00010000

#define HW_IO_REGS              0x04000000
#define HW_IPC_FIFO_RECV_ADDR   0x04100000
#define HW_SCFG_BASE            0x04004000   /* [DSi] registradores SCFG */
#define HW_WIFI_BASE            0x04800000   /* [ARM7] */

#define HW_PLTT_BG_MAIN         0x05000000   /* 0x200 bytes */
#define HW_PLTT_OBJ_MAIN        0x05000200
#define HW_PLTT_BG_SUB          0x05000400
#define HW_PLTT_OBJ_SUB         0x05000600

#define HW_BG_VRAM_MAIN         0x06000000
#define HW_BG_VRAM_SUB          0x06200000
#define HW_OBJ_VRAM_MAIN        0x06400000
#define HW_OBJ_VRAM_SUB         0x06600000
#define HW_LCDC_VRAM_A          0x06800000   /* 128 KB */
#define HW_LCDC_VRAM_B          0x06820000   /* 128 KB */
#define HW_LCDC_VRAM_C          0x06840000   /* 128 KB */
#define HW_LCDC_VRAM_D          0x06860000   /* 128 KB */
#define HW_LCDC_VRAM_E          0x06880000   /*  64 KB */
#define HW_LCDC_VRAM_F          0x06890000   /*  16 KB */
#define HW_LCDC_VRAM_G          0x06894000   /*  16 KB */
#define HW_LCDC_VRAM_H          0x06898000   /*  32 KB */
#define HW_LCDC_VRAM_I          0x068A0000   /*  16 KB */

#define HW_OAM_MAIN             0x07000000   /* 1 KB */
#define HW_OAM_SUB              0x07000400

#define HW_GBA_SLOT_ROM         0x08000000
#define HW_GBA_SLOT_RAM         0x0A000000
#define HW_ARM9_BIOS            0xFFFF0000   /* [ARM9] vetores de exceção em 0xFFFF0000 */

/* ---- TCM do ARM9 (ARM946E-S) ------------------------------------------- */
/* ITCM: 32 KB, base fixa em 0 e espelhada de 32 em 32 KB; o SDK usa o último
 *       espelho (0x01FF8000). DTCM: 16 KB, base configurável via CP15. */
#define HW_ITCM_BASE            0x01FF8000
#define HW_ITCM_SIZE            0x00008000
#define HW_DTCM_BASE_NITRO      0x027E0000   /* padrão do NitroSDK */
#define HW_DTCM_BASE_TWL        0x02FE0000   /* padrão do TWL-SDK (RAM de 16 MB) */
#define HW_DTCM_SIZE            0x00004000

/* CP15 c1,c0,0 (registrador de controle) */
#define CP15_CTRL_DTCM_ENABLE   (1u << 16)
#define CP15_CTRL_ITCM_ENABLE   (1u << 18)

/* ======================================================================== */
/* 2. Layout desta ROM (VS3E)                                               */
/* ======================================================================== */

/* ---- ARM9 ----------------------------------------------------------------
 * ROM offset 0x4000, 0x17C6C0 bytes, não comprimido.
 * O binário na ROM é: [static 0x178AE0][autoload ITCM 0x3AC0][autoload DTCM 0x100][lista 0x20]
 */
#define ARM9_LOAD_ADDR          0x02004000   /* inicio da Secure Area (já descriptografada) */
#define ARM9_SECURE_AREA        0x02004000
#define ARM9_SECURE_AREA_SIZE   0x00000800
#define ARM9_ENTRY              0x02004800   /* _start (crt0, em assembly) */
#define ARM9_MODULE_PARAMS      0x02004FB0   /* ModuleParams do SDK (sdk_version = 0x05047531) */

#define ARM9_STATIC_END         0x0217CAE0   /* fim do código/dados estáticos em RAM  */
#define ARM9_STATIC_BSS_START   0x0217CAE0
#define ARM9_STATIC_BSS_END     0x022503A0   /* 0xD38C0 bytes de .bss; zerados no boot */
#define ARM9_AUTOLOAD_LIST      0x021806A0   /* 2 entradas de 16 bytes */

/* Autoload 0 -> ITCM (código quente copiado da ROM no boot) */
#define ARM9_ITCM_ADDR          HW_ITCM_BASE /* 0x01FF8000 */
#define ARM9_ITCM_DATA_SIZE     0x00003AC0
#define ARM9_ITCM_BSS_SIZE      0x00000020

/* Autoload 1 -> DTCM. Este jogo usa a base do TWL-SDK, não a do NitroSDK. */
#define ARM9_DTCM_ADDR          HW_DTCM_BASE_TWL /* 0x02FE0000 */
#define ARM9_DTCM_DATA_SIZE     0x00000100
#define ARM9_DTCM_BSS_SIZE      0x00000E00

/* Vetor de IRQ e flags de verificação do BIOS ficam no fim da DTCM */
#define ARM9_IRQ_CHECK_FLAGS    (ARM9_DTCM_ADDR + 0x3FF8)
#define ARM9_IRQ_HANDLER_PTR    (ARM9_DTCM_ADDR + 0x3FFC)

/* Overlays ARM9 (FAT ids 0..2; nenhum comprimido). Os overlays 1 e 2 compartilham
 * o mesmo endereço: só um deles fica carregado por vez. */
#define ARM9_OVERLAY_0_ADDR     0x022503A0   /* logo após o .bss estático */
#define ARM9_OVERLAY_0_SIZE     0x0000EA60
#define ARM9_OVERLAY_0_BSS      0x00000020
#define ARM9_OVERLAY_1_ADDR     0x0225EE20
#define ARM9_OVERLAY_1_SIZE     0x000257A0
#define ARM9_OVERLAY_1_BSS      0x00000000
#define ARM9_OVERLAY_2_ADDR     0x0225EE20
#define ARM9_OVERLAY_2_SIZE     0x00001BE0
#define ARM9_OVERLAY_2_BSS      0x00000020

/* ---- ARM7 ----------------------------------------------------------------
 * ROM offset 0x1B6A00, 0x2909C bytes, não comprimido. Carregado na RAM principal.
 * Binário: [static 0x644][autoload WRAM 0x1152C][autoload main-RAM 0x1750C][lista 0x20]
 */
#define ARM7_LOAD_ADDR          0x02380000
#define ARM7_ENTRY              0x02380000
#define ARM7_MODULE_PARAMS      0x023805D4
#define ARM7_WRAM_CODE_ADDR     0x037F8000   /* autoload 0: 0x1152C bytes + 0x2954 de bss */
#define ARM7_WRAM_CODE_SIZE     0x0001152C
#define ARM7_WRAM_BSS_SIZE      0x00002954
#define ARM7_HIGH_CODE_ADDR     0x02FE0000   /* autoload 1: 0x1750C bytes + 0x28F8 de bss */
#define ARM7_HIGH_CODE_SIZE     0x0001750C
#define ARM7_HIGH_BSS_SIZE      0x000028F8

#define ARM7_IRQ_CHECK_FLAGS    0x0380FFF8
#define ARM7_IRQ_HANDLER_PTR    0x0380FFFC

/* ---- Binários DSi (jogo "NDSi Enhanced") ---------------------------------- */
#define ARM9I_LOAD_ADDR         0x02400000   /* 0x2C74 bytes; criptografado (modcrypt) na ROM */
#define ARM7I_LOAD_ADDR         0x02E80000   /* 0x471A4 bytes */

/* ======================================================================== */
/* 3. Registradores de hardware                                             */
/* ======================================================================== */

#define HW_REG8(addr)           (*(vu8  *)(unsigned long)(addr))
#define HW_REG16(addr)          (*(vu16 *)(unsigned long)(addr))
#define HW_REG32(addr)          (*(vu32 *)(unsigned long)(addr))

/* ---- Vídeo 2D [ARM9] ---------------------------------------------------- */
#define ENGINE_B_OFFSET         0x1000       /* registradores da tela inferior = principal + 0x1000 */
#define REG_DISPCNT             HW_REG32(0x04000000)
#define REG_DISPCNT_B           HW_REG32(0x04001000)
#define REG_DISPSTAT            HW_REG16(0x04000004) /* [ambos] */
#define REG_VCOUNT              HW_REG16(0x04000006) /* [ambos] */
#define REG_BGCNT(n)            HW_REG16(0x04000008 + 2 * (n))
#define REG_BGHOFS(n)           HW_REG16(0x04000010 + 4 * (n))
#define REG_BGVOFS(n)           HW_REG16(0x04000012 + 4 * (n))
#define REG_MASTER_BRIGHT       HW_REG16(0x0400006C)
#define REG_MASTER_BRIGHT_B     HW_REG16(0x0400106C)

#define DISPSTAT_VBLANK_FLAG    (1u << 0)
#define DISPSTAT_HBLANK_FLAG    (1u << 1)
#define DISPSTAT_VCOUNT_FLAG    (1u << 2)
#define DISPSTAT_VBLANK_IRQ     (1u << 3)
#define DISPSTAT_HBLANK_IRQ     (1u << 4)
#define DISPSTAT_VCOUNT_IRQ     (1u << 5)

#define DISPCNT_BG0_ON          (1u << 8)
#define DISPCNT_BG1_ON          (1u << 9)
#define DISPCNT_BG2_ON          (1u << 10)
#define DISPCNT_BG3_ON          (1u << 11)
#define DISPCNT_OBJ_ON          (1u << 12)

/* ---- Energia [ARM9] ----------------------------------------------------- */
#define REG_POWCNT1             HW_REG16(0x04000304)
#define POWCNT1_LCD             (1u << 0)
#define POWCNT1_2D_A            (1u << 1)
#define POWCNT1_3D_RENDER       (1u << 2)
#define POWCNT1_3D_GEOMETRY     (1u << 3)
#define POWCNT1_2D_B            (1u << 9)
#define POWCNT1_SWAP_SCREENS    (1u << 15)

/* ---- VRAM / WRAM [ARM9] ------------------------------------------------- */
#define REG_VRAMCNT_A           HW_REG8(0x04000240)
#define REG_VRAMCNT_B           HW_REG8(0x04000241)
#define REG_VRAMCNT_C           HW_REG8(0x04000242)
#define REG_VRAMCNT_D           HW_REG8(0x04000243)
#define REG_VRAMCNT_E           HW_REG8(0x04000244)
#define REG_VRAMCNT_F           HW_REG8(0x04000245)
#define REG_VRAMCNT_G           HW_REG8(0x04000246)
#define REG_WRAMCNT             HW_REG8(0x04000247)
#define REG_VRAMCNT_H           HW_REG8(0x04000248)
#define REG_VRAMCNT_I           HW_REG8(0x04000249)
#define VRAMCNT_ENABLE          0x80

/* ---- 3D [ARM9] ---------------------------------------------------------- */
#define REG_DISP3DCNT           HW_REG16(0x04000060)
#define REG_GXSTAT              HW_REG32(0x04000600)

/* ---- Interrupções [ambos] ------------------------------------------------ */
#define REG_IME                 HW_REG32(0x04000208)
#define REG_IE                  HW_REG32(0x04000210)
#define REG_IF                  HW_REG32(0x04000214)

#define IRQ_VBLANK              (1u << 0)
#define IRQ_HBLANK              (1u << 1)
#define IRQ_VCOUNT              (1u << 2)
#define IRQ_TIMER(n)            (1u << (3 + (n)))
#define IRQ_DMA(n)              (1u << (8 + (n)))
#define IRQ_KEYPAD              (1u << 12)
#define IRQ_GBA_SLOT            (1u << 13)
#define IRQ_IPC_SYNC            (1u << 16)
#define IRQ_IPC_SEND_EMPTY      (1u << 17)
#define IRQ_IPC_RECV_NOT_EMPTY  (1u << 18)
#define IRQ_CARD_DONE           (1u << 19)
#define IRQ_CARD_IREQ           (1u << 20)
#define IRQ_GEOMETRY_FIFO       (1u << 21)  /* [ARM9] */
#define IRQ_LID_OPEN            (1u << 22)  /* [ARM7] */
#define IRQ_SPI                 (1u << 23)  /* [ARM7] */
#define IRQ_WIFI                (1u << 24)  /* [ARM7] */

/* ---- Timers e DMA [ambos] ------------------------------------------------ */
#define REG_TM_COUNT(n)         HW_REG16(0x04000100 + 4 * (n))
#define REG_TM_CTRL(n)          HW_REG16(0x04000102 + 4 * (n))
#define REG_DMA_SRC(n)          HW_REG32(0x040000B0 + 12 * (n))
#define REG_DMA_DST(n)          HW_REG32(0x040000B4 + 12 * (n))
#define REG_DMA_CNT(n)          HW_REG32(0x040000B8 + 12 * (n))

/* ---- Teclas [ambos] / extras [ARM7] ------------------------------------- */
#define REG_KEYINPUT            HW_REG16(0x04000130)   /* 0 = pressionada */
#define REG_KEYCNT              HW_REG16(0x04000132)
#define REG_EXTKEYIN            HW_REG16(0x04000136)   /* [ARM7] X, Y, toque, tampa */

#define KEY_A                   (1u << 0)
#define KEY_B                   (1u << 1)
#define KEY_SELECT              (1u << 2)
#define KEY_START               (1u << 3)
#define KEY_RIGHT               (1u << 4)
#define KEY_LEFT                (1u << 5)
#define KEY_UP                  (1u << 6)
#define KEY_DOWN                (1u << 7)
#define KEY_R                   (1u << 8)
#define KEY_L                   (1u << 9)
#define EXTKEY_X                (1u << 0)
#define EXTKEY_Y                (1u << 1)
#define EXTKEY_PEN_DOWN         (1u << 6)
#define EXTKEY_HINGE_CLOSED     (1u << 7)

/* ---- Comunicação entre os CPUs (IPC) [ambos] ----------------------------- */
#define REG_IPCSYNC             HW_REG16(0x04000180)
#define REG_IPCFIFOCNT          HW_REG16(0x04000184)
#define REG_IPCFIFOSEND         HW_REG32(0x04000188)
#define REG_IPCFIFORECV         HW_REG32(HW_IPC_FIFO_RECV_ADDR)

#define IPCSYNC_RECV_MASK       0x000Fu             /* dado enviado pelo outro CPU */
#define IPCSYNC_SEND(v)         (((u16)(v) & 0xFu) << 8)
#define IPCSYNC_TRIGGER_IRQ     (1u << 13)
#define IPCSYNC_IRQ_ENABLE      (1u << 14)

#define IPCFIFO_SEND_EMPTY      (1u << 0)
#define IPCFIFO_SEND_FULL       (1u << 1)
#define IPCFIFO_SEND_IRQ        (1u << 2)
#define IPCFIFO_SEND_CLEAR      (1u << 3)
#define IPCFIFO_RECV_EMPTY      (1u << 8)
#define IPCFIFO_RECV_FULL       (1u << 9)
#define IPCFIFO_RECV_IRQ        (1u << 10)
#define IPCFIFO_ERROR           (1u << 14)
#define IPCFIFO_ENABLE          (1u << 15)

#define REG_EXMEMCNT            HW_REG16(0x04000204)   /* [ambos] */

/* ---- Cartão / SPI / RTC [ARM7] ------------------------------------------ */
#define REG_RTC                 HW_REG16(0x04000138)
#define REG_SPICNT              HW_REG16(0x040001C0)
#define REG_SPIDATA             HW_REG16(0x040001C2)
#define SPICNT_ENABLE           (1u << 15)
#define SPICNT_HOLD             (1u << 11)             /* mantém CS ativo entre bytes */

/* ---- Energia [ARM7]: mesmo endereço do POWCNT1 do ARM9, outro significado --- */
#define REG_POWCNT2             HW_REG16(0x04000304)
#define POWCNT2_SOUND           (1u << 0)
#define POWCNT2_WIFI            (1u << 1)

/* ---- Som [ARM7] --------------------------------------------------------- */
#define REG_SOUNDCNT            HW_REG16(0x04000500)
#define SOUNDCNT_ENABLE         (1u << 15)
#define REG_SOUND_CH_CNT(n)     HW_REG32(0x04000400 + 0x10 * (n))

#endif /* OPENTS3DS_MEMORY_MAP_H */
