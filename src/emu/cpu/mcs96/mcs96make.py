#!/usr/bin/python

from __future__ import print_function

USAGE = """
Usage:
%s mcs96ops.lst mcs96.inc
"""
import sys

def save_full_one(f, t, name, source):
    print("void %s_device::%s_full()" % (t, name), file=f)
    print("{", file=f)
    for line in source:
        print(line, file=f)
    print("}", file=f)
    print("", file=f)

class Opcode:
    def __init__(self, rng, name, amode, is_196, ea):
        rng1 = rng.split("-")
        self.rng_start = int(rng1[0], 16)
        if len(rng1) == 2:
            self.rng_end = int(rng1[1], 16)
        else:
            self.rng_end = self.rng_start
        self.name = name
        self.amode = amode
        self.source = []
        self.is_196 = is_196
        if amode in ea:
            for line in ea[amode].source:
                self.source.append(line)

    def add_source_line(self, line):
        self.source.append(line)

class Special:
    def __init__(self, name):
        self.name = name
        self.source = []

    def add_source_line(self, line):
        self.source.append(line)

class Macro:
    def __init__(self, tokens):
        self.name = tokens[1]
        self.params = []
        for i in range(2, len(tokens)):
            self.params.append(tokens[i])
        self.source = []

    def add_source_line(self, line):
        self.source.append(line)

    def apply(self, target, tokens):
        values = []
        for i in range(1, len(tokens)):
            values.append(tokens[i])
        for i in range(0, len(self.source)):
            line = self.source[i]
            for j in range(0, len(self.params)):
                line = line.replace(self.params[j], values[j])
            target.add_source_line(line)

class OpcodeList:
    def __init__(self, fname, is_196):
        self.opcode_info = []
        self.opcode_per_id = {}
        self.ea = {}
        self.macros = {}
        try:
            f = open(fname, "rU")
        except Exception:
            err = sys.exc_info()[1]
            sys.stderr.write("Cannot read opcodes file %s [%s]\n" % (fname, err))
            sys.exit(1)

        inf = None
        for line in f:
            if line.startswith("#"):
                continue
            line = line.rstrip()
            if not line:
                continue
            if line.startswith(" ") or line.startswith("\t"):
                # append instruction to last opcode, maybe expand a macro
                tokens = line.split()
                if tokens[0] in self.macros:
                    self.macros[tokens[0]].apply(inf, tokens)
                else:
                    inf.add_source_line(line)
            else:
                # New something
                tokens = line.split()
                #   Addressing mode header
                if tokens[0] == "eadr":
                    inf = Special(tokens[1])
                    self.ea[inf.name] = inf
                elif tokens[0] == "fetch":
                    inf = Special(tokens[0])
                    self.fetch = inf
                elif tokens[0] == "fetch_noirq":
                    inf = Special(tokens[0])
                    self.fetch_noirq = inf
                elif tokens[0] == "macro":
                    inf = Macro(tokens)
                    self.macros[inf.name] = inf
                else:
                    inf = Opcode(tokens[0], tokens[1], tokens[2], len(tokens) >= 4 and tokens[3] == "196", self.ea)
                    self.opcode_info.append(inf)
                    if is_196 or not inf.is_196:
                        for i in range(inf.rng_start, inf.rng_end+1):
                            self.opcode_per_id[i] = inf

    def save_dasm(self, f, t):
        print("const %s_device::disasm_entry %s_device::disasm_entries[0x100] = {" % (t, t), file=f)
        for i in range(0, 0x100):
            if i in self.opcode_per_id:
                opc = self.opcode_per_id[i]
                alt = "NULL"
                if i + 0xfe00 in self.opcode_per_id:
                    alt = "\"" + self.opcode_per_id[i+0xfe00].name + "\""
                if opc.name == "scall" or opc.name == "lcall":
                    flags = "DASMFLAG_STEP_OVER"
                elif opc.name == "rts":
                    flags = "DASMFLAG_STEP_OUT"
                else:
                    flags = "0"
                print("\t{ \"%s\", %s, DASM_%s, %s }," % (opc.name, alt, opc.amode, flags), file=f)
            else:
                print("\t{ \"???\", NULL, DASM_none, 0 },", file=f)
        print("};", file=f)
        print("", file=f)
    
    def save_opcodes(self, f, t):
        pf = ""
        is_196 = False
        if t == "i8xc196":
            pf = "_196"
            is_196 = True
        for opc in self.opcode_info:
            if opc.is_196 == is_196:
                save_full_one(f, t, opc.name + "_" + opc.amode + pf, opc.source)
        if not is_196:
            save_full_one(f, t, "fetch", self.fetch.source)
            save_full_one(f, t, "fetch_noirq", self.fetch_noirq.source)
    
    def save_exec(self, f, t):
        print("void %s_device::do_exec_full()" % t, file=f)
        print("{", file=f)
        print("\tswitch(inst_state) {", file=f)
        for i in range(0x000, 0x200):
            opc = None
            if i >= 0x100 and i-0x100+0xfe00 in self.opcode_per_id:
                opc = self.opcode_per_id[i-0x100+0xfe00]
            if opc is None and (i & 0xff) in self.opcode_per_id:
                opc = self.opcode_per_id[i & 0xff]
            if opc is not None:
                nm = opc.name + "_" + opc.amode
                if opc.is_196:
                    nm += "_196"
                print("\tcase 0x%03x: %s_full(); break;" % (i, nm), file=f)
        print("\tcase 0x200: fetch_full(); break;", file=f)
        print("\tcase 0x201: fetch_noirq_full(); break;", file=f)
        print("\t}", file=f)
        print("}", file=f)

def main(argv):
    if len(argv) != 4:
        print(USAGE % argv[0])
        return 1
    
    t = argv[1]
    opcodes = OpcodeList(argv[2], t == "i8xc196")
    
    try:
        f = open(argv[3], "w")
    except Exception:
        err = sys.exc_info()[1]
        sys.stderr.write("cannot write file %s [%s]\n" % (argv[3], err))
        sys.exit(1)
    
    if t != "mcs96":
        opcodes.save_dasm(f, t)
    if t != "i8x9x":
        opcodes.save_opcodes(f, t)
    if t != "mcs96":
        opcodes.save_exec(f, t)
    f.close()

# ======================================================================
if __name__ == "__main__":
    sys.exit(main(sys.argv))

