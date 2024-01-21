// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Epson C33 disassembler

    TODO:
    * Only C33 ADV Core is currently supported - add support for C33 STD Core, C33 PE Core, etc.
    * Reconstruct more assembler synthetics
    * Should psrset and psrclr use symbolic names for bits?
    * Should 32-bit extended displacements always be displayed as signed?
    * Should loop instructions have friendlier syntax?
*/

#include "emu.h"
#include "c33dasm.h"

#include <tuple>
#include <utility>


namespace {

char const *const special_reg_names[16] = {
		"%psr",
		"%sp",
		"%alr",
		"%ahr",
		"%lco",
		"%lca",
		"%lea",
		"%sor",
		"%ttbr",
		"%dp",
		"%idir",
		"%dbbr",
		nullptr,
		"%usp",
		"%ssp",
		"%pc" };

std::pair<char const *, unsigned> const class_0_00_ops[32] = {
		{ "nop",               0 },   // 000 0000 0 00 00
		{ "slp",               0 },   // 000 0000 0 01 00
		{ "halt",              0 },   // 000 0000 0 10 00
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ "pushn     %%r%1$u", 4 },   // 000 0001 0 00 00
		{ "popn      %%r%1$u", 4 },   // 000 0001 0 01 00
		{ nullptr,             0 },
		{ "jpr       %%r%1$u", 4 },   // 000 0001 0 11 00
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ "jpr.d     %%r%1$u", 4 },   // 000 0001 1 11 00
		{ "brk",               0 },   // 000 0010 0 00 00
		{ "retd",              0 },   // 000 0010 0 01 00
		{ "int       %1$u",    2 },   // 000 0010 0 10 00
		{ "reti",              0 },   // 000 0010 0 11 00
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ nullptr,             0 },
		{ "call      %%r%1$u", 4 },   // 000 0011 0 00 00
		{ "ret",               0 },   // 000 0011 0 01 00
		{ "jp        %%r%1$u", 4 },   // 000 0011 0 10 00
		{ "retm",              0 },   // 000 0011 0 11 00
		{ "call.d    %%r%1$u", 4 },   // 000 0011 1 00 00
		{ "ret.d",             0 },   // 000 0011 1 01 00
		{ "jp.d      %%r%1$u", 4 },   // 000 0011 1 10 00
		{ nullptr,             0 } };

std::tuple<char const *, bool, bool> const class_0_01_ops[32] = {
		{ "push      %%r%1$u",      false, false },   // 000 0000 000 01
		{ "pop       %%r%1$u",      false, false },   // 000 0000 001 01
		{ "pushs     %%r%2$u",      true,  false },   // 000 0000 010 01
		{ "pops      %%r%2$u",      true,  false },   // 000 0000 011 01
		{ "mac.w     %%r%1$u",      false, false },   // 000 0000 100 01
		{ "mac.hw    %%r%1$u",      false, false },   // 000 0000 101 01
		{ "macclr",                 false, true  },   // 000 0000 110 01
		{ "ld.cf",                  false, true  },   // 000 0000 111 01
		{ "divu.w    %%r%1$u",      false, false },   // 000 0001 000 01
		{ "div.w     %%r%1$u",      false, false },   // 000 0001 001 01
		{ "repeat    %%r%1$u",      false, false },   // 000 0001 010 01
		{ "repeat    %1$u",         false, false },   // 000 0001 011 01
		{ nullptr,                  false, false },
		{ "add       %%r%1$u,%%dp", false, false },   // 000 0001 101 01
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false },
		{ nullptr,                  false, false } };

