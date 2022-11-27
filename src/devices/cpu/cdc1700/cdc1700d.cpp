// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    CDC 1700 & Cyber 18 disassembler

***************************************************************************/

#include "emu.h"
#include "cdc1700d.h"

cdc1700_disassembler::cdc1700_disassembler()
	: util::disasm_interface()
{
}

cyber18_disassembler::cyber18_disassembler()
	: cdc1700_disassembler()
{
}

u32 cdc1700_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

const char *const s_skip_insts[16] =
{
	"SAZ", "SAN",
	"SAP", "SAM",
	"SQZ", "SQN",
	"SQP", "SQM",
	"SWS", "SWN",
	"SOV", "SNO",
	"SPE", "SNP",
	"SPF", "SNF"
};

const char *const s_destregs[8] =
{
	"0",
	"M",
	"Q",
	"Q,M",
	"A",
	"A,M",
	"A,Q",
	"A,Q,M"
};

const char *const s_register_insts[15] =
{
	"SLS", "01X", "INP", "OUT",
	"EIN", "IIN", "SPB", "CPB",
	"08X", "INA", "ENA", "NOP",
	"ENQ", "INQ", "EXI"
};

const char *const s_memory_insts[15] =
{
		   "JMP", "MUI", "DVI",
	"STQ", "RTJ", "STA", "SPA",
	"ADD", "SUB", "AND", "EOR",
	"LDA", "RAO", "LDQ", "ADQ"
};

const char *const s_field_insts[6] =
{
	"SFZ", "SFN",
	"LFA", "SFA",
	"CLF", "SEF"
};

const char *const s_misc_micro_insts[13] =
{
	"LMM",
	"LRG",
	"SRG",
	"SIO",
	"SPS",
	"DMI",
	"CPB",
	"GPE",
	"GPO",
	"ASC",
	"APM",
	"PM0",
	"PM1"
};

const char *const s_misc_page_insts[6] =
{
	"LLB",
	"LUB",
	"EMS",
	"WPR",
	"RPR",
	"ECC"
};

const char s_reg_names[8] =
{
	'0',
	'1',
	'2',
	'3',
	'4',
	'Q',
	'A',
	'I'
};

const char *const s_reg_long_names[8] =
{
	"",
	"R1",
	"R2",
	"R3",
	"R4",
	"Q",
	"A",
	"I"
};

} // anonymous namespace

void cdc1700_disassembler::format_s8(std::ostream &stream, u8 value)
{
	if (BIT(value, 7))
	{
		stream << '-';
		value ^= 0xff;
	}
	if (value > 9)
		stream << '$';
	util::stream_format(stream, "%X", value);
}

offs_t cdc1700_disassembler::dasm_register_reference(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst)
{
	util::stream_format(stream, "%-5s", s_register_insts[BIT(inst, 8, 4)]);
	u8 modifier = inst & 0x00ff;
	if (inst >= 0x0900 && inst < 0x0e00)
		format_s8(stream, modifier);
	else if (modifier <= 9)
		util::stream_format(stream, "%d", modifier);
	else
		util::stream_format(stream, "$%02X", modifier);
	return 1 | SUPPORTED;
}

