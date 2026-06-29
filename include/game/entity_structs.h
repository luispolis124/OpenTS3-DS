#ifndef ENTITY_STRUCTS_H
#define ENTITY_STRUCTS_H

#include <stdint.h>

/**
 * SimEntity representa a estrutura de dados de um objeto ou Sim no motor.
 * Os offsets (padding) foram mapeados baseados na engenharia reversa do binário original.
 */
typedef struct {
    // 0x00 - 0x17: Cabeçalho de controle de memória
    char header[0x18];
    
    // 0x18: Identificador principal ou parâmetro de controle
    int16_t id_base; 
    
    // 0x1A - 0x1D: Campos de alinhamento
    char padding1[0x04];
    
    // 0x1E: Ponteiro para processamento secundário (piVar6 no código original)
    int16_t ptr_processamento_secundario;
    
    // 0x20: Ponteiro para dados de entidade (puVar10)
    void* entity_data;
    
    // 0x22: Estado principal da entidade
    int16_t estado_atual;
    
    // 0x24: Alinhamento
    char padding2[0x02];
    
    // 0x26: Offset de processamento de fluxo
    int16_t offset_fluxo;
    
    // 0x28 - 0x31: Espaço para outros atributos
    char padding3[0x0A];
    
    // 0x32: Estado secundário da entidade
    int16_t estado_secundario;
    
    // Espaço adicional para futuros mapeamentos
    char reserved[0x100]; 
} SimEntity;

#endif // ENTITY_STRUCTS_H
