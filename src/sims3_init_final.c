/*
 * sims3_init_final.c - Conclusão do Boot e Salto Final para o ARM7 (The Sims 3 DS)
 */

typedef int s32;
typedef unsigned int u32;

void arm7_startup_phase3(void) {
    // 1. Conclusão da gravação iniciada no bloco anterior
    volatile u32* final_r1 = *(volatile u32**)0x02380174;
    volatile u32* final_r0 = *(volatile u32**)0x02380178;
    *final_r1 = (u32)final_r0; // str r0, [r1]

    // 2. Preparação dos argumentos na pilha para a rotina principal
    volatile u32* table_ptr = *(volatile u32**)0x0238017C; // aav.aav.0x023805b4
    
    u32 arg_val1 = *table_ptr;            // ldr r0, [r1], 4 (e avança o ponteiro)
    u32 arg_val2 = *(table_ptr + 1);      // ldr r0, [r1]

    // 3. Configuração do salto final e chamada de transição
    u32 target_func = *(u32*)0x02380180;
    u32 param_val   = *(u32*)0x02380184;
    u32 link_reg    = *(u32*)0x02380188;

    // Tradução fiel da montagem final de transição e salto via assembly inline
    __asm {
        mov r0, arg_val1
        mov r1, arg_val2
        mov r2, #4
        mov lr, link_reg
        
        // Empilhamento dos argumentos auxiliares e ajuste de ponteiro de pilha
        sub sp, sp, #4
        str r0, [sp, #-4]
        str r1, [sp]
        
        // Salto dinâmico para a função de destino mapeada em target_func
        mov r3, target_func
        sub r3, sp, #4
        bx r3
    }
}
