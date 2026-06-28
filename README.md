# OpenTS3-DS

An open-source reverse engineering and native PC port project for **The Sims 3 (Nintendo DS)**.

## About the Project
The goal of **OpenTS3-DS** is to decompile the original Nintendo DS game code and reconstruct it as a modern, portable game engine. This project aims to preserve the original game logic while allowing it to run natively on PC, with support for higher resolutions, modern input methods, and improved performance.

## Project Status
- [x] Mapping of data structures (`entity.h`)
- [x] Engine architecture and build pipeline (`CMake`)
- [ ] Implementation of the Entity System
- [ ] Implementation of the Renderer
- [ ] PC Portability (Windows/Linux)

## Repository Structure
* `src/`: Engine source code.
* `include/`: Header files with memory definitions and structures.
* `asm/`: Assembly references from the original code for comparison.
* `build/`: Compilation output directory.

## How to Build
To build this project, you will need **CMake** and a C compiler (GCC, Clang, or MSVC).

1. Clone the repository:
   `git clone https://github.com/YOUR_USERNAME/OpenTS3-DS.git`
2. Create the build directory:
   `mkdir build && cd build`
3. Configure and build:
   `cmake ..`
   `cmake --build .`

## License
This project is an educational reverse engineering effort. Please ensure you own a legitimate copy of the original game.
