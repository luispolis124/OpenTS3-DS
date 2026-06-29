#include "../../include/game/entity_structs.h"
#include <stdio.h>

/**
 * Entity_Init_Sim (Baseado em 85a6)
 * Inicializa Sims ou Entidades complexas (Actors).
 */
void Entity_Init_Sim(SimEntity* entity, int16_t estado_base) {
    if (entity == NULL) return;
    
    entity->estado_atual = estado_base;
    entity->estado_secundario = estado_base - 0x20;
    
    printf("[Init] Sim/Actor inicializado (Estado: %d)\n", estado_base);
}

/**
 * Entity_Init_Prop (Baseado em 85ca)
 * Inicializa objetos do cenário (cadeiras, mesas, etc).
 */
void Entity_Init_Prop(SimEntity* entity, int16_t estado_base) {
    if (entity == NULL) return;
    
    entity->estado_atual = estado_base - 0x20;
    entity->estado_secundario = estado_base - 0x40;
    
    printf("[Init] Objeto/Prop inicializado (Estado: %d)\n", estado_base);
}

/**
 * Entity_Init_Trigger (Baseado em 85ee)
 * Inicializa gatilhos ou zonas lógicas do jogo.
 */
void Entity_Init_Trigger(SimEntity* entity, int16_t estado_base) {
    if (entity == NULL) return;
    
    entity->estado_atual = estado_base;
    entity->estado_secundario = estado_base - 0x20;
    
    printf("[Init] Trigger inicializado (Estado: %d)\n", estado_base);
}

/**
 * Entity_Update_State (Baseado em 8694)
 * Ajusta parâmetros internos e registra a entidade no sistema de processamento.
 */
void Entity_Update_State(SimEntity* entity, int16_t novo_param, int16_t sub_param) {
    if (entity == NULL) return;

    // Atualiza o estado principal
    entity->estado_atual = novo_param;
    
    // Opcional: registrar em logs que a entidade mudou de comportamento
    printf("[Update] Entidade atualizada: Param=%d, SubParam=%d\n", novo_param, sub_param);
}

/**
 * Entity_Destroy (Baseado em 8612)
 * Limpeza de memória e finalização de ciclo de vida.
 */
void Entity_Destroy(SimEntity* entity) {
    if (entity == NULL) return;
    
    entity->estado_atual = 0;
    entity->estado_secundario = 0;
    entity->entity_data = NULL; // Limpa o ponteiro de dados
    
    printf("[Destroy] Entidade liberada da memória.\n");
}
