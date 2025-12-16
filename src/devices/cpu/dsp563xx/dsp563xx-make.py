#!/usr/bin/python3

import sys
from enum import IntEnum, auto

class IType(IntEnum):
    ipar = 0
    npar = 1
    move = 2

class FType(IntEnum):
    fixed = 0
    vars = 1
    keyed_vars = 1
    keyed = 2
    none = 3

class SourceType:
    interp_pre = 0
    interp_post = 1
    drc_pre = 2
    drc_post = 3

class SlotMode:
    read = 0
    write = 1
    memory = 2

class SlotChange:
    none = 0
    sel1 = 1
    bus24 = 2
    bus8 = 3
    bus48 = 4

functions = {}

class Function:
    def __init__(self, name, pcount, values):
        global functions
        functions[name] = self
        self.name = name
        self.has_ex = name[:2] == 'ex' or name == 'eam1a' or name == 'eam1i'
        self.pcount = pcount
        self.values = values
        self.ivals, self.icount = self.preexpand()
        self.svals = self.preexpand_slotted()
        self.bcount = None
        if self.icount:
            for bcount in range(16):
                if (1 << bcount) >= self.icount:
                    self.bcount = bcount
                    break

    def get_ftype(self):
        if self.pcount < 0:
            return FType.keyed
        if self.values[0] == 'val':
            return FType.fixed
        if self.values[0] == 'single' or self.values[0] == 'split':
            return FType.vars
        if self.values[0] == 'single-alt' or self.values[0] == 'split-alt':
            return FType.keyed_vars
        return FType.none

    def gen_vals(self, values):
        r = []
        ii = 0
        for va in values:
            if va == None:
                ii += 1
            elif 'rh' in va or 'rl' in va:
                for j in range(4):
                    r.append(ii)
                    ii += 1
            elif va == 'r' or va == 'n' or va == 'm' or va[:2] == '(r' or va == '-(r)':
                for j in range(8):
                    r.append(ii)
                    ii += 1
            else:
                r.append(ii)
                ii += 1
        return r, ii

    def gen_vals_slotted(self, values):
        r = []
        ii = 0
        for va in values:
            if va == None:
                ii += 1
            elif 'rh' in va or 'rl' in va:
                rr = []
                for j in range(4):
                    rr.append(ii)
                    ii += 1
                r.append([va, rr])
            elif va == 'r' or va == 'n' or va == 'm' or va[:2] == '(r' or va == '-(r)':
                rr = []
                for j in range(8):
                    rr.append(ii)
                    ii += 1
                r.append([va, rr])
            else:
                r.append([va, [ii]])
                ii += 1
        return r

    def preexpand(self):
        if self.values[0] == 'range':
            return list(range(self.values[1])), self.values[1]

        elif self.values[0] == 'split-range':
            return list(range(self.values[2])), self.values[2]

        elif self.values[0] == 'single':
            return self.gen_vals(self.values[1])

        elif self.values[0] == 'single-alt':
            return self.gen_vals(self.values[1][:len(self.values[1])//2])

        elif self.values[0] == 'split':
            return self.gen_vals(self.values[2])

        elif self.values[0] == 'split-alt':
            return self.gen_vals(self.values[2][:len(self.values[2])//2])

        elif self.values[0] == 'val':
            return [self.values[1]], 0

        elif self.values[0] == 'pass':
            return None, None

    def preexpand_slotted(self):
        if self.values[0] == 'range' or self.values[0] == 'split-range':
            return None

        elif self.values[0] == 'single':
            return [self.gen_vals_slotted(self.values[1])]

        elif self.values[0] == 'single-alt':
            return [self.gen_vals_slotted(self.values[1][:len(self.values[1])//2]), self.gen_vals_slotted(self.values[1][len(self.values[1])//2:])]

        elif self.values[0] == 'split':
            return [self.gen_vals_slotted(self.values[2])]

        elif self.values[0] == 'split-alt':
            return [self.gen_vals_slotted(self.values[2][:len(self.values[2])//2]), self.gen_vals_slotted(self.values[2][len(self.values[2])//2:])]

        elif self.values[0] == 'val':
            return None

        elif self.values[0] == 'pass':
            return None

    def apply_nosplit(self, idxs, vals, bit):
        r = []
        for v in idxs:
            for i in vals:
                r.append(v | (i << bit))
        return r

    def apply_split(self, idxs, vals, bit1, bit2, m1):
        m2 = (1 << m1) - 1
        r = []
        for i in vals:
            i1 = i >> m1
            i2 = i & m2
            for v in idxs:
                r.append(v | (i1 << bit1) | (i2 << bit2))
        return r

    def expand_idxs(self, params, idxs):
        if len(params) != abs(self.pcount):
            print("Incorrect number of parameters on %s call, expected %d, got %d" % (self.name, abs(self.pcount), len(params)))
            sys.exit(1)
        if self.pcount < 0:
            return idxs
        if self.values[0] == 'range' or self.values[0] == 'single' or self.values[0] == 'single-alt' or self.values[0] == 'val':
            return self.apply_nosplit(idxs, self.ivals, params[0])
        elif self.values[0] == 'split-range' or self.values[0] == 'split' or self.values[0] == 'split-alt':
            return self.apply_split(idxs, self.ivals, params[0], params[1], self.values[1])
        elif self.values[0] == 'pass':
            return idxs
        else:
            print("unknown expansion %s" % self.values[0])
            sys.exit(1)

    def get_slot_count(self):
        if self.svals == None or self.pcount < 0:
            return 1
        return len(self.svals[0])

    def get_val_idxs_slotted(self, slot, params, idxs):
        if self.values[0] == 'val' or self.values[0] == 'range':
            return None, self.apply_nosplit(idxs, self.ivals, params[0])
        if self.values[0] == 'split-range':
            return None, self.apply_split(idxs, self.ivals, params[0], params[1], self.values[1])
        if self.svals == None:
            return None, idxs
        variant = 0
        if self.values[0] == 'single-alt' or self.values[0] == 'split-alt':
            variant = (idxs[0] >> params[-1]) & 1
        if self.pcount < 0:
            if self.values[0] == 'single' or self.values[0] == 'single-alt':
                entryid = (idxs[0] >> params[0]) & ((1 << self.bcount) - 1)
            else:
                assert(self.values[0] == 'split' or self.values[0] == 'split-alt')
                entryid = (idxs[0] >> params[0]) & ((1 << self.values[1]) - 1)
                entryid |= ((idxs[0] >> params[1]) & (1 << (self.bcount - self.values[1]) - 1)) << self.values[1]
            return self.svals[variant][entryid][0], idxs
        else:
            entry = self.svals[variant][slot]
            if self.values[0] == 'single' or self.values[0] == 'single-alt':
                return entry[0], self.apply_nosplit(idxs, entry[1], params[0])
            else:
                assert(self.values[0] == 'split' or self.values[0] == 'split-alt')
                return entry[0], self.apply_split(idxs, entry[1], params[0], params[1], self.values[1])

    def need_array(self):
        return self.values[0] == "single" or self.values[0] == "single-alt" or self.values[0] == "split" or self.values[0] == "split-alt"

    def gen_array(self, f):
        vl = self.values[2 if self.values[0] == 'split' or self.values[0] == 'split-alt' else 1]
        count = 0
        s = ''
        for e in vl:
            if e == None:
                s += ' nullptr,'
                count += 1
            elif e == 'r' or e == 'n' or e == 'm':
                for i in range(8):
                    s += ' "%s%d",' % (e, i)
                count += 8
            elif e == '(r)':
                for i in range(8):
                    s += ' "(r%d)",' % i
                count += 8
            elif e == '(r)-':
                for i in range(8):
                    s += ' "(r%d)-",' % i
                count += 8
            elif e == '(r)+':
                for i in range(8):
                    s += ' "(r%d)+",' % i
                count += 8
            elif e == '-(r)':
                for i in range(8):
                    s += ' "-(r%d)",' % i
                count += 8
            elif e == '(r)-n':
                for i in range(8):
                    s += ' "(r%d)-n%d",' % (i, i)
                count += 8
            elif e == '(r)+n':
                for i in range(8):
                    s += ' "(r%d)+n%d",' % (i, i)
                count += 8
            elif e == '(r+n)':
                for i in range(8):
                    s += ' "(r%d+n%d)",' % (i, i)
                count += 8
            elif e == '(rh)+n':
                for i in range(4, 8):
                    s += ' "(r%d)+n%d",' % (i, i)
                count += 4
            elif e == '(rh)-':
                for i in range(4, 8):
                    s += ' "(r%d)-",' % i
                count += 4
            elif e == '(rh)+':
                for i in range(4, 8):
                    s += ' "(r%d)+",' % i
                count += 4
            elif e == '(rh)':
                for i in range(4, 8):
                    s += ' "(r%d)",' % i
                count += 4
            elif e == '(rl)+n':
                for i in range(4):
                    s += ' "(r%d)+n%d",' % (i, i)
                count += 4
            elif e == '(rl)-':
                for i in range(4):
                    s += ' "(r%d)-",' % i
                count += 4
            elif e == '(rl)+':
                for i in range(4):
                    s += ' "(r%d)+",' % i
                count += 4
            elif e == '(rl)':
                for i in range(4):
                    s += ' "(r%d)",' % i
                count += 4
            else:
                s += ' "%s",' % e
                count += 1
        print("const char *const dsp563xx_disassembler::ts_%s[%d] = {%s };" % (self.name, count, s[:-1]), file=f)

    def format(self):
        if self.values[0] == "single" or self.values[0] == "single-alt" or self.values[0] == "split" or self.values[0] == "split-alt":
            return '%s'
        if self.values[0] == "range" or self.values[0] == "split-range":
            mode = self.values[3 if self.values[0] == "split-range" else 2]
            if mode == 'imm':
                return '$%%0%dx' % ((self.bcount + 3) // 4)
            elif mode == 'immm':
                return '$%x'
            elif mode == 'imms':
                return '%s$%x'
            elif mode == 'bit' or mode == 'shift':
                return '%d'
            elif mode == 'asap' or mode == 'asaq' or mode == 'pcrel' or mode == 'abs':
                return '$%06x'
        if self.name == 'exabs' or self.name == 'expcrel' or self.name == 'eximm' or self.name == 'exco' or self.name == 'eam1a' or self.name == 'eam1i':
            return '$%06x'
        elif self.name == 'exoff':
            return '%s$%x'
        return '[' + self.name + ']'

    def brange(self, bit, bc):
        s = ''
        for bit in range(bit+bc, bit, -1):
            s += ', %d' % (bit-1)
        return s

    def get_value_expression(self, params):
        if self.values[0] == "single":
            return 'BIT(opcode, %d, %d)' % (params[0], self.bcount)
        elif self.values[0] == "single-alt":
            return 'bitswap<%d>(opcode, %d%s)' % (self.bcount+1, params[1], self.brange(params[0], self.bcount))
        elif self.values[0] == 'split':
            b2 = self.values[1]
            b1 = self.bcount - b2
            return 'bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
        elif self.values[0] == 'split-alt':
            b2 = self.values[1]
            b1 = self.bcount - b2
            return 'bitswap<%d>(opcode, %d%s%s)' % (self.bcount+1, params[2], self.brange(params[0], b1), self.brange(params[1], b2))
        elif self.values[0] == 'range':
            mode = self.values[2]
            if mode == 'imm' or mode == 'bit' or mode == 'shift' or mode == 'abs':
                return 'BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'imms':
                return 'util::sext(opcode >> %d, %d)' % (params[0], self.bcount)
            elif mode == 'immm':
                return '0x800000 >> BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'asap':
                return '0xffffc0 + BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'asaq':
                return '0xffff80 + BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'pcrel':
                return '(m_pc + util::sext(BIT(opcode, %d, %d), %d)) & 0xffffff' % (params[0], self.bcount)
            else:
                print('unsupported range on %s %s' % (mode, self.name))
        elif self.values[0] == 'split-range':
            b2 = self.values[1]
            b1 = self.bcount - b2
            mode = self.values[3]
            if mode == 'imm' or mode == 'bit' or mode == 'shift':
                return 'bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'imms':
                return 'util::sext(bitswap<%d>(opcode%s%s), %d)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2), self.bcount)
            elif mode == 'asap':
                return '0xffffc0 + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'asaq':
                return '0xffff80 + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'pcrel':
                return '(m_pc + util::sext(bitswap<%d>(opcode%s%s), %d)) & 0xffffff' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2), self.bcount)
            else:
                print('unsupported split-range on %s %s' % (mode, self.name))
        elif self.name == 'exabs' or self.name == 'eximm' or self.name == 'exoff' or self.name == 'exco' or self.name == 'eam1a' or self.name == 'eam1i':
            return 'exv'
        elif self.name == 'expcrel':
            return '(m_pc+exv) & 0xffffff'
        else:
            print('unsupported get_value_expression on %s %s' % (self.values[0], self.name))
        return ''

    def param(self, params):
        if self.values[0] == "single":
            return ', ts_%s[BIT(opcode, %d, %d)]' % (self.name, params[0], self.bcount)
        elif self.values[0] == "single-alt":
            return ', ts_%s[bitswap<%d>(opcode, %d%s)]' % (self.name, self.bcount+1, params[1], self.brange(params[0], self.bcount))
        elif self.values[0] == 'split':
            b2 = self.values[1]
            b1 = self.bcount - b2
            return ', ts_%s[bitswap<%d>(opcode%s%s)]' % (self.name, self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
        elif self.values[0] == 'split-alt':
            b2 = self.values[1]
            b1 = self.bcount - b2
            return ', ts_%s[bitswap<%d>(opcode, %d%s%s)]' % (self.name, self.bcount+1, params[2], self.brange(params[0], b1), self.brange(params[1], b2))
        elif self.values[0] == 'range':
            mode = self.values[2]
            if mode == 'imm' or mode == 'bit' or mode == 'shift' or mode == 'abs':
                return ', BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'imms':
                return ', BIT(opcode, %d) ? "-" : "+", std::abs(util::sext(opcode >> %d, %d))' % (self.bcount - 1, params[0], self.bcount)
            elif mode == 'immm':
                return ', 0x800000 >> BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'asap':
                return ', 0xffffc0 + BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'asaq':
                return ', 0xffff80 + BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'pcrel':
                return ', (pc + BIT(opcode, %d, %d)) & 0xffffff' % (params[0], self.bcount)
        elif self.values[0] == 'split-range':
            b2 = self.values[1]
            b1 = self.bcount - b2
            mode = self.values[3]
            if mode == 'imm' or mode == 'bit' or mode == 'shift':
                return ', bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'imms':
                return ', BIT(opcode, %d) ? "-" : "+", std::abs(util::sext(bitswap<%d>(opcode%s%s), %d))' % (params[0] + b1 - 1, self.bcount, self.brange(params[0], b1), self.brange(params[1], b2), self.bcount)
            elif mode == 'asap':
                return ', 0xffffc0 + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'asaq':
                return ', 0xffff80 + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'pcrel':
                return ', (pc + util::sext(bitswap<%d>(opcode%s%s), %d)) & 0xffffff' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2), self.bcount)
        elif self.name == 'exabs' or self.name == 'eximm' or self.name == 'exco' or self.name == 'eam1a' or self.name == 'eam1i':
            return ', exv'
        elif self.name == 'exoff':
            return ', BIT(exv, 23) ? "-" : "+", std::abs(util::sext(exv, 24))'
        elif self.name == 'expcrel':
            return ', (pc+exv) & 0xffffff'
        return ''

Function("acc", 1, ["single", ['a', 'b']])
Function("nacc", -1, ["single", ['b', 'a']])
Function("xyr", 1, ["single", ['x', 'y']])
Function("daos", 1, ["single", ['x0', 'y0', 'x1', 'y1']])
Function("daos3", 2, ["single-alt", ['b', None, None, None, 'x0', 'y0', 'x1', 'y1', 'a', None, None, None, 'x0', 'y0', 'x1', 'y1']])
Function("dao3", 1, ["single", [None, None, 'a1', 'b1', 'x0', 'y0', 'x1', 'y1']])
Function("dao3b", 1, ["single", [None, None, 'a0', 'b0', 'x0', 'y0', 'x1', 'y1']])
Function("fvbr1", 1, ["single", [None, None, None, None, 'x0', 'x1', 'y0', 'y1', 'a0', 'b0', 'a2', 'b2', 'a1', 'b1', 'a', 'b', 'r', 'n']])
Function("fvbr1s", 2, ["split", 3, [None, None, None, None, 'x0', 'x1', 'y0', 'y1', 'a0', 'b0', 'a2', 'b2', 'a1', 'b1', 'a', 'b', 'r', 'n']])
Function("fvbr2", 1, ["single", ['m', None, None, 'ep', None, None, None, None, None,
                                 'vba', 'sc', None, None, None, None, None, None,
                                 'sz', 'sr', 'omr', 'sp', 'ssh', 'ssl', 'la', 'lc']])
Function("imm12", 2, ["split-range", 8, 4096, "imm"])
Function("imm8", 1, ["range", 256, "imm"])
Function("imm6", 1, ["range", 64, "imm"])
Function("imm5m", 1, ["range", 24, "immm"])
Function("imm1", 1, ["range", 2, "imm"])
Function("bit5", 1, ["range", 24, "bit"])
Function("shift5", 1, ["range", 17, "shift"])
Function("shift6", 1, ["range", 41, "shift"])
Function("actrl", 1, ["single", [None, None, 'a1', 'b1', 'x0', 'y0', 'x1', 'y1']])
Function("eam4", 1, ["single", ['(r)-n', '(r)+n', '(r)-', '(r)+']])
Function("eam1", 1, ["single", ['(r)-n', '(r)+n', '(r)-', '(r)+', '(r)', '(r+n)', None, None, None, None, None, None, None, None, '-(r)']])
Function("eam1a", 1, ["val", 0x30, "abs"])
Function("eam1i", 1, ["val", 0x34, "imm"])
Function("asa6", 1, ["range", 64, "abs"])
Function("asa6p", 1, ["range", 64, "asap"])
Function("asa6q", 1, ["range", 64, "asaq"])
Function("asa6qs", 2, ["split-range", 5, 64, "asaq"])
Function("tbrx", 1, ["single", ['x0', 'x1', 'a', 'b']])
Function("tbry", 1, ["single", ['y0', 'y1', 'a', 'b']])
Function("xreg", 1, ["single", ['x0', 'x1']])
Function("yreg", 1, ["single", ['y0', 'y1']])
Function("lmr", 2, ["split", 2, ['a10', 'b10', 'x', 'y', 'a', 'b', 'ab', 'ba']])
Function("xyeax", 1, ["single", ['(r)+n', '(r)-', '(r)+', '(r)']])
Function("xyeay", 3, ["split-alt", 2, ['(rh)+n', '(rh)-', '(rh)+', '(rh)', '(rl)+n', '(rl)-', '(rl)+', '(rl)']])
Function("ar", 1, ["single", ['r']])
Function("exoff", 0, ["pass"])
Function("eximm", 0, ["pass"])
Function("expcrel", 0, ["pass"])
Function("exabs", 0, ["pass"])
Function("exco", 0, ["pass"])
Function("pcrel", 2, ["split-range", 5, 512, "pcrel"])
Function("sda7", 2, ["split-range", 1, 128, "imms"])
Function("sda7b", 2, ["split-range", 4, 128, "imms"])
Function("abs12", 1, ["range", 4096, "abs"])
Function("fobr", 1, ["single", [None, None, None, None, 'x0', 'x1', 'y0', 'y1',
                                'a0', 'b0', 'a2', 'b2', 'a1', 'b1', 'a', 'b']])
Function("sbr", 1, ["single", [None, None, None, None, 'x0', 'x1', 'y0', 'y1',
                               'a0', 'b0', 'a2', 'b2', 'a1', 'b1', 'a', 'b',
                               'r', 'n', 'm',
                               None, None, 'ep', None, None, None, None, None,
                               'vba', 'sc', None, None, None, None, None, None,
                               'sz', 'sr', 'omr', 'sp', 'ssh', 'ssl', 'la', 'lc']])
Function("sbr_nos", 1, ["single", [None, None, None, None, 'x0', 'x1', 'y0', 'y1',
                                   'a0', 'b0', 'a2', 'b2', 'a1', 'b1', 'a', 'b',
                                   'r', 'n', 'm',
                                   None, None, 'ep', None, None, None, None, None,
                                   'vba', 'sc', None, None, None, None, None, None,
                                   'sz', 'sr', 'omr', 'sp', None, 'ssl', 'la', 'lc']])
Function("ctrl", 1, ["single", ['mr', 'ccr', 'com', 'eom']])
Function("cc", 1, ["single", ['cc', 'ge', 'ne', 'pl', 'nn', 'ec', 'lc', 'gt', 'cs', 'lt', 'eq', 'mi', 'nr', 'es', 'ls', 'le']])
Function("xyc", 1, ["single", ['x', 'y']])
Function("ss", 2, ["split", 1, ['ss', None, 'su', 'uu']])
Function("ss1", 1, ["single", ['su', 'uu']])
Function("sign", 1, ["single", ['+', '-']])
Function("damo4_a",  1, ["single", ['x0', 'y0', 'x1', 'y1', 'x1', 'y1', 'x0', 'y0', 'x0', 'y0', 'x1', 'y1', 'y1', 'x0', 'y0', 'x1']])
Function("damo4_b", -1, ["single", ['x0', 'y0', 'x0', 'y0', 'x1', 'y1', 'x1', 'y1', 'y1', 'x0', 'y0', 'x1', 'x0', 'y0', 'x1', 'y1']])
Function("agu", 1, ["single", ['r', 'n']])
Function("damo1_a",  1, ["single", ['x0', 'y0', 'x1', 'y1', 'x0', 'y0', 'x1', 'y1']])
Function("damo1_b", -1, ["single", ['x0', 'y0', 'x0', 'y0', 'y1', 'x0', 'y0', 'x1']])
Function("damo2", 1, ["single", ['y1', 'x0', 'y0', 'x1']])

NormalRegs = ['a', 'a0', 'a1', 'a2', 'b', 'b0', 'b1', 'b2', 'x0', 'x1', 'y0', 'y1', 'a10', 'b10', 'ab', 'ba', 'x', 'y', 'ep', 'vba', 'sc', 'sz', 'sr', 'omr', 'sp', 'ssh', 'ssl', 'la', 'lc', 'mr', 'ccr', 'com', 'eom']
ArrayRegs = ['r', 'n', 'm']
IndirectRegs = ['(r)-n', '(r)+n', '(r)-', '(r)+', '(r)', '(r+n)', '-(r)']
DasmFlags = ['over', 'cond', 'out']

class Slot:
    def __init__(self, name, func):
        global functions
        if func not in functions:
            print("Unknown function %s" % func)
            sys.exit(1)
        self.name = name
        self.func = functions[func]
        self.params = []
        self.used = False

    def mark_used(self):
        self.used = True

    def add_param(self, p):
        self.params.append(p)

    def format(self):
        return self.func.format()

    def param(self):
        return self.func.param(self.params)

    def get_ftype(self):
        return self.func.get_ftype()

    def get_slot_count(self):
        return self.func.get_slot_count()

    def get_val_idxs_slotted(self, slot, idxs):
        return self.func.get_val_idxs_slotted(slot, self.params, idxs)

    def get_value_expression(self):
        return self.func.get_value_expression(self.params)

class Variant:
    def __init__(self, inst, cid, has_ex):
        self.inst = inst
        self.cid = cid
        self.slots = {}
        self.has_ex = has_ex

    def add_slot(self, s, value):
        self.slots[s.name] = value

    def create_name(self, segs):
        self.name = ''
        for s in segs:
            if type(s) == str:
                self.name += s
            else:
                if s.name in self.slots:
                    self.name += self.slots[s.name]
                else:
                    self.name += '[' + s.name + ']'

    def generate_code(self, f, post):
        if self.inst.unhandled():
            if self.name != '':
                print('\t\tunhandled("%s");' % self.name, file=f)
        else:
            self.inst.source[SourceType.interp_post if post else SourceType.interp_pre].gen(f, self.slots, self.inst.slots)

class Source:
    def __init__(self):
        self.text = []
        self.slots = {}

    def empty(self):
        return len(self.text) == 0

    def parse(self, line):
        r = []
        isa = lambda c: (c >= 'a' and c <= 'z') or (c >= '0' and c <= '9')
        isp = lambda c: c == '+'
        pos = 0
        while True:
            spos = pos
            while pos != len(line) and line[pos] != '$':
                pos += 1
            if pos != spos:
                r.append(line[spos:pos])
            if pos == len(line):
                break
            pos += 1
            if pos == len(line):
                print("$ at end of line")
                sys.exit(1)
            if not (isa(line[pos]) or isp(line[pos])):
                print("Unexpected %c after $" % line[pos])
                sys.exit(1)
            spos = pos
            if isa(line[pos]):
                pos += 1
                while pos != len(line) and isa(line[pos]):
                    pos += 1
            else:
                pos += 1
            sname = line[spos:pos]
            smode = ''
            if pos != len(line) and line[pos] == ':':
                pos += 1
                spos = pos
                while pos != len(line) and isa(line[pos]):
                    pos += 1
                smode = line[spos:pos]
            if smode != '' and smode != 'w' and smode != 'm' and smode != '1' and smode != 'w1' and smode != 'h' and smode != 'wh' and smode != 'wb' and smode != 'l' and smode != 'wl':
                print("Unexpected slot mode %s" % smode)
                sys.exit(1)
            smode_id = SlotMode.memory if smode == 'm' else SlotMode.write if 'w' in smode else SlotMode.read
            schange_id = SlotChange.sel1 if '1' in smode else SlotChange.bus24 if 'h' in smode else SlotChange.bus8 if 'b' in smode else SlotChange.bus48 if 'l' in smode else SlotChange.none
            if sname not in self.slots:
                self.slots[sname] = [False]*3
            self.slots[sname][smode_id] = True
            sub = []
            if pos != len(line) and line[pos] == '(':
                pos += 1
                depth = 1
                spos = pos
                while pos != len(line) and depth != 0:
                    if line[pos] == '(':
                        depth += 1
                    if line[pos] == ')':
                        depth -= 1
                    pos += 1
                if depth != 0:
                    print("Unbalanced parenthesis")
                    sys.exit(1)
                sub = self.parse(line[spos:pos-1])
            r.append([sname, smode_id, schange_id, sub])
        return r

    def add(self, line):
        self.text.append(self.parse(line))

    def expand(self, line, slots, islots):
        global NormalRegs
        s = ''
        for e in line:
            if type(e) == str:
                s += e
                continue
            if e[1] == SlotMode.read:
                if e[0] in slots:
                    slot = slots[e[0]]
                    if slot == 'a' or slot == 'b':
                        if e[2] == SlotChange.sel1:
                            s += e[0] + '_1'
                        elif e[2] == SlotChange.bus24:
                            s += e[0] + '_h'
                        elif e[2] == SlotChange.bus48:
                            s += e[0] + '_l'
                        else:
                            s += e[0]
                    else:
                        s += e[0]
                else:
                    s += e[0]
            elif e[1] == SlotMode.memory:
                if e[0] not in slots:
                    print("uninstanciated slot (memory)", e)
                s += 'm_' + slots[e[0]]
            else:
                sub = self.expand(e[3], slots, islots)
                if e[0] in slots:
                    slot = slots[e[0]]
                    if slot in NormalRegs:
                        if slot == 'a' or slot == 'b':
                            if e[2] == SlotChange.sel1:
                                s += 'set_%s1(%s)' % (slot, sub)
                            elif e[2] == SlotChange.bus24:
                                s += 'set_%sh(%s)' % (slot, sub)
                            elif e[2] == SlotChange.bus8:
                                s += 'set_%sf(%s)' % (slot, sub)
                            elif e[2] == SlotChange.bus48:
                                s += 'set_%sl(%s)' % (slot, sub)
                            else:
                                s += 'set_%s(%s)' % (slot, sub)
                        elif (slot == 'x0' or slot == 'x1' or slot == 'y0' or slot == 'y1') and e[2] == SlotChange.bus8:
                            s += 'set_%sf(%s)' % (slot, sub)
                        else:
                            s += 'set_%s(%s)' % (slot, sub)
                    elif slot in ArrayRegs:
                        si = islots[e[0]].get_value_expression() + ' & 7'
                        s += 'set_%s(%s, %s)' % (slot, si, sub)
                    else:
                        print("instanciated slot (write)", e, slot)
                else:
                    print("Write on uninstanciated slot %s" % e[0])
                    sys.exit(1)
        return s

    def ab_scan_gen(self, f, slot, inst, line, changes):
        for e in line:
            if type(e) == list and e[0] == slot and e[1] == SlotMode.read:
                if e[2] not in changes:
                    changes.append(e[2])
                    if e[2] == SlotChange.none:
                        print('\t\tu64 %s = get_%s();' % (slot, inst), file=f)
                    elif e[2] == SlotChange.sel1:
                        print('\t\tu32 %s_1 = get_%s1();' % (slot, inst), file=f)
                    elif e[2] == SlotChange.bus24:
                        print('\t\tu32 %s_h = get_%sh();' % (slot, inst), file=f)
                    elif e[2] == SlotChange.bus48:
                        print('\t\tu64 %s_l = get_%sl();' % (slot, inst), file=f)
                    else:
                        print("unhandled SlotChange %d" % e[2])
            if type(e) == list:
                self.ab_scan_gen(f, slot, inst, e[3], changes)

    def gen(self, f, slots, islots):
        for slot, sinfo in self.slots.items():
            if sinfo[SlotMode.read]:
                if slot in slots:
                    inst = slots[slot]
                    if inst in NormalRegs:
                        if inst == 'a' or inst == 'b':
                            changes = []
                            for line in self.text:
                                self.ab_scan_gen(f, slot, inst, line, changes)
                        elif inst == 'a10' or inst == 'b10' or inst == 'ab' or inst == 'ba' or inst == 'x' or inst == 'y':
                            print('\t\tu64 %s = get_%s();' % (slot, inst), file=f)
                        else:
                            print('\t\tu32 %s = get_%s();' % (slot, inst), file=f)
                    elif inst in ArrayRegs:
                        si = islots[slot].get_value_expression() + ' & 7'
                        print('\t\tu32 %s = get_%s(%s & 7);' % (slot, inst, islots[slot].get_value_expression()), file=f)
                    elif inst in IndirectRegs:
                        print('\t\tint %s_r = %s & 7;' % (slot, islots[slot].get_value_expression()), file=f)
                        if inst == '(r)-n':
                            print('\t\tu32 %s = get_r(%s_r);' % (slot, slot), file=f)
                            print('\t\tadd_r(%s_r, -m_n[%s_r]);' % (slot, slot), file=f)
                        elif inst == '(r)+n':
                            print('\t\tu32 %s = get_r(%s_r);' % (slot, slot), file=f)
                            print('\t\tadd_r(%s_r, m_n[%s_r]);' % (slot, slot), file=f)
                        elif inst == '(r)-':
                            print('\t\tu32 %s = get_r(%s_r);' % (slot, slot), file=f)
                            print('\t\tadd_r(%s_r, -1);' % (slot), file=f)
                        elif inst == '(r)+':
                            print('\t\tu32 %s = get_r(%s_r);' % (slot, slot), file=f)
                            print('\t\tadd_r(%s_r, 1);' % (slot), file=f)
                        elif inst == '(r)':
                            print('\t\tu32 %s = get_r(%s_r);' % (slot, slot), file=f)
                        elif inst == '(r+n)':
                            print('\t\tu32 %s = calc_add_r(%s_r, m_n[%s_r]);' % (slot, slot, slot), file=f)
                        elif inst == '-(r)':
                            print('\t\tadd_r(%s_r, -1);' % (slot), file=f)
                            print('\t\tu32 %s = get_r(%s_r);' % (slot, slot), file=f)
                        else:
                            print("Unimplemented IndirectRegs %s" % inst)
                            sys.exit(1)
                    elif islots[slot].func.name == 'cc':
                        print('\t\tbool %s = test_%s();' % (slot, inst), file=f)
                    else:
                        print("instanciated slot (read)", slot, inst)
                else:
                    print('\t\tu32 %s = %s;' % (slot, islots[slot].get_value_expression()), file=f)

        for line in self.text:
            s = '\t\t' + self.expand(line, slots, islots)
            print(s, file=f)

class Instruction:
    def __init__(self, idx, mode, head, ditable, dtable, citable, ctable):
        self.did = len(dtable)
        self.cid = len(ctable)
        dtable.append(self)
        self.mode = mode
        sidx = self.find_separator(head)
        self.idstr = head[:sidx].rstrip(' \t')
        self.parse_slots(head, sidx+2)
        self.check_ex()
        self.fill_ditable(ditable, dtable, idx)
        self.parse_dasm(head[6:sidx].strip(' \t').rstrip(' \t'))
        self.fill_citable(citable, ctable, idx)
        self.source = [Source(), Source(), Source(), Source()]

    def unhandled(self):
        return self.source[SourceType.interp_pre].empty() and self.source[SourceType.interp_post].empty()

    def unhandled_drc(self):
        return self.source[SourceType.drc_pre].empty() and self.source[SourceType.drc_post].empty()

    def add(self, mode, line):
        self.source[mode].add(line)

    def find_separator(self, head):
        sidx = 6
        while sidx != len(head) and head[sidx:sidx+3] != ' - ' and head[sidx:] != ' -':
            sidx += 1
        if sidx == len(head):
            print("Missing '-' separator in instruction %s" % ll)
            sys.exit(1)
        return sidx

    def parse_slots(self, head, pos):
        self.slots = {}
        self.flags = []
        while pos != len(head):
            while pos != len(head) and head[pos] == ' ':
                pos += 1
            if pos == len(head):
                break
            sname = ''
            while pos != len(head) and head[pos] != ':' and head[pos] != '(' and head[pos] != ' ':
                sname += head[pos]
                pos += 1
            if pos == len(head) or head[pos] == ' ':
                if sname not in DasmFlags:
                    print("Unknown dasm flag %s" % sname)
                    sys.exit(1)
                self.flags.append(sname)
                continue
            if pos == len(head) or head[pos] != ':':
                print("Missing : after slot name %s [%s]" % (sname, head[0]))
                sys.exit(1)
            pos += 1
            fname = ''
            while pos != len(head) and head[pos] != '(':
                fname += head[pos]
                pos += 1
            if pos == len(head):
                print("Missing ( after function name %s [%s]" % (fname, head[0]))
                sys.exit(1)
            sinfo = Slot(sname, fname)
            pos += 1
            while True:
                while pos != len(head) and head[pos] == ' ':
                    pos += 1
                if pos != len(head) and head[pos] == ')':
                    pos += 1
                    break
                if pos == len(head):
                    break
                nb = ''
                while pos != len(head) and head[pos] != ',' and head[pos] != ')':
                    nb += head[pos]
                    pos += 1
                if nb == '':
                    print("Unexpected character in function parameters of %s [%s]" % (fname, head[0]))
                    sys.exit(1)
                sinfo.add_param(int(nb))
                while pos != len(head) and head[pos] == ' ':
                    pos += 1
                if pos != len(head) and head[pos] == ',':
                    pos += 1
                    continue
                if pos != len(head) and head[pos] != ')':
                    print("Unexpected character in function parameters of %s [%s]" % (fname, head[0]))
                    sys.exit(1)
                pos += 1
                break
            self.slots[sname] = sinfo

    def check_ex(self):
        self.has_ex = False
        for slot, sinfo in self.slots.items():
            if sinfo.func.has_ex:
                self.has_ex = True
                break

    def fill_ditable(self, ditable, dtable, idx):
        global functions
        shift = 8 if self.mode == IType.move else 0
        idxs = [idx << shift]
        idxc = None
        if idx == idxc:
            print('.'.join(["%06x" % _ for _ in idxs]), self.slots)
        for slot, sinfo in self.slots.items():
            idxs = sinfo.func.expand_idxs(sinfo.params, idxs)
            if idx==idxc:
                print(slot, sinfo.func.name, '.'.join(["%06x" % _ for _ in idxs]))
        if idx == idxc:
            print('.'.join(["%06x" % _ for _ in idxs]))
        for idx in idxs:
            idx2 = idx >> shift
            if ditable[idx2] != 0:
                print("Collision %06x [%s] [%s]" % (idx, self.idstr, dtable[ditable[idx2]].idstr))
                sys.exit(1)
            ditable[idx2] = self.did

    def parse_dasm(self, dasm):
        self.segs = []
        isa = lambda c: (c >= 'a' and c <= 'z') or (c >= '0' and c <= '9')
        isp = lambda c: c == '+'
        s = None
        p_is_a = False
        for i in range(len(dasm)):
            c = dasm[i]
            if isp(c):
                if s != None:
                    self.segs.append(s)
                s = None
                self.segs.append(c)
            elif isa(c):
                if s != None:
                    if p_is_a:
                        s += c
                    else:
                        self.segs.append(s)
                        p_is_a = True
                        s = c
                else:
                    p_is_a = True
                    s = c
            else:
                if s != None:
                    if not p_is_a:
                        s += c
                    else:
                        self.segs.append(s)
                        p_is_a = False
                        s = c
                else:
                    p_is_a = False
                    s = c
        if s != None:
            self.segs.append(s)
        if len(self.segs) > 0 and (self.segs[0][-2:] == 'cc' or self.segs[0][-2:] == 'ss'):
            self.segs.insert(1, self.segs[0][-2:])
            self.segs[0] = self.segs[0][:-2]
        for i in range(len(self.segs)):
            if self.segs[i] in self.slots:
                slot = self.slots[self.segs[i]]
                if slot.used:
                    print("%s: slot %s used twice" % (dasm, self.segs[i]))
                    sys.exit(1)
                slot.mark_used()
                self.segs[i] = slot
        for slot, sinfo in self.slots.items():
            if not sinfo.used:
                print("%s: slot %s not used" % (dasm, slot))
                sys.exit(1)

    def dasm_format(self):
        da = ''
        param = ''
        for s in self.segs:
            if type(s) == str:
                da += s
            else:
                fmt = s.format()
                if fmt == '%s$%x' and da[-1] == '+':
                    da = da[:-1]
                da += fmt
                param += s.param()
        if param:
            return 'util::string_format("' + da + '"' + param + ')'
        else:
            return '"' + da + '"'

    def fill_citable(self, citable, ctable, idx):
        slotorder = []
        has_ex = False
        for slot, sinfo in self.slots.items():
            if sinfo.func.has_ex:
                has_ex = True
            slotorder.append([sinfo.get_ftype(), sinfo])
        slotorder.sort(key=lambda s: s[0])
#        print("%s: %s" % (self.idstr, ' '.join([s[1].name+'.'+s[1].func.name+('.%d' % s[0]) for s in slotorder])))
        shift = 8 if self.mode == IType.move else 0
        indexes = [0]*len(slotorder)
        imax = []
        for s in slotorder:
            imax.append(s[1].get_slot_count())
        while True:
            idxs = [idx << shift]
            cid = len(ctable)
            vv = Variant(self, cid, has_ex)
            ctable.append(vv)
            for si in range(len(slotorder)):
                s = slotorder[si][1]
                entry, idxs = s.get_val_idxs_slotted(indexes[si], idxs)
                if entry != None:
                    vv.add_slot(s, entry)
            vv.create_name(self.segs)
#            print("%4d: %s %s" % (cid, vv.name, '.'.join(['%06x' % _ for _ in idxs])))
#            print("%4d: %s" % (cid, vv.name))
            for idxl in idxs:
                idx2 = idxl >> shift
                citable[idx2] = cid
            si = len(slotorder) - 1
            while si >= 0:
                indexes[si] += 1
                if indexes[si] < imax[si]:
                    break
                indexes[si] = 0
                si -= 1
            if si == -1:
                break

class ISA:
    def __init__(self):
        self.dipar = [0] * 0x100
        self.dipars = [ None ]
        self.dnpar = [0] * 0x1000000
        self.dnpars = [ None ]
        self.dmove = [0] * 0x10000
        self.dmoves = [ None ]
        self.cipar = [0] * 0x100
        self.cipars = [ None ]
        self.cnpar = [0] * 0x1000000
        self.cnpars = [ None ]
        self.cmove = [0] * 0x10000
        self.cmoves = [ None ]
        self.inst = None

    def load(self, fname):
        inst = None
        mode = None
        for l in open(fname, "rt"):
            ll = l.rstrip('\r\n \t')
            if len(ll) == 0 or ll[0] == '#':
                continue
            if ll[0] != ' ' and ll[0] != '\t':
                if ll[:4] == '....':
                    idx = int(ll[4:6], 16)
                    inst = Instruction(idx, IType.ipar, ll, self.dipar, self.dipars, self.cipar, self.cipars)
                elif ll[4:6] == '..':
                    idx = int(ll[0:4], 16)
                    inst = Instruction(idx, IType.move, ll, self.dmove, self.dmoves, self.cmove, self.cmoves)
                else:
                    idx = int(ll[:6], 16)
                    inst = Instruction(idx, IType.npar, ll, self.dnpar, self.dnpars, self.cnpar, self.cnpars)
                mode = SourceType.interp_pre
            else:
                ll = ll.lstrip(' \t')
                if ll[0] == '#':
                    continue
                if ll == '%p':
                    mode = SourceType.interp_post
                elif ll == '%d':
                    mode = SourceType.drc_pre
                elif ll == '%dp':
                    mode = SourceType.drc_post
                else:
                    inst.add(mode, ll)

    def coverage(self, array):
        r = 0
        i0 = 0
        i1 = 0
        for i in range(len(array)):
            if array[i] != 0:
                r += 1
                if i0 == 0:
                    i0 = i
                i1 = i
        return r / (len(array) if len(array) != 0x1000000 else 0x100000) * 100, i0, i1, r

    def dump(self):
        print("d ipar = %4d (%5.2f%%     %02x-    %02x %6x)" % (len(self.dipars), *self.coverage(self.dipar)))
        print("d npar = %4d (%5.2f%% %06x-%06x %6x)" % (len(self.dnpars), *self.coverage(self.dnpar)))
        print("d move = %4d (%5.2f%%   %04x-  %04x %6x)" % (len(self.dmoves), *self.coverage(self.dmove)))
        print("c ipar = %4d (%5.2f%%     %02x-    %02x %6x)" % (len(self.cipars), *self.coverage(self.cipar)))
        print("c npar = %4d (%5.2f%% %06x-%06x %6x)" % (len(self.cnpars), *self.coverage(self.cnpar)))
        print("c move = %4d (%5.2f%%   %04x-  %04x %6x)" % (len(self.cmoves), *self.coverage(self.cmove)))


    def check_move_npar_collisions(self):
        for i in range(0x1000000):
            if self.dnpar[i] != 0 and self.dmove[i >> 8] != 0:
                print("move/npar collision %06x [%s] [%s]" % (i, self.dmoves[self.dmove[i>>8]].idstr, self.dnpars[self.dnpar[i]]))
                sys.exit(1)

    def gen_index_array(self, f, array):
        for i in range(0, len(array), 32):
            s = '\t'
            for j in range(32):
                s += "%d," % array[i+j]
            print(s, file=f)

    def gen_flags_array(self, f, array):
        for i in range(0, 256, 16):
            s = '\t'
            for j in range(16):
                if i+j < len(array) and array[i+j] and len(array[i+j].flags) > 0:
                    s += '|'.join(['STEP_' + flag.upper() for flag in array[i+j].flags]) + ','
                else:
                    s += '0,'
            print(s, file=f)

    def gen_ex_array(self, f, insts):
        s = '\t'
        total = 64*((len(insts) + 63) // 64)
        for i in range(0, total, 64):
            v = 0
            if (i & 7) != 0:
                s += ' '
            for j in range(64):
                if i+j >= len(insts):
                    break
                if (i+j) and insts[i+j].has_ex:
                    v |= 1 << j
            s += '0x%016x' % v
            if total > 64:
                s += ','
            if (i & (7*64)) == (7*64):
                print(s, file=f)
                s = '\t'
        if len(s) > 1:
            print(s, file=f)

    def gen_dasm_switch(self, f, insts):
        for i in range(len(insts)):
            if insts[i] == None:
                print("\tcase %d: return \"\";" % i, file=f)
            else:
                print("\tcase %d: return %s;" % (i, insts[i].dasm_format()), file=f);

    def gen_interp_switch(self, f, insts, post):
        for i in range(len(insts)):
            print('\tcase %d: { // %s' % (i, '-' if i == 0 else insts[i].name), file=f)
            if insts[i] == None:
                print("\t\tbreak;", file=f)
                print("\t\t}", file=f)
            else:
                insts[i].generate_code(f, post);
                print("\t\tbreak;", file=f)
                print("\t\t}", file=f)

    def gen_disasm(self, fname):
        f = open(fname, "wt")
        print("// license:BSD-3-Clause", file=f)
        print("// copyright-holders:Olivier Galibert", file=f)
        print("", file=f)
        print("// Generated file, do not edit, run dsp563xx-make.py instead", file=f)
        print("", file=f)
        print("#include \"emu.h\"", file=f)
        print("#include \"dsp563xxd.h\"", file=f)
        print("", file=f)
        print("const u8 dsp563xx_disassembler::t_ipar[0x100] = {", file=f)
        self.gen_index_array(f, self.dipar)
        print("};", file=f)
        print("", file=f)
        print("const u8 dsp563xx_disassembler::t_move[0x10000] = {", file=f)
        self.gen_index_array(f, self.dmove)
        print("};", file=f)
        print("", file=f)
        print("const u8 dsp563xx_disassembler::t_npar[0x100000] = {", file=f)
        self.gen_index_array(f, self.dnpar[:0x100000])
        print("};", file=f)
        print("", file=f)
        print("const u64 dsp563xx_disassembler::t_move_ex =", file=f)
        self.gen_ex_array(f, self.dmoves)
        print(";", file=f)
        print("", file=f)
        print("const u64 dsp563xx_disassembler::t_npar_ex[4] = {", file=f)
        self.gen_ex_array(f, self.dnpars)
        print("};", file=f)
        print("", file=f)
        print("const u32 dsp563xx_disassembler::t_npar_flags[0x100] = {", file=f)
        self.gen_flags_array(f, self.dnpars)
        print("};", file=f)
        print("", file=f)
        for _,ff in functions.items():
            if ff.need_array():
                ff.gen_array(f)
        print("", file=f)
        print("std::string dsp563xx_disassembler::disasm_ipar(u8 kipar, u32 opcode, u32 exv, u32 pc)", file=f)
        print("{", file=f)
        print("\tswitch(kipar) {", file=f)
        self.gen_dasm_switch(f, self.dipars)
        print("\t}", file=f)
        print("\tabort();", file=f)
        print("}", file=f)
        print("", file=f)
        print("std::string dsp563xx_disassembler::disasm_move(u8 kmove, u32 opcode, u32 exv, u32 pc)", file=f)
        print("{", file=f)
        print("\tswitch(kmove) {", file=f)
        self.gen_dasm_switch(f, self.dmoves)
        print("\t}", file=f)
        print("\tabort();", file=f)
        print("}", file=f)
        print("", file=f)
        print("std::string dsp563xx_disassembler::disasm_npar(u8 knpar, u32 opcode, u32 exv, u32 pc)", file=f)
        print("{", file=f)
        print("\tswitch(knpar) {", file=f)
        self.gen_dasm_switch(f, self.dnpars)
        print("\t}", file=f)
        print("\tabort();", file=f)
        print("}", file=f)

    def gen_itable(self, fname):
        f = open(fname, "wt")
        print("// license:BSD-3-Clause", file=f)
        print("// copyright-holders:Olivier Galibert", file=f)
        print("", file=f)
        print("// Generated file, do not edit, run dsp563xx-make.py instead", file=f)
        print("", file=f)
        print("#include \"emu.h\"", file=f)
        print("#include \"dsp563xx.h\"", file=f)
        print("", file=f)
        print("const u16 dsp563xx_device::t_ipar[0x100] = {", file=f)
        self.gen_index_array(f, self.cipar)
        print("};", file=f)
        print("", file=f)
        print("const u16 dsp563xx_device::t_move[0x10000] = {", file=f)
        self.gen_index_array(f, self.cmove)
        print("};", file=f)
        print("", file=f)
        print("const u16 dsp563xx_device::t_npar[0x100000] = {", file=f)
        self.gen_index_array(f, self.cnpar[:0x100000])
        print("};", file=f)
        print("", file=f)
        print("const u64 dsp563xx_device::t_move_ex[40] = {", file=f)
        self.gen_ex_array(f, self.cmoves)
        print("};", file=f)
        print("", file=f)
        print("const u64 dsp563xx_device::t_npar_ex[72] = {", file=f)
        self.gen_ex_array(f, self.cnpars)
        print("};", file=f)

    def gen_interp(self, fname):
        f = open(fname, "wt")
        print("// license:BSD-3-Clause", file=f)
        print("// copyright-holders:Olivier Galibert", file=f)
        print("", file=f)
        print("// Generated file, do not edit, run dsp563xx-make.py instead", file=f)
        print("", file=f)
        print("#include \"emu.h\"", file=f)
        print("#include \"dsp563xx.h\"", file=f)
        print("", file=f)
        print("void dsp563xx_device::execute_ipar(u16 kipar)", file=f)
        print("{", file=f)
        print("\tswitch(kipar) {", file=f)
        self.gen_interp_switch(f, self.cipars, False)
        print("\t}", file=f)
        print("}", file=f)
        print("", file=f)
        print("void dsp563xx_device::execute_pre_move(u16 kmove, u32 opcode, u32 exv)", file=f)
        print("{", file=f)
        print("\tswitch(kmove) {", file=f)
        self.gen_interp_switch(f, self.cmoves, False)
        print("\t}", file=f)
        print("}", file=f)
        print("", file=f)
        print("void dsp563xx_device::execute_post_move(u16 kmove, u32 opcode, u32 exv)", file=f)
        print("{", file=f)
        print("\tswitch(kmove) {", file=f)
        self.gen_interp_switch(f, self.cmoves, True)
        print("\t}", file=f)
        print("}", file=f)
        print("", file=f)
        print("void dsp563xx_device::execute_npar(u16 knpar, u32 opcode, u32 exv)", file=f)
        print("{", file=f)
        print("\tswitch(knpar) {", file=f)
        self.gen_interp_switch(f, self.cnpars, False)
        print("\t}", file=f)
        print("}", file=f)

isa = ISA()
isa.load("dsp563xx.lst")
isa.check_move_npar_collisions()
isa.dump()

isa.gen_disasm("dsp563xxd-tables.cpp")
isa.gen_itable("dsp563xx-tables.cpp")
isa.gen_interp("dsp563xx-interp.cpp")
