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
        return self.indent != '' or self.str[0] == "{" or self.str[0] == "}"

    def strip_indent(self, n):
        if self.indent != '':
            self.indent = self.indent[n:]

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
    def __init__(self, prefix, code):
        self.prefix = prefix
        self.code = code
        self.source = []

    def add_source_lines(self, lines):
        self.source.extend(lines)

    def with_steps(self):
        for i in range(0, len(self.source)):
            tokens = self.source[i].line().split()
            if (tokens[0] == '+') or (len(tokens) > 2 and tokens[1] == "!!"):
                return True
        return False

    def save_dasm(self, step_switch: bool, f):
        has_steps = self.with_steps()
        if has_steps:
            if step_switch:
                print("\t\tswitch (u8(m_ref))", file=f)
                print("\t\t{", file=f)
                print("\t\tcase 0x00:", file=f)
            else:
                print("\t\t//case 0x00:", file=f)
        step = 0
        for i in range(0, len(self.source)):
            il = self.source[i]
            if not has_steps or not step_switch:
                il.strip_indent(1)
            line = il.line()
            tokens = line.split()
            last_line = i + 1 == len(self.source)

            if tokens[0] == '+':
                il.print("m_icount -= %s;" % (" ".join(tokens[1:])), f)
                step += 1
                il.print("if (m_icount <= 0) {", f)
                if not last_line:
                    to_step = hex(256 + step)[3:]
                    il.print("	m_ref = 0x%s%s%s;" % (self.prefix, self.code, to_step), f)
                    il.print("	return;", f)
                    il.print("}", f)
                    if step_switch:
                        il.print("[[fallthrough]];", f)
                        print("\t\tcase 0x%s:" % (to_step), file=f)
                    else:
                        print("\t\t//case 0x%s:" % (to_step), file=f)
                else:
                    il.print("	m_ref = 0xffff00;", f)
                    il.print("	return;", f)
                    il.print("}", f)
            elif (len(tokens) > 2 and tokens[1] == "!!"):
                step += 1;
                if step_switch:
                    il.print("[[fallthrough]];", f)
                    print("\t\tcase 0x%s:" % (hex(256 + step)[3:]), file=f)
                else:
                    print("\t\t//case 0x%s:" % (hex(256 + step)[3:]), file=f)
                il.print("%s" % " ".join(tokens[2:]), f)
                il.print("m_icount -= %s;" % (tokens[0]), f)
                il.print("if (m_icount <= 0) {", f)
                il.print("	if (access_to_be_redone()) {", f)
                il.print("		m_icount += %s;" % (tokens[0]), f)
                il.print("		m_ref = 0x%s%s%s;" % (self.prefix, self.code, hex(256 + step)[3:]), f)
                il.print("	} else {", f)
                if not last_line:
                    step += 1
                    to_step = hex(256 + step)[3:]
                    il.print("		m_ref = 0x%s%s%s;" % (self.prefix, self.code, to_step), f)
                    il.print("	}", f)
                    il.print("	return;", f)
                    il.print("}", f)
                    if step_switch:
                        il.print("[[fallthrough]];", f)
                        print("\t\tcase 0x%s:" % (to_step), file=f)
                    else:
                        print("\t\t//case 0x%s:" % (to_step), file=f)
                else:
                    il.print("		m_ref = 0xffff00;", f)
                    il.print("	}", f)
                    il.print("	return;", f)
                    il.print("}", f)
            else:
                il.print("%s" % line, f)
        if has_steps and step_switch:
            print("\t\t}", file=f)

class Macro:
    def __init__(self, name, arg_names = None):
        self.name = name
        self.source = []
        self.arg_names = arg_names

    def apply(self, args):
        if self.arg_names is not None:
            src = self.source
            for i, arg in enumerate(args.split(",")):
                src = [ r.replace(self.arg_names[i], arg) for r in src ]
            return src
        else:
            return self.source

    def add_source_lines(self, lines):
        self.source.extend(lines)

