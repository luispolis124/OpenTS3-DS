#!/usr/bin/env python3
"""
tools/project_config.py - lê e valida config/project.json.

    python tools/project_config.py show                    # resumo legível (em hex)
    python tools/project_config.py verify-rom baserom.nds  # size/gamecode/md5/sha1 batem?
    python tools/project_config.py check                   # config x asm/layout.json
    python tools/project_config.py init                    # regenera a config a partir de asm/layout.json
    python tools/project_config.py get rom.output          # imprime um valor (caminho com pontos)

Códigos de saída: 0 = ok, 1 = divergência/erro. Use em CI e no Makefile (alvos verify-rom e check).

Como módulo:
    from project_config import load, hexs
    cfg = load()                       # procura config/project.json a partir da raiz do repositório
    cfg["cpus"]["arm9"]["load"]
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CONFIG = ROOT / "config" / "project.json"
DEFAULT_LAYOUT = ROOT / "asm" / "layout.json"

CPU_INFO = {  # parte "humana" da config: não vem do layout
    "arm9": {"core": "ARM946E-S", "architecture": "ARMv5TE", "ghidra_language": "ARM:LE:32:v5t",
             "ghidra_compiler": "default", "gcc_cpu": "arm946e-s", "objdump_arch": "armv5te"},
    "arm7": {"core": "ARM7TDMI", "architecture": "ARMv4T", "ghidra_language": "ARM:LE:32:v4t",
             "ghidra_compiler": "default", "gcc_cpu": "arm7tdmi", "objdump_arch": "armv4t"},
}
SEG_KEYS = ("name", "ram", "size", "bss", "file_offset", "kind")
MOD_KEYS = ("ram", "file_offset", "sdk_version", "autoload_entry_size", "static_bss_start",
            "static_bss_end", "autoload_start", "autoload_list_start", "autoload_list_end")
OVL_KEYS = ("id", "ram", "size", "bss", "file_id", "compressed")


def hexs(v):
    """Formata inteiros como hex; deixa o resto como está."""
    return f"{v:#010x}" if isinstance(v, int) and not isinstance(v, bool) and v > 255 else str(v)


def load(path: Path = DEFAULT_CONFIG) -> dict:
    try:
        return json.loads(Path(path).read_text(encoding="utf-8"))
    except FileNotFoundError:
        sys.exit(f"[erro] {path} não existe (rode 'project_config.py init' depois de extrair a ROM)")
    except json.JSONDecodeError as ex:
        sys.exit(f"[erro] {path}: JSON inválido ({ex})")


def from_layout(layout: dict, old: dict | None = None) -> dict:
    """Monta a config a partir de asm/layout.json, preservando 'tools'/'build'/nomes de arquivos."""
    old = old or {}
    rom = layout["rom"]
    cfg = {
        "_comment": ["Configuração do projeto. Gerada a partir de asm/layout.json (tools/extract_code.py) e conferida por",
                     "'python tools/project_config.py check'. Se a ROM de referência mudar, regenere com 'project_config.py init'.",
                     "Endereços em decimal (JSON não tem hexadecimal); o utilitário imprime em hex."],
        "project": old.get("project", "OpenTS3DS"),
        "rom": {"filename": old.get("rom", {}).get("filename", "baserom.nds"),
                "output": old.get("rom", {}).get("output", "build/sims3.nds"),
                "title": rom["title"], "gamecode": rom["gamecode"], "unit_code": rom["unit_code"],
                "size": rom["size"], "md5": rom["md5"], "sha1": rom["sha1"]},
        "cpus": {},
        "overlays9": [{k: o[k] for k in OVL_KEYS} for o in layout["overlays9"]],
        "dsi": {k: {f: layout[k][f] for f in ("rom_offset", "load", "size", "encrypted")}
                for k in ("arm9i", "arm7i") if layout.get(k)},
        "tools": old.get("tools", {"asm_dir": "asm", "thumb_ranges": "tools/thumb_ranges.txt",
                                   "disasm_backend": "auto", "ghidra_export_dir": "build/ghidra_export"}),
        "build": old.get("build", {"build_dir": "build", "compiler": "gcc", "linkscript": "linkscript.ld"}),
    }
    for name in ("arm9", "arm7"):
        c = layout[name]
        cfg["cpus"][name] = {
            **CPU_INFO[name],
            "rom_offset": c["rom_offset"], "load": c["load"], "entry": c["entry"], "size": c["size"],
            "module_params": {k: c["module"][k] for k in MOD_KEYS},
            "segments": [{k: s[k] for k in SEG_KEYS} for s in c["segments"]],
        }
    return cfg


def diff(a, b, path="") -> list[str]:
    """Diferenças entre duas estruturas JSON (a = config, b = layout), só as chaves que a config tem."""
    out = []
    if isinstance(a, dict):
        for k, v in a.items():
            if k.startswith("_"):
                continue
            if not isinstance(b, dict) or k not in b:
                out.append(f"{path}{k}: ausente no layout")
            else:
                out += diff(v, b[k], f"{path}{k}.")
    elif isinstance(a, list):
        if not isinstance(b, list) or len(a) != len(b):
            out.append(f"{path[:-1]}: {len(a)} itens na config, {len(b) if isinstance(b, list) else '?'} no layout")
        else:
            for i, (x, y) in enumerate(zip(a, b)):
                out += diff(x, y, f"{path[:-1]}[{i}].")
    elif a != b:
        out.append(f"{path[:-1]}: config={hexs(a)} layout={hexs(b)}")
    return out


def cmd_check(cfg: dict, layout_path: Path) -> int:
    if not layout_path.exists():
        print(f"[erro] {layout_path} não existe; rode tools/extract_code.py primeiro", file=sys.stderr)
        return 1
    layout = json.loads(layout_path.read_text(encoding="utf-8"))
    expect = from_layout(layout, cfg)
    problems = diff({k: v for k, v in cfg.items() if k in ("rom", "cpus", "overlays9", "dsi")}, expect)
    if problems:
        print("config/project.json diverge do layout extraído da ROM:")
        for p in problems:
            print("  -", p)
        return 1
    print("config/project.json confere com", layout_path)
    return 0


def cmd_verify_rom(cfg: dict, rom_path: Path) -> int:
    if not rom_path.exists():
        print(f"[erro] {rom_path} não existe", file=sys.stderr)
        return 1
    data = rom_path.read_bytes()
    want = cfg["rom"]
    got = {"size": len(data), "gamecode": data[0xC:0x10].decode("ascii", "replace"),
           "md5": hashlib.md5(data).hexdigest(), "sha1": hashlib.sha1(data).hexdigest()}
    bad = [k for k in got if got[k] != want[k]]
    for k in got:
        print(f"  {k:9s} {'ok ' if k not in bad else 'ERRO'}  {got[k]}" + (f"  (esperado {want[k]})" if k in bad else ""))
    if bad:
        print("ROM diferente da de referência: endereços e layout do projeto podem não valer para ela.")
        return 1
    return 0


def cmd_show(cfg: dict) -> int:
    r = cfg["rom"]
    print(f"{cfg['project']}: {r['title']} [{r['gamecode']}] md5={r['md5']}")
    for name, c in cfg["cpus"].items():
        print(f"\n{name.upper()} ({c['core']}, {c['architecture']})  ghidra={c['ghidra_language']}  gcc -mcpu={c['gcc_cpu']}")
        print(f"  ROM@{c['rom_offset']:#x}  load={c['load']:#010x}  entry={c['entry']:#010x}  size={c['size']:#x}")
        for s in c["segments"]:
            print(f"    {s['name']:<14}{s['ram']:#010x}  size={s['size']:<#8x} bss={s['bss']:#x}  [{s['kind']}]")
    for o in cfg["overlays9"]:
        print(f"  ov{o['id']}: {o['ram']:#010x} size={o['size']:#x} bss={o['bss']:#x} file_id={o['file_id']}")
    return 0


def get_path(cfg: dict, dotted: str):
    cur = cfg
    for part in dotted.split("."):
        try:
            cur = cur[int(part)] if isinstance(cur, list) else cur[part]
        except (KeyError, IndexError, ValueError):
            sys.exit(f"[erro] chave '{dotted}' não existe (parou em '{part}')")
    return cur


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--config", type=Path, default=DEFAULT_CONFIG)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("show")
    p = sub.add_parser("verify-rom"); p.add_argument("rom", type=Path)
    p = sub.add_parser("check"); p.add_argument("--layout", type=Path, default=DEFAULT_LAYOUT)
    p = sub.add_parser("init"); p.add_argument("--layout", type=Path, default=DEFAULT_LAYOUT)
    p = sub.add_parser("get"); p.add_argument("key")
    args = ap.parse_args()

    if args.cmd == "init":
        if not args.layout.exists():
            sys.exit(f"[erro] {args.layout} não existe; rode tools/extract_code.py primeiro")
        old = json.loads(args.config.read_text(encoding="utf-8")) if args.config.exists() else {}
        cfg = from_layout(json.loads(args.layout.read_text(encoding="utf-8")), old)
        args.config.parent.mkdir(parents=True, exist_ok=True)
        args.config.write_text(json.dumps(cfg, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print("escrito", args.config)
        return

    cfg = load(args.config)
    if args.cmd == "show":
        sys.exit(cmd_show(cfg))
    if args.cmd == "verify-rom":
        sys.exit(cmd_verify_rom(cfg, args.rom))
    if args.cmd == "check":
        sys.exit(cmd_check(cfg, args.layout))
    if args.cmd == "get":
        v = get_path(cfg, args.key)
        print(json.dumps(v) if isinstance(v, (dict, list)) else v)


if __name__ == "__main__":
    main()
