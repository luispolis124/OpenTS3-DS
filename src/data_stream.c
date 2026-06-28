/* ==========================================================================
   PROJETO: OpenTS3-DS
   MÓDULO: data_stream.c (Tratamento de Fluxo de Dados)
   ========================================================================== */

// --- [ DEFINIÇÕES DE TIPOS GLOBAIS ] ---
typedef unsigned int uint;
typedef unsigned char byte;

// --- [ SÍMBOLOS EXTERNOS DO EXECUTÁVEL ] ---
extern int DAT_00005fb0;

// --- [ FUNÇÕES DE CONTROLE DE EXCEÇÃO ] ---
void halt_baddata(void) {
    while(1); // Trava o fluxo caso uma instrução corrompida seja atingida
}

/* ==========================================================================
   FUNÇÃO: FUN_00004d06
   DESCRIÇÃO: Processador de buffers de dados brutos e máscaras de alinhamento
   ========================================================================== */
void FUN_00004d06(int param_1, int *param_2, int param_3)
{
    uint uVar1;
    uint uVar2;
    uint unaff_r4 = 0;
    int unaff_r5 = 0;
    int *unaff_r6 = 0;
    int unaff_r7 = 0;
    uint in_stack_000000c0 = 0;
    
    do {
        uVar1 = *(uint *)(unaff_r5 + 0x2c);
        
        while (1) {
            *param_2 = uVar1 << 10;
            param_2[1] = (int)param_2;
            param_2[2] = param_3;
            param_2[3] = unaff_r4;
            
            if ((int)unaff_r4 > -1) break;
            
            *(char *)(unaff_r5 + 0xb) = (char)param_2;
            unaff_r5 = (uVar1 & 0x3fffff) << 6;
            uVar1 = (uVar1 & 0x3fffff) << 9;
            
            while (1) {
                uVar2 = uVar1;
                *param_2 = uVar2 << 0xe;
                param_2[1] = (int)param_2;
                param_2[2] = param_3;
                param_2[3] = unaff_r4;
                uVar1 = unaff_r4;
                
                if ((int)unaff_r4 > -1) break;
                
                param_2 = *(int **)(unaff_r5 + 0xc);
                unaff_r5 = (uVar2 & 0x3ffff) << 10;
                *param_2 = uVar2 << 0x1f;
                param_2[1] = (int)param_2;
                param_2[2] = param_3;
                param_2[3] = unaff_r4;
                
                if ((int)unaff_r4 < 0) {
                    *unaff_r6 = 0x68;
                    while (1) {
                        *unaff_r6 = 0x68; 
                    }
                }
            }
        }
        
        // Alinhamento e escrita de blocos de controle na pilha/memória
        *(uint *)unaff_r4 = unaff_r4;
        *(int *)(unaff_r4 + 4) = unaff_r5;
        *(int **)(unaff_r4 + 8) = unaff_r6;
        *(int *)(unaff_r4 + 0xc) = unaff_r7;
        param_2 = (int *)0x68;
        param_3 = unaff_r5 << 10;
        *unaff_r6 = 0x68;
        *(uint *)in_stack_000000c0 = in_stack_000000c0;
        *(int *)(in_stack_000000c0 + 4) = unaff_r5;
        *(int **)(in_stack_000000c0 + 8) = unaff_r6;
        *(int *)(in_stack_000000c0 + 0xc) = unaff_r7;
        unaff_r4 = in_stack_000000c0;
        
    } while (1);
}

/* ==========================================================================
   FUNÇÃO: FUN_000057d6
   DESCRIÇÃO: Interpretador bitwise de pacotes de dados
   ========================================================================== */
