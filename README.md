# OpenTS3DS, also known as Open The Sims 3 DS
An open-source reverse engineering and native PC port project for **The Sims 3 (Nintendo DS)**.
> **Disclaimer:** OpenTS3DS is an independent, community-driven project and is **not affiliated with, endorsed by, or connected to Electronic Arts (EA), Nintendo, or any of their subsidiaries.** "The Sims" is a registered trademark of Electronic Arts Inc. All other trademarks are the property of their respective owners.
> 
## About the Project
The goal of **Open The Sims 3 DS** is to decompile the original Nintendo DS game code and reconstruct it as a modern, portable game engine. This project aims to preserve the original game logic while allowing it to run natively on PC, with support for higher resolutions, modern input methods, and improved performance.
## ⚠️ Important Notice: ROM & Region Requirement
**This project does not include the original game ROM.** To use OpenTS3DS, you must provide your own legally obtained copy of *The Sims 3 (Nintendo DS)*.
**Note:** Currently, this project only supports the **North American (USA)** version of the game. Using ROMs from other regions will cause data mismatches and the engine will not function correctly.
## Project Status
 * [x] Mapping of data structures (entity.h)
 * [x] Engine architecture and build pipeline (CMake)
 * [ ] Implementation of the Entity System
 * [ ] Implementation of the Renderer
 * [ ] PC Portability (Windows/Linux)
## Repository Structure
 * src/: Engine source code.
 * include/: Header files with memory definitions and structures.
 * asm/: Assembly references from the original code for comparison.
 * build/: Compilation output directory.
## How to Build
To build this project, you will need **CMake** and a C compiler (GCC, Clang, or MSVC).
 1. Clone the repository:
   git clone https://github.com/luispolis124/OpenTS3-DS.git
 2. Create the build directory:
   mkdir build && cd build
 3. Configure and build:
   cmake ..
   cmake --build .
## Contributing
Contributions are welcome! If you are interested in reverse engineering, game engine development, or porting, feel free to submit a Pull Request. Please follow the existing code style.
## Documentation & Legal
 * **LICENSE:** The project's license terms (GPLv3).
 * **CONTRIBUTING:** Guidelines for code contributions.
## Technologies & Credits
This project utilizes the following open-source libraries:
 * **SDL2:** Window management and input handling (zlib License).
 * **SDL_mixer:** Multi-channel audio mixing (zlib License).
 * **OpenGL:** Hardware-accelerated 3D rendering.
## Acknowledgements
 * Inspired by the community effort of projects like OpenRCT2 and OpenMW.
 * Special thanks to everyone involved in the preservation of Nintendo DS software.
## License
This project is an educational reverse engineering effort. Please ensure you own a legitimate copy of the original game.
