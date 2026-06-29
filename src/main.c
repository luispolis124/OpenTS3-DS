/* ==========================================================================
   PROJETO: OpenTS3-DS
   DESCRIÇÃO: Orquestração do Game Loop e Ciclo de Vida de Entidades.
   ========================================================================== */

#include <nds.h>
#include <stdio.h>
#include "../include/game/entity_structs.h"
#include "../include/game/entity.h" // Assumindo que você criou este header

// Variáveis Globais de Estado
SimEntity g_SimPrincipal;

void Engine_Init() {
    consoleDemoInit();
    printf("OpenTS3-DS: Motor inicializado.\n");
}

int main(void) {
    // 1. Inicialização
    Engine_Init();
    uint32_t buffer_evento[3];

    // 2. Spawn da Entidade
    printf("Simulando nascimento...\n");
    Entity_Init_Sim(&g_SimPrincipal, 100);

    // 3. Game Loop Principal
    int frames = 0;
    while(frames < 100) { // Simulação curta para teste
        
        // A. Update lógico
        Entity_Update_State(&g_SimPrincipal, 150 + frames, 0);
        
        // B. Envio de evento ocasional
        if (frames == 50) {
            Entity_Send_Event(buffer_evento, 1, 999, 0);
        }

        // C. Renderização e Sincronização
        swiWaitForVBlank();
        frames++;
    }

    // 4. Finalização (Cleanup)
    printf("Encerrando ciclo de vida...\n");
    Entity_Destroy(&g_SimPrincipal);
    
    printf("Motor parado com sucesso.\n");
    return 0;
}