std::pair<char const *, char const *> const class_0_jumps[16] = {
		{ nullptr,              nullptr              },
		{ nullptr,              nullptr              },
		{ nullptr,              nullptr              },
		{ nullptr,              nullptr              },
		{ "jrgt%1$s    0x%2$x", "xjrgt%1$s   0x%2$x" },   // 000 0100
		{ "jrge%1$s    0x%2$x", "xjrge%1$s   0x%2$x" },   // 000 0101
		{ "jrlt%1$s    0x%2$x", "xjrlt%1$s   0x%2$x" },   // 000 0110
		{ "jrle%1$s    0x%2$x", "xjrle%1$s   0x%2$x" },   // 000 0111
		{ "jrugt%1$s   0x%2$x", "xjrugt%1$s  0x%2$x" },   // 000 1000
		{ "jruge%1$s   0x%2$x", "xjruge%1$s  0x%2$x" },   // 000 1001
		{ "jrult%1$s   0x%2$x", "xjrult%1$s  0x%2$x" },   // 000 1010
		{ "jrule%1$s   0x%2$x", "xjrule%1$s  0x%2$x" },   // 000 1011
		{ "jreq%1$s    0x%2$x", "xjreq%1$s   0x%2$x" },   // 000 1100
		{ "jrne%1$s    0x%2$x", "xjrne%1$s   0x%2$x" },   // 000 1101
		{ "call%1$s    0x%2$x", "xcall%1$s   0x%2$x" },   // 000 1110
		{ "jp%1$s      0x%2$x", "xjp%1$s     0x%2$x" } }; // 000 1111

std::pair<char const *, char const *> const class_1_ops[32] = {
		{ "ld.b      %%r%1$u,[%%r%2$u]",  "xld.b     %%r%1$u,[%%r%2$u + 0x%4$x]" },   // 001 000 00
		{ "ld.b      %%r%1$u,[%%r%2$u]+", nullptr                                },   // 001 000 01
		{ "add       %%r%1$u,%%r%2$u",    "xadd      %%r%1$u,%%r%2$u,0x%4$x"     },   // 001 000 10
		{ "srl       %%r%1$u,%3$u",       nullptr                                },   // 001 000 11
		{ "ld.ub     %%r%1$u,[%%r%2$u]",  "xld.ub    %%r%1$u,[%%r%2$u + 0x%4$x]" },   // 001 001 00
		{ "ld.ub     %%r%1$u,[%%r%2$u]+", nullptr                                },   // 001 001 01
		{ "sub       %%r%1$u,%%r%2$u",    "xsub      %%r%1$u,%%r%2$u,0x%4$x"     },   // 001 001 10
		{ "sll       %%r%1$u,%3$u",       nullptr                                },   // 001 001 11
		{ "ld.h      %%r%1$u,[%%r%2$u]",  "xld.h     %%r%1$u,[%%r%2$u + 0x%4$x]" },   // 001 010 00
		{ "ld.h      %%r%1$u,[%%r%2$u]+", nullptr                                },   // 001 010 01
		{ "cmp       %%r%1$u,%%r%2$u",    nullptr                                },   // 001 010 10
		{ "sra       %%r%1$u,%3$u",       nullptr                                },   // 001 010 11
		{ "ld.uh     %%r%1$u,[%%r%2$u]",  "xld.uh    %%r%1$u,[%%r%2$u + 0x%4$x]" },   // 001 011 00
		{ "ld.uh     %%r%1$u,[%%r%2$u]+", nullptr                                },   // 001 011 01
		{ "ld.w      %%r%1$u,%%r%2$u",    nullptr                                },   // 001 011 10
		{ "sla       %%r%1$u,%3$u",       nullptr                                },   // 001 011 11
		{ "ld.w      %%r%1$u,[%%r%2$u]",  "xld.w     %%r%1$u,[%%r%2$u + 0x%4$x]" },   // 001 100 00
		{ "ld.w      %%r%1$u,[%%r%2$u]+", nullptr                                },   // 001 100 01
		{ "and       %%r%1$u,%%r%2$u",    "xand      %%r%1$u,%%r%2$u,0x%4$x"     },   // 001 100 10
		{ "rr        %%r%1$u,%3$u",       nullptr                                },   // 001 100 11
		{ "ld.b      [%%r%2$u],%%r%1$u",  "xld.b     [%%r%2$u + 0x%4$x],%%r%1$u" },   // 001 101 00
		{ "ld.b      [%%r%2$u]+,%%r%1$u", nullptr                                },   // 001 101 01
		{ "or        %%r%1$u,%%r%2$u",    "xoor      %%r%1$u,%%r%2$u,0x%4$x"     },   // 001 101 10
		{ "rl        %%r%1$u,%3$u",       nullptr                                },   // 001 101 11
		{ "ld.h      [%%r%2$u],%%r%1$u",  "xld.h     [%%r%2$u + 0x%4$x],%%r%1$u" },   // 001 110 00
		{ "ld.h      [%%r%2$u]+,%%r%1$u", nullptr                                },   // 001 110 01
		{ "xor       %%r%1$u,%%r%2$u",    "xxor      %%r%1$u,%%r%2$u,0x%4$x"     },   // 001 110 10
		{ nullptr,                        nullptr                                },
		{ "ld.w      [%%r%2$u],%%r%1$u",  "xld.w     [%%r%2$u + 0x%4$x],%%r%1$u" },   // 001 111 00
		{ "ld.w      [%%r%2$u]+,%%r%1$u", nullptr                                },   // 001 111 01
		{ "not       %%r%1$u,%%r%2$u",    nullptr                                },   // 001 111 10
		{ nullptr,                        nullptr                                } };

