// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VT50/VT52 microcode disassembler

***************************************************************************/

#include "emu.h"
#include "vt50dasm.h"

vt5x_disassembler::vt5x_disassembler(const char *const opcodes_e[8], const char *const opcodes_f[8], const char *const opcodes_g[8], const char *const jumps_h[2][8], const char *const opcodes_w[8])
	: util::disasm_interface()
	, m_opcodes_e(opcodes_e)
	, m_opcodes_f(opcodes_f)
	, m_opcodes_g(opcodes_g)
	, m_jumps_h(jumps_h)
	, m_opcodes_w(opcodes_w)
{
}

vt50_disassembler::vt50_disassembler()
	: vt5x_disassembler(s_opcodes_e, s_opcodes_f, s_opcodes_g, s_jumps_h, s_opcodes_w)
{
}

vt52_disassembler::vt52_disassembler()
	: vt5x_disassembler(s_opcodes_e, s_opcodes_f, s_opcodes_g, s_jumps_h, s_opcodes_w)
{
}

const char *const vt5x_disassembler::s_opcodes_e[8] = {
	"ZXZY", "X8", "IXDY", "IX", "ZA", "M1", "ZX", "M0"
};

const char *const vt5x_disassembler::s_opcodes_f[8] = {
	"DXDY", "IA", "IA1", "IY", "DY", "IROM", "DX", "DA" // IA is functionally duplicated
};

const char *const vt50_disassembler::s_opcodes_g[8] = {
	"M2A", "A2M", "M2U", "L40M", "M2X", "U2M", "M2B", "SPARE"
};

const char *const vt52_disassembler::s_opcodes_g[8] = {
	"M2A", "A2M", "M2U", "B2M", "M2X", "U2M", "M2B", "GRPH"
};

const char *const vt50_disassembler::s_jumps_h[2][8] = {
	{ "PSC", "TAB", "KCL", "FRQ", "PRQ", "TRU", "UT", "TOS" }, // mode 0
	{ "UR", "AEM", "ALM", "ADX", "AEM2", nullptr, "VSC", "KEY" } // mode 1
};

const char *const vt52_disassembler::s_jumps_h[2][8] = {
	{ "PSC", "TAB", "KCL", "FRQ", "PRQ", "COP", "UT", "TOS" }, // mode 0
	{ "UR", "AEM", "ALM", "ADX", "AEM2", "TRU", "VSC", "KEY" } // mode 1
};

const char *const vt5x_disassembler::s_opcodes_w[8] = {
	"SCFF", "SVID", "B2Y", "CBFF", "ZCAV", "LPB", "EPR", "HPR!ZY"
};

u32 vt5x_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t vt5x_disassembler::disassemble(std::ostream &stream, offs_t pc, const vt5x_disassembler::data_buffer &opcodes, const vt5x_disassembler::data_buffer &params)
{
	u8 opcode = opcodes.r8(pc);

	if (BIT(opcode, 7))
	{
		util::stream_format(stream, "LD %03o", opcode & 0177); // execution varies by mode
		return 1 | SUPPORTED;
	}
	else if ((opcode & 0017) == 0)
	{
		stream << m_opcodes_w[(opcode & 0160) >> 4];
		return 1 | SUPPORTED;
	}
	else
	{
		bool first = true;
		if (BIT(opcode, 3))
		{
			first = false;
			stream << m_opcodes_e[(opcode & 0160) >> 4];
		}
		if (BIT(opcode, 2))
		{
			if (!first)
				stream << "!";
			first = false;
			stream << m_opcodes_f[(opcode & 0160) >> 4];
		}
		if (BIT(opcode, 1))
		{
			if (!first)
				stream << "!";
			first = false;
			stream << m_opcodes_g[(opcode & 0160) >> 4];
		}
		if (BIT(opcode, 0))
		{
			if (!first)
				stream << "!";
			bool m0 = (opcode & 0170) != 0130;
			bool m1 = (opcode & 0170) != 0170;
			if (m_jumps_h[1][(opcode & 0160) >> 4] == nullptr)
				m0 = std::exchange(m1, false);
			if (m0)
			{
				util::stream_format(stream, "%sJ", m_jumps_h[0][(opcode & 0160) >> 4]);
				if (m1)
					stream << "/";
			}
			if (m1)
				util::stream_format(stream, "%sJ", m_jumps_h[1][(opcode & 0160) >> 4]);

			u16 nextpc = pc + 1;
			if ((opcode & 0164) == 0124) // IROM adjustment
				nextpc += 0400;
			util::stream_format(stream, " %04o", (nextpc & 01400) | opcodes.r8(pc + 1));
			return 2 | ((opcode & 0170) != 0130 ? STEP_COND : 0) | SUPPORTED;
		}
		else
			return 1 | SUPPORTED;
	}
}
