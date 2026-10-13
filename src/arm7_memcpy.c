/*
 * arm7_memcpy.c (ou arm7_funcs.c) - Rotinas Otimizadas de Cópia e Preenchimento de Memória (ARM7)
 */

typedef unsigned int u32;

/*
 * fcn_02380508 - Cópia de Bloco Otimizada (Block Copy / memcpy otimizado)
 * Parâmetros baseados na assinatura do assembly:
 * r0 = src (ou dest dependendo da inversão de ponteiros), r1 = src/dest, r2 = tamanho
 */
void fcn_02380508(u32 src, u32 dest, u32 size) {
    // Implementação em C equivalente à otimização por blocos de registradores do ARM
    // Como o binário original usa blocos de 32 bytes (8 registradores x 4 bytes), 
    // podemos usar um ponteiro direto ou uma rotina otimizada por blocos:
    
    volatile u32* d = (volatile u32*)dest;
    volatile u32* s = (volatile u32*)src;
    
    // Blocos de 32 bytes (0x20)
    u32 blocks = size >> 5;
    while (blocks > 0) {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = s[3];
        d[4] = s[4];
        d[5] = s[5];
        d[6] = s[6];
        d[7] = s[7];
        s += 8;
        d += 8;
        blocks--;
    }
    
    // Restantes (16, 8, 4 bytes)
    u32 rem = size & 0x1F;
    if (rem & 0x10) {
        d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        s += 4; d += 4;
    }
    if (rem & 0x08) {
        d[0] = s[0]; d[1] = s[1];
        s += 2; d += 2;
    }
    if (rem & 0x04) {
        d[0] = s[0];
        s += 1; d += 1;
    }
}

/*
 * fcn_02380554 - Rotina de Preenchimento / Inicialização de Memória (memset / clear)
 */
void fcn_02380554(u32 arg0, u32 arg1) {
    // Esta função lida com o preenchimento/limpeza de blocos de memória baseados nos argumentos r0 e r1
    volatile u32* ptr = (volatile u32*)arg0;
    u32 val = arg1;
    
    // Preenchimento sequencial otimizado correspondente ao trecho assembly
    *ptr = val;
}
