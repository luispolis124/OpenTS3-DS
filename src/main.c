#include <stdio.h>
#include <stdbool.h>
#include <unistd.h> // Para usleep
#include "../include/game/entity_structs.h"
#include "../include/game/entity.h"

// Variável Global
SimEntity g_SimPrincipal;
bool g_EngineRunning = false;

void Engine_Init() {
    // consoleDemoInit(); // REMOVIDO: Não existe no PC
    printf("OpenTS3-DS: Motor iniciado no modo PC (x64).\n");
    g_EngineRunning = true;
}

int main(void) {
    Engine_Init();
    
    uint32_t buffer_evento[3];
    uint32_t buffer_serialize[3];

    printf("Spawning Sim...\n");
    Entity_Init_Sim(&g_SimPrincipal, 100);

    int frames = 0;
    while(g_EngineRunning && frames < 200) {
        
        if (g_SimPrincipal.estado_atual != 0) {
            Entity_Update_State(&g_SimPrincipal, 150 + (frames % 50), 1);
            
            if (frames == 50) {
                Entity_Send_Event(buffer_evento, 0xA1, 999, 10);
            }

            if (frames == 100) {
                Entity_Serialize(&g_SimPrincipal, buffer_serialize);
                printf("[Main] Estado persistido no buffer.\n");
            }
        }

        // swiWaitForVBlank(); // REMOVIDO: Substituído por pausa no PC
        usleep(16666); 
        frames++;
    }

    printf("Shutdown: Limpando memória...\n");
    Entity_Destroy(&g_SimPrincipal);
    g_EngineRunning = false;
    
    printf("Motor parado com sucesso.\n");
    return 0;
}
