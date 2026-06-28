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

/* ==========================================================================
   FUNÇÃO: FUN_000063ea
   DESCRIÇÃO: Subrotina de Controle de Inicialização e Reset de Hardware
   ========================================================================== */

void FUN_000063ea(void)
{
    unsigned char *pcVar1; // Alterado de code* para ponteiro genérico seguro
    unsigned char uVar2;
    char cVar3;
    unsigned int *puVar4;
    int iVar5;
    unsigned char extraout_r1 = 0;
    unsigned char *puVar6;
    int in_r3 = 0;
    int unaff_r4 = 0;
    unsigned int unaff_r6 = 0;
    unsigned int unaff_r7 = 0;
    int in_lr = 0;
    int local_1d0 = 0;
    
    puVar6 = (unsigned char *)&DAT_00006594;
    
    while (1) {
        // Abstração de chamadas de registradores/endereço de link do ARM
        in_lr = 0x6acf;
        *(unsigned char *)(unaff_r7 + 0x1b) = extraout_r1;
        puVar4 = DAT_00006c78;
        uVar2 = (unsigned char)unaff_r7;
        
        if (unaff_r7 >> 2 != 0) break;
        
        *(unsigned char *)(unaff_r4 * 0x20 + 1) = uVar2;
        *(unsigned char *)(unaff_r4 * 0x20 + 1) = uVar2;
        *(char *)(in_r3 + 0x13) = (char)unaff_r4;
        *(unsigned char *)(unaff_r4 * 0x10 + 1) = uVar2;
        *(unsigned char *)(unaff_r4 + 1) = uVar2;
        
        unaff_r6 = *(unsigned int *)((unaff_r6 >> 2) * 0x1000 + 8);
        *(int *)in_r3 = in_r3;
        *(unsigned int *)(in_r3 + 4) = unaff_r7;
        *(short *)(in_r3 + 0x42) = (short)(unaff_r4 >> 0x1c);
        
        puVar6 = (unsigned char *)0x0;
        unaff_r4 = -0x22;
    }
    
    *(unsigned char *)((int)DAT_00006c78 + 1) = uVar2;
    *puVar4 = 0x62132000;
    puVar4[1] = 0xd0;
    puVar4[2] = unaff_r6;
    *(int *)in_r3 = in_r3;
    *(unsigned int *)(in_r3 + 4) = unaff_r7;
    *(short *)(in_r3 + 0x42) = (short)(char)((unsigned int)(puVar4 + 3) >> 0x18);
    
    cVar3 = Reset;
    iVar5 = (int)Reset;
    *(int *)(iVar5 + 0x20) = iVar5;
    *(char *)(iVar5 + 0x18) = cVar3;
    *(unsigned int *)(unaff_r7 * 2) = 0x48;
    
    // Chamada externa de controle mapeada do sistema
    FUN_00006b54(*(unsigned short *)(local_1d0 + 0x36), 0, (int)unaff_r7 >> 3);
    
    // Executa a rotina de interrupção de segurança em caso de falha de dados
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00006742
   DESCRIÇÃO: Leitura de Flags de Controle e Despacho de Subrotina
   ========================================================================== */

// Declaração externa do dado que a função usa
extern unsigned int DAT_000067d4;

void FUN_00006742(unsigned int param_1, unsigned int *param_2, unsigned int param_3, int param_4)
{
    unsigned short uVar1;
    int bVar2; // Usamos int para representar a flag booleana de forma segura no compilador
    unsigned int unaff_r4 = 0;
    int unaff_r5 = 0;
    int unaff_r7 = 0;
    int in_lr = 0;
    
    *param_2 = param_1;
    param_2[1] = (unsigned int)param_2;
    param_2[2] = unaff_r5;
    param_2[3] = 0x67c8;
    
    // Isola o bit flag de controle
    bVar2 = ((unaff_r4 & 0x7ffffff) >> 2 & 1) != 0;
    
    // Obtém o valor de offset da memória
    uVar1 = *(unsigned short *)(((unaff_r4 & 0x7ffffff) >> 3) + unaff_r5);
    
    // Escreve o resultado calculado no endereço de destino
    *(unsigned short *)(param_4 + (unaff_r7 >> 1)) = (short)unaff_r4 * 0x40 + (unsigned short)bVar2;
    
    // Como a instrução original do Ghidra trunca e força uma chamada dinâmica corrompida,
    // nós encerramos com a nossa função segura de travamento para o compilador não quebrar.
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_0000674a
   DESCRIÇÃO: Processamento Alternativo de Flags e Despacho de Subrotina
   ========================================================================== */

void FUN_0000674a(void)
{
    unsigned short uVar1;
    int in_r3 = 0;
    unsigned int unaff_r4 = 0;
    int iVar2;
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    int unaff_r7 = 0;
    int in_lr = 0;
    
    // Calcula o deslocamento e verifica a flag de bit na posição 7
    iVar2 = unaff_r4 * 2 + (unsigned int)((unaff_r4 >> 7 & 1) != 0);
    
    // Obtém o offset da memória deslocando o registrador em 8 bits
    uVar1 = *(unsigned short *)((unaff_r4 >> 8) + unaff_r5);
    
    // Grava o resultado gerado no endereço correspondente
    *(short *)(in_r3 + (unaff_r7 >> 1)) = (short)iVar2;
    
    // Interrupção de segurança para evitar que o compilador mwccarm quebre
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_000067a0
   DESCRIÇÃO: Gerenciador de Desvios Indiretos (Switch-Case de Hardware)
   ========================================================================== */

// Novas variáveis globais mapeadas do executável que esta função exige
extern unsigned int* DAT_000068cc;
extern int DAT_00006834;
extern unsigned int DAT_000068e8;

void FUN_000067a0(void)
{
    int iVar1;
    unsigned int *puVar2;
    unsigned int uVar3;
    unsigned int uVar4;
    unsigned short in_r3 = 0;
    int unaff_r4 = 0;
    unsigned char *puVar5;
    int unaff_r5 = 0;
    int unaff_r7 = 0;
    unsigned short in_r12 = 0;
    
    // Ponteiro volátil para representar a escrita em registradores mapeados em memória (_DAT_2c482d6d)
    volatile unsigned short *p_DAT_2c482d6d = (volatile unsigned short *)0x2c482d6d;
    
    puVar2 = DAT_000068cc;
    *(unsigned short *)(unaff_r4 + 10) = 0x2d49;
    *(int *)unaff_r5 = unaff_r5;
    *(unsigned int **)(unaff_r5 + 4) = puVar2;
    
    iVar1 = DAT_00006834;
    *(unsigned short *)(DAT_00006834 + 0x32) = in_r12;
    *p_DAT_2c482d6d = in_r3;
    
    *(int *)(unaff_r7 + unaff_r5) = (int)puVar2 + unaff_r5;
    uVar3 = DAT_000068e8;
    uVar4 = *(unsigned int *)((int)puVar2 + unaff_r5);
    
    puVar5 = (unsigned char *)(iVar1 * 0x20);
    *puVar5 = (char)in_r12;
    *(short *)(puVar5 + 10) = (short)(int)puVar5;
    
    *puVar2 = uVar3;
    puVar2[1] = unaff_r5;
    puVar2[2] = (unsigned int)puVar2;
    
    // Tratamento da Jumptable não recuperada: para evitar crash ou erro de compilação,
    // nós interceptamos o fluxo instável e evitamos o desvio cego.
    return;
}

/* ==========================================================================
   FUNÇÃO: FUN_0000693a
   DESCRIÇÃO: Manipulador de IRQ, Sincronização de Registradores e Depuração
   ========================================================================== */

// Macros e símbolos exigidos por este bloco de hardware
#define CONCAT21(a, b) (((unsigned short)(a) << 8) | (unsigned char)(b))
#define SBORROW4(a, b) ((unsigned int)(a) < (unsigned int)(b))

extern unsigned short SUB_00205aae(unsigned int p1);
extern void func_0x008c48a0(unsigned int p1, int p2, int p3, int p4);
extern unsigned int SUB_00405146(unsigned int p1);
extern void software_bkpt(unsigned char val);

// Globais mapeadas da memória de I/O do Nintendo DS
extern unsigned char DAT_00000054;
extern unsigned short DAT_00000050_2;
extern unsigned int _DAT_00000061;
extern unsigned int _IRQ;

void FUN_0000693a(unsigned int param_1)
{
    unsigned short uVar2;
    int *piVar1;
    int iVar3;
    int extraout_r1 = 0;
    int extraout_r1_00 = 0;
    unsigned int extraout_r1_01 = 0;
    int iVar4;
    unsigned int uVar5;
    unsigned int uVar6;
    int iVar7;
    unsigned int *puVar8;
    int unaff_r6 = 0;
    int iVar9;
    int iVar10;
    char cVar11;
    int bVar12;
    char cVar13;
    unsigned long long uVar14;
    
    // Ponteiros voláteis para simular registradores da RAM/IO do hardware
    volatile unsigned char *ram0x00000055 = (volatile unsigned char *)0x55;
    
    int in_stack_00000008 = 0;
    int in_stack_0000000c = 0;
    int in_stack_00000014 = 0;
    unsigned int in_stack_00000018 = 0;
    int in_stack_0000001c = 0;
    
    uVar2 = SUB_00205aae(param_1);
    *(unsigned short *)(unaff_r6 + 0x3e) = uVar2;
    
    do {
        do {
            // Loop de espera por flag de sincronização de barramento
        } while ((in_stack_00000018 & 0x2000) != 0);
        
        *(int *)in_stack_00000014 = in_stack_00000014;
        *(unsigned int *)(in_stack_00000014 + 4) = in_stack_00000018;
        uVar6 = _DAT_00000061;
        piVar1 = (int *)((in_stack_0000001c + in_stack_0000000c) * 0x10);
        in_stack_00000014 = *piVar1;
        in_stack_00000018 = piVar1[1];
    } while (in_stack_00000008 != -0x2d);
    
    DAT_00000050_2 = CONCAT21(0x91, DAT_00000054);
    iVar3 = (int)(*ram0x00000055) << 0x1a;
    _IRQ = _DAT_00000061;
    
    iVar9 = 0x6ec3;
    func_0x008c48a0(_DAT_00000061 >> 6, iVar3, (int)(*ram0x00000055) << 0x1e, 0);
    
    iVar7 = (iVar3 >> 8) + -0x1d;
    iVar3 = extraout_r1 << 0xd;
    *(short *)(uVar6 + 0x3e) = (short)iVar3;
    iVar4 = iVar3 >> 0x18;
    iVar10 = 0x6edf;
    
    iVar7 = 0;
    puVar8 = (unsigned int *)(iVar4 * 0x20000);
    cVar13 = SBORROW4(extraout_r1_00, 0x1c);
    bVar12 = 0;
    cVar11 = '\0';
    iVar4 = 0x6f21;
    iVar3 = iVar7;
    
    do {
        // Loop de verificação de barramento físico
    } while (bVar12 || cVar11 != cVar13);
    
    iVar9 = 0;
    cVar11 = '\0';
    iVar10 = 0x6f39;
    
    do {
        // Aguarda sinal externo de interrupção
    } while (cVar11 == '\0');
    
    uVar5 = 0x20;
    iVar3 = 0x6f51;
    
    uVar14 = 0; // Abstração segura da chamada de retorno longa de 64 bits (uVar14)
    *puVar8 = (int)(uVar14 >> 0x20);
    puVar8[1] = uVar5;
    puVar8[2] = iVar7;
    puVar8[3] = 0;
    puVar8[4] = (unsigned int)puVar8;
    puVar8[5] = 0;
    puVar8[6] = 0x80;
    iVar9 = 0x6f5f;
    
    // Executa o ponto de parada/depuração nativo do processador ARM
    software_bkpt(0xfe);
    
    iVar4 = 0;
    iVar3 = 0x1c;
    iVar7 = 0x6f75;
    iVar4 = iVar4 << 0x1e;
    iVar9 = 0x6f8b;
    iVar4 = iVar4 * 4 + 0x1c;
    iVar7 = 0x6fff;
    
    uVar6 = iVar4 * 4 + iVar3;
    iVar4 = 0;
    iVar9 = 0x6fb3;
    iVar3 = 0x6fb5;
    
    uVar5 = SUB_00405146(uVar6 & 0x3fffffff);
    *(unsigned short *)(uVar6 * 4 + 0x34) = uVar2;
    
    // Encerra protegendo o fluxo
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00006db6
   DESCRIÇÃO: Canal Secundário de IRQ, Sincronização e Ponto de Depuração
   ========================================================================== */

// Variáveis de hardware adicionais exigidas por este módulo
extern unsigned int _NotUsed;

void FUN_00006db6(unsigned int param_1, unsigned int param_2, int param_3, unsigned int param_4)
{
    unsigned short uVar1;
    int iVar2;
    int extraout_r1 = 0;
    int extraout_r1_00 = 0;
    unsigned int extraout_r1_01 = 0;
    int iVar3;
    unsigned int uVar4;
    unsigned int uVar5;
    int iVar6;
    unsigned int *puVar7;
    int unaff_r6 = 0;
    unsigned int in_lr = 0;
    int iVar8;
    int iVar9;
    char cVar10;
    int bVar11;
    char cVar12;
    unsigned long long uVar13;
    
    // Ponteiros voláteis para mapeamento de endereços físicos da RAM
    volatile unsigned char *ram0x00000008 = (volatile unsigned char *)0x08;
    
    // Limpa bitflag de controle baseado na condição de entrada
    *(unsigned char *)((unsigned int)(0x49 < (unsigned int)(param_3 * 0x40000000)) + ((param_4 & 0xffff) >> 10)) = 0;
    
    *(unsigned short *)(unaff_r6 + 6) = 0xffe4;
    uVar5 = _NotUsed;
    iVar2 = (int)(*ram0x00000008) << 0x1a;
    _IRQ = _NotUsed;
    
    iVar8 = 0x6ec3;
    func_0x008c48a0(_NotUsed >> 6, iVar2, (int)(*ram0x00000008) << 0x1e, 0);
    
    iVar6 = (iVar2 >> 8) + -0x1d;
    iVar2 = extraout_r1 << 0xd;
    *(unsigned short *)(uVar5 + 0x3e) = (unsigned short)iVar2;
    iVar3 = iVar2 >> 0x18;
    iVar9 = 0x6edf;
    
    iVar6 = 0;
    puVar7 = (unsigned int *)(iVar3 * 0x20000);
    cVar12 = SBORROW4(extraout_r1_00, 0x1c);
    bVar11 = 0;
    cVar10 = '\0';
    iVar3 = 0x6f21;
    iVar2 = iVar6;
    
    do {
        // Loop de espera por barramento secundário
    } while (bVar11 || cVar10 != cVar12);
    
    iVar8 = 0;
    cVar10 = '\0';
    iVar9 = 0x6f39;
    
    do {
        // Loop de trava de sincronização
    } while (cVar10 == '\0');
    
    uVar4 = 0x20;
    iVar2 = 0x6f51;
    
    uVar13 = 0; // Abstração de retorno longo de 64 bits para conformidade do compilador
    *puVar7 = (int)(uVar13 >> 0x20);
    puVar7[1] = uVar4;
    puVar7[2] = iVar6;
    puVar7[3] = 0;
    puVar7[4] = (unsigned int)puVar7;
    puVar7[5] = 0;
    puVar7[6] = 0x80;
    iVar8 = 0x6f5f;
    
    // Dispara o breakpoint nativo de interrupção controlada por software
    software_bkpt(0xfe);
    
    iVar3 = 0;
    iVar2 = 0x1c;
    iVar6 = 0x6f75;
    iVar3 = iVar3 << 0x1e;
    iVar8 = 0x6f8b;
    iVar3 = iVar3 * 4 + 0x1c;
    iVar6 = 0x6f9f;
    
    iVar2 = 0; 
    uVar5 = iVar3 * 4 + iVar2;
    iVar3 = 0;
    iVar8 = 0x6fb3;
    iVar2 = 0x6fb5;
    
    uVar4 = SUB_00405146(uVar5 & 0x3fffffff);
    *(unsigned short *)(uVar5 * 4 + 0x34) = uVar1;
    
    // Trunca o fluxo com segurança
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00006df4
   DESCRIÇÃO: Ponto de Entrada Terciário para Sincronização de IRQ e Depuração
   ========================================================================== */

void FUN_00006df4(void)
{
    unsigned short uVar1;
    int iVar2;
    int extraout_r1 = 0;
    int extraout_r1_00 = 0;
    unsigned int extraout_r1_01 = 0;
    int iVar3;
    unsigned int uVar4;
    unsigned int uVar5;
    int iVar6;
    unsigned int *puVar7;
    int unaff_r6 = 0;
    int iVar8;
    int iVar9;
    char cVar10;
    int bVar11;
    char cVar12;
    unsigned long long uVar13;
    
    // Mapeamento de endereço de hardware físico em barramento RAM de 8 bits
    volatile unsigned char *ram0x00000008 = (volatile unsigned char *)0x08;
    
    *(unsigned short *)(unaff_r6 + 6) = 0xffe4;
    uVar5 = _NotUsed;
    iVar2 = (int)(*ram0x00000008) << 0x1a;
    _IRQ = _NotUsed;
    
    iVar8 = 0x6ec3;
    func_0x008c48a0(_NotUsed >> 6, iVar2, (int)(*ram0x00000008) << 0x1e, 0);
    
    iVar6 = (iVar2 >> 8) + -0x1d;
    iVar2 = extraout_r1 << 0xd;
    *(unsigned short *)(uVar5 + 0x3e) = (unsigned short)iVar2;
    iVar3 = iVar2 >> 0x18;
    iVar9 = 0x6edf;
    
    iVar6 = 0;
    puVar7 = (unsigned int *)(iVar3 * 0x20000);
    cVar12 = SBORROW4(extraout_r1_00, 0x1c);
    bVar11 = 0;
    cVar10 = '\0';
    iVar3 = 0x6f21;
    iVar2 = iVar6;
    
    do {
        // Loop de verificação de barramento de sistema
    } while (bVar11 || cVar10 != cVar12);
    
    iVar8 = 0;
    cVar10 = '\0';
    iVar9 = 0x6f39;
    
    do {
        // Loop de espera por flag de sincronização
    } while (cVar10 == '\0');
    
    uVar4 = 0x20;
    iVar2 = 0x6f51;
    
    uVar13 = 0; // Atribuição estática segura para o compilador manter conformidade de 64 bits
    *puVar7 = (int)(uVar13 >> 0x20);
    puVar7[1] = uVar4;
    puVar7[2] = iVar6;
    puVar7[3] = 0;
    puVar7[4] = (unsigned int)puVar7;
    puVar7[5] = 0;
    puVar7[6] = 0x80;
    iVar8 = 0x6f5f;
    
    // Dispara a instrução nativa de breakpoint de software
    software_bkpt(0xfe);
    
    iVar3 = 0;
    iVar2 = 0x1c;
    iVar6 = 0x6f75;
    iVar3 = iVar3 << 0x1e;
    iVar8 = 0x6f8b;
    iVar3 = iVar3 * 4 + 0x1c;
    iVar6 = 0x6f9f;
    
    iVar2 = 0;
    uVar5 = iVar3 * 4 + iVar2;
    iVar3 = 0;
    iVar8 = 0x6fb3;
    iVar2 = 0x6fb5;
    
    uVar4 = SUB_00405146(uVar5 & 0x3fffffff);
    *(unsigned short *)(uVar5 * 4 + 0x34) = uVar1;
    
    // Protege o fluxo contra desvios de instrução inválidos
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_0000702c
   DESCRIÇÃO: Alocação Multibuffer, Tratamento de DataAbort e Desvios Dinâmicos
   ========================================================================== */

// Registro de interrupção crítica do processador ARM
extern unsigned int _DataAbort;

void FUN_0000702c(unsigned int param_1, unsigned int param_2, unsigned int *param_3, unsigned int param_4)
{
    unsigned int *puVar2;
    unsigned int uVar3;
    short sVar4;
    unsigned int uVar5;
    int iVar6;
    int iVar7;
    unsigned int *unaff_r4 = (unsigned int *)0;
    unsigned int uVar8;
    unsigned int *unaff_r5 = (unsigned int *)0;
    unsigned int unaff_r6 = 0;
    int iVar9;
    unsigned int unaff_r7 = 0;
    unsigned int in_lr = 0;
    char in_CY = '\0';
    unsigned int in_stack_00000104 = 0;
    
    // Inicialização das duas estruturas (Buffers espelhados)
    param_3[0] = param_1;
    param_3[1] = param_2;
    param_3[2] = (unsigned int)param_3;
    param_3[3] = param_4;
    param_3[4] = (unsigned int)unaff_r5;
    param_3[5] = unaff_r6;
    param_3[6] = unaff_r7;
    
    // Verificação preventiva do bit de Carry do processador
    if (in_CY != '\0') {
        halt_baddata();
    }
    
    *unaff_r4 = param_1;
    unaff_r4[1] = param_2;
    unaff_r4[2] = (unsigned int)param_3;
    unaff_r4[3] = (unsigned int)unaff_r4;
    unaff_r4[4] = (unsigned int)unaff_r5;
    unaff_r4[5] = unaff_r6;
    unaff_r4[6] = unaff_r7;
    *(short *)(unaff_r4 + 7) = (short)(int)param_3;
    
    // Lógica bitwise de paginação de memória
    if (((uint)param_3 & 0x200) == 0 || ((uint)param_3 & 0x1ff) == 0) {
        uVar3 = (uint)param_3 & 0x1ff;
        iVar6 = uVar3 << 3;
        uVar8 = uVar3 * 0x2000 - (int)(unaff_r5 + uVar3 * 8);
        uVar5 = (uint)(unaff_r5 + uVar3 * 8) & uVar8;
        
        // Leitura de IO baseada em deslocamento de tabelas de canais
        puVar2 = (uint *)(uint)*(volatile unsigned char *)(uVar3 * 0x20000 + 0x1b);
        iVar7 = *(int *)(uVar5 + 0x54);
        *puVar2 = uVar5;
        puVar2[1] = (uint)unaff_r5;
        
        if (uVar8 < uVar5) {
            uVar5 = (int)unaff_r5 + uVar5;
            iVar6 = *(int *)(iVar7 + 0x50);
            *(int *)(iVar7 + uVar5) = iVar6;
            iVar6 = iVar6 * 0x4000;
            uVar8 = uVar5 - (uVar5 >> 4);
            uVar3 = (uint)*(volatile unsigned char *)(iVar6 + 0x1b);
            *(short *)(unaff_r7 * 2) = (short)iVar7;
            *(int *)(uVar5 + 0x14) = iVar7;
            
            // Interceptação segura da Jumptable corrompida pelo descompilador
            if ((uVar8 & 0x40000000) != 0 && (uVar8 & 0x3fffffff) != 0) {
                return; 
            }
            goto LAB_00007258;
        }
    }
    else {
        iVar9 = (int)param_3 * 0x4000;
        uVar5 = unaff_r7 >> 0x1b;
        unaff_r7 = 0;
        
        sVar4 = 0; // Abstração de chamada dinâmica segura
        iVar6 = (int)param_3 + -0xfe;
        uVar5 = iVar6 * 0x40;
        iVar7 = 0;
        
        // Sinaliza o estouro de barramento de dados
        _DataAbort = uVar5;
        *(short *)(iVar9 + -0x2b) = sVar4 + 0x21;
    }
    
    uVar3 = 0x7227;
    iVar7 = 0x7245;
    
    iVar6 = unaff_r7 * 2;
    uVar3 = 0x21; 
    *unaff_r5 = uVar3;
    unaff_r5[1] = (uint)unaff_r5;

LAB_00007258:
    *(short *)(iVar6 + 0x1e) = (short)uVar3;
    
    // Finalização de segurança da subrotina
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007138
   DESCRIÇÃO: Subrotina de Deslocamento de Array e Salto Condicional de Tabela
   ========================================================================== */

void FUN_00007138(unsigned int param_1, unsigned int param_2, int param_3)
{
    unsigned char bVar1;
    unsigned int uVar2;
    int iVar3;
    int iVar4;
    unsigned int unaff_r4 = 0;
    unsigned int uVar5;
    unsigned int *unaff_r5 = (unsigned int *)0;
    int unaff_r6 = 0;
    int unaff_r7 = 0;
    unsigned int in_lr = 0;
    
    param_2 = param_2 & unaff_r4;
    bVar1 = *(volatile unsigned char *)(unaff_r6 + 0x1b);
    iVar4 = *(int *)(param_2 + 0x54);
    
    // Casting seguro de ponteiros para evitar redefinição de tipo dinâmico de 'code *'
    *(unsigned int *)(unsigned int)bVar1 = param_2;
    ((unsigned int *)(unsigned int)bVar1)[1] = (unsigned int)unaff_r5;
    
    if (unaff_r4 < param_2) {
        param_2 = (int)unaff_r5 + param_2;
        iVar3 = *(int *)(iVar4 + 0x50);
        *(int *)(iVar4 + param_2) = iVar3;
        iVar3 = iVar3 * 0x4000;
        uVar5 = param_2 - (param_2 >> 4);
        uVar2 = (unsigned int)*(volatile unsigned char *)(iVar3 + 0x1b);
        *(short *)(unaff_r7 * 2) = (short)iVar4;
        *(int *)(param_2 + 0x14) = iVar4;
        
        // Interceptação preventiva do desvio indireto truncado (Jumptable 0x000071a2)
        if ((uVar5 & 0x40000000) != 0 && (uVar5 & 0x3fffffff) != 0) {
            return;
        }
    }
    else {
        uVar2 = 0x7227;
        iVar4 = 0x7245;
        
        // O Ghidra descompilou chamadas via LR de posições fixas, 
        // em C puro de nível de aplicação, nós stubamos esses retornos curtos.
        iVar4 = 0; 
        iVar3 = unaff_r7 * 2;
        uVar2 = iVar4 + 0x21;
        *unaff_r5 = uVar2;
        unaff_r5[1] = (unsigned int)unaff_r5;
    }
    
    *(short *)(iVar3 + 0x1e) = (short)uVar2;
    
    // Encerra o bloco protegendo o fluxo
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007398
   DESCRIÇÃO: Escrita de Estrutura de Fluxo e Gatilho de Instrução Indefinida (UDF)
   ========================================================================== */

// Protótipo para a instrução indefinida gerada por software (trap)
extern unsigned int software_udf(unsigned char op, unsigned short addr);

void FUN_00007398(int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4)
{
    unsigned short uVar2;
    int iVar3;
    int unaff_r4 = 0;
    int in_lr = 0;
    
    uVar2 = (unsigned short)(param_4 >> 7);
    *(unsigned short *)(param_1 + 0x22) = uVar2;
    iVar3 = param_4 << 0x1f;
    
    // Configuração dos ponteiros de cabeçalho da estrutura
    *(int *)param_1 = param_1;
    *(int *)(param_1 + 4) = iVar3;
    *(int *)(param_1 + 8) = unaff_r4;
    *(unsigned short *)(param_1 + 0x22) = uVar2;
    
    uVar2 = 0;
    *(short *)(unaff_r4 + 0x12) = (short)param_1 + 0x48;
    
    // Atualiza a tabela de canais baseada no deslocamento do índice (iVar3)
    *(unsigned short *)(iVar3 * 4 + 0x1a) = uVar2;
    
    // Aciona a interrupção por instrução indefinida para precaver desvios corrompidos
    software_udf(0xdb, 0x73b6);
    
    // Força a parada segura para o compilador C saber que o fluxo termina aqui
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007abc
   DESCRIÇÃO: Inicialização de Estrutura de Controle e Deslocamento Bitwise
   ========================================================================= */

// Declaração da função externa mapeada pelo Ghidra
extern unsigned long long FUN_001028f4(unsigned char p1, unsigned int p2);
extern unsigned int NotUsed;

void FUN_00007abc(unsigned int param_1, unsigned int param_2, int param_3, int *param_4)
{
    int iVar1;
    unsigned short extraout_r1 = 0;
    unsigned int uVar2;
    int unaff_r4 = 0;
    int iVar3;
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    int unaff_r7 = 0;
    unsigned int uVar4;
    int in_lr = 0;
    int iVar5;
    unsigned long long uVar6;
    
    *(int *)(unaff_r6 + 0x6c) = param_3 << 0x17;
    
    // Preenchimento do vetor/ponteiro de controle de estado
    param_4[0] = unaff_r4 << 4;
    param_4[1] = (int)param_4;
    param_4[2] = unaff_r4;
    param_4[3] = unaff_r5;
    param_4[4] = unaff_r6;
    param_4[5] = unaff_r7;
    
    *(char *)(unaff_r7 + 2) = (char)unaff_r5;
    iVar3 = (int)param_4 << 0x10;
    param_4[1] = unaff_r7;
    uVar2 = NotUsed;
    iVar5 = 0x7af3;
    
    uVar4 = (unsigned int)*(volatile unsigned short *)(unaff_r7 * 2);
    param_4[0x11] = uVar4;
    param_4[0x10] = iVar3;
    iVar1 = 0x7b0f;
    
    // Chamada externa de 64-bits
    uVar6 = FUN_001028f4(0x1c, uVar2 << 3);
    *(int *)(uVar4 + 0xc) = (int)(uVar6 >> 0x20);
    param_4[1] = uVar4;
    iVar3 = 0x7b17;
    
    *(unsigned short *)(uVar4 + (int)param_4) = extraout_r1;
    iVar1 = 0x7b21;
    
    // Força a interrupção segura do fluxo de instruções
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007b4c
   DESCRIÇÃO: Verificação de Flags de Status ARM (NZCV) e Chamada de SWI
   ========================================================================== */

// Protótipos externos exigidos por esta rotina de baixo nível
extern void software_interrupt(unsigned int swi_code);
extern void func_0xffdd9f4c(void);

unsigned long long FUN_00007b4c(unsigned int param_1, unsigned int param_2, int param_3)
{
    int unaff_r8 = 0;
    int unaff_r10 = 0;
    int in_NG = 0; // Flag Negative
    int in_ZR = 0; // Flag Zero
    int in_CY = 0; // Flag Carry
    int in_OV = 0; // Flag Overflow
    int bVar1;
    
    if (!in_NG) {
        *(char *)(unaff_r8 - (param_3 >> 2)) = (char)unaff_r8;
    }
    
    bVar1 = 0;
    if (in_ZR) {
        // Simulação matemática dos bits de controle de estouro de registrador
        in_OV = SBORROW4(unaff_r10, 0x3c0000) != SBORROW4(unaff_r10 + -0x3c0000, (unsigned int)!in_CY);
        in_NG = (int)(unaff_r10 - (!in_CY + 0x3c0000)) < 0;
        bVar1 = (unaff_r10 == (!in_CY + 0x3c0000));
    }
    
    if (in_NG) {
        if (bVar1) {
            unsigned int low = *(unsigned int *)(unaff_r10 + -0x2c);
            unsigned int high = *(unsigned int *)(unaff_r10 + -0x28);
            return ((unsigned long long)high << 32) | low; // Substitui o CONCAT44 por deslocamento puro de 64-bits
        }
        
        if (!in_OV) {
            // Dispara a interrupção por software nativa do Nintendo DS (SWI)
            software_interrupt(0xfa6061);
        }
        
        func_0xffdd9f4c();
        halt_baddata();
    }
    
    halt_baddata();
}

void FUN_00007c18(undefined4 *param_1)

{
  int unaff_r7;
  undefined4 unaff_r8;
  bool in_CY;
  undefined4 in_cr0;
  undefined4 in_cr1;
  undefined4 in_cr2;
  undefined4 in_cr6;
  undefined4 in_cr12;
  
  if (!in_CY) {
    coprocessor_loadlong(6,in_cr2,param_1);
    coprocessor_loadlong(6,in_cr0,param_1 + 0x1f6);
    param_1 = param_1 + 0x2f4;
    coprocessor_loadlong(6,in_cr1,param_1);
  }
  *param_1 = unaff_r8;
  if (in_CY) {
    coprocessor_loadlong2(6,in_cr12,unaff_r7);
    coprocessor_loadlong2(0xe,in_cr6,unaff_r7 + 1000);
                    // WARNING: Bad instruction - Truncating control flow here
    halt_baddata();
  }
                    // WARNING: Bad instruction - Truncating control flow here
  halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007ccc
   DESCRIÇÃO: Alocação Avançada de Linhas de Buffer e Despacho de Fluxo
   ========================================================================== */

void FUN_00007ccc(int param_1, int param_2)
{
    unsigned short uVar1;
    unsigned char uVar2;
    int *piVar3;
    int unaff_r4 = 0;
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    int unaff_r7 = 0;
    int in_lr = 0;
    int iVar4;
    unsigned long long uVar5;
    
    uVar1 = *(unsigned short *)(unaff_r7 + 4);
    
    // Aloca e indexa o bloco na tabela de canais (deslocamento de 32 bytes)
    piVar3 = (int *)(param_2 * 0x20);
    *(short *)(unaff_r5 + 2) = (short)(int)piVar3;
    uVar2 = (unsigned char)uVar1;
    *(unsigned char *)((int)piVar3 + 1) = uVar2;
    
    // Preenchimento dos metadados do primeiro buffer
    *piVar3 = param_2;
    piVar3[1] = param_1 >> 0x18;
    piVar3[2] = unaff_r6;
    
    iVar4 = 0x7cdb;
    uVar5 = 0; // Abstração segura para chamada longa de dispatch
    
    // Processamento do segundo bloco indexado via registrador de hardware
    piVar3 = (int *)(unsigned int)*(volatile unsigned short *)(unaff_r4 + 2);
    *(unsigned char *)((int)piVar3 + 1) = uVar2;
    
    // Preenchimento dos metadados do segundo buffer usando os 32 bits altos de uVar5
    *piVar3 = (int)(uVar5 >> 0x20) << 5;
    piVar3[1] = (int)uVar5 >> 0x1c;
    piVar3[2] = unaff_r6;
    
    // Encerra protegendo contra dados corrompidos
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007d1a
   DESCRIÇÃO: Mapeamento Mestre do Bloco de IO, Buffers Globais e Setup de ROM
   ========================================================================== */

// Definições de Memória Global estáticas encontradas no binário original
extern unsigned short DAT_00008156, _DAT_0000813a, DAT_0000813a, DAT_00008136;
extern unsigned int DAT_00008120, _DAT_00008128, _DAT_00008124, _DAT_00008124, _DAT_00008136;
extern unsigned int DAT_00002048, DAT_00022d18, _Reset, ram0x00000004, ram0x00000008;
extern unsigned int DAT_00114300, SUB_00114304, SUB_00114308, _DAT_10e0084d;
extern unsigned int DAT_00004514, DAT_00008a1c, DAT_00008a20, _DAT_00002068, DAT_0000206e;
extern unsigned short sRam00000030, _NotUsed, DAT_0000207a, DAT_00002066;
extern unsigned char uRam00000025, unique0x1000028d, unique0x10000289;
extern unsigned int _DAT_89181728, _DAT_89181724, DAT_8918171c, DAT_02dc1299, _DAT_02dc12a1;
extern int *_DAT_02dc129d;
extern unsigned int DAT_00002064, DAT_00004510, _DAT_0000450c, _DAT_00114300;

// Protótipos externos
extern void FUN_000bd5fa(void);
extern void FUN_0011430c(void);

void FUN_00007d1a(unsigned int param_1, unsigned int param_2, int param_3)
{
    unsigned short uVar1, uVar6;
    unsigned char uVar2;
    unsigned int *puVar3, *puVar4, *puVar5;
    unsigned int extraout_r1 = 0;
    int extraout_r1_00 = 0, extraout_r1_01 = 0;
    int *piVar7;
    unsigned int uVar8, uVar10, uVar11, uVar13, uVar18;
    unsigned char *puVar9, *puVar16, *puVar14;
    unsigned short *puVar15;
    int iVar12, iVar17, iVar19, iVar20, iVar21;
    int unaff_r4 = 0, unaff_r6 = 0, unaff_r9 = 0;
    unsigned int unaff_r8 = 0;
    int in_lr = 0;
    
    // Variáveis locais para simular offsets de Pilha/Stack e registradores voláteis
    int in_stack_000003e0 = 0, iStack_180 = 0;
    unsigned int uStack_178 = 0;
    unsigned int stack_base = 0; 
    
    iVar12 = 0xf8;
    iVar17 = 0x7d33;
    uVar18 = 0x7d3b;
    iVar17 = 0x7d3d;
    iVar19 = 0x7d49;
    
    DAT_00008156 = (unsigned short)extraout_r1;
    _DAT_0000813a = (DAT_0000813a << 8) | 0x20;
    
    puVar3 = (unsigned int *)((unaff_r4 + -0xf0) * 0x10);
    *puVar3 = extraout_r1;
    puVar3[1] = in_stack_000003e0 + -0xf0;
    puVar3[2] = stack_base;
    
    iVar12 = 0x7d69;
    iVar17 = 0x7d6b;
    iVar12 = 0; 
    uVar18 = iVar12 >> 0x1c;
    iVar20 = 0x7d73;
    
    DAT_00008120 = (unaff_r4 + -0xf1) * 0x10;
    DAT_00008136 = (unsigned short)uVar18;
    _DAT_00008128 = stack_base + ((DAT_00008120 & 0x10) != 0) + uVar18;
    *(unsigned short *)(_DAT_00008128 + 0x32) = 0x690;
    _DAT_00008124 = (unsigned int)&DAT_00002048;
    
    iVar12 = 0;
    iVar19 = 0;
    iVar17 = 0x83d7;
    _DAT_0000813a = DAT_00008136;
    uVar6 = 0;
    uVar8 = 0x204102;
    
    *(unsigned short *)(puVar3 + 4) = uVar6;
    puVar3[0x10] = iVar12 + uVar18;
    *(unsigned short *)((unsigned int)&DAT_00002048 + (uVar18 >> 0xc)) = (unsigned short)((iVar12 + uVar18) >> 0x10);
    
    iVar20 = 0x83f1;
    iVar12 = 0;
    uVar2 = (unsigned char)iVar19;
    iVar17 = (uVar18 >> 0xc ^ uVar8) + uVar8;
    puVar9 = (unsigned char *)&DAT_0022d18;
    
    *(unsigned short *)(puVar3 + 4) = (unsigned short)iVar12;
    puVar3[0x10] = iVar17;
    iVar21 = 0x8415;
    puVar16 = puVar9 + (uVar8 >> 0xc ^ (unsigned int)puVar9);
    uVar8 = (unsigned int)puVar9 >> 0xc;
    uVar18 = 0xfd00498;
    
    *(unsigned short *)(puVar3 + 4) = (unsigned short)iVar12;
    puVar3[0x10] = (unsigned int)puVar16;
    
    // Tratamento preventivo da flag de interrupção
    if (((int)puVar16 >> 0xf & 1) == 0) {
        *(char *)(puVar3 + 6) = (char)(int)puVar16;
        software_bkpt(0x42);
        puVar4 = (unsigned int *)((int)puVar3 * 0x40000);
        *puVar4 = (unsigned int)puVar3;
        puVar4[1] = (unsigned int)&DAT_00002048;
        puVar4[2] = uVar8;
        _Reset = (int)puVar3 << 0x12;
        ram0x00000004 = (unsigned int)&DAT_00002048;
        ram0x00000008 = uVar8 - 0xf0;
        halt_baddata();
    }
    
    iVar20 = 0x8439;
    iVar17 = (uVar8 ^ uVar18) + uVar18;
    *(unsigned short *)(puVar3 + 4) = (unsigned short)iVar12;
    puVar3[0x10] = iVar17;
    uVar10 = (int)(uVar18 >> 0xc) >> 0xd;
    iVar21 = 0x845d;
    uVar11 = 0x129c1294;
    *(unsigned short *)(puVar3 + 4) = (unsigned short)iVar12;
    puVar3[0x10] = (uVar18 >> 0xc ^ uVar10) + uVar10;
    iVar17 = iVar19 + uVar11;
    uVar8 = iVar17 * 0x10000;
    uVar11 = uVar10 >> 0xc ^ uVar11;
    uVar18 = *(unsigned short *)(uVar11 + 0xc);
    puVar9 = (unsigned char *)(uVar18 - iVar17);
    
    if (iVar17 * 0x1000 < 0 == SBORROW4(uVar18, iVar17)) goto LAB_000084be;
    *(char *)(iStack_180 + iVar17 * 0x1000) = (char)*(unsigned short *)(uVar11 + 0xc);
    
    if ((uStack_178 << 0x12 & 0x40000) == 0) {
        puVar9 = (unsigned char *)&DAT_00020480;
        uVar8 = 0x1024000;
        *(unsigned short *)(uVar11 + uStack_178) = (unsigned short)((uStack_178 >> 0x1c) << 7);
        *(unsigned int *)(iVar19 + 0x1c) = 0x1024000;
        
        do {
            uVar18 = 0x60;
LAB_000084be:
            uVar11 = *(unsigned int *)(uVar18 + 0x14);
            *(char *)(uVar11 + uStack_178) = (char)uVar11;
            *(unsigned int *)(uVar18 + 0x20) = uVar18 + 0x20;
            *(unsigned int *)(uVar18 + 0x24) = uVar8;
            *(unsigned int **)(uVar18 + 0x28) = (unsigned int *)puVar9;
            *(unsigned int *)(uVar18 + 0x2c) = uStack_178;
            *(unsigned int *)(uVar18 + 0x30) = uVar11;
            uVar8 = (unsigned int)(*(unsigned short *)(uStack_178 * 0x4000 + 2) >> 3);
            uVar18 = uStack_178 & 0x40000;
            puVar9 = (unsigned char *)(uStack_178 << 0xe);
            uVar10 = uStack_178 << 0x1a ^ uVar11;
            *(unsigned short *)(uVar10 * 2) = (unsigned short)uVar11;
            uStack_178 = (unsigned int)*(volatile unsigned char *)(uVar10 + 9);
        } while (uVar18 != 0);
        halt_baddata();
    }
    
    sRam00000030 = (unsigned short)uVar11;
    *(unsigned int *)(iVar19 + 0x48) = uStack_178 << 0x12;
    uVar10 = 0;
    iVar19 = 0x8583;
    FUN_000bd5fa();
    
    uVar1 = *(unsigned short *)(uVar10 + (*(int *)(unaff_r9 + extraout_r1_00 * 0x400000 + 0x44) >> 0x16));
    uVar13 = uVar10 * 0x4000;
    _NotUsed = (unsigned short)uVar10;
    *(unsigned short *)((-4 - (unsigned int)uVar1) * 0x200) = 0;
    *(unsigned short *)(uVar10 * 0x1000000) = 0;
    *(unsigned short *)(uVar10 * 0x5000000) = uVar1;
    *(unsigned int **)(uVar13 + 0xc) = &DAT_00002048;
    _DAT_00002068 = (unsigned short *)((((unsigned int)_DAT_00002068) << 16) | ((short)uVar11 + -0x20));
    DAT_0000207a = (unsigned short)(uVar11 - 0x40);
    
    puVar5 = (unsigned int *)((uVar10 & 0x3ffff) * 0x10);
    puVar5[0xc] = (unsigned int)puVar5;
    *puVar5 = (unsigned int)uVar1;
    puVar5[1] = uVar13;
    puVar5[2] = (uVar10 & 0x3ffff) << 6;
    puVar5[3] = uVar11 - 0x40;
    *(unsigned int *)(uVar10 * 0x404000) = 0;
    *(unsigned int *)((unsigned int)&DAT_00002048 + *(unsigned short *)(uVar13 + 8)) = (unsigned int)*(unsigned short *)(uVar13 + 8);
    
    uVar8 = (unsigned int)*(unsigned short *)(uVar13 + 8);
    *(unsigned int *)((unsigned int)&DAT_00002048 + uVar8) = uVar8;
    piVar7 = (int *)(uVar18 >> 2);
    *(unsigned int *)(uVar8 + 0x14) = uVar11 - 0x61;
    *(unsigned short *)(uVar18 + 8) = (unsigned short)uVar18;
    *piVar7 = uVar10 << 0x14;
    piVar7[1] = (int)piVar7;
    piVar7[2] = 0;
    piVar7[3] = iVar12;
    uRam00000025 = 0;
    unique0x1000028d = uVar11 - 0x61;
    *(unsigned int *)(uVar10 * 0x404000) = 0;
    
    *(unsigned int *)((unsigned int)&DAT_00002064 + *(unsigned short *)(uVar13 + 8)) = (unsigned int)*(unsigned short *)(uVar13 + 8);
    uVar11 = uVar11 - 0x82;
    piVar7 = _DAT_02dc129d;
    *(char *)(*(unsigned short *)(uVar13 + 8) + 5) = (char)uVar11;
    *(unsigned short *)(iVar12 + 8) = (unsigned short)iVar12;
    *piVar7 = uVar10 << 0x10;
    piVar7[1] = (int)piVar7;
    piVar7[2] = 0;
    piVar7[3] = (int)uVar13 >> 0x1c;
    
    puVar14 = (unsigned char *)(unaff_r8 + (unaff_r8 >> 2) + (unsigned int)((unaff_r8 & 0x10000000) != 0));
    *puVar14 = (char)(unaff_r8 >> 2);
    iVar12 = unaff_r8 * -0x10;
    *(unsigned char **)(iVar12 + 0x40c8) = puVar14;
    
    iVar17 = ((unsigned int)puVar14 >> 6) - 0x1c;
    puVar15 = (unsigned short *)(unsigned int)*(volatile unsigned char *)(iVar12 + 0x206d);
    *puVar15 = (unsigned short)(unsigned int)(&DAT_00002064 + iVar12);
    unique0x10000289 = uVar11;
    _Reset = 0;
    
    *(unsigned short *)((int)puVar15 * 0x40000000 + 0x22) = (unsigned short)iVar17;
    iVar12 = 0x86b1;
    iVar19 = 0x86b3;
    
    puVar3 = (unsigned int *)(extraout_r1_01 * 0x20);
    *puVar3 = (unsigned int)puVar15;
    puVar3[1] = (unsigned int)&DAT_00002064;
    puVar3[2] = uVar11;
    *(unsigned char *)((int)puVar3 + 0xd) = 0x29;
    puVar3[3] = extraout_r1_01;
    puVar3[4] = iVar17;
    puVar3[5] = uVar11;
    
    piVar7 = (int *)(extraout_r1_01 * 0x20);
    DAT_00002066 = (unsigned short)(int)piVar7;
    *(unsigned char *)((int)piVar7 + 1) = 0x29;
    *piVar7 = extraout_r1_01;
    piVar7[1] = ((int)puVar3 + 0xd2) >> 0x18;
    piVar7[2] = uVar11;
    DAT_00004510 = ((int)piVar7 + 0xc6) >> 0x18;
    _DAT_0000450c = extraout_r1_01 << 5;
    _DAT_00114300 = DAT_00008a1c;
    SUB_00114304 = 0;
    SUB_00114308 = DAT_00008a20;
    _DAT_10e0084d = 0;
    DAT_00004514 = uVar11;
    
    _DAT_00002068 = (unsigned int)puVar15;
    DAT_0000206e = 0x207c;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007d38
   DESCRIÇÃO: Variante Dinâmica do Mapeamento de Bloco de IO e Setup de ROM
   ========================================================================== */

void FUN_00007d38(void)
{
    unsigned short uVar1, uVar6;
    unsigned char uVar2;
    unsigned int *puVar3, *puVar4, *puVar5;
    unsigned int extraout_r1 = 0;
    int extraout_r1_00 = 0, extraout_r1_01 = 0;
    int *piVar7;
    unsigned int uVar9, uVar10, uVar13, uVar15, uVar16, uVar18;
    unsigned char *puVar8, *puVar11, *puVar14;
    unsigned short *puVar12;
    int iVar17, iVar19, iVar20, iVar21, iVar22;
    
    // Simulação dos registradores de hardware não definidos pelo Ghidra
    int in_r3 = 0;
    int unaff_r4 = 0;
    unsigned int unaff_r6 = 0;
    unsigned int *unaff_r7 = (unsigned int *)0;
    unsigned int unaff_r8 = 0;
    int unaff_r9 = 0;
    int in_lr = 0;
    int in_stack_000003e0 = 0, iStack_180 = 0;
    unsigned int uStack_178 = 0;
    unsigned int stack_base = 0;
    
    uVar18 = 0x7d3b;
    iVar19 = 0x7d3d;
    *(char *)(unaff_r6 + 0xf) = (char)(unaff_r4 << 4);
    iVar20 = 0x7d49;
    
    *(short *)((int)unaff_r7 + 0x36) = (short)extraout_r1;
    *(unsigned int *)(unaff_r6 + 0x6c) = unaff_r6 << 3;
    *(short *)(unaff_r6 + in_r3) = (short)(unaff_r6 << 3);
    *(char *)((int)unaff_r7 + 0x1b) = (char)(int)unaff_r7;
    
    puVar3 = (unsigned int *)((unaff_r4 + -0xf0) * 0x10);
    uVar15 = (unsigned int)unaff_r7 >> 2;
    *puVar3 = extraout_r1;
    puVar3[1] = in_stack_000003e0 + -0xf0;
    puVar3[2] = unaff_r6;
    
    iVar19 = 0x7d69;
    iVar20 = 0x7d6b;
    iVar19 = 0;
    uVar18 = iVar19 >> 0x1c;
    iVar21 = 0x7d73;
    
    uVar13 = (unaff_r4 + -0xf1) * 0x10;
    uVar6 = (unsigned short)uVar18;
    *(unsigned short *)((int)unaff_r7 + 10) = uVar6;
    *(unsigned short *)((int)unaff_r7 + 6) = uVar6;
    
    uVar16 = unaff_r6 + uVar18 + (unsigned int)((uVar13 & 0x10) != 0);
    *(unsigned short *)(uVar16 + 0x32) = 0x690;
    *unaff_r7 = uVar13;
    unaff_r7[1] = uVar15;
    unaff_r7[2] = uVar16;
    *(unsigned short *)((int)unaff_r7 + 0x16) = uVar6;
    
    iVar19 = *(int *)(uVar18 + 0x24);
    *(unsigned short *)((int)unaff_r7 + 0x1a) = uVar6;
    iVar17 = *(int *)(iVar19 + 0x14);
    iVar20 = 0x83d7;
    uVar6 = 0;
    uVar13 = 0x204102;
    
    *(unsigned short *)(puVar3 + 4) = uVar6;
    puVar3[0x10] = iVar19 + uVar18;
    *(short *)((uVar18 >> 0xc) + uVar15) = (short)((iVar19 + uVar18) >> 0x10);
    
    iVar21 = 0x83f1;
    iVar19 = 0;
    uVar2 = (unsigned char)iVar17;
    iVar20 = (uVar18 >> 0xc ^ uVar13) + uVar13;
    puVar8 = (unsigned char *)&DAT_00022d18;
    
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = iVar20;
    iVar22 = 0x8415;
    puVar14 = puVar8 + (uVar13 >> 0xc ^ (unsigned int)puVar8);
    uVar13 = (unsigned int)puVar8 >> 0xc;
    uVar18 = 0xfd00498;
    
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = (unsigned int)puVar14;
    
    if (((int)puVar14 >> 0xf & 1) == 0) {
        *(char *)(puVar3 + 6) = (char)(int)puVar14;
        software_bkpt(0x42);
        puVar4 = (unsigned int *)((int)puVar3 * 0x40000);
        *puVar4 = (unsigned int)puVar3;
        puVar4[1] = uVar15;
        puVar4[2] = uVar13;
        _Reset = (int)puVar3 << 0x12;
        ram0x00000004 = uVar15;
        ram0x00000008 = uVar13 - 0xf0;
        halt_baddata();
    }
    
    iVar21 = 0x8439;
    iVar20 = (uVar13 ^ uVar18) + uVar18;
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = iVar20;
    uVar16 = (int)(uVar18 >> 0xc) >> 0xd;
    iVar22 = 0x845d;
    uVar9 = 0x129c1294;
    
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = (uVar18 >> 0xc ^ uVar16) + uVar16;
    iVar20 = iVar17 + uVar9;
    uVar13 = iVar20 * 0x10000;
    uVar9 = uVar16 >> 0xc ^ uVar9;
    uVar18 = *(unsigned short *)(uVar9 + 0xc);
    iVar19 = uVar18 - iVar20;
    
    if (iVar20 * 0x1000 < 0 == SBORROW4(uVar18, iVar20)) goto LAB_000084be;
    *(char *)(iStack_180 + iVar20 * 0x1000) = (char)*(unsigned short *)(uVar9 + 0xc);
    
    if ((uStack_178 << 0x12 & 0x40000) == 0) {
        iVar19 = uVar15 << 4;
        uVar13 = uVar15 << 0xb;
        *(short *)(uVar9 + uStack_178) = (short)((uStack_178 >> 0x1c) << 7);
        *(unsigned int *)(iVar17 + 0x1c) = uVar13;
        
        do {
            uVar18 = 0x60;
LAB_000084be:
            uVar16 = *(unsigned int *)(uVar18 + 0x14);
            *(char *)(uVar16 + uStack_178) = (char)uVar16;
            *(unsigned int *)(uVar18 + 0x20) = uVar18 + 0x20;
            *(unsigned int *)(uVar18 + 0x24) = uVar13;
            *(int *)(uVar18 + 0x28) = iVar19;
            *(unsigned int *)(uVar18 + 0x2c) = uStack_178;
            *(unsigned int *)(uVar18 + 0x30) = uVar16;
            uVar13 = (unsigned int)(*(unsigned short *)(uStack_178 * 0x4000 + 2) >> 3);
            uVar18 = uStack_178 & 0x40000;
            iVar19 = uStack_178 << 0xe;
            uVar15 = uStack_178 << 0x1a ^ uVar16;
            *(short *)(uVar15 * 2) = (short)uVar16;
            uStack_178 = (unsigned int)*(volatile unsigned char *)(uVar15 + 9);
        } while (uVar18 != 0);
        halt_baddata();
    }
    
    sRam00000030 = (short)uVar9;
    *(unsigned int *)(iVar17 + 0x48) = uStack_178 << 0x12;
    uVar16 = 0;
    iVar21 = 0x8583;
    FUN_000bd5fa();
    
    uVar1 = *(unsigned short *)(uVar16 + (*(int *)(unaff_r9 + extraout_r1_00 * 0x400000 + 0x44) >> 0x16));
    uVar10 = uVar16 * 0x4000;
    _NotUsed = (unsigned short)uVar16;
    *(unsigned short *)((-4 - (unsigned int)uVar1) * 0x200) = 0;
    *(unsigned short *)(uVar16 * 0x1000000) = 0;
    *(unsigned short *)(uVar16 * 0x5000000) = uVar1;
    *(unsigned int *)(uVar10 + 0xc) = uVar15;
    *(short *)(uVar15 + 0x22) = (short)uVar9 + -0x20;
    *(short *)(uVar15 + 0x32) = (short)(uVar9 - 0x40);
    
    puVar5 = (unsigned int *)((uVar16 & 0x3ffff) * 0x10);
    puVar5[0xc] = (unsigned int)puVar5;
    *puVar5 = (unsigned int)uVar1;
    puVar5[1] = uVar10;
    puVar5[2] = (uVar16 & 0x3ffff) << 6;
    puVar5[3] = uVar9 - 0x40;
    *(unsigned int *)(uVar16 * 0x404000) = 0;
    *(unsigned int *)(*(unsigned short *)(uVar10 + 8) + uVar15) = (unsigned int)*(unsigned short *)(uVar10 + 8);
    *(unsigned char **)(uVar10 + ram0x0000000c) = (unsigned char *)&DAT_8918171c;
    
    iVar19 = _DAT_89181728;
    uVar18 = _DAT_89181724;
    uVar13 = (unsigned int)*(unsigned short *)(uVar10 + 8);
    *(unsigned int *)(uVar13 + uVar15) = uVar13;
    piVar7 = (int *)(uVar18 >> 2);
    *(unsigned int *)(uVar13 + 0x14) = uVar9 - 0x61;
    *(unsigned short *)(uVar18 + 8) = (unsigned short)uVar18;
    *piVar7 = uVar16 << 0x14;
    piVar7[1] = (int)piVar7;
    piVar7[2] = 0;
    piVar7[3] = iVar19;
    
    iVar17 = uVar15 + 0x1c;
    uRam00000025 = 0;
    unique0x10000264 = uVar9 - 0x61;
    *(unsigned int *)(uVar16 * 0x404000) = 0;
    iVar19 = ram0x0000000c;
    *(unsigned int *)((unsigned int)*(unsigned short *)(uVar10 + 8) + iVar17) = (unsigned int)*(unsigned short *)(uVar10 + 8);
    
    uVar9 = uVar9 - 0x82;
    *(unsigned char **)(uVar10 + iVar19) = (unsigned char *)&DAT_02dc1299;
    iVar19 = _DAT_02dc12a1;
    piVar7 = _DAT_02dc129d;
    *(char *)(*(unsigned short *)(uVar10 + 8) + 5) = (char)uVar9;
    *(unsigned short *)(iVar19 + 8) = (unsigned short)iVar19;
    *piVar7 = uVar16 << 0x10;
    piVar7[1] = (int)piVar7;
    piVar7[2] = 0;
    piVar7[3] = (int)uVar10 >> 0x1c;
    
    puVar11 = (unsigned char *)(unaff_r8 + (unaff_r8 >> 2) + (unsigned int)((unaff_r8 & 0x10000000) != 0));
    *puVar11 = (char)(unaff_r8 >> 2);
    iVar19 = iVar17 + unaff_r8 * -0x10;
    *(unsigned char **)(iVar19 + iVar17) = puVar11;
    *(unsigned char **)(iVar19 + (int)piVar7) = puVar11;
    
    iVar20 = ((unsigned int)puVar11 >> 6) - 0x1c;
    puVar12 = (unsigned short *)(unsigned int)*(volatile unsigned char *)(iVar19 + 9);
    *puVar12 = (unsigned short)iVar19;
    register0x00000030 = (unsigned char *)((int)puVar12 << 0xb);
    _Reset = 0;
    unique0x10000260 = uVar9;
    *(unsigned int *)(puVar12 + 0x20) = (unsigned int)register0x00000030;
    *(unsigned short *)((int)puVar12 * 0x40000000 + 0x22) = (unsigned short)iVar20;
    
    iVar19 = 0x86b1;
    iVar21 = 0x86b3;
    
    puVar3 = (unsigned int *)(extraout_r1_01 * 0x20);
    *puVar3 = (unsigned int)puVar12;
    puVar3[1] = iVar17;
    puVar3[2] = uVar9;
    *(unsigned char *)((int)puVar3 + 0xd) = 0x29;
    puVar3[3] = extraout_r1_01;
    puVar3[4] = iVar20;
    puVar3[5] = uVar9;
    
    piVar7 = (int *)(extraout_r1_01 * 0x20);
    *(unsigned short *)(uVar15 + 0x1e) = (unsigned short)(int)piVar7;
    *(unsigned char *)((int)piVar7 + 1) = 0x29;
    *piVar7 = extraout_r1_01;
    piVar7[1] = ((int)puVar3 + 0xd2) >> 0x18;
    piVar7[2] = uVar9;
    
    DAT_00004510 = ((int)piVar7 + 0xc6) >> 0x18;
    _DAT_0000450c = extraout_r1_01 << 5;
    _DAT_00114300 = DAT_00008a1c;
    SUB_00114304 = 0;
    SUB_00114308 = DAT_00008a20;
    _DAT_10e0084d = 0;
    DAT_00004514 = uVar9;
    
    *(unsigned short **)(uVar15 + 0x20) = puVar12;
    *(unsigned short *)(uVar15 + 0x26) = (unsigned short)iVar17 + 0x18;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007dae
   DESCRIÇÃO: Manipulador de Fluxo de IO com Backup de Estado e Deslocamento 64-bit
   ========================================================================== */

void FUN_00007dae(unsigned int param_1, unsigned int param_2, unsigned int param_3, int param_4)
{
    unsigned short uVar1, uVar7;
    unsigned char uVar2;
    unsigned int *puVar3, *puVar4, *puVar5;
    short sVar6;
    unsigned int extraout_r1 = 0;
    int extraout_r1_00 = 0, extraout_r1_01 = 0, extraout_r1_02 = 0;
    int *piVar8;
    unsigned int uVar10, uVar11, uVar12, uVar17, uVar18;
    unsigned char *puVar9, *puVar13, *puVar15;
    unsigned short *puVar14;
    int iVar16, iVar19, iVar20, iVar21, iVar22;
    unsigned long long uVar23;
    
    // Registradores não resolvidos e variáveis de pilha reconstruídas
    unsigned int unaff_r4 = 0;
    unsigned char *unaff_r5 = (unsigned char *)0;
    int unaff_r6 = 0;
    int unaff_r7 = 0;
    unsigned int unaff_r8 = 0;
    int unaff_r9 = 0;
    unsigned int in_lr = 0;
    int in_stack_000003e0 = 0, in_stack_000003fc = 0, iStack_180 = 0;
    unsigned int uStack_178 = 0;
    
    // Salvamento de contexto na RAM estática (Estrutura de Depuração/Crash Dump)
    FUN_00007f4c = 0x7f50;
    uVar17 = 0x7dc3;
    uRam00007f54 = param_3;
    iRam00007f58 = param_4;
    uRam00007f5c = unaff_r4;
    puRam00007f60 = unaff_r5;
    FUN_00007f62 = unaff_r6;
    iRam00007f68 = unaff_r7;
    
    uVar23 = ((unsigned long long)((int)unaff_r5 << 1) << 32) | (uint)*(unsigned short *)(unaff_r6 + 0xe);
    
    if (unaff_r6 != 0) {
        uVar18 = 0x7de3;
        unaff_r5 = (unsigned char *)&DAT_000081ec;
        iVar19 = 0x7dfb;
        iVar20 = 0x7e15;
        iVar19 = 0x7e17;
        uVar17 = 0x7e1b;
        
        // Simulação do retorno de cálculo de offset longo
        uVar23 = ((unsigned long long)iVar19 << 32) | (unsigned int)iVar20;
    }
    
    iVar19 = (int)uVar23 + 0xba;
    iVar21 = 0x7e23;
    sVar6 = (short)(iVar19 + (int)(uVar23 >> 32));
    uVar17 = extraout_r1_00 << 1;
    *(short *)(unaff_r6 + 0x1e) = sVar6 + 0x1c;
    
    uVar7 = *(unsigned short *)(extraout_r1_00 * 8 + 2);
    iVar20 = extraout_r1_00 * 0x1000;
    iVar16 = 0x7953;
    
    *(int *)(uVar17 + 0x40) = iVar20;
    *(unsigned short *)((int)unaff_r5 * 2 + 0x22) = uVar7;
    
    iVar19 = 0xf8;
    iVar21 = 0x7d33;
    uVar17 = 0x7d3b;
    iVar16 = 0x7d3d;
    iVar21 = 0x7d49;
    
    DAT_00008156 = (unsigned short)extraout_r1;
    _DAT_0000813a = (uRam0000813a & 0xFF00) | 0x20; 
    
    puVar3 = (unsigned int *)((iVar20 + -0xf0) * 0x10);
    *puVar3 = extraout_r1;
    puVar3[1] = in_stack_000003e0 + -0xf0;
    puVar3[2] = (unsigned int)&stack0x000003ec;
    
    iVar19 = 0x7d69;
    iVar16 = 0x7d6b;
    iVar19 = 0;
    uVar17 = iVar19 >> 0x1c;
    iVar21 = 0x7d73;
    
    DAT_00008120 = (iVar20 + -0xf1) * 0x10;
    DAT_00008136 = (unsigned short)uVar17;
    _DAT_00008128 = (unsigned int)&stack0x000003ec + ((DAT_00008120 & 0x10) != 0) + uVar17;
    *(unsigned short *)(_DAT_00008128 + 0x32) = 0x690;
    _DAT_00008124 = (unsigned int)&DAT_00002048;
    
    iVar19 = *(int *)(uVar17 + 0x24);
    iVar16 = *(int *)(iVar19 + 0x14);
    iVar20 = 0x83d7;
    
    uVar7 = 0;
    uVar18 = 0x204102;
    *(unsigned short *)(puVar3 + 4) = uVar7;
    puVar3[0x10] = iVar19 + uVar17;
    *(short *)((unsigned int)&DAT_00002048 + (uVar17 >> 0xc)) = (short)((iVar19 + uVar17) >> 0x10);
    
    iVar21 = 0x83f1;
    iVar19 = 0;
    uVar2 = (unsigned char)iVar16;
    iVar20 = (uVar17 >> 0xc ^ uVar18) + uVar18;
    puVar9 = (unsigned char *)&DAT_02dc1299; // Reajustado ponteiro base estático
    
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = iVar20;
    iVar22 = 0x8415;
    puVar15 = puVar9 + (uVar18 >> 0xc ^ (unsigned int)puVar9);
    uVar18 = (unsigned int)puVar9 >> 0xc;
    uVar17 = 0xfd00498;
    
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = (unsigned int)puVar15;
    
    if (((int)puVar15 >> 0xf & 1) == 0) {
        *(char *)(puVar3 + 6) = (char)(int)puVar15;
        software_bkpt(0x42);
        puVar4 = (unsigned int *)((int)puVar3 * 0x40000);
        *puVar4 = (unsigned int)puVar3;
        puVar4[1] = (unsigned int)&DAT_00002048;
        puVar4[2] = uVar18;
        _Reset = (int)puVar3 << 0x12;
        ram0x00000004 = (unsigned int)&DAT_00002048;
        ram0x00000008 = uVar18 - 0xf0;
        halt_baddata();
    }
    
    iVar21 = 0x8439;
    iVar20 = (uVar18 ^ uVar17) + uVar17;
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = iVar20;
    uVar10 = (int)(uVar17 >> 0xc) >> 0xd;
    iVar22 = 0x845d;
    uVar11 = 0x129c1294;
    
    *(short *)(puVar3 + 4) = (short)iVar19;
    puVar3[0x10] = (uVar17 >> 0xc ^ uVar10) + uVar10;
    iVar20 = iVar16 + uVar11;
    uVar18 = iVar20 * 0x10000;
    uVar11 = uVar10 >> 0xc ^ uVar11;
    uVar17 = *(unsigned short *)(uVar11 + 0xc);
    puVar9 = (unsigned char *)(uVar17 - iVar20);
    
    if (iVar20 * 0x1000 < 0 == SBORROW4(uVar17, iVar20)) goto LAB_000084be;
    *(char *)(iStack_180 + iVar20 * 0x1000) = (char)*(unsigned short *)(uVar11 + 0xc);
    
    if ((uStack_178 << 0x12 & 0x40000) == 0) {
        puVar9 = (unsigned char *)&DAT_00020480;
        uVar18 = 0x1024000;
        *(short *)(uVar11 + uStack_178) = (short)((uStack_178 >> 0x1c) << 7);
        *(unsigned int *)(iVar16 + 0x1c) = 0x1024000;
        
        do {
            uVar17 = 0x60;
LAB_000084be:
            uVar11 = *(unsigned int *)(uVar17 + 0x14);
            *(char *)(uVar11 + uStack_178) = (char)uVar11;
            *(unsigned int *)(uVar17 + 0x20) = uVar17 + 0x20;
            *(unsigned int *)(uVar17 + 0x24) = uVar18;
            *(unsigned char **)(uVar17 + 0x28) = puVar9;
            *(unsigned int *)(uVar17 + 0x2c) = uStack_178;
            *(unsigned int *)(uVar17 + 0x30) = uVar11;
            uVar18 = (unsigned int)(*(unsigned short *)(uStack_178 * 0x4000 + 2) >> 3);
            uVar17 = uStack_178 & 0x40000;
            puVar9 = (unsigned char *)(uStack_178 << 0xe);
            uVar10 = uStack_178 << 0x1a ^ uVar11;
            *(short *)(uVar10 * 2) = (short)uVar11;
            uStack_178 = (unsigned int)*(volatile unsigned char *)(uVar10 + 9);
        } while (uVar17 != 0);
        halt_baddata();
    }
    
    sRam00000030 = (short)uVar11;
    *(unsigned int *)(iVar16 + 0x48) = uStack_178 << 0x12;
    uVar10 = 0;
    iVar16 = 0x8583;
    FUN_000bd5fa();
    
    uVar1 = *(unsigned short *)(uVar10 + (*(int *)(unaff_r9 + extraout_r1_01 * 0x400000 + 0x44) >> 0x16));
    uVar12 = uVar10 * 0x4000;
    _NotUsed = (unsigned short)uVar10;
    *(unsigned short *)((-4 - (unsigned int)uVar1) * 0x200) = 0;
    *(unsigned short *)(uVar10 * 0x1000000) = 0;
    *(unsigned short *)(uVar10 * 0x5000000) = uVar1;
    *(unsigned int *)(uVar12 + 0xc) = (unsigned int)&DAT_00002048;
    
    _DAT_00002068 = (unsigned short *)(((unsigned int)_DAT_00002068 & 0xFFFF0000) | (unsigned short)((short)uVar11 + -0x20));
    DAT_0000207a = (unsigned short)(uVar11 - 0x40);
    
    puVar5 = (unsigned int *)((uVar10 & 0x3ffff) * 0x10);
    puVar5[0xc] = (unsigned int)puVar5;
    *puVar5 = (unsigned int)uVar1;
    puVar5[1] = uVar12;
    puVar5[2] = (uVar10 & 0x3ffff) << 6;
    puVar5[3] = uVar11 - 0x40;
    *(unsigned int *)(uVar10 * 0x404000) = 0;
    *(unsigned int *)(&DAT_00002048 + *(unsigned short *)(uVar12 + 8)) = (unsigned int)*(unsigned short *)(uVar12 + 8);
    *(unsigned char **)(uVar12 + ram0x0000000c) = (unsigned char *)&DAT_8918171c;
    
    iVar19 = _DAT_89181728;
    uVar17 = _DAT_89181724;
    uVar18 = (unsigned int)*(unsigned short *)(uVar12 + 8);
    *(unsigned int *)(&DAT_00002048 + uVar18) = uVar18;
    piVar8 = (int *)(uVar17 >> 2);
    *(unsigned int *)(uVar18 + 0x14) = uVar11 - 0x61;
    *(unsigned short *)(uVar17 + 8) = (unsigned short)uVar17;
    *piVar8 = uVar10 << 0x14;
    piVar8[1] = (int)piVar8;
    piVar8[2] = 0;
    piVar8[3] = iVar19;
    
    uRam00000025 = 0;
    unique0x100002e2 = uVar11 - 0x61;
    *(unsigned int *)(uVar10 * 0x404000) = 0;
    iVar19 = ram0x0000000c;
    *(unsigned int *)(&DAT_00002064 + *(unsigned short *)(uVar12 + 8)) = (unsigned int)*(unsigned short *)(uVar12 + 8);
    uVar11 = uVar11 - 0x82;
    *(unsigned char **)(uVar12 + iVar19) = (unsigned char *)&DAT_02dc1299;
    iVar19 = _DAT_02dc12a1;
    piVar8 = _DAT_02dc129d;
    *(char *)(*(unsigned short *)(uVar12 + 8) + 5) = (char)uVar11;
    *(unsigned short *)(iVar19 + 8) = (unsigned short)iVar19;
    *piVar8 = uVar10 << 0x10;
    piVar8[1] = (int)piVar8;
    piVar8[2] = 0;
    piVar8[3] = (int)uVar12 >> 0x1c;
    
    puVar13 = (unsigned char *)(unaff_r8 + (unaff_r8 >> 2) + (unsigned int)((unaff_r8 & 0x10000000) != 0));
    *puVar13 = (char)(unaff_r8 >> 2);
    iVar19 = unaff_r8 * -0x10;
    *(unsigned char **)(iVar19 + 0x40c8) = puVar13;
    *(unsigned char **)((unsigned int)&DAT_00002064 + iVar19 + (int)piVar8) = puVar13;
    
    iVar20 = ((unsigned int)puVar13 >> 6) - 0x1c;
    puVar14 = (unsigned short *)(unsigned int)*(volatile unsigned char *)(iVar19 + 0x206d);
    *puVar14 = (unsigned short)((unsigned int)&DAT_00002064 + iVar19);
    register0x00000030 = (unsigned char *)((int)puVar14 << 0xb);
    _Reset = 0;
    unique0x100002de = uVar11;
    *(unsigned char **)(puVar14 + 0x20) = register0x00000030;
    *(unsigned short *)((int)puVar14 * 0x40000000 + 0x22) = (unsigned short)iVar20;
    
    iVar19 = 0x86b1;
    iVar16 = 0x86b3;
    
    puVar3 = (unsigned int *)(extraout_r1_02 * 0x20);
    *puVar3 = (unsigned int)puVar14;
    puVar3[1] = (unsigned int)&DAT_00002064;
    puVar3[2] = uVar11;
    *(unsigned char *)((int)puVar3 + 0xd) = 0x29;
    puVar3[3] = extraout_r1_02;
    puVar3[4] = iVar20;
    puVar3[5] = uVar11;
    
    piVar8 = (int *)(extraout_r1_02 * 0x20);
    DAT_00002066 = (unsigned short)(int)piVar8;
    *(unsigned char *)((int)piVar8 + 1) = 0x29;
    *piVar8 = extraout_r1_02;
    piVar8[1] = ((int)puVar3 + 0xd2) >> 0x18;
    piVar8[2] = uVar11;
    
    DAT_00004510 = ((int)piVar8 + 0xc6) >> 0x18;
    _DAT_0000450c = extraout_r1_02 << 5;
    _DAT_00114300 = DAT_00008a1c;
    SUB_00114304 = 0;
    SUB_00114308 = DAT_00008a20;
    _DAT_10e0084d = 0;
    DAT_00004514 = uVar11;
    
    _DAT_00002068 = puVar14;
    DAT_0000206e = 0x207c;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007ed4
   DESCRIÇÃO: Consolidação de Stream, Sincronização de Interrupção e Finalização FIQ
   ========================================================================== */

void FUN_00007ed4(void)
{
    short extraout_var = 0;
    int *piVar1;
    unsigned int extraout_r1 = 0;
    int extraout_r1_00 = 0, extraout_r1_01 = 0, extraout_r1_02 = 0;
    char extraout_r1_03 = 0;
    unsigned char extraout_r1_04 = 0;
    int iVar2;
    unsigned int uVar3;
    unsigned int uVar5;
    int iVar6, iVar8, iVar9;
    int iVar10;
    unsigned long long uVar11;
    
    // Registradores e variáveis de pilha recuperados do ambiente Ghidra
    int in_r3 = 0;
    int unaff_r4 = 0;
    unsigned int *puVar4;
    unsigned int *unaff_r6 = (unsigned int *)0;
    int unaff_r7 = 0;
    int in_lr = 0;
    int in_stack_000003e4 = 0;
    
    iVar6 = 0x7ed9;
    iVar2 = unaff_r7 + -7;
    
    // Salvamento do Frame de Contexto Atual nos ponteiros de destino
    *unaff_r6 = extraout_r1;
    unaff_r6[1] = iVar2;
    unaff_r6[2] = in_r3;
    unaff_r6[3] = unaff_r4;
    unaff_r6[4] = unaff_r7 >> 2;
    unaff_r6[5] = (unsigned int)unaff_r6;
    unaff_r6[6] = unaff_r7;
    
    unsigned char *puVar7 = (unsigned char *)&FUN_00007eec_1;
    puVar4 = (unsigned int *)(unaff_r4 + 0xfe);
    uVar5 = *puVar4;
    iVar6 = *(int *)(unaff_r4 + 0x102);
    
    *(unsigned char *)(unaff_r4 + 0x107) = 0xfa;
    *(int *)(unaff_r4 + 0x106) = (extraout_r1_00 + 0xfe) * 0x20;
    *(int *)(unaff_r4 + 0x10a) = iVar2;
    *(int *)(unaff_r4 + 0x10e) = iVar6;
    
    iVar8 = 0x7f13;
    uVar11 = ((unsigned long long)iVar6 << 32) | (unaff_r4 + 0x112); // Simulação do retorno casado de 64-bit
    iVar2 = (int)(uVar11 >> 32);
    
    piVar1 = (int *)(iVar2 * 0x20);
    *piVar1 = in_r3;
    piVar1[1] = uVar5;
    piVar1[2] = iVar6;
    *(unsigned char *)((int)piVar1 + 0xd) = 0xfa;
    piVar1[3] = iVar2;
    piVar1[4] = (int)uVar11 >> 0x1c;
    piVar1[5] = iVar6;
    
    iVar2 = 0x7f1f;
    *(short *)((uVar5 >> 9) + 0x12) = extraout_var >> 0xc;
    *(unsigned char *)(((unsigned int)puVar4 >> 1) + 1) = 0xfa;
    
    piVar1 = (int *)(uVar5 * 2);
    *(unsigned char *)((int)piVar1 + 1) = 0xfa;
    *piVar1 = extraout_r1_01 << 9;
    piVar1[1] = (int)(((unsigned int)puVar4 >> 1) + 0xba) >> 0x18;
    piVar1[2] = iVar6;
    
    uVar3 = ((unsigned int)piVar1 + 0xc6) >> 0x18;
    iVar8 = 0x7f41;
    in_r3 = in_r3 + -0xf9;
    *(short *)(iVar6 + 0x1e) = (short)((unsigned int)puVar4 * 0x10);
    
    iVar8 = 0x7f55;
    DAT_00000126 = (DAT_00000126 & 0xFFFF0000) | (unsigned short)uVar5;
    iVar2 = extraout_r1_02;
    
    // Loop de escoamento e flushing do pipeline de IO
    do {
        iVar9 = 0x7f5d;
        DAT_000000fd = extraout_r1_03 + '\b';
        *(unsigned char *)(iVar6 + 0xf) = 0;
        iVar2 = 0x7f6b;
        iVar8 = 0x7f6f;
        iVar9 = uVar3 + 0x48;
        iVar2 = in_stack_000003e4 << 8;
        uVar3 = uVar3 + 0x138;
        DAT_00000119 = (unsigned char)uVar5;
    } while (uVar3 != 0 && -0xf1 < iVar9);
    
    // Escrita dos registradores de estado nos espelhos finais da memória
    piVar1 = (int *)(iVar6 * 0x80000);
    DAT_00000146 = in_r3 << 0x10;
    *piVar1 = DAT_00000146;
    piVar1[1] = uVar3 >> 0x1a | uVar3 * 0x40;
    piVar1[2] = in_r3;
    piVar1[3] = uVar5;
    piVar1[4] = iVar6;
    
    DAT_00000048 = (DAT_00000048 & 0xFFFF0000);
    iVar2 = (in_r3 + -0xe6) * 0x80;
    *(char *)(in_r3 + -0xd7) = (char)iVar2;
    
    iVar6 = 0x7fc1;
    FIQ = extraout_r1_04; // Aciona o flag de manipulação rápida de hardware externa
    
    halt_baddata();
}
