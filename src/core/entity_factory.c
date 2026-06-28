/* ==========================================================================
   MÓDULO: Entity Factory
   DESCRIÇÃO: Sistema central de criação e registro de entidades.
   ========================================================================== */

// Função privada que realiza o trabalho pesado de escrita em memória
static void Internal_RegisterEntity(int entity_id, uint base_param, int type_id)
{
    // Mova aqui todo o bloco de código repetido que preenche:
    // puVar1[0xc], piVar4, puVar7, etc.
    // Isso é o registro real da entidade no sistema global de Sims 3 DS.
}

// Interfaces públicas que apenas preparam os dados e chamam o núcleo
void FUN_000085ca(int param_1, undefined4 param_2, int param_3, uint param_4) 
{
    // A lógica única desta função está apenas no cálculo inicial dos offsets
    // Exemplo:
    // int derived_offset = (param_1 * 0x200);
    
    // Chama o núcleo comum de registro
    Internal_RegisterEntity(param_1, param_4, param_2);
    
    // halt_baddata(); // Removemos para evitar crash no port
}
