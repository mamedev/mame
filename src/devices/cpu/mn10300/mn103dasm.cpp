// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Panasonic MN10300 disassembler

    TODO: add extended instructions, especially those pertaining to the
    MN103E (AM33).

***************************************************************************/

#include "emu.h"
#include "mn103dasm.h"

mn10300_disassembler::mn10300_disassembler()
	: util::disasm_interface()
{
}

u32 mn10300_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

const char *const f_conds[10] = {
	"lt", "gt", "ge", "le", "cs", "hi", "cc", "ls", "eq", "ne"
};

const char *const f_ext_branches[4] = {
	"bvc", "bvs", "bnc", "bns"
};

const char *const f_move_ops[4] = {
	"mov", "mov", "movbu", "movhu"
};

const char *const f_logical_ops[4] = {
	"and", "or", "xor", "btst"
};

const char *const f_shift_ops[3] = {
	"asl", "lsr", "asr"
};

const char *const f_reg_spec[8] = {
	"other0",
	"other", // i.e. D0, D1, A0, A1, MDR, LIR and LAR
	"other2",
	"other3",
	"a3",
	"a2",
	"d3",
	"d2"
};

} // anonymous namespace

void mn10300_disassembler::format_immediate(std::ostream &stream, u32 imm) const
{
	if (s32(imm) < 0)
	{
		stream << '-';
		imm = -imm;
	}
	if (imm > 9)
		stream << "0x";
	util::stream_format(stream, "%x", imm);
}

void mn10300_disassembler::format_immediate_unsigned(std::ostream &stream, u32 imm) const
{
	if (imm > 9)
		stream << "0x";
	util::stream_format(stream, "%x", imm);
}

void mn10300_disassembler::format_an_disp(std::ostream &stream, u32 imm, int n) const
{
	stream << '(';
	format_immediate(stream, imm);
	util::stream_format(stream, ", a%d)", n);
}

void mn10300_disassembler::format_sp_disp(std::ostream &stream, u32 imm) const
{
	stream << '(';
	if (imm > 9)
		stream << "0x";
	util::stream_format(stream, "%x, sp)", imm);
}

void mn10300_disassembler::format_regs(std::ostream &stream, u8 regs) const
{
	if (regs == 0)
		stream << "0";
	else
	{
		stream << '[';
		bool first = true;
		for (int n = 7; n >= 0; n--)
		{
			if (BIT(regs, n))
			{
				if (first)
					first = false;
				else
					stream << ',';
				stream << f_reg_spec[n];
			}
		}
		stream << ']';
	}
}

