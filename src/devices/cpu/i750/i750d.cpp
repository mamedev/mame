// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Intel 82750PB disassembler
*/

#include "emu.h"
#include "i750d.h"

namespace {

constexpr const char *const A_SOURCES[64] =
{
	"null", "hwid", "cc", "-", "alu", "cnt", "cnt2", "lcnt",
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"mcode3", "mcode2", "mcode1", "pc", "-", "pixint", "*dram1", "*dram2",
	"*dram1++", "*dram2++", "*dram1--", "*dram2--", "dram1", "dram2", "dram3", "dram4",
	"*in1", "*in2", "*stat", "*stat#", "-", "-", "-", "-",
	"-", "-", "-", "in1-hi", "-", "-", "-", "in2-hi",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"shift", "-", "*dram3", "*dram4", "*dram3++", "*dram4++", "*dram3--", "*dram4--",
};

constexpr const char *const A_DESTINATIONS[64] =
{
	"null", "-", "-", "maddr", "-", "cnt", "cnt2", "-",
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"mcode3", "mcode2", "mcode1", "pc", "pixint-c", "pixint", "*dram1", "*dram2",
	"*dram1++", "*dram2++", "*dram1--", "*dram2--", "dram1", "dram2", "dram3", "dram4",
	"*out1", "out1++", "shift-rl", "out1-hi", "*out2", "out2++", "shift-r", "out2-hi",
	"out1-c", "in1-c", "shift-l", "in1-hi", "out2-c", "in2-c", "-", "in2-hi",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"cc", "-", "*dram3", "*dram4", "*dram3++", "*dram4++", "*dram3--", "*dram4--",
};

constexpr const char *const B_SOURCES[64] =
{
	"null", "alu", "*dram3", "*dram4", "*dram3++", "*dram4++", "*dram3--", "*dram4--",
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"*in1", "*in2", "*stat", "*stat#", "-", "-", "-", "-",
	"#0", "#1", "#2", "#3", "#4", "#5", "#6", "#7",
	"prof", "-", "out1-lo", "out1-hi", "stat-lo", "stat-hi", "out2-lo", "out2-hi",
	"out1-c", "in1-c", "in1-lo", "in1-hi", "out2-c", "in2-c", "in2-lo", "in2-hi",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"shift", "stat-c", "*dram1", "*dram2", "*dram1++", "*dram2++", "*dram1--", "*dram2--",
};

constexpr const char *const B_DESTINATIONS[64] =
{
	"null", "-", "*dram3", "*dram4", "*dram3++", "*dram4++", "*dram3--", "*dram4--",
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"-", "-", "*dram1", "*dram2", "*dram1++", "*dram2++", "*dram1--", "*dram2--",
	"*out1", "out1++", "out1-lo", "out1-hi", "*out2", "out2++", "out2-lo", "out2-hi",
	"out1-c", "in1-c", "in1-lo", "in1-hi", "out2-c", "in2-c", "in2-lo", "in2-hi",
	"stat-ram", "stat-c", "stat-lo", "stat-hi", "yeven-lo", "yeven-hi", "yodd-lo", "yodd-hi",
	"ypitch", "-", "vu-lo", "vu-hi", "vupitch", "vpitch", "vptr-lo", "vptr-hi",
};

constexpr const char *const OPERATIONS[32] =
{
	"nop", "zero", "a", "b", "~a", "~b", "a&b", "~a&b",
	"a&~b", "a+b+1", "a|b", "~(a|b)", "a^b", "a-b-1+c", "a-b-1", "b-a-1+c",
	"a+b", "a-b", "b-a", "-a", "-b", "a+1", "b+1", "a-1",
	"b-1", "int", "prof", "a*", "b*", "a+b+c", "add-clamp", "sub-clamp",
};

constexpr const char *const FLAGS[8] =
{
	"false", "carry", "overflow", "sign", "zero", "cntzero", "r0lsb", "r0msb",
};

} // anonymous namespace

u32 i750_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t i750_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u64 w = opcodes.r64(pc);

	util::stream_format(stream, "A %s=", A_DESTINATIONS[BIT(w, 18, 6)]);
	if (BIT(w, 37))
	{
		util::stream_format(stream, "#%03x", BIT(w, 9, 9));
	}
	else
	{
		stream << A_SOURCES[BIT(w, 12, 6)];
	}

	util::stream_format(stream, "; B %s=%s; %s ss=%u; cnt%u%s; sh=%u; %s -> %03x",
			B_DESTINATIONS[BIT(w, 30, 6)], B_SOURCES[BIT(w, 24, 6)], OPERATIONS[BIT(w, 40, 5)], BIT(w, 38, 2),
			BIT(w, 47), BIT(w, 36) ? "--" : "", BIT(w, 45, 2), FLAGS[BIT(w, 37) ? 0 : BIT(w, 9, 3)], w & 0x1ff);
	return 1 | SUPPORTED;
}
