/*
 * fcn_0238018c - Configuração de Registradores de Hardware do ARM7
 */

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

void fcn_0238018c(void) {
    // 1. Leitura inicial do registrador base de controle
    volatile u32* ptr_base = *(volatile u32**)0x02380258;
    u32 val = *ptr_base;
    
    // Teste de bit (tst r0, #-80000000 equivale a testar o bit mais significativo / sinal)
    if ((val & 0x80000000) == 0) {
        // Se o bit não estiver setado, desvia para o fim (0x2380254)
        // (Aqui entrará o restante da função que veremos a seguir)
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
    
    // 3. Salvamento de estado em ponteiros de destino
    volatile u32* dest_ptr = *(volatile u32**)0x02380264;
    dest_ptr[1] = val; // str r0, [r2, 4] (equivalente ao offset de 4 bytes)
    
    // 4. Operações de manipulação de bytes (ldrb / and / orr com deslocamentos)
    volatile u8* src_b1 = *(volatile u8**)0x02380268;
    u8 b1 = *src_b1;
    
    volatile u8* src_b2 = *(volatile u8**)0x0238026C;
    u8 b2 = *src_b2;
    
    u32 combined = b1 | ((b2 & 0x03) << 2);
    
    volatile u8* src_b3 = *(volatile u8**)0x02380270;
    u8 b3 = *src_b3;
    
    combined = combined | ((b3 & 0x06) << 4);
    
    // (A continuação desta função processa mais registradores logo abaixo...)
}
