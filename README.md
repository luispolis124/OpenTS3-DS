# OpenTS3DS

Projeto de engenharia reversa e decompilação de **The Sims 3 (Nintendo DS)**, versão USA
(código `VS3E`, "NDSi Enhanced"), para ARM9 (ARM946E-S) e ARM7 (ARM7TDMI).

> **Estado atual:** o repositório monta o ambiente (extração, layout de memória, linker script,
> build, comparação e empacotamento). O código em `src/` ainda é um **esqueleto**: a ROM gerada
> hoje não é jogável. O objetivo do build agora é validar o toolchain e medir, segmento por
> segmento, o quanto o código compilado se aproxima do binário original (`make compare`).

Este repositório **não contém** a ROM nem código extraído dela. Você precisa da sua própria cópia.

---

## 1. Estrutura

```
Makefile                 build (devkitARM / arm-none-eabi-gcc)
linkscript.ld            linker script único; preprocessado por CPU (-DARM9 / -DARM7)
config/project.json      identidade da ROM, CPUs, segmentos, overlays (dados verificados)
include/
  types.h                tipos básicos, ponto fixo, STATIC_ASSERT
  memory_map.h           mapa de memória, layout desta ROM, registradores
  nds.h                  header global: inclui os dois acima + vídeo/3D/DMA/timers/SPI/som/SWI
  entity.h               esqueleto PROVISÓRIO das entidades do jogo
src/
  arm9.c  arm7.c         pontos de entrada e stubs de init (esqueletos)
  arm9_crt0.s            STUB de crt0 ARM9 (não é o original)
  arm7_crt0.s            STUB de crt0 ARM7 (não é o original)
  arm9/  arm7/           (crie) módulos novos: *.c e *.s entram no build automaticamente
tools/
  extract_code.py        extrai binários/layout da ROM e desmonta  -> asm/
  project_config.py      valida a ROM e a config contra o layout extraído
  rebuild_rom.py         compara o build com o original e empacota a ROM
  ghidra_export_functions.py   exporta pseudo-C do Ghidra
  thumb_ranges.txt       intervalos Thumb (ARM x Thumb não se distingue pelos bytes)
docs/decomp_workflow.md  roteiro Ghidra/radare2 → C
baserom.nds              SUA ROM (ignorada pelo git)
asm/  build/             gerados (ignorados pelo git)
```

## 2. Pré-requisitos

| Item | Observação |
|---|---|
| Toolchain ARM | **devkitARM** *ou* **arm-none-eabi-gcc + binutils** (prefixo `arm-none-eabi-`) |
| `make` | GNU make |
| Python | 3.9 ou superior |
| `capstone` (opcional) | `pip install capstone`; melhora o disassembly dos `.s` |
| Sistema | Linux, macOS ou **WSL2** no Windows |

Instalação do toolchain (escolha um):

```sh
# devkitARM (https://devkitpro.org/wiki/Getting_Started): instale o devkitPro pacman e então
sudo dkp-pacman -S devkitARM

# Debian / Ubuntu
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi make python3

# macOS (Homebrew)
brew install arm-none-eabi-gcc
```

O Makefile usa `$DEVKITARM/bin/arm-none-eabi-` se existir; senão procura `arm-none-eabi-*` no
`PATH`. Para um prefixo diferente: `make PREFIX=/caminho/arm-none-eabi-`.

## 3. ROM de referência

Copie a ROM para a raiz como `baserom.nds`. Ela precisa ser exatamente esta:

| Campo | Valor |
|---|---|
| Código | `VS3E` |
| Tamanho | 134217728 bytes |
| MD5 | `7cdf32d02eeeff592a0a4ab6bec7b5c1` |
| SHA-1 | `623dea494449e3f60ace691dfbf4f7fd285950f6` |

```sh
make verify-rom
```

Com outra versão da ROM os endereços deste repositório **não valem**.

## 4. Extrair os binários originais

```sh
make extract                          # gera asm/ (com disassembly)
make extract EXTRACT_FLAGS=--no-disasm   # só extrai e gera o layout (mais rápido)
make check-layout                     # config/project.json confere com o que foi extraído?
```

O alvo roda `tools/extract_code.py`. Resultado em `asm/`:

| Arquivo | Conteúdo |
|---|---|
| `bin/arm9.bin`, `bin/arm7.bin` | binários exatamente como na ROM |
| `bin/arm9_secure.bin`, `arm9_main.bin`, `arm9_itcm.bin`, … | um arquivo por segmento |
| `bin/ov9_NNN.bin`, `bin/y9.bin` | overlays e tabela de overlays |
| `bin/arm9i.bin`, `bin/arm7i.bin` | binários DSi (o ARM9i é criptografado) |
| `layout.json`, `memory_layout.txt` | layout completo (fonte da verdade) |
| `symbols/*.sym`, `arm9.r2`, `arm7.r2` | símbolos para Ghidra / radare2 |
| `*.s` | disassembly por segmento (backend capstone, objdump ou `raw`) |
| `nitrofs_files.txt` | lista de arquivos do NitroFS |

