# OpenTS3DS

Reverse engineering and decompilation project for **The Sims 3 (Nintendo DS)**, USA version 
(code `VS3E`, "NDSi Enhanced"), targeting ARM9 (ARM946E-S) and ARM7 (ARM7TDMI).

> **Current status:** The repository sets up the environment (extraction, memory layout, linker script,
> build, comparison, and repacking). The code in `src/` is still a **skeleton**: the generated ROM
> is currently not playable. The build goal right now is to validate the toolchain and measure, segment
> by segment, how closely the compiled code approaches the original binary (`make compare`).

This repository **does not contain** the ROM or any code extracted from it. You must provide your own copy.

---

## 1. Structure


Makefile                 build (devkitARM / arm-none-eabi-gcc)
linkscript.ld            single linker script; preprocessed via CPU (-DARM9 / -DARM7)
config/project.json      ROM identity, CPUs, segments, overlays (verified data)
include/
types.h                basic types, fixed point, STATIC_ASSERT
memory_map.h           memory map, ROM layout, registers
nds.h                  global header: includes the above + video/3D/DMA/timers/SPI/sound/SWI
entity.h               PROVISIONAL skeleton of game entities
src/
arm9.c  arm7.c         entry points and initialization stubs (skeletons)
arm9_crt0.s            ARM9 crt0 STUB (not original)
arm7_crt0.s            ARM7 crt0 STUB (not original)
arm9/  arm7/           (create) new modules: *.c and *.s automatically included in build
tools/
extract_code.py        extracts binaries/layout from ROM and disassembles -> asm/
project_config.py      validates ROM and config against extracted layout
rebuild_rom.py         compares build with original and repacks ROM
ghidra_export_functions.py   exports Ghidra pseudo-C
thumb_ranges.txt       Thumb ranges (ARM vs Thumb cannot be distinguished by bytes alone)
docs/decomp_workflow.md  Ghidra/radare2 → C guide
baserom.nds              YOUR ROM (ignored by git)
asm/  build/             generated (ignored by git)

## 2. Prerequisites

| Item | Note |
|---|---|
| ARM Toolchain | **devkitARM** *or* **arm-none-eabi-gcc + binutils** (`arm-none-eabi-` prefix) |
| `make` | GNU make |
| Python | 3.9 or higher |
| `capstone` (optional) | `pip install capstone`; improves `.s` disassembly |
| System | Linux, macOS, or **WSL2** on Windows |

Toolchain installation (choose one):

