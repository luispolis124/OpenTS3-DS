# Makefile - OpenTS3DS (The Sims 3 / Nintendo DS, VS3E)
#
#   make extract       extrai binários e layout da ROM para asm/ (precisa de baserom.nds)
#   make               compila build/arm9.bin e build/arm7.bin
#   make compare       compara o compilado com o original (% igual por segmento)
#   make rom           gera build/sims3.nds (patcha a ROM base; ver tools/rebuild_rom.py)
#   make help          lista todos os alvos
#
# Toolchain: devkitARM ou arm-none-eabi-gcc (binutils + gcc). Veja README.md.
# IMPORTANTE: GCC NAO gera os mesmos bytes do compilador original do jogo. Este Makefile
# produz um build funcional/estrutural e mede a distancia (make compare). Matching de
# verdade exige o compilador original (ver README, secao "Matching").

# ----------------------------------------------------------------------------
# Configuracao
# ----------------------------------------------------------------------------
BASEROM  ?= baserom.nds
BUILD    ?= build
ASM      ?= asm
PYTHON   ?= python3
OPT      ?= -O2
STRICT   ?= 0
EXTRACT_FLAGS ?=
V        ?= 0

ifeq ($(V),1)
  Q :=
else
  Q := @
endif

# Prefixo do toolchain: usa DEVKITARM se existir, senao procura no PATH.
DEVKITARM ?= /opt/devkitpro/devkitARM
ifneq ($(wildcard $(DEVKITARM)/bin/arm-none-eabi-gcc),)
  PREFIX ?= $(DEVKITARM)/bin/arm-none-eabi-
else
  PREFIX ?= arm-none-eabi-
endif

CC      := $(PREFIX)gcc
LD      := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
SIZE    := $(PREFIX)size

# Flags por CPU (sobrescreva na linha de comando se precisar)
ARCH9  ?= -mcpu=arm946e-s -marm
ARCH7  ?= -mcpu=arm7tdmi -marm

# -fno-short-enums: arm-none-eabi usa enums de 1 byte por padrao; o SDK usa int.
CFLAGS_COMMON := -std=gnu99 $(OPT) -Wall -Wextra -ffreestanding -fno-builtin -fno-common \
                 -fno-short-enums -fno-unwind-tables -fno-asynchronous-unwind-tables \
                 -fno-stack-protector -Iinclude
ifeq ($(DEBUG),1)
  CFLAGS_COMMON += -g
endif
CFLAGS9 := $(CFLAGS_COMMON) $(ARCH9) -DARM9
CFLAGS7 := $(CFLAGS_COMMON) $(ARCH7) -DARM7
ASFLAGS9 := $(ARCH9) -DARM9 -Iinclude -x assembler-with-cpp
ASFLAGS7 := $(ARCH7) -DARM7 -Iinclude -x assembler-with-cpp

# Pre-processamento do linkscript (-DSTRICT_LAYOUT liga as ASSERTs de tamanho original)
LDDEFS :=
ifeq ($(STRICT),1)
  LDDEFS += -DSTRICT_LAYOUT
endif
# binutils >= 2.39 avisa sobre segmentos RWX; os DS sao RWX por natureza.
LDFLAGS := $(shell $(LD) --help 2>/dev/null | grep -q -- --no-warn-rwx-segments && echo --no-warn-rwx-segments)

