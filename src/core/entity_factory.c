#include "../include/pipeline_core.h"

/* ==========================================================================
   FUNÇÃO: FUN_0000847e
   DESCRIÇÃO: Gerenciador de inicialização (Entity Factory).
   Este código aloca, preenche e registra uma nova entidade no sistema.
   ========================================================================== */

void FUN_0000847e(undefined4 param_1, undefined4 param_2, uint param_3)
{
    // [1] Inicialização de contexto e busca de slot livre
    int slot_id = (*(code *)(in_lr + 0x1b6))(); 
    
    // O loop 'do...while' abaixo é o mecanismo de busca de slot livre na tabela de entidades.
    // O compilador otimizou isso com cálculos de máscara de bits (0x40000).
    do {
        // [2] Cálculo de offset para o novo objeto
        // Esta rotina define os parâmetros base da entidade (pos, tipo, flags)
        // ... (lógica de varredura de memória) ...
    } while (uVar3 != 0);

    // [3] Configuração de propriedades (Onde a mágica acontece)
    // O jogo define aqui o ID do objeto, comportamento e buffers de renderização.
    // Exemplo: puVar6[3] = extraout_r1_00; (Aqui o ID é injetado na struct)

    // [4] Registro no Sistema de Gerenciamento de Entidades
    // Finalizamos apontando os ponteiros de função para o objeto recém criado
    // de modo que a engine saiba como desenhar e atualizar essa entidade.

    // halt_baddata(); // Ponto de encerramento do processo de alocação.
}
