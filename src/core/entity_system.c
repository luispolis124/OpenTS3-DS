#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_ENTITIES 256

// Definição da estrutura moderna da entidade
typedef struct {
    int id;
    int type_id;
    float x, y;
    bool is_active;
    void (*update)(void); // Callback para lógica de atualização
} Entity;

// Gerenciador global de entidades
static Entity entity_pool[MAX_ENTITIES];

/* ==========================================================================
   FUNÇÃO: Entity_System_Init
   DESCRIÇÃO: Inicializa o pool de entidades do seu port.
   ========================================================================== */
void Entity_System_Init() {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entity_pool[i].is_active = false;
        entity_pool[i].id = i;
    }
    printf("Sistema de Entidades inicializado com sucesso.\n");
}

/* ==========================================================================
   FUNÇÃO: Entity_Spawn
   DESCRIÇÃO: Substitui as funções legadas (847e, 858c, etc).
   ========================================================================== */
int Entity_Spawn(int type_id, float x, float y) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entity_pool[i].is_active) {
            entity_pool[i].is_active = true;
            entity_pool[i].type_id = type_id;
            entity_pool[i].x = x;
            entity_pool[i].y = y;
            
            printf("Entidade %d (Tipo: 0x%X) criada em (%.1f, %.1f).\n", i, type_id, x, y);
            return i; // Retorna o ID da entidade criada
        }
    }
    printf("Erro: Pool de entidades cheio!\n");
    return -1;
}

/* ==========================================================================
   FUNÇÃO: Entity_UpdateAll
   DESCRIÇÃO: Processa todas as entidades ativas.
   ========================================================================== */
void Entity_UpdateAll() {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entity_pool[i].is_active) {
            // Aqui você chamaria a lógica de cada entidade
            // Ex: if (entity_pool[i].update) entity_pool[i].update();
        }
    }
}

/* ==========================================================================
   FUNÇÃO: Entity_Commit (Refatoração da FUN_00008612)
   DESCRIÇÃO: Conclui o registro de uma entidade, ligando-a ao sistema
   de atualização global e renderização.
   ========================================================================== */
void Entity_Commit(Entity* entity)
{
    // 1. Vincula a lógica de atualização (callbacks)
    // No NDS, isso apontava para endereços estáticos como DAT_8918171c.
    // No seu port, você apontará para funções C++ ou C definidas por você.
    entity->update_func = &Global_Update_Callback;

    // 2. Configura buffers de hardware (Substitui os acessos a 0x4000000)
    // Aqui você atualizará o seu buffer de renderização (ex: OpenGL/DirectX/SDL).
    
    // 3. Ativa a entidade
    entity->is_active = true;
    
    printf("Entidade comprometida (registrada) com sucesso.\n");
}
