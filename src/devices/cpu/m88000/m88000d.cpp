// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    M88000 family disassembler

    The first (MC88100) and second (MC88110) generations of this RISC
    architecture differ in several ways. Though the MC88110 instruction
    set is a mostly backward-compatible extension to that of the MC88100,
    it deletes some load/store instruction forms (mostly to make way for
    extended register file support). The D, S1 and/or S2 fields are also
    not decoded on the MC88100 for instructions that don't use them, but
    the MC88110 documentation makes no such guarantees; the MC88110
    disassembler has been made accordingly stricter.

    Of the 32 registers in the general-purpose register file, two have
    hardware-defined properties: r0 always contains zero (no matter what
    data is written to it) and r1 stores the subroutine return pointer
    for bsr and jsr. r30 may be used as a frame pointer and r31 may be
    used as a stack pointer, but this is merely a calling convention.

    Though data memory accesses can be configured to use little-endian
    byte ordering by setting bit 30 of PSR, this has no effect on
    instruction decoding.

    The MC88100 executes integer multiplication instructions by using the
    floating-point multiply pipeline. This is not the case on the MC88110.

    The extended register file added to the MC88110 is used mainly for
    floating-point operations. Numbers of any precision can be stored in
    any of these 80-bit registers except x0, which is fixed to contain
    (positive) zero. The 80-bit double-extended-precision values are
    padded out to 128 bits when stored in memory.

***************************************************************************/

#include "emu.h"
#include "m88000d.h"

m88000_disassembler::m88000_disassembler(const std::map<u32, const m88000_disassembler::instruction> &ops)
	: util::disasm_interface()
	, m_ops(ops)
{
}

mc88100_disassembler::mc88100_disassembler()
	: m88000_disassembler(s_ops)
{
}

mc88110_disassembler::mc88110_disassembler()
	: m88000_disassembler(s_ops)
{
}

