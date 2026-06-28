/* ==========================================================================
   PROJETO: OpenTS3-DS
   MÓDULO: data_stream.c (Tratamento de Fluxo de Dados)
   ========================================================================== */

typedef unsigned int uint;
typedef unsigned char byte;

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
        while( 1 ) {
            *param_2 = uVar1 << 10;
            param_2[1] = (int)param_2;
            param_2[2] = param_3;
            param_2[3] = unaff_r4;
            
            if ((int)unaff_r4 > -1) break;
            
            *(char *)(unaff_r5 + 0xb) = (char)param_2;
            unaff_r5 = (uVar1 & 0x3fffff) << 6;
            uVar1 = (uVar1 & 0x3fffff) << 9;
            
            while( 1 ) {
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
                    while(1) {
                        *unaff_r6 = 0x68; // Impede que o compilador acuse loop quebrado
                    }
                }
            }
        }
        
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
        
    } while( 1 );
}

extern int DAT_00005fb0;

void halt_baddata(void) {
    while(1);
}

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
        
        while( 1 ) {
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
    } while( 1 );
}
