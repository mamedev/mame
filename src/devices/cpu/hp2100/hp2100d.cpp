// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hewlett-Packard HP2100 disassembler

***************************************************************************/

#include "emu.h"
#include "hp2100d.h"

hp2100_disassembler::hp2100_disassembler()
	: util::disasm_interface()
{
}

u32 hp2100_disassembler::opcode_alignment() const
{
	return 1;
}

void hp2100_disassembler::format_address(std::ostream &stream, u16 addr) const
{
	if ((addr & 077776) == 0)
		stream << char('A' + BIT(addr, 0));
	else
		util::stream_format(stream, "%05oB", addr & 077777);

	// Indirect addressing?
	if (BIT(addr, 15))
		stream << ",I";
}

void hp2100_disassembler::format_sc(std::ostream &stream, u8 sc) const
{
	util::stream_format(stream, "%o", sc);
	if (sc > 7)
		stream << 'B';
}

// Memory reference group
offs_t hp2100_disassembler::dasm_mrg(std::ostream &stream, u16 inst, offs_t pc) const
{
	offs_t flags = SUPPORTED;
	switch (BIT(inst, 12, 3))
	{
	case 1:
		if (BIT(inst, 11))
		{
			stream << "JSB ";
			flags |= STEP_OVER;
		}
		else
			stream << "AND ";
		break;

	case 2:
		if (BIT(inst, 11))
		{
			stream << "JMP ";
			if (BIT(inst, 15))
				flags |= STEP_OUT;
		}
		else
			stream << "XOR ";
		break;

	case 3:
		if (BIT(inst, 11))
		{
			stream << "ISZ ";
			flags |= STEP_COND;
		}
		else
			stream << "IOR ";
		break;

	case 4:
		util::stream_format(stream, "AD%c ", cab(inst));
		break;

	case 5:
		util::stream_format(stream, "CP%c ", cab(inst));
		flags |= STEP_COND;
		break;

	case 6:
		util::stream_format(stream, "LD%c ", cab(inst));
		break;

	case 7:
		util::stream_format(stream, "ST%c ", cab(inst));
		break;
	}

	// Page zero or current page
	format_address(stream, (BIT(inst, 10) ? (pc & 077600) : 0) | (inst & 0100177));

	return 1 | flags;
}

const char *const hp2100_disassembler::s_shift_ops[2][8] =
{
	{ "ALS", "ARS", "RAR", "RAL", "ALR", "ERA", "ELA", "ALF" },
	{ "BLS", "BRS", "RBR", "RBL", "BLR", "ERB", "ELB", "BLF" }
};

// Shift-rotate group
offs_t hp2100_disassembler::dasm_srg(std::ostream &stream, u16 inst) const
{
	offs_t flags = SUPPORTED;

	if ((inst & 001070) == 0)
	{
		if (inst == 0)
			stream << "NOP";
		else
			util::stream_format(stream, "OCT %o", inst);
	}
	else
	{
		if (BIT(inst, 9))
			stream << s_shift_ops[BIT(inst, 11)][BIT(inst, 6, 3)];

		if (BIT(inst, 5))
		{
			if (BIT(inst, 9))
				stream << ',';
			stream << "CLE";
		}

		// Skip on LSB
		if (BIT(inst, 3))
		{
			if ((inst & 001040) != 0)
				stream << ',';
			util::stream_format(stream, "SL%c", cab(inst));
			flags |= STEP_COND;
		}

		if (BIT(inst, 4))
		{
			if ((inst & 001050) != 0)
				stream << ',';
			stream << s_shift_ops[BIT(inst, 11)][BIT(inst, 0, 3)];
		}
	}
	
	return 1 | flags;
}