offs_t cyber18_disassembler::dasm_register_reference(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst)
{
	if (inst < 0x0100 && inst != 0x0000)
	{
		// Type 2 skip instructions
		util::stream_format(stream, "S%d%-3c%d", BIT(inst, 6, 2) == 0 ? 4 : BIT(inst, 6, 2), "ZNPM"[BIT(inst, 4, 2)], inst & 0x000f);
		return 1 | STEP_COND | SUPPORTED;
	}
	else if (BIT(inst, 9, 3) == 2 && (inst & 0x00ff) != 0)
	{
		// Type 2 storage reference instructions & field reference instructions
		u16 inst2 = opcodes.r16(pc + 1);
		u32 flags = 0;
		if (BIT(inst, 8))
		{
			if (BIT(inst, 1, 2) != 0)
			{
				stream << s_field_insts[BIT(inst, 0, 3) - 2];
				flags = (BIT(inst, 2) ? 0 : STEP_COND) | SUPPORTED;
			}
		}
		else switch (BIT(inst2, 8, 8))
		{
		case 0x50:
			if (BIT(inst, 0, 3) == 0)
			{
				stream << "SJE";
				flags = STEP_OUT | SUPPORTED;
			}
			else
			{
				util::stream_format(stream, "SJ%c", s_reg_names[BIT(inst, 0, 3)]);
				flags = STEP_OVER | SUPPORTED;
			}
			break;

		case 0x80:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "AR%c", s_reg_names[BIT(inst, 0, 3)]);
				flags = SUPPORTED;
			}
			break;

		case 0x90:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "SB%c", s_reg_names[BIT(inst, 0, 3)]);
				flags = SUPPORTED;
			}
			break;

		case 0xa0: case 0xa1:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "A%c%c", BIT(inst2, 8) ? 'M' : 'N', s_reg_names[BIT(inst, 0, 3)]);
				flags = SUPPORTED;
			}
			break;

		case 0xc0: case 0xc1:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "%cR%c", BIT(inst2, 8) ? 'S' : 'L', s_reg_names[BIT(inst, 0, 3)]);
				flags = SUPPORTED;
			}
			break;

		case 0xc2: case 0xc3:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "%cCA", BIT(inst2, 8) ? 'S' : 'L');
				flags = SUPPORTED;
			}
			break;

		case 0xd0: case 0xd1:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "O%c%c", BIT(inst2, 8) ? 'M' : 'R', s_reg_names[BIT(inst, 0, 3)]);
				flags = SUPPORTED;
			}
			break;

		case 0xe0:
			if (BIT(inst, 0, 3) != 0)
			{
				util::stream_format(stream, "C%cE", s_reg_names[BIT(inst, 0, 3)]);
				flags = SUPPORTED | STEP_COND;
			}
			break;

		case 0xe2:
			if (BIT(inst, 0, 3) != 0)
			{
				stream << "CCE";
				flags = SUPPORTED | STEP_COND;
			}
			break;
		}

		if (flags != 0)
		{
			u8 delta = inst2 & 0x00ff;
			if (delta == 0)
			{
				u16 addr = opcodes.r16(pc + 2);
				util::stream_format(stream, "%c ", BIT(inst, 6, 2) == 1 ? '+' : ' ');
				if (BIT(inst, 7))
				{
					// Two-word relative addressing
					if (BIT(inst, 6))
						stream << '(';
					util::stream_format(stream, "$%04X", pc + 2 + addr - (BIT(addr, 15) ? 0xffff : 0));
					if (BIT(inst, 6))
						stream << ')';
				}
				else if (BIT(inst, 6))
				{
					// Two-word absolute addressing (32K mode assumed; 65K mode does not use bit 15 for indirection)
					if (BIT(addr, 15))
						stream << '(';
					util::stream_format(stream, "$%04X", addr & 0x7fff);
					if (BIT(addr, 15))
						stream << ')';
				}
				else
				{
					// Two-word constant addressing
					stream << "=N";
					if (BIT(addr, 15) && !BIT(inst, 8) && (inst2 & 0xef00) == 0x8000)
					{
						stream << '-';
						addr ^= 0xffff;
					}
					util::stream_format(stream, "$%04X", addr);
				}
			}
			else
			{
				// One-word addressing
				util::stream_format(stream, "%c ", BIT(inst, 7) ? '*' : '-');
				if (BIT(inst, 6))
					stream << '(';
				if (BIT(inst, 7))
					util::stream_format(stream, "$%04X", pc + 1 + delta - (BIT(delta, 7) ? 0xff : 0));
				else if (BIT(inst, 3, 3) != 0 && !BIT(inst, 6))
					format_s8(stream, delta);
				else if (delta == 0xff && !BIT(inst, 8))
					stream << 'I';
				else
					util::stream_format(stream, "$%04X", delta);
				if (BIT(inst, 6))
					stream << ')';
			}

			// For field reference instructions, FLDBUF and FLDLEN are specified after address but before indexing
			if (BIT(inst, 8))
				util::stream_format(stream, ",%d,%d", BIT(inst2, 12, 4), BIT(inst2, 8, 4) + 1);

			// For LCA, SCA and CCE, first index is a character index
			if (!BIT(inst, 8) && (inst2 & 0xde00) == 0xc200)
				util::stream_format(stream, ",%s", s_reg_long_names[BIT(inst, 0, 3)]);

			// Ra indexing is specified last
			if (BIT(inst, 3, 3) != 0)
				util::stream_format(stream, ",%s", s_reg_long_names[BIT(inst, 3, 3)]);

			return (delta == 0 ? 3 : 2) | flags;
		}
		else
		{
			util::stream_format(stream, "%-5s$%04X", "NUM", inst);
			return 1 | SUPPORTED;
		}
	}
	else if (BIT(inst, 8, 4) == 6 && BIT(inst, 5, 3) != 0)
	{
		// Decrement and repeat
		util::stream_format(stream, "DR%-3c%d", s_reg_names[BIT(inst, 5, 3)], inst & 0x000f);
		return 1 | STEP_COND | SUPPORTED;
	}
	else if (BIT(inst, 8, 4) == 7 && BIT(inst, 5, 3) != 0 && BIT(inst, 0, 3) != 0)
	{
		// Type 2 inter-register transfer (bits 5-7 specify origin register, bits 0-2 specify destination register)
		util::stream_format(stream, "XF%-3c%s", s_reg_names[BIT(inst, 5, 3)], s_reg_long_names[BIT(inst, 0, 3)]);
		return 1 | SUPPORTED;
	}
	else if (inst > 0x0b00 && inst < 0x0c00)
	{
		// Miscellaneous enhanced instructions
		if (BIT(inst, 5, 3) != 0 && BIT(inst, 0, 3) < 6)
			util::stream_format(stream, "%-5s%s", s_misc_page_insts[BIT(inst, 0, 3)], s_reg_long_names[BIT(inst, 5, 3)]);
		else if (inst <= 0x0b0d)
			util::stream_format(stream, "%-5s0", s_misc_micro_insts[BIT(inst, 0, 4) - 1]);
		else
			util::stream_format(stream, "%-5s$%04X", "NUM", inst);
		return 1 | SUPPORTED;
	}
	else
		return cdc1700_disassembler::dasm_register_reference(stream, pc, opcodes, inst);
}