void FUN_000057d6(unsigned int param_1, unsigned int param_2, int param_3, int param_4, unsigned int param_5)
{
    unsigned char uVar2;
    unsigned int uVar3;
    unsigned int *puVar4;
    unsigned int extraout_r1 = 0; 
    int iVar5;
    unsigned int uVar6;
    unsigned int uVar7;
    int iVar8;
    int iVar9;
    unsigned int *puVar10;
    unsigned int *unaff_r5 = (unsigned int*)0;
    unsigned int uVar11;
    unsigned int unaff_r6 = 0;
    unsigned int uVar12;
    int unaff_r7 = 0;
    unsigned int uVar13;
    int unaff_r10 = 0;
    int in_lr = 0;
    int iVar14;
    unsigned long long uVar15;
    
    iVar8 = param_4 * 0x10;
    uVar11 = *unaff_r5;
    uVar6 = (unsigned int)(*(unsigned short *)(param_4 + 10) >> 4);
    *(int *)iVar8 = iVar8;
    *(unsigned int *)(iVar8 + 4) = uVar11;
    *(unsigned int *)(iVar8 + 8) = unaff_r6;
    *(int *)(iVar8 + 0xc) = unaff_r7;
    
    uVar3 = param_3 << 7; 
    *(char *)(unaff_r7 + 0x17) = (char)(unaff_r6 >> 2);
    iVar9 = uVar6 * 0x10000;
    
    uVar15 = 0; 
    uVar3 = (unsigned int)(uVar15 >> 0x20);
    *(int *)(unaff_r7 * 2) = (int)uVar15;
    uVar12 = (uVar3 & 0x7ff) * 0x20000;
    uVar13 = unaff_r7 + 0xfd;
    
    uVar3 = 0;
    uVar6 = extraout_r1;
    puVar10 = (unsigned int *)(iVar9 + -0xf8);
    
    do {
        puVar10 = (unsigned int *)(unsigned int)*(unsigned short *)((int)puVar10 * 2 + uVar6 * -0x200000);
        uVar7 = (uVar13 >> 2) << 0x13;
        
        uVar6 = (int)puVar10 << 4;
        uVar3 = (int)puVar10 * 0x20;
        uVar2 = (unsigned char)uVar13;
        *(unsigned char *)(uVar3 + 1) = uVar2;
        *puVar10 = uVar3;
        puVar10[1] = uVar6;
        puVar10[2] = uVar7;
        puVar10[3] = (unsigned int)puVar10;
        puVar10[4] = uVar12;
        puVar10[5] = uVar13;
        
        while (1) {
            *(unsigned int *)uVar3 = uVar3;
            *(unsigned int *)(uVar3 + 4) = uVar6;
            *(unsigned int *)(uVar3 + 8) = uVar7;
            *(unsigned int **)(uVar3 + 0xc) = puVar10;
            *(unsigned int *)(uVar3 + 0x10) = uVar12;
            *(unsigned int *)(uVar3 + 0x14) = uVar13;
            *(short *)(uVar3 - 0x8d) = (short)uVar3 + -0xb5;
            
            iVar9 = DAT_00005fb0;
            iVar5 = (int)puVar10 << 4;
            *(short *)(uVar12 + 0x1e) = (short)((int)puVar10 << 8);
            puVar10 = (unsigned int *)(uVar13 >> 3);
            iVar14 = iVar9 * 0x4000000;
            uVar6 = iVar9 * 0x2000 + unaff_r10;
            *(unsigned int *)(iVar14 + -0xbd) = uVar6;
            *(int *)(iVar14 + -0xb9) = iVar5;
            *(unsigned int *)(iVar14 + -0xb5) = uVar12;
            uVar3 = iVar14 - 0xb1;
            uVar7 = (unsigned int)*(byte *)(uVar12 + 0x12);
            
            if ((int)uVar3 >> 0x18 < 0) break;
            
            *puVar10 = uVar3;
            puVar10[1] = uVar6;
            puVar10[2] = uVar7;
            puVar10[3] = (unsigned int)puVar10;
            puVar10[4] = uVar12;
            puVar10[5] = uVar13;
            *(short *)(iVar14 + -0xa9) = (short)uVar3;
            uVar3 = uVar6 * 0x20 ^ uVar7;
            *(unsigned char *)(uVar3 + 1) = uVar2;
            *puVar10 = uVar3;
            puVar10[1] = uVar6;
            puVar10[2] = uVar7;
            puVar10[3] = (unsigned int)puVar10;
            puVar10[4] = uVar12;
            puVar10[5] = uVar13;
            *(short *)(uVar3 + 8) = (short)uVar3;
            uVar3 = uVar6 * 0x20 ^ uVar7;
            *(unsigned char *)(uVar3 + 1) = uVar2;
            
            if (-1 < (int)uVar3) {
                iVar8 = uVar7 * 0x10000;
                *(int *)iVar8 = iVar8;
                *(unsigned int *)(iVar8 + 4) = uVar6;
                *(unsigned int *)(iVar8 + 8) = uVar12;
                halt_baddata();
            }
        }
    } while (1);
}