char const *const class_1_ext_shifts[4] = {
		nullptr,
		"ext       sra,%1$u",   // 001 110 11 0000 01
		"ext       srl,%1$u",   // 001 110 11 0000 10
		"ext       sll,%1$u" }; // 001 110 11 0000 11

char const *const class_1_ext_predicates[16] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		"ext       gt",    // 001 110 11 0100
		"ext       ge",    // 001 110 11 0101
		"ext       lt",    // 001 110 11 0110
		"ext       le",    // 001 110 11 0111
		"ext       ugt",   // 001 110 11 1000
		"ext       uge",   // 001 110 11 1001
		"ext       ult",   // 001 110 11 1010
		"ext       ule",   // 001 110 11 1011
		"ext       eq",    // 001 110 11 1100
		"ext       ne",    // 001 110 11 1101
		nullptr,
		nullptr };

std::pair<char const *, bool> class_1_ext_3[4] = {
		{ "ext       %%r%1$u",          true  },
		{ "ext       %%r%1$u,sra,%2$u", false },
		{ "ext       %%r%1$u,srl,%2$u", false },
		{ "ext       %%r%1$u,sll,%2$u", false } };

std::tuple<char const *, char const *, unsigned> const class_2_3_ops[16] = {
		{ "ld.b      %%r%1$u,[%%sp + 0x%2$x]",  "xld.b     %%r%1$u,[%%sp + 0x%2$x]",1 },   // 010 000
		{ "ld.ub     %%r%1$u,[%%sp + 0x%2$x]",  "xld.ub    %%r%1$u,[%%sp + 0x%2$x]",1 },   // 010 001
		{ "ld.h      %%r%1$u,[%%sp + 0x%2$x]",  "xld.h     %%r%1$u,[%%sp + 0x%2$x]",2 },   // 010 010
		{ "ld.uh     %%r%1$u,[%%sp + 0x%2$x]",  "xld.uh    %%r%1$u,[%%sp + 0x%2$x]",2 },   // 010 011
		{ "ld.w      %%r%1$u,[%%sp + 0x%2$x]",  "xld.w     %%r%1$u,[%%sp + 0x%2$x]",4 },   // 010 100
		{ "ld.b      [%%sp + 0x%2$x],%%r%1$u",  "xld.b     [%%sp + 0x%2$x],%%r%1$u",1 },   // 010 101
		{ "ld.h      [%%sp + 0x%2$x],%%r%1$u",  "xld.h     [%%sp + 0x%2$x],%%r%1$u",2 },   // 010 110
		{ "ld.w      [%%sp + 0x%2$x],%%r%1$u",  "xld.w     [%%sp + 0x%2$x],%%r%1$u",4 },   // 010 111
		{ "add       %%r%1$u,0x%2$x",           "xadd      %%r%1$u,0x%2$x",         1 },   // 011 000
		{ "sub       %%r%1$u,0x%2$x",           "xsub      %%r%1$u,0x%2$x",         1 },   // 011 001
		{ "cmp       %%r%1$u,%5$s0x%4$x",       "xcmp      %%r%1$u,%5$s0x%4$x",     1 },   // 011 010
		{ "ld.w      %%r%1$u,%5$s0x%4$x",       "xld.w     %%r%1$u,%5$s0x%4$x",     1 },   // 011 011
		{ "and       %%r%1$u,0x%3$x",           "xand      %%r%1$u,0x%3$x",         1 },   // 011 100
		{ "or        %%r%1$u,0x%3$x",           "xoor      %%r%1$u,0x%3$x",         1 },   // 011 101
		{ "xor       %%r%1$u,0x%3$x",           "xxor      %%r%1$u,0x%3$x",         1 },   // 011 110
		{ "not       %%r%1$u,0x%3$x",           "xnot      %%r%1$u,0x%3$x",         1 } }; // 011 111