offs_t cdc1700_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	if (inst >= 0x1000)
	{
		stream << s_memory_insts[BIT(inst, 12, 4) - 1];
		u8 delta = inst & 0x00ff;
		if (delta == 0)
		{
			u16 addr = opcodes.r16(pc + 1);
			util::stream_format(stream, "%c ", BIT(inst, 10, 2) == 1 ? '+' : ' ');
			if (BIT(inst, 11))
			{
				// Two-word relative addressing
				if (BIT(inst, 10))
					stream << '(';
				util::stream_format(stream, "$%04X", pc + 1 + addr - (BIT(addr, 15) ? 0xffff : 0));
				if (BIT(inst, 10))
					stream << ')';
			}
			else if (BIT(inst, 10))
			{
				// Two-word absolute addressing (32K mode assumed; 65K mode does not use bit 15 for indirection)
				if (BIT(addr, 15))
					stream << '(';
				util::stream_format(stream, "$%04X", addr & 0x7fff);
				if (BIT(addr, 15))
					stream << ')';
			}
			else
			{
				// Two-word constant addressing
				stream << "=N";
				if (BIT(addr, 15) && (BIT(inst, 13, 3) == 1 || BIT(inst, 13, 3) == 4 || inst >= 0xf000))
				{
					stream << '-';
					addr ^= 0xffff;
				}
				util::stream_format(stream, "$%04X", addr);
			}
		}
		else
		{
			// One-word addressing
			util::stream_format(stream, "%c ", BIT(inst, 11) ? '*' : '-');
			if (BIT(inst, 10))
				stream << '(';
			if (BIT(inst, 11))
				util::stream_format(stream, "$%04X", pc + delta - (BIT(delta, 7) ? 0xff : 0));
			else if (BIT(inst, 8, 2) != 0 && !BIT(inst, 10))
				format_s8(stream, delta);
			else if (delta == 0xff)
				stream << 'I';
			else
				util::stream_format(stream, "$%04X", delta);
			if (BIT(inst, 10))
				stream << ')';
		}
		if (BIT(inst, 8, 2) != 0)
			util::stream_format(stream, ",%c", "IQB"[BIT(inst, 8, 2) - 1]);
		return (delta == 0 ? 2 : 1) | (BIT(inst, 12, 4) == 5 ? STEP_OVER : BIT(inst, 8, 8) == 0x1c ? STEP_OUT : 0) | SUPPORTED;
	}
	else switch (BIT(inst, 8, 4))
	{
	case 1:
		// Skip instructions
		util::stream_format(stream, "%-5s%d", s_skip_insts[BIT(inst, 4, 4)], inst & 0x000f);
		return 1 | STEP_COND | SUPPORTED;

	case 8:
		// Interregister operations
		switch (inst & 0x00f8)
		{
		case 0x08:
			util::stream_format(stream, "%-5s%s", "TRM", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x10:
			util::stream_format(stream, "%-5s%s", "TRQ", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x18:
			util::stream_format(stream, "%-5s%s", "TRB", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x20:
			util::stream_format(stream, "%-5s%s", "TRA", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x28:
			util::stream_format(stream, "%-5s%s", "AAM", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x30:
			util::stream_format(stream, "%-5s%s", "AAQ", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x38:
			util::stream_format(stream, "%-5s%s", "AAB", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x40:
			util::stream_format(stream, "%-5s%s", "CLR", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x48:
			util::stream_format(stream, "%-5s%s", "TCM", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x50:
			util::stream_format(stream, "%-5s%s", "TCQ", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x58:
			util::stream_format(stream, "%-5s%s", "TCB", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x60:
			util::stream_format(stream, "%-5s%s", "TCA", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x68:
			util::stream_format(stream, "%-5s%s", "EAM", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x70:
			util::stream_format(stream, "%-5s%s", "EAQ", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x78:
			util::stream_format(stream, "%-5s%s", "EAB", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0x80:
			util::stream_format(stream, "%-5s%s", "SET", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0xa8:
			util::stream_format(stream, "%-5s%s", "LAM", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0xb0:
			util::stream_format(stream, "%-5s%s", "LAQ", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0xb8:
			util::stream_format(stream, "%-5s%s", "LAB", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0xe8:
			util::stream_format(stream, "%-5s%s", "CAM", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0xf0:
			util::stream_format(stream, "%-5s%s", "CAQ", s_destregs[BIT(inst, 0, 3)]);
			break;

		case 0xf8:
			util::stream_format(stream, "%-5s%s", "CAB", s_destregs[BIT(inst, 0, 3)]);
			break;

		default:
			util::stream_format(stream, "%-5s$%04X", "NUM", inst);
			break;
		}
		return 1 | SUPPORTED;

	case 0xf:
		// Shift instructions
		if (BIT(inst, 5, 2) != 0)
			util::stream_format(stream, "%c%c%-3c%d", "QAL"[BIT(inst, 5, 2) - 1], BIT(inst, 7) ? 'L' : 'R', 'S', inst & 0x001f);
		else
			util::stream_format(stream, "%-5s$%04X", "NUM", inst);
		return 1 | SUPPORTED;

	default:
		return dasm_register_reference(stream, pc, opcodes, inst);
	}
}
