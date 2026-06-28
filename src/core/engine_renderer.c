/* ==========================================================================
   FUNÇÃO: FUN_00007f3e
   DESCRIÇÃO: Pipeline de Renderização - Escalonamento e Sincronização de VRAM
   ========================================================================== */

#include "../include/pipeline_core.h"

void FUN_00007f3e(undefined4 param_1, undefined4 param_2, uint param_3, int param_4)
{
    uint uVar1;
    int *piVar2;
    int extraout_r1 = 0;
    char extraout_r1_00 = 0;
    undefined1 extraout_r1_01 = 0;
    uint uVar3;
    
    // Contexto de Registradores do ARM9 (Otimizados pelo compilador do jogo)
    int unaff_r4 = 0; 
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int in_lr = 0;
    int iVar4, iVar5, iVar6;
    int in_stack_000003e4 = 0;
    
    // Início do pipeline: Sincronização de Clock de Hardware (0xdee offset)
    iVar4 = 0x7f41;
    ((void (*)(void))(in_lr + 0xdee))();
    
    param_4 = param_4 + -0xf9;
    *(short *)(unaff_r6 + 0x1e) = (short)(unaff_r4 << 4);
    
    // Execução da sub-rotina de transformação de vértices
    ((void (*)(int, int*))(iVar4 + 0xfee))(unaff_r4 << 4, (int*)&in_stack_000003e4);
    
    iVar5 = 0x7f55;
    ((void (*)(void))&SUB_00207d3e)();
    *(short *)(unaff_r7 + 0x2e) = (short)unaff_r5;
    
    iVar4 = extraout_r1;
    
    // Loop principal: Varredura de Buffer de Objetos
    do {
        iVar6 = 0x7f5d;
        ((void (*)(uint, int))(iVar5 + 0xfee))(param_3 << 0xb, iVar4);
        
        *(undefined1 *)(unaff_r6 + 0xf) = 0;
        *(char *)(unaff_r7 + 3) = extraout_r1_00 + '\b';
        
        iVar4 = 0x7f6b;
        ((void (*)(int))(iVar6 + 0x9ee))(unaff_r6 << 7);
        
        iVar5 = 0x7f6f;
        ((void (*)(void))(iVar4 + 0x9ee))();
        
        iVar6 = param_3 + 0x48;
        iVar4 = in_stack_000003e4 << 8;
        param_3 = param_3 + 0x138;
        *(char *)(unaff_r7 + 0x1f) = (char)unaff_r5;
    } while (param_3 != 0 && -0xf1 < iVar6);
    
    // Escrita final no Buffer de VRAM
    piVar2 = (int *)(unaff_r6 * 0x80000);
    *piVar2 = param_4 << 0x10;
    
    // Cálculo de rotação de coordenadas (Bit Rotation)
    piVar2[1] = (param_3 >> (unaff_r7 & 0x1f)) | (param_3 << (0x20 - (unaff_r7 & 0x1f)));
    piVar2[2] = param_4;
    piVar2[3] = unaff_r5;
    piVar2[4] = unaff_r6;
    
    *(int *)(unaff_r7 + 0x4c) = param_4 << 0x10;
    iVar6 = (unaff_r7 - 0xf0) + (param_4 - 0xf0);
    *(undefined2 *)(unaff_r7 - 0xb2) = 0;
    
    uVar3 = (unaff_r7 - 0xf3) & (param_4 - 0xf0);
    iVar4 = iVar6 * 0x80;
    uVar1 = (unaff_r7 - 0xf0) & 0x1f;
    *(char *)(iVar6 + 0xf) = (char)iVar4;
    
    // Finalização e Sync
    iVar6 = 0x7fc1;
    ((void (*)(int, int, uint))(iVar5 + 0x9ee))(iVar4, *(int *)(unaff_r7 - 0x74), (uVar3 >> uVar1) | (uVar3 << (0x20 - uVar1)));
    ((void (*)(void))(iVar6 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 - 0xd9) = extraout_r1_01;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007f4c
   DESCRIÇÃO: Pipeline de Renderização (Fast-path)
   ========================================================================== */

void FUN_00007f4c(undefined4 param_1, undefined4 param_2, uint param_3, int param_4)
{
    uint uVar1;
    int *piVar2;
    int extraout_r1 = 0;
    char extraout_r1_00 = 0;
    undefined1 extraout_r1_01 = 0;
    uint uVar3;
    
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int iVar4, iVar5, iVar6;
    int in_stack_000003e4 = 0;
    
    iVar4 = 0x7f55;
    ((void (*)(void))&SUB_00207d3e)();
    *(short *)(unaff_r7 + 0x2e) = (short)unaff_r5;
    
    iVar6 = extraout_r1;
    
    do {
        iVar5 = 0x7f5d;
        ((void (*)(uint, int))(iVar4 + 0xfee))(param_3 << 0xb, iVar6);
        
        *(undefined1 *)(unaff_r6 + 0xf) = 0;
        *(char *)(unaff_r7 + 3) = extraout_r1_00 + '\b';
        
        iVar6 = 0x7f6b;
        ((void (*)(int))(iVar5 + 0x9ee))(unaff_r6 << 7);
        
        iVar4 = 0x7f6f;
        ((void (*)(void))(iVar6 + 0x9ee))();
        
        iVar5 = param_3 + 0x48;
        iVar6 = in_stack_000003e4 << 8;
        param_3 = param_3 + 0x138;
        *(char *)(unaff_r7 + 0x1f) = (char)unaff_r5;
    } while (param_3 != 0 && -0xf1 < iVar5);
    
    piVar2 = (int *)(unaff_r6 * 0x80000);
    *piVar2 = param_4 << 0x10;
    piVar2[1] = (param_3 >> (unaff_r7 & 0x1f)) | (param_3 << (0x20 - (unaff_r7 & 0x1f)));
    piVar2[2] = param_4;
    piVar2[3] = unaff_r5;
    piVar2[4] = unaff_r6;
    
    *(int *)(unaff_r7 + 0x4c) = param_4 << 0x10;
    iVar5 = (unaff_r7 - 0xf0) + (param_4 - 0xf0);
    *(undefined2 *)(unaff_r7 - 0xb2) = 0;
    
    uVar3 = (unaff_r7 - 0xf3) & (param_4 - 0xf0);
    iVar6 = iVar5 * 0x80;
    uVar1 = (unaff_r7 - 0xf0) & 0x1f;
    *(char *)(iVar5 + 0xf) = (char)iVar6;
    
    iVar5 = 0x7fc1;
    ((void (*)(int, int, uint))(iVar4 + 0x9ee))(iVar6, *(int *)(unaff_r7 - 0x74), (uVar3 >> uVar1) | (uVar3 << (0x20 - uVar1)));
    ((void (*)(void))(iVar5 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 - 0xd9) = extraout_r1_01;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007f62
   DESCRIÇÃO: Processamento de Entidades e Sincronização Final
   ========================================================================== */

void FUN_00007f62(undefined4 param_1, int param_2, uint param_3, int param_4)
{
    uint uVar1;
    int *piVar2;
    int extraout_r1 = 0;
    undefined1 extraout_r1_00 = 0;
    uint uVar3;
    
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int in_lr = 0;
    int iVar4, iVar5, iVar6;
    int in_stack_000003e4 = 0;
    
    // Loop principal de processamento de entidades (Entity Processing Loop)
    while (1) {
        *(char *)(unaff_r7 + 3) = (char)param_2;
        
        iVar5 = 0x7f6b;
        ((void (*)(int))(in_lr + 0x9ee))(unaff_r6 << 7);
        
        iVar6 = 0x7f6f;
        ((void (*)(void))(iVar5 + 0x9ee))();
        
        iVar5 = param_3 + 0x48;
        param_3 = param_3 + 0x138;
        *(char *)(unaff_r7 + 0x1f) = (char)unaff_r5;
        
        // Verificação de parada do ciclo de renderização
        if (param_3 == 0 || iVar5 < -0xf0) break;
        
        in_lr = 0x7f5d;
        ((void (*)(int, int))(iVar6 + 0xfee))(param_3 * 0x800, in_stack_000003e4 << 8);
        
        param_2 = extraout_r1 + -0xf8;
        *(undefined1 *)(unaff_r6 + 0xf) = 0;
    }
    
    // Escrita no buffer de saída (VRAM)
    piVar2 = (int *)(unaff_r6 * 0x80000);
    *piVar2 = param_4 << 0x10;
    piVar2[1] = (param_3 >> (unaff_r7 & 0x1f)) | (param_3 << (0x20 - (unaff_r7 & 0x1f)));
    piVar2[2] = param_4;
    piVar2[3] = unaff_r5;
    piVar2[4] = unaff_r6;
    
    *(int *)(unaff_r7 + 0x4c) = param_4 << 0x10;
    iVar4 = (unaff_r7 - 0xf0) + (param_4 - 0xf0);
    *(undefined2 *)(unaff_r7 - 0xb2) = 0;
    
    uVar3 = (unaff_r7 - 0xf3) & (param_4 - 0xf0);
    iVar5 = iVar4 * 0x80;
    uVar1 = (unaff_r7 - 0xf0) & 0x1f;
    *(char *)(iVar4 + 0xf) = (char)iVar5;
    
    // Sincronização final do pipeline
    iVar4 = 0x7fc1;
    ((void (*)(int, int, uint))(iVar6 + 0x9ee))(iVar5, *(int *)(unaff_r7 - 0x74), (uVar3 >> uVar1) | (uVar3 << (0x20 - uVar1)));
    ((void (*)(void))(iVar4 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 - 0xd9) = extraout_r1_00;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007f74
   DESCRIÇÃO: Loop de processamento variante para alinhamento de entidades
   ========================================================================== */

void FUN_00007f74(undefined4 param_1, int param_2, int param_3, int param_4)
{
    int *piVar1;
    int iVar2;
    char extraout_r1 = 0;
    undefined1 extraout_r1_00 = 0;
    uint uVar3, uVar4;
    
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int in_lr = 0;
    int iVar5;
    int in_stack_000003e4 = 0;
    
    // Loop de processamento de entidades com cálculo de offset dinâmico
    while (1) {
        uVar3 = param_3 + 0xf0;
        *(char *)(unaff_r7 + 0x1f) = (char)unaff_r5;
        
        if (uVar3 == 0 || param_3 < -0xf0) break;
        
        iVar2 = 0x7f5d;
        ((void (*)(uint, int))(in_lr + 0xfee))(uVar3 * 0x800, param_2);
        
        *(undefined1 *)(unaff_r6 + 0xf) = 0;
        *(char *)(unaff_r7 + 3) = extraout_r1 + '\b';
        
        iVar5 = 0x7f6b;
        ((void (*)(int))(iVar2 + 0x9ee))(unaff_r6 << 7);
        
        in_lr = 0x7f6f;
        ((void (*)(void))(iVar5 + 0x9ee))();
        
        param_3 = uVar3 + 0x48;
        param_2 = in_stack_000003e4 << 8;
    }
    
    // Flush para a VRAM (Lógica central do motor)
    piVar1 = (int *)(unaff_r6 * 0x80000);
    *piVar1 = param_4 << 0x10;
    piVar1[1] = (uVar3 >> (unaff_r7 & 0x1f)) | (uVar3 << (0x20 - (unaff_r7 & 0x1f)));
    piVar1[2] = param_4;
    piVar1[3] = unaff_r5;
    piVar1[4] = unaff_r6;
    
    *(int *)(unaff_r7 + 0x4c) = param_4 << 0x10;
    iVar5 = (unaff_r7 - 0xf0) + (param_4 - 0xf0);
    *(undefined2 *)(unaff_r7 - 0xb2) = 0;
    
    uVar4 = (unaff_r7 - 0xf3) & (param_4 - 0xf0);
    iVar2 = iVar5 * 0x80;
    uVar3 = (unaff_r7 - 0xf0) & 0x1f;
    *(char *)(iVar5 + 0xf) = (char)iVar2;
    
    // Finalização
    iVar5 = 0x7fc1;
    ((void (*)(int, int, uint))(in_lr + 0x9ee))(iVar2, *(int *)(unaff_r7 - 0x74), (uVar4 >> uVar3) | (uVar4 << (0x20 - uVar3)));
    ((void (*)(void))(iVar5 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 - 0xd9) = extraout_r1_00;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007f78
   DESCRIÇÃO: Loop de renderização com controle de flags de CPU (ARM Status)
   ========================================================================== */

void FUN_00007f78(undefined4 param_1, int param_2, uint param_3, int param_4)
{
    uint uVar1;
    int *piVar2;
    int iVar3;
    char extraout_r1 = 0;
    undefined1 extraout_r1_00 = 0;
    uint uVar4;
    
    int unaff_r5 = 0;
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int in_lr = 0;
    int iVar5;
    int in_stack_000003e4 = 0;
    
    // Flags de controle do ARM9
    bool in_ZR = false;
    bool in_NG = false;
    bool in_OV = false;
    
    while (1) {
        *(char *)(unaff_r7 + 0x1f) = (char)unaff_r5;
        
        // Verificação de flags de estado do processador
        if (in_ZR || (in_NG != in_OV)) break;
        
        iVar3 = 0x7f5d;
        ((void (*)(uint, int))(in_lr + 0xfee))(param_3 << 0xb, param_2);
        
        *(undefined1 *)(unaff_r6 + 0xf) = 0;
        *(char *)(unaff_r7 + 3) = extraout_r1 + '\b';
        
        iVar5 = 0x7f6b;
        ((void (*)(int))(iVar3 + 0x9ee))(unaff_r6 << 7);
        
        in_lr = 0x7f6f;
        ((void (*)(void))(iVar5 + 0x9ee))();
        
        param_2 = in_stack_000003e4 << 8;
        
        // Atualização de estado baseada em aritmética de ponto fixo
        param_3 = param_3 + 0x138;
        in_ZR = (param_3 == 0);
        in_NG = ((int)param_3 < 0);
    }
    
    // Flush final idêntico às rotinas anteriores
    piVar2 = (int *)(unaff_r6 * 0x80000);
    *piVar2 = param_4 << 0x10;
    piVar2[1] = (param_3 >> (unaff_r7 & 0x1f)) | (param_3 << (0x20 - (unaff_r7 & 0x1f)));
    piVar2[2] = param_4;
    piVar2[3] = unaff_r5;
    piVar2[4] = unaff_r6;
    
    *(int *)(unaff_r7 + 0x4c) = param_4 << 0x10;
    iVar5 = (unaff_r7 - 0xf0) + (param_4 - 0xf0);
    *(undefined2 *)(unaff_r7 - 0xb2) = 0;
    
    uVar4 = (unaff_r7 - 0xf3) & (param_4 - 0xf0);
    iVar3 = iVar5 * 0x80;
    uVar1 = (unaff_r7 - 0xf0) & 0x1f;
    *(char *)(iVar5 + 0xf) = (char)iVar3;
    
    iVar5 = 0x7fc1;
    ((void (*)(int, int, uint))(in_lr + 0x9ee))(iVar3, *(int *)(unaff_r7 - 0x74), (uVar4 >> uVar1) | (uVar4 << (0x20 - uVar1)));
    ((void (*)(void))(iVar5 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 - 0xd9) = extraout_r1_00;
    
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007f8c
   DESCRIÇÃO: Rotina de finalização (Flush) isolada.
   ========================================================================== */

void FUN_00007f8c(undefined4 param_1, undefined4 param_2, undefined2 param_3, int param_4)
{
    uint uVar1;
    int iVar2, iVar4;
    undefined1 extraout_r1 = 0;
    uint uVar3;
    
    // Registradores de estado interno do ARM
    undefined4 unaff_r4 = 0; 
    int unaff_r7 = 0;
    int in_lr = 0;

    // Escrita direta nos offsets de controle do buffer
    *(int *)(unaff_r7 + 0x4c) = (int)unaff_r4;
    
    iVar4 = (unaff_r7 - 0xf0) + (param_4 - 0xf0);
    *(undefined2 *)(unaff_r7 - 0xb2) = param_3;
    
    uVar3 = (unaff_r7 - 0xf3) & (param_4 - 0xf0);
    iVar2 = iVar4 * 0x80;
    uVar1 = (unaff_r7 - 0xf0) & 0x1f;
    
    // Atualização de caracteres de controle de hardware
    *(char *)(iVar4 + 0xf) = (char)iVar2;
    
    // Sincronização final com o hardware (via ponteiros de código)
    iVar4 = 0x7fc1;
    ((void (*)(int, int, uint))(in_lr + 0x9ee))(iVar2, *(int *)(unaff_r7 - 0x74), (uVar3 >> uVar1) | (uVar3 << (0x20 - uVar1)));
    ((void (*)(void))(iVar4 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 - 0xd9) = extraout_r1;
    
    // Comando de parada/limpeza do pipeline
    halt_baddata();
}

/* ==========================================================================
   FUNÇÃO: FUN_00007f9c
   DESCRIÇÃO: Rotina de atualização de parâmetro e sincronização de fluxo
   ========================================================================== */

void FUN_00007f9c(undefined4 param_1, undefined4 param_2, undefined2 param_3, uint param_4)
{
    int iVar1;
    undefined1 extraout_r1 = 0;
    
    // Registradores do ARM
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int in_lr = 0;
    
    // Atualização de registro de estado
    *(undefined2 *)(unaff_r7 + 0x3e) = param_3;
    
    // Máscara de bits para o parâmetro 4
    param_4 = (unaff_r7 - 3) & param_4;
    *(char *)(unaff_r6 + 0xf) = (char)(unaff_r6 << 7);
    
    // Sincronização final do pipeline com rotação de bits
    iVar1 = 0x7fc1;
    ((void (*)(int, int, uint))(in_lr + 0x9ee))(
        (unaff_r6 << 7), 
        *(int *)(unaff_r7 + 0x7c), 
        (param_4 >> (unaff_r7 & 0x1f)) | (param_4 << (0x20 - (unaff_r7 & 0x1f)))
    );
    ((void (*)(void))(iVar1 + 0x9ee))();
    
    *(undefined1 *)(unaff_r7 + 0x17) = extraout_r1;
    
    halt_baddata();
}
