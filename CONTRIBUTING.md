# Contributing to OpenTS3DS

Thank you for your interest in contributing to OpenTS3DS! To keep this project legal, sustainable, and respectful of intellectual property, please follow these guidelines:

## How to Contribute
1. **Clean Room Engineering:** All contributed code must be your own original work. 
   - Do not copy/paste code directly from the original game binaries or decompilation outputs that include proprietary code.
   - If you are reversing an original game function, document how you arrived at your conclusion (e.g., "Observed memory pattern in debugger") so we can verify it's an independent implementation.
2. **No Assets:** Do not submit any original game assets (textures, sounds, 3D models, or ROM files) in your Pull Requests. This project is strictly for the engine code.
3. **Coding Standards:**
   - Keep the project structure: `include/` for headers, `src/` for logic.
   - Ensure your code is compatible with the existing `CMake` build pipeline.
   - Use meaningful variable names and document complex memory-mapped structures.
4. **License:** By submitting a Pull Request, you agree that your code will be licensed under the same **GPLv3** license as the project.

## Workflow
- Create a new branch for your feature or fix.
- Ensure your changes compile on both Windows and Linux (if possible).
- Open a Pull Request with a clear description of what the code does and how it was derived.

## Legal Disclaimer
By contributing, you acknowledge that OpenTS3DS is an educational project and you agree not to introduce any code that violates copyright laws or third-party intellectual property rights.

Let's preserve *The Sims 3 DS* history together!
