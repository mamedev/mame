#!/usr/bin/python

from __future__ import print_function

USAGE = """
Usage:
%s h8.lst <type> h8.inc (type = o/h/s20/s26)
"""
import sys

def name_to_type(name):
    if name == "o":
        return 0
    if name == "h":
        return 1
    if name == "s20":
        return 2
    if name == "s26":
        return 3
    sys.stderr.write("Unknown chip type name %s\n" % name)
    sys.exit(1)

def type_to_device(dtype):
    if dtype == 0:
        return "h8_device"
    if dtype == 1:
        return "h8h_device"
    if dtype == 2:
        return "h8s2000_device"
    return "h8s2600_device"

def hexsplit(str):
    res = []
    for i in range(0, len(str), 2):
        res.append(int(str[i:i+2], 16))
    return res
        
def has_memory(ins):
    for s in ["read", "write", "sp_push", "sp_pop", "sp32_push", "sp32_pop", "fetch(", "prefetch_start(", "prefetch(", "prefetch_noirq("]:
        if s in ins:
            return True
    return False

def has_eat(ins):
    if "eat-all-cycles" in ins:
        return True
    return False

def save_full_one(f, t, name, source):
    print("void %s::%s_full()" % (t, name), file=f)
    print("{", file=f)
    substate = 1
    for line in source:
        if has_memory(line):
            print("\tif(icount <= bcount) { inst_substate = %d; return; }" % substate, file=f)
            print(line, file=f)
            substate += 1
        elif has_eat(line):
            print("\tif(icount) icount = bcount; inst_substate = %d; return;" % substate, file=f)
            substate += 1
        else:
            print(line, file=f)
    print("}", file=f)
    print("", file=f)

def save_partial_one(f, t, name, source):
    print("void %s::%s_partial()" % (t, name), file=f)
    print("{", file=f)
    print("switch(inst_substate) {", file=f)
    print("case 0:", file=f)
    substate = 1
    for line in source:
        if has_memory(line):
            print("\tif(icount <= bcount) { inst_substate = %d; return; }" % substate, file=f)
            print("case %d:;" % substate, file=f)
            print(line, file=f)
            substate += 1
        elif has_eat(line):
            print("\tif(icount) icount = bcount; inst_substate = %d; return;" % substate, file=f)
            print("case %d:;" % substate, file=f)
            substate += 1
        else:
            print(line, file=f)
    print("\tbreak;", file=f)
    print("}", file=f)
    print("\tinst_substate = 0;", file=f)
    print("}", file=f)
    print("", file=f)

class Hash:
    def __init__(self, premask):
        self.mask = 0x00
        self.enabled = False
        self.premask = premask
        self.d = {}

    def get(self, val, premask):
        if val in self.d:
            h = self.d[val]
            if h.premask != premask:
                sys.stderr.write("Premask conflict\n")
                sys.exit(1)
            return h
        h = Hash(premask)
        self.d[val] = h
        return h
    
    def set(self, val, opc):
        if val in self.d:
            sys.stderr.write("Collision on %s\n" % opc.description())
            sys.exit(1)
        self.d[val] = opc

