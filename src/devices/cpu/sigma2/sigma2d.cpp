// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    SDS/XDS Sigma 2/3 & Xerox 530 (16-bit) disassemblers

***************************************************************************/

#include "emu.h"
#include "sigma2d.h"

namespace {

const char *const s_inst_names[16] =
{
	"WD", "RD",
	"S", "MUL",
	"B", "DIV",
	"BC", "COPY", // non-memory instructions
	"LDA", "AND",
	"ADD", "SUB",
	"LDX", "CP",
	"STA", "IM"
};

const char *const s_shift_names[8] =
{
	"SARS", "SALS", "SCRS", "SCLS", // Single shifts
	"SARD", "SALD", "SCRD", "SCLD"  // Double shifts
};

const char *const s_bc_names[8] =
{
	"BNO", "BNC", "BAZ", "BIX",
	"BXNO", "BXNC", "BEN", "BAN"
};

const char *const s_copy_names[32] =
{
	"RAND", "", "RANDI", "", "RANDC", "", "", "",
	"ROR", "RCPY", "RORI", "RCPYI", "RORC", "RCPYC", "", "",
	"REOR", "", "REORI", "", "REORC", "", "", "",
	"RADD", "RCLA", "RADDI", "RCLAI", "RADDC", "RCLAC", "", ""
};

const char *const s_sigma2_reg_names[8] =
{
	"Z", "P",
	"L", "T",
	"X1", "X2",
	"E", "A"
};

const char *const s_xerox530_reg_names[8] =
{
	"Z", "P",
	"L", "T",
	"X", "B",
	"E", "A"
};

} // anonymous namespace

sigma2_disassembler::sigma2_disassembler(const char *const *reg_names)
	: util::disasm_interface()
	, m_reg_names(reg_names)
{
}

sigma2_disassembler::sigma2_disassembler()
	: sigma2_disassembler(s_sigma2_reg_names)
{
}

xerox530_disassembler::xerox530_disassembler()
	: sigma2_disassembler(s_xerox530_reg_names)
{
}

u32 sigma2_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t sigma2_disassembler::dasm_memory_reference(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) const
{
	if (BIT(inst, 10))
	{
		// Indirect addressing
		stream << '*';
	}
	if (BIT(inst, 11))
	{
		// Self-relative addressing
		util::stream_format(stream, "X'%04X'", (pc + util::sext(inst, 9)) & 0xffff);
		if (BIT(inst, 9))
			stream << ',' << m_reg_names[4];
	}
	else if (BIT(inst, 8, 2) == 0 || BIT(inst, 8, 3) == 6)
	{
		// Nonrelative direct addressing
		util::stream_format(stream, "X'%04X'", inst & 0x00ff);
		if (BIT(inst, 9))
			stream << ',' << m_reg_names[4];

		// Interrupt exit sequence (very special case in which LDX loads PSD and not X1)
		if (inst == 0x00d8 && (opcodes.r16(pc + 1) & 0xf000) == 0xc000)
			return 1 | STEP_OUT | step_over_extra(1) | SUPPORTED;
	}
	else
	{
		// Indexed addressing
		if ((inst & 0x00ff) <= 9)
			util::stream_format(stream, "%d,", inst & 0x00ff);
		else
			util::stream_format(stream, "X'%X',", inst & 0x00ff);
		if (BIT(inst, 9))
			stream << m_reg_names[4];
		if (BIT(inst, 8))
			stream << ',' << m_reg_names[5];
	}

	return 1 | SUPPORTED;
}

offs_t sigma2_disassembler::dasm_read_direct(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) const
{
	// RD internal control functions
	if (inst == 0x1041)
	{
		stream << "SIO";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1042)
	{
		stream << "TIO";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1044)
	{
		stream << "TDV";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1048)
	{
		stream << "HIO";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1050)
	{
		stream << "AIO";
		return 1 | SUPPORTED;
	}
	else if (inst > 0x1080 && inst <= 0x10bf)
	{
		// Multiple precision mode prefix
		util::stream_format(stream, "%-8s%s,%d", "SMP", m_reg_names[BIT(inst, 0, 3)], BIT(inst, 3, 3));
		return 1 | SUPPORTED;
	}
	else
	{
		// Fall back to disassembling RD using RIXS modes
		util::stream_format(stream, "%-8s", "RD");
		return dasm_memory_reference(stream, pc, opcodes, inst);
	}
}