// Alter-skip group
offs_t hp2100_disassembler::dasm_asg(std::ostream &stream, u16 inst) const
{
	offs_t flags = SUPPORTED;

	if ((inst & 01777) == 0)
		util::stream_format(stream, "OCT %o", inst);
	else
	{
		if (BIT(inst, 8, 2) != 0)
			util::stream_format(stream, "C%c%c", "LMC"[BIT(inst, 8, 2) - 1], cab(inst));

		// Extend skip
		if (BIT(inst, 5))
		{
			if (BIT(inst, 8, 2) != 0)
				stream << ',';
			stream << "SEZ";
			flags |= STEP_COND;
		}

		if (BIT(inst, 6, 2) != 0)
		{
			if ((inst & 01440) != 0)
				stream << ',';
			util::stream_format(stream, "C%cE", "LMC"[BIT(inst, 6, 2) - 1]);
		}

		// Accumulator skips
		if (BIT(inst, 4))
		{
			if ((inst & 01740) != 0)
				stream << ',';
			util::stream_format(stream, "SS%c", cab(inst));
			flags |= STEP_COND;
		}
		if (BIT(inst, 3))
		{
			if ((inst & 01760) != 0)
				stream << ',';
			util::stream_format(stream, "SL%c", cab(inst));
			flags |= STEP_COND;
		}
		if (BIT(inst, 2))
		{
			if ((inst & 01770) != 0)
				stream << ',';
			util::stream_format(stream, "IN%c", cab(inst));
			flags |= STEP_COND;
		}
		if (BIT(inst, 1))
		{
			if ((inst & 01774) != 0)
				stream << ',';
			util::stream_format(stream, "SZ%c", cab(inst));
			flags |= STEP_COND;
		}

		// Reverse skip sense
		if (BIT(inst, 0))
		{
			if ((inst & 01776) != 0)
				stream << ',';
			stream << "RSS";
		}
	}

	return 1 | flags;
}

// Input-output group
offs_t hp2100_disassembler::dasm_iog(std::ostream &stream, u16 inst) const
{
	u8 sc = BIT(inst, 0, 6);
	offs_t flags = SUPPORTED;

	switch (BIT(inst, 6, 3))
	{
	case 0:
		stream << "HLT";
		if (sc != 0)
		{
			stream << ' ';
			format_sc(stream, sc);
		}
		break;

	case 1:
		if (sc == 1)
		{
			// Clear or set overflow flag
			if (BIT(inst, 9))
				stream << "CLO";
			else
				stream << "STO";
		}
		else
		{
			if (BIT(inst, 9))
				stream << "CLF ";
			else
				stream << "STF ";
			format_sc(stream, sc);
		}
		break;

	case 2:
		if (sc == 1)
			stream << "SOC";
		else
		{
			stream << "SFC ";
			format_sc(stream, sc);
		}
		flags |= STEP_COND;
		break;

	case 3:
		if (sc == 1)
			stream << "SOS";
		else
		{
			stream << "SFS ";
			format_sc(stream, sc);
		}
		flags |= STEP_COND;
		break;

	case 4:
		util::stream_format(stream, "MI%c ", cab(inst));
		format_sc(stream, sc);
		break;

	case 5:
		util::stream_format(stream, "LI%c ", cab(inst));
		format_sc(stream, sc);
		break;

	case 6:
		util::stream_format(stream, "OT%c ", cab(inst));
		format_sc(stream, sc);
		break;

	case 7:
		if (BIT(inst, 11))
			stream << "CLC ";
		else
			stream << "STC ";
		format_sc(stream, sc);
		break;
	}

	// Clear flag
	if (BIT(inst, 9) && BIT(inst, 6, 3) != 1)
		stream << ",C";

	return 1 | flags;
}

