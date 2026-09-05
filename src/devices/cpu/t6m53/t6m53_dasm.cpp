// license:BSD-3-Clause
// copyright-holders:grubbyplaya

#include "emu.h"
#include "t6m53_dasm.h"

u32 t6m53_disassembler::opcode_alignment() const
{
	return 2;
}

u16 t6m53_disassembler::bit_address(u16 op)
{
    const uint8_t YX = ((op & 0xf) << 4) | ((op >> 4) & 0xf);
	return (BIT(op, 9) << 8) | YX;
}

u16 t6m53_disassembler::ef(u16 op)
{
    return 0x100 | ((op >> 7) & 0x20) | ((op >> 4) & 0x1f);
}

unsigned t6m53_disassembler::bit_number(u16 op)
{
	return unsigned(BIT(op, 8) | (BIT(op, 10) << 1));
}

const char *t6m53_disassembler::size_suffix(u16 op)
{
	return BIT(op, 8) ? ".N" : ".B";
}

const char *t6m53_disassembler::compare_name(u16 op)
{
	switch (op & 0x0c00)
	{
	case 0x0400: return "=";
	case 0x0600: return "!=";
	case 0x0800: return "<";
	case 0x0a00: return ">=";
	default:      return "?";
	}
}

bool t6m53_disassembler::has_branch(u16 op)
{
	/*
	 * The assembler adds the branch modifier returned by jo():
	 *
	 *   000: no branch
	 *   200: JUMP/CALL
	 *   400: IF NO CARRY
	 *   600: IF CARRY
	 */
	return (op & 0x0600) != 0;
}

void t6m53_disassembler::print_location(std::ostream &stream, u16 location)
{
	location &= 0x1ff;

	switch (location)
	{
	case 0x0ff: stream << "A"; return;
	case 0x100: stream << "I"; return;
	case 0x118: stream << "SP"; return;
	case 0x11c: stream << "DP"; return;
	case 0x1ff: stream << "(I)"; return;
	default: break;
	}

	stream << "R" << util::string_format("%03X", location);
}

void t6m53_disassembler::print_bit_location(std::ostream &stream, u16 op)
{
	print_location(stream, bit_address(op));
	stream << "." << bit_number(op);
}

void t6m53_disassembler::print_target(std::ostream &stream, u16 target)
{
	if (target & 0x8000)
		stream << "CALL   $" << util::string_format("%04X", (target & 0x7fff) << 1);
	else
		stream << "JUMP   $" << util::string_format("%04X", target << 1);
}

void t6m53_disassembler::print_branch(std::ostream &stream, u16 op, u16 target, offs_t &flags)
{
	switch (op & 0x0600)
	{
	case 0x0200:
		print_target(stream, target);
		flags |= BIT(target, 15) ? STEP_OVER : STEP_COND;
		break;

	case 0x0400:
		stream << "IF NO CARRY: ";
		print_target(stream, target);
		flags |= STEP_COND;
		break;

	case 0x0600:
		stream << "IF CARRY: ";
		print_target(stream, target);
		flags |= STEP_COND;
		break;

	default:
		break;
	}
}

