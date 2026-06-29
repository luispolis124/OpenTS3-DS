// Arquivo: src/game/entity.c
#include "../../include/game/entity_structs.h"
#include <stdio.h>

/**
 * Entity_Init_Prop: Inicializa um objeto estático no mundo.
 * O 'unaff_r6' que você via no Ghidra provavelmente era um parâmetro 
 * de estado base. Vamos chamá-lo de 'estado_base'.
 */
void Entity_Init_Prop(int tipo, uint32_t valor_config, SimEntity* entity, int16_t estado_base) {
    if (entity == NULL) return;

    // Inicialização da estrutura mapeada
    entity->id_base = (int16_t)valor_config;
    
    // Aplicando os offsets que vimos no Ghidra
    entity->estado_atual = estado_base - 0x20;
    entity->estado_secundario = estado_base - 0x40;

    // Log para você acompanhar no terminal durante o desenvolvimento
    printf("Entidade %d inicializada com estado base: %d\n", valor_config, estado_base);
}

/**
 * Entity_Init_Actor: Poderia ser uma variação para Sims (Actors)
 */
void Entity_Init_Actor(int tipo, uint32_t valor_config, SimEntity* entity, int16_t estado_base) {
    // Aqui a lógica pode diferir de um objeto para um Sim
    entity->id_base = (int16_t)valor_config;
    entity->estado_atual = estado_base; // Sims podem ter estados diferentes
    
    printf("Sim %d criado no mundo!\n", valor_config);
}
