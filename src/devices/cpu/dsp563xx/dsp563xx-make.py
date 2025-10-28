#!/usr/bin/python

import sys
from enum import IntEnum, auto

class IType(IntEnum):
    ipar = 0
    npar = 1
    move = 2

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
        self.bcount = None
        if self.icount:
            for bcount in range(16):
                if (1 << bcount) >= self.icount:
                    self.bcount = bcount
                    break

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

    def dual_split(self, m1, vals):
        m2 = (1 << m1) - 1
        r = []
        for v in vals:
            i1 = v >> m1
            i2 = v & m2
            r.append([i1, i2])
        return r
        
    def expand_idxs(self, params, idxs):
        if len(params) != abs(self.pcount):
            print("Incorrect number of parameters on %s call, expected %d, got %d" % (self.name, abs(self.pcount), len(params)))
            sys.exit(1)
        if self.pcount < 0:
            return idxs
        r = []
        for v in idxs:
            if self.values[0] == 'range' or self.values[0] == 'single' or self.values[0] == 'single-alt' or self.values[0] == 'val':
                bit = params[0]
                for i in self.ivals:
                    r.append(v | (i << bit))

            elif self.values[0] == 'split-range' or self.values[0] == 'split' or self.values[0] == 'split-alt':
                bit1 = params[0]
                bit2 = params[1]
                for e in self.dual_split(self.values[1], self.ivals):
                    r.append(v | (e[0] << bit1) | (e[1] << bit2))

            elif self.values[0] == 'pass':
                r.append(v)

            else:
                print("unknown expansion %s" % self.values[0])
                sys.exit(1)
        return r

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
                return '0x%%0%dx' % ((self.bcount + 3) // 4)
            elif mode == 'bit':
                return '%d'
            elif mode == 'asap' or mode == 'asaq' or mode == 'pcrel':
                return '0x%06x'                
        if self.name == 'exabs' or self.name == 'expcrel' or self.name == 'eximm' or self.name == 'eam1a' or self.name == 'eam1i':
            return '0x%06x'
        return '[' + self.name + ']'

    def brange(self, bit, bc):
        s = ''
        for bit in range(bit+bc, bit, -1):
            s += ', %d' % (bit-1)
        return s

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
            mode = self.values[3 if self.values[0] == "split-range" else 2]
            if mode == 'imm' or mode == 'bit':
                return ', BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'asap':
                return ', 0xffffc0 + BIT(opcode, %d, %d)' % (params[0], self.bcount)
            elif mode == 'asaq':
                return ', 0xffff80 + BIT(opcode, %d, %d)' % (params[0], self.bcount)                
        elif self.values[0] == 'split-range':
            b2 = self.values[1]
            b1 = self.bcount - b2
            mode = self.values[3 if self.values[0] == "split-range" else 2]
            if mode == 'imm' or mode == 'bit':
                return ', bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'asap':
                return ', 0xffffc0 + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'asaq':
                return ', 0xffff80 + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
            elif mode == 'pcrel':
                return ', pc + bitswap<%d>(opcode%s%s)' % (self.bcount, self.brange(params[0], b1), self.brange(params[1], b2))
        elif self.name == 'exabs' or self.name == 'eximm' or self.name == 'eam1a' or self.name == 'eam1i':
            return ', exv'
        elif self.name == 'expcrel':
            return ', (pc+exv) & 0xffffff'
        return ''
                
Function("acc", 1, ["single", ['a', 'b']])
Function("nacc", -1, ["single", ['b', 'a']])
Function("xyr", 1, ["single", ['x', 'y']])
Function("dao2", 2, ["single-alt", [None, "b", "x", "y", "x0", "y0", "x1", "y1", None, "a", "x", "y", "x0", "y0", "x1", "y1"]])
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
Function("imm4", 1, ["range", 16, "imm"])
Function("imm1", 1, ["range", 2, "imm"])
Function("bit5", 1, ["range", 24, "bit"])
Function("shift5", 1, ["range", 17, "shift"])
Function("actrl", 1, ["single", [None, None, 'a1', 'b1', 'x0', 'y0', 'x1', 'y1']])
Function("eam4", 1, ["single", ['(r)-n', '(r)+n', '(r)-', '(r)+']])
Function("eam1", 1, ["single", ['(r)-n', '(r)+n', '(r)-', '(r)+', '(r)', '(r+n)', None, None, None, None, None, None, None, None, '-(r)']])
Function("eam1a", 1, ["val", 0x30, "abs"])
Function("eam1i", 1, ["val", 0x34, "imm"])
Function("asa6", 1, ["range", 64, "asa"])
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
Function("eximm16", 0, ["pass"])
Function("eximm24", 0, ["pass"])
Function("expcrel", 0, ["pass"])
Function("exabs", 0, ["pass"])
Function("exco", 0, ["pass"])
Function("pcrel", 2, ["split-range", 5, 512, "pcrel"])
Function("sda7", 2, ["split-range", 1, 128, "imm"])
Function("sda7b", 2, ["split-range", 4, 128, "imm"])
Function("pcrel12", 1, ["range", 4096, "pcrel"])
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

class Slot:
    def __init__(self, func):
        global functions
        if func not in functions:
            print("Unknown function %s" % func)
            sys.exit(1)
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


class Instruction:
    def __init__(self, id, idx, mode, head, itable, table):
        self.id = id
        table.append(self)
        self.mode = mode
        sidx = self.find_separator(head)
        self.idstr = head[:sidx].rstrip(' \t')
        self.parse_slots(head, sidx+2)
        self.check_ex()
        self.fill_itable(id, itable, table, mode, idx)
        self.parse_dasm(head[6:sidx].strip(' \t').rstrip(' \t'))

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
        self.keywords = []
        while pos != len(head):
            while pos != len(head) and head[pos] == ' ':
                pos += 1
            if pos == len(head):
                break
            sname = ''
            while pos != len(head) and head[pos] != ':' and head[pos] != '(' and head[pos] != ' ':
                sname += head[pos]
                pos += 1
            if pos != len(head) and head[pos] == ' ':
                self.keyword.append(sname)
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
            sinfo = Slot(fname)
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

    def fill_itable(self, id, itable, table, mode, idx):
        global functions
        shift = 8 if mode == IType.move else 0
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
            if itable[idx2] != 0:
                print("Collision %06x [%s] [%s]" % (idx, self.idstr, table[itable[idx2]].idstr))
                sys.exit(1)
            itable[idx2] = id

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
                da += s.format()
                param += s.param()
        if param:
            return 'util::string_format("' + da + '"' + param + ')'
        else:
            return '"' + da + '"'

class ISA:
    def __init__(self):
        self.ipar = [0] * 0x100
        self.ipars = [ None ]
        self.ipar_id = 1
        self.npar = [0] * 0x1000000
        self.npars = [ None ]
        self.npar_id = 1
        self.move = [0] * 0x10000
        self.moves = [ None ]
        self.move_id = 1
        self.inst = None

    def load(self, fname):
        for l in open(fname, "rt"):
            ll = l.rstrip('\r\n \t')
            if len(ll) == 0 or ll[0] == '#':
                continue
            if ll[0] != ' ' and ll[0] != '\t':
                if ll[:4] == '....':
                    idx = int(ll[4:6], 16)
                    self.inst = Instruction(self.ipar_id, idx, IType.ipar, ll, self.ipar, self.ipars)
                    self.ipar_id += 1
                elif ll[4:6] == '..':
                    idx = int(ll[0:4], 16)
                    self.inst = Instruction(self.move_id, idx, IType.move, ll, self.move, self.moves)
                    self.move_id += 1
                else:
                    idx = int(ll[:6], 16)
                    self.inst = Instruction(self.npar_id, idx, IType.npar, ll, self.npar, self.npars)
                    self.npar_id += 1
            else:
                pass

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
        return r / (len(array) if len(array) != 0x1000000 else 0x100000) * 100, i0, i1

    def dump(self):
        print("d ipar = %4d (%5.2f%%     %02x-    %02x)" % (self.ipar_id, *self.coverage(self.ipar)))
        print("d npar = %4d (%5.2f%% %06x-%06x)" % (self.npar_id, *self.coverage(self.npar)))
        print("d move = %4d (%5.2f%%   %04x-  %04x)" % (self.move_id, *self.coverage(self.move)))
                    

    def check_move_npar_collisions(self):
        for i in range(0x1000000):
            if self.npar[i] != 0 and self.move[i >> 8] != 0:
                print("move/npar collision %06x [%s] [%s]" % (i, self.moves[self.move[i>>8]].idstr, self.npars[self.npar[i]]))
                sys.exit(1)

    def gen_index_array(self, f, array):
        for i in range(0, len(array), 32):
            s = '\t'
            for j in range(32):
                s += "%d," % array[i+j]
            print(s, file=f)

    def gen_ex_array(self, f, insts):
        s = '\t'
        for i in range(0, 64 if len(insts) <= 64 else 256, 64):
            v = 0
            if i:
                s += ' '
            for j in range(64):
                if i+j >= len(insts):
                    break
                if (i+j) and insts[i+j].has_ex:
                    v |= 1 << j
            s += '0x%016x' % v
            if len(insts) > 64:
                s += ','
        print(s, file=f)

    def gen_dasm_switch(self, f, insts):
        for i in range(len(insts)):
            if insts[i] == None:
                print("\tcase %d: return \"\";" % i, file=f)
            else:
                print("\tcase %d: return %s;" % (i, insts[i].dasm_format()), file=f);

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
        self.gen_index_array(f, self.ipar)
        print("};", file=f)
        print("", file=f)
        print("const u8 dsp563xx_disassembler::t_move[0x10000] = {", file=f)
        self.gen_index_array(f, self.move)
        print("};", file=f)
        print("", file=f)
        print("const u8 dsp563xx_disassembler::t_npar[0x100000] = {", file=f)
        self.gen_index_array(f, self.npar[:0x100000])
        print("};", file=f)
        print("", file=f)
        print("const u64 dsp563xx_disassembler::t_move_ex =", file=f)
        self.gen_ex_array(f, self.moves)
        print(";", file=f)
        print("", file=f)
        print("const u64 dsp563xx_disassembler::t_npar_ex[4] = {", file=f)
        self.gen_ex_array(f, self.npars)
        print("};", file=f)
        print("", file=f)
        for _,ff in functions.items():
            if ff.need_array():
                ff.gen_array(f)
        print("", file=f)
        print("std::string dsp563xx_disassembler::disasm_ipar(u8 kipar, u32 opcode, u32 exv, u32 pc)", file=f)
        print("{", file=f)
        print("\tswitch(kipar) {", file=f)
        self.gen_dasm_switch(f, self.ipars)
        print("\t}", file=f)
        print("\tabort();", file=f)
        print("}", file=f)
        print("", file=f)
        print("std::string dsp563xx_disassembler::disasm_move(u8 kmove, u32 opcode, u32 exv, u32 pc)", file=f)
        print("{", file=f)
        print("\tswitch(kmove) {", file=f)
        self.gen_dasm_switch(f, self.moves)
        print("\t}", file=f)
        print("\tabort();", file=f)
        print("}", file=f)
        print("", file=f)
        print("std::string dsp563xx_disassembler::disasm_npar(u8 knpar, u32 opcode, u32 exv, u32 pc)", file=f)
        print("{", file=f)
        print("\tswitch(knpar) {", file=f)
        self.gen_dasm_switch(f, self.npars)
        print("\t}", file=f)
        print("\tabort();", file=f)
        print("}", file=f)

isa = ISA()
isa.load("dsp563xx.lst")
isa.check_move_npar_collisions()
isa.dump()

#print(isa.npar[0x050c00])

isa.gen_disasm("dsp563xxd-tables.cpp")