`asm/bin/arm9_secure.bin` também é consumido pelo build (`src/arm9_crt0.s` o inclui com `.incbin`).

### Layout resumido (confirmado na ROM)

| | ARM9 | ARM7 |
|---|---|---|
| Carga / entrada | `0x02004000` / `0x02004800` | `0x02380000` / `0x02380000` |
| Tamanho na ROM | `0x17C6C0` (sem compressão) | `0x2909C` (sem compressão) |
| Autoload 0 | ITCM `0x01FF8000`, `0x3AC0` + bss `0x20` | WRAM `0x037F8000`, `0x1152C` + bss `0x2954` |
| Autoload 1 | DTCM `0x02FE0000`, `0x100` + bss `0xE00` | `0x02FE0000`, `0x1750C` + bss `0x28F8` |
| `.bss` estático | `0x0217CAE0`–`0x022503A0` | vazio |

A base da DTCM (`0x02FE0000`) é a do TWL-SDK, **não** a do NitroSDK (`0x027E0000`).
O ARM9 ainda traz as tags de SDK `[SDK+NINTENDO:BACKUP]` e `[SDK+Actimagine:Mobiclip SDK V1.2.1]`
(offset `0x1050`): o jogo usa o middleware de vídeo Mobiclip.

## 5. Compilar

```sh
make                 # build/arm9.bin e build/arm7.bin
make compare         # % de bytes iguais ao original, por segmento
make rom             # build/sims3.nds (patcha a ROM base com os binários novos)
make selftest        # reempacota os binários ORIGINAIS: precisa reproduzir a ROM bit a bit
make help            # todos os alvos
```

Opções úteis: `V=1` (mostra os comandos), `DEBUG=1` (`-g`), `OPT=-O1`, `STRICT=1`.

- **`make compare` termina com erro enquanto houver diferença.** É de propósito (serve de teste em CI).
- **`STRICT=1`** liga asserts no linker que exigem tamanhos de seção idênticos aos do original.
  Use quando estiver fazendo matching; no dia a dia deixe desligado.
- **`make rom` hoje gera uma ROM que não funciona**, porque o código compilado ainda é muito menor
  que o original. A ferramenta avisa.
- **Folga de tamanho:** o `pack` troca os binários no lugar. Sobram só `0x140` bytes no ARM9 e
  `0x164` no ARM7 até a próxima estrutura da ROM. Binários maiores exigem reconstruir o NitroFS
  (ndstool) e mover as demais estruturas. O empacotador também **não** recalcula o CRC da Secure
  Area nem os hashes/assinatura do cabeçalho DSi: use a ROM gerada em emulador/flashcart no
  modo DS.

### Adicionando código

Crie `src/arm9/meu_modulo.c` (ou `src/arm7/…`): o Makefile inclui `*.c` e `*.s` dessas pastas
sozinho. Comece o arquivo com `#include "nds.h"`.

Para colocar código ou dados em uma região específica, use as macros de `nds.h`:

```c
#include "nds.h"

ITCM_CODE void HotLoop(void);      /* ARM9: ITCM    (.itcm)      */
DTCM_BSS  u32  gScratch[64];       /* ARM9: DTCM    (.dtcm_bss)  */
WRAM_CODE void Mix(void);          /* ARM7: WRAM    (.wram)      */
HIGH_BSS  u8   gBuffer[0x1000];    /* ARM7: RAM alta (.high_bss) */
```

## 6. Linker script

`linkscript.ld` é um arquivo só, preprocessado com `cpp` para cada CPU (o Makefile faz isso):

```sh
cpp -P -x c -undef -DARM9 linkscript.ld > build/arm9.ld
```

O formato da imagem é o do SDK: `[estático][dados do autoload 0][dados do autoload 1][lista]`.
As seções de TCM têm endereço de execução (VMA) na TCM e de carga (LMA) logo após o bloco
estático; `objcopy -O binary` segue a ordem de LMA e produz um arquivo no mesmo formato do
`arm9.bin` da ROM. O `ModuleParams` (9 palavras) é gerado com `LONG()` no endereço original
(`0x02004FB0` no ARM9, `0x023805D4` no ARM7). O `.bss` estático começa em cima da área onde os
autoloads são carregados (como no original), por isso o crt0 copia os autoloads **antes** de
zerar o `.bss`.

Não coberto: overlays (cada um precisa do seu script), seções de C++ (`.init_array`), e o 3º
campo das entradas da lista de autoload (desconhecido; emitimos o endereço).

## 7. Configuração (`config/project.json`)