class OpcodeList:
    def __init__(self, gen, fname):
        self.gen = gen
        self.opcode_info = {} # prefix -> [Opcode]
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
                    arg_names = None
                    if len(tokens) > 2:
                        arg_names = tokens[2:]
                    nnames = tokens[1].split(":")
                    if len(nnames) == 2:
                        inf = Macro(nnames[1], arg_names)
                        if nnames[0] == self.gen:
                            self.macros[nnames[1]] = inf
                    else:
                        inf = Macro(nnames[0], arg_names)
                        if None == self.gen:
                            if nnames[0] in self.macros:
                                sys.stderr.write("Replacing macro: %s\n" % nnames[0])
                            self.macros[nnames[0]] = inf
                        else:
                            if not nnames[0] in self.macros:
                                self.macros[nnames[0]] = inf
                else:
                    ntokens = tokens[0].split(":")
                    gen = None if len(ntokens) == 1 else ntokens[0]
                    prefix = ntokens[0][:2] if len(ntokens) == 1 else ntokens[1][:2]
                    opcode = ntokens[0][2:] if len(ntokens) == 1 else ntokens[1][2:]
                    if self.opcode_info.get(prefix) is None:
                        self.opcode_info[prefix] = []
                    opcodes = self.opcode_info[prefix]

                    if None == gen:
                        inf = Opcode(prefix, opcode)
                        opcodes.append(inf)
                    elif gen == self.gen:
                        # Replace for ext generator
                        found = False
                        found_index = 0
                        for i in range(len(opcodes)):
                            if opcodes[i].code == opcode:
                                found = True
                                found_index = i
                        if found:
                            inf = Opcode(prefix, opcode)
                            opcodes[found_index] = inf
                        else:
                            sys.stderr.write("[%s] Cannot find opcode: %s%s\n" % (gen, prefix, opcode))
                            sys.exit(1)
                    else:
                        inf = Opcode(prefix, '/dev/null')

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
            if line_toc[0].startswith('@'):
                name = line_toc[0][1:]
                args = None
                if len(line_toc) > 1:
                    args = " ".join(line_toc[1:])
                if name in self.macros:
                    macro = self.macros[name]
                    ([out.extend(self.pre_process(il)) for il in macro.apply(args)])
                else:
                    sys.stderr.write("Macro not found %s\n" % name)
                    out.append(iline.with_str("... %s" % name))
            else:
                out.append(iline.with_str(line))
        return out

    def switch_prefix(self, prefixes, reenter: bool, f):
        prefix_switch = len(prefixes) > 1

        if prefix_switch:
            print("switch (u8(m_ref >> 16)) // prefix", file=f)
            print("{", file=f)

        for prefix in sorted(prefixes):
            is_rop = prefix == 'ff'
            opc_switch = not is_rop
            if prefix_switch:
                print("case 0x%s:" % (prefix), file=f)
                print("{", file=f)
            if opc_switch:
                print("\tswitch (u8(m_ref >> 8)) // opcode", file=f)
                print("\t{", file=f)
            for opc in self.opcode_info[prefix]:
                # reenter loop only process steps > 0
                if not reenter or opc.with_steps():
                    if opc_switch:
                        print("\tcase 0x%s:" % (opc.code), file=f)
                    opc.save_dasm(step_switch=reenter, f=f)
                    print("\t\tcontinue;", file=f)
                    print("", file=f)
            if opc_switch:
                print("\t}", file=f)

            if prefix_switch:
                print("} break; // prefix: %s" % (prefix), file=f)
                print("", file=f)
        if prefix_switch:
            print("} // switch prefix", file=f)

    def save_exec(self, f):
        print("if (m_wait_state)", file=f)
        print("{", file=f)
        print("\tm_icount = 0; // stalled", file=f)
        print("\treturn;", file=f)
        print("}", file=f)
        print("", file=f)
        print("const bool nomemrq_en = !m_nomreq_cb.isunset();", file=f)
        print("[[maybe_unused]] const bool refresh_en = !m_refresh_cb.isunset();", file=f)
        print("", file=f)
        print("bool interrupted = true;", file=f)
        print("while (u8(m_ref) != 0x00) {", file=f)
        print("// slow re-enter", file=f)
        print("\t// workaround to simulate main loop behavior where continue statement relays on having it set after", file=f)
        print("\tif (!interrupted) {", file=f)
        print("\t\tm_ref = 0xffff00;", file=f)
        print("\t\tcontinue;", file=f)
        print("\t}", file=f)
        print("\tinterrupted = false;", file=f)
        print("", file=f)
        self.switch_prefix(self.opcode_info.keys(), reenter=True, f=f)
        print("", file=f)
        print('assert((void("switch statement above must cover all possible cases!"), false));', file=f)
        print("} // end: slow", file=f)
        print("if (m_ref != 0xffff00) goto process;", file=f)
        print("", file=f)
        print("while (true) { // fast process", file=f)
        print("\t\t// rop: unwrapped ff prefix", file=f)
        self.switch_prefix(['ff'], reenter=False, f=f)
        print("", file=f)
        print("process:", file=f)
        self.switch_prefix(self.opcode_info.keys() - ['ff'], reenter=False, f=f)
        print("} // end: fast", file=f)
        print('assert((void("unreachable!"), false));', file=f)

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