offs_t t6m53_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u16 op = (u16(opcodes.r8(pc)) << 8) | opcodes.r8(pc + 1);
	const u16 param0 = (u16(opcodes.r8(pc + 2)) << 8) | opcodes.r8(pc + 3);
	offs_t flags = SUPPORTED;
	offs_t words = 1;

	// -----------------------------------------------------------------
	// System / I / DP instructions
	// -----------------------------------------------------------------
    if ((op & 0xFE00) == 0x0000) {
        bool first = true;
        auto append_inst = [&](const char* fmt, auto... args) {
            if (!first)
                stream << "; ";
            util::stream_format(stream, fmt, args...);
            first = false;
        };

        if (op & 0x04) {
            switch (op & 0x07) {
                case 0x04:
                    append_inst("READU%s", BIT(op, 8) ? ".N" : "");
                    break;

                case 0x05:
                    append_inst("READD%s", BIT(op, 8) ? ".N" : "");
                    break;

                case 0x06:
                    append_inst("WRITEU%s", BIT(op, 8) ? ".N" : "");
                    break;

                case 0x07:
                    append_inst("WRITED%s", BIT(op, 8) ? ".N" : "");
                    break;
            }
        } else if ((op & 0x03) == op) {
			util::stream_format(stream, "SYS     $%03X", op & 0x1ff);
        }

        if ((op & 0x0018) == 0x0018)
            append_inst("SHR     A");

        if (op & 0x0020)
            append_inst("SCANKEYS");

        if (op & 0x0040) {
            append_inst("RET");
            flags |= STEP_OUT;
        }

        if (op & 0x0080)
            append_inst("JUMP    (104)");
    } else if ((op & 0x0ff0) == 0x0f00)
	{
		util::stream_format(stream, "REP     $%X", (op & 0x0f) + 1);
	}

	// -----------------------------------------------------------------
	// Register transfers
	// -----------------------------------------------------------------

	else if (op < 0x0400)
	{
		stream << "LOAD" << size_suffix(op) << "  JYX, (I)";
	}
	else if (op < 0x0600)
	{
		stream << "LOAD" << size_suffix(op) << "  (I), JYX";
	}
	else if (op < 0x0800)
	{
		stream << "XCHG" << size_suffix(op) << "  (I), JYX";
	}
	else if (op < 0x0a00)
	{
		stream << "LOAD    I, ";
		print_location(stream, op & 0x1ff);
	}
	else if (op < 0x0c00)
	{
		words = 2;
		stream << "LOAD    I, ";
		print_location(stream, op & 0x1ff);
		stream << "; ";
		print_target(stream, param0);
		flags |= BIT(param0, 15) ? STEP_OVER : STEP_COND;
	}
	else if (op < 0x0e00)
	{
		stream << "LOAD    (I+), #$" << util::string_format("%02X", op & 0xff);
	}
	else if (op < 0x1000)
	{
		stream << "LOAD    ";
		print_location(stream, ef(op));
		stream << ", #$" << util::string_format("%02X", op & 0xf);
	}
	else if (op < 0x1400)
	{
		stream << "LOAD" << size_suffix(op) << "  A, WYX";
	}
	else if (op < 0x1800)
	{
		stream << "XCHG" << size_suffix(op) << "  A, WYX";
	}
	else if (op < 0x1c00)
	{
		stream << "STORE" << size_suffix(op) << " WYX, A";
	}
	else if (op < 0x1e00)
	{
		stream << "LOAD    A, #$" << util::string_format("%02X", op & 0xff);
	}
	else if (op < 0x2000)
	{
		stream << "LOAD.N  ";
		print_location(stream, ef(op));
		stream << ", #$" << util::string_format("%01X", op & 0x0f);
	}

	// -----------------------------------------------------------------
	// Bit operations
	// -----------------------------------------------------------------

	else if (op < 0x2800)
	{
		stream << "TOGGLE  ";
		print_bit_location(stream, op);
	}
	else if (op < 0x3000)
	{
		stream << "CLEAR   ";
		print_bit_location(stream, op);
	}
	else if (op < 0x3800)
	{
		stream << "SET     ";
		print_bit_location(stream, op);
	}
	else if (op < 0x3c00)
	{
		stream << "NOT" << size_suffix(op) << "  WYX";
	}
	else if (op < 0x3e00)
	{
		stream << "XOR" << size_suffix(op) << "   JYX";
	}
	else if (op < 0x4000)
	{
		stream << "OR" << size_suffix(op) << "    JYX";
	}

	// -----------------------------------------------------------------
	// IF / compare
	// -----------------------------------------------------------------

	else if (op < 0x6000)
	{
		words = 2;
		stream << "IF      ";
		print_location(stream, ef(op));
		stream << " " << compare_name(op) << " #$"
		       << util::string_format("%02X", op & 0xff);
		stream << ": ";
		print_target(stream, param0);
		flags |= STEP_COND;
	}
	else if (op < 0x7000)
	{
		const char *name = BIT(op, 8) ? "IF.N" : "IF.B";
		words = 2;
		stream << name << "   (I) " << compare_name(op) << " #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff));
		stream << ": ";
		print_target(stream, param0);
		flags |= STEP_COND;
	}
	else if (op < 0x8000)
	{
		const char *name = BIT(op, 8) ? "IF.N" : "IF.B";
		words = 2;
		stream << name << "   A " << compare_name(op) << " #$"
		       << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff));
		stream << ": ";
		print_target(stream, param0);
		flags |= STEP_COND;
	}

	// -----------------------------------------------------------------
	// Arithmetic register forms
	// -----------------------------------------------------------------

	else if (op < 0xa000)
	{
		const char *mnemonic;
		switch (op & 0x1800)
		{
		case 0x0000: mnemonic = "ADD"; break;
		case 0x0800: mnemonic = "DADD"; break;
		case 0x1000: mnemonic = "SUB"; break;
		default:      mnemonic = "DSUB"; break;
		}

		stream << mnemonic << size_suffix(op) << "  JYX";

		if (has_branch(op))
		{
			words = 2;
			stream << "; ";
			print_branch(stream, op, param0, flags);
		}
	}

	// -----------------------------------------------------------------
	// Bit test
	// -----------------------------------------------------------------

	else if (op < 0xb000)
	{
		words = 2;
		stream << "IF     ";
		if (BIT(op, 11))
			stream << "SET   ";
		else
			stream << "CLEAR ";
		print_bit_location(stream, op);
		stream << ": ";
		print_target(stream, param0);
		flags |= STEP_COND;
	}
	else if (op == 0xb000)
	{
		stream << "BREAK";
	}
	else if (op < 0xc000)
	{
		const char *name = BIT(op, 8) ? "IF.N" : "IF.B";
		words = 2;
		stream << name << "    JYX";
		stream << " " << compare_name(op) << " $" << util::string_format("%02X", op & 0xff) << ": ";
		print_target(stream, param0);
		flags |= STEP_COND;
	}

	// -----------------------------------------------------------------
	// Register/immediate arithmetic forms
	// -----------------------------------------------------------------

	else if (op < 0xe000)
	{
        stream << ((op & 0x0800) ? "DADD    JYX" : "ADD     JYX");
		stream << ", #$" << util::string_format("%01X", op & 0xf);

		if (has_branch(op))
		{
			words = 2;
			stream << "; ";
			print_branch(stream, op, param0, flags);
		}
	}
	else if (op < 0xf000)
	{
		const char *mnemonic = (op & 0x1000) ? "SUB" : "ADD";
		stream << mnemonic << size_suffix(op) << "  (I), #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff));

		if (has_branch(op))
		{
			words = 2;
			stream << "; ";
			print_branch(stream, op, param0, flags);
		}
	}
	else
	{
		const char *mnemonic = (op & 0x1000) ? "SUB" : "ADD";
		stream << mnemonic << size_suffix(op) << "  A, #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff));

		if (has_branch(op))
		{
			words = 2;
			stream << "; ";
			print_branch(stream, op, param0, flags);
		}
	}

	return (words * 2) | flags;
}