/* ==========================================================================
   PROJETO: OpenTS3-DS
   DESCRIÇÃO: Orquestração do Game Loop com verificação de segurança.
   ========================================================================== */

#include <nds.h>
#include <stdio.h>
#include "../include/game/entity_structs.h"
#include "../include/game/entity.h"

// Variável Global de Estado
SimEntity g_SimPrincipal;
bool g_EngineRunning = false;

void Engine_Init() {
    consoleDemoInit();
    printf("OpenTS3-DS: Iniciando subsistemas...\n");
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

            // C. Serialização periódica (exemplo: salvar estado a cada 100 frames)
            if (frames == 100) {
                Entity_Serialize(&g_SimPrincipal, buffer_serialize);
                printf("[Main] Estado persistido no buffer.\n");
            }
        }

        // Sincronização de hardware
        swiWaitForVBlank();
        frames++;
    }

    // 3. Finalização Segura
    printf("Shutdown: Limpando memória...\n");
    Entity_Destroy(&g_SimPrincipal);
    g_EngineRunning = false;
    
    printf("Motor parado com sucesso.\n");
    return 0;
}
