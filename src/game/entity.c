#include "../../include/game/entity_structs.h"
#include <stdio.h>
#include <stdint.h>

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

    entity->estado_atual = novo_param;
    printf("[Update] Entidade atualizada: Param=%d, SubParam=%d\n", novo_param, sub_param);
}

/**
 * Entity_Send_Event (Baseado em 86e0)
 * Empacota dados de um evento para serem processados pelo motor.
 */
void Entity_Send_Event(uint32_t* buffer, uint32_t tipo_evento, uint32_t valor_evento, uint32_t dados_extras) {
    if (buffer == NULL) return;

    buffer[0] = tipo_evento;
    buffer[1] = valor_evento;
    buffer[2] = dados_extras;

    // Marcador de evento ativo no segundo byte
    ((char*)buffer)[1] = 0x01; 

    printf("[Event] Evento enviado: Tipo=%u, Valor=%u\n", tipo_evento, valor_evento);
}

/**
 * Entity_Destroy (Baseado em 8612)
 * Limpeza de memória e finalização de ciclo de vida.
 */
void Entity_Destroy(SimEntity* entity) {
    if (entity == NULL) return;
    
    entity->estado_atual = 0;
    entity->estado_secundario = 0;
    entity->entity_data = NULL;
    
    printf("[Destroy] Entidade liberada da memória.\n");
}
