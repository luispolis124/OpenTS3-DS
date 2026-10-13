/*
 * arm7_funcs.c - Sub-rotinas auxiliares e de configuração de hardware do ARM7
 * Projeto: The Sims 3 (Nintendo DS)
 */

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

// Protótipos das funções auxiliares chamadas pelas rotinas de inicialização
void fcn_02380508(u32 src, u32 dest, u32 size);
void fcn_02380554(u32 arg0, u32 arg1);

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

/*
 * fcn_02380278 - Processamento de Tabelas e Inicialização de Blocos do ARM7
 */
void fcn_02380278(void) {
    // 1. Acesso à estrutura base de controle (aav.0x023805d4)
    volatile u32** struct_base_ptr = *(volatile u32***)0x023805D4;
    u32* struct_ptr = (u32*)struct_base_ptr;

    u32 ip = struct_ptr[0];       // Ponteiro atual (offset 0)
    u32 limit = struct_ptr[1];     // Limite final (offset 4)
    u32 r1_val = struct_ptr[2];    // Acumulador/Base (offset 8)

    // 2. Loop de varredura e processamento de blocos
    while ((s32)ip < (s32)limit) {
        // Leitura de r0 com pós-incremento de 4 bytes (ldr r0, [ip], 4)
        u32 r0 = *(u32*)ip;
        ip += 4;

        u32 special_val = *(u32*)0x02380358;
        if (r0 == special_val) {
            // Se for igual ao valor especial, avança o ponteiro em 0xC (12 bytes) e ajusta r1
            u32 r2_skip = *(u32*)ip;
            ip += 12;
            r1_val += r2_skip;
            continue;
        }

        // Leitura do próximo argumento e chamada de fcn_02380508
        u32 arg_second = *(u32*)ip;
        ip += 4;
        
        fcn_02380508(r0, arg_second, 4);

        // Leitura dos próximos ponteiros para a rotina de preenchimento
        u32 arg_third = *(u32*)ip;
        ip += 4;
        
        u32 arg_fourth = *(u32*)ip;
        ip += 4;

        fcn_02380554(arg_third, arg_fourth);
    }

    // Atualiza de volta os ponteiros na estrutura
    struct_ptr[0] = ip;
    struct_ptr[2] = r1_val;
}
