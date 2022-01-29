 // license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Microchip PIC17 disassembler

***************************************************************************/

#include "emu.h"
#include "pic17d.h"

const char *const pic17_disassembler::s_peripheral_regs[0x20] =
{
	"INDF0", "FSR0", "PCL", "PCLATH", "ALUSTA", "T0STA", "CPUSTA", "INTSTA",
	"INDF1", "FSR1", "WREG", "TMR0L", "TMR0H", "TBLPTRL", "TBLPTRH", "BSR",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // banked
	"PRODL", "PRODH", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const pic17_disassembler::s_tb_ops[4] =
{
	"TLRD", "TLWT", "TABLRD", "TABLWT"
};

const char *const pic17_disassembler::s_bit_ops[4] =
{
	"BSF", "BCF", "BTFSS", "BTFSC"
};

const char *const pic17_disassembler::s_cp_ops[4] =
{
	"CPFSLT", "CPFSEQ", "CPFSGT", "TSTFSZ"
};

const char *const pic17_disassembler::s_alu_ops[0x30 / 2] =
{
	"MOVWF", "SUBWFB", "SUBWF", "DECF", "IORWF", "ANDWF", "XORWF", "ADDWF",
	"ADDWFC", "COMF", "INCF", "DECFSZ", "RRCF", "RLCF", "SWAPF", "INCFSZ",
	"RRNCF", "RLNCF", "INFSNZ", "DCFSNZ", "CLRF", "SETF", "NEGW", "DAW"
};

pic17_disassembler::pic17_disassembler()
	: util::disasm_interface()
{
}

u32 pic17_disassembler::opcode_alignment() const
{
	return 1;
}

void pic17_disassembler::format_register(std::ostream &stream, u8 reg) const
{
	if (reg < 0x20 && s_peripheral_regs[reg] != nullptr)
		stream << s_peripheral_regs[reg];
	else
		util::stream_format(stream, "R%02X", reg);
}

void pic17_disassembler::format_literal(std::ostream &stream, u8 data) const
{
	util::stream_format(stream, "%02Xh", data);
}

void pic17_disassembler::format_address(std::ostream &stream, u16 dst) const
{
	util::stream_format(stream, "%04Xh", dst);
}

offs_t pic17_disassembler::disassemble(std::ostream &stream, offs_t pc, const pic17_disassembler::data_buffer &opcodes, const pic17_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);
	offs_t words = 1;

	if (opcode >= 0xc000)
	{
		if (BIT(opcode, 13))
		{
			util::stream_format(stream, "%-8s", "CALL");
			words |= STEP_OVER;
		}
		else
			util::stream_format(stream, "%-8s", "GOTO");
		format_address(stream, ((pc + 1) & 0xe000) | (opcode & 0x1fff));
	}
	else if (opcode >= 0xb000)
	{
		// Literal and control operations
		switch (opcode & 0x0f00)
		{
		case 0x000:
			util::stream_format(stream, "%-8s", "MOVLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		case 0x100:
			util::stream_format(stream, "%-8s", "ADDLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		case 0x200:
			util::stream_format(stream, "%-8s", "SUBLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		case 0x300:
			util::stream_format(stream, "%-8s", "IORLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		case 0x400:
			util::stream_format(stream, "%-8s", "XORLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		case 0x500:
			util::stream_format(stream, "%-8s", "ANDLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		case 0x600:
			util::stream_format(stream, "%-8s", "RETLW");
			format_literal(stream, opcode & 0x00ff);
			words |= STEP_OUT;
			break;

		case 0x700:
			util::stream_format(stream, "%-8s", "LCALL");
			format_literal(stream, opcode & 0x00ff);
			words |= STEP_OVER;
			break;

		case 0x800:
			util::stream_format(stream, "%-8s%d", "MOVLB", opcode & 0x000f);
			break;

		case 0xa00: case 0xb00:
			util::stream_format(stream, "%-8s%d", "MOVLR", (opcode & 0x00f0) >> 4);
			break;

		case 0xc00:
			util::stream_format(stream, "%-8s", "MULLW");
			format_literal(stream, opcode & 0x00ff);
			break;

		default:
			util::stream_format(stream, "%-8s%04Xh", "DW", opcode);
			break;
		}
	}
	else if (opcode >= 0xa000)
	{
		util::stream_format(stream, "%-8s%d,", s_tb_ops[BIT(opcode, 10, 2)], BIT(opcode, 9));
		if (BIT(opcode, 11))
			util::stream_format(stream, "%d,", BIT(opcode, 8));
		format_register(stream, opcode & 0x00ff);
	}
	else if (opcode >= 0x8000)
	{
		// Bit-oriented file register operations
		util::stream_format(stream, "%-8s", s_bit_ops[BIT(opcode, 11, 2)]);
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d", BIT(opcode, 8, 3));
	}
	else if (opcode >= 0x6000)
	{
		// Byte-to-byte move operations
		util::stream_format(stream, "%-8s", "MOVFP");
		format_register(stream, opcode & 0x00ff);
		stream << ",";
		format_register(stream, (opcode & 0x1f00) >> 8);
	}
	else if (opcode >= 0x4000)
	{
		util::stream_format(stream, "%-8s", "MOVPF");
		format_register(stream, (opcode & 0x1f00) >> 8);
		stream << ",";
		format_register(stream, opcode & 0x00ff);
	}
	else if (opcode >= 0x3800)
	{
		util::stream_format(stream, "%-8s", "BTG");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d", BIT(opcode, 8, 3));
	}
	else if (opcode >= 0x3000)
	{
		if (opcode < 0x3400)
		{
			util::stream_format(stream, "%-8s", s_cp_ops[BIT(opcode, 8, 2)]);
			format_register(stream, opcode & 0x00ff);
		}
		else if (opcode < 0x3500)
		{
			util::stream_format(stream, "%-8s", "MULWF");
			format_register(stream, opcode & 0x00ff);
		}
		else
			util::stream_format(stream, "%-8s%04Xh", "DW", opcode);
	}
	else if (opcode >= 0x0100)
	{
		// Byte-oriented file register operations
		util::stream_format(stream, "%-8s", s_alu_ops[BIT(opcode, 9, 5)]);
		format_register(stream, opcode & 0x00ff);
		if (opcode >= 0x0200)
			util::stream_format(stream, ",%c", BIT(opcode, 8) ? 'F' : 'W');
	}
	else switch (opcode)
	{
	case 0x0000:
		stream << "NOP";
		break;

	case 0x0002:
		stream << "RETURN";
		words |= STEP_OUT;
		break;

	case 0x0003:
		stream << "SLEEP";
		break;

	case 0x0004:
		stream << "CLRWDT";
		break;

	case 0x0005:
		stream << "RETFIE";
		words |= STEP_OUT;
		break;

	default:
		util::stream_format(stream, "%-8s%04Xh", "DW", opcode);
		break;
	}

	return words | SUPPORTED;
}