```sh
# devkitARM ([https://devkitpro.org/wiki/Getting_Started](https://devkitpro.org/wiki/Getting_Started)): install devkitPro pacman then
sudo dkp-pacman -S devkitARM

# Debian / Ubuntu
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi make python3

# macOS (Homebrew)
brew install arm-none-eabi-gcc

The Makefile uses $DEVKITARM/bin/arm-none-eabi- if it exists; otherwise, it looks for arm-none-eabi-* in the
PATH. For a different prefix: make PREFIX=/path/to/arm-none-eabi-.
3. Reference ROM
Copy your ROM to the root directory as baserom.nds. It must match this exact version:
| Field | Value |
|---|---|
| Code | VS3E |
| Size | 134217728 bytes |
| MD5 | 7cdf32d02eeeff592a0a4ab6bec7b5c1 |
| SHA-1 | 623dea494449e3f60ace691dfbf4f7fd285950f6 |
make verify-rom

If you use a different ROM version, the addresses in this repository will not match.
4. Extracting Original Binaries
make extract                          # generates asm/ (with disassembly)
make extract EXTRACT_FLAGS=--no-disasm   # extracts and generates layout only (faster)
make check-layout                     # checks config/project.json against extracted layout

This target runs tools/extract_code.py. Output in asm/:
| File | Content |
|---|---|
| bin/arm9.bin, bin/arm7.bin | binaries exactly as they are in the ROM |
| bin/arm9_secure.bin, arm9_main.bin, arm9_itcm.bin, … | one file per segment |
| bin/ov9_NNN.bin, bin/y9.bin | overlays and overlay table |
| bin/arm9i.bin, bin/arm7i.bin | DSi binaries (ARM9i is encrypted) |
| layout.json, memory_layout.txt | complete layout (source of truth) |
| symbols/*.sym, arm9.r2, arm7.r2 | symbols for Ghidra / radare2 |
| *.s | disassembly per segment (capstone backend, objdump, or raw) |
| nitrofs_files.txt | NitroFS file list |
asm/bin/arm9_secure.bin is also consumed by the build (src/arm9_crt0.s includes it via .incbin).
Summary Layout (confirmed in ROM)
|  | ARM9 | ARM7 |
|---|---|---|
| Load / Entry | 0x02004000 / 0x02004800 | 0x02380000 / 0x02380000 |
| ROM Size | 0x17C6C0 (uncompressed) | 0x2909C (uncompressed) |
| Autoload 0 | ITCM 0x01FF8000, 0x3AC0 + bss 0x20 | WRAM 0x037F8000, 0x1152C + bss 0x2954 |
| Autoload 1 | DTCM 0x02FE0000, 0x100 + bss 0xE00 | 0x02FE0000, 0x1750C + bss 0x28F8 |
| Static .bss | 0x0217CAE0–0x022503A0 | empty |
The DTCM base (0x02FE0000) is from the TWL-SDK, not NitroSDK (0x027E0000).
ARM9 also contains the SDK tags [SDK+NINTENDO:BACKUP] and [SDK+Actimagine:Mobiclip SDK V1.2.1]
(offset 0x1050): the game uses Mobiclip video middleware.
5. Building
make                 # build/arm9.bin and build/arm7.bin
make compare         # % of matching bytes compared to original, per segment
make rom             # build/sims3.nds (patches base ROM with new binaries)
make selftest        # repacks ORIGINAL binaries: must reproduce ROM bit-by-bit
make help            # all targets

Useful options: V=1 (verbose commands), DEBUG=1 (-g), OPT=-O1, STRICT=1.
 * make compare exits with an error while there are differences. This is intentional (used as a CI test).
 * STRICT=1 enables linker asserts that require section sizes to be identical to the original.
   Use when match-building; keep disabled during daily development.
 * make rom currently generates a non-working ROM, because the compiled code is still much smaller
   than the original. The tool will warn you.
 * Size headroom: pack swaps binaries in-place. There is only 0x140 bytes left on ARM9 and
   0x164 on ARM7 before the next ROM structure. Larger binaries require rebuilding the NitroFS
   (ndstool) and moving other structures. The packer also does not recalculate the Secure Area CRC
   or DSi header hashes/signatures: use the generated ROM in an emulator or flashcart in DS mode.
Adding Code
Create src/arm9/my_module.c (or src/arm7/…): the Makefile automatically includes *.c and *.s from these folders. Start your file with #include "nds.h".
To place code or data into a specific region, use nds.h macros:
#include "nds.h"

ITCM_CODE void HotLoop(void);      /* ARM9: ITCM    (.itcm)      */
DTCM_BSS  u32  gScratch[64];       /* ARM9: DTCM    (.dtcm_bss)  */
WRAM_CODE void Mix(void);          /* ARM7: WRAM    (.wram)      */
HIGH_BSS  u8   gBuffer[0x1000];    /* ARM7: High RAM (.high_bss) */

6. Linker Script
linkscript.ld is a single file preprocessed with cpp for each CPU (handled by the Makefile):
cpp -P -x c -undef -DARM9 linkscript.ld > build/arm9.ld

The image format follows the SDK layout: [static][autoload 0 data][autoload 1 data][list].
TCM sections have execution addresses (VMA) in TCM and load addresses (LMA) right after the static block;
objcopy -O binary follows LMA order, producing a file in the same format as the ROM's arm9.bin.
ModuleParams (9 words) is generated via LONG() at the original address (0x02004FB0 on ARM9,
0x023805D4 on ARM7). Static .bss starts right above the area where autoloads are loaded (just like
in the original), which is why crt0 copies the autoloads before clearing .bss.
Not covered: overlays (each requires its own script), C++ sections (.init_array), and the 3rd
field of the autoload list entries (unknown; outputs address).
7. Configuration (config/project.json)
JSON file containing ROM identity, both CPUs (offsets, load, entry, ModuleParams, segments),
overlays, DSi binaries, and tool paths. Decimal addresses.
python tools/project_config.py show          # summary in hex
python tools/project_config.py verify-rom baserom.nds
python tools/project_config.py check         # config vs asm/layout.json
python tools/project_config.py get cpus.arm9.ghidra_language   # ARM:LE:32:v5t
python tools/project_config.py init          # regenerates from asm/layout.json

In Python scripts: from project_config import load; cfg = load().
Ghidra languages: ARM:LE:32:v5t (ARM9) and ARM:LE:32:v4t (ARM7).
8. Ghidra / radare2 Analysis
See docs/decomp_workflow.md (segment-based import, Thumb, overlays, symbols). Summary:
r2 -a arm -b 32 -m 0x02004800 asm/bin/arm9_main.bin -i asm/arm9.r2

In Ghidra, tools/ghidra_export_functions.py exports pseudo-C and generates thumb_ranges.txt,
which is used by tools/extract_code.py --thumb-ranges.
9. Matching
"Matching" means recompiling C code to yield exact byte-for-byte matches with the original. This requires
the original compiler version with original flags. This repository uses GCC, which
does not reproduce game code byte-for-byte. The current build serves to:
 * validate the memory layout and image format;
 * write and test C logic (functional decompilation, "non-matching");
 * measure distance using make compare.
The game's compiler has not been identified yet: a simple string search in ARM9/ARM7 and overlays
did not reveal any compiler signatures. Before investing in matching:
 * identify the compiler and flags (comparing candidate-generated code with small binary functions; decomp.me helps);
 * verify whether a GNU ld linker script is sufficient or if the manufacturer's linker is required;
 * swap CC/CFLAGS9/CFLAGS7 (make CC=… CFLAGS9=…) and use function-level assembly diffing
   (objdiff/asm-differ).
10. Tested Items
| Item | Method | Result |
|---|---|---|
| extract_code.py, project_config.py, rebuild_rom.py | run on real ROM | ok; selftest reproduces ROM bit-by-bit; header CRC16 matches |
| linkscript.ld | linked with host (x86) ld using real sources and synthetic objects, ARM9 and ARM7, with and without STRICT=1 | ModuleParams, autoload list, and image are correct; asserts trigger |
| Makefile | full workflow using a fake toolchain translating arm-none-eabi-* to host | extract, build, incremental, -j, relink, compare, rom, selftest, clean |
| C/H | compiled with gcc -std=c99 -Wall -Wextra -Wpedantic | no warnings |
| Not tested | actual compilation/assembly with arm-none-eabi-gcc; assembly crt0 files; booting in an emulator or hardware; Ghidra/radare2; capstone/objdump backends with real output | may require tweaks |
The crt0 files are stubs written without an available ARM assembler: CP15 values and initialization
sequence follow hardware standards, but should be replaced by decompilation of the real _start
(0x02004800).
11. Common Issues
| Symptom | Probable Cause |
|---|---|
| arm-none-eabi-gcc not found | toolchain missing from PATH; use PREFIX=… or export DEVKITARM |
| baserom.nds not found | copy the ROM to the root directory (section 3) |
| ROM differs from reference | different ROM version/region; addresses won't match |
| .module_params / .crt0 overlap | src/arm9_crt0.s exceeded 0x7B0 bytes (ARM9) or 0x5D4 (ARM7) |
| ARM9: static .bss invades overlay 0 | .bss/.data grew past 0x022503A0 |
| ARM9: DTCM out of stack space | .dtcm* sections too large (16 KB minus 2 KB stack and 1 KB system) |
| make compare fails | expected while differences remain; check % per segment |
| make rom: "only 0x… bytes left" | binary larger than available space (section 5) |
12. Legal Disclaimer
The Sims 3 and game code belong to their respective owners. This project does not distribute the ROM or
any derived code from it. asm/, build/, and *.nds are listed in .gitignore. Only use a legally
owned copy.
