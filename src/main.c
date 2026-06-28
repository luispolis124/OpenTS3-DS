/* ==========================================================================
   PROJETO: OpenTS3-DS
   ARQUIVO: main.c
   DESCRIÇÃO: Ponto de entrada que orquestra a inicialização dos sistemas.
   ========================================================================== */

#include <nds.h>
#include "../include/entity.h" // Incluindo sua "bíblia" de estruturas

// Inicialização dos sistemas que mapeamos
void Engine_Init() {
    Entity_System_Init();
    // Aqui você pode adicionar outros sistemas que mapear no futuro
    // (Ex: Input_Init(), Graphics_Init(), Audio_Init())
}

int main(void) {
    // 1. Inicializa o hardware do NDS
    consoleDemoInit(); 
    Engine_Init();

    // 2. Loop Principal (Game Loop)
    while(1) {
        // A. Processar Entidades (Lógica de Spawn, Update e Destruição)
        // Entity_UpdateAll(); 

        // B. Renderização (Desenhar tudo na tela)
        // Entity_RenderAll();

        // C. Sincronização
        swiWaitForVBlank(); 
    }
    
    return 0;
}
