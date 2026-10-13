# --- Configurações do Toolchain ---
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

# --- Flags de Compilação para ARM9 / ARM7 ---
CFLAGS = -Wall -O2 -mcpu=arm946e-s -mthumb -ffreestanding -Iinclude
LDFLAGS = -T linkscript.ld -nostdlib

# --- Arquivos Fonte ---
SRC_ARM9 = src/arm9.c
SRC_ARM7 = src/arm7.c

# --- Alvos de Build ---
all: rom.nds

rom.nds: arm9.bin arm7.bin
	@echo "Gerando ROM final do NDS..."
	# Ferramenta de empacotamento (como ndstool) entraria aqui

arm9.bin: $(SRC_ARM9)
	$(CC) $(CFLAGS) -c $(SRC_ARM9) -o arm9.o
	$(LD) $(LDFLAGS) arm9.o -o arm9.elf
	$(OBJCOPY) -O binary arm9.elf arm9.bin

arm7.bin: $(SRC_ARM7)
	$(CC) $(CFLAGS) -c $(SRC_ARM7) -o arm7.o
	$(LD) $(LDFLAGS) arm7.o -o arm7.elf
	$(OBJCOPY) -O binary arm7.elf arm7.bin

clean:
	rm -f *.o *.elf *.bin *.nds
