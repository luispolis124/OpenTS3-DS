#include "../include/pipeline_core.h"

/* ==========================================================================
   FUNÇÃO: FUN_0000858c
   DESCRIÇÃO: Construtor e Inicializador de Entidade (Variante).
   Este módulo mapeia a estrutura de memória do objeto e o registra no 
   sistema de processamento global do The Sims 3 DS.
   ========================================================================== */

void FUN_0000858c(undefined4 param_1, int param_2, uint param_3)
{
    // Mapeamento de estado interno da entidade
    int entity_base_offset = (param_2 - 4) * 0x2000 >> 0xd;
    uint entity_data_ptr = param_3 * 0x4000;
    
    // 1. Inicialização dos campos principais da estrutura de entidade
    // O jogo está configurando os ponteiros para as tabelas de referência
    *(short *)(entity_base_offset + 0x18) = (short)param_3;
    
    // 2. Limpeza e reset de blocos de memória da VRAM (0x01000000/0x05000000)
    // Isso é crítico: o jogo zera a memória de texturas/vértices antes de escrever
    *(undefined2 *)(param_3 * 0x1000000) = 0;
    *(ushort *)(param_3 * 0x5000000) = *(ushort *)(param_3 + 0x1234); // Exemplo de acesso

    // 3. Serialização da estrutura da entidade
    // puVar3 atua como um ponteiro para a struct da entidade em memória
    uint *entity_struct = (uint *)((param_3 & 0x3ffff) * 0x10);
    entity_struct[0xc] = (uint)entity_struct; // Auto-referência (Self-link)
    entity_struct[0]   = (uint)*(ushort *)(entity_data_ptr + 8);
    entity_struct[1]   = entity_data_ptr;
    entity_struct[2]   = (param_3 & 0x3ffff) << 6;

    // 4. Registro no sistema de callbacks (Callbacks de atualização de entidade)
    // O jogo vincula funções externas (DAT_8918171c, etc.) para gerenciar a entidade
    int *callback_table = (int *)(_DAT_89181724 >> 2);
    callback_table[0] = param_3 << 0x14;
    callback_table[1] = (int)callback_table;

    // 5. Finalização e Gatilho de Hardware
    // O encerramento chama funções críticas para atualizar o motor de renderização
    // (A parte que o Ghidra trunca é o salto para o escalonador de tarefas)
    
    // halt_baddata(); // Ponto de parada original
}
