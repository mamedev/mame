#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert, Karl Stenerud

# To regerenerate m68kops.* run m68kmake.py m68k_in.lst m68kops.h m68kops.cpp

from __future__ import print_function
import sys
import copy

CPU_000 = 0
CPU_070 = 1
CPU_010 = 2
CPU_020 = 3
CPU_030 = 4
CPU_040 = 5
CPU_FSCPU32 = 6
CPU_COLDFIRE = 7
CPU_COUNT = 8

cpu_names = '071234fc'

cc_table_up = [ "T", "F", "HI", "LS", "CC", "CS", "NE", "EQ", "VC", "VS", "PL", "MI", "GE", "LT", "GT", "LE" ]
cc_table_dn = [ "t", "f", "hi", "ls", "cc", "cs", "ne", "eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le" ]


# Probably incorrect starting with the 030 and further
#
# The C code was going out-of-bounds, or more precisely
# out-of-initialization, on the Coldfire, explaning the
# zeroes.
#
#                   000           010           020           030           040        FSCPU32      Coldfire
ea_cycle_table = {
    'none' : [[ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0]],
    'ai'   : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pi'   : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pi7'  : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pd'   : [[ 0,  6, 10], [ 0,  6, 10], [ 0,  6, 10], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'pd7'  : [[ 0,  6, 10], [ 0,  6, 10], [ 0,  6, 10], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'di'   : [[ 0,  8, 12], [ 0,  8, 12], [ 0,  8, 12], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'ix'   : [[ 0, 10, 14], [ 0, 10, 14], [ 0, 10, 14], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  0,  0]],
    'aw'   : [[ 0,  8, 12], [ 0,  8, 12], [ 0,  8, 12], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'al'   : [[ 0, 12, 16], [ 0, 12, 16], [ 0, 12, 16], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pcdi' : [[ 0,  8, 12], [ 0,  8, 12], [ 0,  8, 12], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'pcix' : [[ 0, 10, 14], [ 0, 10, 14], [ 0, 10, 14], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  0,  0]],
    'i'    : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  8], [ 0,  2,  4], [ 0,  2,  4], [ 0,  2,  4], [ 0,  2,  4], [ 0,  0,  0]],
}

jmp_jsr_cycle_table = { 'none': 0, 'ai': 4, 'pi': 0, 'pi7': 0, 'pd': 0, 'pd7': 0, 'di': 6, 'ix': 10, 'aw': 6, 'al': 8, 'pcdi': 6, 'pcix': 10, 'i': 0 }
lea_cycle_table = { 'none': 0, 'ai': 4, 'pi': 0, 'pi7': 0, 'pd': 0, 'pd7': 0, 'di': 8, 'ix': 12, 'aw': 8, 'al': 12, 'pcdi': 8, 'pcix': 12, 'i': 0 }
pea_cycle_table = { 'none': 0, 'ai': 6, 'pi': 0, 'pi7': 0, 'pd': 0, 'pd7': 0, 'di': 10, 'ix': 14, 'aw': 10, 'al': 14, 'pcdi': 10, 'pcix': 14, 'i': 0 }
movem_cycle_table = { 'none': 0, 'ai': 0, 'pi': 0, 'pi7': 0, 'pd': 0, 'pd7': 0, 'di': 4, 'ix': 6, 'aw': 4, 'al': 8, 'pcdi': 0, 'pcix': 0, 'i': 0 }

moves_cycle_table = {
    'none' : [ 0,  0,  0],
    'ai'   : [ 0,  4,  6],
    'pi'   : [ 0,  4,  6],
    'pi7'  : [ 0,  4,  6],
    'pd'   : [ 0,  6, 12],
    'pd7'  : [ 0,  6, 12],
    'di'   : [ 0, 12, 16],
    'ix'   : [ 0, 16, 20],
    'aw'   : [ 0, 12, 16],
    'al'   : [ 0, 16, 20],
    'pcdi' : [ 0,  0,  0],
    'pcix' : [ 0,  0,  0],
    'i'    : [ 0,  0,  0],
}

clr_cycle_table = {
    'none' : [ 0,  0,  0],
    'ai'   : [ 0,  4,  6],
    'pi'   : [ 0,  4,  6],
    'pi7'  : [ 0,  4,  6],
    'pd'   : [ 0,  6,  8],
    'pd7'  : [ 0,  6,  8],
    'di'   : [ 0,  8, 10],
    'ix'   : [ 0, 10, 14],
    'aw'   : [ 0,  8, 10],
    'al'   : [ 0, 10, 14],
    'pcdi' : [ 0,  0,  0],
    'pcix' : [ 0,  0,  0],
    'i'    : [ 0,  0,  0],
}

