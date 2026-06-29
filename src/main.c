/* ==========================================================================
   PROJETO: OpenTS3-DS (Portado para x64)
   DESCRIÇÃO: Orquestração do Game Loop adaptada para ambiente Windows/PC.
   ========================================================================== */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h> // Para usleep()
#include "../include/game/entity_structs.h"
#include "../include/game/entity.h"

// Variável Global de Estado
SimEntity g_SimPrincipal;
bool g_EngineRunning = false;

void Engine_Init() {
    printf("OpenTS3-DS: Motor iniciado no modo PC (x64).\n");
    g_EngineRunning = true;
}

int main(void) {
    Engine_Init();
    
    uint32_t buffer_evento[3];
    uint32_t buffer_serialize[3];

    // 1. Spawn da Entidade
    printf("Spawning Sim...\n");
    Entity_Init_Sim(&g_SimPrincipal, 100);

    // 2. Loop Principal
    int frames = 0;
    while(g_EngineRunning && frames < 200) {
        
        // Verificação de segurança: Entidade está ativa?
        if (g_SimPrincipal.estado_atual != 0) {
            
            // A. Update lógico
            Entity_Update_State(&g_SimPrincipal, 150 + (frames % 50), 1);
            
            // B. Envio de evento condicional
            if (frames == 50) {
                Entity_Send_Event(buffer_evento, 0xA1, 999, 10);
            }

            // C. Serialização periódica
            if (frames == 100) {
                Entity_Serialize(&g_SimPrincipal, buffer_serialize);
                printf("[Main] Estado persistido no buffer.\n");
            }
        }

        // Sincronização: Substituindo swiWaitForVBlank por ~60 FPS
        usleep(16666); 
        frames++;
    }

    // 3. Finalização Segura
    printf("Shutdown: Limpando memória...\n");
    Entity_Destroy(&g_SimPrincipal);
    g_EngineRunning = false;
    
    printf("Motor parado com sucesso.\n");
    return 0;
}
