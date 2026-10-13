/*
 * sims3_init_ext.c - Continuação da Inicialização do ARM7 para The Sims 3 (Nintendo DS)
 */

typedef int s32;
typedef unsigned int u32;

// Protótipos das funções auxiliares chamadas no bloco
void fcn_0238018c(void);
void fcn_02380400(void);
s32  fcn_023803dc(void);
void fcn_02380508(u32 arg0, u32 arg1, u32 arg2);
void fcn_02380278(void);
void fcn_02380554(u32 arg0, u32 arg1);
void fcn_02380424(void);

void arm7_startup_phase2(void) {
    // 1. Chamadas iniciais de subsistemas
    fcn_0238018c();
    fcn_02380400();
    
    // 2. Verificação condicional baseada no resultado de fcn_023803dc()
    s32 cond_res = fcn_023803dc();
    
    if (cond_res != 0) {
        // Ramo BNE (Branch if Not Equal / Não Zero)
        volatile u32* ptr_val = *(volatile u32**)0x0238015C;
        u32 val = *(u16*)ptr_val; // ldrh
        
        if (val == 2) {
            u32 arg_a = *(u32*)0x02380160;
            u32 arg_b = *(u32*)0x02380164;
            fcn_02380508(arg_a, arg_b, 0x160);
            
            arg_a = *(u32*)0x02380168;
            arg_b = *(u32*)0x0238016C;
            fcn_02380508(arg_a, arg_b, 0x20);
        }
    } else {
        // Ramo BEQ (Equal / Zero)
        volatile u32** target_ptr = *(volatile u32***)0x02380154;
        u32 target_val = **target_ptr;
        
        if (target_val == 0) {
            u32 default_val = *(u32*)0x02380158;
            **target_ptr = default_val;
        }
    }
    
    // 3. Chamadas de sincronização e gerenciamento de memória subsequentes
    fcn_02380278();
    
    volatile u32* struct_base = *(volatile u32**)0x02380170; // aav.aav.0x023805d4
    u32 val_c = struct_base[0xC / 4];
    u32 val_10 = struct_base[0x10 / 4];
    
    s32 diff = (s32)val_10 - (s32)val_c;
    if (diff > 0) {
        fcn_02380554(val_c, val_10);
    }
    
    fcn_02380424();
    
    // Carregamento dos ponteiros finais do bloco
    volatile u32* final_r1 = *(volatile u32**)0x02380174;
    volatile u32* final_r0 = *(volatile u32**)0x02380178;
    
    // (Continua para o próximo bloco de rotinas...)
}