const std::map<u32, const mc88100_disassembler::instruction> mc88100_disassembler::s_ops =
{
	{ 0x00000000, { 0xfc000000, "xmem.bu",     mc88100_disassembler::addressing::IMM16 }},
	{ 0x04000000, { 0xfc000000, "xmem",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x08000000, { 0xfc000000, "ld.hu",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x0c000000, { 0xfc000000, "ld.bu",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x10000000, { 0xfc000000, "ld.d",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x14000000, { 0xfc000000, "ld",          mc88100_disassembler::addressing::IMM16 }},
	{ 0x18000000, { 0xfc000000, "ld.h",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x1c000000, { 0xfc000000, "ld.b",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x20000000, { 0xfc000000, "st.d",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x24000000, { 0xfc000000, "st",          mc88100_disassembler::addressing::IMM16 }},
	{ 0x28000000, { 0xfc000000, "st.h",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x2c000000, { 0xfc000000, "st.b",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x30000000, { 0xfc000000, "lda.d",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x34000000, { 0xfc000000, "lda",         mc88100_disassembler::addressing::IMM16 }},
	{ 0x38000000, { 0xfc000000, "lda.h",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x3c000000, { 0xfc000000, "lda.b",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x40000000, { 0xfc000000, "and",         mc88100_disassembler::addressing::IMM16 }},
	{ 0x44000000, { 0xfc000000, "and.u",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x48000000, { 0xfc000000, "mask",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x4c000000, { 0xfc000000, "mask.u",      mc88100_disassembler::addressing::IMM16 }},
	{ 0x50000000, { 0xfc000000, "xor",         mc88100_disassembler::addressing::IMM16 }},
	{ 0x54000000, { 0xfc000000, "xor.u",       mc88100_disassembler::addressing::IMM16 }},
	{ 0x58000000, { 0xfc000000, "or",          mc88100_disassembler::addressing::IMM16 }},
	{ 0x5c000000, { 0xfc000000, "or.u",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x60000000, { 0xfc000000, "addu",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x64000000, { 0xfc000000, "subu",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x68000000, { 0xfc000000, "divu",        mc88100_disassembler::addressing::IMM16 }},
	{ 0x6c000000, { 0xfc000000, "mul",         mc88100_disassembler::addressing::IMM16 }},
	{ 0x70000000, { 0xfc000000, "add",         mc88100_disassembler::addressing::SIMM16 }},
	{ 0x74000000, { 0xfc000000, "sub",         mc88100_disassembler::addressing::SIMM16 }},
	{ 0x78000000, { 0xfc000000, "div",         mc88100_disassembler::addressing::SIMM16 }},
	{ 0x7c000000, { 0xfc000000, "cmp",         mc88100_disassembler::addressing::SIMM16 }},
	{ 0x80004000, { 0xfc00f800, "ldcr",        mc88100_disassembler::addressing::CR }}, // privileged; S1, S2 not used
	{ 0x80004800, { 0xfc00f800, "fldcr",       mc88100_disassembler::addressing::CR }}, // S1, S2 not used
	{ 0x80008000, { 0xfc00f800, "stcr",        mc88100_disassembler::addressing::CR }}, // privileged; D not used
	{ 0x80008800, { 0xfc00f800, "fstcr",       mc88100_disassembler::addressing::CR }}, // D not used
	{ 0x8000c000, { 0xfc00f800, "xcr",         mc88100_disassembler::addressing::CR }}, // privileged
	{ 0x8000c800, { 0xfc00f800, "fxcr",        mc88100_disassembler::addressing::CR }},
	{ 0x84000000, { 0xfc00ffe0, "fmul.sss",    mc88100_disassembler::addressing::FP }},
	{ 0x84000020, { 0xfc00ffe0, "fmul.dss",    mc88100_disassembler::addressing::FP }},
	{ 0x84000080, { 0xfc00ffe0, "fmul.ssd",    mc88100_disassembler::addressing::FP }},
	{ 0x840000a0, { 0xfc00ffe0, "fmul.dsd",    mc88100_disassembler::addressing::FP }},
	{ 0x84000200, { 0xfc00ffe0, "fmul.sds",    mc88100_disassembler::addressing::FP }},
	{ 0x84000220, { 0xfc00ffe0, "fmul.dds",    mc88100_disassembler::addressing::FP }},
	{ 0x84000280, { 0xfc00ffe0, "fmul.sdd",    mc88100_disassembler::addressing::FP }},
	{ 0x840002a0, { 0xfc00ffe0, "fmul.ddd",    mc88100_disassembler::addressing::FP }},
	{ 0x84002000, { 0xfc1fffe0, "flt.ss",      mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84002020, { 0xfc1fffe0, "flt.ds",      mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84002800, { 0xfc00ffe0, "fadd.sss",    mc88100_disassembler::addressing::FP }},
	{ 0x84002820, { 0xfc00ffe0, "fadd.dss",    mc88100_disassembler::addressing::FP }},
	{ 0x84002880, { 0xfc00ffe0, "fadd.ssd",    mc88100_disassembler::addressing::FP }},
	{ 0x840028a0, { 0xfc00ffe0, "fadd.dsd",    mc88100_disassembler::addressing::FP }},
	{ 0x84002a00, { 0xfc00ffe0, "fadd.sds",    mc88100_disassembler::addressing::FP }},
	{ 0x84002a20, { 0xfc00ffe0, "fadd.dds",    mc88100_disassembler::addressing::FP }},
	{ 0x84002a80, { 0xfc00ffe0, "fadd.sdd",    mc88100_disassembler::addressing::FP }},
	{ 0x84002aa0, { 0xfc00ffe0, "fadd.ddd",    mc88100_disassembler::addressing::FP }},
	{ 0x84003000, { 0xfc00ffe0, "fsub.sss",    mc88100_disassembler::addressing::FP }},
	{ 0x84003020, { 0xfc00ffe0, "fsub.dss",    mc88100_disassembler::addressing::FP }},
	{ 0x84003080, { 0xfc00ffe0, "fsub.ssd",    mc88100_disassembler::addressing::FP }},
	{ 0x840030a0, { 0xfc00ffe0, "fsub.dsd",    mc88100_disassembler::addressing::FP }},
	{ 0x84003200, { 0xfc00ffe0, "fsub.sds",    mc88100_disassembler::addressing::FP }},
	{ 0x84003220, { 0xfc00ffe0, "fsub.dds",    mc88100_disassembler::addressing::FP }},
	{ 0x84003280, { 0xfc00ffe0, "fsub.sdd",    mc88100_disassembler::addressing::FP }},
	{ 0x840032a0, { 0xfc00ffe0, "fsub.ddd",    mc88100_disassembler::addressing::FP }},
	{ 0x84003800, { 0xfc00ffe0, "fcmp.sss",    mc88100_disassembler::addressing::FP }},
	{ 0x84003880, { 0xfc00ffe0, "fcmp.ssd",    mc88100_disassembler::addressing::FP }},
	{ 0x84003a00, { 0xfc00ffe0, "fcmp.sds",    mc88100_disassembler::addressing::FP }},
	{ 0x84003a80, { 0xfc00ffe0, "fcmp.sdd",    mc88100_disassembler::addressing::FP }},
	{ 0x84004800, { 0xfc1fffe0, "int.ss",      mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84004880, { 0xfc1fffe0, "int.sd",      mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84005000, { 0xfc1fffe0, "nint.ss",     mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84005080, { 0xfc1fffe0, "nint.sd",     mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84005800, { 0xfc1fffe0, "trnc.ss",     mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84005880, { 0xfc1fffe0, "trnc.sd",     mc88100_disassembler::addressing::FP }}, // S1 not used
	{ 0x84007000, { 0xfc00ffe0, "fdiv.sss",    mc88100_disassembler::addressing::FP }},
	{ 0x84007020, { 0xfc00ffe0, "fdiv.dss",    mc88100_disassembler::addressing::FP }},
	{ 0x84007080, { 0xfc00ffe0, "fdiv.ssd",    mc88100_disassembler::addressing::FP }},
	{ 0x840070a0, { 0xfc00ffe0, "fdiv.dsd",    mc88100_disassembler::addressing::FP }},
	{ 0x84007200, { 0xfc00ffe0, "fdiv.sds",    mc88100_disassembler::addressing::FP }},
	{ 0x84007220, { 0xfc00ffe0, "fdiv.dds",    mc88100_disassembler::addressing::FP }},
	{ 0x84007280, { 0xfc00ffe0, "fdiv.sdd",    mc88100_disassembler::addressing::FP }},
	{ 0x840072a0, { 0xfc00ffe0, "fdiv.ddd",    mc88100_disassembler::addressing::FP }},
	{ 0xc0000000, { 0xfc000000, "br",          mc88100_disassembler::addressing::D26 }},
	{ 0xc4000000, { 0xfc000000, "br.n",        mc88100_disassembler::addressing::D26 }},
	{ 0xc8000000, { 0xfc000000, "bsr",         mc88100_disassembler::addressing::D26 }},
	{ 0xcc000000, { 0xfc000000, "bsr.n",       mc88100_disassembler::addressing::D26 }},
	{ 0xd0000000, { 0xfc000000, "bb0",         mc88100_disassembler::addressing::D16 }}, // bit test
	{ 0xd4000000, { 0xfc000000, "bb0.n",       mc88100_disassembler::addressing::D16 }}, // bit test
	{ 0xd8000000, { 0xfc000000, "bb1",         mc88100_disassembler::addressing::D16 }}, // bit test
	{ 0xdc000000, { 0xfc000000, "bb1.n",       mc88100_disassembler::addressing::D16 }}, // bit test
	{ 0xe8000000, { 0xfc000000, "bcnd",        mc88100_disassembler::addressing::D16 }}, // conditional test
	{ 0xec000000, { 0xfc000000, "bcnd.n",      mc88100_disassembler::addressing::D16 }}, // conditional test
	{ 0xf0008000, { 0xfc00fc00, "clr",         mc88100_disassembler::addressing::BITFIELD }},
	{ 0xf0008800, { 0xfc00fc00, "set",         mc88100_disassembler::addressing::BITFIELD }},
	{ 0xf0009000, { 0xfc00fc00, "ext",         mc88100_disassembler::addressing::BITFIELD }},
	{ 0xf0009800, { 0xfc00fc00, "extu",        mc88100_disassembler::addressing::BITFIELD }},
	{ 0xf000a000, { 0xfc00fc00, "mak",         mc88100_disassembler::addressing::BITFIELD }},
	{ 0xf000a800, { 0xfc00fc00, "rot",         mc88100_disassembler::addressing::BITFIELD }}, // W5 not used
	{ 0xf000d000, { 0xfc00fe00, "tb0",         mc88100_disassembler::addressing::VEC9 }}, // bit test
	{ 0xf000d800, { 0xfc00fe00, "tb1",         mc88100_disassembler::addressing::VEC9 }}, // bit test
	{ 0xf000e800, { 0xfc00fe00, "tcnd",        mc88100_disassembler::addressing::VEC9 }}, // conditional test
	{ 0xf4000000, { 0xfc00ffe0, "xmem.bu",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4000100, { 0xfc00ffe0, "xmem.bu.usr", mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4000200, { 0xfc00ffe0, "xmem.bu",     mc88100_disassembler::addressing::SCALED }},
	{ 0xf4000300, { 0xfc00ffe0, "xmem.bu.usr", mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4000400, { 0xfc00ffe0, "xmem",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4000500, { 0xfc00ffe0, "xmem.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4000600, { 0xfc00ffe0, "xmem",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4000700, { 0xfc00ffe0, "xmem.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4000800, { 0xfc00ffe0, "ld.hu",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4000900, { 0xfc00ffe0, "ld.hu.usr",   mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4000a00, { 0xfc00ffe0, "ld.hu",       mc88100_disassembler::addressing::SCALED }},
	{ 0xf4000b00, { 0xfc00ffe0, "ld.hu.usr",   mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4000c00, { 0xfc00ffe0, "ld.bu",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4000d00, { 0xfc00ffe0, "ld.bu.usr",   mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4000e00, { 0xfc00ffe0, "ld.bu",       mc88100_disassembler::addressing::SCALED }},
	{ 0xf4000f00, { 0xfc00ffe0, "ld.bu.usr",   mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4001000, { 0xfc00ffe0, "ld.d",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4001100, { 0xfc00ffe0, "ld.d.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4001200, { 0xfc00ffe0, "ld.d",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4001300, { 0xfc00ffe0, "ld.d.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4001400, { 0xfc00ffe0, "ld",          mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4001500, { 0xfc00ffe0, "ld.usr",      mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4001600, { 0xfc00ffe0, "ld",          mc88100_disassembler::addressing::SCALED }},
	{ 0xf4001700, { 0xfc00ffe0, "ld.usr",      mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4001800, { 0xfc00ffe0, "ld.h",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4001900, { 0xfc00ffe0, "ld.h.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4001a00, { 0xfc00ffe0, "ld.h",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4001b00, { 0xfc00ffe0, "ld.h.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4001c00, { 0xfc00ffe0, "ld.b",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4001d00, { 0xfc00ffe0, "ld.b.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4001e00, { 0xfc00ffe0, "ld.b",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4001f00, { 0xfc00ffe0, "ld.b.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4002000, { 0xfc00ffe0, "st.d",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4002100, { 0xfc00ffe0, "st.d.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4002200, { 0xfc00ffe0, "st.d",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4002300, { 0xfc00ffe0, "st.d.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4002400, { 0xfc00ffe0, "st",          mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4002500, { 0xfc00ffe0, "st.usr",      mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4002600, { 0xfc00ffe0, "st",          mc88100_disassembler::addressing::SCALED }},
	{ 0xf4002700, { 0xfc00ffe0, "st.usr",      mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4002800, { 0xfc00ffe0, "st.h",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4002900, { 0xfc00ffe0, "st.h.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4002a00, { 0xfc00ffe0, "st.h",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4002b00, { 0xfc00ffe0, "st.h.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4002c00, { 0xfc00ffe0, "st.b",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4002d00, { 0xfc00ffe0, "st.b.usr",    mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4002e00, { 0xfc00ffe0, "st.b",        mc88100_disassembler::addressing::SCALED }},
	{ 0xf4002f00, { 0xfc00ffe0, "st.b.usr",    mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4003000, { 0xfc00ffe0, "lda.d",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4003100, { 0xfc00ffe0, "lda.d.usr",   mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4003200, { 0xfc00ffe0, "lda.d",       mc88100_disassembler::addressing::SCALED }},
	{ 0xf4003300, { 0xfc00ffe0, "lda.d.usr",   mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4003400, { 0xfc00ffe0, "lda",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4003500, { 0xfc00ffe0, "lda.usr",     mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4003600, { 0xfc00ffe0, "lda",         mc88100_disassembler::addressing::SCALED }},
	{ 0xf4003700, { 0xfc00ffe0, "lda.usr",     mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4003800, { 0xfc00ffe0, "lda.h",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4003900, { 0xfc00ffe0, "lda.h.usr",   mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4003a00, { 0xfc00ffe0, "lda.h",       mc88100_disassembler::addressing::SCALED }},
	{ 0xf4003b00, { 0xfc00ffe0, "lda.h.usr",   mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4003c00, { 0xfc00ffe0, "lda.b",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4003d00, { 0xfc00ffe0, "lda.b.usr",   mc88100_disassembler::addressing::TRIADIC }}, // privileged
	{ 0xf4003e00, { 0xfc00ffe0, "lda.b",       mc88100_disassembler::addressing::SCALED }},
	{ 0xf4003f00, { 0xfc00ffe0, "lda.b.usr",   mc88100_disassembler::addressing::SCALED }}, // privileged
	{ 0xf4004000, { 0xfc00ffe0, "and",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4004400, { 0xfc00ffe0, "and.c",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4005000, { 0xfc00ffe0, "xor",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4005400, { 0xfc00ffe0, "xor.c",       mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4005800, { 0xfc00ffe0, "or",          mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4005c00, { 0xfc00ffe0, "or.c",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006000, { 0xfc00ffe0, "addu",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006100, { 0xfc00ffe0, "addu.co",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006200, { 0xfc00ffe0, "addu.ci",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006300, { 0xfc00ffe0, "addu.cio",    mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006400, { 0xfc00ffe0, "subu",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006500, { 0xfc00ffe0, "subu.co",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006600, { 0xfc00ffe0, "subu.ci",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006700, { 0xfc00ffe0, "subu.cio",    mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006800, { 0xfc00fce0, "divu",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4006c00, { 0xfc00fce0, "mul",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007000, { 0xfc00ffe0, "add",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007100, { 0xfc00ffe0, "add.co",      mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007200, { 0xfc00ffe0, "add.ci",      mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007300, { 0xfc00ffe0, "add.cio",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007400, { 0xfc00ffe0, "sub",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007500, { 0xfc00ffe0, "sub.co",      mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007600, { 0xfc00ffe0, "sub.ci",      mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007700, { 0xfc00ffe0, "sub.cio",     mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007800, { 0xfc00fce0, "div",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4007c00, { 0xfc00fce0, "cmp",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4008000, { 0xfc00ffe0, "clr",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4008800, { 0xfc00ffe0, "set",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4009000, { 0xfc00ffe0, "ext",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf4009800, { 0xfc00ffe0, "extu",        mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf400a000, { 0xfc00ffe0, "mak",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf400a800, { 0xfc00ffe0, "rot",         mc88100_disassembler::addressing::TRIADIC }},
	{ 0xf400c000, { 0xfc00ffe0, "jmp",         mc88100_disassembler::addressing::JUMP }},
	{ 0xf400c400, { 0xfc00ffe0, "jmp.n",       mc88100_disassembler::addressing::JUMP }},
	{ 0xf400c800, { 0xfc00ffe0, "jsr",         mc88100_disassembler::addressing::JUMP }},
	{ 0xf400cc00, { 0xfc00ffe0, "jsr.n",       mc88100_disassembler::addressing::JUMP }},
	{ 0xf400e000, { 0xfc00ffe0, "ff1",         mc88100_disassembler::addressing::TRIADIC }}, // S1 not used
	{ 0xf400e800, { 0xfc00ffe0, "ff0",         mc88100_disassembler::addressing::TRIADIC }}, // S1 not used
	{ 0xf400f800, { 0xfc00ffe0, "tbnd",        mc88100_disassembler::addressing::TRIADIC }}, // D not used
	{ 0xf400fc00, { 0xfc00ffe0, "rte",         mc88100_disassembler::addressing::NONE }}, // privileged
	{ 0xf8000000, { 0xfc000000, "tbnd",        mc88100_disassembler::addressing::IMM16 }} // D not used
};

const std::map<u32, const mc88110_disassembler::instruction> mc88110_disassembler::s_ops =
{
	{ 0x00000000, { 0xfc000000, "ld.d",        mc88110_disassembler::addressing::SI16_XRF }},
	{ 0x04000000, { 0xfc000000, "ld",          mc88110_disassembler::addressing::SI16_XRF }},
	{ 0x08000000, { 0xfc000000, "ld.hu",       mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x0c000000, { 0xfc000000, "ld.bu",       mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x10000000, { 0xfc000000, "ld.d",        mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x14000000, { 0xfc000000, "ld",          mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x18000000, { 0xfc000000, "ld.h",        mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x1c000000, { 0xfc000000, "ld.b",        mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x20000000, { 0xfc000000, "st.d",        mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x24000000, { 0xfc000000, "st",          mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x28000000, { 0xfc000000, "st.h",        mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x2c000000, { 0xfc000000, "st.b",        mc88110_disassembler::addressing::SI16_GRF }},
	{ 0x30000000, { 0xfc000000, "st.d",        mc88110_disassembler::addressing::SI16_XRF }},
	{ 0x34000000, { 0xfc000000, "st",          mc88110_disassembler::addressing::SI16_XRF }},
	{ 0x38000000, { 0xfc000000, "st.x",        mc88110_disassembler::addressing::SI16_XRF }},
	{ 0x3c000000, { 0xfc000000, "ld.x",        mc88110_disassembler::addressing::SI16_XRF }},
	{ 0x40000000, { 0xfc000000, "and",         mc88110_disassembler::addressing::IMM16 }},
	{ 0x44000000, { 0xfc000000, "and.u",       mc88110_disassembler::addressing::IMM16 }},
	{ 0x48000000, { 0xfc000000, "mask",        mc88110_disassembler::addressing::IMM16 }},
	{ 0x4c000000, { 0xfc000000, "mask.u",      mc88110_disassembler::addressing::IMM16 }},
	{ 0x50000000, { 0xfc000000, "xor",         mc88110_disassembler::addressing::IMM16 }},
	{ 0x54000000, { 0xfc000000, "xor.u",       mc88110_disassembler::addressing::IMM16 }},
	{ 0x58000000, { 0xfc000000, "or",          mc88110_disassembler::addressing::IMM16 }},
	{ 0x5c000000, { 0xfc000000, "or.u",        mc88110_disassembler::addressing::IMM16 }},
	{ 0x60000000, { 0xfc000000, "addu",        mc88110_disassembler::addressing::IMM16 }},
	{ 0x64000000, { 0xfc000000, "subu",        mc88110_disassembler::addressing::IMM16 }},
	{ 0x68000000, { 0xfc000000, "divu",        mc88110_disassembler::addressing::IMM16 }},
	{ 0x6c000000, { 0xfc000000, "mulu",        mc88110_disassembler::addressing::IMM16 }},
	{ 0x70000000, { 0xfc000000, "add",         mc88110_disassembler::addressing::SIMM16 }},
	{ 0x74000000, { 0xfc000000, "sub",         mc88110_disassembler::addressing::SIMM16 }},
	{ 0x78000000, { 0xfc000000, "divs",        mc88110_disassembler::addressing::SIMM16 }},
	{ 0x7c000000, { 0xfc000000, "cmp",         mc88110_disassembler::addressing::SIMM16 }},
	{ 0x80004000, { 0xfc1ff81f, "ldcr",        mc88110_disassembler::addressing::CR }},
	{ 0x80004800, { 0xfc1ff81f, "fldcr",       mc88110_disassembler::addressing::CR }},
	{ 0x80008000, { 0xffe0f800, "stcr",        mc88110_disassembler::addressing::CR }},
	{ 0x80008800, { 0xffe0f800, "fstcr",       mc88110_disassembler::addressing::CR }},
	{ 0x8000c000, { 0xfc00f800, "xcr",         mc88110_disassembler::addressing::CR }},
	{ 0x8000c800, { 0xfc00f800, "fxcr",        mc88110_disassembler::addressing::CR }},
	{ 0x84000000, { 0xfc007fe0, "fmul.sss",    mc88110_disassembler::addressing::FP }},
	{ 0x84000020, { 0xfc007fe0, "fmul.dss",    mc88110_disassembler::addressing::FP }},
	{ 0x84000080, { 0xfc007fe0, "fmul.ssd",    mc88110_disassembler::addressing::FP }},
	{ 0x840000a0, { 0xfc007fe0, "fmul.dsd",    mc88110_disassembler::addressing::FP }},
	{ 0x84000200, { 0xfc007fe0, "fmul.sds",    mc88110_disassembler::addressing::FP }},
	{ 0x84000220, { 0xfc007fe0, "fmul.dds",    mc88110_disassembler::addressing::FP }},
	{ 0x84000280, { 0xfc007fe0, "fmul.sdd",    mc88110_disassembler::addressing::FP }},
	{ 0x840002a0, { 0xfc007fe0, "fmul.ddd",    mc88110_disassembler::addressing::FP }},
	{ 0x84000820, { 0xfc1f7fe0, "fcvt.ds",     mc88110_disassembler::addressing::FP }},
	{ 0x84000880, { 0xfc1f7fe0, "fcvt.sd",     mc88110_disassembler::addressing::FP }},
	{ 0x84002000, { 0xfc1f7fe0, "flt.ss",      mc88110_disassembler::addressing::FP }},
	{ 0x84002020, { 0xfc1f7fe0, "flt.ds",      mc88110_disassembler::addressing::FP }},
	{ 0x84002800, { 0xfc007fe0, "fadd.sss",    mc88110_disassembler::addressing::FP }},
	{ 0x84002820, { 0xfc007fe0, "fadd.dss",    mc88110_disassembler::addressing::FP }},
	{ 0x84002880, { 0xfc007fe0, "fadd.ssd",    mc88110_disassembler::addressing::FP }},
	{ 0x840028a0, { 0xfc007fe0, "fadd.dsd",    mc88110_disassembler::addressing::FP }},
	{ 0x84002a00, { 0xfc007fe0, "fadd.sds",    mc88110_disassembler::addressing::FP }},
	{ 0x84002a20, { 0xfc007fe0, "fadd.dds",    mc88110_disassembler::addressing::FP }},
	{ 0x84002a80, { 0xfc007fe0, "fadd.sdd",    mc88110_disassembler::addressing::FP }},
	{ 0x84002aa0, { 0xfc007fe0, "fadd.ddd",    mc88110_disassembler::addressing::FP }},
	{ 0x84003000, { 0xfc007fe0, "fsub.sss",    mc88110_disassembler::addressing::FP }},
	{ 0x84003020, { 0xfc007fe0, "fsub.dss",    mc88110_disassembler::addressing::FP }},
	{ 0x84003080, { 0xfc007fe0, "fsub.ssd",    mc88110_disassembler::addressing::FP }},
	{ 0x840030a0, { 0xfc007fe0, "fsub.dsd",    mc88110_disassembler::addressing::FP }},
	{ 0x84003200, { 0xfc007fe0, "fsub.sds",    mc88110_disassembler::addressing::FP }},
	{ 0x84003220, { 0xfc007fe0, "fsub.dds",    mc88110_disassembler::addressing::FP }},
	{ 0x84003280, { 0xfc007fe0, "fsub.sdd",    mc88110_disassembler::addressing::FP }},
	{ 0x840032a0, { 0xfc007fe0, "fsub.ddd",    mc88110_disassembler::addressing::FP }},
	{ 0x84003800, { 0xfc007fe0, "fcmp.sss",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84003820, { 0xfc007fe0, "fcmpu.sss",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84003880, { 0xfc007fe0, "fcmp.ssd",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x840038a0, { 0xfc007fe0, "fcmpu.ssd",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84003a00, { 0xfc007fe0, "fcmp.sds",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84003a20, { 0xfc007fe0, "fcmpu.sds",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84003a80, { 0xfc007fe0, "fcmp.sdd",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84003aa0, { 0xfc007fe0, "fcmpu.sdd",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84004200, { 0xfc1fffe0, "mov.s",       mc88110_disassembler::addressing::FP }}, // GRF to XRF
	{ 0x84004280, { 0xfc1fffe0, "mov.d",       mc88110_disassembler::addressing::FP }}, // GRF to XRF
	{ 0x84004800, { 0xfc1f7fe0, "int.ss",      mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84004880, { 0xfc1f7fe0, "int.sd",      mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84005000, { 0xfc1f7fe0, "nint.ss",     mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84005080, { 0xfc1f7fe0, "nint.sd",     mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84005800, { 0xfc1f7fe0, "trnc.ss",     mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84005880, { 0xfc1f7fe0, "trnc.sd",     mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x84007000, { 0xfc007fe0, "fdiv.sss",    mc88110_disassembler::addressing::FP }},
	{ 0x84007020, { 0xfc007fe0, "fdiv.dss",    mc88110_disassembler::addressing::FP }},
	{ 0x84007080, { 0xfc007fe0, "fdiv.ssd",    mc88110_disassembler::addressing::FP }},
	{ 0x840070a0, { 0xfc007fe0, "fdiv.dsd",    mc88110_disassembler::addressing::FP }},
	{ 0x84007200, { 0xfc007fe0, "fdiv.sds",    mc88110_disassembler::addressing::FP }},
	{ 0x84007220, { 0xfc007fe0, "fdiv.dds",    mc88110_disassembler::addressing::FP }},
	{ 0x84007280, { 0xfc007fe0, "fdiv.sdd",    mc88110_disassembler::addressing::FP }},
	{ 0x840072a0, { 0xfc007fe0, "fdiv.ddd",    mc88110_disassembler::addressing::FP }},
	{ 0x84007800, { 0xfc1f7fe0, "fsqrt.ss",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x84007820, { 0xfc1f7fe0, "fsqrt.ds",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x84007880, { 0xfc1f7fe0, "fsqrt.sd",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x840078a0, { 0xfc1f7fe0, "fsqrt.dd",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x840080c0, { 0xfc00ffe0, "fmul.xsd",    mc88110_disassembler::addressing::FP }},
	{ 0x84008100, { 0xfc00ffe0, "fmul.ssx",    mc88110_disassembler::addressing::FP }},
	{ 0x84008120, { 0xfc00ffe0, "fmul.dsx",    mc88110_disassembler::addressing::FP }},
	{ 0x84008140, { 0xfc00ffe0, "fmul.xsx",    mc88110_disassembler::addressing::FP }},
	{ 0x84008240, { 0xfc00ffe0, "fmul.xds",    mc88110_disassembler::addressing::FP }},
	{ 0x840082c0, { 0xfc00ffe0, "fmul.xdd",    mc88110_disassembler::addressing::FP }},
	{ 0x84008400, { 0xfc00ffe0, "fmul.sxs",    mc88110_disassembler::addressing::FP }},
	{ 0x84008420, { 0xfc00ffe0, "fmul.dxs",    mc88110_disassembler::addressing::FP }},
	{ 0x84008440, { 0xfc00ffe0, "fmul.xxs",    mc88110_disassembler::addressing::FP }},
	{ 0x84008480, { 0xfc00ffe0, "fmul.sxd",    mc88110_disassembler::addressing::FP }},
	{ 0x840084a0, { 0xfc00ffe0, "fmul.dxd",    mc88110_disassembler::addressing::FP }},
	{ 0x840084c0, { 0xfc00ffe0, "fmul.xxd",    mc88110_disassembler::addressing::FP }},
	{ 0x84008500, { 0xfc00ffe0, "fmul.sxx",    mc88110_disassembler::addressing::FP }},
	{ 0x84008520, { 0xfc00ffe0, "fmul.dxx",    mc88110_disassembler::addressing::FP }},
	{ 0x84008540, { 0xfc00ffe0, "fmul.xxx",    mc88110_disassembler::addressing::FP }},
	{ 0x84008840, { 0xfc1fffe0, "fcvt.xs",     mc88110_disassembler::addressing::FP }},
	{ 0x840088c0, { 0xfc1fffe0, "fcvt.xd",     mc88110_disassembler::addressing::FP }},
	{ 0x84008900, { 0xfc1fffe0, "fcvt.sx",     mc88110_disassembler::addressing::FP }},
	{ 0x84008920, { 0xfc1fffe0, "fcvt.dx",     mc88110_disassembler::addressing::FP }},
	{ 0x8400a040, { 0xfc1fffe0, "flt.xs",      mc88110_disassembler::addressing::FP }},
	{ 0x8400a840, { 0xfc00ffe0, "fadd.xss",    mc88110_disassembler::addressing::FP }},
	{ 0x8400a8c0, { 0xfc00ffe0, "fadd.xsd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400a900, { 0xfc00ffe0, "fadd.ssx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400a920, { 0xfc00ffe0, "fadd.dsx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400a940, { 0xfc00ffe0, "fadd.xsx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400aa40, { 0xfc00ffe0, "fadd.xds",    mc88110_disassembler::addressing::FP }},
	{ 0x8400aac0, { 0xfc00ffe0, "fadd.xdd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ac00, { 0xfc00ffe0, "fadd.sxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ac20, { 0xfc00ffe0, "fadd.dxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ac40, { 0xfc00ffe0, "fadd.xxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ac80, { 0xfc00ffe0, "fadd.sxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400aca0, { 0xfc00ffe0, "fadd.dxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400acc0, { 0xfc00ffe0, "fadd.xxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ad00, { 0xfc00ffe0, "fadd.sxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ad20, { 0xfc00ffe0, "fadd.dxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400ad40, { 0xfc00ffe0, "fadd.xxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b040, { 0xfc00ffe0, "fsub.xss",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b0c0, { 0xfc00ffe0, "fsub.xsd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b100, { 0xfc00ffe0, "fsub.ssx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b120, { 0xfc00ffe0, "fsub.dsx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b140, { 0xfc00ffe0, "fsub.xsx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b240, { 0xfc00ffe0, "fsub.xds",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b2c0, { 0xfc00ffe0, "fsub.xdd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b400, { 0xfc00ffe0, "fsub.sxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b420, { 0xfc00ffe0, "fsub.dxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b440, { 0xfc00ffe0, "fsub.xxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b480, { 0xfc00ffe0, "fsub.sxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b4a0, { 0xfc00ffe0, "fsub.dxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b4c0, { 0xfc00ffe0, "fsub.xxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b500, { 0xfc00ffe0, "fsub.sxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b520, { 0xfc00ffe0, "fsub.dxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b540, { 0xfc00ffe0, "fsub.xxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400b900, { 0xfc00ffe0, "fcmp.ssx",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400b920, { 0xfc00ffe0, "fcmpu.ssx",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bb00, { 0xfc00ffe0, "fcmp.sdx",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bb20, { 0xfc00ffe0, "fcmpu.sdx",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bc00, { 0xfc00ffe0, "fcmp.sxs",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bc20, { 0xfc00ffe0, "fcmpu.sxs",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bc80, { 0xfc00ffe0, "fcmp.sxd",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bca0, { 0xfc00ffe0, "fcmpu.sxd",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bd00, { 0xfc00ffe0, "fcmp.sxx",    mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400bd20, { 0xfc00ffe0, "fcmpu.sxx",   mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400c000, { 0xfc1fffe0, "mov.s",       mc88110_disassembler::addressing::FP }}, // XRF to GRF
	{ 0x8400c080, { 0xfc1fffe0, "mov.d",       mc88110_disassembler::addressing::FP }}, // XRF to GRF
	{ 0x8400c300, { 0xfc1fffe0, "mov",         mc88110_disassembler::addressing::FP }}, // XRF to XRF
	{ 0x8400c900, { 0xfc1fffe0, "int.sx",      mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400d100, { 0xfc1fffe0, "nint.sx",     mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400d900, { 0xfc1fffe0, "trnc.sx",     mc88110_disassembler::addressing::FP }}, // destination in GRF
	{ 0x8400f040, { 0xfc00ffe0, "fdiv.xss",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f0c0, { 0xfc00ffe0, "fdiv.xsd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f100, { 0xfc00ffe0, "fdiv.ssx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f120, { 0xfc00ffe0, "fdiv.dsx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f140, { 0xfc00ffe0, "fdiv.xsx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f240, { 0xfc00ffe0, "fdiv.xds",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f2c0, { 0xfc00ffe0, "fdiv.xdd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f400, { 0xfc00ffe0, "fdiv.sxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f420, { 0xfc00ffe0, "fdiv.dxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f440, { 0xfc00ffe0, "fdiv.xxs",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f480, { 0xfc00ffe0, "fdiv.sxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f4a0, { 0xfc00ffe0, "fdiv.dxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f4c0, { 0xfc00ffe0, "fdiv.xxd",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f500, { 0xfc00ffe0, "fdiv.sxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f520, { 0xfc00ffe0, "fdiv.dxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f540, { 0xfc00ffe0, "fdiv.xxx",    mc88110_disassembler::addressing::FP }},
	{ 0x8400f840, { 0xfc1fffe0, "fsqrt.xs",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x8400f8c0, { 0xfc1fffe0, "fsqrt.xd",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x8400f900, { 0xfc1fffe0, "fsqrt.sx",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x8400f920, { 0xfc1fffe0, "fsqrt.dx",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x8400f940, { 0xfc1fffe0, "fsqrt.xx",    mc88110_disassembler::addressing::FP }}, // not implemented in hardware
	{ 0x88000000, { 0xfc00ffe0, "pmul",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88002020, { 0xfc00ffe0, "padd.b",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88002040, { 0xfc00ffe0, "padd.h",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88002060, { 0xfc00ffe0, "padd",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880020a0, { 0xfc00ffe0, "padds.u.b",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880020c0, { 0xfc00ffe0, "padds.u.h",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880020e0, { 0xfc00ffe0, "padds.u",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88002120, { 0xfc00ffe0, "padds.us.b",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88002140, { 0xfc00ffe0, "padds.us.h",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88002160, { 0xfc00ffe0, "padds.us",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880021a0, { 0xfc00ffe0, "padds.s.b",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880021c0, { 0xfc00ffe0, "padds.s.h",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880021e0, { 0xfc00ffe0, "padds.s",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003020, { 0xfc00ffe0, "psub.b",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003040, { 0xfc00ffe0, "psub.h",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003060, { 0xfc00ffe0, "psub",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003080, { 0xfc00ffe0, "psubs.u.b",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880030c0, { 0xfc00ffe0, "psubs.u.h",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880030e0, { 0xfc00ffe0, "psubs.u",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003120, { 0xfc00ffe0, "psubs.us.b",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003140, { 0xfc00ffe0, "psubs.us.h",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003160, { 0xfc00ffe0, "psubs.us",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880031a0, { 0xfc00ffe0, "psubs.b",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880031c0, { 0xfc00ffe0, "psubs.h",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x880031e0, { 0xfc00ffe0, "psubs",       mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88003860, { 0xfc00ffe0, "pcmp",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006160, { 0xfc00ffe0, "ppack.8",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006240, { 0xfc00ffe0, "ppack.16.h",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006260, { 0xfc00ffe0, "ppack.16",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006420, { 0xfc00ffe0, "ppack.32.b",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006440, { 0xfc00ffe0, "ppack.32.h",  mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006460, { 0xfc00ffe0, "ppack.32",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006800, { 0xfc00ffff, "punpk.n",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006820, { 0xfc00ffff, "punpk.b",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88006840, { 0xfc00ffff, "punpk.h",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0x88007000, { 0xfc00f87f, "prot",        mc88110_disassembler::addressing::IMM6 }},
	{ 0x88007800, { 0xfc00ffe0, "prot",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xc0000000, { 0xfc000000, "br",          mc88110_disassembler::addressing::D26 }},
	{ 0xc4000000, { 0xfc000000, "br.n",        mc88110_disassembler::addressing::D26 }},
	{ 0xc8000000, { 0xfc000000, "bsr",         mc88110_disassembler::addressing::D26 }},
	{ 0xcc000000, { 0xfc000000, "bsr.n",       mc88110_disassembler::addressing::D26 }},
	{ 0xd0000000, { 0xfc000000, "bb0",         mc88110_disassembler::addressing::D16 }},
	{ 0xd4000000, { 0xfc000000, "bb0.n",       mc88110_disassembler::addressing::D16 }},
	{ 0xd8000000, { 0xfc000000, "bb1",         mc88110_disassembler::addressing::D16 }},
	{ 0xdc000000, { 0xfc000000, "bb1.n",       mc88110_disassembler::addressing::D16 }},
	{ 0xe8000000, { 0xfc000000, "bcnd",        mc88110_disassembler::addressing::D16 }},
	{ 0xec000000, { 0xfc000000, "bcnd.n",      mc88110_disassembler::addressing::D16 }},
	{ 0xf0001000, { 0xf800ffe0, "ld.d",        mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0001100, { 0xf800ffe0, "ld.d.usr",    mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0001200, { 0xf800ffe0, "ld.d",        mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0001300, { 0xf800ffe0, "ld.d.usr",    mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0001400, { 0xf800ffe0, "ld",          mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0001500, { 0xf800ffe0, "ld.usr",      mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0001600, { 0xf800ffe0, "ld",          mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0001700, { 0xf800ffe0, "ld.usr",      mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0001800, { 0xfc00ffe0, "ld.x",        mc88110_disassembler::addressing::TRIADIC }}, // XRF only
	{ 0xf0001900, { 0xfc00ffe0, "ld.x.usr",    mc88110_disassembler::addressing::TRIADIC }}, // XRF only
	{ 0xf0001a00, { 0xfc00ffe0, "ld.x",        mc88110_disassembler::addressing::SCALED }}, // XRF only
	{ 0xf0001b00, { 0xfc00ffe0, "ld.x.usr",    mc88110_disassembler::addressing::SCALED }}, // XRF only
	{ 0xf0002000, { 0xf800ffe0, "st.d",        mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002080, { 0xf800ffe0, "st.d.wt",     mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002100, { 0xf800ffe0, "st.d.usr",    mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002180, { 0xf800ffe0, "st.d.usr.wt", mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002200, { 0xf800ffe0, "st.d",        mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002280, { 0xf800ffe0, "st.d.wt",     mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002300, { 0xf800ffe0, "st.d.usr",    mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002380, { 0xf800ffe0, "st.d.usr.wt", mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002400, { 0xf800ffe0, "st",          mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002480, { 0xf800ffe0, "st.wt",       mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002500, { 0xf800ffe0, "st.usr",      mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002580, { 0xf800ffe0, "st.usr.wt",   mc88110_disassembler::addressing::TRIADIC }}, // GRF or XRF
	{ 0xf0002600, { 0xf800ffe0, "st",          mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002680, { 0xf800ffe0, "st.wt",       mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002700, { 0xf800ffe0, "st.usr",      mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002780, { 0xf800ffe0, "st.usr.wt",   mc88110_disassembler::addressing::SCALED }}, // GRF or XRF
	{ 0xf0002800, { 0xfc00ffe0, "st.x",        mc88110_disassembler::addressing::TRIADIC }}, // XRF only
	{ 0xf0002880, { 0xfc00ffe0, "st.x.wt",     mc88110_disassembler::addressing::TRIADIC }}, // XRF only
	{ 0xf0002900, { 0xfc00ffe0, "st.x.usr",    mc88110_disassembler::addressing::TRIADIC }}, // XRF only
	{ 0xf0002980, { 0xfc00ffe0, "st.x.usr.wt", mc88110_disassembler::addressing::TRIADIC }}, // XRF only
	{ 0xf0002a00, { 0xfc00ffe0, "st.x",        mc88110_disassembler::addressing::SCALED }}, // XRF only
	{ 0xf0002a80, { 0xfc00ffe0, "st.x.wt",     mc88110_disassembler::addressing::SCALED }}, // XRF only
	{ 0xf0002b00, { 0xfc00ffe0, "st.x.usr",    mc88110_disassembler::addressing::SCALED }}, // XRF only
	{ 0xf0002b80, { 0xfc00ffe0, "st.x.usr.wt", mc88110_disassembler::addressing::SCALED }}, // XRF only
	{ 0xf0008000, { 0xfc00fc00, "clr",         mc88110_disassembler::addressing::BITFIELD }},
	{ 0xf0008800, { 0xfc00fc00, "set",         mc88110_disassembler::addressing::BITFIELD }},
	{ 0xf0009000, { 0xfc00fc00, "ext",         mc88110_disassembler::addressing::BITFIELD }},
	{ 0xf0009800, { 0xfc00fc00, "extu",        mc88110_disassembler::addressing::BITFIELD }},
	{ 0xf000a000, { 0xfc00fc00, "mak",         mc88110_disassembler::addressing::BITFIELD }},
	{ 0xf000a800, { 0xfc00ffe0, "rot",         mc88110_disassembler::addressing::BITFIELD }},
	{ 0xf000d000, { 0xfc00fe00, "tb0",         mc88100_disassembler::addressing::VEC9 }},
	{ 0xf000d800, { 0xfc00fe00, "tb1",         mc88100_disassembler::addressing::VEC9 }},
	{ 0xf000e800, { 0xfc00fe00, "tcnd",        mc88100_disassembler::addressing::VEC9 }},
	{ 0xf4000000, { 0xfc00ffe0, "xmem.bu",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000100, { 0xfc00ffe0, "xmem.bu.usr", mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000200, { 0xfc00ffe0, "xmem.bu",     mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000300, { 0xfc00ffe0, "xmem.bu.usr", mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000400, { 0xfc00ffe0, "xmem",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000500, { 0xfc00ffe0, "xmem.usr",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000600, { 0xfc00ffe0, "xmem",        mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000700, { 0xfc00ffe0, "xmem.usr",    mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000800, { 0xfc00ffe0, "ld.hu",       mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000900, { 0xfc00ffe0, "ld.hu.usr",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000a00, { 0xfc00ffe0, "ld.hu",       mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000b00, { 0xfc00ffe0, "ld.hu.usr",   mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000c00, { 0xfc00ffe0, "ld.bu",       mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000d00, { 0xfc00ffe0, "ld.bu.usr",   mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4000e00, { 0xfc00ffe0, "ld.bu",       mc88110_disassembler::addressing::SCALED }},
	{ 0xf4000f00, { 0xfc00ffe0, "ld.bu.usr",   mc88110_disassembler::addressing::SCALED }},
	{ 0xf4001800, { 0xfc00ffe0, "ld.h",        mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4001900, { 0xfc00ffe0, "ld.h.usr",    mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4001a00, { 0xfc00ffe0, "ld.h",        mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4001b00, { 0xfc00ffe0, "ld.h.usr",    mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4001c00, { 0xfc00ffe0, "ld.b",        mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4001d00, { 0xfc00ffe0, "ld.b.usr",    mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4001e00, { 0xfc00ffe0, "ld.b",        mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4001f00, { 0xfc00ffe0, "ld.b.usr",    mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002800, { 0xfc00ffe0, "st.h",        mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002880, { 0xfc00ffe0, "st.h.wt",     mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002900, { 0xfc00ffe0, "st.h.usr",    mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002980, { 0xfc00ffe0, "st.h.usr.wt", mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002a00, { 0xfc00ffe0, "st.h",        mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002a80, { 0xfc00ffe0, "st.h.wt",     mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002b00, { 0xfc00ffe0, "st.h.usr",    mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002b80, { 0xfc00ffe0, "st.h.usr.wt", mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002c00, { 0xfc00ffe0, "st.b",        mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002c80, { 0xfc00ffe0, "st.b.wt",     mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002d00, { 0xfc00ffe0, "st.b.usr",    mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002d80, { 0xfc00ffe0, "st.b.usr.wt", mc88110_disassembler::addressing::TRIADIC }}, // GRF only
	{ 0xf4002e00, { 0xfc00ffe0, "st.b",        mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002e80, { 0xfc00ffe0, "st.b.wt",     mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002f00, { 0xfc00ffe0, "st.b.usr",    mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4002f80, { 0xfc00ffe0, "st.b.usr.wt", mc88110_disassembler::addressing::SCALED }}, // GRF only
	{ 0xf4003200, { 0xfc00ffe0, "lda.d",       mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003300, { 0xfc00ffe0, "lda.d.usr",   mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003600, { 0xfc00ffe0, "lda",         mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003700, { 0xfc00ffe0, "lda.usr",     mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003a00, { 0xfc00ffe0, "lda.h",       mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003b00, { 0xfc00ffe0, "lda.h.usr",   mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003e00, { 0xfc00ffe0, "lda.x",       mc88110_disassembler::addressing::SCALED }},
	{ 0xf4003f00, { 0xfc00ffe0, "lda.x.usr",   mc88110_disassembler::addressing::SCALED }},
	{ 0xf4004000, { 0xfc00ffe0, "and",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4004400, { 0xfc00ffe0, "and.c",       mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4005000, { 0xfc00ffe0, "xor",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4005400, { 0xfc00ffe0, "xor.c",       mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4005800, { 0xfc00ffe0, "or",          mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4005c00, { 0xfc00ffe0, "or.c",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006000, { 0xfc00ffe0, "addu",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006100, { 0xfc00ffe0, "addu.co",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006200, { 0xfc00ffe0, "addu.ci",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006300, { 0xfc00ffe0, "addu.cio",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006400, { 0xfc00ffe0, "subu",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006500, { 0xfc00ffe0, "subu.co",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006600, { 0xfc00ffe0, "subu.ci",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006700, { 0xfc00ffe0, "subu.cio",    mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006800, { 0xfc00ffe0, "divu",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006900, { 0xfc00ffe0, "divu.d",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006c00, { 0xfc00ffe0, "mulu",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006d00, { 0xfc00ffe0, "mulu.d",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4006e00, { 0xfc00ffe0, "muls",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007000, { 0xfc00ffe0, "add",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007100, { 0xfc00ffe0, "add.co",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007200, { 0xfc00ffe0, "add.ci",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007300, { 0xfc00ffe0, "add.cio",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007400, { 0xfc00ffe0, "sub",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007500, { 0xfc00ffe0, "sub.co",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007600, { 0xfc00ffe0, "sub.ci",      mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007700, { 0xfc00ffe0, "sub.cio",     mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007800, { 0xfc00ffe0, "divs",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4007c00, { 0xfc00ffe0, "cmp",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4008000, { 0xfc00ffe0, "clr",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4008800, { 0xfc00ffe0, "set",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4009000, { 0xfc00ffe0, "ext",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf4009800, { 0xfc00ffe0, "extu",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf400a000, { 0xfc00ffe0, "mak",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf400a800, { 0xfc00ffe0, "rot",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf400c000, { 0xffffffe0, "jmp",         mc88110_disassembler::addressing::JUMP }},
	{ 0xf400c400, { 0xffffffe0, "jmp.n",       mc88110_disassembler::addressing::JUMP }},
	{ 0xf400c800, { 0xffffffe0, "jsr",         mc88110_disassembler::addressing::JUMP }},
	{ 0xf400cc00, { 0xffffffe0, "jsr.n",       mc88110_disassembler::addressing::JUMP }},
	{ 0xf400e000, { 0xfc1fffe0, "ff1",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf400e800, { 0xfc1fffe0, "ff0",         mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf400f800, { 0xffe0ffe0, "tbnd",        mc88110_disassembler::addressing::TRIADIC }},
	{ 0xf400fc00, { 0xffffffff, "rte",         mc88110_disassembler::addressing::NONE }},
	{ 0xf400fc01, { 0xffffffff, "illop1",      mc88110_disassembler::addressing::NONE }},
	{ 0xf400fc02, { 0xffffffff, "illop2",      mc88110_disassembler::addressing::NONE }},
	{ 0xf400fc03, { 0xffffffff, "illop3",      mc88110_disassembler::addressing::NONE }},
	{ 0xf8000000, { 0xffe00000, "tbnd",        mc88110_disassembler::addressing::IMM16 }}
};

u32 m88000_disassembler::opcode_alignment() const
{
	return 4;
}

void mc88100_disassembler::format_gcr(std::ostream &stream, u8 cr)
{
	switch (cr)
	{
	case 0:
		// Processor identification register
		stream << "pid";
		break;

	case 1:
		// Processor status register
		stream << "psr";
		break;

	case 2:
		// Exception time processor status register
		stream << "epsr";
		break;

	case 3:
		// Shadow scoreboard register
		stream << "ssbr";
		break;

	case 4:
		// Shadow execute instruction pointer
		stream << "sxip";
		break;

	case 5:
		// Shadow next instruction pointer
		stream << "snip";
		break;

	case 6:
		// Shadow fetched instruction pointer
		stream << "sfip";
		break;

	case 7:
		// Vector base register
		stream << "vbr";
		break;

	case 8: case 11: case 14:
		// Transaction registers 0–2
		util::stream_format(stream, "dmt%d", (cr - 8) / 3);
		break;

	case 9: case 12: case 15:
		// Data registers 0–2
		util::stream_format(stream, "dmd%d", (cr - 9) / 3);
		break;

	case 10: case 13: case 16:
		// Address registers 0–2
		util::stream_format(stream, "dma%d", (cr - 9) / 3);
		break;

	case 17: case 18: case 19: case 20:
		// Supervisor storage registers 0–3
		util::stream_format(stream, "sr%d", cr - 17);
		break;

	default:
		util::stream_format(stream, "cr%d", cr);
		break;
	}
}

void mc88110_disassembler::format_gcr(std::ostream &stream, u8 cr)
{
	switch (cr)
	{
	case 0:
		// Processor identification
		stream << "pid";
		break;

	case 1:
		// Processor status register
		stream << "psr";
		break;

	case 2:
		// Exception processor status register
		stream << "epsr";
		break;

	case 4:
		// Exception executing instruction pointer
		stream << "exip";
		break;

	case 5:
		// Exception next instruction pointer
		stream << "enip";
		break;

	case 7:
		// Vector base register
		stream << "vbr";
		break;

	case 16: case 17: case 18: case 19: case 20:
		// Storage registers 0–4
		util::stream_format(stream, "sr%d", cr - 16);
		break;

	case 25:
		// Instruction MMU/cache/TIC command
		stream << "icmd";
		break;

	case 26:
		// Instruction MMU/cache control
		stream << "ictl";
		break;

	case 27:
		// Instruction system address
		stream << "isar";
		break;

	case 28:
		// Instruction MMU supervisor address pointer
		stream << "isap";
		break;

	case 29:
		// Instruction MMU user area pointer
		stream << "iuap";
		break;

	case 30:
		// Instruction MMU ATC index register
		stream << "iir";
		break;

	case 31:
		// Instruction MMU BATC R/W port
		stream << "ibp";
		break;

	case 32:
		// Instruction MMU PATC R/W port (upper)
		stream << "ippu";
		break;

	case 33:
		// Instruction MMU PATC R/W port (lower)
		stream << "ippl";
		break;

	case 34:
		// Instruction access status register
		stream << "isr";
		break;

	case 35:
		// Instruction access logical address
		stream << "ilar";
		break;

	case 36:
		// Instruction access physical address
		stream << "ipar";
		break;

	case 40:
		// Data MMU/cache command
		stream << "dcmd";
		break;

	case 41:
		// Data MMU/cache control
		stream << "dctl";
		break;

	case 42:
		// Data system address register
		stream << "dsar";
		break;

	case 43:
		// Data MMU supervisor area pointer
		stream << "dsap";
		break;

	case 44:
		// Data MMU user area pointer
		stream << "duap";
		break;

	case 45:
		// Data MMU ATC index register
		stream << "dir";
		break;

	case 46:
		// Data MMU BATC R/W port
		stream << "dbp";
		break;

	case 47:
		// Data MMU PATC R/W port (upper)
		stream << "dppu";
		break;

	case 48:
		// Data MMU PATC R/W port (lower)
		stream << "dppl";
		break;

	case 49:
		// Data access status register
		stream << "dsr";
		break;

	case 50:
		// Data access logical address
		stream << "dlar";
		break;

	case 51:
		// Data access physical address
		stream << "dpar";
		break;

	default:
		util::stream_format(stream, "cr%d", cr);
		break;
	}
}

void mc88100_disassembler::format_fcr(std::ostream &stream, u8 cr)
{
	switch (cr)
	{
	case 0:
		// Floating-point exception cause register (privileged)
		stream << "fpecr";
		break;

	case 1: case 3:
		// Floating-point source 1/2 operand high register (privileged)
		util::stream_format(stream, "fphs%d", (cr - 1) / 2);
		break;

	case 2: case 4:
		// Floating-point source 1/2 operand low register (privileged)
		util::stream_format(stream, "fpls%d", (cr - 2) / 2);
		break;

	case 5:
		// Floating-point precise operation type register (privileged)
		stream << "fppt";
		break;

	case 6:
		// Floating-point result high register (privileged)
		stream << "fprh";
		break;

	case 7:
		// Floating-point result low register (privileged)
		stream << "fprl";
		break;

	case 8:
		// Floating-point precise operation type register (privileged)
		stream << "fpit";
		break;

	case 62:
		// Floating-point user status register
		stream << "fpsr";
		break;

	case 63:
		// Floating-point user control register
		stream << "fpcr";
		break;

	default:
		util::stream_format(stream, "fcr%d", cr);
		break;
	}
}

void mc88110_disassembler::format_fcr(std::ostream &stream, u8 cr)
{
	switch (cr)
	{
	case 0:
		// Floating-point exception cause register (privileged)
		stream << "fpecr";
		break;

	case 62:
		// Floating-point status register
		stream << "fpsr";
		break;

	case 63:
		// Floating-point control register
		stream << "fpcr";
		break;

	default:
		util::stream_format(stream, "fcr%d", cr);
		break;
	}
}

void m88000_disassembler::format_condition(std::ostream &stream, u8 cnd)
{
	switch (cnd)
	{
	case 0x01:
		stream << "gt0";
		break;

	case 0x02:
		stream << "eq0";
		break;

	case 0x03:
		stream << "ge0";
		break;

	case 0x0c:
		stream << "lt0";
		break;

	case 0x0d:
		stream << "ne0";
		break;

	case 0x0e:
		stream << "le0";
		break;

	default:
		util::stream_format(stream, "$%02x", cnd);
		break;
	}
}

offs_t m88000_disassembler::dasm_triadic(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Triadic register addressing
	util::stream_format(stream, "%-12s", mnemonic);
	if ((inst & 0xf800f800) != 0xf000f800)
		util::stream_format(stream, "%c%d,", BIT(inst, 26) ? 'r' : 'x', (inst & 0x03e00000) >> 21);
	if ((inst & 0xf800f000) != 0xf000e000)
	{
		util::stream_format(stream, "r%d", (inst & 0x001f0000) >> 16);
		if ((inst & 0xfc00f800) != 0x88006800)
			stream << ",";
	}
	if ((inst & 0xfc00f800) != 0x88006800)
		util::stream_format(stream, "r%d", inst & 0x0000001f);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_fp(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Triadic register addressing (floating-point instructions)
	if ((inst & 0xfc006000) == 0x84004000)
	{
		util::stream_format(stream, "%-12s%c%d,", mnemonic,
							(inst & 0xfc007e00) == 0x84004200 ? 'x' : 'r',
							(inst & 0x03e00000) >> 21);
	}
	else
	{
		util::stream_format(stream, "%-12s%c%d,", mnemonic,
							BIT(inst, 15) && (inst & 0xfc007800) != 0x84003800 ? 'x' : 'r',
							(inst & 0x03e00000) >> 21);
		if ((inst & 0xfc007800) != 0x84000800 && (inst & 0xfc007800) != 0x84002000 && (inst & 0xfc007800) != 0x84007800)
			util::stream_format(stream, "%c%d,", BIT(inst, 15) ? 'x' : 'r', (inst & 0x001f0000) >> 16);
	}
	util::stream_format(stream, "%c%d,", BIT(inst, 15) ? 'x' : 'r', inst & 0x0000001f);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_imm6(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Register with 6-bit immediate addressing
	util::stream_format(stream, "%-12sr%d,r%d,<%d>",
						BIT(inst, 26) ? 'r' : 'x',
						(inst & 0x03e00000) >> 21,
						(inst & 0x001f0000) >> 16,
						(inst & 0x000007e0) >> 5);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_bitfield(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Register with 10-bit immediate addressing
	util::stream_format(stream, "%-12sr%d,r%d,", mnemonic,
						(inst & 0x03e00000) >> 21,
						(inst & 0x001f0000) >> 16);
	if ((inst & 0xfc00fc00) != 0xf000a800 && (inst & 0x000003e0) != 0)
		util::stream_format(stream, "%d", (inst & 0x000003e0) >> 5);
	util::stream_format(stream, "<%d>", inst & 0x0000001f);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_simm16(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Register with 16-bit signed immediate addressing
	util::stream_format(stream, "%-12sr%d,r%d,", mnemonic,
						(inst & 0x03e00000) >> 21,
						(inst & 0x001f0000) >> 16);
	if (BIT(inst, 15))
		util::stream_format(stream, "-$%x", 0x10000 - (inst & 0x0000ffff));
	else
		util::stream_format(stream, "$%x", inst & 0x0000ffff);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_imm16(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Register with 16-bit unsigned immediate addressing
	util::stream_format(stream, "%-12s", mnemonic);
	if ((inst & 0xfc000000) != 0xf8000000)
		util::stream_format(stream, "r%d,", (inst & 0x03e00000) >> 21);
	util::stream_format(stream, "r%d,", (inst & 0x001f0000) >> 16);
	if (inst >= 0x40000000)
		util::stream_format(stream, "$%04x", inst & 0x0000ffff);
	else
		util::stream_format(stream, "$%x", inst & 0x0000ffff);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_cr(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Control register addressing
	util::stream_format(stream, "%-12s", mnemonic);
	if (BIT(inst, 14))
		util::stream_format(stream, "r%d,", (inst & 0x03e00000) >> 21);
	if (BIT(inst, 15))
	{
		// S1 and S2 must be the same
		if ((inst & 0x001f0000) >> 16 != (inst & 0x0000001f))
			stream << "r<mismatch>,";
		else
			util::stream_format(stream, "r%d,", inst & 0x0000001f);
	}

	switch ((inst & 0x00003800) >> 11)
	{
	case 0:
		format_gcr(stream, (inst & 0x000007e0) >> 5);
		break;

	case 1:
		format_fcr(stream, (inst & 0x000007e0) >> 5);
		break;

	default:
		util::stream_format(stream, "SFU%d_cr%d", (inst & 0x00003800) >> 11, (inst & 0x000007e0) >> 5);
		break;
	}

	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_si16(std::ostream &stream, const char *mnemonic, u32 inst, bool xrf)
{
	// Register indirect with extended immediate index
	util::stream_format(stream, "%-12s%c%d,r%d,", mnemonic,
						xrf ? 'x' : 'r',
						(inst & 0x03e00000) >> 21,
						(inst & 0x001f0000) >> 16);
	if (BIT(inst, 15))
		util::stream_format(stream, "-$%x", 0x10000 - (inst & 0x0000ffff));
	else
		util::stream_format(stream, "$%x", inst & 0x0000ffff);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_scaled(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Register indirect with scaled index
	util::stream_format(stream, "%-12s%c%d,r%d[r%d]", mnemonic,
						BIT(inst, 26) ? 'r' : 'x',
						(inst & 0x03e00000) >> 21,
						(inst & 0x001f0000) >> 16,
						inst & 0x0000001f);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_jump(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Triadic register addressing (jump instructions)
	util::stream_format(stream, "%-12sr%d", mnemonic, inst & 0x0000001f);

	// Set flags for jump to subroutine, return to r1 and/or optional delay slot
	if (BIT(inst, 11))
		return 4 | SUPPORTED | STEP_OVER | (BIT(inst, 10) ? step_over_extra(1) : 0);
	else if ((inst & 0x0000001f) == 0x00000001)
		return 4 | SUPPORTED | STEP_OUT | (BIT(inst, 10) ? step_over_extra(1) : 0);
	else
		return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_vec9(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// Register with 9-bit vector table index
	util::stream_format(stream, "%-12s", mnemonic);
	if (BIT(inst, 12))
		util::stream_format(stream, "%d", (inst & 0x03e00000) >> 21);
	else
		format_condition(stream, (inst & 0x03e00000) >> 21);
	util::stream_format(stream, ",r%d,$%03x", (inst & 0x001f0000) >> 16, inst & 0x000001ff);
	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_d16(std::ostream &stream, const char *mnemonic, u32 inst, offs_t pc)
{
	// Register with 16-bit displacement
	s32 disp = s16(inst & 0x0000ffff) * 4;
	util::stream_format(stream, "%-12s", mnemonic);
	if (BIT(inst, 28))
		util::stream_format(stream, "%d", (inst & 0x03e00000) >> 21);
	else
		format_condition(stream, (inst & 0x03e00000) >> 21);
	util::stream_format(stream, ",r%d,$%08x", (inst & 0x001f0000) >> 16, pc + disp);

	// Set flags for optional delay slot
	return 4 | SUPPORTED | STEP_COND | (BIT(inst, 26) ? step_over_extra(1) : 0);
}

offs_t m88000_disassembler::dasm_d26(std::ostream &stream, const char *mnemonic, u32 inst, offs_t pc)
{
	// 26-bit branch displacement
	s32 disp = (inst & 0x03ffffff) * 4 - (BIT(inst, 25) ? 0x10000000 : 0);
	util::stream_format(stream, "%-12s$%08x", mnemonic, pc + disp);

	// Set flags for branch to subroutine and/or optional delay slot
	if (BIT(inst, 27))
		return 4 | SUPPORTED | STEP_OVER | (BIT(inst, 26) ? step_over_extra(1) : 0);
	else
		return 4 | SUPPORTED;
}

offs_t m88000_disassembler::dasm_none(std::ostream &stream, const char *mnemonic, u32 inst)
{
	// No operands indicated
	stream << mnemonic;

	// Set flags for return-from-exception instruction
	return 4 | SUPPORTED | ((inst & 0xfc00ffe0) == 0xf400fc00 ? STEP_OUT : 0);
}

offs_t m88000_disassembler::dasm_illop(std::ostream &stream, u32 inst)
{
	// Illegal operation (at least probably illegal)
	util::stream_format(stream, "%-12s$%08x", "illop", inst);

	// SFU-specific
	if ((inst & 0xe0000000) == 0x80000000)
		util::stream_format(stream, " ; SFU%d unimplemented", (inst & 0x1c000000) >> 26);

	return 4 | SUPPORTED;
}

offs_t m88000_disassembler::disassemble(std::ostream &stream, offs_t pc, const m88000_disassembler::data_buffer &opcodes, const m88000_disassembler::data_buffer &params)
{
	u32 inst = opcodes.r32(pc);
	auto j = m_ops.upper_bound(inst);
	for (auto i = m_ops.lower_bound(inst & 0xf8000000); i != j; ++i)
	{
		if ((inst & i->second.mask) == i->first)
		{
			switch (i->second.mode)
			{
			case addressing::TRIADIC:
				return dasm_triadic(stream, i->second.mnemonic, inst);

			case addressing::FP:
				return dasm_fp(stream, i->second.mnemonic, inst);

			case addressing::IMM6:
				return dasm_imm6(stream, i->second.mnemonic, inst);

			case addressing::BITFIELD:
				return dasm_bitfield(stream, i->second.mnemonic, inst);

			case addressing::SIMM16:
				return dasm_simm16(stream, i->second.mnemonic, inst);

			case addressing::IMM16:
				return dasm_imm16(stream, i->second.mnemonic, inst);

			case addressing::CR:
				return dasm_cr(stream, i->second.mnemonic, inst);

			case addressing::SI16_GRF:
				return dasm_si16(stream, i->second.mnemonic, inst, false);

			case addressing::SI16_XRF:
				return dasm_si16(stream, i->second.mnemonic, inst, true);

			case addressing::SCALED:
				return dasm_scaled(stream, i->second.mnemonic, inst);

			case addressing::JUMP:
				return dasm_jump(stream, i->second.mnemonic, inst);

			case addressing::VEC9:
				return dasm_vec9(stream, i->second.mnemonic, inst);

			case addressing::D16:
				return dasm_d16(stream, i->second.mnemonic, inst, pc);

			case addressing::D26:
				return dasm_d26(stream, i->second.mnemonic, inst, pc);

			case addressing::NONE:
				return dasm_none(stream, i->second.mnemonic, inst);
			}
		}
	}

	return dasm_illop(stream, inst);
}
