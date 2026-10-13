# Exporta o pseudo-C do decompilador do Ghidra, uma função por arquivo, mais um
# índice (index.csv) e a lista de funções Thumb (thumb_ranges.txt), que alimenta
# tools/extract_code.py --thumb-ranges.
#
# @category OpenTS3DS
#
# Compatível com Jython (Ghidra clássico) e PyGhidra (Python 3): sem f-strings.
#
# Uso na GUI : Window > Script Manager > este script (pergunta a pasta de saída).
# Uso headless:
#   analyzeHeadless <pasta_projeto> <projeto> -process <programa> -noanalysis \
#       -postScript ghidra_export_functions.py <pasta_saida>
#
# NÃO TESTADO: escrito contra a API do Ghidra, mas sem Ghidra no ambiente onde foi feito.
# Rode primeiro em uma função só (ajuste LIMIT) para validar.
import io
import os
import re

from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

LIMIT = 0          # 0 = todas as funções; N > 0 = só as N primeiras (para teste)
TIMEOUT_S = 60     # tempo máximo por função


def get_out_dir():
    args = getScriptArgs()
    if len(args) > 0:
        return args[0]
    return askDirectory("Pasta de saída", "Exportar aqui").getAbsolutePath()


def is_thumb(func):
    """True se a função está em modo Thumb (registrador de contexto TMode = 1)."""
    try:
        reg = currentProgram.getRegister("TMode")
        if reg is None:
            return False
        val = currentProgram.getProgramContext().getRegisterValue(reg, func.getEntryPoint())
        return val is not None and val.hasValue() and val.getUnsignedValue().intValue() == 1
    except Exception:
        return False


def safe_name(text):
    return re.sub(r"[^A-Za-z0-9_.-]", "_", text)


def write_text(path, text):
    with io.open(path, "w", encoding="utf-8") as fh:
        fh.write(u"%s" % text)


def main():
    outdir = get_out_dir()
    funcs_dir = os.path.join(outdir, "functions")
    if not os.path.isdir(funcs_dir):
        os.makedirs(funcs_dir)

    ifc = DecompInterface()
    ifc.openProgram(currentProgram)
    monitor = ConsoleTaskMonitor()

    index = [u"addr,name,size,mode,status,file"]
    thumb = [u"# programa: %s" % currentProgram.getName()]
    done = 0

    for func in currentProgram.getFunctionManager().getFunctions(True):
        if func.isThunk() or func.isExternal():
            continue
        if LIMIT and done >= LIMIT:
            break
        done += 1

        entry = func.getEntryPoint()
        start = entry.getOffset()
        size = func.getBody().getNumAddresses()
        thumb_mode = is_thumb(func)
        name = func.getName()
        fname = "%08x_%s.c" % (start, safe_name(name))

        status = "ok"
        res = ifc.decompileFunction(func, TIMEOUT_S, monitor)
        if res is not None and res.decompileCompleted():
            write_text(os.path.join(funcs_dir, fname), res.getDecompiledFunction().getC())
        else:
            status = "falhou"
            fname = ""

        index.append(u"0x%08x,%s,%d,%s,%s,%s" % (start, name, size, "thumb" if thumb_mode else "arm", status, fname))
        if thumb_mode:
            thumb.append(u"0x%08x 0x%08x" % (start, start + size))

    write_text(os.path.join(outdir, "index.csv"), u"\n".join(index) + u"\n")
    write_text(os.path.join(outdir, "thumb_ranges.txt"), u"\n".join(thumb) + u"\n")
    println("Exportadas %d funções para %s" % (done, outdir))


main()
