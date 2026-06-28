#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include <stdint.h>

// Definição dos estados que descobrimos em FUN_0000881c
typedef enum {
    STATE_IDLE = 0,
    STATE_WALKING = 1,
    STATE_INTERACTING = 2,
    STATE_SLEEPING = 3
} EntityState;

// Estrutura principal da Entidade (O "Template" do seu motor)
typedef struct {
    int id;
    int type_id;           // Identificador do tipo (0x29, etc)
    float x, y;            // Posição no mundo
    float rotation;        // Rotação para renderização
    
    EntityState state;     // Estado atual (Gerenciado pelo System)
    bool is_active;        // Flag de controle
    bool is_selected;      // Flag para efeitos de destaque (FUN_000088cc)

    // Ponteiros para funções de callback (o "cérebro" da entidade)
    void (*update_logic)(void* self);
    void (*render_graphics)(void* self);
} Entity;

// --- Protótipos das funções que você vai implementar ---

// Funções do entity_system.c
void Entity_System_Init();
int  Entity_Spawn(int type_id, float x, float y);
void Entity_SetState(Entity* entity, EntityState new_state);

// Funções do entity_renderer.c
void Entity_UpdateGraphics(Entity* entity);

#endif // ENTITY_H