std::pair<char const *, bool> const class_4_ops[32] = {
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ nullptr,                     false },
		{ "srl       %%r%1$u,%2$u",    false },   // 100 010 00
		{ "srl       %%r%1$u,%%r%2$u", false },   // 100 010 01
		{ "scan0     %%r%1$u,%%r%2$u", false },   // 100 010 10
		{ "div0s     %%r%2$u",         true  },   // 100 010 11
		{ "sll       %%r%1$u,%2$u",    false },   // 100 011 00
		{ "sll       %%r%1$u,%%r%2$u", false },   // 100 011 01
		{ "scan1     %%r%1$u,%%r%2$u", false },   // 100 011 10
		{ "div0u     %%r%2$u",         true  },   // 100 011 11
		{ "sra       %%r%1$u,%2$u",    false },   // 100 100 00
		{ "sra       %%r%1$u,%%r%2$u", false },   // 100 100 01
		{ "swap      %%r%1$u,%%r%2$u", false },   // 100 100 10
		{ "div1      %%r%2$u",         true  },   // 100 100 11
		{ "sla       %%r%1$u,%2$u",    false },   // 100 101 00
		{ "sla       %%r%1$u,%%r%2$u", false },   // 100 101 01
		{ "mirror    %%r%1$u,%%r%2$u", false },   // 100 101 10
		{ "div2s     %%r%2$u",         true  },   // 100 101 11
		{ "rr        %%r%1$u,%2$u",    false },   // 100 110 00
		{ "rr        %%r%1$u,%%r%2$u", false },   // 100 110 01
		{ "swaph     %%r%1$u,%%r%2$u", false },   // 100 110 10
		{ "div3s     %%r%2$u",         true  },   // 100 110 11
		{ "rl        %%r%1$u,%2$u",    false },   // 100 111 00
		{ "rl        %%r%1$u,%%r%2$u", false },   // 100 111 01
		{ "sat.b     %%r%1$u,%%r%2$u", false },   // 100 111 10
		{ "sat.ub    %%r%1$u,%%r%2$u", false } }; // 100 111 11

