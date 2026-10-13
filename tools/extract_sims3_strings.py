#!/usr/bin/env python3
"""
Extrator de strings do The Sims 3 (Nintendo DS).

Aceita dois tipos de entrada (detectado automaticamente):

  1) Arquivo no formato 'STRG' (o que você descreveu):
        'STRG' | u32 LE = total de strings | strings terminadas em NUL

  2) A ROM .nds inteira: lê o NitroFS, abre DATA/LOCALIZATION/*.arc e extrai
     as strings de cada idioma (formato real encontrado nesta ROM, veja abaixo).

Formato dos .arc de localização (little-endian):
    u32 n_entradas | u32 tamanho_do_bloco_de_nomes | nomes concatenados
    5 vetores de n u32:
        [0] tamanho do nome   [1] offset do dado no arc   [2] offset do nome
        [3] tamanho comprimido (0xFFFFFFFF = sem compressão)
        [4] tamanho descomprimido (ou tamanho armazenado, se sem compressão)
    Entradas: <Idioma>.dat (zlib ou cru) e textdb.idx (1o u32 = nº de strings)

Formato do <Idioma>.dat:
    N pares (u32 offset, u32 tamanho) seguidos da área de strings (UTF-8, NUL).
    Os offsets são relativos ao início da área de strings (= 8*N).

Uso:
    python extract_sims3_strings.py ROM.nds                  # inglês -> txt
    python extract_sims3_strings.py ROM.nds --lang Spanish
    python extract_sims3_strings.py ROM.nds --all-langs
    python extract_sims3_strings.py arquivo_strg.bin -o saida.txt
"""
import argparse
import struct
import sys
import zlib
from pathlib import Path

RAW = 0xFFFFFFFF


# ---------------------------------------------------------------- formato STRG
def read_strg(data: bytes, encoding: str = "utf-8") -> list[str]:
    """Lê 'STRG' + u32 LE (total) + strings terminadas em NUL."""
    if data[:4] != b"STRG":
        raise ValueError("assinatura 'STRG' não encontrada")
    total = struct.unpack_from("<I", data, 4)[0]
    strings, pos = [], 8
    for _ in range(total):
        end = data.find(b"\x00", pos)
        if end == -1:  # arquivo truncado
            print(f"[aviso] esperado {total} strings, achei {len(strings)}", file=sys.stderr)
            break
        strings.append(data[pos:end].decode(encoding, errors="replace"))
        pos = end + 1
    return strings