class Opcode:
    def __init__(self, val, mask, skip, name, am1, am2, otype, dtype):
        self.name = name
        self.val = hexsplit(val)
        self.mask = hexsplit(mask)
        self.skip = int(skip)
        self.am1 = am1
        self.am2 = am2
        self.source = []
        self.otype = otype
        self.enabled = otype == -1 or (otype == 0 and dtype == 0) or (otype != 0 and dtype >= otype)
        self.needed = self.enabled and (otype == dtype or (otype == -1 and dtype == 0))
        if dtype == 0 and (am1 == "r16l" or am2 == "r16l"):
            self.mask[len(self.mask) - 1] |= 0x08
        if dtype == 0 and (am1 == "r16h" or am2 == "r16h"):
            self.mask[len(self.mask) - 1] |= 0x80
        extra_words = 0
        if (am1 == "abs16" or am2 == "abs16" or am1 == "abs16e" or am1 == "abs24e") and self.skip == 0:
            extra_words += 1
        if (am1 == "abs32" or am2 == "abs32") and self.skip == 0:
            extra_words += 2
        if am1 == "imm16" or am1 == "rel16" or am1 == "r16d16h" or am2 == "r16d16h" or am1 == "r32d16h" or am2 == "r32d16h":
            extra_words += 1
        if am1 == "imm32" or am1 == "r32d32hh" or am2 == "r32d32hh":
            extra_words += 2
        self.extra_words = extra_words
        base_offset = len(self.val)/2 + self.skip
        for i in range(0, extra_words):
            self.source.append("\tfetch(%d);\n" % (i+base_offset))

    def description(self):
        return "%s %s %s" % (self.name, self.am1, self.am2)
    
    def add_source_line(self, line):
        self.source.append(line)
    
    def is_dispatch(self):
        return False
    
    def function_name(self):
        n = self.name.replace(".", "_")
        if self.am1 != "-":
            n = n + "_" + self.am1
        if self.am2 != "-":
            n = n + "_" + self.am2
        return n

    def save_dasm(self, f):
        if len(self.mask) == 2:
            mask  = (self.mask[0] << 8) | self.mask[1]
            val   = (self.val[0]  << 8) | self.val[1]
            mask2 = 0
            val2  = 0
            slot  = 0
        elif len(self.mask) == 4:
            mask  = (self.mask[0] << 24) | (self.mask[1] << 16) | (self.mask[2] << 8) | self.mask[3]
            val   = (self.val[0]  << 24) | (self.val[1]  << 16) | (self.val[2]  << 8) | self.val[3]
            mask2 = 0
            val2  = 0
            slot = self.skip + 1
        else:
            mask  = (self.mask[2] << 24) | (self.mask[3] << 16) | (self.mask[4] << 8) | self.mask[5]
            val   = (self.val[2]  << 24) | (self.val[3]  << 16) | (self.val[4]  << 8) | self.val[5]
            mask2 = (self.mask[0] << 8)  | self.mask[1]
            val2  = (self.val[0]  << 8)  | self.val[1]
            slot = 4
        
        size = len(self.val) + 2*self.skip + 2*self.extra_words
        
        if self.name == "jsr" or self.name == "bsr":
            flags = "%d | DASMFLAG_STEP_OVER" % size
        elif self.name == "rts" or self.name == "rte":
            flags = "%d | DASMFLAG_STEP_OUT" % size
        else:
            flags = "%d" % size
        
        print("\t{ %d, 0x%08x, 0x%08x, 0x%04x, 0x%04x, \"%s\", DASM_%s, DASM_%s, %s }, // %s" % ( slot, val, mask, val2, mask2, self.name, self.am1 if self.am1 != "-" else "none", self.am2 if self.am2 != "-" else "none", flags, "needed" if self.needed else "inherited"), file=f)

class Special:
    def __init__(self, val, name, otype, dtype):
        self.name = name
        self.val = int(val, 16)
        self.enabled = otype == -1 or (otype == 0 and dtype == 0) or (otype != 0 and dtype >= otype)
        self.needed = otype == dtype or (otype == -1 and dtype == 0)
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
        if len(self.params) > 1:
            for i in range(0, len(self.params)-1):
                values.append(tokens[i+1])
        lval = ""
        for i in range(len(self.params)-1, len(tokens)-1):
            if lval != "":
                lval += " "
            lval = lval + tokens[i+1]
        values.append(lval)
        for i in range(0, len(self.source)):
            line = self.source[i]
            for j in range(0, len(self.params)):
                line = line.replace(self.params[j], values[j])
            target.add_source_line(line)

