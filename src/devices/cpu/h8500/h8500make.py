#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert, R. Belmont
from __future__ import print_function

USAGE = """
Usage: %s h8500.lst <mode> <output>
   mode = s   interpreter source
        = d   disassembler tables
"""
import sys

# H8/500 addressing modes
AM_INFO = {
    "-":         (0, "none"),
    "rel8":      (1, "rel8"),
    "rel16":     (2, "rel16"),
    "imm8":      (1, "imm8"),
    "imm16":     (2, "imm16"),
    "abs16":     (2, "abs16"),
    "abs24":     (3, "abs24"),
    # Implicit / encoded-in-byte operands
    "r8l":       (0, "r8l"),
    "r8h":       (0, "r8h"),
    "r16l":      (0, "r16l"),
    "r16h":      (0, "r16h"),
    "sp":        (0, "sp"),
    "imm3":      (0, "imm3"),
    "imm4":      (0, "imm4"),
    "sr":        (0, "sr"),
    "ccr":       (0, "ccr"),
    "cp":        (0, "cp"),
    "dp":        (0, "dp"),
    "ep":        (0, "ep"),
    "tp":        (0, "tp"),
    "br":        (0, "br"),
    "one":       (0, "one"),
    "two":       (0, "two"),
}

# H8/500 Effective Address prefixes
EA_PREFIXES = [
    # (val_lo, val_hi, premask, operand_bytes, name)
    (0x04, 0x04, 0xff, 1, "ea_04"),    # #imm8  (immediate source, byte)
    (0x0c, 0x0c, 0xff, 2, "ea_0c"),    # #imm16 (immediate source, word)
    (0x05, 0x05, 0xff, 1, "ea_05"),    # @aa:8, byte size
    (0x0d, 0x0d, 0xff, 1, "ea_0d"),    # @aa:8, word size
    (0x15, 0x15, 0xff, 2, "ea_15"),    # @aa:16, byte size
    (0x1d, 0x1d, 0xff, 2, "ea_1d"),    # @aa:16, word size
    (0xa0, 0xaf, 0xf0, 0, "ea_a0"),    # Rn (size in bit 3, reg in low 3)
    (0xb0, 0xbf, 0xf0, 0, "ea_b0"),    # @-Rn
    (0xc0, 0xcf, 0xf0, 0, "ea_c0"),    # @Rn+
    (0xd0, 0xdf, 0xf0, 0, "ea_d0"),    # @Rn
    (0xe0, 0xef, 0xf0, 1, "ea_e0"),    # @(d:8, Rn)
    (0xf0, 0xff, 0xf0, 2, "ea_f0"),    # @(d:16, Rn)
]

# Dispatch level reserved for the post-EA opcode table.
POST_EA_LEVEL = 2

# Recognised scope flags.
#   's'       - standalone only
#   'e'       - post-EA, any EA prefix
#   'b'       - appears in both standalone and post-EA trees
#   'e:NAME'  - post-EA, only when m_ir[0] is in EA_CLASSES[NAME]
#   'e:!NAME' - post-EA, only when m_ir[0] is NOT in EA_CLASSES[NAME]
#
# Two opcodes can share the same post-EA (val, mask) slot when their
# EA classes are disjoint.  This happens several places in the H8/500
# instruction set.

# EA prefix bytes
def _all_ea_bytes():
    s = set()
    for (lo, hi, _premask, _ob, _name) in EA_PREFIXES:
        s |= set(range(lo, hi + 1))
    return frozenset(s)
EA_CLASS_ALL = _all_ea_bytes()

# Named EA classes that the .lst can reference in scope='e:NAME' form.
EA_CLASSES = {
    'imm':  frozenset({0x04, 0x0c}),                     # #imm8 / #imm16
    'absb': frozenset({0x05, 0x0d}),                     # @aa:8
    'absw': frozenset({0x15, 0x1d}),                     # @aa:16
    'reg':  frozenset(range(0xa0, 0xb0)),                # Rn (any size)
    'regb': frozenset(range(0xa0, 0xa8)),                # Rn byte-size
    'regw': frozenset(range(0xa8, 0xb0)),                # Rn word-size
    'pdec': frozenset(range(0xb0, 0xc0)),                # @-Rn
    'pinc': frozenset(range(0xc0, 0xd0)),                # @Rn+
    'ind':  frozenset(range(0xd0, 0xe0)),                # @Rn
    'd8':   frozenset(range(0xe0, 0xf0)),                # @(d:8, Rn)
    'd16':  frozenset(range(0xf0, 0x100)),               # @(d:16, Rn)
}

