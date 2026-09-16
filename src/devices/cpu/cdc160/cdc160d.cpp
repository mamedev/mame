// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    CDC 160/160-A disassembler

    Some departures from the syntax used by CDC's assemblers:
    — Two-word instructions are always written on one line each. This
      was not necessarily permitted, especially for INP and OUT.
    — For forward and backward addressing modes, the effective address is
      given rather than the 6-bit displacement to be added to P.
    — For 160-A only, a synthetic two-word instruction JPM (Jump Memory)
      represents the common pattern of using JFI with a 1-word displacement
      to jump to any address in the 4096-word bank.

***************************************************************************/

#include "emu.h"
#include "cdc160d.h"

cdc160_disassembler::cdc160_disassembler()
	: util::disasm_interface()
{
}

u32 cdc160_disassembler::opcode_alignment() const
{
	return 1;
}

cdc160a_disassembler::cdc160a_disassembler()
	: cdc160_disassembler()
{
}

u32 cdc160a_disassembler::interface_flags() const
{
	return PAGED;
}

u32 cdc160a_disassembler::page_address_bits() const
{
	return 12;
}

namespace {

const char *const f_160a_bank_ops[7] =
{
	"SRJ", "SIC", "IRJ", "SDC", "DRJ", "SID", "ACJ"
};

const char *const f_160_alu_ops[10] =
{
	"LP", "LS", "LD", "LC", "AD", "SB", "ST", "SR", "RA", "AO"
};

const char *const f_160a_alu_ops[10] =
{
	"LP", "SC", "LD", "LC", "AD", "SB", "ST", "SR", "RA", "AO"
};

const char *const f_cond_jp_ops[6] =
{
	"ZJ", "NZ", "PJ", "NJ"
};

// 12-bit ones' complement addition
inline u16 add_relative(uint_fast16_t a, uint_fast16_t b) noexcept
{
	u16 c = (a & 07777) + (b & 07777);
	if (c >= 07777)
		c -= 07777;
	return c;
}

} // anonymous namespace

offs_t cdc160_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 inst = opcodes.r16(pc) & 07777;
	if (inst < 0200)
	{
		util::stream_format(stream, "%s %o", BIT(inst, 6) ? "SHA" : "ERR", inst & 0077);
		return 1 | SUPPORTED;
	}
	else if (inst < 01000)
	{
		// No address (6-bit immediate) mode
		util::stream_format(stream, "%sN %o", f_160_alu_ops[BIT(inst, 6, 3) - 2], inst & 0077);
		return 1 | SUPPORTED;
	}
	else if (inst < 06000)
	{
		if (BIT(inst, 7))
		{
			// Relative forward or backward address mode
			util::stream_format(stream, "%s%c %04o", f_160_alu_ops[BIT(inst, 8, 4) - 2], BIT(inst, 6) ? 'B' : 'F',
				add_relative(pc, BIT(inst, 6) ? ~(inst & 0077) : inst & 0077));
			return 1 | SUPPORTED;
		}
		else
		{
			// Direct or indirect address mode
			util::stream_format(stream, "%s%c %02o", f_160_alu_ops[BIT(inst, 8, 4) - 2], BIT(inst, 6) ? 'I' : 'D', inst & 0077);
			return 1 | SUPPORTED;
		}
	}
	else if (inst < 07000)
	{
		// Relative conditional jumps
		util::stream_format(stream, "%s%c %04o", f_cond_jp_ops[BIT(inst, 6, 2)], BIT(inst, 8) ? 'B' : 'F',
			add_relative(pc, BIT(inst, 8) ? ~(inst & 0077) : inst & 0077));
		return 1 | STEP_COND | SUPPORTED;
	}
	else switch (BIT(inst, 6, 3))
	{
	case 0:
		util::stream_format(stream, "JPI %02o", inst & 0077);
		return 1 | STEP_OUT | SUPPORTED;

	case 1:
		// Jump forward indirect
		util::stream_format(stream, "JFI %04o", add_relative(pc, inst & 0077));
		return 1 | SUPPORTED;

	case 2: case 3:
		util::stream_format(stream, "%s %04o %04o", BIT(inst, 6) ? "OUT" : "INP", add_relative(pc, inst & 0077), opcodes.r16(pc + 1) & 07777);
		return 2 | SUPPORTED;

	case 4:
		// Output direct
		util::stream_format(stream, "OTN %02o", inst & 0077);
		return 1 | SUPPORTED;

	case 5:
		// External function
		util::stream_format(stream, "EXF %04o", (pc + (inst & 0077)) & 07777);
		return 1 | SUPPORTED;

	case 6:
		util::stream_format(stream, "INA %o", inst & 0077);
		return 1 | SUPPORTED;

	case 7: default:
		util::stream_format(stream, "HLT %02o", inst & 0077);
		return 1 | SUPPORTED;
	}
}