std::tuple<char const *, char const *, unsigned, unsigned> const class_5_ops[32] = {
		{ "ld.w      %4$s,%%r%2$d",    nullptr,                             4, 1 },   // 101 000 00
		{ "ld.b      %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 000 01
		{ "mlt.h     %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 000 10
		{ "mlt.hw    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 000 11
		{ "ld.w      %%r%1$d,%4$s",    nullptr,                             4, 2 },   // 101 001 00
		{ "ld.ub     %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 001 01
		{ "mltu.h    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 001 10
		{ "mac1.h    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 001 11
		{ "btst      [%%r%2$u],%1$u",  "xbtst     [%%r%2$u + 0x%3$x],%1$u", 3, 0 },   // 101 010 00
		{ "ld.h      %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 010 01
		{ "mlt.w     %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 010 10
		{ "mac1.hw   %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 010 11
		{ "bclr      [%%r%2$u],%1$u",  "xbclr     [%%r%2$u + 0x%3$x],%1$u", 3, 0 },   // 101 011 00
		{ "ld.uh     %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 011 01
		{ "mltu.w    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 011 10
		{ nullptr,                     nullptr,                             0, 0 },
		{ "bset      [%%r%2$u],%1$u",  "xbset     [%%r%2$u + 0x%3$x],%1$u", 3, 0 },   // 101 100 00
		{ "ld.c      %%r%1$u,0x%2$x",  nullptr,                             4, 0 },   // 101 100 01
		{ "mac       %%r%2$u",         nullptr,                             0, 0 },   // 101 100 10
		{ "mac1.w    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 100 11
		{ "bnot      [%%r%2$u],%1$u",  "xbnot     [%%r%2$u + 0x%3$x],%1$u", 3, 0 },   // 101 101 00
		{ "ld.c      0x%2$x,%%r%1$u",  nullptr,                             4, 0 },   // 101 101 01
		{ "sat.h     %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 101 10
		{ "sat.uh    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 101 11
		{ "addc      %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 110 00
		{ "loop      %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 110 01
		{ "loop      %%r%1$u,%2$u",    nullptr,                             4, 0 },   // 101 110 10
		{ "loop      %1$u,%2$u",       nullptr,                             4, 0 },   // 101 110 11
		{ "sbc       %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 111 00
		{ "sat.w     %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 111 01
		{ "sat.uw    %%r%1$u,%%r%2$u", nullptr,                             4, 0 },   // 101 111 10
		{ nullptr,                     nullptr,                             0, 0 } };

std::tuple<char const *, char const *, unsigned> const class_7_ops[8] = {
		{ "ld.b      %%r%1$u,[%%dp + 0x%2$x]", "xld.b     %%r%1$u,[%%dp + 0x%2$x]", 1 },   // 111 000
		{ "ld.ub     %%r%1$u,[%%dp + 0x%2$x]", "xld.ub    %%r%1$u,[%%dp + 0x%2$x]", 1 },   // 111 001
		{ "ld.h      %%r%1$u,[%%dp + 0x%2$x]", "xld.h     %%r%1$u,[%%dp + 0x%2$x]", 2 },   // 111 010
		{ "ld.uh     %%r%1$u,[%%dp + 0x%2$x]", "xld.uh    %%r%1$u,[%%dp + 0x%2$x]", 2 },   // 111 011
		{ "ld.w      %%r%1$u,[%%dp + 0x%2$x]", "xld.w     %%r%1$u,[%%dp + 0x%2$x]", 4 },   // 111 100
		{ "ld.b      [%%dp + 0x%2$x],%%r%1$u", "xld.b     [%%dp + 0x%2$x],%%r%1$u", 1 },   // 111 101
		{ "ld.h      [%%dp + 0x%2$x],%%r%1$u", "xld.h     [%%dp + 0x%2$x],%%r%1$u", 2 },   // 111 110
		{ "ld.w      [%%dp + 0x%2$x],%%r%1$u", "xld.w     [%%dp + 0x%2$x],%%r%1$u", 4 } }; // 111 111

} // anonymous namespace

c33_disassembler::c33_disassembler()
{
}


u32 c33_disassembler::opcode_alignment() const
{
	return 2;
}


offs_t c33_disassembler::disassemble(
		std::ostream &stream,
		offs_t pc,
		data_buffer const &opcodes,
		data_buffer const &params)
{
	u16 op;

	u32 ext_imm = 0;
	unsigned ext_count = 0;
	op = opcodes.r16(pc);
	if (BIT(op, 13, 3) == 6)
	{
		ext_imm = BIT(op, 0, 13);
		ext_count = 1;
		op = opcodes.r16(pc + 2);
		if (BIT(op, 13, 3) == 6)
		{
			ext_imm = (ext_imm << 13) | BIT(op, 0, 13);
			ext_count = 2;
			op = opcodes.r16(pc + 4);
		}
	}

	switch (BIT(op, 13, 3))
	{
	case 0:
		if (offs_t const r = dasm_class_0(stream, pc, op, ext_imm, ext_count); r)
			return r;
		break;
	case 1:
		if (offs_t const r = dasm_class_1(stream, op, ext_imm, ext_count); r)
			return r;
		break;
	case 2:
	case 3:
		if (offs_t const r = dasm_class_2_3(stream, op, ext_imm, ext_count); r)
			return r;
		break;
	case 4:
		if (offs_t const r = dasm_class_4(stream, op, ext_imm, ext_count); r)
			return r;
		break;
	case 5:
		if (offs_t const r = dasm_class_5(stream, op, ext_imm, ext_count); r)
			return r;
		break;
	case 6:
		if (offs_t const r = dasm_class_6(stream, op, ext_imm, ext_count); r)
			return r;
		break;
	case 7:
		if (offs_t const r = dasm_class_7(stream, op, ext_imm, ext_count); r)
			return r;
		break;
	};

	if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
		return r;

	stream << "<invalid>";
	return 2;
}


offs_t c33_disassembler::dasm_class_0(std::ostream &stream, offs_t pc, u16 op, u32 ext_imm, unsigned ext_count) const
{
	switch (BIT(op, 9, 4))
	{
	case 0b0000:
	case 0b0001:
	case 0b0010:
	case 0b0011:
		if (BIT(op, 4, 2) == 0)
		{
			auto const [inst, target_bits] = class_0_00_ops[BIT(op, 6, 5)];
			if (inst && ((4 == target_bits) || !BIT(op, target_bits, 4 - target_bits)))
			{
				if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
					return r;

				util::stream_format(stream, inst, BIT(op, 0, 4));
				return 2;
			}
		}
		else if (BIT(op, 4, 2) == 1)
		{
			auto const [inst, use_special_reg, no_arg] = class_0_01_ops[BIT(op, 6, 5)];
			char const *const special_reg = use_special_reg ? special_reg_names[BIT(op, 0, 4)] : nullptr;
			if (inst && (!use_special_reg || special_reg) && (!no_arg || !BIT(op, 0, 4)))
			{
				if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
					return r;

				util::stream_format(stream, inst, BIT(op, 0, 4), special_reg);
				return 2;
			}
		}
		break;

	default:
		{
			s32 imm = (BIT(ext_imm, 16, 10) << 22) | (BIT(ext_imm, 0, 13) << 9) | (BIT(op, 0, 8) << 1);
			if (!ext_count)
				imm = util::sext(imm, 9);
			else if (1 == ext_count)
				imm = util::sext(imm, 22);

			auto const [inst, inst_ext] = class_0_jumps[BIT(op, 9, 5)];
			util::stream_format(stream, ext_count ? inst_ext : inst, BIT(op, 8) ? ".d" : "  ", pc + (ext_count * 2) + imm);
			return 2 + (ext_count * 2);
		}
	}

	return 0; // catch invalid forms
}

offs_t c33_disassembler::dasm_class_1(std::ostream &stream, u16 op, u32 ext_imm, unsigned ext_count) const
{
	switch (BIT(op, 8, 5))
	{
	case 0b110'11:
		if (!BIT(op, 4, 4))
		{
			char const *const inst = class_1_ext_shifts[BIT(op, 2, 2)];
			if (inst)
			{
				if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
					return r;

				util::stream_format(stream, inst, BIT(op, 0, 2));
				return 2;
			}
		}
		else
		{
			char const *const inst = class_1_ext_predicates[BIT(op, 4, 4)];
			if (inst && !BIT(op, 0, 4))
			{
				if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
					return r;

				stream << inst;
				return 2;
			}
		}
		break;

	case 0b111'11:
		{
			auto const [inst, no_imm] = class_1_ext_3[BIT(op, 2, 2)];
			if (!no_imm || !BIT(op, 0, 2))
			{
				if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
					return r;

				util::stream_format(stream, inst, BIT(op, 4, 4), BIT(op, 0, 2));
				return 2;
			}
		}
		break;

	default:
		{
			auto const [inst, inst_ext] = class_1_ops[BIT(op, 8, 5)];
			if (offs_t const r = max_ext(stream, inst_ext ? 2 : 0, ext_imm, ext_count); r)
				return r;

			util::stream_format(stream, ext_count ? inst_ext : inst, BIT(op, 0, 4), BIT(op, 4, 4), 0x10 | BIT(op, 4, 4), ext_imm);
			return 2 + (ext_count * 2);
		}
	}

	return 0; // catch invalid ext forms
}


offs_t c33_disassembler::dasm_class_2_3(std::ostream &stream, u16 op, u32 ext_imm, unsigned ext_count) const
{
	auto const [inst, inst_ext, mul] = class_2_3_ops[BIT(op, 10, 4)];

	u8 const reg = BIT(op, 0, 4);
	u32 imm = (ext_imm << 6) | BIT(op, 4, 6);
	s32 simm = util::sext(imm, 6 + (ext_count * 13));
	u32 abs = std::abs(simm);
	char const *const sign = (0 > simm) ? "-" : "";
	if (!ext_count)
	{
		imm *= mul;
		simm *= mul;
		abs *= mul;
	}
	util::stream_format(stream, ext_count ? inst_ext : inst, reg, imm, simm, abs, sign);
	return 2 + (ext_count * 2);
}


offs_t c33_disassembler::dasm_class_4(std::ostream &stream, u16 op, u32 ext_imm, unsigned ext_count) const
{
	if (BIT(op, 11, 2) == 0)
	{
		if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
			return r;

		util::stream_format(stream, BIT(op, 10) ? "sub       %%sp,0x%1$x" : "add       %%sp,0x%1$x", BIT(op, 0, 10) * 4);
		return 2;
	}
	else
	{
		auto const [inst, no_dest] = class_4_ops[BIT(op, 8, 5)];
		if (!no_dest || !BIT(op, 0, 4))
		{
			if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
				return r;

			util::stream_format(stream, inst, BIT(op, 0, 4), BIT(op, 4, 4));
			return 2;
		}
	}

	return 0; // catch invalid forms of division instructions
}


offs_t c33_disassembler::dasm_class_5(std::ostream &stream, u16 op, u32 ext_imm, unsigned ext_count) const
{
	if (BIT(op, 6, 7) == 0b111'11'00)
	{
		if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
			return r;

		util::stream_format(stream, "do.c      0x%1$x", BIT(op, 0, 6));
		return 2;
	}
	else if (BIT(op, 5, 8) == 0b111'11'01'0)
	{
		if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
			return r;

		util::stream_format(stream, "psrset    %1$u", BIT(op, 0, 5));
		return 2;
	}
	else if (BIT(op, 5, 8) == 0b111'11'10'0)
	{
		if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
			return r;

		util::stream_format(stream, "psrclr    %1$u", BIT(op, 0, 5));
		return 2;
	}
	else
	{
		auto const [inst, inst_ext, target_bits, special_reg_pos] = class_5_ops[BIT(op, 8, 5)];
		char const *const special_reg = special_reg_pos ? special_reg_names[BIT(op, (special_reg_pos - 1) * 4, 4)] : nullptr;
		if (inst && (!special_reg_pos || special_reg) && ((4 == target_bits) || !BIT(op, target_bits, 4 - target_bits)))
		{
			if (offs_t const r = max_ext(stream, inst_ext ? 2 : 0, ext_imm, ext_count); r)
				return r;

			util::stream_format(stream, ext_count ? inst_ext : inst, BIT(op, 0, 4), BIT(op, 4, 4), ext_imm, special_reg);
			return 2 + (ext_count * 2);
		}
	}

	return 0; // catch invalid forms
}


offs_t c33_disassembler::dasm_class_6(std::ostream &stream, u16 op, u32 ext_imm, unsigned ext_count) const
{
	if (offs_t const r = max_ext(stream, 0, ext_imm, ext_count); r)
		return r;

	util::stream_format(stream, "ext       0x%1$x", BIT(op, 0, 13));
	return 2;
}


offs_t c33_disassembler::dasm_class_7(std::ostream &stream, u16 op, u32 ext_imm, unsigned ext_count) const
{
	auto const [inst, inst_ext, mul] = class_7_ops[BIT(op, 10, 3)];

	u8 const reg = BIT(op, 0, 4);
	u32 imm = (ext_imm << 6) | BIT(op, 4, 6);
	if (!ext_count)
		imm *= mul;
	util::stream_format(stream, ext_count ? inst_ext : inst, reg, imm);
	return 2 + (ext_count * 2);
}


offs_t c33_disassembler::max_ext(std::ostream &stream, unsigned max, u32 ext_imm, unsigned ext_count) const
{
	if (max >= ext_count)
	{
		return 0;
	}
	else
	{
		util::stream_format(stream, "ext       0x%1$x", BIT(ext_imm, (ext_count - 1) * 13, 13));
		return 2;
	}
}
