#!/usr/bin/env python3
"""
tools/extract_code.py - Extrai os executáveis de uma ROM NDS e gera o layout de memória.

Uso (a partir da raiz do repositório):
    python tools/extract_code.py baserom.nds                 # tudo em asm/
    python tools/extract_code.py baserom.nds -o asm --backend raw
    python tools/extract_code.py baserom.nds --thumb-ranges tools/thumb_ranges.txt
    python tools/extract_code.py baserom.nds --no-disasm     # só extrai + layout

Saída (pasta --out, padrão "asm/"):
    bin/arm9.bin, bin/arm7.bin          binários exatamente como estão na ROM
    bin/arm9_<seg>.bin ...              um arquivo por segmento (secure, main, itcm, dtcm...)
    bin/y9.bin, bin/y7.bin              tabelas de overlay
    bin/ov9_NNN.bin, bin/ov7_NNN.bin    overlays
    bin/arm9i.bin, bin/arm7i.bin        binários DSi (NDSi Enhanced), se existirem
    bin/header.bin, bin/banner.bin
    layout.json                         layout completo (para ferramentas)
    memory_layout.txt                   o mesmo, legível
    symbols/arm9.sym, arm7.sym          "nome 0xENDEREÇO" (formato do ImportSymbolsScript do Ghidra)
    arm9.r2, arm7.r2                    scripts de flags para o radare2
    nitrofs_files.txt                   lista dos arquivos do NitroFS
    *.s                                 disassembly por segmento (ver --backend)

Backends de disassembly (--backend auto escolhe o primeiro disponível):
    capstone   pip install capstone
    objdump    arm-none-eabi-objdump (binutils para ARM)
    raw        sempre disponível: despeja .word/.hword com o endereço em comentário

ARM e Thumb se misturam no mesmo binário e não dá para saber só pelos bytes.
Por padrão tudo é ARM; informe trechos Thumb com --thumb-ranges (linhas "0xINICIO 0xFIM",
endereços de RAM, '#' comenta). Conforme você identificar funções Thumb no Ghidra, vá
registrando os intervalos nesse arquivo e rode a ferramenta de novo.

Os arquivos extraídos vêm do código da ROM (protegido por copyright): adicione asm/ ao
.gitignore, ou deixe a ferramenta rodar no build a partir da ROM de cada usuário.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import struct
import subprocess
import sys
from pathlib import Path

# ROM de referência (USA, NDSi Enhanced). Se o hash não bater a ferramenta só avisa.
KNOWN_ROM = {
    "gamecode": "VS3E",
    "md5": "7cdf32d02eeeff592a0a4ab6bec7b5c1",
    "sha1": "623dea494449e3f60ace691dfbf4f7fd285950f6",
}

MOD_MAGIC = struct.pack("<II", 0xDEC00621, 0x2106C0DE)  # fim do ModuleParams (SDK Nitro/TWL)
SECURE_AREA_SIZE = 0x800
MODULE_FIELDS = ("autoload_list_start", "autoload_list_end", "autoload_start",
                 "static_bss_start", "static_bss_end", "compressed_end", "sdk_version")
OBJ_RE = re.compile(r"^\s*([0-9a-f]+):\t([0-9a-f ]+?)\s*\t(.*)$")


def warn(msg: str) -> None:
    print(f"[aviso] {msg}", file=sys.stderr)


def die(msg: str) -> None:
    sys.exit(f"[erro] {msg}")


def u32(b: bytes, off: int) -> int:
    return struct.unpack_from("<I", b, off)[0]


# ----------------------------------------------------------------------------- ROM
def parse_header(rom: bytes) -> dict:
    if len(rom) < 0x4000:
        die("arquivo pequeno demais para ser uma ROM NDS")
    h: dict = {
        "title": rom[0:12].split(b"\0")[0].decode("ascii", "replace"),
        "gamecode": rom[0xC:0x10].decode("ascii", "replace"),
        "unit_code": rom[0x12],
    }
    for cpu, base in (("arm9", 0x20), ("arm7", 0x30)):
        off, entry, load, size = struct.unpack_from("<IIII", rom, base)
        h[cpu] = {"rom_offset": off, "entry": entry, "load": load, "size": size}
    h["fnt"] = struct.unpack_from("<II", rom, 0x40)
    h["fat"] = struct.unpack_from("<II", rom, 0x48)
    h["y9"] = struct.unpack_from("<II", rom, 0x50)
    h["y7"] = struct.unpack_from("<II", rom, 0x58)
    h["banner_offset"] = u32(rom, 0x68)
    # Extensão DSi (só em unit_code 2 = NDSi Enhanced ou 3 = DSi exclusivo)
    h["dsi"] = None
    if h["unit_code"] in (2, 3) and len(rom) > 0x240:
        a9i = struct.unpack_from("<IxxxxII", rom, 0x1C0)  # offset, (reservado), ram, size
        a7i = struct.unpack_from("<IxxxxII", rom, 0x1D0)
        mc1 = struct.unpack_from("<II", rom, 0x220)       # modcrypt1: início, tamanho
        mc2 = struct.unpack_from("<II", rom, 0x228)
        h["dsi"] = {
            "arm9i": {"rom_offset": a9i[0], "load": a9i[1], "size": a9i[2]},
            "arm7i": {"rom_offset": a7i[0], "load": a7i[1], "size": a7i[2]},
            "modcrypt1": mc1, "modcrypt2": mc2,
        }
    return h


def read_fat(rom: bytes, h: dict) -> list[tuple[int, int]]:
    off, size = h["fat"]
    return [struct.unpack_from("<II", rom, off + i * 8) for i in range(size // 8)]


def nitro_paths(rom: bytes, h: dict) -> dict[int, str]:
    fnt_off = h["fnt"][0]
    paths: dict[int, str] = {}

    def walk(dir_id: int, prefix: str) -> None:
        sub, first_id, _ = struct.unpack_from("<IHH", rom, fnt_off + (dir_id & 0xFFF) * 8)
        pos, fid = fnt_off + sub, first_id
        while True:
            t = rom[pos]
            pos += 1
            if t == 0:
                return
            name = rom[pos:pos + (t & 0x7F)].decode("latin1")
            pos += t & 0x7F
            if t & 0x80:
                walk(struct.unpack_from("<H", rom, pos)[0], f"{prefix}{name}/")
                pos += 2
            else:
                paths[fid] = f"{prefix}{name}"
                fid += 1

    walk(0xF000, "")
    return paths


def parse_overlay_table(rom: bytes, off: int, size: int) -> list[dict]:
    out = []
    for i in range(size // 0x20):
        (oid, ram, rsize, bss, sinit_a, sinit_b, fid, word) = struct.unpack_from("<8I", rom, off + i * 0x20)
        out.append({
            "id": oid, "ram": ram, "size": rsize, "bss": bss,
            "static_init_start": sinit_a, "static_init_end": sinit_b, "file_id": fid,
            "compressed": bool(word & 0x01000000), "authenticated": bool(word & 0x02000000),
            "compressed_size": word & 0x00FFFFFF,
        })
    return out


# ---------------------------------------------------------------- layout do módulo
def region_name(cpu: str, addr: int, idx: int) -> str:
    if cpu == "arm9":
        if 0x01FF8000 <= addr < 0x02000000:
            return "itcm"
        if addr in (0x027E0000, 0x02FE0000):  # DTCM: base do NitroSDK / base do TWL SDK
            return "dtcm"
    if cpu == "arm7" and 0x037F8000 <= addr < 0x03810000:
        return "wram"
    return f"autoload{idx}"


def parse_module(blob: bytes, load: int) -> dict | None:
    """Acha o ModuleParams (termina em 0xDEC00621 0x2106C0DE) e decodifica a lista de autoload."""
    i = blob.find(MOD_MAGIC)
    if i < 4 * len(MODULE_FIELDS):
        return None
    p = i - 4 * len(MODULE_FIELDS)
    m = dict(zip(MODULE_FIELDS, struct.unpack_from(f"<{len(MODULE_FIELDS)}I", blob, p)))
    m["file_offset"], m["ram"] = p, load + p
    lo, hi = m["autoload_list_start"] - load, m["autoload_list_end"] - load
    if not (0 <= lo <= hi <= len(blob)):
        return None
    expected = m["autoload_list_start"] - m["autoload_start"]  # bytes de dados dos autoloads
    m["autoloads"], m["autoload_entry_size"] = [], 0
    # SDKs diferentes usam entradas de 12 bytes (addr,size,bss) ou 16 (addr,size,?,bss).
    # Escolhe a que fecha com o espaço entre autoload_start e a lista.
    for esz in (16, 12):
        if (hi - lo) % esz:
            continue
        ents = []
        for k in range((hi - lo) // esz):
            f = struct.unpack_from(f"<{esz // 4}I", blob, lo + k * esz)
            ents.append({"address": f[0], "size": f[1], "aux": f[2] if esz == 16 else None, "bss": f[-1]})
        if sum(e["size"] for e in ents) == expected:
            m["autoloads"], m["autoload_entry_size"] = ents, esz
            break
    return m


def seg(cpu: str, name: str, ram: int, size: int, bss: int, file_off: int, kind: str) -> dict:
    return {"name": name, "ram": ram, "size": size, "bss": bss, "file_offset": file_off,
            "kind": kind, "bin": f"bin/{cpu}_{name}.bin"}


def build_cpu_layout(cpu: str, blob: bytes, cinfo: dict) -> dict:
    load, entry = cinfo["load"], cinfo["entry"]
    lay: dict = {**cinfo, "module": None, "segments": [], "notes": []}
    m = parse_module(blob, load)
    lay["module"] = m
    if m is None:
        lay["notes"].append("ModuleParams não encontrado: layout reduzido a um segmento único")
        lay["segments"].append(seg(cpu, "main", load, len(blob), 0, 0, "code"))
        return lay
    if m["compressed_end"]:
        die(f"{cpu.upper()} está comprimido (compressed_end={m['compressed_end']:#x}); "
            "descomprima com BLZ (ndstool/blz) antes de usar esta ferramenta")
    if not m["autoloads"]:
        lay["notes"].append("lista de autoload não decodificada; segmentos de autoload omitidos")

    main_size = m["autoload_start"] - load
    bss = m["static_bss_end"] - m["static_bss_start"]
    has_secure = cpu == "arm9" and cinfo["rom_offset"] == 0x4000 and main_size > SECURE_AREA_SIZE
    if has_secure:
        head = blob[:8]
        state = ("descriptografada" if head == b"\xff\xde\xff\xe7\xff\xde\xff\xe7"
                 else "criptografada" if head == b"encryObj" else "desconhecida")
        lay["secure_area"] = state
        lay["segments"].append(seg(cpu, "secure", load, SECURE_AREA_SIZE, 0, 0, "data"))
        lay["segments"].append(seg(cpu, "main", load + SECURE_AREA_SIZE, main_size - SECURE_AREA_SIZE,
                                   bss, SECURE_AREA_SIZE, "code"))
    else:
        lay["segments"].append(seg(cpu, "main", load, main_size, bss, 0, "code"))
    off = main_size
    for k, e in enumerate(m["autoloads"]):
        name = region_name(cpu, e["address"], k)
        lay["segments"].append(seg(cpu, name, e["address"], e["size"], e["bss"], off,
                                   "data" if name == "dtcm" else "code"))
        off += e["size"]
    lo = m["autoload_list_start"] - load
    lay["segments"].append(seg(cpu, "autoload_list", m["autoload_list_start"],
                               m["autoload_list_end"] - m["autoload_list_start"], 0, lo, "data"))
    return lay


# ---------------------------------------------------------------------- disassembly
def parse_ranges(path: Path | None) -> list[tuple[int, int]]:
    if not path:
        return []
    out = []
    for n, line in enumerate(path.read_text().splitlines(), 1):
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        try:
            a, b = (int(x, 0) for x in line.split()[:2])
        except ValueError:
            die(f"{path}:{n}: linha inválida (esperado '0xINICIO 0xFIM')")
        out.append((a, b))
    return sorted(out)


def split_modes(base: int, size: int, thumb: list[tuple[int, int]]) -> list[tuple[int, int, str]]:
    end, pos, pieces = base + size, base, []
    for a, b in thumb:
        a, b = max(a, base), min(b, end)
        if a >= b:
            continue
        if pos < a:
            pieces.append((pos, a, "arm"))
        nxt = min((b + 3) & ~3, end)  # o trecho ARM seguinte precisa estar alinhado em 4
        pieces.append((a, nxt, "thumb"))
        pos = nxt
    if pos < end:
        pieces.append((pos, end, "arm"))
    return pieces


class Disassembler:
    def __init__(self, backend: str, arch: str, objdump: str | None):
        self.backend, self.arch, self.objdump = backend, arch, objdump
        self._cs: dict[str, object] = {}

    # -- raw: sempre funciona
    def _raw(self, data: bytes, base: int, lo: int, hi: int, mode: str):
        step, fmt = (2, ".hword") if mode == "thumb" else (4, ".word")
        for a in range(lo, hi, step):
            chunk = data[a - base:a - base + step].ljust(step, b"\0")
            v = int.from_bytes(chunk, "little")
            yield f"\t{fmt} 0x{v:0{step * 2}x}\t@ {a:08x}"

    # -- capstone
    def _capstone(self, data: bytes, base: int, lo: int, hi: int, mode: str):
        import capstone as cs
        md = self._cs.get(mode)
        if md is None:
            md = cs.Cs(cs.CS_ARCH_ARM, cs.CS_MODE_THUMB if mode == "thumb" else cs.CS_MODE_ARM)
            self._cs[mode] = md
        step = 2 if mode == "thumb" else 4
        chunk_sz, pos = 0x10000, lo
        while pos < hi:
            end = min(pos + chunk_sz, hi)
            n = 0
            for addr, size, mnem, ops in md.disasm_lite(data[pos - base:end - base], pos):
                raw = data[addr - base:addr - base + size].hex()
                yield f"\t{mnem} {ops}".rstrip() + f"\t@ {addr:08x} {raw}"
                n += size
            pos += n
            if pos < end and (end - pos >= step or end == hi):  # bytes que o capstone não decodifica
                yield from self._raw(data, base, pos, min(pos + step, hi), mode)
                pos += step

    # -- objdump
    def _objdump(self, path: Path, base: int, lo: int, hi: int, mode: str):
        cmd = [self.objdump, "-D", "-b", "binary", "-m", self.arch, f"--adjust-vma={base:#x}",
               f"--start-address={lo:#x}", f"--stop-address={hi:#x}"]
        if mode == "thumb":
            cmd += ["-M", "force-thumb"]
        res = subprocess.run(cmd + [str(path)], capture_output=True, text=True)
        if res.returncode:
            raise RuntimeError(res.stderr.strip() or "objdump falhou")
        for line in res.stdout.splitlines():
            m = OBJ_RE.match(line)
            if m:
                text = re.sub(r"\s+", " ", m.group(3)).strip()
                yield f"\t{text}\t@ {int(m.group(1), 16):08x} {m.group(2).replace(' ', '')}"

    def run(self, path: Path, data: bytes, base: int, kind: str, thumb):
        backend = "raw" if kind == "data" else self.backend
        lines = [f"@ {path.stem}: RAM 0x{base:08x}..0x{base + len(data):08x}  backend={backend}",
                 "\t.text"]
        for lo, hi, mode in split_modes(base, len(data), thumb):
            lines.append(f"\t.{'thumb' if mode == 'thumb' else 'arm'}")
            try:
                if backend == "capstone":
                    lines.extend(self._capstone(data, base, lo, hi, mode))
                elif backend == "objdump":
                    lines.extend(self._objdump(path, base, lo, hi, mode))
                else:
                    lines.extend(self._raw(data, base, lo, hi, mode))
            except Exception as ex:  # backend quebrou: cai para raw neste trecho
                warn(f"{path.name}: backend {backend} falhou ({ex}); usando raw")
                lines.extend(self._raw(data, base, lo, hi, mode))
        return lines


def pick_backend(choice: str, objdump_arg: str | None) -> tuple[str, str | None]:
    if choice in ("auto", "capstone"):
        try:
            import capstone  # noqa: F401
            return "capstone", None
        except ImportError:
            if choice == "capstone":
                die("capstone não instalado (pip install capstone)")
    if choice in ("auto", "objdump"):
        for exe in ([objdump_arg] if objdump_arg else
                    ["arm-none-eabi-objdump", "arm-linux-gnueabihf-objdump", "arm-linux-gnueabi-objdump"]):
            if exe and shutil.which(exe):
                return "objdump", shutil.which(exe)
        if choice == "objdump":
            die("objdump para ARM não encontrado (use --objdump CAMINHO)")
    if choice == "auto":
        warn("nem capstone nem objdump-ARM encontrados: usando backend 'raw' (.word)")
    return "raw", None


# -------------------------------------------------------------------------- saídas
def hx(n: int) -> str:
    return f"0x{n:08X}"


def symbols_for(cpu: str, lay: dict, overlays: list[dict]) -> list[tuple[str, int]]:
    syms = [("_start", lay["entry"])]
    m = lay["module"]
    if m:
        syms += [("ModuleParams", m["ram"]), ("AutoloadList", m["autoload_list_start"]),
                 ("StaticBssStart", m["static_bss_start"]), ("StaticBssEnd", m["static_bss_end"])]
    for s in lay["segments"]:
        syms.append((f"{cpu}_{s['name']}_start", s["ram"]))
    for ov in overlays:
        syms.append((f"ov{ov['id']}_start", ov["ram"]))
    return syms


def write_r2(path: Path, cpu: str, lay: dict, syms: list[tuple[str, int]], out: Path) -> None:
    main = next(s for s in lay["segments"] if s["name"] == "main")
    lines = [f"# radare2: r2 -a arm -b 32 -m {main['ram']:#x} {out}/{main['bin']} -i {path}",
             "e asm.arch=arm", "e asm.bits=32"]
    for s in lay["segments"]:
        if s is not main and s["kind"] != "data":
            lines.append(f"o {out}/{s['bin']} {s['ram']:#x}")
    lines += [f"f sym.{n} 4 @ {a:#x}" for n, a in syms]
    path.write_text("\n".join(lines) + "\n")


def write_memory_layout(path: Path, layout: dict) -> None:
    L = [f"ROM: {layout['rom']['title']} [{layout['rom']['gamecode']}]  md5={layout['rom']['md5']}",
         f"     sha1={layout['rom']['sha1']}  referência: {layout['rom']['matches_known']}", ""]
    for cpu in ("arm9", "arm7"):
        lay = layout[cpu]
        L.append(f"== {cpu.upper()} ==  ROM@{lay['rom_offset']:#x}  entry={hx(lay['entry'])}  "
                 f"load={hx(lay['load'])}  size={lay['size']:#x}")
        if lay.get("secure_area"):
            L.append(f"   Secure Area: {lay['secure_area']}")
        if lay["module"]:
            m = lay["module"]
            L.append(f"   ModuleParams @ {hx(m['ram'])}  SDK={m['sdk_version']:#x}  "
                     f"bss estático {hx(m['static_bss_start'])}..{hx(m['static_bss_end'])}")
        L.append(f"   {'segmento':<14}{'RAM':<13}{'tamanho':<10}{'bss':<8}{'arq.off':<10}tipo")
        for s in lay["segments"]:
            L.append(f"   {s['name']:<14}{hx(s['ram']):<13}{s['size']:<#10x}{s['bss']:<#8x}"
                     f"{s['file_offset']:<#10x}{s['kind']}")
        for n in lay["notes"]:
            L.append(f"   nota: {n}")
        L.append("")
    for key, label in (("overlays9", "Overlays ARM9"), ("overlays7", "Overlays ARM7")):
        if layout[key]:
            L.append(f"== {label} ==")
            for o in layout[key]:
                L.append(f"   ov{o['id']:<3} RAM={hx(o['ram'])} size={o['size']:#x} bss={o['bss']:#x} "
                         f"file_id={o['file_id']} comprimido={o['compressed']} autenticado={o['authenticated']}")
            L.append("")
    for key in ("arm9i", "arm7i"):
        d = layout.get(key)
        if d:
            L.append(f"== {key.upper()} (DSi) == ROM@{d['rom_offset']:#x} load={hx(d['load'])} "
                     f"size={d['size']:#x} criptografado={d['encrypted']}")
    path.write_text("\n".join(L) + "\n")


# ------------------------------------------------------------------------------ main
def main() -> None:
    ap = argparse.ArgumentParser(description="Extrai ARM9/ARM7/overlays de uma ROM NDS.")
    ap.add_argument("rom", type=Path)
    ap.add_argument("-o", "--out", type=Path, default=Path("asm"))
    ap.add_argument("--backend", choices=("auto", "capstone", "objdump", "raw"), default="auto")
    ap.add_argument("--objdump", help="caminho do arm-none-eabi-objdump")
    ap.add_argument("--thumb-ranges", type=Path, help="arquivo com intervalos Thumb (endereços de RAM)")
    ap.add_argument("--no-disasm", action="store_true", help="não gera os .s")
    args = ap.parse_args()

    rom = args.rom.read_bytes()
    h = parse_header(rom)
    md5, sha1 = hashlib.md5(rom).hexdigest(), hashlib.sha1(rom).hexdigest()
    known = h["gamecode"] == KNOWN_ROM["gamecode"] and md5 == KNOWN_ROM["md5"] and sha1 == KNOWN_ROM["sha1"]
    if not known:
        warn(f"ROM diferente da referência (gamecode={h['gamecode']}, md5={md5}); "
             "os endereços podem não bater com o projeto")

    out: Path = args.out
    (out / "bin").mkdir(parents=True, exist_ok=True)
    (out / "symbols").mkdir(exist_ok=True)

    def save(rel: str, data: bytes) -> Path:
        p = out / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(data)
        return p

    layout: dict = {"rom": {"title": h["title"], "gamecode": h["gamecode"], "unit_code": h["unit_code"],
                            "size": len(rom), "md5": md5, "sha1": sha1, "matches_known": known}}
    save("bin/header.bin", rom[:0x4000])
    if h["banner_offset"] and h["banner_offset"] + 0xA00 <= len(rom):
        save("bin/banner.bin", rom[h["banner_offset"]:h["banner_offset"] + 0xA00])

    blobs: dict[str, bytes] = {}
    for cpu in ("arm9", "arm7"):
        c = h[cpu]
        blob = rom[c["rom_offset"]:c["rom_offset"] + c["size"]]
        if len(blob) != c["size"]:
            die(f"{cpu.upper()} se estende além do fim da ROM")
        blobs[cpu] = blob
        save(f"bin/{cpu}.bin", blob)
        lay = build_cpu_layout(cpu, blob, c)
        for s in lay["segments"]:
            save(s["bin"], blob[s["file_offset"]:s["file_offset"] + s["size"]])
        layout[cpu] = lay

    # tabelas de overlay + overlays
    fat = read_fat(rom, h)
    for cpu, key, tab in (("arm9", "overlays9", "y9"), ("arm7", "overlays7", "y7")):
        off, size = h[tab]
        layout[key] = parse_overlay_table(rom, off, size) if size else []
        if size:
            save(f"bin/{tab}.bin", rom[off:off + size])
        for ov in layout[key]:
            if ov["file_id"] >= len(fat):
                die(f"overlay {ov['id']}: file_id {ov['file_id']} fora da FAT")
            start, end = fat[ov["file_id"]]
            ov["bin"] = f"bin/ov{cpu[-1]}_{ov['id']:03d}.bin"
            save(ov["bin"], rom[start:end])
            if ov["compressed"]:
                warn(f"overlay {ov['id']} está comprimido (BLZ); descomprima antes de desmontar")

    # binários DSi
    for key in ("arm9i", "arm7i"):
        layout[key] = None
    if h["dsi"]:
        mc_start, mc_size = h["dsi"]["modcrypt1"]
        for key in ("arm9i", "arm7i"):
            d = dict(h["dsi"][key])
            if not d["size"] or d["rom_offset"] + d["size"] > len(rom):
                continue
            data = rom[d["rom_offset"]:d["rom_offset"] + d["size"]]
            d["encrypted"] = bool(mc_size and mc_start <= d["rom_offset"] < mc_start + mc_size)
            d["bin"] = f"bin/{key}.bin"
            save(d["bin"], data)
            layout[key] = d
            if d["encrypted"]:
                warn(f"{key.upper()} está criptografado (modcrypt): não será desmontado")

    # nitrofs
    paths = nitro_paths(rom, h)
    with open(out / "nitrofs_files.txt", "w", encoding="utf-8") as f:
        for fid, (a, b) in enumerate(fat):
            f.write(f"{fid:5d}  {a:#010x}  {b - a:9d}  {paths.get(fid, '<overlay/sem nome>')}\n")

    # símbolos, r2, layout
    for cpu, ovs in (("arm9", layout["overlays9"]), ("arm7", layout["overlays7"])):
        syms = symbols_for(cpu, layout[cpu], ovs)
        (out / f"symbols/{cpu}.sym").write_text("".join(f"{n} {a:#010x}\n" for n, a in syms))
        write_r2(out / f"{cpu}.r2", cpu, layout[cpu], syms, out)
    (out / "layout.json").write_text(json.dumps(layout, indent=2, default=list))
    write_memory_layout(out / "memory_layout.txt", layout)

    # disassembly
    if not args.no_disasm:
        backend, exe = pick_backend(args.backend, args.objdump)
        thumb = parse_ranges(args.thumb_ranges)
        jobs = []  # (nome, caminho do bin, base, tipo, arch)
        for cpu, arch in (("arm9", "armv5te"), ("arm7", "armv4t")):
            for s in layout[cpu]["segments"]:
                jobs.append((f"{cpu}_{s['name']}", s["bin"], s["ram"], s["kind"], arch))
        for key, arch in (("overlays9", "armv5te"), ("overlays7", "armv4t")):
            for ov in layout[key]:
                if not ov["compressed"]:
                    jobs.append((f"ov{key[-1]}_{ov['id']:03d}", ov["bin"], ov["ram"], "code", arch))
        for key, arch in (("arm9i", "armv5te"), ("arm7i", "armv4t")):
            d = layout[key]
            if d and not d["encrypted"]:
                jobs.append((key, d["bin"], d["load"], "code", arch))
        for name, rel, base, kind, arch in jobs:
            data = (out / rel).read_bytes()
            lines = Disassembler(backend, arch, exe).run(out / rel, data, base, kind, thumb)
            (out / f"{name}.s").write_text("\n".join(lines) + "\n", encoding="utf-8")
            print(f"  {name + '.s':<24}{len(data):>9} bytes  ({backend if kind == 'code' else 'raw'})")

    print(f"\nPronto. Layout em {out / 'memory_layout.txt'}")


if __name__ == "__main__":
    main()
