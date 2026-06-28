#include "../include/pipeline_core.h"

/* ==========================================================================
   MÓDULO: Hardware Interface (HW_Interface)
   DESCRIÇÃO: Funções de baixo nível para comunicação com o Coprocessador ARM9
   e gerenciamento de estados de hardware do Nintendo DS.
   ========================================================================== */

/**
 * FUNÇÃO: FUN_000080b0
 * DESCRIÇÃO: Interface de carregamento do Coprocessador (Geometria/Transformações).
 */
void FUN_000080b0(void)
{
    // Registradores simulados de estado da CPU (CPSR)
    // No NDS, estas flags determinam se a operação aritmética foi válida.
    bool in_CY = false; // Carry Flag
    bool in_OV = false; // Overflow Flag
    
    // Registradores auxiliares para endereçamento de memória
    int in_r12 = 0;
    undefined4 in_cr8 = 0; 
    
    // Lógica de proteção: o coprocessador só recebe instruções se as
    // flags de estado da CPU indicarem uma operação aritmética estável.
    if (!in_CY) {
        if (in_OV) {
            // ID 3 é tipicamente mapeado para a unidade de geometria no NDS.
            coprocessor_load(3, in_cr8, (in_r12 - 0x150));
        }
        
        // NOTA: Em um port para PC, o halt_baddata() é um ponto de falha intencional
        // caso o hardware (emulado) retorne um estado inesperado.
    }
}

// Futuras funções de hardware (ex: acesso a registradores de vídeo, entrada de toque)
// serão adicionadas aqui...
