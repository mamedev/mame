// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

(function($scope, $as) {
"use strict";

// Import.
const base = $scope.base ? $scope.base : require("./base.js");

const dict = base.dict;
const NONE = base.NONE;
const Parsing = base.Parsing;
const MapUtils = base.MapUtils;

// Export.
const x86 = $scope[$as] = {};

function FAIL(msg) { throw new Error("[X86] " + msg); }

// Database
// ========

x86.dbName = "isa_x86.json";

// Metadata Tables
// ===============

const ArchGroupInfo = dict({
  "ry": ["ANY", "X64"],
  "rv": ["ANY", "ANY", "X64"]
});

// Groups are used by instruction tables to group multiple operand combinations into a single record. In general
// X86 and X86_64 instructions can be divided into GP and SIMD groups, where GP groups use `ry/my` syntax to
// specify operation for 16/32/64 bit registers and "xy/mxy"/"xyz/mxyz" groups to specify a SIMD instruction that
// uses either XMM/YMM (AVX) or XMM/YMM/ZMM registers (AVX-512).
const OperandGroupInfo = dict({
  "ry"   : { "group": "ry" , "subst": ["r32", "r64"] },
  "my"   : { "group": "ry" , "subst": ["m32", "m64"] },
  "axy"  : { "group": "ry" , "subst": ["eax", "rax"] },
  "bxy"  : { "group": "ry" , "subst": ["ebx", "rbx"] },
  "cxy"  : { "group": "ry" , "subst": ["ecx", "rcx"] },
  "dxy"  : { "group": "ry" , "subst": ["edx", "rdx"] },

  "rv"   : { "group": "rv" , "subst": ["r16", "r32", "r64"] },
  "mv"   : { "group": "rv" , "subst": ["m16", "m32", "m64"] },
  "axv"  : { "group": "rv" , "subst": ["ax", "eax", "rax"] },
  "bxv"  : { "group": "rv" , "subst": ["bx", "ebx", "rbx"] },
  "cxv"  : { "group": "rv" , "subst": ["cx", "ecx", "rcx"] },
  "dxv"  : { "group": "rv" , "subst": ["dx", "edx", "rdx"] },
  "immv" : { "group": "rv" , "subst": ["imm16", "imm32", "imms32"] },

  "xy"   : { "group": "xy" , "subst": ["xmm", "ymm"] },
  "mxy"  : { "group": "xy" , "subst": ["m128", "m256"] },

  "xxx"  : { "group": "xyz", "subst": ["xmm[31:0]", "xmm[63:0]", "xmm"] },
  "xxy"  : { "group": "xyz", "subst": ["xmm[63:0]", "xmm", "ymm"] },
  "xyz"  : { "group": "xyz", "subst": ["xmm", "ymm", "zmm"] },
  "mxxx" : { "group": "xyz", "subst": ["m32", "m64", "m128"] },
  "mxxy" : { "group": "xyz", "subst": ["m64", "m128", "m256"] },
  "mxyz" : { "group": "xyz", "subst": ["m128", "m256", "m512"] }
});

const OpcodeGroupInfo = dict({
  "Wy"   : { "group": "ry" , "subst": ["W0", "W1"] },
  "iv"   : { "group": "rv" , "subst": ["iw", "id", "id"] },
  "Pv"   : { "group": "rv" , "subst": ["66", "NP", "NP"] },
  "Wv"   : { "group": "rv" , "subst": ["W0", "W0", "W1"] }
});

// Instruction tables use various notations to specify L/LL field, which is used by VEX/EVEX/XOP encodings. This
// field has 1 bit (VEX/XOP) and 2 bits (EVEX) and in general the notation used is 128/256/512, which determines
// the size of SIMD operation, and this is also the notation we want to convert everything else into.
const OpcodeLLMapping = dict({
  "128": "128",
  "256": "256",
  "512": "512",
  "LZ" : "128",
  "LLZ": "128",
  "L0" : "128",
  "L1" : "256",
  "LIG": "LIG",
  "Lxy": "xy",
  "xyz": "xyz"
});

const RegSize = Object.freeze({
  "r8"  : 8,
  "r8hi": 8,
  "r16" : 16,
  "r32" : 32,
  "r64" : 64,
  "mm"  : 64,
  "xmm" : 128,
  "ymm" : 256,
  "zmm" : 512,
  "tmm" : 512, // Maximum size (64 bytes).
  "bnd" : 128,
  "k"   : 64,
  "st"  : 80
});

// CpuRegs
// =======

// Build an object containing CPU registers as keys mapping them to type, kind, and index.
function buildCpuRegs(defs) {
  const map = dict();

  for (let type in defs) {
    const def = defs[type];
    const kind = def.kind;
    const names = def.names;
    const group = def.group;

    if (def.any)
      map[def.any] = { type: type, kind: kind, index: -1, group: group };

    if (names) {
      for (let i = 0; i < names.length; i++) {
        let name = names[i];
        let m = /^([A-Za-z\(\)]+)(\d+)-(\d+)([A-Za-z\(\)]*)$/.exec(name);

        if (m) {
          let a = parseInt(m[2], 10);
          let b = parseInt(m[3], 10);

          for (let n = a; n <= b; n++) {
            const index = m[1] + n + m[4];
            map[index] = { type: type, kind: kind, index: index };
          }
        }
        else {
          map[name] = { type: type, kind: kind, index: i };
        }
      }
    }
  }

  // HACK: In instruction manuals `r8` denotes low 8-bit register, however,
  // that collides with `r8`, which is a 64-bit register. Since the result
  // of this function is only used internally we patch it to be compatible
  // with what Intel specifies.
  map.r8.type = "r8";

  return map;
}

const CpuRegisters = buildCpuRegs({
  "r8"  : { "kind": "gp"  , "any": "r8"   , "names": ["al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8-15b"] },
  "r8hi": { "kind": "gp"                  , "names": ["ah", "ch", "dh", "bh"] },
  "r16" : { "kind": "gp"  , "any": "r16"  , "names": ["ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8-15w"] },
  "r32" : { "kind": "gp"  , "any": "r32"  , "names": ["eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8-15d"] },
  "r64" : { "kind": "gp"  , "any": "r64"  , "names": ["rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8-15"] },
  "rxx" : { "kind": "gp"                  , "names": ["zax", "zcx", "zdx", "zbx", "zsp", "zbp", "zsi", "zdi"] },
  "sreg": { "kind": "sreg", "any": "sreg" , "names": ["es", "cs", "ss", "ds", "fs", "gs" ] },
  "creg": { "kind": "creg", "any": "creg" , "names": ["cr0-15"]  },
  "dreg": { "kind": "dreg", "any": "dreg" , "names": ["dr0-15"]  },
  "bnd" : { "kind": "bnd" , "any": "bnd"  , "names": ["bnd0-3"]  },
  "st"  : { "kind": "st"  , "any": "st(i)", "names": ["st(0-7)"] },
  "mm"  : { "kind": "mm"  , "any": "mm"   , "names": ["mm0-7"]   },
  "k"   : { "kind": "k"   , "any": "k"    , "names": ["k0-7"]    },
  "xmm" : { "kind": "vec" , "any": "xmm"  , "names": ["xmm0-31"] },
  "ymm" : { "kind": "vec" , "any": "ymm"  , "names": ["ymm0-31"] },
  "zmm" : { "kind": "vec" , "any": "zmm"  , "names": ["zmm0-31"] },
  "tmm" : { "kind": "tile", "any": "tmm"  , "names": ["tmm0-7"]  }
});

// asmdb.x86.Utils
// ===============

// X86/X64 utilities.
class Utils {
  static groupOf(op) {
    return Object.hasOwn(OperandGroupInfo, op) ? OperandGroupInfo[op].group : null;
  }

  static splitInstructionSignature(s) {
    let prefixes = [];
    if (s.startsWith("[")) {
      const prefixEnd = Parsing.matchClosingChar(s, 0);
      prefixes = s.substring(1, prefixEnd).replace("xacqrel", "xacquire|xrelease").split("|");

      s = s.substring(prefixEnd + 1).trim();
    }

    let nameEnd = s.indexOf(" ");
    let names = s.substring(0, nameEnd === -1 ? s.length : nameEnd);
    let operands = nameEnd === -1 ? "" : s.substring(nameEnd + 1).trim();

    if (names.endsWith("{nf}")) {
      names = names.substring(0, names.length - 4);
      prefixes.nf = true;
    }

    return {
      names: names.split("|"),
      prefixes: prefixes,
      operands: operands
    }
  }

  // Split the operand(s) string into individual operands as defined by the
  // instruction database.
  //
  // NOTE: X86/X64 doesn't require anything else than separating the commas,
  // this function is here for compatibility with other instruction sets.
  static splitOperands(s) {
    const array = s.split(",");
    for (let i = 0; i < array.length; i++)
      array[i] = array[i].trim();
    return array;
  }

  // Get whether the string `s` describes a register operand.
  static isRegOp(s) { return s && Object.hasOwn(CpuRegisters, s); }
  // Get whether the string `s` describes a memory operand.
  static isMemOp(s) { return s && /^(?:mem|mib|tmem|moff||(?:m(?:off)?\d+(?:dec|bcd|fp|int)?)|(?:m16_\d+)|(?:vm\d+(?:x|y|z)))$/.test(s); }
  // Get whether the string `s` describes an immediate operand.
  static isImmOp(s) { return s && /^(?:1|imm4|imm8|imm16|imm32|imm64|imms8|imms32|immu16|immu32|immv|if|p16_16|p16_32|dfv)$/.test(s); }
  // Get whether the string `s` describes a relative displacement (label).
  static isRelOp(s) { return s && /^rel\d+$/.test(s); }

  // Get a register type of a `s`, returns `null` if the register is unknown.
  static regTypeOf(s) { return Object.hasOwn(CpuRegisters, s) ? CpuRegisters[s].type : null; }
  // Get a register kind of a `s`, returns `null` if the register is unknown.
  static regKindOf(s) { return Object.hasOwn(CpuRegisters, s) ? CpuRegisters[s].kind : null; }
  // Get a register type of a `s`, returns `null` if the register is unknown and `-1`
  // if the given string does only represent a register type, but not a specific reg.
  static regIndexOf(s) { return Object.hasOwn(CpuRegisters, s) ? CpuRegisters[s].index : null; }

  static regSize(s) {
    if (s in RegSize)
      return RegSize[s];

    const reg = CpuRegisters[s];
    if (reg && reg.type in RegSize)
      return RegSize[reg.type];

    return -1;
  }

  // Get size of an immediate `s` [in bits].
  //
  // Handles "ib", "iw", "id", "if", "iq", and also "/is4".
  static immSize(s) {
    switch (s) {
      case "/is4"  : return 4;
      case "imm4"  : return 4;
      case "1"     : return 8;
      case "imm8"  : return 8;
      case "imm16" : return 16;
      case "imm32" : return 32;
      case "imm64" : return 64;
      case "imms8" : return 8;
      case "imms32": return 32;
      case "immu16": return 16;
      case "immu32": return 32;
      case "ib"    :
      case "ub"    : return 8;
      case "iw"    :
      case "uw"    : return 16;
      case "id"    :
      case "ud"    : return 32;
      case "iq"    :
      case "uq"    : return 64;
      case "p16_16": return 32;
      case "if"    :
      case "p16_32": return 48;

      // Influences EVEX encoding, not an immediate byte.
      case "dfv"   : return 0;

      // Invalid immediate.
      default      : FAIL(`Invalid immediate ${s}`);
    }
  }

  // Get size of a relative displacement [in bits].
  static relSize(s) {
    switch (s) {
      case "rel8"  : return 8;
      case "rel16" : return 16;
      case "rel32" : return 32;
      default      : return -1;
    }
  }
}
x86.Utils = Utils;

// asmdb.x86.Operand
// =================

// X86/X64 operand.
class Operand extends base.Operand {
  constructor() {
    super();

    this.groupPattern = "";    // Group pattern in case this operand was created from a group.
    this.memSegment = "";      // Segment specified with register that is used to perform a memory IO.
    this.memOff = false;       // Memory operand is an absolute offset (only a specific version of MOV).
    this.memFar = false;       // Memory is a far pointer (includes segment in first two bytes).
    this.vsibReg = "";         // AVX VSIB register type (xmm/ymm/zmm).
    this.vsibSize = -1;        // AVX VSIB register size (32/64).
    this.bcstSize = -1;        // AVX-512 broadcast size.
  }

  _substituteGroupOp(op, groupIndex) {
    const opPart = op.match(/^([A-Za-z]+)/);
    if (opPart) {
      const groupPattern = Utils.groupOf(opPart[1]);
      if (groupPattern) {
        this.groupPattern = groupPattern;
        return OperandGroupInfo[opPart[1]].subst[groupIndex] + op.substring(opPart[1].length);
      }
    }
    return op;
  }

  assignData(data, defaultAccess, groupIndex) {
    let s = data;
    this.data = data;

    const type = [];

    // Handle RWX decorators prefix "[RWwXx]:".
    let access = defaultAccess;
    const access_match = /^(R|W|w|X|x)(\?)?\:/.exec(s);
    if (access_match) {
      // TODO: Conditional access is ignored at the moment.
      access = access_match[1];
      s = s.substring(access_match[0].length);
    }

    // Handle commutativity attribute.
    if (Parsing.isCommutative(s)) {
      this.commutative = true;
      s = Parsing.clearCommutative(s);
    }

    // Handle AVX-512 broadcast possibility specified as "/bN" suffix.
    const mBcst = /\/b(\d+)/.exec(s);
    if (mBcst) {
      this.bcstSize = parseInt(mBcst[1], 10);

      // Remove the broadcast attribute from the definition; it's not needed anymore.
      s = s.substring(0, mBcst.index) + s.substring(mBcst.index + mBcst[0].length);
    }

    // Handle <implicit> attribute.
    if (Parsing.isImplicit(s)) {
      this.implicit = true;
      s = Parsing.clearImplicit(s);
    }

    // Support multiple operands separated by "/" (only used by r/m).
    let ops = s.split("/");
    let oArr = [];

    for (let i = 0; i < ops.length; i++) {
      let origOp = ops[i].trim();
      let op = this._substituteGroupOp(origOp, groupIndex);

      // Handle range suffix [A] or [A:B]:
      const mRange = /\[(\d+)\s*(?:\:\s*(\d+)\s*)?\]$/.exec(op);
      if (mRange) {
        const a = parseInt(mRange[1], 10);
        const b = parseInt(mRange[2] || String(a), 10);

        if (a < b)
          FAIL(`Operand '${origOp}' contains invalid range '[${a}:${b}]'`)

        this.rwxIndex = b;
        this.rwxWidth = a - b + 1;

        op = op.substring(0, op.length - mRange[0].length);
      }

      // Handle a segment specification if this is an implicit register performing memory access.
      const memSegRegM = op.match(/\((ds|es)\:\s*([\w]+)\)$/);
      if (memSegRegM) {
        this.memSegment = memSegRegM[1];
        this.memRegOnly = memSegRegM[2];
        op = op.substring(0, memSegRegM.index).trim();
      }

      oArr.push(op);

      let regIndexRel = 0;
      if (op.endsWith("+1") || op.endsWith("+2") || op.endsWith("+3")) {
        regIndexRel = parseInt(op.substr(op.length - 1, 1));
        op = op.substring(0, op.length - 2);
      }

      // Group substitution - when a rv/mv instruction uses 'w' or 'x' access it's only used by
      // the 16-bit form, 32-bit and 64-bit always use 'W' and 'X' when used in a 'rv/mv' group.
      if (this.groupPattern === "rv" && groupIndex > 0 && access !== "R") {
        access = access.toUpperCase();
      }

      if (Utils.isRegOp(op)) {
        this.reg = op;
        this.regType = Utils.regTypeOf(op);
        this.regIndexRel = regIndexRel;
        this.setAccess(access);

        type.push("reg");
        continue;
      }

      if (Utils.isMemOp(op)) {
        this.mem = op;
        this.setAccess(access);

        // Handle memory size.
        const mOff = /^m(?:off)?(\d+)/.exec(op);
        this.memSize = mOff ? parseInt(mOff[1], 10) : 0;
        this.memOff = op.indexOf("moff") === 0;

        const mSeg = /^m16_(\d+)/.exec(op);
        if (mSeg) {
          this.memFar = true;
          this.memSize = parseInt(mSeg[1], 10) + 16;
        }

        // Handle vector addressing mode and size "vmXXr".
        const mVM = /^vm(\d+)(x|y|z)$/.exec(op);
        if (mVM) {
          this.vsibReg = mVM[2] + "mm";
          this.vsibSize = parseInt(mVM[1], 10);
        }

        type.push("mem");
        continue;
      }

      if (Utils.isImmOp(op)) {
        const size = Utils.immSize(op);
        if (!this.imm)
          this.imm = size;
        else if (this.imm !== size)
          FAIL(`Immediate size mismatch: ${this.imm} != ${size}`);

        // Sign-extend / zero-extend.
        const sign = op.startsWith("imms") ? "signed" :
                     op.startsWith("immu") ? "unsigned" : "any";
        this.immSign = sign;

        if (op === "1") {
          this.immValue = 1;
          this.implicit = true;
        }

        if (type.indexOf("imm") !== -1)
          type.push("imm");
        continue;
      }

      if (Utils.isRelOp(op)) {
        this.rel = Utils.relSize(op);

        type.push("rel");
        continue;
      }

      FAIL(`Operand '${origOp}' unhandled`);
    }

    // In case the data has been modified it's always better to use the stripped off
    // version as we have already processed and stored all the possible decorators.
    this.data = oArr.join("/");
    this.type = type.join("/");

    if (this.rwxIndex === -1) {
      const opSize = this.isReg() ? this.regSize :
                     this.isMem() ? this.memSize : -1;
      if (opSize !== -1) {
        this.rwxIndex = 0;
        this.rwxWidth = opSize;
      }
    }
  }

  get regSize() {
    return Utils.regSize(this.reg);
  }

  setAccess(x) {
    const u = x.toUpperCase();
    this.zext  = x === "W" || x === "X";
    this.read  = u === "R" || u === "X";
    this.write = u === "W" || u === "X";
    return this;
  }


  isFixedReg() { return this.reg && this.reg !== this.regType && this.reg !== "st(i)"; }
  isFixedMem() { return this.memSegment && this.isFixedReg(); }

  isPartialOp() {
    const maybePartial = this.regType === "r8"   ||
                         this.regType === "r8hi" ||
                         this.regType === "r16"  ||
                         this.regType === "xmm";
    return maybePartial && !this.zext;
  }

  toRegMem() {
    if (this.reg && this.mem)
      return this.reg + "/m";
    else if (this.mem && (this.vsibReg || /fp$|int$/.test(this.mem)))
      return this.mem;
    else if (this.mem)
      return "m";
    else
      return this.toString();
  }

  toString() { return this.data; }
}
x86.Operand = Operand;

// asmdb.x86.Instruction
// =====================

// X86/X64 instruction.
class Instruction extends base.Instruction {
  constructor(db) {
    super(db);

    this.opcode = dict({
      byte : "",                  // Opcode byte (a single value specified as HEX string "00-FF").
      ri   : false,               // Instruction opcode is combined with register, "XX+r" or "XX+i".
      _67h : false,               // Opcode 67h prefix use.
      mm   : "",                  // Opcode MM[MMM] part (map).
      pp   : "",                  // Opcode PP part.
      w    : "",                  // Opcode W field.
      l    : "",                  // EVEX.LL (nothing, 128, 256, 512, LIG).
      nd   : 0,                   // EVEX.ND (new dest) field (default is false, specified as ND=0 or ND=1).
      nf   : 0,                   // EVEX.NF (no flags) field (default is false, specified as NF=0 or NF=1).
      scc  : "",                  // EVEX.SCC field (4 bits - condition flags).
      mod  : "",                  // MODRM.MOD part (2 bits) - either "xx", "11" or "!(11)".
      modr : "",                  // MODRM.R part (3 bits) - either "rrr"
      modrm: ""                   // MODRM.R/M part - either "bbb"
    });

    this.prefix = "";             // Prefix - "", "3DNOW", "EVEX", "VEX", "XOP".
    this.privilege = "L3";        // Privilege level required to execute the instruction.
    this.groupPattern = "";       // Group pattern in case the instruction was created from a group such as "ry", "rv", "xy", "xyz".
    this.groupIndex = -1;         // Group index.

    this.rel = 0;                 // Displacement ("cb", "cw", and "cd" parts).

    this.fpuTop = 0;              // FPU top index manipulation [-1, 0, 1, 2].
    this.fpuStack = "";           // FPU stack manipulation

    this.vsibReg = "";            // AVX VSIB register type (xmm/ymm/zmm).
    this.vsibSize = -1;           // AVX VSIB register size (32/64).

    this.broadcast = false;       // AVX-512 broadcast support.
    this.bcstSize = -1;           // AVX-512 broadcast size.

    this.k = "";                  // AVX-512 K function ("", "blend", "zeroing").
    this.kmask = false;           // AVX-512 merging {k}.
    this.zmask = false;           // AVX-512 zeroing {kz}, implies {k}.
    this.er = false;              // AVX-512 embedded rounding {er}, implies {sae}.
    this.sae = false;             // AVX-512 suppress all exceptions {sae} support.

    this.tupleType = "";          // AVX-512 tuple-type.
    this.elementSize = -1;        // Instruction's element size.
    this.encodingPreference = ""; // Encoding preference (either nothing or "EVEX").

    this.consecutiveLead = 0;     // Consecutive register leading N other registers.
    this.prefixes = dict();       // Allowed prefixes.
  }

  _substituteOpcodePart(op, groupIndex) {
    if (Object.hasOwn(OpcodeGroupInfo, op)) {
      return OpcodeGroupInfo[op].subst[groupIndex];
    }
    else {
      return op;
    }
  }

  assignData(data, groupIndex) {
    this.name = data.name;
    this.groupIndex = groupIndex;

    if (data.tt)
      this.tupleType = data.tt;

    const em = data.op.match(/^\[\s*(\w+)\s*\](.*)$/);
    const encodingField = em ? em[1] : "NONE";
    const opcodeField = em ? em[2] : data.op;

    this._assignOperands(data.operands, groupIndex);
    this._assignEncoding(encodingField);
    this._assignOpcode(opcodeField.trim(), groupIndex);

    for (let k in data) {
      if (k === "name" || k === "op" || k === "operands")
        continue;
      this._assignAttribute(k, data[k]);
    }

    this._updateOperandsInfo();
    this._postProcess();
  }

  _assignAttribute(key, value) {
    switch (key) {
      case "vl":
        if (value) {
          this.ext["AVX512_VL"] = true;
        }
        return;

      case "prefixes":
        this._combineAttribute("prefixes", value);
        return;

      case "fpuStack":
        this.fpuStack = value;
        switch (value) {
          case "dec"  : this.fpuTop = -1; break;
          case "inc"  : this.fpuTop =  1; break;
          case "pop"  : this.fpuTop =  1; break;
          case "pop2x": this.fpuTop =  2; break;
          case "push" : this.fpuTop = -1; break;
          default:
            FAIL(`Invalid fpuStack value '${value}'`);
        }
        return;

      case "kz":
        this.zmask = true;
        this.kmask = true;
        return;

      case "k":
        this.kmask = true;
        if (typeof value === "string")
          super._assignAttribute(key, value);
        return;

      case "er":
        this.er = true;
        this.sae = true; // {er} implies {sae}.
        return;

      case "sae":
        this.sae = true;
        return;

      case "broadcast":
        this.broadcast = true;
        this.elementSize = value;
        return;

      default:
        super._assignAttribute(key, value);
    }
  }

  _assignOperands(s, groupIndex) {
    if (!s) return;

    // First remove all flags specified as {...}. We put them into `flags`
    // map and mix with others. This seems to be the best we can do here.
    for (;;) {
      let a = s.indexOf("{");
      let b = s.indexOf("}");

      if (a === -1 || b === -1)
        break;

      // Get the `flag` and remove it from `s`.
      this._assignAttribute(s.substring(a + 1, b), true);
      s = s.substring(0, a) + s.substring(b + 1);
    }

    // Split into individual operands and push them to `operands`.
    const arr = Utils.splitOperands(s);
    for (let i = 0; i < arr.length; i++) {
      const operand = new Operand();
      operand.assignData(arr[i].trim(), i === 0 ? "X" : "R", groupIndex);

      if (operand.mem == "tmem") {
        this.tsib = true;
      }

      if (operand.groupPattern && this.groupPattern !== operand.groupPattern) {
        if (this.groupPattern) {
          FAIL(`Instruction ${this.name}: Operand's group pattern mismatch '${this.groupPattern}' != '${operand.groupPattern}'`);
        }
        this.groupPattern = operand.groupPattern;
      }

      this.operands.push(operand);
    }
  }

  _assignEncoding(s) {
    this.encoding = s;
  }

  _assignOpcode(s, groupIndex) {
    this.opcodeString = s;

    let parts = s.split(" ");

    if (/^(VEX|EVEX|XOP)\./.test(s)) {
      // Parse VEX/XOP and EVEX encoded instruction, which looks like "<PREFIX>.[APX-DATA].<LL>.<PP>.<MAP>.<W>"
      let prefix = parts[0].split(".");
      this.prefix = prefix[0];

      for (let i = 1; i < prefix.length; i++) {
        let comp = prefix[i];

        if (/^(Pv|Wv|Wy)$/.test(comp)) {
          comp = OpcodeGroupInfo[comp].subst[groupIndex];
        }

        // Process APX EVEX.ND field - ND=0 or ND=1.
        if (/^ND=[01]$/.test(comp)) {
          this.opcode.nd = comp === "ND=1";
          continue;
        }

        // Process APX EVEX.NF field - NF=0 or NF=1.
        if (/^NF=[01]$/.test(comp)) {
          this.opcode.nf = comp === "NF=1";
          continue;
        }

        // Process APX EVEX.SCC field - SCC=0-F
        if (/^SCC=[0-9A-F]$/.test(comp)) {
          this.opcode.scc = comp.charAt(5);
          continue;
        }

        // Process `L/LL` field.
        if (Object.hasOwn(OpcodeLLMapping, comp)) {
          this.opcode.l = OpcodeLLMapping[comp];
          continue;
        }

        // Process `PP` field - 66/F2/F3/NP (NP means no PP field used)
        if (comp === "P0") { /* ignored, `P` is zero... */ continue; }
        if (/^(?:66|F2|F3|NP)$/.test(comp)) { this.opcode.pp = comp; continue; }

        // Process `MM` field - 0F/0F3A/0F38/MAP4/MAP5/MAP6/M8/M9.
        if (/^(?:0F|0F3A|0F38|MAP[4-9A])$/.test(comp)) { this.opcode.mm = comp; continue; }

        // Process `W` field.
        if (/^(WIG|W0|W1|)$/.test(comp)) { this.opcode.w = comp; continue; }

        // TODO: Some new APX instructions don't have W specified (ENQCMD/ENQCMDS).
        if (comp === "W?") { this.opcode.w = "W0"; continue; }

        // ERROR.
        this.report(`'${this.opcodeString}' Unhandled component: ${comp}`);
      }

      for (let i = 1; i < parts.length; i++) {
        let comp = parts[i];

        // Parse opcode.
        if (/^[0-9A-Fa-f]{2}$/.test(comp)) {
          this.opcode.byte = comp.toUpperCase();
          continue;
        }

        // Parse ModR/M field using "/r" or "/0-7" notation.
        if (/^\/[r0-7]$/.test(comp)) {
          this.opcode.mod = "xx";
          this.opcode.modr = comp.charAt(1);
          this.opcode.modm = "b";
          continue;
        }

        // Parse ModR/M field using "11:xxx:xxx" and "!(11):xxx:xxx" notation.
        const m = comp.match(/^(11|!\(11\)):(rrr|[01]{3}):(bbb|[01]{3})$/);
        if (m) {
          this.opcode.mod = m[1];
          this.opcode.modr = m[2] === "rrr" ? "r" : String(parseInt(m[2], 2));
          this.opcode.modrm = m[3] === "bbb" ? "b" : String(parseInt(m[3], 2));
         continue;
        }

        // Parse immediate byte, word, dword, or qword.
        comp = this._substituteOpcodePart(comp, groupIndex);
        if (/^(?:ib|iw|id|iq|\/is4)$/.test(comp)) {
          this.imm += Utils.immSize(comp);
          continue;
        }

        this.report(`'${this.opcodeString}' Unhandled opcode component: ${comp}`);
      }
    }
    else {
      // Parse X86/X64 instruction (including legacy MMX/SSE/3DNOW instructions).
      let rex_parsed = false;

      for (let i = 0; i < parts.length; i++) {
        let comp = parts[i];

        if (comp === "NFx" || comp === "NOREP" || comp === "NO67") {
          // Ignored for now.
          continue;
        }

        // Parse REX or REX2 prefix.
        if (comp.startsWith("REX2.") || comp === "REX.W") {
          if (rex_parsed) {
            FAIL(`'${this.opcodeString}' Multiple REX prefixes are invalid`);
          }

          rex_parsed = true;

          // Instructions that force REX.W prefix or use REX2 prefix are always 64-bit instructions.
          this.arch = "X64";

          if (comp === "REX.W") {
            this.opcode.w = "W1";
          }
          else {
            this.prefix = "REX2";

            // REX2 has always 3 components - "REX2.<MAP>.<W>".
            const rex2 = comp.split(".");
            if (rex2.length !== 3) {
              FAIL(`'${this.opcodeString}' Invalid REX2 prefix - expected exactly 3 REX2 components`);
            }

            if (rex2[1] === "MAP0") {
              // nothing.
            }
            else if (rex2[1] === "MAP1") {
              this.opcode.mm = "0F";
            }
            else {
              FAIL(`'${this.opcodeString}' Invalid REX2 prefix - REX2.MAP component could be either MAP0 or MAP1`);
            }

            this.opcode.w = rex2[2];
          }

          continue;
        }

        // Parse `PP` prefixes.
        if (this.opcode.mm === "") {
          if (this.opcode.pp === ""   && /^(?:66|F2|F3|NP)$/.test(comp) ||
              this.opcode.pp === "66" && /^(?:F2|F3)$/.test(comp)) {
            this.opcode.pp += comp;
            continue;
          }
        }

        // Parse `MM` prefixes.
        if ((this.opcode.mm === ""   && comp === "0F") ||
            (this.opcode.mm === "0F" && /^(?:01|3A|38)$/.test(comp))) {
          this.opcode.mm += comp;
          continue;
        }

        // Recognize "0F 0F /r XX" encoding.
        if (this.opcode.mm === "0F" && comp === "0F") {
          this.prefix = "3DNOW";
          continue;
        }

        // Parse opcode byte.
        if (/^[0-9A-F]{2}(?:\+[ri])?$/.test(comp)) {
          // Parse "+r" or "+i" suffix.
          if (comp.length > 2) {
            this.opcode.ri = true;
            comp = comp.substring(0, 2);
          }

          // FPU instructions are encoded as "PREFIX XX", where prefix is not the same
          // as MM prefixes used everywhere else. AsmJit internally extends MM field in
          // instruction tables to allow storing this prefix together with other "MM"
          // prefixes, currently the unused indexes are used, but if X86 moves forward
          // and starts using these we can simply use more bits in the opcode DWORD.
          if (!this.opcode.pp && this.opcode.byte === "9B") {
            this.opcode.pp = this.opcode.byte;
            this.opcode.byte = comp;
            continue;
          }

          if (!this.opcode.mm && (/^(?:D8|D9|DA|DB|DC|DD|DE|DF)$/.test(this.opcode.byte))) {
            this.opcode.mm = this.opcode.byte;
            this.opcode.byte = comp;
            continue;
          }

          if (this.opcode.byte) {
            if (this.opcode.byte === "67") {
              this.opcode._67h = true;
            }
            else {
              if (!this.opcode.modr && !this.opcode.modrm) {
                const value = parseInt(comp, 16);
                if ((value & 0xC0) == 0xC0) {
                  this.opcode.mod = "11";
                  this.opcode.modr = String((value >> 3) & 0x7);
                  this.opcode.modrm = String((value >> 0) & 0x7);
                }
                else {
                  this.report(`'${this.opcodeString}' Unsupported secondary opcode (MOD/RM) '${comp}' value`);
                }
              }
              else {
                this.report(`'${this.opcodeString}' Multiple opcodes, have ${this.opcode.byte}, found ${comp}`);
              }
            }
          }

          this.opcode.byte = comp;
          continue;
        }

        // Parse ModR/M field using "/r" or "/0-7" notation.
        if (/^\/[r0-7]$/.test(comp) && !this.opcode.modr) {
          this.opcode.mod = "xx";
          this.opcode.modr = comp.charAt(1);
          this.opcode.modm = "b";
          continue;
        }

        // Parse ModR/M field using "11:xxx:xxx" and "!(11):xxx:xxx" notation.
        const m = comp.match(/^(11|!\(11\)):(rrr|[01]{3}):(bbb|[01]{3})$/);
        if (m) {
          this.opcode.mod = m[1];
          this.opcode.modr = m[2] === "rrr" ? "r" : String(parseInt(m[2], 2));
          this.opcode.modrm = m[3] === "bbb" ? "b" : String(parseInt(m[3], 2));
          continue;
        }

        // Parse immediate byte, word, dword, fword, or qword.
        if (/^(?:ib|iw|id|iq|iv|if)$/.test(comp)) {
          if (comp === "iv")
            comp = OpcodeGroupInfo[comp].subst[groupIndex];
          this.imm += Utils.immSize(comp);
          continue;
        }

        if (comp === "moff") {
          this.moff = true;
          continue;
        }

        // Parse displacement.
        if (/^(?:cb|cw|cd)$/.test(comp) && !this.rel) {
          this.rel = comp === "cb" ? 1 :
                     comp === "cw" ? 2 :
                     comp === "cd" ? 4 : -1;
          continue;
        }

        // ERROR.
        this.report(`'${this.opcodeString}' Unhandled opcode component: ${comp}`);
      }
    }

    // HACK: Fix instructions having opcode "01".
    if (this.opcode.byte === "" && this.opcode.mm.indexOf("0F01") === this.opcode.mm.length - 4) {
      this.opcode.byte = "01";
      this.opcode.mm = this.opcode.mm.substring(0, this.opcode.mm.length - 2);
    }

    if (this.opcode.byte)
      this.opcodeValue = parseInt(this.opcode.byte, 16);

    if (!this.opcode.byte)
      this.report(`Couldn't parse instruction's opcode '${this.opcodeString}'`);
  }

  _updateOperandsInfo() {
    super._updateOperandsInfo();

    let consecutiveLead = null;
    let consecutiveLastIndex = 0;

    for (let i = 0; i < this.operands.length; i++) {
      const op = this.operands[i];

      // Instructions that use 64-bit GP registers are always 64-bit instructions.
      if (op.reg === "r64" || op.reg === "rax" || op.reg === "rbx" || op.reg === "rcx" || op.reg === "rdx" || op.reg === "rsi" || op.reg === "rdi")
        this.arch = "X64";

      // Propagate broadcast.
      if (op.bcstSize > 0)
        this._assignAttribute("broadcast", op.bcstSize);

      // Propagate VSIB.
      if (op.vsibReg) {
        if (this.vsibReg) {
          this.report("Only one operand can be a vector memory address (vmNNx)");
        }

        this.vsibReg = op.vsibReg;
        this.vsibSize = op.vsibSize;
      }

      if (op.regIndexRel) {
        if (i - op.regIndexRel < 0) {
          this.report(`The consecutive register information is invalid, index of the lead (${i - op.regIndexRel}) is out of range`);
        }
        else {
          const lead = this.operands[i - op.regIndexRel];
          if (consecutiveLead && consecutiveLead != lead) {
            this.report(`The consecutive register chain is invalid`);
          }
          else {
            consecutiveLead = lead;
            consecutiveLastIndex = Math.max(consecutiveLastIndex, op.regIndexRel);
          }
        }
      }
    }

    if (consecutiveLead) {
      consecutiveLead.consecutive_lead_count = consecutiveLastIndex + 1;
    }
  }

  // Validate the instruction's definition. Common mistakes can be checked and
  // reported easily, however, if the mistake is just an invalid opcode or
  // something else it's impossible to detect.
  _postProcess() {
    if (this.groupPattern) {
      const archInfo = ArchGroupInfo[this.groupPattern];
      if (this.arch === "ANY" && archInfo && this.arch !== archInfo[this.groupIndex]) {
        // TODO: Never triggered, which means it should be removed.
        this.arch = archInfo[this.groupIndex];
      }
    }
    else {
      this.groupIndex = -1;
    }

    if (this.privilege === "L0")
      this.category.SYSTEM = true;

    let immCount = this.immCount;

    // Verify that the immediate operand/operands are specified in instruction
    // encoding and opcode field. Basically if there is an "ix" in operands,
    // the encoding should contain "I".
    if (immCount > 0) {
      if (immCount === 1 && this.operands[this.operands.length - 1].data === "1") {
        // This must be one of rcl|rcr|rol|ror|sar|sal|shr. We won't validate
        // these as these have "1" as implicit (encoded within opcode, not after).
      }
      else {
        // Every immediate should have its imm byte ("ib", "iw", "id", or "iq") in the opcode data.
        let m = this.opcodeString.match(/(?:^|\s+)(ib|iw|id|iq|iv|if|\/is4)/g);
        if (!m || m.length !== immCount) {
          this.report(`Immediate(s) [${immCount}] not found in opcode: ${this.opcodeString}`);
        }
      }
    }
  }

  isAVX() { return this.isVEX() || this.isEVEX(); }
  isVEX() { return this.prefix === "VEX" || this.prefix === "XOP"; }
  isEVEX() { return this.prefix === "EVEX" }

  getWValue() {
    switch (this.opcode.w) {
      case "W0": return 0;
      case "W1": return 1;
    }
    return -1;
  }

  // Get signature of the instruction as "ARCH PREFIX ENCODING[:operands]" form.
  get signature() {
    let operands = this.operands;
    let sign = this.arch;

    if (this.prefix) {
      sign += " " + this.prefix;
      if (this.prefix !== "3DNOW") {
        if (this.opcode.l === "L1")
          sign += ".256";
        else if (this.opcode.l === "256" || this.opcode.l === "512")
          sign += `.${this.opcode.l}`;
        else
          sign += ".128";

        if (this.opcode.w === "W1")
          sign += ".W";
      }
    }
    else if (this.opcode.w === "W1") {
      sign += " REX.W";
    }

    sign += " " + this.encoding;

    for (let i = 0; i < operands.length; i++) {
      sign += (i === 0) ? ":" : ",";

      let operand = operands[i];
      if (operand.implicit)
        sign += `[${operand.reg}]`;
      else
        sign += operand.toRegMem();
    }

    return sign;
  }

  get immCount() {
    let ops = this.operands;
    let n = 0;
    for (let i = 0; i < ops.length; i++)
      if (ops[i].isImm())
        n++;
    return n;
  }

  get modRValue() {
    if (/^[0-7]$/.test(this.opcode.modr))
      return parseInt(this.opcode.modr, 10);
    else
      return 0;
  }

  get modRMValue() {
    if (/^[0-7]$/.test(this.opcode.modrm))
      return parseInt(this.opcode.modrm, 10);
    else
      return 0;
  }
}
x86.Instruction = Instruction;

// asmdb.x86.ISA
// =============

const ArchKeys = MapUtils.mapFromArray(["any", "x86", "x64", "apx", "___"]);

function findArch(inst) {
  for (let a in ArchKeys) {
    if (typeof inst[a] === "string") {
      return a;
    }
  }

  FAIL(`Instruction signature not found in record: ${JSON.stringify(inst)}`);
}

function mergeGroupData(data, group) {
  for (let k in group) {
    switch (k) {
      case "group":
      case "instructions":
        break;

      case "ext":
        data[k] = (data[k] ? data[k] + " " : "") + group[k];
        break;

      default:
        if (data[k] === undefined)
          data[k] = group[k]
        break;
    }
  }
}

// X86/X64 instruction database - stores Instruction instances in a map and
// aggregates all instructions with the same name.
class ISA extends base.ISA {
  constructor(data) {
    super(data);
    this.addData(data || NONE);
  }

  _addInstructions(groups) {
    for (let group of groups) {
      for (let record of group.instructions) {
        let arch = findArch(record);

        // TODO: Ignore records having this (only used for testing purposes).
        if (arch === "___")
          continue;

        const apx = arch === "apx";

        const sgn = Utils.splitInstructionSignature(record[arch]);
        const data = MapUtils.cloneExcept(record, arch);

        mergeGroupData(data, group)

        for (let j = 0; j < sgn.names.length; j++) {
          data.name = sgn.names[j];
          data.prefixes = sgn.prefixes;
          data.operands = sgn.operands;

          if (j > 0) {
            data.aliasOf = sgn.names[0];
          }

          let groupIndex = 0;
          let instruction = null;
          do {
            instruction = new Instruction(this);
            instruction.arch = apx ? "X64" : arch.toUpperCase();
            instruction.assignData(data, groupIndex);

            if (apx) {
              instruction.ext["APX_F"] = true;
              if (instruction.category.GP) {
                instruction.category.GP_EXT = true
              }
            }

            this._addInstruction(instruction);
          } while (instruction.groupPattern && ++groupIndex < OperandGroupInfo[instruction.groupPattern].subst.length);
        }
      }
    }

    return this;
  }
}
x86.ISA = ISA;

// asmdb.x86.X86DataCheck
// ======================

class X86DataCheck {
  static checkVexEvex(db) {
    const map = db.instructionMap;
    for (let name in map) {
      const instructions = map[name];
      for (let i = 0; i < instructions.length; i++) {
        const instA = instructions[i];
        for (let j = i + 1; j < instructions.length; j++) {
          const instB = instructions[j];
          if (instA.operands.join("_") === instB.operands.join("_")) {
            const vex  = instA.prefix === "VEX"  ? instA : instB.prefix === "VEX"  ? instB : null;
            const evex = instA.prefix === "EVEX" ? instA : instB.prefix === "EVEX" ? instB : null;

            if (vex && evex && vex.opcode.byte === evex.opcode.byte) {
              // NOTE: There are some false positives, they will be printed as well.
              let ok = vex.opcode.w === evex.opcode.w && vex.opcode.l === evex.opcode.l;

              if (!ok) {
                console.log(`Instruction ${name} differs:`);
                console.log(`  ${vex.operands.join(" ")}: ${vex.opcodeString}`);
                console.log(`  ${evex.operands.join(" ")}: ${evex.opcodeString}`);
              }
            }
          }
        }
      }
    }
  }
}
x86.X86DataCheck = X86DataCheck;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "x86"]);
