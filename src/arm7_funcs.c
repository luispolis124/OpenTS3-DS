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
s32  fcn_023803dc(void);
void fcn_02380364(void);
void fcn_02380400(void);
void fcn_02380424(void);

/*
 * fcn_0238018c - Configuração Completa de Registradores de Hardware e Estruturas do ARM7
 */
void fcn_0238018c(void) {
    volatile u32* ptr_base = *(volatile u32**)0x02380258;
    u32 val = *ptr_base;
    
    if ((val & 0x80000000) == 0) {
        return;
    }
    
    volatile u16* reg_ctrl1 = *(volatile u16**)0x0238025C;
    *reg_ctrl1 = *reg_ctrl1 | 0x0001;
    
    volatile u16* reg_ctrl2 = *(volatile u16**)0x02380260;
    *reg_ctrl2 = *reg_ctrl2 | 0x0100;
    
    val = *ptr_base | 0x00010000;
    *ptr_base = val;
    
    volatile u32* dest_ptr = *(volatile u32**)0x02380264;
    dest_ptr[1] = val;
    
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
    
    volatile u8* dest_struct = (volatile u8*)dest_ptr;
    dest_struct[8] = val_r0;
    
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

    dest_struct[9] = (u8)half_r0;

    u32 base_val = *ptr_base;
    base_val = base_val & ~0x80000000;
    *ptr_base = base_val;
}

/*
 * fcn_02380278 - Processamento de Tabelas, Inicialização de Blocos e Verificações do ARM7
 */
void fcn_02380278(void) {
    volatile u32** struct_base_ptr = *(volatile u32***)0x023805D4;
    u32* struct_ptr = (u32*)struct_base_ptr;

    u32 ip = struct_ptr[0];
    u32 limit = struct_ptr[1];
    u32 r1_val = struct_ptr[2];

    while ((s32)ip < (s32)limit) {
        u32 r0 = *(u32*)ip;
        ip += 4;

        u32 special_val = *(u32*)0x02380358;
        if (r0 == special_val) {
            u32 r2_skip = *(u32*)ip;
            ip += 12;
            r1_val += r2_skip;
            continue;
        }

        u32 arg_second = *(u32*)ip;
        ip += 4;
        fcn_02380508(r0, arg_second, 4);

        u32 arg_third = *(u32*)ip;
        ip += 4;
        u32 arg_fourth = *(u32*)ip;
        ip += 4;

        fcn_02380554(arg_third, arg_fourth);
    }

    struct_ptr[0] = ip;
    struct_ptr[2] = r1_val;

    s32 check_res = fcn_023803dc();
    if (check_res == 0) {
        fcn_02380364();
        
        volatile u32* sub_struct = *(volatile u32**)0x0238035C;
        u32 sub_val = sub_struct[0x1DC / 4];
        
        if (sub_val != 0) {
            volatile u32** struct_bc_ptr = *(volatile u32***)0x023805BC;
            u32* bc_ptr = (u32*)struct_bc_ptr;

            u32 bc_ip = bc_ptr[0];
            u32 bc_limit = bc_ptr[1];

            while ((s32)bc_ip < (s32)bc_limit) {
                u32 r0 = *(u32*)bc_ip;
                bc_ip += 4;

                u32 arg_second = *(u32*)bc_ip;
                bc_ip += 4;
                fcn_02380508(r0, arg_second, 4);

                u32 arg_third = *(u32*)bc_ip;
                bc_ip += 4;
                u32 arg_fourth = *(u32*)bc_ip;
                bc_ip += 4;

                fcn_02380554(arg_third, arg_fourth);
            }
            bc_ptr[0] = bc_ip;
        }
    }
}

/*
 * fcn_02380364 - Verificações de Status e Cópia de Blocos de Memória do ARM7
 */
void fcn_02380364(void) {
    volatile u32* struct_base = *(volatile u32**)0x023803CC;
    
    u32 status_val = struct_base[0x234 / 4];
    if ((status_val & 4) != 0) {
        return;
    }

    u32 check_1dc = struct_base[0x1DC / 4];
    if (check_1dc != 0) {
        u32 src_addr = struct_base[0x1D8 / 4];
        volatile u32* ptr_5b0 = *(volatile u32**)0x023805B0;
        u32 base_dest = *ptr_5b0;
        u32 dest_addr = base_dest - 0x4000;
        
        fcn_02380508(src_addr, dest_addr, 0);
    }

    u32 check_1cc = struct_base[0x1CC / 4];
    if (check_1cc != 0) {
        u32 src_addr2 = struct_base[0x1C8 / 4];
        volatile u32* ptr_5b0 = *(volatile u32**)0x023805B0;
        u32 base_dest = *ptr_5b0;
        u32 dest_addr2 = base_dest - 0x4000 - 0x4000;
        
        fcn_02380508(src_addr2, dest_addr2, 0);
    }
}

/*
 * fcn_023803dc - Verificação de bit de estado na estrutura de controle
 */
s32 fcn_023803dc(void) {
    volatile u8* struct_ptr = *(volatile u8**)0x023803F8;
    u8 val = struct_ptr[8];
    u8 masked = val & 0x0C;
    
    if (masked == 4) {
        return 1;
    }
    return 0;
}

/*
 * fcn_02380400 - Cópia de registros e halfwords estruturais
 */
void fcn_02380400(void) {
    volatile u32* src_struct = *(volatile u32**)0x0238041C;
    volatile u32* dest_struct = *(volatile u32**)0x02380420;
    
    u32 val_w = src_struct[4 / 4];
    *dest_struct = val_w;
    
    volatile u16* src_h = (volatile u16*)src_struct;
    volatile u16* dest_h = (volatile u16*)dest_struct;
    
    u16 val_h = src_h[8 / 2];
    dest_h[4 / 2] = val_h;
}

/*
 * fcn_02380424 - Inicialização e varredura de memória (Parte 1)
 */
void fcn_02380424(void) {
    u32 r1 = 0;
    
    volatile u16* r2_ptr = *(volatile u16**)0x023804F8;
    volatile u16* r3_ptr = (volatile u16*)((u32)r2_ptr - 0x400000);
    
    *r2_ptr = (u16)r1;
    u16 ip_val = *r3_ptr;
    
    // (Continua na próxima sequência do assembly...)
}