# C++ boolean expressions that test for the various EA classes
EA_CLASS_TESTS = {
    'imm':  "(m_ir[0] == 0x04 || m_ir[0] == 0x0c)",
    'absb': "(m_ir[0] == 0x05 || m_ir[0] == 0x0d)",
    'absw': "(m_ir[0] == 0x15 || m_ir[0] == 0x1d)",
    'reg':  "((m_ir[0] & 0xf0) == 0xa0)",
    'regb': "((m_ir[0] & 0xf8) == 0xa0)",
    'regw': "((m_ir[0] & 0xf8) == 0xa8)",
    'pdec': "((m_ir[0] & 0xf0) == 0xb0)",
    'pinc': "((m_ir[0] & 0xf0) == 0xc0)",
    'ind':  "((m_ir[0] & 0xf0) == 0xd0)",
    'd8':   "((m_ir[0] & 0xf0) == 0xe0)",
    'd16':  "((m_ir[0] & 0xf0) == 0xf0)",
}

def _scope_parts(scope):
    """Return (base, ea_class_set, ea_test_expr) for a scope token, or
    raise ValueError if the scope is unrecognised.

      base       - 's', 'e', or 'b' (used by the placement logic)
      ea_class_set - frozenset of m_ir[0] bytes the entry accepts; None
                   for scope='s' (no post-EA presence)
      ea_test_expr - C++ boolean expression; "true" when unrestricted
    """
    if scope == 's':
        return ('s', None, "true")
    if scope in ('e', 'b'):
        return (scope, EA_CLASS_ALL, "true")
    if scope.startswith('e:'):
        rest = scope[2:]
        if rest.startswith('!'):
            cls_name = rest[1:]
            invert = True
        else:
            cls_name = rest
            invert = False
        if cls_name not in EA_CLASSES:
            raise ValueError("unknown EA class '%s' (valid: %s)" %
                             (cls_name, ", ".join(sorted(EA_CLASSES))))
        cls = EA_CLASSES[cls_name]
        test = EA_CLASS_TESTS[cls_name]
        if invert:
            cls = EA_CLASS_ALL - cls
            test = "!" + test
        return ('e', cls, test)
    raise ValueError("unknown scope '%s'" % scope)

def is_valid_scope(s):
    try:
        _scope_parts(s)
        return True
    except ValueError:
        return False

# Mnemonic flag bits for the disassembler.
STEP_OVER_MNEMONICS = {"jsr", "bsr", "pjsr"}
STEP_OUT_MNEMONICS = {"rts", "rte", "prts", "rtd", "prtd"}
UNCONDITIONAL_BRANCH_MNEMONICS = {"bra", "brn"}


def hexsplit(s):
    if len(s) % 2 != 0:
        sys.stderr.write("Hex string '%s' has odd length\n" % s)
        sys.exit(1)
    return [int(s[i:i+2], 16) for i in range(0, len(s), 2)]


def has_memory_access(line):
    return ("read" in line) or ("write" in line)


def has_eat(line):
    return "eat-all-cycles" in line


def emit_full(f, dname, fname, source):
    print("void %s::%s_full()" % (dname, fname), file=f)
    print("{", file=f)
    substate = 1
    for line in source:
        if has_memory_access(line):
            print(line, file=f)
            print("\tif(m_icount <= m_bcount) {", file=f)
            print("\t\tif(access_to_be_redone()) {", file=f)
            print("\t\t\tm_icount++;", file=f)
            print("\t\t\tm_inst_substate = %d;" % substate, file=f)
            print("\t\t} else", file=f)
            print("\t\t\tm_inst_substate = %d;" % (substate + 1), file=f)
            print("\t\treturn;", file=f)
            print("\t}", file=f)
            substate += 2
        elif has_eat(line):
            print("\tif(m_icount) { m_icount = m_bcount; } m_inst_substate = %d; return;" % substate, file=f)
            substate += 1
        else:
            print(line, file=f)
    print("}", file=f)
    print("", file=f)