ea_info_table = {
    'none' : [ '',       0x00, 0x00 ],
    'ai'   : [ 'AY_AI',  0x38, 0x10 ],
    'pi'   : [ 'AY_PI',  0x38, 0x18 ],
    'pi7'  : [ 'A7_PI',  0x3f, 0x1f ],
    'pd'   : [ 'AY_PD',  0x38, 0x20 ],
    'pd7'  : [ 'A7_PD',  0x3f, 0x27 ],
    'di'   : [ 'AY_DI',  0x38, 0x28 ],
    'ix'   : [ 'AY_IX',  0x38, 0x30 ],
    'aw'   : [ 'AW',     0x3f, 0x38 ],
    'al'   : [ 'AL',     0x3f, 0x39 ],
    'pcdi' : [ 'PCDI',   0x3f, 0x3a ],
    'pcix' : [ 'PCIX',   0x3f, 0x3b ],
    'i'    : [ 'I',      0x3f, 0x3c ],
}



class Opcode:
    def __init__(self, line):
        entries = line.split()
        self.op_value = int(entries[0], 16)
        self.op_mask = int(entries[1], 16)
        self.name = entries[2]
        self.size = entries[3]
        self.ea_allowed = entries[4]
        self.priv = [None] * CPU_COUNT
        self.cycles = [None] * CPU_COUNT
        for i in range(5, len(entries)):
            parts = entries[i].split(':')
            ci = parts[1]
            priv = ci[len(ci)-1:] == 'p'
            if priv:
                ci = ci[:len(ci)-1]
            cycles = int(ci)
            for c in parts[0]:
                cpu = cpu_names.index(c)
                self.cycles[cpu] = cycles
                self.priv[cpu] = priv
        self.body = ''

    def append(self, line):
        self.body += line + '\n'

    def generate(self, handlers):
        if self.name == 'bcc' or self.name == 'scc' or self.name == 'dbcc' or self.name == 'trapcc':
            self.cc_variants(handlers)
        else:
            self.ea_variants(handlers)

    def ea_variants(self, handlers):
        ea_allowed = self.ea_allowed
        if ea_allowed == '.':
            handlers.append(OpcodeHandler(self, 'none'))
            return
        if 'A' in ea_allowed[0]:
            handlers.append(OpcodeHandler(self, 'ai'))
        if '+' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'pi'))
            if self.size == 'b':
                handlers.append(OpcodeHandler(self, 'pi7'))
        if '-' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'pd'))
            if self.size == 'b':
                handlers.append(OpcodeHandler(self, 'pd7'))
        if 'D' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'di'))
        if 'X' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'ix'))
        if 'W' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'aw'))
        if 'L' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'al'))
        if 'd' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'pcdi'))
        if 'x' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'pcix'))
        if 'I' in ea_allowed:
            handlers.append(OpcodeHandler(self, 'i'))

    def cc_variants(self, handlers):
        bname = self.name[:-2]
        for cc in range(2, 16):
            op = copy.copy(self)
            op.name = bname + cc_table_dn[cc]
            op.body = self.body.replace('M68KMAKE_CC', 'COND_%s()' % cc_table_up[cc]).replace('M68KMAKE_NOT_CC', 'COND_NOT_%s()' % cc_table_up[cc])
            op.op_mask |= 0x0f00
            op.op_value = (op.op_value & 0xf0ff) | (cc << 8)
            op.ea_variants(handlers)

class OpcodeHandler:
    def __init__(self, op, ea_mode):
        self.cycles = [None]*CPU_COUNT
        size_order = 0 if op.size == '.' else 1 if op.size == 'b' or op.size == 'w' else 2
        for i in range(0, CPU_COUNT):
            if op.cycles[i] == None:
                continue
            if i == CPU_010 and op.name == 'moves':
                self.cycles[i] = op.cycles[i] + moves_cycle_table[ea_mode][size_order]
            elif i == CPU_010 and op.name == 'clr':
                self.cycles[i] = op.cycles[i] + clr_cycle_table[ea_mode][size_order]
            elif (i == CPU_000 or i == CPU_070)  and (ea_mode == 'i' or ea_mode == 'none') and op.size == 'l' and ((op.cycles[i] == 6 and (op.name == 'add' or op.name == 'and' or op.name == 'or' or op.name == 'sub')) or op.name == 'adda' or op.name == 'suba'):
                self.cycles[i] = op.cycles[i] + ea_cycle_table[ea_mode][i][size_order] + 2
            elif i < CPU_020 and (op.name == 'jmp' or op.name == 'jsr'):
                self.cycles[i] = op.cycles[i] + jmp_jsr_cycle_table[ea_mode]
            elif i < CPU_020 and op.name == 'lea':
                self.cycles[i] = op.cycles[i] + lea_cycle_table[ea_mode]
            elif i < CPU_020 and op.name == 'pea':
                self.cycles[i] = op.cycles[i] + pea_cycle_table[ea_mode]
            elif i < CPU_020 and op.name == 'movem':
                self.cycles[i] = op.cycles[i] + movem_cycle_table[ea_mode]
            else:
                self.cycles[i] = op.cycles[i] + ea_cycle_table[ea_mode][i][size_order]
        self.op_value = op.op_value | ea_info_table[ea_mode][2]
        self.op_mask  = op.op_mask  | ea_info_table[ea_mode][1]
        self.function_name = 'x%04x_%s%s%s_' % (self.op_value, op.name, '' if op.size == '.' else '_' + op.size, '' if ea_mode == 'none' else '_' + ea_mode)
        for i in range(0, CPU_COUNT):
            if self.cycles[i] != None:
                self.function_name += cpu_names[i]
        self.bits = 0
        for i in range(0, 16):
            if (self.op_mask & (1 << i)) != 0:
                self.bits += 1
        if ea_mode != 'none':
            n = ea_info_table[ea_mode][0]
            body = op.body
            body = body.replace('M68KMAKE_GET_EA_AY_8', 'EA_%s_8()' % n)
            body = body.replace('M68KMAKE_GET_EA_AY_16', 'EA_%s_16()' % n)
            body = body.replace('M68KMAKE_GET_EA_AY_32', 'EA_%s_32()' % n)
            body = body.replace('M68KMAKE_GET_OPER_AY_8', 'OPER_%s_8()' % n)
            body = body.replace('M68KMAKE_GET_OPER_AY_16', 'OPER_%s_16()' % n)
            body = body.replace('M68KMAKE_GET_OPER_AY_32', 'OPER_%s_32()' % n)
            self.body = body
        else:
            self.body = op.body

