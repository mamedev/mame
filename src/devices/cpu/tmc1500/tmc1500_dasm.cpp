// license:BSD-3-Clause
// copyright-holders:baco, Jeff Parsons
/***************************************************************************

    Texas Instruments TMC1500 family disassembler

    Decodes the 13-bit instruction word of the TMC1500 CPU family.
    Includes support for standard arithmetic, masked operational groups,
    and specialized PF/FF group instructions.

***************************************************************************/

#include "emu.h"
#include "tmc1500_dasm.h"

tmc1500_disassembler::tmc1500_disassembler()
{
}

offs_t tmc1500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint16_t op = opcodes.r16(pc) & 0x1fff;

	// 1. Control Flow (Bit 12 = 1)
	if (op & 0x1000) {
		if (op & 0x0800) {
			const char *name = (op & 0x0400) ? "BRC" : "BRNC";
			util::stream_format(stream, "%-8s $%03X", name, (pc & 0x0400) | (op & 0x03ff));
		} else {
			util::stream_format(stream, "%-8s $%03X", "CALL", op & 0x07ff);
		}
		return 1 | SUPPORTED;
	}

	// 2. Masked Operations (Bit 12 = 0)
	int mask = (op >> 8) & 0x0f;
	int j    = (op >> 6) & 0x03;
	int k    = (op >> 3) & 0x07;
	int l    = (op >> 1) & 0x03;
	int n    = op & 0x01;

	static const char *regs[4] = { "A", "B", "C", "D" };
	static const char *s_src2[8] = { "A", "B", "C", "D", "1", "SHF", "R5L", "R5" };
	static const char *s_mask[16] = { "MMSD", "ALL", "MANT", "MAEX", "LLSD", "EXP", "RES1", "FMAEX", "D14", "FLAG", "DIGIT", "RES2", "RES3", "D13", "RES4", "D15" };

	// FLAG Operations (Group FF)
	if (mask == 0x0c) {
		int d = (op >> 4) & 0x03;
		int b = (op >> 2) & 0x03;
		int mm = op & 0x03;
		static const char *s_mm[4] = { "SET", "RES", "TST", "NOT" };
		if (d != 0) {
			util::stream_format(stream, "FLAG.%s %s[%d].%d", s_mm[mm], regs[j], 12 + d, b);
			return 1 | SUPPORTED;
		}
	}
	
	// Miscellaneous Operations (Group PF)
	if (mask == 0x0e) {
		int pf = op & 0x0f;
		static const char *s_pf[16] = { "STYA", "RABI", "BRR5", "RET", "STAX", "STXA", "STAY", "DISP", "BCDS", "BCDR", "RABR5", "RES", "RES", "RES", "RES", "RES" };
		if (pf == 1) util::stream_format(stream, "RABI %d", (op >> 4) & 7);
		else util::stream_format(stream, "%s", s_pf[pf]);
		return 1 | SUPPORTED;
	}

	// Operator selection
	const char *s_operator = n ? "-" : "+";
	if (k == 5) s_operator = n ? ">>" : "<<";

	char s_dst[16];
	if (l == 0) sprintf(s_dst, "%s", regs[j]);
	else if (l == 1) sprintf(s_dst, "%s", (k < 4 ? regs[k] : "NUL"));
	else if (l == 2) sprintf(s_dst, "%s", (k < 5 ? "NUL" : regs[j]));
	else {
		// MOVE / XCHG
		if (n == 0) util::stream_format(stream, "XCHG A,%s,%s", s_src2[k], s_mask[mask]);
		else util::stream_format(stream, "MOVE %s,%s,%s", regs[j], s_src2[k], s_mask[mask]);
		return 1 | SUPPORTED;
	}

	// Format standard arithmetic/shift instruction
	util::stream_format(stream, "%-8s %s,%s%s%s,%s", (k == 5 ? "SHF" : (n ? "SUB" : "ADD")), s_dst, regs[j], s_operator, s_src2[k], s_mask[mask]);

	return 1 | SUPPORTED;
}