def emit_partial(f, dname, fname, source):
    print("void %s::%s_partial()" % (dname, fname), file=f)
    print("{", file=f)
    print("switch(m_inst_substate) {", file=f)
    print("case 0:", file=f)
    substate = 1
    for line in source:
        if has_memory_access(line):
            print("\t[[fallthrough]];", file=f)
            print("case %d:;" % substate, file=f)
            print(line, file=f)
            print("\tif(m_icount <= m_bcount) {", file=f)
            print("\t\tif(access_to_be_redone()) {", file=f)
            print("\t\t\tm_icount++;", file=f)
            print("\t\t\tm_inst_substate = %d;" % substate, file=f)
            print("\t\t} else", file=f)
            print("\t\t\tm_inst_substate = %d;" % (substate + 1), file=f)
            print("\t\treturn;", file=f)
            print("\t}", file=f)
            print("\t[[fallthrough]];", file=f)
            print("case %d:;" % (substate + 1), file=f)
            substate += 2
        elif has_eat(line):
            print("\tif(m_icount) { m_icount = m_bcount; } m_inst_substate = %d; return;" % substate, file=f)
            print("case %d:;" % substate, file=f)
            substate += 1
        else:
            print(line, file=f)
    print("\tbreak;", file=f)
    print("}", file=f)
    print("\tm_inst_substate = 0;", file=f)
    print("}", file=f)
    print("", file=f)


class Hash:
    def __init__(self, premask=0):
        self.mask = 0x00
        self.enabled = False
        self.premask = premask
        self.d = {}

    def set(self, val, entry):
        if val in self.d:
            sys.stderr.write("Collision at byte 0x%02x (entry: %s)\n" %
                             (val, getattr(entry, "name", repr(entry))))
            sys.exit(1)
        self.d[val] = entry


class Opcode:
    def __init__(self, val, mask, name, scope='e', am1='-', am2='-'):
        try:
            self._scope_base, self._scope_ea_class, self._scope_ea_test = _scope_parts(scope)
        except ValueError as err:
            sys.stderr.write("Opcode %s: bad scope '%s' (%s)\n"
                             % (name, scope, err))
            sys.exit(1)
        if am1 not in AM_INFO:
            sys.stderr.write("Opcode %s: unknown am1 '%s'\n" % (name, am1))
            sys.exit(1)
        if am2 not in AM_INFO:
            sys.stderr.write("Opcode %s: unknown am2 '%s'\n" % (name, am2))
            sys.exit(1)
        self.name = name
        self.val = hexsplit(val)
        self.mask = hexsplit(mask)
        if len(self.val) != len(self.mask):
            sys.stderr.write("Opcode %s: val/mask length mismatch\n" % name)
            sys.exit(1)
        self.scope = scope
        self.am1 = am1
        self.am2 = am2
        self.extra_bytes = AM_INFO[am1][0] + AM_INFO[am2][0]
        self.enabled = True
        self.premask = 0          # set when placed in a Hash level
        self.source = []

        base = len(self.val)
        for i in range(self.extra_bytes):
            self.source.append("\tm_ir[%d] = read8i(pc24());" % (i + base))
            self.source.append("\tm_pc = (m_pc + 1) & 0xffff;")

    def description(self):
        return "%s %s %s (%s)" % (self.name, self.am1, self.am2, self.scope)

    def add_source_line(self, line):
        self.source.append(line)

    def is_dispatch(self):
        return False

    def function_name(self):
        n = self.name.replace(".", "_").replace("/", "_")
        if self.am1 != "-":
            n = n + "_" + self.am1
        if self.am2 != "-":
            n = n + "_" + self.am2
        return n

    def save_dasm(self, f):
        v = 0
        m = 0
        for i in range(min(len(self.val), 4)):
            shift = (3 - i) * 8
            v |= self.val[i] << shift
            m |= self.mask[i] << shift
        size = len(self.val) + self.extra_bytes
        if self.name in STEP_OVER_MNEMONICS:
            flags = "%d | STEP_OVER" % size
        elif self.name in STEP_OUT_MNEMONICS:
            flags = "%d | STEP_OUT" % size
        elif (self.am1 in ("rel8", "rel16")
              and self.name not in UNCONDITIONAL_BRANCH_MNEMONICS):
            flags = "%d | STEP_COND" % size
        else:
            flags = "%d" % size
        print('\t{ 0x%08x, 0x%08x, "%s", DASM_%s, DASM_%s, %s },' %
              (v, m, self.name,
               AM_INFO[self.am1][1], AM_INFO[self.am2][1], flags),
              file=f)


