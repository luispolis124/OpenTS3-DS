# Fluxo de decompilação — The Sims 3 (DS), ARM9/ARM7

## 0. O que já foi verificado nesta ROM (VS3E, md5 `7cdf32d0…c1`)

Tudo isto vem de `tools/extract_code.py` rodando na ROM, e está em `asm/memory_layout.txt`.

| Item | Valor |
|---|---|
| ARM9 | ROM `0x4000`, 0x17C6C0 bytes, **sem compressão**, carga em `0x02004000`, entry `0x02004800` |
| Secure Area | primeiros 0x800 bytes do ARM9, **já descriptografada** |
| ARM9 em RAM | estático até `0x0217CAE0`, `.bss` até `0x022503A0` |
| Autoload ARM9 | ITCM `0x01FF8000` (0x3AC0 B) e DTCM **`0x02FE0000`** (0x100 B). A base da DTCM é a do TWL-SDK, não a do NitroSDK (`0x027E0000`) |
| ARM7 | carga em `0x02380000` (RAM principal), 0x2909C bytes; quase todo o código está em autoloads (`0x037F8000` e `0x02FE0000`) |
| Overlays ARM9 | 3, sem compressão. O 1 e o 2 compartilham o endereço `0x0225EE20` |
| DSi | `ARM9i` (0x2C74 B) criptografado (modcrypt); `ARM7i` (0x471A4 B) em claro |
| SDK | campo `sdk_version = 0x05047531` (major 5; provável TWL-SDK, deduzido também da base da DTCM) |

O ARM9i criptografado não dá para desmontar sem a chave/decriptação; comece pelo ARM9/ARM7, que rodam no modo DS.

## 1. Preparar o repositório

```
baserom.nds          # sua ROM (NÃO versionar)
tools/               # extract_code.py, ghidra_export_functions.py, thumb_ranges.txt
include/  src/       # C que você vai escrevendo
asm/                 # gerado pela ferramenta (NÃO versionar)
```

`.gitignore`:

```
baserom.nds
*.nds
asm/
build/
```

O conteúdo de `asm/` e as saídas do decompilador são derivados do código da ROM, que é protegido por copyright. O padrão em projetos de decompilação é cada pessoa rodar a extração na sua própria ROM. Se quiser que o build cheque isso, no Makefile:

```make
BASEROM := baserom.nds

asm/layout.json: $(BASEROM) tools/extract_code.py
	python3 tools/extract_code.py $(BASEROM) -o asm --thumb-ranges tools/thumb_ranges.txt

extract: asm/layout.json
```

## 2. Extrair e desmontar

```
pip install capstone                          # ou tenha arm-none-eabi-objdump no PATH
python tools/extract_code.py baserom.nds      # gera asm/
```

Sem capstone nem objdump-ARM a ferramenta cai para `--backend raw` (`.word`). Dá para trabalhar assim, porque o Ghidra é quem vai desmontar de verdade; os `.s` servem de referência e de base para diff.

## 3. Ghidra

### 3.1 Importar

Linguagem: **`ARM:LE:32:v5t`** para o ARM9 (ARM946E-S, ARMv5TE) e **`ARM:LE:32:v4t`** para o ARM7 (ARM7TDMI). Um projeto Ghidra por CPU.

Em cada import: *Format = Raw Binary*, *Language* como acima, e em *Options* o *Base Address* do segmento. Para o ARM9:

| Arquivo (`asm/bin/`) | Base address | Observação |
|---|---|---|
| `arm9_main.bin` | `0x02004800` | importe este primeiro; vira o programa principal |
| `arm9_secure.bin` | `0x02004000` | *File → Add To Program*; é dado, marque como não executável |
| `arm9_itcm.bin` | `0x01FF8000` | *Add To Program* |
| `arm9_dtcm.bin` | `0x02FE0000` | dado |

Depois, em *Window → Memory Map* adicione um bloco **não inicializado** (`.bss`) em `0x0217CAE0`, tamanho `0xD38C0`, e blocos `.bss` de 0x20 B no fim da ITCM e 0xE00 B no fim da DTCM (valores em `asm/memory_layout.txt`).

Para o ARM7 a ideia é a mesma: `arm7_main.bin` em `0x02380000`, `arm7_wram.bin` em `0x037F8000` e `arm7_autoload1.bin` em `0x02FE0000`.

**Overlays**: como o 1 e o 2 se sobrepõem, importe cada overlay (`bin/ov9_NNN.bin`) como bloco *Overlay* (Memory Map → Add Block → marcar *Overlay*) no endereço de `memory_layout.txt`. O Ghidra cria um espaço de endereços separado por bloco, e as chamadas vindas do código estático precisam ser resolvidas manualmente (ver 3.4).

### 3.2 Símbolos e ponto de partida

1. Rode `ImportSymbolsScript.py` (vem com o Ghidra) com `asm/symbols/arm9.sym`. Isso nomeia `_start`, `ModuleParams`, a lista de autoload e o início de cada segmento.
2. Desmonte a partir de `_start` (`D` em `0x02004800`) e rode a análise automática.
3. Habilite *Decompiler Parameter ID*. Deixe *Aggressive Instruction Finder* desligado no começo: em binário misto ARM/Thumb ele gera muito falso positivo.