/* ==========================================================================
   FUNÇÃO: FUN_000063cc
   DESCRIÇÃO: Gerenciador de Estado de Hardware e Paginação de Memória
   ========================================================================== */

// Variáveis e símbolos globais mapeados do executável
extern int DAT_00006594;
extern unsigned int* DAT_00006c78;
extern unsigned short DAT_000065e4;
extern unsigned int DAT_00006886;
extern unsigned int DAT_00005cfc;
extern unsigned int DAT_00005a00;
extern char Reset;

// Pseudo-funções e mapeamento de registradores específicos do Ghidra
#define SCARRY4(a, b) (((unsigned int)(a) + (unsigned int)(b)) < (unsigned int)(a))
extern void FUN_00006b54(unsigned short p1, int p2, int p3);

void FUN_000063cc(int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4)
{
    unsigned short uVar2;
    unsigned char uVar3;
    char cVar4;
    unsigned int *puVar5;
    int iVar6;
    int *piVar7;
    unsigned int *puVar8;
    int iVar9;
    unsigned short extraout_var = 0;
    unsigned int uVar10;
    unsigned char extraout_r1 = 0;
    unsigned char *puVar11;
    unsigned int unaff_r4 = 0;
    unsigned short *unaff_r5 = (unsigned short*)0;
    int iVar12;
    unsigned int unaff_r6 = 0;
    unsigned int uVar13;
    int unaff_r7 = 0;
    unsigned int uVar14;
    unsigned int uVar15;
    int unaff_r9 = 0;
    int iVar16;
    unsigned int uVar17 = 0;
    unsigned int in_lr = 0;
    int in_stack_00000060 = 0;
    int in_stack_000003e8 = 0;
    int iStack_1d0 = 0;
    
    // Ponteiros voláteis para simular escritas diretas em endereços de hardware (RAM)
    volatile unsigned int *uRam0000002b = (volatile unsigned int *)0x2b;
    volatile unsigned int *uRam00000363 = (volatile unsigned int *)0x363;
    volatile unsigned int *uRam00000367 = (volatile unsigned int *)0x367;
    volatile unsigned int *uRam0000036b = (volatile unsigned int *)0x36b;
    volatile unsigned int *uRam0000033d = (volatile unsigned int *)0x49; // ram0x00000049

    *(int *)unaff_r5 = param_1;
    *(unsigned int *)(unaff_r5 + 2) = param_2;
    *(unsigned int *)(unaff_r5 + 4) = param_4;
    *(unsigned int *)(unaff_r5 + 6) = unaff_r4;
    *(unsigned int *)(unaff_r5 + 8) = unaff_r6;
    *(int *)(unaff_r5 + 10) = unaff_r7;
    
    puVar8 = (unsigned int *)(param_1 + unaff_r9);
    *puVar8 = param_4;
    puVar8[1] = (unsigned int)(unaff_r5 + 0xc);
    puVar8[2] = unaff_r4;
    
    uVar14 = unaff_r7 + unaff_r9;
    uVar15 = uVar14 - 0x48;
    uVar13 = param_4 & 0xff;
    uVar3 = (unsigned char)uVar15;
    
    if ((uVar13 == 0 && 0x47 < uVar14 ||
        uVar13 != 0 && ((int)(unsigned int)*(unsigned char *)(param_4 + 0x10) >> (uVar13 - 1) & 1U) != 0) &&
        (int)(unsigned int)*(unsigned char *)(param_4 + 0x10) >> uVar13 != 0) {
        
        unaff_r5[-4] = (unsigned short)(unaff_r5 + -4);
        puVar11 = (unsigned char *)&DAT_00006594;
        uVar13 = unaff_r4;
        
        while( 1 ) {
            // Abstração de chamadas dinâmicas do Ghidra em loops
            in_lr = 0x6acf;
            *(unsigned char *)(uVar14 - 0x2d) = extraout_r1;
            puVar5 = DAT_00006c78;
            
            if (uVar15 >> 2 != 0) break;
            
            *(unsigned char *)(unaff_r4 * 0x20 + 1) = uVar3;
            *(unsigned char *)(unaff_r4 * 0x20 + 1) = uVar3;
            *(char *)(param_4 + 0x13) = (char)unaff_r4;
            *(unsigned char *)(unaff_r4 * 0x10 + 1) = uVar3;
            *(unsigned char *)(unaff_r4 + 1) = uVar3;
            uVar13 = *(unsigned int *)((uVar13 >> 2) * 0x1000 + 8);
            *(unsigned int *)param_4 = param_4;
            *(unsigned int *)(param_4 + 4) = uVar15;
            *(short *)(param_4 + 0x42) = (short)((int)unaff_r4 >> 0x1c);
            puVar11 = (unsigned char *)0x0;
            unaff_r4 = 0xffffffde;
        }
        
        *(unsigned char *)((int)DAT_00006c78 + 1) = uVar3;
        *puVar5 = 0x62132000;
        puVar5[1] = 0xd0;
        puVar5[2] = uVar13;
        *(unsigned int *)param_4 = param_4;
        *(unsigned int *)(param_4 + 4) = uVar15;
        *(short *)(param_4 + 0x42) = (short)(char)((unsigned int)(puVar5 + 3) >> 0x18);
        cVar4 = Reset;
        iVar9 = (int)Reset;
        *(int *)(iVar9 + 0x20) = iVar9;
        *(char *)(iVar9 + 0x18) = cVar4;
        *(unsigned int *)(uVar15 * 2) = 0x48;
        
        FUN_00006b54(*(unsigned short *)(iStack_1d0 + 0x36), 0, (int)uVar15 >> 3);
        halt_baddata();
    }
    
    uVar2 = *unaff_r5;
    *(unsigned int *)(param_4 + ~param_4 + 0x1c) = param_4 << 0x12;
    iVar9 = uVar2 + 0x49;
    
    if ((int)(param_4 << 0x18) < 0 == SCARRY4((unsigned int)uVar2, 0x49)) {
        *uVar33d = param_4 << 0x12;
        DAT_00006886 = 0;
        halt_baddata();
    }
    
    *(short *)(in_stack_00000060 + iVar9) = (short)iVar9;
    uVar2 = DAT_000065e4;
    *(unsigned int *)(~param_4 + 0x40) = (unsigned int)DAT_000065e4;
    *(unsigned short *)(unaff_r4 + 0xc) = uVar2;
    iVar12 = *(int *)(unaff_r4 + 8);
    uVar14 = iVar12 + 0x22;
    uVar13 = *(unsigned int *)(unaff_r4 + 4) >> 9;
    iVar9 = 0;
    uVar2 = *(unsigned short *)(iVar12 + 0x30);
    *(unsigned int *)(uVar2 + 0x14) = 0;
    Reset = (char)uVar2;
    iVar16 = 0x594d;
    
    // Desvios lógicos e flags do sistema
    uVar13 = 0; 
    uVar17 = 0x5955;
    
    if (uVar15 == 0) {
        uVar15 = uVar17 & 0xfffffffc;
        uVar17 = 0x595d;
        *(unsigned short *)(uVar13 + 0x22) = extraout_var >> 1;
        *(unsigned short *)(uVar13 + 10) = 0;
        *(unsigned short *)(in_stack_000003e8 + 0x46) = 0;
        uVar15 = DAT_00005cfc;
        
        *uRam0000002b = 0;
        *uRam00000363 = DAT_00005a00;
        uVar10 = (unsigned int)*(unsigned char *)(iVar9 * 2);
        *uRam00000367 = uVar13;
        *uRam0000036b = uVar14;
        *(unsigned int *)(iVar9 * 2) = uVar10;
        uVar13 = (uVar14 >> 2) + 0x36f + (unsigned int)(0xffffffde < uVar15);
        puVar8 = (unsigned int *)(uVar10 << 5 ^ uVar13);
        *(unsigned char *)((int)puVar8 + 1) = 0;
        *puVar8 = uVar10;
        puVar8[1] = 0;
        puVar8[2] = uVar14;
        iVar9 = (unsigned int)*(unsigned char *)(uVar13 * 2) << 0xd;
        uVar13 = 0; 
        piVar7 = (int *)((in_stack_000003e8 + 0x3c) * 2);
        
        *(unsigned char *)((int)piVar7 + 1) = 0;
        *piVar7 = iVar9;
        piVar7[1] = uVar13;
        piVar7[2] = uVar14;
        *(short *)(iVar12 + 0x46) = (short)piVar7 + 0xc;
    }
    
    halt_baddata();
}
