#ifndef OTS3_CONFIG_H
#define OTS3_CONFIG_H

/*
 * config.h - plataforma, orçamentos e tamanhos fixos do motor OpenTS3-DS.
 *
 * O mesmo código (entity, nav, simulation, save, assets) compila para:
 *   -DNDS_ARM9   hardware original (ARM946E-S, 4 MB de RAM, 656 KB de VRAM)
 *   -DNATIVE_PC  port nativo (GCC/Clang/MSVC em C99 ou C++)
 *
 * Princípios que valem para todos os módulos:
 *   - nenhum malloc/free; todo armazenamento é estático e dimensionado aqui;
 *   - estado persistente sem ponteiros: referências são EntityId (índice + geração)
 *     ou índices pequenos, sempre validados antes do uso;
 *   - aritmética inteira / ponto fixo no caminho da simulação (determinismo);
 *   - tipos de tamanho fixo e serialização campo a campo (nada de memcpy de struct).
 */

#include "types.h"

#if defined(NATIVE_PC) && defined(NDS_ARM9)
#  error "NATIVE_PC e NDS_ARM9 sao mutuamente exclusivos"
#endif

#if defined(NDS_ARM9)
#  define OTS3_NDS 1
#  ifndef ARM9
#    define ARM9 1          /* habilita as macros de seção (ITCM_CODE...) de nds.h */
#  endif
#  include "nds.h"
#elif defined(NATIVE_PC)
#  define OTS3_PC 1
#else
#  error "Defina NATIVE_PC ou NDS_ARM9"
#endif

#ifdef __cplusplus
#  define OTS3_EXTERN_C_BEGIN extern "C" {
#  define OTS3_EXTERN_C_END   }
#else
#  define OTS3_EXTERN_C_BEGIN
#  define OTS3_EXTERN_C_END
#endif

/* ---------------------------------------------------------------------------
 * Seções do linkscript.ld (ITCM = 32 KB, DTCM = 16 KB). Só valem no NDS.
 * OTS3_HOT: laços quentes (decaimento, A*, passo da simulação) vão para a ITCM,
 * que é executada sem espera. Na PC não fazem nada.
 * ------------------------------------------------------------------------- */
#if defined(OTS3_NDS)
#  define OTS3_HOT        ITCM_CODE
#  define OTS3_HOT_DATA   DTCM_DATA
#else
#  define OTS3_HOT
#  define OTS3_HOT_DATA
#endif

/* ---------------------------------------------------------------------------
 * Orçamento de hardware (NDS)
 * ------------------------------------------------------------------------- */
#define OTS3_RAM_TOTAL_BYTES      (4u * 1024u * 1024u)
/* VRAM: bancos A-D 128 KB, E 64 KB, F/G 16 KB, H 32 KB, I 16 KB = 656 KB */
#define OTS3_VRAM_TOTAL_BYTES     (656u * 1024u)

/* Partições lógicas da VRAM (somam 656 KB). O jogo original usa a sua parte; os pets
 * só podem gastar a deles. Ajuste conforme medir o uso real. */
#define OTS3_VRAM_BUDGET_WORLD    (400u * 1024u)
#define OTS3_VRAM_BUDGET_UI       (96u * 1024u)
#define OTS3_VRAM_BUDGET_PET      (96u * 1024u)
#define OTS3_VRAM_BUDGET_RESERVED (64u * 1024u)

/* ---------------------------------------------------------------------------
 * Simulação: passo fixo
 * ------------------------------------------------------------------------- */
#define OTS3_SIM_HZ                 30u
#define OTS3_STEP_US                (1000000u / OTS3_SIM_HZ)   /* 33333 us */
#define OTS3_MAX_SPEED              3u        /* 0 = pausa, 1x, 2x, 3x */
#define OTS3_MAX_STEPS_PER_UPDATE   6u        /* proteção contra a "espiral da morte" */
#define OTS3_MAX_DELTA_US           250000u   /* quadros mais longos são cortados */
#define OTS3_TICKS_PER_GAME_MINUTE  30u       /* 1x: 1 s real = 1 minuto de jogo */
#define OTS3_MINUTES_PER_DAY        1440u

/* ---------------------------------------------------------------------------
 * Pools fixos
 * ------------------------------------------------------------------------- */
#define OTS3_MAX_ENTITIES     16u   /* Sims + pets vivos ao mesmo tempo */
#define OTS3_MAX_HOUSEHOLDS   4u
#define OTS3_MAX_OBJECTS      96u
#define OTS3_NAME_LEN         12u   /* nome de Sim/pet, com o NUL */
#define OTS3_HOUSEHOLD_NAME_LEN 16u

/* ---------------------------------------------------------------------------
 * Save
 * ------------------------------------------------------------------------- */
#define OTS3_SAVE_PAYLOAD_MAX 16384u

/* ---------------------------------------------------------------------------
 * Asserções em tempo de compilação (nomes únicos por arquivo)
 * ------------------------------------------------------------------------- */
#define OTS3_STATIC_ASSERT(name, cond) STATIC_ASSERT(name, cond)

OTS3_STATIC_ASSERT(config_vram_partition_sum,
    OTS3_VRAM_BUDGET_WORLD + OTS3_VRAM_BUDGET_UI + OTS3_VRAM_BUDGET_PET +
    OTS3_VRAM_BUDGET_RESERVED == OTS3_VRAM_TOTAL_BYTES);
OTS3_STATIC_ASSERT(config_entities_fit_u8, OTS3_MAX_ENTITIES < 255u);
OTS3_STATIC_ASSERT(config_objects_fit_u8, OTS3_MAX_OBJECTS < 255u);

#endif /* OTS3_CONFIG_H */
