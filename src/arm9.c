/*
 * arm9.c - ponto de entrada do ARM9 e stubs de inicialização.
 *
 * Onde isso se encaixa no binário real (valores de asm/memory_layout.txt):
 *   0x02004000  Secure Area (0x800 bytes)
 *   0x02004800  _start - crt0 em ASSEMBLY (não é C). Ele configura o CP15
 *               (TCM, caches, MPU), copia os autoloads (ITCM 0x01FF8000 e
 *               DTCM 0x02FE0000), zera o .bss estático e só então chama o
 *               "main" em C. O SDK chama esse main de NitroMain/TwlMain; o nome
 *               real neste binário ainda precisa ser confirmado.
 *
 * Este arquivo é um ESQUELETO. As funções abaixo descrevem a sequência típica
 * de um boot de ARM9, não o código original do jogo. Substitua cada stub pelo
 * resultado da decompilação conforme identificar a função correspondente
 * (deixe o endereço original no comentário "TODO(decomp)").
 */

#include "types.h"
#include "memory_map.h"

static vu32 sVBlankCount;

/* ------------------------------------------------------------------------ */
/* IRQ                                                                       */
/* ------------------------------------------------------------------------ */

/* Handler de IRQ de usuário. O BIOS (vetor em 0xFFFF0018) salva o contexto e
 * salta para o ponteiro guardado em ARM9_IRQ_HANDLER_PTR (fim da DTCM). */
void Arm9_IrqHandler(void)
{
    u32 flags = REG_IF;

    if (flags & IRQ_VBLANK) {
        sVBlankCount++;
    }

    /* O BIOS (IntrWait/VBlankIntrWait) acorda olhando estas flags. */
    HW_REG32(ARM9_IRQ_CHECK_FLAGS) |= flags;

    REG_IF = flags; /* escrever 1 reconhece a interrupção */
}

static void Arm9_InitInterrupts(void)
{
    REG_IME = 0;
    REG_IE = 0;
    REG_IF = 0xFFFFFFFFu;

    HW_REG32(ARM9_IRQ_HANDLER_PTR) = (u32)(unsigned long)&Arm9_IrqHandler;

    REG_DISPSTAT |= DISPSTAT_VBLANK_IRQ;
    REG_IE = IRQ_VBLANK | IRQ_IPC_RECV_NOT_EMPTY;
    REG_IME = 1;
}

/* ------------------------------------------------------------------------ */
/* Hardware                                                                  */
/* ------------------------------------------------------------------------ */

/* O crt0 faz isto em assembly; aqui só para documentar o intervalo. */
static void Arm9_ClearStaticBss(void)
{
    u32 *p;

    for (p = (u32 *)ARM9_STATIC_BSS_START; p < (u32 *)ARM9_STATIC_BSS_END; p++) {
        *p = 0;
    }
}

static void Arm9_InitPower(void)
{
    /* TODO(decomp): confirmar quais engines o jogo liga no boot. */
    REG_POWCNT1 = POWCNT1_LCD | POWCNT1_2D_A | POWCNT1_2D_B |
                  POWCNT1_3D_RENDER | POWCNT1_3D_GEOMETRY;
}

static void Arm9_InitVideo(void)
{
    REG_DISPCNT = 0;
    REG_DISPCNT_B = 0;
    REG_MASTER_BRIGHT = 0;
    REG_MASTER_BRIGHT_B = 0;

    /* TODO(decomp): mapeamento real dos bancos de VRAM (A..I). Desligados por ora. */
    REG_VRAMCNT_A = 0;
    REG_VRAMCNT_B = 0;
    REG_VRAMCNT_C = 0;
    REG_VRAMCNT_D = 0;
    REG_VRAMCNT_E = 0;
    REG_VRAMCNT_F = 0;
    REG_VRAMCNT_G = 0;
    REG_VRAMCNT_H = 0;
    REG_VRAMCNT_I = 0;
}

static void Arm9_InitIpc(void)
{
    REG_IPCSYNC = 0;
    REG_IPCFIFOCNT = IPCFIFO_ENABLE | IPCFIFO_SEND_CLEAR;
}

void Arm9_InitHardware(void)
{
    Arm9_InitPower();
    Arm9_InitVideo();
    Arm9_InitIpc();
    Arm9_InitInterrupts();
}

/* ------------------------------------------------------------------------ */
/* Laço principal                                                            */
/* ------------------------------------------------------------------------ */

static void Arm9_WaitVBlank(void)
{
    u32 start = sVBlankCount;

    while (sVBlankCount == start) {
        /* TODO(decomp): o jogo provavelmente usa o BIOS (halt) aqui. */
    }
}

/* TODO(decomp): renomear para o símbolo real (NitroMain / TwlMain / outro). */
void Arm9_Main(void)
{
    Arm9_InitHardware();

    for (;;) {
        Arm9_WaitVBlank();

        /* TODO(decomp): ler teclas/toque, atualizar o mundo, montar a cena 3D,
         * trocar overlays (ARM9_OVERLAY_*), tratar mensagens do ARM7 via FIFO. */
    }
}
