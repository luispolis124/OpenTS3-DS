#include "../../include/entity_system.h"
#include <stdio.h>

/* ==========================================================================
   MÓDULO: Entity Renderer
   DESCRIÇÃO: Gerencia a transformação e exibição visual das entidades.
   ========================================================================== */

void Entity_UpdateGraphics(Entity* entity) 
{
    // A função original FUN_000088a6 fazia um monte de cálculos de bits
    // para alinhar o objeto. Aqui, nós traduzimos isso para o nosso sistema.
    
    // 1. Calcular a matriz de transformação baseada no estado atual
    // A matriz define onde o objeto está e como ele está rotacionado
    // Matrix4x4 transform = Matrix_Create(entity->x, entity->y, entity->rotation);

    // 2. Comunicar com a API Gráfica (OpenGL/DirectX/SDL)
    // GPU_DrawEntity(entity->type_id, transform);
    
    printf("[RENDERER] Entidade %d renderizada na posição (%.1f, %.1f)\n", 
            entity->id, entity->x, entity->y);
}
