/* ==========================================================================
   FUNÇÃO: Entity_CommonRegister
   DESCRIÇÃO: Núcleo comum de registro de entidade. 
   Evita a duplicação de código entre as variações 858c e 85a6.
   ========================================================================== */
void Entity_CommonRegister(uint param_3, int offset_data, uint entity_id)
{
    // Coloque aqui todo aquele bloco de escritas em memória que se repete 
    // em todas as funções (o preenchimento da struct `puVar5` ou `puVar3`).
    
    // Agora, as funções 858c e 85a6 apenas calculam seus valores iniciais
    // e chamam esta função para realizar o trabalho pesado.
}

void FUN_000085a6(...)
{
    // Apenas a diferença específica do carregamento do uVar1
    ushort uVar1 = *(ushort *)(param_3 + (param_4 >> 0xc));
    
    // Chama o núcleo comum
    Entity_CommonRegister(param_3, ..., uVar1);
    
    halt_baddata();
}
