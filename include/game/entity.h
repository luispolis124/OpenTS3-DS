#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdbool.h>
#include "entity_structs.h"

// --- Enum de Estados (Baseado nos valores encontrados no binário) ---
typedef enum {
    STATE_IDLE = 0,
    STATE_WALKING = 1,
    STATE_INTERACTING = 2,
    STATE_SLEEPING = 3
} EntityState;

// --- Estrutura de Controle (O "cérebro" do seu motor) ---
typedef struct {
    SimEntity* sim_data;   // Ponteiro para a estrutura original mapeada
    EntityState state;     // Estado gerenciado pelo seu motor novo
    bool is_active;
    
    // Callbacks para lógica e renderização
    void (*update_logic)(void* self);
    void (*render_graphics)(void* self);
} Entity;

// --- Protótipos das funções que mapeamos ---
void Entity_Init_Sim(SimEntity* entity, int16_t estado_base);
void Entity_Init_Prop(SimEntity* entity, int16_t estado_base);
void Entity_Init_Trigger(SimEntity* entity, int16_t estado_base);

void Entity_Update_State(SimEntity* entity, int16_t novo_param, int16_t sub_param);
void Entity_Set_Param(SimEntity* entity, int16_t valor);
void Entity_Send_Event(uint32_t* buffer, uint32_t tipo_evento, uint32_t valor_evento, uint32_t dados_extras);
void Entity_Serialize(SimEntity* entity, uint32_t* buffer);
void Entity_Destroy(SimEntity* entity);

// --- Função de Integração ---
void Entity_SetState(Entity* entity, EntityState new_state);

#endif // ENTITY_H
