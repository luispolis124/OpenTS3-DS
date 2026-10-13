# OpenTS3DS, also known as Open The Sims 3 DS

An open-source reverse engineering and native PC port project for **The Sims 3 (Nintendo DS)**.

> ⚠️ **Disclaimer:** OpenTS3DS is an independent, community-driven project and is **not affiliated with, endorsed by, or connected to Electronic Arts (EA), Nintendo, or any of their subsidiaries.** "The Sims" is a registered trademark of Electronic Arts Inc. All other trademarks are the property of their respective owners.

---

## About the Project

The goal of **Open The Sims 3 DS** is to decompile the original Nintendo DS game code (`arm9` and `arm7` architectures) and reconstruct it as a modern, portable game engine. This project aims to preserve the original game logic while allowing it to run natively on PC, with support for higher resolutions, modern input methods, and improved performance.

---

## ⚠️ Important Notice: ROM & Region Requirement

**This project does not include the original game ROM.** To use OpenTS3DS, you must provide your own legally obtained copy of *The Sims 3 (Nintendo DS)*.

**Note:** Currently, this project primarily targets the **North American (USA)** version of the game. Using ROMs from other regions may cause data mismatches, and the engine might not function correctly.

---

## Project Status

- [ ] ARM9 and ARM7 binary extraction and baseline integration (`arm9.c`, `arm7.c`)
- [ ] Mapping of data structures (`entity.h`, memory definitions)
- [ ] Engine architecture and build pipeline (CMake & Linker scripts)
- [ ] Implementation of the Entity System
- [ ] Implementation of the Renderer (OpenGL & SDL2)
- [ ] PC Portability (Windows/Linux)

---

## Repository Structure

```text
OpenTS3DS/
├── asm/         # Assembly references from original code for verification
├── docs/        # Technical specifications (File structures, formats)
├── include/     # Header files, memory definitions, and system maps
├── src/         # Engine source code and decompiled modules
├── tools/       # Utility scripts (Localization extractors, asset tools)
├── .gitignore
├── CMakeLists.txt
├── LICENSE      # GNU General Public License v3.0 (GPLv3)
└── README.md

How to Build
To build this project, you will need CMake (v3.15+) and a C/C++ compiler (GCC, Clang, or MSVC).
 * Clone the repository:
   git clone [https://github.com/luispolis124/OpenTS3-DS.git](https://github.com/luispolis124/OpenTS3-DS.git)
cd OpenTS3-DS

 * Create the build directory:
   mkdir build && cd build

 * Configure and build:
   cmake ..
cmake --build .

Tools & Asset Extraction
The repository includes Python tools in the tools/ directory to analyze and extract game assets.
Extracting Text & Localizations
To extract game dialogue and string tables directly from a legally acquired ROM:
# Extract default strings (English)
python tools/extract_sims3_strings.py ROM.nds -o extracted_strings.txt

# Extract a specific language
python tools/extract_sims3_strings.py ROM.nds --lang Spanish -o strings_espanhol.txt

# Extract all languages
python tools/extract_sims3_strings.py ROM.nds --all-langs

For technical details regarding the localization format, see docs/LOCALIZATION_FORMAT.md.
Contributing
Contributions are welcome! If you are interested in reverse engineering, game engine development, or porting, feel free to submit a Pull Request. Please follow the existing code style.
Technologies & Credits
This project utilizes the following open-source libraries:
 * SDL2: Window management and input handling (zlib License).
 * SDL_mixer: Multi-channel audio mixing (zlib License).
 * OpenGL: Hardware-accelerated 3D rendering.
AI-Assisted Development
The code structures, documentation, and conversion assistance for this project were generated or refined with the support of Artificial Intelligence tools. All AI-generated outputs have been verified and integrated by the project maintainer to ensure accuracy and project standards.
Acknowledgements
 * Inspired by the community efforts of projects like OpenRCT2 and OpenMW.
 * Special thanks to everyone involved in the preservation of Nintendo DS software.
License
This project is an educational reverse engineering effort. Distributed under the GNU General Public License v3.0 (GPLv3). Please ensure you own a legitimate copy of the original game before using asset extraction utilities. See the LICENSE file for more details.
