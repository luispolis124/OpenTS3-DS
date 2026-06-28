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