### 3.3 ARM × Thumb

O Ghidra decide o modo pelo registrador de contexto `TMode`. Ele segue `BX`/`BLX` sozinho, mas funções só chamadas por ponteiro precisam de ajuda: no ponto de entrada, *Set Register Values → TMode = 1*, depois *D*. Quando uma região ficar errada (código virando lixo), quase sempre é modo trocado.

Conforme marcar funções Thumb, exporte `thumb_ranges.txt` (item 5) e rode `extract_code.py` de novo para os `.s` acompanharem.

### 3.4 Identificar o que é SDK e o que é jogo

Boa parte do ARM9 é biblioteca do SDK (OS, MI, FS, GX, SND…), não código do jogo. Identifique-a primeiro, porque tira milhares de funções da frente e dá nomes às que sobram:

- Procure strings de assert/versão do SDK no `.data` (*Search → For Strings*). Elas costumam apontar para os módulos e para a versão do compilador.
- Os projetos de decompilação de outros jogos de DS publicam nomes de funções da mesma família de SDK; compare padrões de bytes e a ordem das funções para reaproveitar nomes.
- Funções de overlay: ache o carregador (função que lê a tabela `y9` e chama o FS), anote o endereço e crie *referências manuais* dos chamadores estáticos para o overlay certo.

### 3.5 Estruturas

Quando descobrir o tamanho de uma struct (stride de laço, argumento do alocador), crie-a em *Data Type Manager* e aplique nos parâmetros (*Retype Variable*). Depois reflita em `include/entity.h` e trave com `STATIC_ASSERT(offsetof(...))`.

## 4. radare2 (alternativa)

```
r2 -a arm -b 32 -m 0x02004800 asm/bin/arm9_main.bin -i asm/arm9.r2
> aaa          # análise
> afl          # lista de funções
> pdf @ fcn.XXXX
> ahb 16 @ 0xENDERECO   # força Thumb a partir de um endereço
```

Com o plugin `r2ghidra` instalado, `pdg @ fcn.XXXX` dá o pseudo-C. O `arm9.r2` gerado já abre os segmentos extras (ITCM) e cria flags para os símbolos. Rode de dentro da raiz do repositório, porque os caminhos são relativos.

## 5. Exportar e transformar em módulos C

1. No Ghidra, rode `tools/ghidra_export_functions.py` (Script Manager, ou `analyzeHeadless … -postScript`). Ele gera:
   - `functions/ENDERECO_nome.c`: pseudo-C de cada função;
   - `index.csv`: endereço, nome, tamanho, modo (arm/thumb) e status;
   - `thumb_ranges.txt`: alimenta o `extract_code.py`.
2. Agrupe as funções em módulos pelo que elas fazem e pelo endereço: funções contíguas costumam vir do mesmo `.c` original. Um módulo por vez, começando pelos folha (sem chamadas) e subindo.
3. Limpe o pseudo-C à mão: tipos reais, nomes, `switch` em vez de cascatas de `if`, estruturas em vez de `*(int*)(p+0x14)`. O pseudo-C do Ghidra **não compila de primeira nem produz o mesmo assembly**; é rascunho.
4. Para *matching* (C que recompila para os mesmos bytes) é preciso o **compilador original**. Descobrir qual (provavelmente Metrowerks CodeWarrior para ARM, mas confirme) e quais flags (`-O4,p`, `-proc arm946e` etc.) é a parte mais trabalhosa; sem isso o resultado é uma decompilação *não-matching*, que ainda é útil. Teste função por função em decomp.me e compare com um diff de assembly (objdiff/asm-differ).
5. Registre cada função concluída no `.sym` (nome real) e reimporte no Ghidra, para os chamadores já aparecerem com nome.

Existe também o **ds-decomp (`dsd`)**, ferramenta feita para esse tipo de projeto: divide a ROM em módulos a partir de uma configuração, gera linker script e objetos "delinkados". Vale avaliar para o esqueleto do build; consulte a documentação atual dele, porque o fluxo de comandos muda entre versões.

## 6. Ordem sugerida

1. `crt0` em assembly (`_start` → cópia dos autoloads → zerar `.bss` → main). É curto, serve de teste do pipeline e valida as constantes de `memory_map.h`.
2. Biblioteca do SDK (OS/MI/FS), identificando e nomeando.
3. Carregador de overlays.
4. Laço principal e a tabela de despacho de entidades (para confirmar `entity.h`).
5. ARM7 (menor, bom para treinar o fluxo completo).

## 7. O que não foi testado

`extract_code.py` foi executado na ROM real nos backends `raw`; o código dos backends `capstone` e `objdump`, o `ghidra_export_functions.py` e o `arm9.r2` não puderam ser exercitados no ambiente onde foram escritos (sem capstone, sem objdump-ARM, sem Ghidra nem radare2). A lógica de chunking e de parsing foi validada com substitutos simulados, mas o primeiro uso real de cada um pode pedir pequenos ajustes de flags/API.
