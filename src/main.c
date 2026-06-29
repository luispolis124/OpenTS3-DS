#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h> // Necessário para Threading
#include "../include/game/entity_structs.h"
#include "../include/game/entity.h"

// Variáveis Globais de Estado
SimEntity g_SimPrincipal;
bool g_rom_loaded = false;
char* g_rom_path = "../rom/Sims3.nds"; // Caminho exemplo

// Função que roda o Emulador em segundo plano
void* Emulator_Thread_Func(void* arg) {
    printf("[Emulator] Thread iniciada...\n");
    // Emulator_Init(); 
    // Emulator_LoadROM(g_rom_path);
    while (g_rom_loaded) {
        // Emulator_RunFrame();
    }
    return NULL;
}

int main(void) {
    InitWindow(800, 600, "OpenTS3-DS - Emulador Integrado");
    SetTargetFPS(60);

    pthread_t emu_thread;
    Entity_Init_Sim(&g_SimPrincipal, 100);

    while (!WindowShouldClose()) {
        // 1. Sistema de Input (Importação)
        if (!g_rom_loaded && IsKeyPressed(KEY_I)) {
            g_rom_loaded = true;
            pthread_create(&emu_thread, NULL, Emulator_Thread_Func, NULL);
            printf("[Main] Emulador iniciado via Thread.\n");
        }

        // 2. Renderização
        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (!g_rom_loaded) {
                DrawText("Pressione 'I' para importar a ROM do The Sims 3", 200, 300, 20, GRAY);
            } else {
                // Aqui você desenharia a textura vinda do emulador:
                // DrawTexture(emulator_frame_texture, 0, 0, WHITE);
                DrawText("Emulando The Sims 3...", 20, 20, 20, BLUE);
            }

            DrawText(TextFormat("Sim Estado: %d", g_SimPrincipal.estado_atual), 20, 550, 20, MAROON);
        EndDrawing();
    }

    // 3. Cleanup
    if (g_rom_loaded) {
        g_rom_loaded = false;
        pthread_join(emu_thread, NULL);
    }
    
    Entity_Destroy(&g_SimPrincipal);
    CloseWindow();
    return 0;
}
