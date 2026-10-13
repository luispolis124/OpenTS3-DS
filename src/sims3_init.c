/*
 * sims3_init.c - Rotina de Inicialização (Entrypoint) para The Sims 3 (Nintendo DS)
 */

typedef int s32;
typedef unsigned int u32;

void arm9_entry(void) {
    volatile u32* io_base;
    volatile u32* src_ptr;
    volatile u32* dest_ptr;
    s32 block_size;

    // 1. Configuração inicial de hardware (I/O base 0x04000000)
    io_base = (volatile u32*)0x04000000;
    io_base[0x208 / 4] = (u32)io_base;

    // 2. Cópia do bloco de inicialização de dados da ROM para a RAM
    src_ptr = *(volatile u32**)0x023805F8;
    dest_ptr = *(volatile u32**)0x02380138;
    block_size = (s32)dest_ptr + 0x28;

    do {
        u32 temp = *src_ptr;
        src_ptr = (volatile u32*)((u32)src_ptr + 4);
        *dest_ptr = temp;
        dest_ptr = (volatile u32*)((u32)dest_ptr + 4);
    } while ((s32)dest_ptr < block_size);

    // 3. Chamada de função ponteiro armazenada
    {
        u32 fn_target = *(u32*)0x02380138;
        // Salva o retorno e executa a rotina via ponteiro
        // (No assembly original: mov lr, pc; bx r2)
        __asm {
            mov r2, fn_target
            mov lr, pc
            bx r2
        }
    }

    // 4. Configuração dos modos de exceção e pilhas do processador ARM (CPSR)
    // Modo IRQ (Interrupt Request)
    __asm {
        mov r0, #0x13
        msr cpsr_c, r0
        ldr sp, =0x03004000
    }

    // Modo Supervisor / System
    __asm {
        mov r0, #0x12
        msr cpsr_c, r0
        ldr sp, =0x03003000
    }
}
