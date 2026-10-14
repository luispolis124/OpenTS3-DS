#!/usr/bin/env python3
"""
tools/rebuild_rom.py - empacota os binários compilados em uma ROM e mede o quanto batem.

    # 1) comparar o que foi compilado com o original extraído (progresso do matching)
    python tools/rebuild_rom.py compare --build-dir build --asm-dir asm

    # 2) gerar uma ROM: copia a ROM base e troca o ARM9 e/ou o ARM7 no lugar
    python tools/rebuild_rom.py pack --rom baserom.nds \\
        --arm9 build/arm9.bin --arm7 build/arm7.bin -o build/sims3.nds

    # 3) autoteste: reempacotar os binários ORIGINAIS tem que reproduzir a ROM bit a bit
    python tools/rebuild_rom.py selftest --rom baserom.nds --asm-dir asm

COMO O PACK FUNCIONA
  Não reconstrói o NitroFS do zero (isso é trabalho para o ndstool). Ele PATCHA a ROM base:
  escreve o novo arm9.bin/arm7.bin no mesmo offset, atualiza o tamanho no cabeçalho e
  recalcula o CRC16 do cabeçalho. O binário novo precisa caber no espaço até a próxima
  estrutura da ROM (tabela de overlays, FNT, etc.); a ferramenta calcula esse limite.
  Nesta ROM a folga é pequena: ARM9 +0x140 bytes (a tabela de overlays vem logo depois)
  e ARM7 +0x164. Ou seja, o binário compilado precisa ter praticamente o MESMO tamanho
  do original: serve para matching, não para builds que crescem. Para binários maiores
  é preciso reconstruir o NitroFS inteiro (ndstool) e mover as demais estruturas.

LIMITES
  - Overlays não são empacotados (ainda).
  - O CRC16 da Secure Area (0x06C) e os hashes/assinatura do cabeçalho DSi NÃO são
    recalculados. A ROM gerada serve para emulador/flashcart em modo DS; no modo DSi a
    verificação do cabeçalho pode falhar.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path


def die(msg: str) -> None:
    sys.exit(f"[erro] {msg}")


def crc16(data: bytes, crc: int = 0xFFFF) -> int:
    """CRC-16/MODBUS (poly 0xA001 refletido, init 0xFFFF): o do cabeçalho NDS."""
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc


def u32(b: bytes, off: int) -> int:
    return struct.unpack_from("<I", b, off)[0]


# --------------------------------------------------------------------------- pack
def known_offsets(rom: bytes) -> list[int]:
    """Offsets de todas as estruturas conhecidas da ROM (limites para o espaço livre)."""
    offs = {0x4000, u32(rom, 0x20), u32(rom, 0x30), u32(rom, 0x40), u32(rom, 0x48), u32(rom, 0x68)}
    for hdr_off in (0x50, 0x58):
        if u32(rom, hdr_off + 4):
            offs.add(u32(rom, hdr_off))
    fat_off, fat_size = u32(rom, 0x48), u32(rom, 0x4C)
    for i in range(fat_size // 8):
        offs.add(u32(rom, fat_off + i * 8))
    if rom[0x12] in (2, 3):  # binários DSi
        for o in (0x1C0, 0x1D0):
            if u32(rom, o):
                offs.add(u32(rom, o))
    return sorted(o for o in offs if o)


def patch_binary(rom: bytearray, cpu: str, blob: bytes) -> None:
    base = {"arm9": 0x20, "arm7": 0x30}[cpu]
    start, old_size = u32(rom, base), u32(rom, base + 12)
    nxt = min((o for o in known_offsets(bytes(rom)) if o > start), default=len(rom))
    room = nxt - start
    if len(blob) > room:
        die(f"{cpu}: binário novo tem {len(blob):#x} bytes, mas só cabem {room:#x} "
            f"(até a próxima estrutura em {nxt:#x})")
    if len(blob) % 4:
        die(f"{cpu}: tamanho {len(blob):#x} não é múltiplo de 4")
    if len(blob) != old_size:
        print(f"[aviso] {cpu}: novo={len(blob):#x} bytes, original={old_size:#x}. Enquanto a decompilação "
              f"não cobrir o binário inteiro, a ROM gerada NÃO deve funcionar.", file=sys.stderr)
    rom[start:start + len(blob)] = blob
    if len(blob) < old_size:  # zera o resto da região antiga
        rom[start + len(blob):start + old_size] = b"\0" * (old_size - len(blob))
    struct.pack_into("<I", rom, base + 12, len(blob))


def pack(rom_bytes: bytes, arm9: bytes | None, arm7: bytes | None) -> bytes:
    rom = bytearray(rom_bytes)
    if arm9 is not None:
        patch_binary(rom, "arm9", arm9)
    if arm7 is not None:
        patch_binary(rom, "arm7", arm7)
    struct.pack_into("<H", rom, 0x15E, crc16(bytes(rom[:0x15E])))
    return bytes(rom)


def cmd_pack(a: argparse.Namespace) -> int:
    rom = a.rom.read_bytes()
    arm9 = a.arm9.read_bytes() if a.arm9 else None
    arm7 = a.arm7.read_bytes() if a.arm7 else None
    if arm9 is None and arm7 is None:
        die("informe --arm9 e/ou --arm7")
    out = pack(rom, arm9, arm7)
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_bytes(out)
    print(f"ROM escrita em {a.output}  sha1={hashlib.sha1(out).hexdigest()}")
    if rom[0x12] in (2, 3):
        print("[aviso] ROM NDSi Enhanced: hashes/assinatura DSi não recalculados (use em modo DS).")
    return 0


def cmd_selftest(a: argparse.Namespace) -> int:
    rom = a.rom.read_bytes()
    a9 = (a.asm_dir / "bin" / "arm9.bin").read_bytes()
    a7 = (a.asm_dir / "bin" / "arm7.bin").read_bytes()
    out = pack(rom, a9, a7)
    ok = out == rom
    print(f"crc16 do cabeçalho: calculado={crc16(rom[:0x15E]):#06x}  gravado={struct.unpack_from('<H', rom, 0x15E)[0]:#06x}")
    print("selftest:", "OK (ROM reproduzida bit a bit)" if ok else "FALHOU (ROM diferente da original)")
    if not ok:
        first = next(i for i, (x, y) in enumerate(zip(out, rom)) if x != y)
        print(f"  primeira diferença no offset {first:#x}")
    return 0 if ok else 1


# ------------------------------------------------------------------------ compare
def load_segments(layout_path: Path, cpu: str) -> list[dict]:
    if layout_path.exists():
        return json.loads(layout_path.read_text(encoding="utf-8"))[cpu]["segments"]
    return []


def compare_one(cpu: str, built: bytes, ref: bytes, segs: list[dict]) -> bool:
    n = min(len(built), len(ref))
    same = built[:n] == ref[:n]
    print(f"{cpu}: compilado={len(built):#x} bytes  original={len(ref):#x} bytes", end="  ")
    if same and len(built) == len(ref):
        print(f"-> IDÊNTICO  sha1={hashlib.sha1(built).hexdigest()}")
        return True
    first = next((i for i in range(n) if built[i] != ref[i]), n)
    print(f"-> DIFERENTE (primeira diferença em {first:#x})")
    for s in segs or [{"name": "(tudo)", "file_offset": 0, "size": len(ref), "ram": 0}]:
        lo, hi = s["file_offset"], min(s["file_offset"] + s["size"], n)
        if hi <= lo:
            print(f"    {s['name']:<14} sem dados compilados")
            continue
        eq = sum(x == y for x, y in zip(built[lo:hi], ref[lo:hi]))
        print(f"    {s['name']:<14} {100.0 * eq / s['size']:6.2f}% igual  ({eq:#x}/{s['size']:#x} bytes)")
    return False


def cmd_compare(a: argparse.Namespace) -> int:
    ok, found = True, False
    for cpu in ("arm9", "arm7"):
        built_p, ref_p = a.build_dir / f"{cpu}.bin", a.asm_dir / "bin" / f"{cpu}.bin"
        if not built_p.exists():
            print(f"{cpu}: {built_p} não existe (rode 'make')")
            continue
        if not ref_p.exists():
            die(f"{ref_p} não existe (rode 'make extract')")
        found = True
        ok &= compare_one(cpu, built_p.read_bytes(), ref_p.read_bytes(),
                          load_segments(a.asm_dir / "layout.json", cpu))
    return 0 if (ok and found) else 1


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("pack")
    p.add_argument("--rom", type=Path, default=Path("baserom.nds"))
    p.add_argument("--arm9", type=Path)
    p.add_argument("--arm7", type=Path)
    p.add_argument("-o", "--output", type=Path, default=Path("build/sims3.nds"))
    p = sub.add_parser("compare")
    p.add_argument("--build-dir", type=Path, default=Path("build"))
    p.add_argument("--asm-dir", type=Path, default=Path("asm"))
    p = sub.add_parser("selftest")
    p.add_argument("--rom", type=Path, default=Path("baserom.nds"))
    p.add_argument("--asm-dir", type=Path, default=Path("asm"))
    a = ap.parse_args()
    sys.exit({"pack": cmd_pack, "compare": cmd_compare, "selftest": cmd_selftest}[a.cmd](a))


if __name__ == "__main__":
    main()