offs_t cdc160a_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 inst = opcodes.r16(pc) & 07777;
	if (inst < 0200)
	{
		if (inst >= 0010 && inst < 0100)
		{
			util::stream_format(stream, "%s %d", f_160a_bank_ops[BIT(inst, 3, 3) - 1], inst & 0007);
			return 1 | SUPPORTED;
		}
		else switch (inst)
		{
		case 0000:
			stream << "ERR";
			return 1 | SUPPORTED;

		case 0001:
			stream << "NOP";
			return 1 | SUPPORTED;

		case 0100:
			util::stream_format(stream, "BLS %04o", opcodes.r16(pc + 1) & 07777);
			return 2 | STEP_COND | SUPPORTED;

		case 0101:
			stream << "PTA";
			return 1 | (BIT(opcodes.r16(pc + 1), 6, 3) == 070 ? STEP_OVER | step_over_extra(1) : 0) | SUPPORTED;

		case 0102:
			stream << "LS1";
			return 1 | SUPPORTED;

		case 0103:
			stream << "LS2";
			return 1 | SUPPORTED;

		case 0104:
			stream << "CBC";
			return 1 | SUPPORTED;

		case 0105:
			util::stream_format(stream, "ATE %04o", opcodes.r16(pc + 1) & 07777);
			return 2 | STEP_COND | SUPPORTED;

		case 0106:
			util::stream_format(stream, "ATX %04o", opcodes.r16(pc + 1) & 07777);
			return 2 | STEP_COND | SUPPORTED;

		case 0107:
			stream << "ETA";
			return 1 | SUPPORTED;

		case 0110:
			stream << "LS3";
			return 1 | SUPPORTED;

		case 0111:
			stream << "LS6";
			return 1 | SUPPORTED;

		case 0112:
			stream << "MUT";
			return 1 | SUPPORTED;

		case 0113:
			stream << "MUH";
			return 1 | SUPPORTED;

		case 0114:
			stream << "RS1";
			return 1 | SUPPORTED;

		case 0115:
			stream << "RS2";
			return 1 | SUPPORTED;

		case 0120:
			stream << "CIL";
			return 1 | SUPPORTED;

		case 0130:
			stream << "CTA";
			return 1 | SUPPORTED;

		case 0140: case 0141: case 0142: case 0143: case 0144: case 0145: case 0146: case 0147:
			util::stream_format(stream, "SBU %d", inst & 0007);
			return 1 | SUPPORTED;

		case 0150: case 0151: case 0152: case 0153: case 0154: case 0155: case 0156: case 0157:
			util::stream_format(stream, "STP %02o", inst & 0077);
			return 1 | SUPPORTED;

		case 0160: case 0161: case 0162: case 0163: case 0164: case 0165: case 0166: case 0167:
			util::stream_format(stream, "STE %02o", inst & 0077);
			return 1 | SUPPORTED;

		default:
			util::stream_format(stream, "%04o", inst);
			return 1 | SUPPORTED;
		}
	}
	else if (inst < 01000)
	{
		// No address (6-bit immediate) mode
		util::stream_format(stream, "%sN %o", f_160a_alu_ops[BIT(inst, 6, 3) - 2], inst & 0077);
		return 1 | SUPPORTED;
	}
	else if (inst < 06000)
	{
		if (BIT(inst, 7))
		{
			if ((inst & 0077) != 0)
			{
				// Relative forward or backward address mode
				util::stream_format(stream, "%s%c %04o", f_160a_alu_ops[BIT(inst, 8, 4) - 2], BIT(inst, 6) ? 'B' : 'F',
					add_relative(pc, BIT(inst, 6) ? ~(inst & 0077) : inst & 0077));
				return 1 | SUPPORTED;
			}
			else if (BIT(inst, 6))
			{
				// Specific address mode
				util::stream_format(stream, "%sS", f_160a_alu_ops[BIT(inst, 8, 4) - 2]);
				return 1 | SUPPORTED;
			}
			else
			{
				// Constant address (12-bit immediate) mode
				util::stream_format(stream, "%sC %04o", f_160a_alu_ops[BIT(inst, 8, 4) - 2], opcodes.r16(pc + 1) & 07777);
				return 2 | SUPPORTED;
			}
		}
		else if ((inst & 0177) == 0100)
		{
			// Memory address mode
			util::stream_format(stream, "%sM %04o", f_160a_alu_ops[BIT(inst, 8, 4) - 2], opcodes.r16(pc + 1) & 07777);
			return 2 | SUPPORTED;
		}
		else
		{
			// Direct or indirect address mode
			util::stream_format(stream, "%s%c %02o", f_160a_alu_ops[BIT(inst, 8, 4) - 2], BIT(inst, 6) ? 'I' : 'D', inst & 0077);
			return 1 | SUPPORTED;
		}
	}
	else if (inst < 07000)
	{
		// Relative conditional jumps
		util::stream_format(stream, "%s%c %04o", f_cond_jp_ops[BIT(inst, 6, 2)], BIT(inst, 8) ? 'B' : 'F',
			add_relative(pc, BIT(inst, 8) ? ~(inst & 0077) : inst & 0077));
		return 1 | STEP_COND | SUPPORTED;
	}
	else switch (BIT(inst, 6, 3))
	{
	case 0:
		util::stream_format(stream, "JPI %02o", inst & 0077);
		return 1 | STEP_OUT | SUPPORTED;

	case 1:
		if ((inst & 0077) <= 1)
		{
			util::stream_format(stream, "JP%c %04o", BIT(inst, 0) ? 'M' : 'R', opcodes.r16(pc + 1) & 07777);
			return 2 | (BIT(inst, 0) ? 0 : STEP_OVER) | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "JFI %04o", add_relative(pc, inst & 0077));
			return 1 | SUPPORTED;
		}

	case 2: case 3:
		if ((inst & 0077) == 0)
		{
			// Buffer input/output
			util::stream_format(stream, "IB%c %04o", BIT(inst, 6) ? 'O' : 'I', opcodes.r16(pc + 1) & 07777);
			return 2 | STEP_COND | SUPPORTED;
		}
		else
		{
			// Normal input/output
			util::stream_format(stream, "%s %04o %04o", BIT(inst, 6) ? "OUT" : "INP", add_relative(pc, inst & 0077), opcodes.r16(pc + 1) & 07777);
			return 2 | SUPPORTED;
		}
		return 1 | SUPPORTED;

	case 4:
		// Output direct
		util::stream_format(stream, "OTN %02o", inst & 0077);
		return 1 | SUPPORTED;

	case 5:
		// External function
		if ((inst & 0077) == 0)
		{
			util::stream_format(stream, "EXC %04o", opcodes.r16(pc + 1) & 07777);
			return 2 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "EXF %04o", (pc + (inst & 0077)) & 07777);
			return 1 | SUPPORTED;
		}

	case 6:
		if ((inst & 0077) == 0)
			stream << "INA";
		else if ((inst & 0077) == 0077)
			stream << "OTA";
		else
			util::stream_format(stream, "HWI %02o", inst & 0077);
		return 1 | SUPPORTED;

	case 7: default:
		if ((inst & 0077) == 0 || (inst & 0077) == 0077)
		{
			stream << "HLT";
			return 1 | SUPPORTED;
		}
		else if ((inst & 070) == 0)
		{
			util::stream_format(stream, "SLS %d", inst & 0007);
			return 1 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%s %02o %04o", (inst & 0007) != 0 ? "SJS" : "SLJ", inst & 077, opcodes.r16(pc + 1) & 07777);
			return 2 | STEP_COND | SUPPORTED;
		}
	}
}