// Macro instructions (including extended arithmetic group and 12901A floating-point instructions)
offs_t hp2100_disassembler::dasm_mac(std::ostream &stream, u16 inst, offs_t pc, const hp2100_disassembler::data_buffer &opcodes) const
{
	switch (inst & 05760)
	{
	case 00020: case 01020:
	{
		u8 n = BIT(inst, 0, 4);
		util::stream_format(stream, "AS%c %d", BIT(inst, 9) ? 'R' : 'L', n == 0 ? 16 : n);
		return 1 | SUPPORTED;
	}

	case 00040: case 01040:
	{
		u8 n = BIT(inst, 0, 4);
		util::stream_format(stream, "LS%c %d", BIT(inst, 9) ? 'R' : 'L', n == 0 ? 16 : n);
		return 1 | SUPPORTED;
	}

	case 00100: case 01100:
	{
		u8 n = BIT(inst, 0, 4);
		util::stream_format(stream, "RR%c %d", BIT(inst, 9) ? 'R' : 'L', n == 0 ? 16 : n);
		return 1 | SUPPORTED;
	}

	case 00200:
		stream << "MPY ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 00400:
		stream << "DIV ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 04000:
		stream << "DLD ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 04400:
		stream << "DST ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 05000:
		stream << "FAD ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 05020:
		stream << "FSB ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 05040:
		stream << "FML ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 05060:
		stream << "FDV ";
		format_address(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;

	case 05100:
		stream << "FIX";
		return 1 | SUPPORTED;

	case 05120:
		stream << "FLT";
		return 1 | SUPPORTED;

	default:
		util::stream_format(stream, "MAC %oB", inst & 05777);
		return 1 | SUPPORTED;
	}
}

offs_t hp21mx_disassembler::dasm_mac(std::ostream &stream, u16 inst, offs_t pc, const hp2100_disassembler::data_buffer &opcodes) const
{
	if ((inst & 01740) == 01740)
	{
		// Extended instruction group
		switch (inst & 04027)
		{
		case 0000: case 04000:
			util::stream_format(stream, "S%c%c ", cab(inst), xy(inst));
			format_address(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;

		case 0001: case 04001:
			util::stream_format(stream, "C%c%c", cab(inst), xy(inst));
			return 1 | SUPPORTED;

		case 0002: case 04002:
			util::stream_format(stream, "L%c%c ", cab(inst), xy(inst));
			format_address(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;

		case 04003:
			util::stream_format(stream, "ST%c ", xy(inst));
			format_address(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;

		case 0004: case 04004:
			util::stream_format(stream, "C%c%c", xy(inst), cab(inst));
			return 1 | SUPPORTED;

		case 04005:
			util::stream_format(stream, "LD%c ", xy(inst));
			format_address(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;

		case 04006:
			util::stream_format(stream, "AD%c ", xy(inst));
			format_address(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;

		case 0007: case 04007:
			util::stream_format(stream, "X%c%c", cab(inst), xy(inst));
			return 1 | SUPPORTED;

		case 04020:
			util::stream_format(stream, "IS%c", xy(inst));
			return 1 | STEP_COND | SUPPORTED;

		case 04021:
			util::stream_format(stream, "DS%c", xy(inst));
			return 1 | STEP_COND | SUPPORTED;

		case 04022:
			if (BIT(inst, 3))
			{
				stream << "JPY ";
				format_address(stream, opcodes.r16(pc + 1) & 077777);
			}
			else
			{
				stream << "JLY ";
				format_address(stream, opcodes.r16(pc + 1));
			}
			return 2 | SUPPORTED;

		case 04023:
			if (BIT(inst, 3))
			{
				stream << "SBS ";
				format_address(stream, opcodes.r16(pc + 1));
				stream << ',';
				format_address(stream, opcodes.r16(pc + 2));
				return 3 | SUPPORTED;
			}
			else
			{
				stream << "LBT";
				return 1 | SUPPORTED;
			}

		case 04024:
			if (BIT(inst, 3))
			{
				stream << "CBS ";
				format_address(stream, opcodes.r16(pc + 1));
				stream << ',';
				format_address(stream, opcodes.r16(pc + 2));
				return 3 | STEP_COND | SUPPORTED;
			}
			else
			{
				stream << "SBT";
				return 1 | SUPPORTED;
			}

		case 04025:
			if (BIT(inst, 3))
			{
				stream << "TBS ";
				format_address(stream, opcodes.r16(pc + 1));
				stream << ',';
				format_address(stream, opcodes.r16(pc + 2));
				return 3 | STEP_COND | SUPPORTED;
			}
			else
			{
				stream << "MBT ";
				format_address(stream, opcodes.r16(pc + 1));
				return 3 | SUPPORTED; // one extra word reserved for microcode use
			}

		case 04026:
			if (BIT(inst, 3))
				stream << "CMW ";
			else
				stream << "CBT ";
			format_address(stream, opcodes.r16(pc + 1));
			return 3 | STEP_COND | SUPPORTED; // one extra word reserved for microcode use

		case 04027:
			if (BIT(inst, 3))
			{
				stream << "MVW ";
				format_address(stream, opcodes.r16(pc + 1));
				return 3 | SUPPORTED; // one extra word reserved for microcode use
			}
			else
			{
				stream << "SBT";
				return 1 | STEP_COND | SUPPORTED;
			}

		default:
			util::stream_format(stream, "MAC %oB", inst & 05777);
			return 1 | SUPPORTED;
		}
	}
	else
		return hp2100_disassembler::dasm_mac(stream, inst, pc, opcodes);
}

offs_t hp2100_disassembler::disassemble(std::ostream &stream, offs_t pc, const hp2100_disassembler::data_buffer &opcodes, const hp2100_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	if (BIT(inst, 12, 3) != 0)
		return dasm_mrg(stream, inst, pc);
	else if (BIT(inst, 15))
	{
		if (BIT(inst, 10))
			return dasm_iog(stream, inst);
		else
			return dasm_mac(stream, inst, pc, opcodes);
	}
	else
	{
		if (BIT(inst, 10))
			return dasm_asg(stream, inst);
		else
			return dasm_srg(stream, inst);
	}
}
