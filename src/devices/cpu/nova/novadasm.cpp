// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Data General Nova disassembler

***************************************************************************/

#include "emu.h"
#include "novadasm.h"

nova_disassembler::nova_disassembler()
{
}

u32 nova_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

const char *const s_alc_ops[8] =
{
	"COM", "NEG", "MOV", "INC",
	"ADC", "SUB", "ADD", "AND"
};

const char *const s_skip_codes[8] =
{
	"0", "SKP",
	"SZC", "SNC",
	"SZR", "SNR",
	"SEZ", "SBN"
};

const char *const s_io_ops[8] =
{
	"NIO", "DIA", "DOA", "DIB",
	"DOB", "DIC", "DOC", "SKP"
};

} // anonymous namespace

void nova_disassembler::format_effective_address(std::ostream &stream, offs_t pc, u16 inst)
{
	if (BIT(inst, 10))
		stream << '@';
	u8 disp = inst & 0377;
	if (BIT(inst, 9))
	{
		if (s8(disp) < 0)
		{
			stream << '-';
			disp = -disp;
		}
		util::stream_format(stream, "%o,%d", disp, BIT(inst, 8, 2));
	}
	else
		util::stream_format(stream, "%05o", BIT(inst, 8) ? (pc + int(s8(disp))) & 077777 : disp);
}

void nova_disassembler::format_device_code(std::ostream &stream, u8 device)
{
	switch (device)
	{
	case 001:
		stream << "MDV";
		break;

	case 002: case 003: case 004:
		util::stream_format(stream, "MAP%d", device - 2);
		break;

	case 006:
		stream << "MCAT";
		break;

	case 007:
		stream << "MCAR";
		break;

	case 010:
		stream << "TTI";
		break;

	case 011:
		stream << "TTO";
		break;

	case 012:
		stream << "PTR";
		break;

	case 013:
		stream << "PTP";
		break;

	case 014:
		stream << "RTC";
		break;

	case 015:
		stream << "PLT";
		break;

	case 016:
		stream << "CDR";
		break;

	case 017:
		stream << "LPT";
		break;

	case 020:
		stream << "DSK";
		break;

	case 021:
		stream << "ADCV";
		break;

	case 022:
		stream << "MTA";
		break;

	case 023:
		stream << "DACV";
		break;

	case 024:
		stream << "DCM";
		break;

	case 033:
		stream << "DKP";
		break;

	case 040:
		stream << "SCR";
		break;

	case 041:
		stream << "SCT";
		break;

	case 077:
		stream << "CPU";
		break;

	default:
		util::stream_format(stream, "%o", device);
		break;
	}
}

offs_t nova_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	if (BIT(inst, 15))
	{
		// ALC (arithmetic-logical)
		std::string alcop(s_alc_ops[BIT(inst, 8, 3)]);
		if (BIT(inst, 4, 2) != 0)
			alcop.push_back("ZOC"[BIT(inst, 4, 2) - 1]);
		if (BIT(inst, 6, 2) != 0)
			alcop.push_back("LRS"[BIT(inst, 6, 2) - 1]);
		if (BIT(inst, 3))
			alcop.push_back('#');
		util::stream_format(stream, "%-8s%d,%d", alcop, BIT(inst, 13, 2), BIT(inst, 11, 2));
		if (BIT(inst, 0, 3) != 0)
			stream << ',' << s_skip_codes[BIT(inst, 0, 3)];
		return 1 | (BIT(inst, 1, 2) != 0 ? STEP_COND : 0) | SUPPORTED;
	}
	else if (inst >= 060000)
	{
		// I/O
		switch (inst & 03777)
		{
		case 00177:
			stream << "INTEN";
			return 1 | SUPPORTED;

		case 00277:
			stream << "INTDS";
			return 1 | SUPPORTED;

		case 00477:
			util::stream_format(stream, "%-8s%d", "READS", BIT(inst, 11, 2));
			return 1 | SUPPORTED;

		case 01477:
			util::stream_format(stream, "%-8s%d", "INTA", BIT(inst, 11, 2));
			return 1 | SUPPORTED;

		case 02077:
			util::stream_format(stream, "%-8s%d", "MSKO", BIT(inst, 11, 2));
			return 1 | SUPPORTED;

		case 02677:
			stream << "IORST";
			return 1 | SUPPORTED;

		case 03077:
			stream << "HALT";
			return 1 | SUPPORTED;

		case 03101:
			if (BIT(inst, 11, 2) == 2)
			{
				stream << "DIV";
				return 1 | SUPPORTED;
			}
			break;

		case 03301:
			if (BIT(inst, 11, 2) == 2)
			{
				stream << "MUL";
				return 1 | SUPPORTED;
			}
			break;
		}

		stream << s_io_ops[BIT(inst, 8, 3)];
		if (BIT(inst, 8, 3) == 7)
		{
			util::stream_format(stream, "%c%-4c", BIT(inst, 7) ? 'D' : 'B', BIT(inst, 6) ? 'Z' : 'N');
			format_device_code(stream, BIT(inst, 0, 6));
			return 1 | STEP_COND | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%-5c", " SCP"[BIT(inst, 6, 2)]);
			if (BIT(inst, 8, 3) != 0)
				util::stream_format(stream, "%d,", BIT(inst, 11, 2));
			format_device_code(stream, BIT(inst, 0, 6));
			return 1 | SUPPORTED;
		}
	}
	else if (inst >= 020000)
	{
		// Memory reference with accumulator
		util::stream_format(stream, "%-8s%d,", BIT(inst, 14) ? "STA" : "LDA", BIT(inst, 11, 2));
		format_effective_address(stream, pc, inst);
		return 1 | SUPPORTED;
	}
	else if (BIT(inst, 12))
	{
		// Memory reference with conditional skip
		util::stream_format(stream, "%-8s", BIT(inst, 11) ? "DSZ" : "ISZ");
		format_effective_address(stream, pc, inst);
		return 1 | STEP_COND | SUPPORTED;
	}
	else
	{
		// Memory reference with jump
		util::stream_format(stream, "%-8s", BIT(inst, 11) ? "JSR" : "JMP");
		format_effective_address(stream, pc, inst);
		return 1 | (BIT(inst, 11) ? STEP_OVER : 0) | SUPPORTED;
	}
}