# --------------------------------------------------------------------- NitroFS
def nitro_files(rom: bytes) -> dict[str, tuple[int, int]]:
    """Retorna {caminho: (offset, tamanho)} para todos os arquivos do NitroFS."""
    fnt_off, _, fat_off, fat_sz = struct.unpack_from("<IIII", rom, 0x40)
    fat = [struct.unpack_from("<II", rom, fat_off + i * 8) for i in range(fat_sz // 8)]
    files: dict[str, tuple[int, int]] = {}

    def walk(dir_id: int, path: str) -> None:
        sub_off, first_id, _ = struct.unpack_from("<IHH", rom, fnt_off + (dir_id & 0xFFF) * 8)
        pos, fid = fnt_off + sub_off, first_id
        while True:
            t = rom[pos]
            pos += 1
            if t == 0:
                break
            name = rom[pos:pos + (t & 0x7F)].decode("latin1")
            pos += t & 0x7F
            if t & 0x80:  # diretório
                sub_id = struct.unpack_from("<H", rom, pos)[0]
                pos += 2
                walk(sub_id, f"{path}{name}/")
            else:
                start, end = fat[fid]
                files[f"{path}{name}"] = (start, end - start)
                fid += 1

    walk(0xF000, "")
    return files


# ------------------------------------------------------------------------ .arc
def read_arc(blob: bytes) -> dict[str, bytes]:
    n, names_sz = struct.unpack_from("<II", blob, 0)
    names = blob[8:8 + names_sz]
    base = 8 + names_sz
    name_len, offs, name_off, csize, usize = (
        struct.unpack_from(f"<{n}I", blob, base + k * 4 * n) for k in range(5)
    )
    out = {}
    for i in range(n):
        name = names[name_off[i]:name_off[i] + name_len[i]].decode("latin1")
        if csize[i] == RAW:
            out[name] = blob[offs[i]:offs[i] + usize[i]]
        else:
            chunk = blob[offs[i]:offs[i] + csize[i]]
            try:
                out[name] = zlib.decompress(chunk)
            except zlib.error:
                out[name] = chunk
    return out


def read_dat(dat: bytes, count: int, encoding: str = "utf-8") -> list[str]:
    if not dat or count == 0:
        return []
    table = struct.unpack_from(f"<{2 * count}I", dat, 0)
    area = 8 * count
    strings = []
    for i in range(count):
        off, size = table[2 * i], table[2 * i + 1]
        raw = dat[area + off:area + off + size].split(b"\x00", 1)[0]
        strings.append(raw.decode(encoding, errors="replace"))
    return strings


# ------------------------------------------------------------------- saída/CLI
def esc(s: str) -> str:
    return s.replace("\\", "\\\\").replace("\r", "\\r").replace("\n", "\\n").replace("\t", "\\t")


def write_txt(path: Path, sections: list[tuple[str, list[str]]]) -> int:
    total = 0
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        for title, strings in sections:
            f.write(f"### {title} ({len(strings)} strings)\n")
            for i, s in enumerate(strings):
                f.write(f"{i:05d}\t{esc(s)}\n")
            f.write("\n")
            total += len(strings)
    return total


def extract_rom(rom: bytes, langs: list[str], encoding: str) -> dict[str, list]:
    files = nitro_files(rom)
    arcs = sorted(p for p in files if p.upper().startswith("DATA/LOCALIZATION/") and p.lower().endswith(".arc"))
    if not arcs:
        raise SystemExit("Nenhum .arc em DATA/LOCALIZATION/ — é mesmo a ROM do Sims 3 DS?")
    result: dict[str, list] = {lang: [] for lang in langs}
    for path in arcs:
        start, size = files[path]
        arc = read_arc(rom[start:start + size])
        idx = arc.get("textdb.idx", b"")
        count = struct.unpack_from("<I", idx, 0)[0] if len(idx) >= 4 else 0
        for lang in langs:
            dat = arc.get(f"{lang}.dat")
            if dat is None:
                continue
            result[lang].append((f"{Path(path).name} / {lang}.dat", read_dat(dat, count, encoding)))
    return result


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input", type=Path, help="ROM .nds ou arquivo STRG")
    ap.add_argument("-o", "--output", type=Path, help="arquivo .txt de saída")
    ap.add_argument("--lang", default="English",
                    help="English, French, Italian, German, Spanish, Dutch, Japanese, Korean")
    ap.add_argument("--all-langs", action="store_true", help="um .txt por idioma")
    ap.add_argument("--encoding", default="utf-8")
    args = ap.parse_args()

    data = args.input.read_bytes()

    if data[:4] == b"STRG":
        strings = read_strg(data, args.encoding)
        out = args.output or args.input.with_suffix(".txt")
        n = write_txt(out, [(args.input.name, strings)])
        print(f"{n} strings salvas em {out}")
        return

    all_langs = ["English", "French", "Italian", "German", "Spanish", "Dutch", "Japanese", "Korean"]
    langs = all_langs if args.all_langs else [args.lang]
    result = extract_rom(data, langs, args.encoding)
    for lang, sections in result.items():
        if not sections:
            print(f"[aviso] idioma '{lang}' não encontrado", file=sys.stderr)
            continue
        if args.output and not args.all_langs:
            out = args.output
        else:
            stem = (args.output or args.input).with_suffix("").name
            out = (args.output.parent if args.output else args.input.parent) / f"{stem}_{lang}.txt"
        n = write_txt(out, sections)
        print(f"{lang}: {n} strings salvas em {out}")


if __name__ == "__main__":
    main()
