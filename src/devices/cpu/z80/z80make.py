#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Andre I. Holub
from __future__ import print_function

USAGE = """
Usage:
%s [type] z80.lst z80.inc
"""
import sys

class IndStr:
    def __init__(self, src, indent = None):
        self.str = src.strip()
        self.indent = indent
        if not indent:
            self.indent = src[:len(src) - len(src.lstrip())]

    def line(self):
        return self.str
    
    def get_indent(self):
        return self.indent
    
    def is_comment(self):
        return self.str.startswith("#") and not self.str.startswith("#if") and not self.str.startswith("#endif")
    
    def is_blank(self):
        return not self.str
    
    def has_indent(self):
        return self.indent != ''

    def replace(self, new):
        self.str = new
    
    def replace(self, old, new):
        return IndStr(self.str.replace(old, new), self.indent)
    
    def with_str(self, new_str):
        return IndStr(new_str, self.indent)
    
    def split(self):
        return self.str.split()
    
    def print(self, str, f):
        print("\t\t%s%s" % (self.indent, str), file=f)



class Opcode:
    def __init__(self, code):
        self.code = code
        self.source = []

    def add_source_lines(self, lines):
        self.source.extend(lines)

    def save_dasm(self, f):
        code = self.code
        has_steps = len(self.source) > 1
        if has_steps:
            print("\t\tswitch (u8(m_ref))", file=f)
            print("\t\t{", file=f)
            print("\t\tcase 0x00:", file=f)
        step = 0
        for i in range(0, len(self.source)):
            il = self.source[i]
            line = il.line()
            tokens = line.split()
            if tokens[0] == '+':
                il.print("m_icount -= %s;" % (" ".join(tokens[1:])), f)
                step += 1
                to_step = "0x%s" % (hex(256 + step)[3:])
                il.print("if (m_icount <= 0) {", f)
                il.print("	m_ref = (m_ref & 0xffff00) | %s;" % (to_step), f)
                il.print("	return;", f)
                il.print("}", f)
                il.print("[[fallthrough]];", f)
                print("\t\tcase %s:" % (to_step), file=f)
            elif (len(tokens) > 2 and tokens[1] == "!!"):
                il.print("[[fallthrough]];", f)
                step += 1;
                print("\t\tcase 0x%s:" % (hex(256 + step)[3:]), file=f)
                il.print("%s" % " ".join(tokens[2:]), f)
                il.print("m_icount -= %s;" % (tokens[0]), f)
                il.print("if (m_icount <= 0) {", f)
                il.print("	if (access_to_be_redone()) {", f)
                il.print("		m_icount += %s;" % (tokens[0]), f)
                il.print("		m_ref = (m_ref & 0xffff00) | 0x%s;" % (hex(256 + step)[3:]), f)
                il.print("	} else", f)
                step += 1
                to_step = "0x%s" % (hex(256 + step)[3:])
                il.print("		m_ref = (m_ref & 0xffff00) | %s;" % (to_step), f)
                il.print("	return;", f)
                il.print("}", f)
                il.print("[[fallthrough]];", f)
                print("\t\tcase %s:" % (to_step), file=f)
            else:
                il.print("%s" % line, f)
        if has_steps:
            print("\t\t\tbreak;\n", file=f)
            print("\t\t}", file=f)

class Macro:
    def __init__(self, name, arg_name = None):
        self.name = name
        self.source = []
        self.arg_name = arg_name

    def apply(self, arg):
        if self.arg_name is not None:
            return [ r.replace(self.arg_name, arg) for r in self.source ]
        else:
            return self.source

    def add_source_lines(self, lines):
        self.source.extend(lines)

