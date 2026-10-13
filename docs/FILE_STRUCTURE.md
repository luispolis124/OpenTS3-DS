# The Sims 3 (NDS) - NitroFS File System Structure

This document outlines the file layout and directory organization of *The Sims 3* Nintendo DS ROM image (`NitroFS`).

---

## 1. Directory Tree Overview

```text
NitroFS (ROM Root)
├── DATA/
│   ├── LOCALIZATION/      # Text databases and string archives (.arc)
│   ├── AUDIO/             # Sound effects, voice samples, and music tracks
│   ├── GRAPHICS/          # Textures, sprites, UI elements, and 3D models
│   ├── OBJECTS/           # Furniture, item definitions, and interactions
│   └── SCRIPT/            # Game logic scripts and event triggers
├── ftc/                   # Nintendo file table cache
└── main.srl / arm9.bin    # Executable binary code (ARM9 processor)

2. Key Directories & Descriptions
/DATA/LOCALIZATION/
Contains all game dialogue, UI labels, object descriptions, and multi-language assets.
 * text_db_dynamic.arc: Strings for dynamic events, notifications, and active Sim interactions.
 * text_db_static.arc: System menus, options, fixed UI elements, and base item names.
 * text_db_wifi.arc: Online / Nintendo Wi-Fi Connection UI strings and network error messages.
 * xtrctextdb.arc: Additional auxiliary text entries.
/DATA/GRAPHICS/
Stores visual assets formatted for the Nintendo DS 2D/3D graphics engine.
 * Contains texture banks, tilemaps, palette data (.NCLR, .NCGR, .NSCR), and 3D geometry (.NSBMD).
/DATA/AUDIO/
Contains sound effects, ambient tracks, and Simlish voice files.
 * Uses standard Nintendo DS sound archive structures (.SDAT) or custom EA audio wrappers.
Executable Binaries (ARM9 / ARM7)
 * arm9.bin: Main game logic, state machines, physics engine, and core game loops.
 * arm7.bin: Sound processing, hardware input handling, and wireless communication driver routines.
3. Notes for Developers
 * Extracted Files: Do not commit extracted ROM directory trees to version control.
 * Asset Modification: Modifying file sizes within .arc archives without updating header offsets or pointers will lead to memory alignment crashes or soft-locks on hardware/emulators.