class Info:
    def __init__(self, path):
        try:
            f = open(path, 'r')
        except Exception:
            err = sys.exc_info()[1]
            sys.stderr.write('cannot read file %s [%s]\n' % (path, err))
            sys.exit(1)

        self.opcodes = []
        cur_opcode = None
        for line in f:
            line = line.rstrip()
            if line == '' or line[0] == ' ' or line[0] == '\t':
                if cur_opcode != None:
                    cur_opcode.append(line)
            elif line[0] != '#':
                cur_opcode = Opcode(line)
                self.opcodes.append(cur_opcode)

        self.opcode_handlers = []
        for op in self.opcodes:
            op.generate(self.opcode_handlers)

    def save_header(self, f):
        f.write("// Generated source, edits will be lost.  Run m68kmake.py instead\n")
        f.write("\n")
        for h in self.opcode_handlers:
            f.write('void %s();\n' % h.function_name)

    def save_source(self, f):
        f.write("// Generated source, edits will be lost.  Run m68kmake.py instead\n")
        f.write("\n")
        f.write("#include \"emu.h\"\n")
        f.write("#include \"m68000.h\"\n")
        f.write("\n")
        for h in self.opcode_handlers:
            f.write('void m68000_base_device::%s()\n{\n%s}\n' % (h.function_name, h.body))

        order = list(range(len(self.opcode_handlers)))
        order.sort(key = lambda id: "%02d %04x %04x" % (self.opcode_handlers[id].bits, self.opcode_handlers[id].op_mask, self.opcode_handlers[id].op_value))

        illegal_id = 0
        nid = 0
        f.write("const m68000_base_device::opcode_handler_ptr m68000_base_device::m68k_handler_table[] =\n{\n\n")
        for id in order:
            oh = self.opcode_handlers[id]
            f.write("\t&m68000_base_device::%s,\n" % oh.function_name)
            if oh.function_name == 'x4afc_illegal_' + cpu_names:
                illegal_id = nid
            nid += 1
        f.write("};\n\n")
        f.write("const u16 m68000_base_device::m68k_state_illegal = %d;\n\n" % illegal_id)
        f.write("const m68000_base_device::opcode_handler_struct m68000_base_device::m68k_opcode_table[] =\n{\n\n")
        for id in order:
            oh = self.opcode_handlers[id]
            f.write("\t{ 0x%04x, 0x%04x, {" % (oh.op_value, oh.op_mask))
            for i in range(0, CPU_COUNT):
                if i != 0:
                    f.write(", ")
                f.write("%3d" % (255 if oh.cycles[i] == None else oh.cycles[i]))
            f.write("}},\n")
        f.write("\t{ 0, 0, {0, 0, 0, 0, 0}}\n};\n")

def main(argv):
    if len(argv) != 4:
        print('Usage:\n%s m68k_in.lst m68kops.h m68kops.cpp' % argv[0])
        return 1

    info = Info(argv[1])
    try:
        fh = open(argv[2], 'w')
    except Exception:
        err = sys.exc_info()[1]
        sys.stderr.write('cannot write file %s [%s]\n' % (argv[2], err))
        sys.exit(1)
    try:
        fs = open(argv[3], 'w')
    except Exception:
        err = sys.exc_info()[1]
        sys.stderr.write('cannot write file %s [%s]\n' % (argv[3], err))
        sys.exit(1)
    info.save_header(fh)
    info.save_source(fs)

if __name__ == '__main__':
    sys.exit(main(sys.argv))