class Special:
    def __init__(self, val, name):
        self.name = name
        self.val = int(val, 16)
        self.enabled = True
        self.source = []

    def add_source_line(self, line):
        self.source.append(line)


class Macro:
    def __init__(self, tokens):
        self.name = tokens[1]
        self.params = tokens[2:]
        self.source = []

    def add_source_line(self, line):
        self.source.append(line)

    def apply(self, target, tokens):
        n_params = len(self.params)
        values = []
        if n_params >= 1:
            for i in range(n_params - 1):
                values.append(tokens[i + 1])
            tail = " ".join(tokens[n_params:])
            values.append(tail)
        for line in self.source:
            out = line
            for i in range(n_params):
                out = out.replace(self.params[i], values[i])
            target.add_source_line(out)


class EAPrefixStep:
    """Auto-generated dispatcher for an H8/500 EA prefix byte / range.
    Reads the EA's operand bytes into m_ir[1..operand_bytes], then sets
    m_inst_state to POST_EA_LEVEL | next byte so that the dispatcher
    re-enters into the post-EA tree."""
    def __init__(self, val, premask, operand_bytes, name):
        self.name = name
        self.val = val
        self.premask = premask
        self.operand_bytes = operand_bytes
        self.enabled = True

    def is_dispatch(self):
        return True

    def source(self):
        lines = []
        for i in range(1, 1 + self.operand_bytes + 1):
            lines.append("\tm_ir[%d] = read8i(pc24());" % i)
            lines.append("\tm_pc = (m_pc + 1) & 0xffff;")
        lines.append("\tm_ea_op_bytes = %d;" % self.operand_bytes)
        lines.append("\tm_inst_state = 0x%x0000 | m_ir[%d];" %
                     (POST_EA_LEVEL, 1 + self.operand_bytes))
        return lines


class MultiOpcodeEntry:
    """Holds two or more post-EA opcodes that share the same (val, mask)
    slot but accept disjoint sets of EA-prefix bytes.  The generator
    emits a small sub-dispatch on m_ir[0] at the case site, calling
    whichever opcode's EA-class includes the current EA byte.  Used for
    e.g. BSET vs ORC, BCLR vs ANDC, BNOT vs XORC, where the op byte is
    the same but the meaning depends on whether the EA prefix is
    immediate (#imm8/#imm16) or any other addressing mode."""
    def __init__(self):
        self.entries = []
        self.enabled = True
        self.premask = 0

    def add(self, opc):
        ea_class = opc._scope_ea_class or frozenset()
        for existing in self.entries:
            ex_class = existing._scope_ea_class or frozenset()
            if ea_class & ex_class:
                sys.stderr.write(
                    "EA-class collision: %s (%s) and %s (%s) both accept "
                    "EA byte(s) 0x%s\n" %
                    (opc.name, opc.scope, existing.name, existing.scope,
                     ", 0x".join("%02x" % v for v in sorted(ea_class & ex_class))))
                sys.exit(1)
        self.entries.append(opc)
        self.premask |= opc.premask

    def is_dispatch(self):
        return False

    def emit_call(self, f, variant, indent):
        """Emit C++ that calls whichever entry's EA-class matches m_ir[0]."""
        first = True
        for opc in self.entries:
            test = opc._scope_ea_test
            keyword = "if" if first else "else if"
            print("%s%s (%s) {" % (indent, keyword, test), file=f)
            print("%s\t%s_%s();" % (indent, opc.function_name(), variant), file=f)
            print("%s}" % indent, file=f)
            first = False


