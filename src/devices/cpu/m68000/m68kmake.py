#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert, Karl Stenerud

# To regerenerate m68kops.* run m68kmame.py m68k_in.lst m68kops.h m68kops.cpp

from __future__ import print_function
import sys
import copy

CPU_000 = 0
CPU_010 = 1
CPU_020 = 2
CPU_030 = 3
CPU_040 = 4
CPU_FSCPU32 = 5
CPU_COLDFIRE = 6
CPU_COUNT = 7

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
    'none' : [[ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0], [ 0,  0,  0]],
    'ai'   : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pi'   : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pi7'  : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pd'   : [[ 0,  6, 10], [ 0,  6, 10], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'pd7'  : [[ 0,  6, 10], [ 0,  6, 10], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'di'   : [[ 0,  8, 12], [ 0,  8, 12], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'ix'   : [[ 0, 10, 14], [ 0, 10, 14], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  0,  0]],
    'aw'   : [[ 0,  8, 12], [ 0,  8, 12], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'al'   : [[ 0, 12, 16], [ 0, 12, 16], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  4,  4], [ 0,  0,  0]],
    'pcdi' : [[ 0,  8, 12], [ 0,  8, 12], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  5,  5], [ 0,  0,  0]],
    'pcix' : [[ 0, 10, 14], [ 0, 10, 14], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  7,  7], [ 0,  0,  0]],
    'i'    : [[ 0,  4,  8], [ 0,  4,  8], [ 0,  2,  4], [ 0,  2,  4], [ 0,  2,  4], [ 0,  2,  4], [ 0,  0,  0]],
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
        self.name = entries[0]
        self.size = int(entries[1])
        self.spec_proc = entries[2]
        self.spec_ea = entries[3]
        self.key = '%s %d %s %s' % (self.name, self.size, self.spec_proc, self.spec_ea)
        self.op_mask = 0
        self.op_value = 0
        for bit in range(0, 16):
            if entries[4][15-bit] != '.':
                self.op_mask |= 1 << bit
            if entries[4][15-bit] == '1':
                self.op_value |= 1 << bit
        self.ea_allowed = entries[5]
        self.cpu_modes = [None] * CPU_COUNT
        for i in range(0, CPU_COUNT):
            self.cpu_modes[i] = entries[6+i]
        self.cycles = [None] * CPU_COUNT
        for i in range(0, CPU_COUNT):
            if entries[6+CPU_COUNT+i] != '.':
                self.cycles[i] = int(entries[6+CPU_COUNT+i])
        
class OpcodeGenericHandler:
    def __init__(self, block, opcodes):
        parts = block.split('\n', 1)
        head = parts[0]
        head = head[head.find('(')+1:head.rfind(')')].split(',')
        self.name = head[0].split()[0]
        self.size = int(head[1].split()[0])
        self.spec_proc = head[2].split()[0]
        self.spec_ea = head[3].split()[0]
        self.body = parts[1]
        key = '%s %d %s %s' % (self.name, self.size, self.spec_proc, self.spec_ea)
        self.opcode = opcodes[key]

    def generate(self, handlers):
        if self.name == 'bcc' or self.name == 'scc' or self.name == 'dbcc' or self.name == 'trapcc':
            self.cc_variants(handlers)
        else:
            self.ea_variants(handlers)

    def ea_variants(self, handlers):
        ea_allowed = self.opcode.ea_allowed
        if ea_allowed == '..........':
            handlers.append(OpcodeHandler(self, 'none'))
            return
        if ea_allowed[0] == 'A':
            handlers.append(OpcodeHandler(self, 'ai'))
        if ea_allowed[1] == '+':
            handlers.append(OpcodeHandler(self, 'pi'))
            if self.opcode.size == 8:
                handlers.append(OpcodeHandler(self, 'pi7'))
        if ea_allowed[2] == '-':
            handlers.append(OpcodeHandler(self, 'pd'))
            if self.opcode.size == 8:
                handlers.append(OpcodeHandler(self, 'pd7'))
        if ea_allowed[3] == 'D':
            handlers.append(OpcodeHandler(self, 'di'))
        if ea_allowed[4] == 'X':
            handlers.append(OpcodeHandler(self, 'ix'))
        if ea_allowed[5] == 'W':
            handlers.append(OpcodeHandler(self, 'aw'))
        if ea_allowed[6] == 'L':
            handlers.append(OpcodeHandler(self, 'al'))
        if ea_allowed[7] == 'd':
            handlers.append(OpcodeHandler(self, 'pcdi'))
        if ea_allowed[8] == 'x':
            handlers.append(OpcodeHandler(self, 'pcix'))
        if ea_allowed[9] == 'I':
            handlers.append(OpcodeHandler(self, 'i'))

    def cc_variants(self, handlers):
        bname = self.name[:-2]
        for cc in range(2, 16):
            opc1 = copy.copy(self.opcode)
            opc = copy.copy(self)
            opc.opcode = opc1
            opc1.name = bname + cc_table_dn[cc]
            opc.body = self.body.replace('M68KMAKE_CC', 'COND_%s()' % cc_table_up[cc]).replace('M68KMAKE_NOT_CC', 'COND_NOT_%s()' % cc_table_up[cc])
            opc1.op_mask |= 0x0f00
            opc1.op_value = (opc1.op_value & 0xf0ff) | (cc << 8)
            opc.ea_variants(handlers)

