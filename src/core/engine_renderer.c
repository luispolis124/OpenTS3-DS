/* ==========================================================================
   FUNÇÃO: FUN_00007f3e
   DESCRIÇÃO: Pipeline de Renderização - Escalonamento e Sincronização de VRAM
   ========================================================================== */

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