class OpcodeList:
    def __init__(self, gen, fname):
        self.gen = gen
        self.opcode_info = []
        self.macros = {}

        try:
            f = open(fname, "r")
        except Exception:
            err = sys.exc_info()[1]
            sys.stderr.write("Cannot read opcodes file %s [%s]\n" % (fname, err))
            sys.exit(1)

        inf = None
        for ln in f:
            line = IndStr(ln)
            # Skip comments except macros
            if line.is_comment() or line.is_blank():
                continue
            if line.has_indent():
                if inf is not None:
                    if isinstance(inf, Macro):
                        inf.add_source_lines([line])
                    else:
                        inf.add_source_lines(self.pre_process(line))
            else:
                # New opcode
                tokens = line.split()
                if tokens[0] == "macro":
                    arg_name = None
                    if len(tokens) > 2:
                        arg_name = tokens[2]
                    nnames = tokens[1].split(":")
                    if len(nnames) == 2:
                        inf = Macro(nnames[1], arg_name)
                        if nnames[0] == self.gen:
                            self.macros[nnames[1]] = inf
                    else:
                        inf = Macro(nnames[0], arg_name)
                        if None == self.gen:
                            if nnames[0] in self.macros:
                                sys.stderr.write("Replacing macro: %s\n" % nnames[0])
                            self.macros[nnames[0]] = inf
                        else:
                            if not nnames[0] in self.macros:
                                self.macros[nnames[0]] = inf
                else:
                    ntokens = tokens[0].split(":")
                    if len(ntokens) == 2:
                        inf = Opcode(ntokens[1])
                        if ntokens[0] == self.gen:
                            # Replace in list when already present, otherwise append
                            found = False
                            found_index = 0
                            for i in range(len(self.opcode_info)):
                                if self.opcode_info[i].code == inf.code:
                                    found = True
                                    found_index = i
                            if found:
                                self.opcode_info[found_index] = inf
                            else:
                                self.opcode_info.append(inf)
                    else:
                        inf = Opcode(ntokens[0])
                        if None == self.gen:
                            self.opcode_info.append(inf)
                        else:
                            # Only place in list when not already present
                            found = False
                            for i in range(len(self.opcode_info)):
                                if self.opcode_info[i].code == inf.code:
                                    found = True
                            if not found:
                                self.opcode_info.append(inf)


    def pre_process(self, iline):
        out = []
        line = iline.str
        line_toc = line.split()
        times = 1
        if len(line_toc) > 2 and line_toc[1] == "*":
            times = int(line_toc[0])
            line_toc = line_toc[2:]
            line = " ".join(line_toc)
        for i in range(times):
            if line_toc[0] == 'call':
                name = line_toc[1]
                arg = None
                if len(line_toc) > 2:
                    arg = line_toc[2]
                if name in self.macros:
                    macro = self.macros[name]
                    ([out.extend(self.pre_process(il)) for il in macro.apply(arg)])
                else:
                    sys.stderr.write("Macro not found %s\n" % name)
                    out.append(iline.with_str("... %s" % name))
            else:
                out.append(iline.with_str(line))
        return out

    def save_exec(self, f):
        prefix = None
        print("switch (u8(m_ref >> 16)) // prefix", file=f)
        print("{", file=f)
        for opc in self.opcode_info:
            if (opc.code[:2]) != prefix:
                if prefix is not None:
                    print("\n\t}", file=f)
                    print("\t\tbreak;", file=f)
                    print("", file=f)
                    print("}", file=f)
                    print("break; // prefix: 0x%s" % (prefix), file=f)
                    print("", file=f)
                prefix = opc.code[:2]
                print("case 0x%s:" % (opc.code[:2]), file=f)
                print("{", file=f)
                print("\tswitch (u8(m_ref >> 8)) // opcode", file=f)
                print("\t{", file=f)
            print("\tcase 0x%s:" % (opc.code[2:]), file=f)
            #print("\t{", file=f)
            opc.save_dasm(f)
            #print("\t}", file=f)
            print("\t\tbreak;", file=f)
            print("", file=f)
        print("\t} // switch opcode", file=f)
        print("}", file=f)
        print("break; // prefix: 0x%s" % (prefix), file=f)
        print("", file=f)
        print("} // switch prefix", file=f)
        print("", file=f)
        print("m_ref = 0xffff00;", file=f)

def main(argv):
    if len(argv) != 3 and len(argv) != 4:
        print(USAGE % argv[0])
        return 1

    fidx = 1
    gen = None
    if len(argv) == 4:
        gen = argv[1]
        fidx = 2

    opcodes = OpcodeList(gen, argv[fidx])

    try:
        f = open(argv[fidx + 1], "w")
    except Exception:
        err = sys.exc_info()[fidx]
        sys.stderr.write("cannot write file %s [%s]\n" % (argv[fidx + 1], err))
        sys.exit(1)

    opcodes.save_exec(f)
    f.close()

# ======================================================================
if __name__ == "__main__":
    sys.exit(main(sys.argv))