class DispatchStep:
    def __init__(self, id, pos, opc):
        self.id = id
        self.pos = pos
        self.name = ""
        self.enabled = False
        self.mask = opc.mask[pos-1]
        for i in range(0, pos):
            self.name += "%02x" % opc.val[i]
        if pos == 2:
            self.skip = opc.skip
        else:
            self.skip = 0

    def is_dispatch(self):
        return True

    def source(self):
        start = self.pos // 2
        end = start + self.skip
        s = []
        for i in range(start, end+1):
            s.append("\tIR[%d] = fetch();" % i)
        s.append("\tinst_state = 0x%x0000 | IR[%d];" % (self.id, end))
        return s


class OpcodeList:
    def __init__(self, fname, dtype):
        self.opcode_info = []
        self.dispatch_info = []
        self.states_info = []
        self.dispatch = {}
        self.macros = {}
        try:
            f = open(fname, "r")
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
                if inf is not None:
                    # append instruction to last opcode, maybe expand a macro
                    tokens = line.split()
                    if tokens[0] in self.macros:
                        self.macros[tokens[0]].apply(inf, tokens)
                    else:
                        inf.add_source_line(line)
            else:
                # New opcode
                tokens = line.split()
                if tokens[0] == "macro":
                    inf = Macro(tokens)
                    self.macros[inf.name] = inf
                elif len(tokens) == 2 or len(tokens) == 3:
                    if len(tokens) >= 3:
                        otype = name_to_type(tokens[2])
                    else:
                        otype = -1
                    inf = Special(tokens[0], tokens[1], otype, dtype)
                    self.states_info.append(inf)
                else:
                    if len(tokens) >= 7:
                        otype = name_to_type(tokens[6])
                    else:
                        otype = -1
                    if otype == -1 or dtype == 0 or (otype != 0 and dtype != 0):
                        inf = Opcode(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5], otype, dtype)
                        self.opcode_info.append(inf)
                    else:
                        inf = None

    def get(self, i):
        if i in self.dispatch:
            return self.dispatch[i]
        h = Hash(0)
        self.dispatch[i] = h
        return h
    
    def build_dispatch(self):
        for opc in self.opcode_info:
            for i in range(0, len(opc.val)):
                v = opc.val[i]
                if i == 0:
                    h = self.get(0)
                if opc.enabled:
                    h.mask = h.mask | opc.mask[i]
                    h.enabled = True
                if (i & 1) == 0:
                    h = h.get(v, opc.mask[i])
                elif i == len(opc.val)-1:
                    if opc.enabled:
                        h.set(v, opc)
                else:
                    if v in h.d:
                        d = h.d[v]
                        if not d.is_dispatch():
                            sys.stderr.write("Collision on %s\n" % opc.description())
                            sys.exit(1)
                        if opc.enabled:
                            d.enabled = True
                        h = self.get(d.id)
                    else:
                        d = DispatchStep(len(self.dispatch_info)+2, i+1, opc)
                        self.dispatch_info.append(d)
                        if opc.enabled:
                            d.enabled = True
                        h.set(v, d)
                        h = self.get(d.id)
    
    def save_dasm(self, f, dname):
        print("const %s::disasm_entry %s::disasm_entries[] = {" % (dname, dname), file=f)
        for opc in self.opcode_info:
            if opc.enabled:
                opc.save_dasm(f)
        print("\t{ 0, 0, 0, 0, 0, \"illegal\", 0, 0, 2 },", file=f)
        print("};", file=f)
        print("", file=f)
    
    def save_opcodes(self, f, t):
        for opc in self.opcode_info:
            if opc.needed:
                save_full_one(f, t, opc.function_name(), opc.source)
                save_partial_one(f, t, opc.function_name(), opc.source)
        
        for sta in self.states_info:
            if sta.needed:
                save_full_one(f, t, "state_" + sta.name, sta.source)
                save_partial_one(f, t, "state_" + sta.name, sta.source)
    
    def save_dispatch(self, f, t):
        for dsp in self.dispatch_info:
            save_full_one(f, t, "dispatch_" + dsp.name, dsp.source())
            save_partial_one(f, t, "dispatch_" + dsp.name, dsp.source())
    
    def save_exec(self, f, t, dtype, v):
        print("void %s::do_exec_%s()" % (t, v), file=f)
        print("{", file=f)
        print("\tswitch(inst_state >> 16) {", file=f)
        for i in range(0, len(self.dispatch_info)+2):
            if i == 1:
                print("\tcase 0x01: {", file=f)
                print("\t\tswitch(inst_state & 0xffff) {", file=f)
                for sta in self.states_info:
                    if sta.enabled:
                        print("\t\tcase 0x%02x: state_%s_%s(); break;" % (sta.val & 0xffff, sta.name, v), file=f)
                print("\t\t}", file=f)
                print("\t\tbreak;", file=f)
                print("\t}", file=f)
            else:
                if i == 0 or self.dispatch_info[i-2].enabled:
                    print("\tcase 0x%02x: {" % i, file=f)
                    h = self.get(i)
                    print("\t\tswitch((inst_state >> 8) & 0x%02x) {" % h.mask, file=f)
                    for val, h2 in sorted(h.d.items()):
                        if h2.enabled:
                            fmask = h2.premask | (h.mask ^ 0xff)
                            c = ""
                            s = 0
                            while s < 0x100:
                                c += "case 0x%02x: " % (val | s)
                                s += 1
                                while s & fmask:
                                    s += s & fmask
                            print("\t\t%s{" % c, file=f)
                            if h2.mask == 0x00:
                                n = h2.d[0]
                                if n.is_dispatch():
                                    print("\t\t\tdispatch_%s_%s();" % (n.name, v), file=f)
                                else:
                                    print("\t\t\t%s_%s();" % (n.function_name(), v), file=f)
                                print("\t\t\tbreak;", file=f)
                            else:
                                print("\t\t\tswitch(inst_state & 0x%02x) {" % h2.mask, file=f)
                                if i == 0:
                                    mpos = 1
                                else:
                                    mpos = self.dispatch_info[i-2].pos + 1
                                for val2, n in sorted(h2.d.items()):
                                    if n.enabled:
                                        fmask = h2.mask ^ 0xff
                                        if n.is_dispatch():
                                            fmask = fmask | n.mask
                                        else:
                                            fmask = fmask | n.mask[mpos]
                                        c = ""
                                        s = 0
                                        while s < 0x100:
                                            c += "case 0x%02x: " % (val2 | s)
                                            s += 1
                                            while s & fmask:
                                                s += s & fmask
                                        if n.is_dispatch():
                                            print("\t\t\t%sdispatch_%s_%s(); break;" % (c, n.name, v), file=f)
                                        else:
                                            print("\t\t\t%s%s_%s(); break;" % (c, n.function_name(), v), file=f)
                                print("\t\t\tdefault: illegal(); break;", file=f)
                                print("\t\t\t}", file=f)
                                print("\t\t\tbreak;", file=f)
                            print("\t\t}", file=f)
                    print("\t\tdefault: illegal(); break;", file=f)
                    print("\t\t}", file=f)
                    print("\t\tbreak;", file=f)
                    print("\t}", file=f)
        print("\t}", file=f)
        print("}", file=f)

def main(argv):
    if len(argv) != 4:
        print(USAGE % argv[0])
        return 1

    dtype = name_to_type(argv[2])
    dname = type_to_device(dtype)
    opcodes = OpcodeList(argv[1], dtype)
    
    try:
        f = open(argv[3], "w")
    except Exception:
        err = sys.exc_info()[1]
        sys.stderr.write("cannot write file %s [%s]\n" % (argv[3], err))
        sys.exit(1)

    opcodes.build_dispatch()
    opcodes.save_dasm(f, dname)
    opcodes.save_opcodes(f, dname)
    if dtype == 0:
        opcodes.save_dispatch(f, dname)
    opcodes.save_exec(f, dname, dtype, "full")
    opcodes.save_exec(f, dname, dtype, "partial")
    f.close()

# ======================================================================
if __name__ == "__main__":
    sys.exit(main(sys.argv))

