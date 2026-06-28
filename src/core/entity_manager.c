#include "../include/pipeline_core.h"

/* ==========================================================================
   FUNÇÃO: FUN_00008032
   DESCRIÇÃO: Escrita de propriedades no Buffer de Entidades.
   MÓDULO: Entity Manager
   ========================================================================== */

void FUN_00008032(undefined4 param_1, undefined4 param_2, undefined4 param_3)
{
    int iVar1;
    
    // Registradores do contexto ARM9
    int unaff_r4 = 0;
    uint unaff_r5 = 0;
    undefined4 unaff_r6 = 0;
    
    // Cálculo do endereço de memória do objeto (Strid de 32 bytes)
    // O ajuste com 0x8000000 é uma correção de alinhamento comum em sistemas embarcados
    iVar1 = (unaff_r5 * 0x20) + unaff_r4 + (uint)((unaff_r5 & 0x8000000) != 0);
    
    // Escrita direta nas propriedades do objeto (Offsets fixos)
    *(undefined4 *)(iVar1 - 0xbd) = param_2; 
    *(undefined4 *)(iVar1 - 0xb9) = param_3; 
    *(undefined4 *)(iVar1 - 0xb5) = unaff_r6;
    
    // No port para PC, removemos o halt_baddata para evitar encerramento forçado
    // halt_baddata(); 
}
