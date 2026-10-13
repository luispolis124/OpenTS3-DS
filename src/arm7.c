/*
 * arm7.c - ponto de entrada do ARM7 e stubs de inicialização.
 *
 * Layout no binário real (valores de asm/memory_layout.txt):
 *   0x02380000  _start (crt0, ASSEMBLY): carrega os autoloads e chama o main em C.
 *   0x037F8000  autoload 0 -> WRAM do ARM7: 0x1152C bytes de código + 0x2954 de .bss
 *   0x02FE0000  autoload 1 -> RAM principal: 0x1750C bytes + 0x28F8 de .bss
 *
 * O ARM7 cuida do que o ARM9 não alcança: som, toque (SPI), RTC, Wi-Fi, botões
 * X/Y/tampa e o cartão. Ele conversa com o ARM9 por IPC (sync + FIFO).
 *
 * ESQUELETO: descreve a estrutura típica de um ARM7 de DS, não o código original.
 * Substitua cada stub pela decompilação (anote o endereço em "TODO(decomp)").
 * O SDK chama o main do ARM7 de NitroSpMain/TwlSpMain; confirmar o nome real.
 */

#include "types.h"
#include "memory_map.h"

static vu32 sVBlankCount;

/* ------------------------------------------------------------------------ */
/* IRQ                                                                       */
/* ------------------------------------------------------------------------ */

void Arm7_IrqHandler(void)
{
    u32 flags = REG_IF;

    if (flags & IRQ_VBLANK) {
        sVBlankCount++;
    }

    HW_REG32(ARM7_IRQ_CHECK_FLAGS) |= flags;
    REG_IF = flags;
}

static void Arm7_InitInterrupts(void)
{
    REG_IME = 0;
    REG_IE = 0;
    REG_IF = 0xFFFFFFFFu;

    HW_REG32(ARM7_IRQ_HANDLER_PTR) = (u32)(unsigned long)&Arm7_IrqHandler;

    REG_DISPSTAT |= DISPSTAT_VBLANK_IRQ;
    REG_IE = IRQ_VBLANK | IRQ_IPC_RECV_NOT_EMPTY;
    REG_IME = 1;
}

/* ------------------------------------------------------------------------ */
/* Hardware                                                                  */
/* ------------------------------------------------------------------------ */

static void Arm7_InitPower(void)
{
    /* TODO(decomp): confirmar o que é ligado no boot (Wi-Fi costuma ser sob demanda). */
    REG_POWCNT2 = POWCNT2_SOUND;
}

static void Arm7_InitIpc(void)
{
    REG_IPCSYNC = 0;
    REG_IPCFIFOCNT = IPCFIFO_ENABLE | IPCFIFO_SEND_CLEAR;
}

static void Arm7_InitSound(void)
{
    u32 i;

    REG_SOUNDCNT = SOUNDCNT_ENABLE;
    for (i = 0; i < 16; i++) {
        REG_SOUND_CH_CNT(i) = 0; /* desliga todos os 16 canais */
    }
    /* TODO(decomp): o jogo usa o mixer do SDK (SND) ou o seu próprio? */
}

static void Arm7_InitTouch(void)
{
    REG_SPICNT = SPICNT_ENABLE;
    /* TODO(decomp): protocolo do controlador de toque/ADC (SPI). */
}

static void Arm7_InitRtc(void)
{
    /* TODO(decomp): leitura/escrita do relógio (REG_RTC). O Sims usa a hora real? */
}

void Arm7_InitHardware(void)
{
    Arm7_InitPower();
    Arm7_InitIpc();
    Arm7_InitSound();
    Arm7_InitTouch();
    Arm7_InitRtc();
    Arm7_InitInterrupts();
}

/* ------------------------------------------------------------------------ */
/* IPC com o ARM9                                                            */
/* ------------------------------------------------------------------------ */

static void Arm7_HandleCommand(u32 message)
{
    (void)message;
    /* TODO(decomp): tabela de comandos enviados pelo ARM9 (som, toque, RTC, wifi...). */
}

static void Arm7_PollIpc(void)
{
    while (!(REG_IPCFIFOCNT & IPCFIFO_RECV_EMPTY)) {
        Arm7_HandleCommand(REG_IPCFIFORECV);
    }
}

/* ------------------------------------------------------------------------ */
/* Laço principal                                                            */
/* ------------------------------------------------------------------------ */

static void Arm7_WaitVBlank(void)
{
    u32 start = sVBlankCount;

    while (sVBlankCount == start) {
        Arm7_PollIpc();
    }
}

/* TODO(decomp): renomear para o símbolo real (NitroSpMain / TwlSpMain / outro). */
void Arm7_Main(void)
{
    Arm7_InitHardware();

    for (;;) {
        Arm7_WaitVBlank();

        /* TODO(decomp): por quadro, amostrar toque e teclas (EXTKEYIN), atualizar o
         * som, tratar o RTC e enviar o estado de volta ao ARM9. */
    }
}