offs_t xerox530_disassembler::dasm_read_direct(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) const
{
	// RD internal control functions
	if (inst == 0x1041)
	{
		stream << "SIO";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1042)
	{
		stream << "TIO";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1044)
	{
		stream << "TDV";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1048)
	{
		stream << "HIO";
		return 1 | SUPPORTED;
	}
	else if (inst == 0x1050)
	{
		stream << "AIO";
		return 1 | SUPPORTED;
	}
	else if (inst >= 0x108a && inst <= 0x108e)
	{
		// General register instructions
		u16 inst2 = opcodes.r16(pc + 1);
		switch (BIT(inst2, 12, 4))
		{
		case 8:
			util::stream_format(stream, "LW,%-5s", m_reg_names[BIT(inst, 0, 3)]);
			break;

		case 9:
			util::stream_format(stream, "AND,%-4s", m_reg_names[BIT(inst, 0, 3)]);
			break;

		case 0xa:
			util::stream_format(stream, "AW,%-5s", m_reg_names[BIT(inst, 0, 3)]);
			break;

		case 0xb:
			util::stream_format(stream, "SW,%-5s", m_reg_names[BIT(inst, 0, 3)]);
			break;

		case 0xd:
			util::stream_format(stream, "CW,%-5s", m_reg_names[BIT(inst, 0, 3)]);
			break;

		case 0xe:
			util::stream_format(stream, "STW,%-4s", m_reg_names[BIT(inst, 0, 3)]);
			break;

		default:
			util::stream_format(stream, "%-8sX'%04X'", "DATA", inst);
			return 1 | SUPPORTED;
		}
		return 1 + dasm_memory_reference(stream, pc + 1, opcodes, inst2);
	}
	else if (inst == 0x109e)
	{
		// Floating mode: LDA, STA, ADD, SUB, MUL, DIV, CP executed as FLD, FST, FAD, FSB, FML, FDV, FCP until next branch
		stream << "SFM";
		return 1 | SUPPORTED;
	}
	else if (inst >= 0x1088 && inst <= 0x10bf)
	{
		u16 inst2 = opcodes.r16(pc + 1);
		if (BIT(inst, 0, 3) <= 1 || BIT(inst, 0, 3) == 7)
		{
			// Field addressing instructions
			switch (BIT(inst2, 12, 4))
			{
			case 5:
				stream << "CLF";
				break;

			case 8:
				stream << "LLF";
				break;

			case 9:
				stream << "LAF";
				break;

			case 0xa:
				stream << "STF";
				break;

			case 0xb:
				stream << "SZF";
				break;

			case 0xc:
				stream << "SOF";
				break;

			case 0xd:
				stream << "CAF";
				break;

			case 0xf:
				stream << "SLF";
				break;

			default:
				util::stream_format(stream, "%-8sX'%04X'", "DATA", inst);
				return 1 | SUPPORTED;
			}
			util::stream_format(stream, ",%s,%d ", BIT(inst, 3, 3) == 1 ? "0" : m_reg_names[BIT(inst, 3, 3)], util::sext(inst, 3));
			return 1 + dasm_memory_reference(stream, pc + 1, opcodes, inst2);
		}
		else if (inst == 0x1096)
		{
			// Double-register instructions
			switch (BIT(inst2, 12, 4))
			{
			case 8:
				util::stream_format(stream, "%-8s", "LDD");
				break;

			case 0xa:
				util::stream_format(stream, "%-8s", "DAD");
				break;

			case 0xb:
				util::stream_format(stream, "%-8s", "DSB");
				break;

			case 0xd:
				util::stream_format(stream, "%-8s", "CPD");
				break;

			case 0xe:
				util::stream_format(stream, "%-8s", "STD");
				break;

			default:
				util::stream_format(stream, "%-8sX'%04X'", "DATA", inst);
				return 1 | SUPPORTED;
			}
			return 1 + dasm_memory_reference(stream, pc + 1, opcodes, inst2);
		}
		else if (BIT(inst, 3, 3) + BIT(inst, 0, 3) <= 8 && (BIT(inst2, 12, 4) == 8 || BIT(inst2, 12, 4) == 0xe))
		{
			// Multiple-register load and store instructions
			util::stream_format(stream, "%-8s", inst2 >= 0xe000 ? "STM" : "LDM");
			offs_t result = 1 + dasm_memory_reference(stream, pc + 1, opcodes, inst2);
			util::stream_format(stream, ",%s,%d", m_reg_names[BIT(inst, 0, 3)], BIT(inst, 3, 3));
			return result;
		}
		else
		{
			util::stream_format(stream, "%-8sX'%04X'", "DATA", inst);
			return 1 | SUPPORTED;
		}
	}
	else
	{
		// Fall back to disassembling RD using RIXS modes
		util::stream_format(stream, "%-8s", "RD");
		return dasm_memory_reference(stream, pc, opcodes, inst);
	}
}

offs_t sigma2_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	if ((inst & 0xf000) == 0x6000)
	{
		// Conditional branch instructions use self-relative addressing only
		util::stream_format(stream, "%-8sX'%04X'", s_bc_names[BIT(inst, 9, 3)], (pc + util::sext(inst, 9)) & 0xffff);
		return 1 | STEP_COND | SUPPORTED;
	}
	else if ((inst & 0xf000) == 0x7000)
	{
		// Copy instructions
		const char *name = s_copy_names[BIT(inst, 7, 5)];
		if (name[0] != '\0')
		{
			util::stream_format(stream, "%-8s", name);

			// Optionally invert source
			if (BIT(inst, 3))
				stream << '*';
			util::stream_format(stream, "%s,%s", m_reg_names[BIT(inst, 0, 3)], m_reg_names[BIT(inst, 4, 3)]);

			// Recognize branch-and-link calling convention
			if (inst == 0x75a1 && (opcodes.r16(pc + 1) & 0xf000) == 0x4000)
				return 1 | STEP_OVER | step_over_extra(1) | SUPPORTED;
			else if ((inst & 0xfcff) == 0x7492)
				return 1 | STEP_OUT | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8sX'%04X'", "DATA", inst);
		return 1 | SUPPORTED;
	}
	else if ((inst & 0xfc00) == 0x2000)
	{
		// Shift instructions
		util::stream_format(stream, "%-8s%d", s_shift_names[BIT(inst, 5, 3)], BIT(inst, 0, 5));
		if (BIT(inst, 8, 2) != 0)
		{
			// Shift count may be indexed
			stream << ',';
			if (BIT(inst, 9))
				stream << m_reg_names[4];
			if (BIT(inst, 8))
				stream << ',' << m_reg_names[5];
		}
		return 1 | SUPPORTED;
	}
	else if ((inst & 0xf000) == 0x1000)
		return dasm_read_direct(stream, pc, opcodes, inst);
	else
	{
		// Memory reference instructions (RIXS)
		util::stream_format(stream, "%-8s", s_inst_names[BIT(inst, 12, 4)]);
		return dasm_memory_reference(stream, pc, opcodes, inst);
	}
}