class DispatchStep:
    """Generated transition between byte N and byte N+1 of a multi-byte
    opcode.  Reads the next byte into m_ir[depth] and sets m_inst_state
    so the dispatcher re-enters at the deeper level identified by 'id'."""
    def __init__(self, id_, depth, opc):
        self.id = id_
        self.depth = depth     # number of opcode bytes already consumed
        self.enabled = False
        # premask = the mask bits this entry locked at the byte that placed it.
        self.premask = opc.mask[depth - 1]
        self.mask = opc.mask[depth]
        self.name = ""
        for i in range(depth):
            self.name += "%02x" % opc.val[i]

    def is_dispatch(self):
        return True

    def source(self):
        return [
            "\tm_ir[%d] = read8i(pc24());" % self.depth,
            "\tm_pc = (m_pc + 1) & 0xffff;",
            "\tm_inst_state = 0x%x0000 | m_ir[%d];" % (self.id, self.depth),
        ]


class OpcodeList:
    def __init__(self, fname):
        self.opcode_info = []
        self.standalone_opcodes = []
        self.post_ea_opcodes = []
        self.states_info = []
        self.ea_steps = []
        self.dispatch_info = []    # DispatchStep entries for multi-byte opcodes
        self.dispatch = {}         # level id -> Hash
        self.macros = {}
        try:
            f = open(fname, "r")
        except Exception:
            sys.stderr.write("Cannot open %s\n" % fname)
            sys.exit(1)
        inf = None
        for raw in f:
            if raw.startswith("#"):
                continue
            line = raw.rstrip()
            if not line:
                continue
            if line.startswith(" ") or line.startswith("\t"):
                if inf is not None:
                    tokens = line.split()
                    if tokens and tokens[0] in self.macros:
                        self.macros[tokens[0]].apply(inf, tokens)
                    else:
                        inf.add_source_line(line)
                continue
            tokens = line.split()
            if not tokens:
                continue
            if tokens[0] == "macro":
                inf = Macro(tokens)
                self.macros[inf.name] = inf
            elif len(tokens) == 2:
                inf = Special(tokens[0], tokens[1])
                self.states_info.append(inf)
            elif 3 <= len(tokens) <= 6:
                inf = self._make_opcode(tokens)
                self.opcode_info.append(inf)
                if inf._scope_base in ('s', 'b'):
                    self.standalone_opcodes.append(inf)
                if inf._scope_base in ('e', 'b'):
                    self.post_ea_opcodes.append(inf)
            else:
                sys.stderr.write("Can't parse line: %s\n" % line)
                sys.exit(1)
        f.close()

    def _make_opcode(self, tokens):
        val, mask, name = tokens[0], tokens[1], tokens[2]
        rest = tokens[3:]
        scope = 'e'
        am1 = am2 = '-'
        if len(rest) == 1:
            if is_valid_scope(rest[0]):
                scope = rest[0]
            else:
                sys.stderr.write("Opcode %s: 4th token '%s' is not a scope "
                                 "(see _scope_parts in h8500make.py)\n"
                                 % (name, rest[0]))
                sys.exit(1)
        elif len(rest) == 2:
            if is_valid_scope(rest[0]):
                scope, am1 = rest
            else:
                am1, am2 = rest
        elif len(rest) == 3:
            if not is_valid_scope(rest[0]):
                sys.stderr.write("Opcode %s: 4th token '%s' is not a scope "
                                 "(see _scope_parts in h8500make.py)\n"
                                 % (name, rest[0]))
                sys.exit(1)
            scope, am1, am2 = rest
        return Opcode(val, mask, name, scope, am1, am2)

    def get_level(self, key):
        if key in self.dispatch:
            return self.dispatch[key]
        h = Hash(0)
        self.dispatch[key] = h
        return h

    def build_dispatch(self):
        # tree for standalone opcodes (no EA prefix)
        std = self.get_level(0)
        for opc in self.standalone_opcodes:
            self._place_opcode(std, opc)

        # tree for the various EA prefix byte sequences
        for (lo, hi, premask, operand_bytes, name) in EA_PREFIXES:
            step = EAPrefixStep(lo, premask, operand_bytes, name)
            self.ea_steps.append(step)
            std.mask |= premask
            std.enabled = True

            for v in range(lo, hi + 1):
                if v != lo:
                    break
            std.set(lo, step)

        if self.post_ea_opcodes:
            post = self.get_level(POST_EA_LEVEL)
            for opc in self.post_ea_opcodes:
                self._place_opcode(post, opc)

    def _place_opcode(self, h, opc):
        """Walk opc.val byte-by-byte into the dispatch tree rooted at h,
        creating a DispatchStep at each transition between dispatched
        bytes.  Reuses an existing DispatchStep if another opcode has
        already opened the same sub-tree."""
        for i in range(len(opc.val)):
            v = opc.val[i]
            h.mask |= opc.mask[i]
            h.enabled = True
            if i == len(opc.val) - 1:
                opc.premask = opc.mask[i]
                if v in h.d:
                    existing = h.d[v]
                    # allow opcodes with restricted EA prefix scopes to share a slot
                    opc_restricted = (opc._scope_ea_class is not None and
                                      opc._scope_ea_class != EA_CLASS_ALL)
                    if (isinstance(existing, Opcode) and
                            existing._scope_ea_class is not None and
                            existing._scope_ea_class != EA_CLASS_ALL and
                            opc_restricted):
                        bundle = MultiOpcodeEntry()
                        bundle.add(existing)
                        bundle.add(opc)
                        h.d[v] = bundle
                    elif isinstance(existing, MultiOpcodeEntry):
                        if not opc_restricted:
                            sys.stderr.write(
                                "Collision on %s: other opcodes already at "
                                "byte 0x%02x; either narrow this opcode's "
                                "scope (e:NAME / e:!NAME) or pick a "
                                "different val/mask\n"
                                % (opc.description(), v))
                            sys.exit(1)
                        existing.add(opc)
                    else:
                        sys.stderr.write("Collision on %s (terminal already at "
                                         "byte 0x%02x)\n" % (opc.description(), v))
                        sys.exit(1)
                else:
                    h.set(v, opc)
                return
            if v in h.d:
                existing = h.d[v]
                if not existing.is_dispatch():
                    sys.stderr.write("Collision on %s (terminal already at "
                                     "byte 0x%02x)\n" % (opc.description(), v))
                    sys.exit(1)
                existing.enabled = True
                if isinstance(existing, EAPrefixStep):
                    sys.stderr.write("Opcode %s: byte 0x%02x is an EA prefix; "
                                     "cannot also be a multi-byte opcode\n"
                                     % (opc.name, v))
                    sys.exit(1)
                h = self.get_level(existing.id)
            else:
                new_id = POST_EA_LEVEL + 1 + len(self.dispatch_info)
                step = DispatchStep(new_id, i + 1, opc)
                self.dispatch_info.append(step)
                step.enabled = True
                h.set(v, step)
                h = self.get_level(step.id)

    def save_dasm(self, f, dname):
        print("const %s::disasm_entry %s::disasm_entries[] = {" % (dname, dname), file=f)
        for opc in self.opcode_info:
            opc.save_dasm(f)
        print('\t{ 0, 0, "illegal", DASM_none, DASM_none, 1 },', file=f)
        print("};", file=f)
        print("", file=f)

    def save_opcodes(self, f, dname):
        for opc in self.opcode_info:
            emit_full(f, dname, opc.function_name(), opc.source)
            emit_partial(f, dname, opc.function_name(), opc.source)
        for sta in self.states_info:
            emit_full(f, dname, "state_" + sta.name, sta.source)
            emit_partial(f, dname, "state_" + sta.name, sta.source)
        for step in self.ea_steps:
            emit_full(f, dname, step.name, step.source())
            emit_partial(f, dname, step.name, step.source())
        for step in self.dispatch_info:
            if step.enabled:
                emit_full(f, dname, "dispatch_" + step.name, step.source())
                emit_partial(f, dname, "dispatch_" + step.name, step.source())

    def save_exec(self, f, dname, variant):
        print("void %s::do_exec_%s()" % (dname, variant), file=f)
        print("{", file=f)
        print("\tswitch(m_inst_state >> 16) {", file=f)

        # Level 0: standalone dispatch + EA prefix transitions.
        std = self.get_level(0)
        if std.enabled:
            print("\tcase 0x00: {", file=f)
            print("\t\tswitch(m_inst_state & 0x%02x) {" % std.mask, file=f)
            self._emit_level_cases(f, std, variant)
            print("\t\tdefault: illegal(); break;", file=f)
            print("\t\t}", file=f)
            print("\t\tbreak;", file=f)
            print("\t}", file=f)

        # Level 1: special states.
        print("\tcase 0x01: {", file=f)
        print("\t\tswitch(m_inst_state & 0xffff) {", file=f)
        for sta in self.states_info:
            if sta.enabled:
                print("\t\tcase 0x%04x: state_%s_%s(); break;" %
                      (sta.val & 0xffff, sta.name, variant), file=f)
        print("\t\t}", file=f)
        print("\t\tbreak;", file=f)
        print("\t}", file=f)

        # Level 2: post-EA dispatch.
        if POST_EA_LEVEL in self.dispatch:
            post = self.dispatch[POST_EA_LEVEL]
            if post.enabled:
                print("\tcase 0x%02x: {" % POST_EA_LEVEL, file=f)
                print("\t\tswitch(m_inst_state & 0x%02x) {" % post.mask, file=f)
                self._emit_level_cases(f, post, variant)
                print("\t\tdefault: illegal(); break;", file=f)
                print("\t\t}", file=f)
                print("\t\tbreak;", file=f)
                print("\t}", file=f)

        # Levels POST_EA_LEVEL+1 and up: multi-byte opcode sub-trees
        # opened by DispatchStep transitions.  Emit one switch arm per
        # sub-level that's reachable from an enabled opcode.
        for level_id in sorted(self.dispatch):
            if level_id <= POST_EA_LEVEL:
                continue
            sub = self.dispatch[level_id]
            if not sub.enabled:
                continue
            print("\tcase 0x%02x: {" % level_id, file=f)
            print("\t\tswitch(m_inst_state & 0x%02x) {" % sub.mask, file=f)
            self._emit_level_cases(f, sub, variant)
            print("\t\tdefault: illegal(); break;", file=f)
            print("\t\t}", file=f)
            print("\t\tbreak;", file=f)
            print("\t}", file=f)

        print("\tdefault: illegal(); break;", file=f)
        print("\t}", file=f)
        print("}", file=f)
        print("", file=f)

    def _emit_level_cases(self, f, h, variant):
        for val, child in sorted(h.d.items()):
            if not child.enabled:
                continue
            fmask = child.premask | (h.mask ^ 0xff)
            cases = ""
            s = 0
            while s < 0x100:
                cases += "case 0x%02x: " % (val | s)
                s += 1
                while s & fmask:
                    s += s & fmask
            if isinstance(child, MultiOpcodeEntry):
                # EA-class sub-dispatch: emit an if/else chain on m_ir[0].
                print("\t\t%s{" % cases, file=f)
                child.emit_call(f, variant, "\t\t\t")
                print("\t\t\tbreak;", file=f)
                print("\t\t}", file=f)
                continue
            if isinstance(child, EAPrefixStep):
                fn = child.name
            elif child.is_dispatch():
                fn = "dispatch_" + child.name
            else:
                fn = child.function_name()
            print("\t\t%s{ %s_%s(); break; }" % (cases, fn, variant), file=f)


def main(argv):
    if len(argv) != 4:
        sys.stderr.write(USAGE % argv[0])
        return 1
    mode = argv[2]
    if mode not in ("s", "d"):
        sys.stderr.write("Unknown mode '%s'\n" % mode)
        return 1
    opcodes = OpcodeList(argv[1])
    try:
        f = open(argv[3], "w")
    except Exception:
        sys.stderr.write("Cannot write %s\n" % argv[3])
        return 1
    dname = "h8500_device" if mode == "s" else "h8500_disassembler"
    if mode == "s":
        opcodes.build_dispatch()
        opcodes.save_opcodes(f, dname)
        opcodes.save_exec(f, dname, "full")
        opcodes.save_exec(f, dname, "partial")
    else:
        opcodes.save_dasm(f, dname)
    f.close()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
