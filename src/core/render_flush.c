#include "../include/pipeline_core.h"

/* ==========================================================================
   FUNÇÃO: FUN_00007fa2
   DESCRIÇÃO: Rotina de atualização de parâmetro e sincronização final (Flush).
   MÓDULO: Render Flush
   ========================================================================== */

void FUN_00007fa2(undefined4 param_1, undefined4 param_2, undefined2 param_3, uint param_4)
{
    // Registradores simulados do ARM9
    int unaff_r6 = 0;
    uint unaff_r7 = 0;
    int in_lr = 0;
    int iVar1 = 0;
    undefined1 extraout_r1 = 0;

    // 1. Atualização do buffer de estado
    *(undefined2 *)(unaff_r7 + 0x3e) = param_3;
    
    // 2. Processamento da máscara de bits e alinhamento
    param_4 = (unaff_r7 - 3) & param_4;
    *(char *)(unaff_r6 + 0xf) = (char)(unaff_r6 << 7);
    
    // 3. Execução da Sincronização Final com Hardware (Flush de Pipeline)
    iVar1 = 0x7fc1;
    ((void (*)(int, int, uint))(in_lr + 0x9ee))(
        (unaff_r6 << 7), 
        *(int *)(unaff_r7 + 0x7c), 
        (param_4 >> (unaff_r7 & 0x1f)) | (param_4 << (0x20 - (unaff_r7 & 0x1f)))
    );
    
    ((void (*)(void))(iVar1 + 0x9ee))();
    
    // 4. Limpeza e encerramento do frame
    *(undefined1 *)(unaff_r7 + 0x17) = extraout_r1;
    
    // halt_baddata(); // Comentado para evitar parada brusca no port de PC
}
