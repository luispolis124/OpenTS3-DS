/* ==========================================================================
   PROJETO: OpenTS3-DS
   ARQUIVO: main.c (Ponto Central do Código)
   ========================================================================== */

#include <nds.h> // Inclui as bibliotecas nativas de desenvolvimento do DS
#include "data_stream.c" // Linka o arquivo de tratamento de dados que criamos

int main(void) {
    // O ponto de entrada principal do código do jogo futuramente ficará aqui.
    // Por enquanto, as funções descompiladas são declaradas e chamadas conforme necessário.
    
    while(1) {
        swiWaitForVBlank(); // Mantém o loop principal do hardware do DS estável
    }
    
    return 0;
}
