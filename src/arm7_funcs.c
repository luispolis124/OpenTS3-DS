/*
 * arm7_funcs.c - Sub-rotinas auxiliares e de configuração de hardware do ARM7
 * Projeto: The Sims 3 (Nintendo DS)
 */

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

/*
 * fcn_0238018c - Configuração Completa de Registradores de Hardware e Estruturas do ARM7
 */
void fcn_0238018c(void) {
    // 1. Leitura inicial do registrador base de controle
    volatile u32* ptr_base = *(volatile u32**)0x02380258;
    u32 val = *ptr_base;
    
    // Teste de bit (tst r0, #-80000000 equivale a testar o bit mais significativo / sinal)
    if ((val & 0x80000000) == 0) {
        // Se o bit não estiver setado, encerra a rotina (desvia para o fim / bx lr)
        return;
    }
    
    // 2. Modificação de flags de hardware (leitura, alteração por OR e escrita)
    volatile u16* reg_ctrl1 = *(volatile u16**)0x0238025C;
    *reg_ctrl1 = *reg_ctrl1 | 0x0001; // orr r0, r0, 1
    
    volatile u16* reg_ctrl2 = *(volatile u16**)0x02380260;
    *reg_ctrl2 = *reg_ctrl2 | 0x0100; // orr r0, r0, 0x100
    
    // Atualização do registrador base
    val = *ptr_base | 0x00010000;
    *ptr_base = val;
    
    // 3. Salvamento de estado em ponteiros de destino (offset +4)
    volatile u32* dest_ptr = *(volatile u32**)0x02380264;
    dest_ptr[1] = val; // str r0, [r2, 4]
    
    // 4. Operações de manipulação de bytes (ldrb / and / orr com deslocamentos)
    volatile u8* src_b1 = *(volatile u8**)0x02380268;
    u8 val_r0 = *src_b1;
    
    volatile u8* src_b2 = *(volatile u8**)0x0238026C;
    u8 val_r1 = *src_b2;
    u32 ip = val_r1 & 3;
    val_r0 = val_r0 | (ip << 2);
    
    volatile u8* src_b3 = *(volatile u8**)0x02380270;
    val_r1 = *src_b3;
    ip = val_r1 & 6;
    val_r0 = val_r0 | (ip << 4);

    volatile u8* ctrl_b = *(volatile u8**)0x0238025C;
    val_r1 = *ctrl_b;
    ip = val_r1 & 1;
    val_r0 = val_r0 | (ip << 7);
    
    // Recuperação da estrutura de destino e gravação no offset +8 (strb r0, [r2, 8])
    volatile u8* dest_struct = (volatile u8*)dest_ptr;
    dest_struct[8] = val_r0;
    
    // 5. Processamento de halfwords (16 bits) para o offset +9
    volatile u16* src_h1 = *(volatile u16**)0x02380274;
    u16 half_r0 = *src_h1;
    ip = half_r0 & 0x100;
    half_r0 = half_r0 | (ip >> 6);

    volatile u16* src_h2 = *(volatile u16**)0x02380260;
    u16 half_r1 = *src_h2;
    ip = half_r1 & 7;
    half_r0 = half_r0 | (ip << 3);

    ip = half_r1 & 0x180;
    half_r0 = half_r0 | (ip >> 1);

    // Gravação no offset +9 da estrutura (strb r0, [r2, 9])
    dest_struct[9] = (u8)half_r0;

    // 6. Limpeza da flag de status no registrador base (bic r0, r0, -0x80000000)
    u32 base_val = *ptr_base;
    base_val = base_val & ~0x80000000;
    *ptr_base = base_val;
}
