# The Sims 3 (NDS) - Localization File Format Specification
This document details the binary structure of the localization and text database files used in *The Sims 3* for the Nintendo DS (`DATA/LOCALIZATION/`).
---
## 1. Overview
The game handles multi-language support through container files with the `.arc` extension. Inside these containers reside `.dat` files for each supported language (e.g., `English.dat`, `Spanish.dat`, `French.dat`) along with an index file named `textdb.idx`.
### File Location
`ROM_ROOT/DATA/LOCALIZATION/`
- `text_db_dynamic.arc`
- `text_db_static.arc`
- `text_db_wifi.arc`
- `xtrctextdb.arc`
---
## 2. Container Structure (`.arc`)
The `.arc` files act as simple archives that store localized `.dat` streams and index blocks.
### Compression
Entries within the `.arc` container can either be **zlib-compressed** or **uncompressed (raw)**:
- **Uncompressed Entry:** Indicated by a compressed size value of `0xFFFFFFFF`.
- **Compressed Entry:** Standard `zlib` stream header (`0x78 0x9C` or similar).
---
## 3. String Table Format (`.dat`)
Once uncompressed, each language `.dat` file follows a structured offset/length table followed by null-terminated string data.
### Header & Lookup Table
The stream begins with an array of $N$ string entries, where $N$ is defined by the first `uint32_t` in `textdb.idx`.

| Offset | Type | Description |
| :--- | :--- | :--- |
| `0x00` | `uint32_t LE` | Entry 0 String Offset (Relative to Data Section) |
| `0x04` | `uint32_t LE` | Entry 0 String Length (in bytes) |
| `0x08` | `uint32_t LE` | Entry 1 String Offset |
| `0x0C` | `uint32_t LE` | Entry 1 String Length |
| `...` | `...` | Pair array for $N$ total entries |

*Note: All multi-byte integers are stored in **Little-Endian (LE)** format.*
### String Data Section
The data section starts immediately after the index pairs at byte offset $8 \times N$.
- **Encoding:** UTF-8
- **Termination:** Null byte (`0x00` / `NUL`)
- **Control Characters:** 
  - Line breaks are represented as standard `\n` (`0x0A`).
  - Formatting tags (variables, parameters) are embedded directly within the string stream.
---
## 4. Alternative Format: `STRG` Header
Some standalone or decompressed text assets across EA Nintendo DS titles utilize a simplified `STRG` header format:

| Offset | Type | Value / Description |
| :--- | :--- | :--- |
| `0x00` | `char[4]` | Magic Header Signature (`"STRG"`) |
| `0x04` | `uint32_t LE` | Total string count ($M$) |
| `0x08` | `bytes` | Null-terminated UTF-8 / Latin-1 strings |

---
## 5. Re-packing Guidelines
When editing localized text for mods or translations:
1. Ensure strings remain null-terminated.
2. Recalculate each string's byte length.
3. Update all offset pointers relative to $8 \times N$.
4. Re-calculate zlib block boundaries if repacking back into `.arc` containers.