# ----------------------------------------------------------------------------
# Fontes (adicione modulos novos em src/arm9/ ou src/arm7/)
# ----------------------------------------------------------------------------
SRC9_C := $(wildcard src/arm9.c src/arm9/*.c)
SRC9_S := $(wildcard src/arm9_crt0.s src/arm9/*.s)
SRC7_C := $(wildcard src/arm7.c src/arm7/*.c)
SRC7_S := $(wildcard src/arm7_crt0.s src/arm7/*.s)

OBJ9 := $(SRC9_C:%.c=$(BUILD)/arm9/%.o) $(SRC9_S:%.s=$(BUILD)/arm9/%.o)
OBJ7 := $(SRC7_C:%.c=$(BUILD)/arm7/%.o) $(SRC7_S:%.s=$(BUILD)/arm7/%.o)
DEPS := $(OBJ9:.o=.d) $(OBJ7:.o=.d)

# ----------------------------------------------------------------------------
# Alvos principais
# ----------------------------------------------------------------------------
.PHONY: all arm9 arm7 extract verify-rom check-layout compare rom selftest dis size \
        check-tools clean distclean help
.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

all: arm9 arm7
arm9: $(BUILD)/arm9.bin
arm7: $(BUILD)/arm7.bin

help:
	@echo "Alvos:"
	@echo "  extract       extrai arm9/arm7/overlays e layout da ROM para $(ASM)/"
	@echo "  all           compila $(BUILD)/arm9.bin e $(BUILD)/arm7.bin  (padrao)"
	@echo "  compare       compara o compilado com o original, por segmento"
	@echo "  rom           gera $(BUILD)/sims3.nds a partir da ROM base"
	@echo "  selftest      reempacota os binarios originais e confere bit a bit"
	@echo "  verify-rom    confere hash/tamanho de $(BASEROM) com config/project.json"
	@echo "  check-layout  confere config/project.json com $(ASM)/layout.json"
	@echo "  dis           gera $(BUILD)/*.dis (objdump -d dos ELFs)"
	@echo "  size          tamanho das secoes dos ELFs"
	@echo "  clean | distclean   (distclean tambem apaga $(ASM)/)"
	@echo "Variaveis: STRICT=1 (exige layout identico), V=1 (verboso), DEBUG=1, OPT=-O1, PREFIX=..., EXTRACT_FLAGS=--no-disasm"

check-tools:
	@command -v $(CC) >/dev/null 2>&1 || { echo "[erro] $(CC) nao encontrado. Instale devkitARM ou arm-none-eabi-gcc (README.md), ou passe PREFIX=..."; exit 1; }
	@command -v $(LD) >/dev/null 2>&1 || { echo "[erro] $(LD) nao encontrado."; exit 1; }
	@command -v $(OBJCOPY) >/dev/null 2>&1 || { echo "[erro] $(OBJCOPY) nao encontrado."; exit 1; }

$(BASEROM):
	@echo "[erro] $(BASEROM) nao encontrado. Copie a sua ROM (USA, codigo VS3E) para ./$(BASEROM) (README.md)."; exit 1

# ----------------------------------------------------------------------------
# Extracao da ROM original (gera asm/, incluindo bin/arm9_secure.bin usado pelo crt0)
# ----------------------------------------------------------------------------
extract: $(ASM)/layout.json

$(ASM)/layout.json: $(BASEROM) tools/extract_code.py
	$(Q)$(PYTHON) tools/project_config.py verify-rom $(BASEROM)
	$(Q)$(PYTHON) tools/extract_code.py $(BASEROM) -o $(ASM) --thumb-ranges tools/thumb_ranges.txt $(EXTRACT_FLAGS)

$(ASM)/bin/arm9_secure.bin: $(ASM)/layout.json
	@touch $@

verify-rom:
	$(Q)$(PYTHON) tools/project_config.py verify-rom $(BASEROM)

check-layout:
	$(Q)$(PYTHON) tools/project_config.py check --layout $(ASM)/layout.json

# ----------------------------------------------------------------------------
# Compilacao
# ----------------------------------------------------------------------------
$(BUILD)/arm9/%.o: %.c | check-tools
	@echo "CC9  $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS9) -MMD -MP -c -o $@ $<

$(BUILD)/arm7/%.o: %.c | check-tools
	@echo "CC7  $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS7) -MMD -MP -c -o $@ $<

$(BUILD)/arm9/%.o: %.s | check-tools
	@echo "AS9  $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(ASFLAGS9) -MMD -MP -c -o $@ $<

$(BUILD)/arm7/%.o: %.s | check-tools
	@echo "AS7  $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(ASFLAGS7) -MMD -MP -c -o $@ $<

# O crt0 do ARM9 inclui a Secure Area extraida da ROM
$(BUILD)/arm9/src/arm9_crt0.o: $(ASM)/bin/arm9_secure.bin

# Linkscript: um arquivo, preprocessado por CPU
$(BUILD)/arm9.ld: linkscript.ld | check-tools
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -E -P -x c -undef -DARM9 $(LDDEFS) $< -o $@

$(BUILD)/arm7.ld: linkscript.ld | check-tools
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -E -P -x c -undef -DARM7 $(LDDEFS) $< -o $@

$(BUILD)/arm9.elf: $(OBJ9) $(BUILD)/arm9.ld
	@echo "LD9  $@"
	$(Q)$(LD) $(LDFLAGS) -T $(BUILD)/arm9.ld -Map $(BUILD)/arm9.map -o $@ $(OBJ9)

$(BUILD)/arm7.elf: $(OBJ7) $(BUILD)/arm7.ld
	@echo "LD7  $@"
	$(Q)$(LD) $(LDFLAGS) -T $(BUILD)/arm7.ld -Map $(BUILD)/arm7.map -o $@ $(OBJ7)

# A imagem binaria segue a ordem de LMA: [estatico][autoload ITCM][autoload DTCM][lista],
# o mesmo formato do arm9.bin/arm7.bin da ROM.
$(BUILD)/%.bin: $(BUILD)/%.elf
	@echo "BIN  $@"
	$(Q)$(OBJCOPY) -O binary $< $@

# ----------------------------------------------------------------------------
# Comparacao, ROM final, utilitarios
# ----------------------------------------------------------------------------
compare: all
	$(Q)$(PYTHON) tools/rebuild_rom.py compare --build-dir $(BUILD) --asm-dir $(ASM)

rom: all
	$(Q)$(PYTHON) tools/rebuild_rom.py pack --rom $(BASEROM) --arm9 $(BUILD)/arm9.bin \
	    --arm7 $(BUILD)/arm7.bin -o $(BUILD)/sims3.nds

selftest: $(ASM)/layout.json
	$(Q)$(PYTHON) tools/rebuild_rom.py selftest --rom $(BASEROM) --asm-dir $(ASM)

dis: $(BUILD)/arm9.elf $(BUILD)/arm7.elf
	$(Q)$(OBJDUMP) -d $(BUILD)/arm9.elf > $(BUILD)/arm9.dis
	$(Q)$(OBJDUMP) -d $(BUILD)/arm7.elf > $(BUILD)/arm7.dis

size: $(BUILD)/arm9.elf $(BUILD)/arm7.elf
	$(Q)$(SIZE) -A $(BUILD)/arm9.elf $(BUILD)/arm7.elf

clean:
	$(Q)rm -rf $(BUILD)

distclean: clean
	$(Q)rm -rf $(ASM)

# make print-CFLAGS9  -> mostra o valor de uma variavel
print-%:
	@echo '$*=$($*)'

-include $(DEPS)
