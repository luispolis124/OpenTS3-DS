#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include "../include/game/entity_structs.h"
#include "../include/game/entity.h"

// Variável Global
SimEntity g_SimPrincipal;

int main(void) {
    // 1. Inicialização da Janela e Motor
    InitWindow(800, 600, "OpenTS3-DS - Visual");
    SetTargetFPS(60); // Define o tempo de frame automaticamente
    
    printf("OpenTS3-DS: Motor iniciado com Raylib.\n");
    Entity_Init_Sim(&g_SimPrincipal, 100);

    uint32_t buffer_evento[3];
    uint32_t buffer_serialize[3];
    int frames = 0;

    // 2. Loop Principal (Substitui o while antigo)
    while (!WindowShouldClose()) {
        
        // A. Lógica (Seu motor)
        if (g_SimPrincipal.estado_atual != 0) {
            Entity_Update_State(&g_SimPrincipal, 150 + (frames % 50), 1);
            
            if (frames == 50) Entity_Send_Event(buffer_evento, 0xA1, 999, 10);
            if (frames == 100) Entity_Serialize(&g_SimPrincipal, buffer_serialize);
        }

        // B. Renderização
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Exemplo visual do estado da entidade
            DrawText("OpenTS3-DS Engine", 20, 20, 20, DARKGRAY);
            DrawText(TextFormat("Sim Estado: %d", g_SimPrincipal.estado_atual), 20, 50, 20, MAROON);
            
            if (frames > 100) {
                DrawText("Sim Serializado!", 20, 80, 20, GREEN);
            }
        EndDrawing();

        frames++;
    }

    // 3. Finalização
    Entity_Destroy(&g_SimPrincipal);
    CloseWindow();
    
    printf("Motor parado com sucesso.\n");
    return 0;
}