JSON com a identidade da ROM, os dois CPUs (offsets, carga, entrada, `ModuleParams`,
segmentos), overlays, binários DSi e caminhos de ferramentas. Endereços em decimal.

```sh
python tools/project_config.py show          # resumo em hex
python tools/project_config.py verify-rom baserom.nds
python tools/project_config.py check         # config x asm/layout.json
python tools/project_config.py get cpus.arm9.ghidra_language   # ARM:LE:32:v5t
python tools/project_config.py init          # regenera a partir de asm/layout.json
```

Em scripts Python: `from project_config import load; cfg = load()`.
Linguagens do Ghidra: **`ARM:LE:32:v5t`** (ARM9) e **`ARM:LE:32:v4t`** (ARM7).

## 8. Análise no Ghidra / radare2

Veja `docs/decomp_workflow.md` (importação por segmento, Thumb, overlays, símbolos). Resumo:

```sh
r2 -a arm -b 32 -m 0x02004800 asm/bin/arm9_main.bin -i asm/arm9.r2
```

No Ghidra, `tools/ghidra_export_functions.py` exporta o pseudo-C e gera o `thumb_ranges.txt`
que `tools/extract_code.py --thumb-ranges` usa.

## 9. Matching

"Matching" significa recompilar C e obter **os mesmos bytes** do original. Isso exige o
**compilador original, na versão e com as flags originais**. Este repositório usa GCC, que
**não** reproduz o código do jogo byte a byte. O build atual serve para:

- validar o layout de memória e o formato da imagem;
- escrever e testar a lógica em C (decompilação funcional, "non-matching");
- medir a distância com `make compare`.

O compilador do jogo **ainda não foi identificado**: uma busca simples por strings no ARM9/ARM7 e
nos overlays não achou assinatura de compilador. Antes de investir em matching:

1. identifique o compilador e as flags (comparando o código gerado por candidatos com funções
   pequenas do binário; decomp.me ajuda);
2. confira se um linker script do GNU `ld` basta ou se será preciso o linker do mesmo fabricante;
3. troque `CC`/`CFLAGS9`/`CFLAGS7` (`make CC=… CFLAGS9=…`) e use um diff de assembly
   (objdiff/asm-differ) por função.

## 10. O que foi testado

| Item | Como | Resultado |
|---|---|---|
| `extract_code.py`, `project_config.py`, `rebuild_rom.py` | rodados na ROM real | ok; `selftest` reproduz a ROM bit a bit; CRC16 do cabeçalho confere |
| `linkscript.ld` | linkado com o `ld` do host (x86) usando os fontes reais e objetos sintéticos, ARM9 e ARM7, com e sem `STRICT=1` | ModuleParams, lista de autoload e imagem corretos; asserts disparam |
| Makefile | fluxo completo com um toolchain falso que traduz `arm-none-eabi-*` para o host | extract, build, incremental, `-j`, relink, compare, rom, selftest, clean |
| C/H | compilados com `gcc -std=c99 -Wall -Wextra -Wpedantic` | sem avisos |
| **Não testado** | compilar/assemblar de verdade com `arm-none-eabi-gcc`; os `crt0` em assembly; boot em emulador ou hardware; Ghidra/radare2; backends capstone/objdump com saída real | podem exigir ajustes |

Os `crt0` são stubs escritos sem assembler ARM disponível: valores de CP15 e sequência de
inicialização seguem o padrão do hardware, mas devem ser trocados pela decompilação do `_start`
real (`0x02004800`).

## 11. Problemas comuns

| Sintoma | Causa provável |
|---|---|
| `arm-none-eabi-gcc nao encontrado` | toolchain fora do `PATH`; use `PREFIX=…` ou exporte `DEVKITARM` |
| `baserom.nds nao encontrado` | copie a ROM para a raiz (seção 3) |
| `ROM diferente da de referência` | outra versão/região da ROM; os endereços não valem |
| `.module_params` / `.crt0` sobrepõem | `src/arm9_crt0.s` passou de `0x7B0` bytes (ARM9) ou `0x5D4` (ARM7) |
| `ARM9: .bss estatico invade o overlay 0` | `.bss`/`.data` cresceram além de `0x022503A0` |
| `ARM9: DTCM sem espaco para a pilha` | seções `.dtcm*` grandes demais (16 KB menos 2 KB de pilha e 1 KB de sistema) |
| `make compare` falha | esperado enquanto houver diferença; veja o % por segmento |
| `make rom`: "só cabem 0x…" | binário maior que o espaço livre (seção 5) |

## 12. Aviso legal

The Sims 3 e o código do jogo pertencem aos seus titulares. Este projeto não distribui a ROM nem
código derivado dela. `asm/`, `build/` e `*.nds` ficam no `.gitignore`. Use apenas uma cópia que
você possui legalmente.