class OpcodeHandler:
    def __init__(self, ogh, ea_mode):
        op = ogh.opcode
        self.cycles = [None]*CPU_COUNT
        size_order = 0 if op.size == 0 else 1 if op.size == 8 or op.size == 16 else 2
        for i in range(0, CPU_COUNT):
            if op.cycles[i] == None:
                continue
            if i == CPU_010 and op.name == 'moves':
                self.cycles[i] = op.cycles[i] + moves_cycle_table[ea_mode][size_order]
            elif i == CPU_010 and op.name == 'clr':
                self.cycles[i] = op.cycles[i] + clr_cycle_table[ea_mode][size_order]
            elif i == CPU_000 and (ea_mode == 'i' or ea_mode == 'none') and op.size == 32 and ((op.spec_proc == 'er' and (op.name == 'add' or op.name == 'and' or op.name == 'or' or op.name == 'sub')) or op.name == 'adda' or op.name == 'suba'):
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
        self.spec_ea = ea_mode if ea_mode != 'none' and op.spec_ea == '.' else op.spec_ea
        self.op_value = op.op_value | ea_info_table[ea_mode][2]
        self.op_mask  = op.op_mask  | ea_info_table[ea_mode][1]
        self.function_name = 'm68k_op_' + op.name
        if op.size > 0:
            self.function_name += '_%d' % op.size
        if op.spec_proc != '.':
            self.function_name += '_' + op.spec_proc
        if self.spec_ea != '.':
            self.function_name += '_' + self.spec_ea
        self.bits = 0
        for i in range(0, 16):
            if (self.op_mask & (1 << i)) != 0:
                self.bits += 1
        if ea_mode != 'none':
            n = ea_info_table[ea_mode][0]
            body = ogh.body
            body = body.replace('M68KMAKE_GET_EA_AY_8', 'EA_%s_8()' % n)
            body = body.replace('M68KMAKE_GET_EA_AY_16', 'EA_%s_16()' % n)
            body = body.replace('M68KMAKE_GET_EA_AY_32', 'EA_%s_32()' % n)
            body = body.replace('M68KMAKE_GET_OPER_AY_8', 'OPER_%s_8()' % n)
            body = body.replace('M68KMAKE_GET_OPER_AY_16', 'OPER_%s_16()' % n)
            body = body.replace('M68KMAKE_GET_OPER_AY_32', 'OPER_%s_32()' % n)
            self.body = body
        else:
            self.body = ogh.body

class Info:
    def __init__(self, path):
        try:
            f = open(path, 'r')
        except Exception:
            err = sys.exc_info()[1]
            sys.stderr.write('cannot read file %s [%s]\n' % (path, err))
            sys.exit(1)

        itext = f.read()
        blocks = itext.split('X'*79 + '\n')
        # first block skipped
        for i in range(1, len(blocks)):
            b = blocks[i]
            if b.startswith('M68KMAKE_PROTOTYPE_HEADER\n\n'):
                self.prototype_header = b[27:]
            elif b.startswith('M68KMAKE_PROTOTYPE_FOOTER\n\n'):
                self.prototype_footer = b[27:]
            elif b.startswith('M68KMAKE_TABLE_HEADER\n\n'):
                self.table_header = b[23:]
            elif b.startswith('M68KMAKE_TABLE_FOOTER\n\n'):
                self.table_footer = b[23:]
            elif b.startswith('M68KMAKE_OPCODE_HANDLER_HEADER\n\n'):
                self.opcode_handler_header = b[32:]
            elif b.startswith('M68KMAKE_OPCODE_HANDLER_FOOTER\n\n'):
                self.opcode_handler_footer = b[32:]
            elif b.startswith('M68KMAKE_TABLE_BODY\n'):
                self.handle_table_body(b[20:])
            elif b.startswith('M68KMAKE_OPCODE_HANDLER_BODY\n\n'):
                self.handle_opcode_handler_body(b[30:])
            elif b.startswith('M68KMAKE_END\n\n'):
                pass

    def handle_table_body(self, b):
        s1 = b[b.index('M68KMAKE_TABLE_START\n')+21:]
        lines = s1.split('\n')
        self.opcodes = {}
        for line in lines:
            if line == '':
                continue
            opc = Opcode(line)
            self.opcodes[opc.key] = opc
            
    def handle_opcode_handler_body(self, b):
        blocks = b.split('M68KMAKE_OP')
        self.opcode_handlers = []
        for block in blocks:
            if block != '':
                opc = OpcodeGenericHandler(block, self.opcodes)
                opc.generate(self.opcode_handlers)

    def save_header(self, f):
        f.write("// Generated source, edits will be lost.  Run m68kmake.py instead\n")
        f.write("\n")
        f.write(self.prototype_header)
        for h in self.opcode_handlers:
            f.write('void %s();\n' % h.function_name)
        f.write(self.prototype_footer)

    def save_source(self, f):
        f.write("// Generated source, edits will be lost.  Run m68kmake.py instead\n")
        f.write("\n")
        f.write("#include \"emu.h\"\n")
        f.write("#include \"m68000.h\"\n")
        f.write("\n")
        f.write(self.opcode_handler_header)
        for h in self.opcode_handlers:
            f.write('void m68000_base_device::%s()\n%s' % (h.function_name, h.body))
        f.write(self.opcode_handler_footer)
        f.write(self.table_header)
        order = list(range(len(self.opcode_handlers)))
        order.sort(key = lambda id: "%02d %04x %04x" % (self.opcode_handlers[id].bits, self.opcode_handlers[id].op_mask, self.opcode_handlers[id].op_value))
        for id in order:
            oh = self.opcode_handlers[id]
            f.write("\t{&m68000_base_device::%s, 0x%04x, 0x%04x, {" % (oh.function_name, oh.op_mask, oh.op_value))
            for i in range(0, CPU_COUNT):
                if i != 0:
                    f.write(", ")
                f.write("%3d" % (255 if oh.cycles[i] == None else oh.cycles[i]))
            f.write("}},\n")
        f.write(self.table_footer)

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
