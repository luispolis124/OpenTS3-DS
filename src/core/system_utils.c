#include "../include/pipeline_core.h"

/* ==========================================================================
   FUNÇÃO: FUN_00008090
   DESCRIÇÃO: Verificação de integridade de pilha e tratamento de exceção.
   MÓDULO: System Utils (Core)
   ========================================================================== */

undefined8 FUN_00008090(undefined4 param_1, int param_2, undefined4 param_3, 
                       int param_4, undefined4 param_5, undefined4 param_6)
{
    // Registradores do ARM9 (Contexto de exceção)
    undefined4 unaff_r6 = 0;
    undefined4 unaff_r11 = 0;
    
    // Flags de status do processador ARM
    bool in_ZR = false;
    char in_NG = 0;
    char in_OV = 0;

    // 1. Verificação de Integridade: Se o estado for válido, registra o contexto
    if (!in_ZR && (in_NG == in_OV)) {
        *(undefined4 *)(param_4 - 4)  = 0x809c; // Endereço de retorno/handler
        *(undefined4 *)(param_4 - 8)  = unaff_r11;
        *(undefined4 *)(param_4 - 0xc) = unaff_r6;
        *(int *)(param_4 - 0x10)      = param_2;
    }

    // 2. Lógica de decisão de interrupção
    if (in_ZR) {
        in_NG = (param_2 < 0);
    }

    // 3. Gatilho de Segurança: Caso o sistema detecte inconsistência, 
    // ele dispara uma interrupção de software ou interrompe a execução
    if (!in_ZR || (param_2 != 0)) {
        if (in_NG == in_OV) {
            software_interrupt(0xfe4af0);
        }
        halt_baddata();
    }

    return CONCAT44(param_6, param_5);
}