offs_t mn10300_disassembler::disassemble_f0(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	switch (op2 >> 4)
	{
	case 0x0: case 0x4: case 0x6:
		util::stream_format(stream, "%-8s(a%d), %c%d", f_move_ops[BIT(op2, 5, 2)], BIT(op2, 0, 2), (op2 >> 4) == 0x0 ? 'a' : 'd', BIT(op2, 2, 2));
		return 2 | SUPPORTED;

	case 0x1: case 0x5: case 0x7:
		util::stream_format(stream, "%-8s%c%d, (a%d)", f_move_ops[BIT(op2, 5, 2)], (op2 >> 4) == 0x1 ? 'a' : 'd', BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x8:
		util::stream_format(stream, "%-8sd%d, (a%d)", "bset", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x9:
		util::stream_format(stream, "%-8sd%d, (a%d)", "bclr", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0xf:
		if (op2 < 0xf4)
		{
			util::stream_format(stream, "%-8s(a%d)", "calls", BIT(op2, 0, 2));
			return 2 | STEP_OVER | SUPPORTED;
		}
		else if (op2 < 0xf8)
		{
			util::stream_format(stream, "%-8s(a%d)", "jmp", BIT(op2, 0, 2));
			return 2 | SUPPORTED;
		}
		else if (op2 == 0xfc)
		{
			stream << "rets";
			return 2 | STEP_OUT | SUPPORTED;
		}
		else if (op2 == 0xfd)
		{
			stream << "rti";
			return 2 | STEP_OUT | SUPPORTED;
		}
		else if (op2 == 0xfe)
		{
			stream << "trap";
			return 2 | STEP_OVER | SUPPORTED;
		}
		[[fallthrough]];

	default:
		stream << "?";
		return 1 | SUPPORTED;
	}
}

offs_t mn10300_disassembler::disassemble_f1(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	switch (op2 >> 4)
	{
	case 0x0:
		util::stream_format(stream, "%-8sd%d, d%d", "sub", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x1:
		util::stream_format(stream, "%-8sa%d, d%d", "sub", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x2:
		util::stream_format(stream, "%-8sd%d, a%d", "sub", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x3:
		util::stream_format(stream, "%-8sa%d, a%d", "sub", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x4:
		util::stream_format(stream, "%-8sd%d, d%d", "addc", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x5:
		util::stream_format(stream, "%-8sa%d, d%d", "add", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x6:
		util::stream_format(stream, "%-8sd%d, a%d", "add", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x7:
		util::stream_format(stream, "%-8sa%d, a%d", "add", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x8:
		util::stream_format(stream, "%-8sd%d, d%d", "subc", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x9:
		util::stream_format(stream, "%-8sa%d, d%d", "cmp", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0xa:
		util::stream_format(stream, "%-8sd%d, a%d", "cmp", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0xd:
		util::stream_format(stream, "%-8sa%d, d%d", "mov", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0xe:
		util::stream_format(stream, "%-8sd%d, a%d", "mov", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;
	}

	stream << "?";
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f2(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	switch (op2 >> 4)
	{
	case 0x0: case 0x1: case 0x2:
		util::stream_format(stream, "%-8sd%d, d%d", f_logical_ops[op2 >> 4], BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x3:
		if (op2 < 0x34)
		{
			util::stream_format(stream, "%-8sd%d", "not", BIT(op2, 0, 2));
			return 2 | SUPPORTED;
		}
		else
			break;

	case 0x4:
		util::stream_format(stream, "%-8sd%d, d%d", "mul", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x5:
		util::stream_format(stream, "%-8sd%d, d%d", "mulu", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x6:
		util::stream_format(stream, "%-8sd%d, d%d", "div", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x7:
		util::stream_format(stream, "%-8sd%d, d%d", "divu", BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0x8:
		if (op2 < 0x84)
		{
			util::stream_format(stream, "%-8sd%d", "rol", BIT(op2, 0, 2));
			return 2 | SUPPORTED;
		}
		else if (op2 < 0x88)
		{
			util::stream_format(stream, "%-8sd%d", "ror", BIT(op2, 0, 2));
			return 2 | SUPPORTED;
		}
		else
			break;

	case 0x9: case 0xa: case 0xb:
		util::stream_format(stream, "%-8sd%d, d%d", f_shift_ops[(op2 >> 4) - 0x9], BIT(op2, 2, 2), BIT(op2, 0, 2));
		return 2 | SUPPORTED;

	case 0xd:
		if (op2 < 0xd4)
		{
			util::stream_format(stream, "%-8sd%d", "ext", BIT(op2, 0, 2));
			return 2 | SUPPORTED;
		}
		else
			break;

	case 0xe:
		if (op2 < 0xe8)
		{
			util::stream_format(stream, "%-8s%s, d%d", "mov", BIT(op2, 2) ? "psw" : "mdr", BIT(op2, 0, 2));
			return 2 | SUPPORTED;
		}
		else
			break;

	case 0xf:
		if (BIT(op2, 1))
		{
			util::stream_format(stream, "%-8sd%d, %s", "mov", BIT(op2, 2, 2), BIT(op2, 0) ? "psw" : "mdr");
			return 2 | SUPPORTED;
		}
		else if (!BIT(op2, 0))
		{
			util::stream_format(stream, "%-8sa%d, sp", "mov", BIT(op2, 2, 2));
			return 2 | SUPPORTED;
		}
		else
			break;
	}

	stream << "?";
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f3(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	util::stream_format(stream, "%-8s", "mov");
	if (BIT(op2, 6))
		util::stream_format(stream, "%c%d, (d%d, a%d)", BIT(op2, 7) ? 'a' : 'd', BIT(op2, 4, 2), BIT(op2, 2, 2), BIT(op2, 0, 2));
	else
		util::stream_format(stream, "(d%d, a%d), %c%d", BIT(op2, 2, 2), BIT(op2, 0, 2), BIT(op2, 7) ? 'a' : 'd', BIT(op2, 4, 2));
	return 2 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f4(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 7) + 2]);
	if (BIT(op2, 6))
		util::stream_format(stream, "d%d, (d%d, a%d)", BIT(op2, 4, 2), BIT(op2, 2, 2), BIT(op2, 0, 2));
	else
		util::stream_format(stream, "(d%d, a%d), d%d", BIT(op2, 2, 2), BIT(op2, 0, 2), BIT(op2, 4, 2));
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f5(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	util::stream_format(stream, "udf%02d   d%d, d%d", 20 + (op2 >> 4), BIT(op2, 2, 2), BIT(op2, 0, 2));
	return 2 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f6(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	util::stream_format(stream, "udf%02d   d%d, d%d", op2 >> 4, BIT(op2, 2, 2), BIT(op2, 0, 2));
	return 2 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f8(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	if (op2 < 0x80)
	{
		util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 5, 2)]);
		if (BIT(op2, 4))
		{
			util::stream_format(stream, "%c%d, ", BIT(op2, 5, 2) == 1 ? 'a' : 'd', BIT(op2, 2, 2));
			format_an_disp(stream, s8(opcodes.r8(pc + 2)), BIT(op2, 0, 2));
		}
		else
		{
			format_an_disp(stream, s8(opcodes.r8(pc + 2)), BIT(op2, 0, 2));
			util::stream_format(stream, ", %c%d", BIT(op2, 5, 2) == 1 ? 'a' : 'd', BIT(op2, 2, 2));
		}
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0xf2) == 0x92)
	{
		util::stream_format(stream, "%-8sd%d, ", f_move_ops[BIT(op2, 0, 2)], BIT(op2, 2, 2));
		format_sp_disp(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else switch (op2 & 0xfc)
	{
	case 0xb8: case 0xbc:
		util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 2, 2)]);
		format_sp_disp(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ", d%d", BIT(op2, 0, 2));
		return 3 | SUPPORTED;

	case 0xc0: case 0xc4: case 0xc8:
		util::stream_format(stream, "%-8s%u, d%d", f_shift_ops[BIT(op2, 2, 2)], opcodes.r8(pc + 2), BIT(op2, 0, 2));
		return 3 | SUPPORTED;

	case 0xe0: case 0xe4: case 0xec:
		util::stream_format(stream, "%-8s0x%02x, d%d", f_logical_ops[BIT(op2, 2, 2)], opcodes.r8(pc + 2), BIT(op2, 0, 2)); // zero_ext
		return 3 | SUPPORTED;

	case 0xe8:
		util::stream_format(stream, "%-8s0x%08x", f_ext_branches[BIT(op2, 0, 2)], u32(pc + s8(opcodes.r8(pc + 2))));
		return 3 | STEP_COND | SUPPORTED;

	case 0xf0:
		util::stream_format(stream, "%-8s", "mov");
		format_an_disp(stream, s8(opcodes.r8(pc + 2)), BIT(op2, 0, 2));
		stream << ", sp";
		return 3 | SUPPORTED;

	case 0xf4:
		util::stream_format(stream, "%-8ssp, ", "mov");
		format_an_disp(stream, s8(opcodes.r8(pc + 2)), BIT(op2, 0, 2));
		return 3 | SUPPORTED;

	case 0xfc:
		if (op2 == 0xfe)
		{
			util::stream_format(stream, "%-8s", "add");
			format_immediate(stream, s8(opcodes.r8(pc + 2))); // sign_ext
			stream << ", sp";
			return 3 | SUPPORTED;
		}
		break;
	}

	stream << "?";
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_f9(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	if (BIT(op2, 2))
		util::stream_format(stream, "udfu%02d  0x%02x, d%d", (BIT(op2, 3) ? 20 : 0) + (op2 >> 4), opcodes.r8(pc + 2), BIT(op2, 0, 2));
	else
		util::stream_format(stream, "udf%02d   0x%02x, d%d", (BIT(op2, 3) ? 20 : 0) + (op2 >> 4), opcodes.r8(pc + 2), BIT(op2, 0, 2));
	return 3 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_fa(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	if (op2 < 0x80)
	{
		util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 5, 2)]);
		if (BIT(op2, 4))
		{
			util::stream_format(stream, "%c%d, ", BIT(op2, 5, 2) == 1 ? 'a' : 'd', BIT(op2, 2, 2));
			format_an_disp(stream, s16(opcodes.r16(pc + 2)), BIT(op2, 0, 2));
		}
		else
		{
			format_an_disp(stream, s16(opcodes.r16(pc + 2)), BIT(op2, 0, 2));
			util::stream_format(stream, ", %c%d", BIT(op2, 5, 2) == 1 ? 'a' : 'd', BIT(op2, 2, 2));
		}
		return 4 | SUPPORTED;
	}
	else if (op2 < 0xa0)
	{
		switch (op2 & 0x13)
		{
		case 0x00:
			util::stream_format(stream, "%-8sa%d, (0x%04x)", "mov", BIT(op2, 2, 2), opcodes.r16(pc + 2));
			return 4 | SUPPORTED;

		case 0x10: case 0x11: case 0x12: case 0x13:
			util::stream_format(stream, "%-8sd%d, ", f_move_ops[BIT(op2, 0, 2)], BIT(op2, 0, 2) == 0 ? 'a' : 'd', BIT(op2, 2, 2));
			format_sp_disp(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
	}
	else switch (op2 & 0xfc)
	{
	case 0xa0:
		util::stream_format(stream, "%-8s(0x%04x), a%d", "mov", opcodes.r16(pc + 2), BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xb0: case 0xb4: case 0xb8: case 0xbc:
		util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 2, 2)]);
		format_sp_disp(stream, opcodes.r16(pc + 2));
		util::stream_format(stream, ", %c%d", BIT(op2, 2, 2) == 0 ? 'a' : 'd', BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xc0: case 0xd0:
		util::stream_format(stream, "%-8s", "add");
		format_immediate(stream, s16(opcodes.r16(pc + 2)));
		util::stream_format(stream, ", %c%d", BIT(op2, 4) ? 'a' : 'd', BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xc8:
		util::stream_format(stream, "%-8s", "cmp");
		format_immediate(stream, s16(opcodes.r16(pc + 2))); // sign_ext
		util::stream_format(stream, ", d%d", BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xd8:
		util::stream_format(stream, "%-8s", "cmp");
		format_immediate_unsigned(stream, opcodes.r16(pc + 2)); // zero_ext
		util::stream_format(stream, ", a%d", BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xe0: case 0xe4: case 0xe8: case 0xec:
		util::stream_format(stream, "%-8s0x%04x, d%d", f_logical_ops[BIT(op2, 2, 2)], opcodes.r16(pc + 2), BIT(op2, 0, 2)); // zero_ext
		return 4 | SUPPORTED;

	case 0xf0:
		util::stream_format(stream, "%-8s0x%02x, ", "bset", opcodes.r8(pc + 3));
		format_an_disp(stream, opcodes.r8(pc + 2), BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xf4:
		util::stream_format(stream, "%-8s0x%02x, ", "bclr", opcodes.r8(pc + 3));
		format_an_disp(stream, opcodes.r8(pc + 2), BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xf8:
		util::stream_format(stream, "%-8s0x%02x, ", "btst", opcodes.r8(pc + 3));
		format_an_disp(stream, opcodes.r8(pc + 2), BIT(op2, 0, 2));
		return 4 | SUPPORTED;

	case 0xfc:
		if (op2 == 0xfe)
		{
			util::stream_format(stream, "%-8s", "add");
			format_immediate(stream, s16(opcodes.r16(pc + 2))); // sign_ext
			stream << ", sp";
			return 4 | SUPPORTED;
		}
		else if (op2 == 0xff)
		{
			util::stream_format(stream, "%-8s(0x%08x)", "calls", u32(pc + s16(opcodes.r16(pc + 2))));
			return 4 | STEP_OVER | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%-8s0x%04x, psw", f_logical_ops[BIT(op2, 0)], opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
	}

	stream << "?";
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_fb(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	if (BIT(op2, 2))
		util::stream_format(stream, "udfu%02d  0x%04x, d%d", (BIT(op2, 3) ? 20 : 0) + (op2 >> 4), opcodes.r16(pc + 2), BIT(op2, 0, 2));
	else
		util::stream_format(stream, "udf%02d   0x%04x, d%d", (BIT(op2, 3) ? 20 : 0) + (op2 >> 4), opcodes.r16(pc + 2), BIT(op2, 0, 2));
	return 4 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_fc(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	if (op2 < 0x80)
	{
		util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 5, 2)]);
		if (BIT(op2, 4))
			util::stream_format(stream, "%c%d, (0x%08x, a%d)", BIT(op2, 5, 2) == 1 ? 'a' : 'd', BIT(op2, 2, 2), opcodes.r32(pc + 2), BIT(op2, 0, 2));
		else
			util::stream_format(stream, "(0x%08x, a%d), %c%d", opcodes.r32(pc + 2), BIT(op2, 2, 2), BIT(op2, 5, 2) == 1 ? 'a' : 'd', BIT(op2, 0, 2));
		return 6 | SUPPORTED;
	}
	else if (op2 < 0xa0)
	{
		util::stream_format(stream, "%-8s%c%d, ", f_move_ops[BIT(op2, 0, 2)], BIT(op2, 0, 2) == 0 ? 'a' : 'd', BIT(op2, 2, 2));
		if (BIT(op2, 4))
			format_sp_disp(stream, opcodes.r32(pc + 2));
		else
			util::stream_format(stream, "(0x%08x)", opcodes.r32(pc + 2));
		return 6 | SUPPORTED;
	}
	else if (op2 < 0xc0)
	{
		util::stream_format(stream, "%-8s", f_move_ops[BIT(op2, 2, 2)]);
		if (BIT(op2, 4))
			format_sp_disp(stream, opcodes.r32(pc + 2));
		else
			util::stream_format(stream, "(0x%08x)", opcodes.r32(pc + 2));
		util::stream_format(stream, ", %c%d", BIT(op2, 2, 2) == 0 ? 'a' : 'd', BIT(op2, 0, 2));
		return 6 | SUPPORTED;
	}
	else switch (op2 & 0xfc)
	{
	case 0xc0: case 0xd0:
		util::stream_format(stream, "%-8s", "add");
		format_immediate_unsigned(stream, opcodes.r32(pc + 2));
		util::stream_format(stream, ", %c%d", BIT(op2, 4) ? 'a' : 'd', BIT(op2, 0, 2));
		return 6 | SUPPORTED;

	case 0xc4: case 0xd4:
		util::stream_format(stream, "%-8s", "sub");
		format_immediate_unsigned(stream, opcodes.r32(pc + 2));
		util::stream_format(stream, ", %c%d", BIT(op2, 4) ? 'a' : 'd', BIT(op2, 0, 2));
		return 6 | SUPPORTED;

	case 0xc8: case 0xd8:
		util::stream_format(stream, "%-8s", "cmp");
		format_immediate_unsigned(stream, opcodes.r32(pc + 2));
		util::stream_format(stream, ", %c%d", BIT(op2, 4) ? 'a' : 'd', BIT(op2, 0, 2));
		return 6 | SUPPORTED;

	case 0xcc: case 0xdc:
		util::stream_format(stream, "%-8s0x%08x, %c%d", "mov", opcodes.r32(pc + 2), BIT(op2, 4) ? 'a' : 'd', BIT(op2, 0, 2));
		return 6 | SUPPORTED;

	case 0xe0: case 0xe4: case 0xe8: case 0xec:
		util::stream_format(stream, "%-8s0x%08x, d%d", f_logical_ops[BIT(op2, 2, 2)], opcodes.r32(pc + 2), BIT(op2, 0, 2));
		return 6 | SUPPORTED;

	case 0xfc:
		if (op2 == 0xfe)
		{
			util::stream_format(stream, "%-8s0x%08x, sp", "add", opcodes.r32(pc + 2));
			return 6 | SUPPORTED;
		}
		else if (op2 == 0xff)
		{
			util::stream_format(stream, "%-8s(0x%08x)", "calls", u32(pc + opcodes.r32(pc + 2)));
			return 6 | STEP_OVER | SUPPORTED;
		}
		else
			break;
	}

	stream << "?";
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_fd(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	if (BIT(op2, 2))
		util::stream_format(stream, "udfu%02d  0x%08x, d%d", (BIT(op2, 3) ? 20 : 0) + (op2 >> 4), opcodes.r32(pc + 2), BIT(op2, 0, 2));
	else
		util::stream_format(stream, "udf%02d   0x%08x, d%d", (BIT(op2, 3) ? 20 : 0) + (op2 >> 4), opcodes.r32(pc + 2), BIT(op2, 0, 2));
	return 6 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble_fe(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes) const
{
	u8 op2 = opcodes.r8(pc + 1);
	switch (op2)
	{
	case 0x00:
		util::stream_format(stream, "%-8s0x%02x, (0x%08x)", "bset", opcodes.r8(pc + 6), opcodes.r32(pc + 2));
		return 7 | SUPPORTED;

	case 0x01:
		util::stream_format(stream, "%-8s0x%02x, (0x%08x)", "bclr", opcodes.r8(pc + 6), opcodes.r32(pc + 2));
		return 7 | SUPPORTED;

	case 0x02:
		util::stream_format(stream, "%-8s0x%02x, (0x%08x)", "btst", opcodes.r8(pc + 6), opcodes.r32(pc + 2));
		return 7 | SUPPORTED;

	case 0x80:
		util::stream_format(stream, "%-8s0x%02x, (0x%04x)", "bset", opcodes.r8(pc + 4), opcodes.r16(pc + 2));
		return 5 | SUPPORTED;

	case 0x81:
		util::stream_format(stream, "%-8s0x%02x, (0x%04x)", "bclr", opcodes.r8(pc + 4), opcodes.r16(pc + 2));
		return 5 | SUPPORTED;

	case 0x82:
		util::stream_format(stream, "%-8s0x%02x, (0x%04x)", "btst", opcodes.r8(pc + 4), opcodes.r16(pc + 2));
		return 5 | SUPPORTED;
	}

	stream << "?";
	return 1 | SUPPORTED;
}

offs_t mn10300_disassembler::disassemble(std::ostream &stream, offs_t pc, const mn10300_disassembler::data_buffer &opcodes, const mn10300_disassembler::data_buffer &params)
{
	u8 opcode = opcodes.r8(pc);
	switch (opcode)
	{
	case 0x00: case 0x04: case 0x08: case 0x0c:
		util::stream_format(stream, "%-8sd%d", "clr", BIT(opcode, 2, 2));
		return 1 | SUPPORTED;

	case 0x01: case 0x05: case 0x09: case 0x0d:
	case 0x02: case 0x06: case 0x0a: case 0x0e:
	case 0x03: case 0x07: case 0x0b: case 0x0f:
		util::stream_format(stream, "%-8sd%d, (0x%04x)", f_move_ops[BIT(opcode, 0, 2)], BIT(opcode, 2, 2), opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x10: case 0x11: case 0x12: case 0x13:
		util::stream_format(stream, "%-8sd%d", "extb", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x14: case 0x15: case 0x16: case 0x17:
		util::stream_format(stream, "%-8sd%d", "extbu", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x18: case 0x19: case 0x1a: case 0x1b:
		util::stream_format(stream, "%-8sd%d", "exth", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		util::stream_format(stream, "%-8sd%d", "exthu", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x28: case 0x29: case 0x2a: case 0x2b:
		util::stream_format(stream, "%-8s", "add");
		format_immediate(stream, s8(opcodes.r8(pc + 1)));
		util::stream_format(stream, ", %c%d", BIT(opcode, 3) ? 'd' : 'a', BIT(opcode, 0, 2));
		return 2 | SUPPORTED;

	case 0x24: case 0x25: case 0x26: case 0x27:
		util::stream_format(stream, "%-8s", "mov");
		format_immediate_unsigned(stream, opcodes.r16(pc + 1)); // zero_ext
		util::stream_format(stream, ", a%d", BIT(opcode, 0, 2));
		return 3 | SUPPORTED;

	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		util::stream_format(stream, "%-8s", "mov");
		format_immediate(stream, s16(opcodes.r16(pc + 1))); // sign_ext
		util::stream_format(stream, ", d%d", BIT(opcode, 0, 2));
		return 3 | SUPPORTED;

	case 0x30: case 0x31: case 0x32: case 0x33:
	case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b:
		util::stream_format(stream, "%-8s(0x%04x), d%d", f_move_ops[BIT(opcode, 2, 2) + 1], opcodes.r16(pc + 1), BIT(opcode, 0, 2));
		return 3 | SUPPORTED;

	case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		util::stream_format(stream, "%-8ssp, a%d", "mov", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x40: case 0x44: case 0x48: case 0x4c:
	case 0x41: case 0x45: case 0x49: case 0x4d:
		util::stream_format(stream, "%-8s%c%d", "inc", BIT(opcode, 0) ? 'a' : 'd', BIT(opcode, 2, 2));
		return 1 | SUPPORTED;

	case 0x42: case 0x46: case 0x4a: case 0x4e:
	case 0x43: case 0x47: case 0x4b: case 0x4f:
		util::stream_format(stream, "%-8s%c%d, ", "mov", BIT(opcode, 0) ? 'a' : 'd', BIT(opcode, 2, 2));
		format_sp_disp(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x50: case 0x51: case 0x52: case 0x53:
		util::stream_format(stream, "%-8sa%d", "inc4", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x54: case 0x55: case 0x56: case 0x57:
		util::stream_format(stream, "%-8sd%d", "asl2", BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x58: case 0x59: case 0x5a: case 0x5b:
	case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		util::stream_format(stream, "%-8s", "mov");
		format_sp_disp(stream, opcodes.r8(pc + 1));
		util::stream_format(stream, ", %c%d", BIT(opcode, 2) ? 'a' : 'd', BIT(opcode, 0, 2));
		return 2 | SUPPORTED;

	case 0x60: case 0x61: case 0x62: case 0x63:
	case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b:
	case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		util::stream_format(stream, "%-8sd%d, (a%d)", "mov", BIT(opcode, 2, 2), BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x70: case 0x71: case 0x72: case 0x73:
	case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b:
	case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		util::stream_format(stream, "%-8s(a%d), d%d", "mov", BIT(opcode, 0, 2), BIT(opcode, 2, 2));
		return 1 | SUPPORTED;

	case 0x80: case 0x85: case 0x8a: case 0x8f:
	case 0xa0: case 0xa5: case 0xaa: case 0xaf:
		util::stream_format(stream, "%-8s", BIT(opcode, 5) ? "cmp" : "mov");
		format_immediate(stream, s8(opcodes.r8(pc + 1)));
		util::stream_format(stream, ", d%d", BIT(opcode, 0, 2));
		return 2 | SUPPORTED;

	case 0x81: case 0x82: case 0x83:
	case 0x84: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8b:
	case 0x8c: case 0x8d: case 0x8e:
	case 0xa1: case 0xa2: case 0xa3:
	case 0xa4: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xab:
	case 0xac: case 0xad: case 0xae:
		util::stream_format(stream, "%-8sd%d, d%d", BIT(opcode, 5) ? "cmp" : "mov", BIT(opcode, 2, 2), BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0x90: case 0x95: case 0x9a: case 0x9f:
	case 0xb0: case 0xb5: case 0xba: case 0xbf:
		util::stream_format(stream, "%-8s", BIT(opcode, 5) ? "cmp" : "mov");
		format_immediate_unsigned(stream, opcodes.r8(pc + 1)); // zero_ext
		util::stream_format(stream, ", a%d", BIT(opcode, 0, 2));
		return 2 | SUPPORTED;

	case 0x91: case 0x92: case 0x93:
	case 0x94: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9b:
	case 0x9c: case 0x9d: case 0x9e:
	case 0xb1: case 0xb2: case 0xb3:
	case 0xb4: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xbb:
	case 0xbc: case 0xbd: case 0xbe:
		util::stream_format(stream, "%-8sa%d, a%d", BIT(opcode, 5) ? "cmp" : "mov", BIT(opcode, 2, 2), BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9:
		util::stream_format(stream, "b%-7s0x%08x", f_conds[opcode & 0x0f], u32(pc + s8(opcodes.r8(pc + 1))));
		return 2 | STEP_COND | SUPPORTED;

	case 0xca:
		util::stream_format(stream, "%-8s0x%08x", "bra", u32(pc + s8(opcodes.r8(pc + 1))));
		return 2 | SUPPORTED;

	case 0xcb:
		stream << "nop";
		return 1 | SUPPORTED;

	case 0xcc:
		util::stream_format(stream, "%-8s0x%08x", "jmp", u32(pc + s16(opcodes.r16(pc + 1))));
		return 3 | SUPPORTED;

	case 0xcd:
		util::stream_format(stream, "%-8s0x%08x, ", "call", u32(pc + s16(opcodes.r16(pc + 1))));
		format_regs(stream, opcodes.r8(pc + 3));
		stream << ", ";
		format_immediate_unsigned(stream, opcodes.r8(pc + 4));
		return 5 | STEP_OVER | SUPPORTED;

	case 0xce:
		util::stream_format(stream, "%-8s(sp), ", "movm");
		format_regs(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xcf:
		util::stream_format(stream, "%-8s", "movm");
		format_regs(stream, opcodes.r8(pc + 1));
		stream << ", (sp)";
		return 2 | SUPPORTED;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8: case 0xd9:
		util::stream_format(stream, "l%s", f_conds[opcode & 0x0f]);
		return 1 | STEP_COND | SUPPORTED;

	case 0xda:
		stream << "lra";
		return 1 | SUPPORTED;

	case 0xdb:
		stream << "setlb";
		return 1 | SUPPORTED;

	case 0xdc:
		util::stream_format(stream, "%-8s0x%08x", "jmp", u32(pc + opcodes.r32(pc + 1)));
		return 5 | SUPPORTED;

	case 0xdd:
		util::stream_format(stream, "%-8s0x%08x, ", "call", u32(pc + opcodes.r32(pc + 1)));
		format_regs(stream, opcodes.r8(pc + 5));
		stream << ", ";
		format_immediate_unsigned(stream, opcodes.r8(pc + 6));
		return 7 | STEP_OVER | SUPPORTED;

	case 0xde: case 0xdf:
		util::stream_format(stream, "%-8s", BIT(opcode, 0) ? "ret" : "retf");
		format_regs(stream, opcodes.r8(pc + 1));
		stream << ", ";
		format_immediate_unsigned(stream, opcodes.r8(pc + 2));
		return 3 | STEP_OUT | SUPPORTED;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3:
	case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb:
	case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "%-8sd%d, d%d", "add", BIT(opcode, 2, 2), BIT(opcode, 0, 2));
		return 1 | SUPPORTED;

	case 0xf0:
		return disassemble_f0(stream, pc, opcodes);

	case 0xf1:
		return disassemble_f1(stream, pc, opcodes);

	case 0xf2:
		return disassemble_f2(stream, pc, opcodes);

	case 0xf3:
		return disassemble_f3(stream, pc, opcodes);

	case 0xf4:
		return disassemble_f4(stream, pc, opcodes);

	case 0xf5:
		return disassemble_f5(stream, pc, opcodes);

	case 0xf6:
		return disassemble_f6(stream, pc, opcodes);

	case 0xf8:
		return disassemble_f8(stream, pc, opcodes);

	case 0xf9:
		return disassemble_f9(stream, pc, opcodes);

	case 0xfa:
		return disassemble_fa(stream, pc, opcodes);

	case 0xfb:
		return disassemble_fb(stream, pc, opcodes);

	case 0xfc:
		return disassemble_fc(stream, pc, opcodes);

	case 0xfd:
		return disassemble_fd(stream, pc, opcodes);

	case 0xfe:
		return disassemble_fe(stream, pc, opcodes);

	default:
		stream << "?";
		return 1 | SUPPORTED;
	}
}
