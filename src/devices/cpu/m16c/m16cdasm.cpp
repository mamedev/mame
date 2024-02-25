// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Mitsubishi/Renesas M16C disassembler

***************************************************************************/

#include "emu.h"
#include "m16cdasm.h"

m16c_disassembler::m16c_disassembler()
	: util::disasm_interface()
{
}

const char *const m16c_disassembler::s_regs[2][6] =
{
	{ "R0L", "R0H", "R1L", "R1H", "A0", "A1" },
	{ "R0", "R1", "R2", "R3", "A0", "A1" }
};

const char *const m16c_disassembler::s_cregs[8] =
{
	"", "INTBL", "INTBH", "FLG", "ISP", "SP", "SB", "FB"
};

const char *const m16c_disassembler::s_byte_ops[8] =
{
	"mov.b", "mov.b",
	"and.b", "or.b",
	"add.b", "sub.b",
	"mov.b", "cmp.b"
};

const char *const m16c_disassembler::s_bit_ops[14] =
{
	"btstc", "btsts",
	"bmcnd", "bntst",
	"band", "bnand",
	"bor", "bnor",
	"bclr", "bset",
	"bnot", "btst",
	"bxor", "bxnor"
};

const char *const m16c_disassembler::s_cnds[16] =
{
	"c", "gtu", "z", "n", // GEU = C, EQ = Z
	"nc", "leu", "nz", "pz", // LTU = NC, NE = NZ
	"le", "o", "ge", "",
	"gt", "no", "lt", ""
};

const char *const m16c_disassembler::s_imm76_ops[2][9] =
{
	{ "tst.b", "xor.b", "and.b", "or.b", "add.b", "sub.b", "adc.b", "sbb.b", "cmp.b" },
	{ "tst.w", "xor.w", "and.w", "or.w", "add.w", "sub.w", "adc.w", "sbb.w", "cmp.w" }
};

const char *const m16c_disassembler::s_nibmov_ops[4] =
{
	"movll", "movhl", "movlh", "movhh"
};

const char *const m16c_disassembler::s_decimal_ops[2][4] =
{
	{ "dadd.b", "dsub.b", "dadc.b", "dsbb.b" },
	{ "dadd.w", "dsub.w", "dadc.w", "dsbb.w" }
};


u32 m16c_disassembler::opcode_alignment() const
{
	return 1;
}

void m16c_disassembler::format_imm8(std::ostream &stream, u8 imm) const
{
	stream << '#';
	if (imm >= 0xa0)
		stream << '0';
	util::stream_format(stream, "%02Xh", imm);
}

void m16c_disassembler::format_imm16(std::ostream &stream, u16 imm) const
{
	util::stream_format(stream, "#%0*Xh", imm >= 0xa000 ? 5 : 4, imm);
}

void m16c_disassembler::format_label(std::ostream &stream, u32 label) const
{
	label &= 0xfffff;
	util::stream_format(stream, "%0*Xh", label >= 0xa0000 ? 6 : 5, label);
}

void m16c_disassembler::format_imm_signed(std::ostream &stream, s16 imm) const
{
	stream << '#';
	if (imm < 0)
	{
		stream << '-';
		imm = -imm;
	}
	if (u16(imm) > 9)
	{
		int places = 1;
		for (u16 temp = imm; temp > 9; temp >>= 4)
			++places;
		util::stream_format(stream, "%0*Xh", places, u16(imm));
	}
	else
		util::stream_format(stream, "%d", imm);
}

void m16c_disassembler::format_relative(std::ostream &stream, const char *reg, s32 disp) const
{
	if (disp < 0)
	{
		stream << '-';
		disp = -disp;
	}
	if (disp > 9)
	{
		int places = 1;
		for (u32 temp = disp; temp > 9; temp >>= 4)
			++places;
		util::stream_format(stream, "%0*Xh[%s]", places, u32(disp), reg);
	}
	else
		util::stream_format(stream, "%d[%s]", disp, reg);
}

void m16c_disassembler::dasm_ea(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 mode, bool size) const
{
	if (mode < 6)
		stream << s_regs[size][mode];
	else if (mode < 8)
		util::stream_format(stream, "[%s]", s_regs[1][mode - 2]);
	else if (mode < 0xc)
		format_relative(stream, mode < 0xa ? s_regs[1][mode - 4] : s_cregs[mode - 4], mode == 0xb ? s8(opcodes.r8(pc++)) : opcodes.r8(pc++));
	else
	{
		if (mode == 0xf)
			format_label(stream, opcodes.r16(pc));
		else
			format_relative(stream, mode < 0xe ? s_regs[1][mode - 8] : s_cregs[mode - 8], s16(opcodes.r16(pc)));
		pc += 2;
	}
}

void m16c_disassembler::dasm_general(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const
{
	const u8 op2 = opcodes.r8(pc++);
	dasm_ea(stream, pc, opcodes, op2 >> 4, size);
	stream << ", ";
	dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
}

void m16c_disassembler::dasm_quick(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const
{
	const u8 op2 = opcodes.r8(pc++);
	util::stream_format(stream, "#%d, ", s8(op2) >> 4);
	dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
}

void m16c_disassembler::dasm_shift(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const
{
	const u8 op2 = opcodes.r8(pc++);
	util::stream_format(stream, "#%d, ", s8(op2) < 0 ? s8(op2) >> 4 : (op2 >> 4) + 1);
	dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
}

void m16c_disassembler::dasm_74(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const
{
	const u8 op2 = opcodes.r8(pc++);
	switch (op2 >> 4)
	{
	case 0x0: case 0x1:
	{
		util::stream_format(stream, "%-8s", size ? "ste.w" : "ste.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		stream << ", ";
		format_label(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc));
		if (BIT(op2, 4))
			util::stream_format(stream, "[%s]", s_regs[1][4]);
		pc += 3;
		break;
	}

	case 0x2:
		util::stream_format(stream, "%-8s", size ? "ste.w" : "ste.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		util::stream_format(stream, ", [%s%s]", s_regs[1][5], s_regs[1][4]);
		break;

	case 0x3:
		util::stream_format(stream, "%-8s", size ? "mov.w" : "mov.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		stream << ", ";
		format_relative(stream, s_cregs[5], s8(opcodes.r8(pc++)));
		break;

	case 0x4:
		util::stream_format(stream, "%-8s", size ? "push.w" : "push.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0x5:
		util::stream_format(stream, "%-8s", size ? "neg.w" : "neg.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0x6:
		util::stream_format(stream, "%-8s%s, ", size ? "rot.w" : "rot.b", s_regs[0][3]);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0x7:
		util::stream_format(stream, "%-8s", size ? "not.w" : "not.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0x8: case 0x9:
	{
		const int offset = BIT(op2, 3) ? (BIT(op2, 2) ? 2 : 1) : 0;
		util::stream_format(stream, "%-8s", size ? "lde.w" : "lde.b");
		format_label(stream, u32(opcodes.r8(pc + offset + 2)) << 16 | opcodes.r16(pc + offset));
		if (BIT(op2, 4))
			util::stream_format(stream, "[%s]", s_regs[1][4]);
		stream << ", ";
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		pc += 3;
		break;
	}

	case 0xa:
		util::stream_format(stream, "%-8s[%s%s], ", size ? "lde.w" : "lde.b", s_regs[1][5], s_regs[1][4]);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xb:
	{
		const int offset = BIT(op2, 3) ? (BIT(op2, 2) ? 2 : 1) : 0;
		util::stream_format(stream, "%-8s", size ? "mov.w" : "mov.b");
		format_relative(stream, s_cregs[5], s8(opcodes.r8(pc + offset)));
		stream << ", ";
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		++pc;
		break;
	}

	case 0xc:
	{
		const int offset = BIT(op2, 3) ? (BIT(op2, 2) ? 2 : 1) : 0;
		util::stream_format(stream, "%-8s", size ? "mov.w" : "mov.b");
		if (size)
			format_imm16(stream, opcodes.r16(pc + offset));
		else
			format_imm8(stream, opcodes.r8(pc + offset));
		stream << ", ";
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		pc += size ? 2 : 1;
		break;
	}

	case 0xd:
		util::stream_format(stream, "%-8s", size ? "pop.w" : "pop.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xe:
		util::stream_format(stream, "%-8s%s, ", size ? "shl.w" : "shl.b", s_regs[0][3]);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xf:
		util::stream_format(stream, "%-8s%s, ", size ? "sha.w" : "sha.b", s_regs[0][3]);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	default:
		--pc;
		util::stream_format(stream, "%-8s%Xh ; und", ".byte", size ? 0x75 : 0x74);
		break;
	}
}

void m16c_disassembler::dasm_76(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const
{
	const u8 op2 = opcodes.r8(pc++);
	if (op2 < 0x90)
	{
		const int offset = BIT(op2, 3) ? (BIT(op2, 2) ? 2 : 1) : 0;
		util::stream_format(stream, "%-8s", s_imm76_ops[size][op2 >> 4]);
		if (size)
		{
			if (op2 < 0x40)
				format_imm16(stream, opcodes.r16(pc + offset));
			else
				format_imm_signed(stream, s16(opcodes.r16(pc + offset)));
		}
		else
		{
			if (op2 < 0x40)
				format_imm8(stream, opcodes.r8(pc + offset));
			else
				format_imm_signed(stream, s8(opcodes.r8(pc + offset)));
		}
		stream << ", ";
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		pc += size ? 2 : 1;
	}
	else switch (op2 >> 4)
	{
	case 0x9:
		util::stream_format(stream, "%-8s", size ? "divx.w" : "divx.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xa:
		util::stream_format(stream, "%-8s", size ? "rolc.w" : "rolc.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xb:
		util::stream_format(stream, "%-8s", size ? "rorc.w" : "rorc.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xc:
		util::stream_format(stream, "%-8s", size ? "divu.w" : "divu.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xd:
		util::stream_format(stream, "%-8s", size ? "div.w" : "div.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xe:
		util::stream_format(stream, "%-8s", size ? "adcf.w" : "adcf.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	case 0xf:
		util::stream_format(stream, "%-8s", size ? "abs.w" : "abs.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
		break;

	default:
		--pc;
		util::stream_format(stream, "%-8s%Xh ; und", ".byte", size ? 0x77 : 0x76);
		break;
	}
}

void m16c_disassembler::dasm_7a(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const
{
	const u8 op2 = opcodes.r8(pc++);
	if (op2 < 0x40)
	{
		util::stream_format(stream, "%-8s%s, ", size ? "xchg.w" : "xchg.b", s_regs[size][op2 >> 4]);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, size);
	}
	else if (op2 >= 0x90)
	{
		if (size)
			util::stream_format(stream, "%-8s%s, ", "stc", s_cregs[BIT(op2, 4, 3)]);
		else
			util::stream_format(stream, "%-8s", "ldc");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, true);
		if (!size)
			util::stream_format(stream, ", %s", s_cregs[BIT(op2, 4, 3)]);
	}
	else
	{
		--pc;
		util::stream_format(stream, "%-8s%Xh ; und", ".byte", size ? 0x7b : 0x7a);
	}
}

void m16c_disassembler::dasm_7c(std::ostream &stream, offs_t &pc, const data_buffer &opcodes) const
{
	const u8 op2 = opcodes.r8(pc++);
	if (!BIT(op2, 6))
	{
		util::stream_format(stream, "%-8s", s_nibmov_ops[BIT(op2, 4, 2)]);
		if (BIT(op2, 7))
			util::stream_format(stream, "%s, ", s_regs[0][0]);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, false);
		if (!BIT(op2, 7))
			util::stream_format(stream, ", %s", s_regs[0][0]);
	}
	else if (op2 < 0x60)
	{
		const int offset = BIT(op2, 3) ? (BIT(op2, 2) ? 2 : 1) : 0;
		util::stream_format(stream, "%-8s", BIT(op2, 4) ? "mul.b" : "mulu.b");
		format_imm8(stream, opcodes.r8(pc + offset));
		stream << ", ";
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, false);
		++pc;
	}
	else if (op2 < 0x70)
	{
		util::stream_format(stream, "%-8s", "exts.b");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, false);
	}
	else if ((op2 & 0xf0) == 0xc0)
	{
		util::stream_format(stream, "%-8sPC, ", "stc");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, true);
		if ((op2 & 0x0e) < 0x06)
			stream << s_regs[1][(op2 & 0x0e) | BIT(op2, 2) ? 1 : 2];
	}
	else if ((op2 & 0xf4) == 0xe4)
	{
		util::stream_format(stream, "%-8s", s_decimal_ops[0][op2 & 0x03]);
		if (BIT(op2, 3))
			format_imm8(stream, opcodes.r8(pc++));
		else
			stream << s_regs[0][1];
		util::stream_format(stream, ", %s", s_regs[0][0]);
	}
	else switch (op2)
	{
	case 0xe0:
		util::stream_format(stream, "%-8s", "divu.b");
		format_imm8(stream, opcodes.r8(pc++));
		break;

	case 0xe1:
		util::stream_format(stream, "%-8s", "div.b");
		format_imm_signed(stream, s8(opcodes.r8(pc++)));
		break;

	case 0xe2:
		util::stream_format(stream, "%-8s", "push.b");
		format_imm8(stream, opcodes.r8(pc++));
		break;

	case 0xe3:
		util::stream_format(stream, "%-8s", "divx.b");
		format_imm_signed(stream, s8(opcodes.r16(pc++)));
		break;

	case 0xe8:
		stream << "smovf.b";
		break;

	case 0xe9:
		stream << "smovb.b";
		break;

	case 0xea:
		stream << "sstr.b";
		break;

	case 0xeb:
		util::stream_format(stream, "%-8s", "add.b");
		format_imm_signed(stream, s8(opcodes.r8(pc++)));
		util::stream_format(stream, ", %s", s_cregs[5]);
		break;

	case 0xf0:
		util::stream_format(stream, "%-8s", "ldctx");
		format_label(stream, opcodes.r16(pc));
		stream << ", ";
		format_label(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
		pc += 5;
		break;

	case 0xf1:
		stream << "rmpa.b";
		break;

	case 0xf2:
		util::stream_format(stream, "%-8s", "enter");
		format_imm_signed(stream, opcodes.r8(pc++)); // probably not signed
		break;

	case 0xf3:
		util::stream_format(stream, "%-8s%s", "exts.w", s_regs[1][0]);
		break;

	default:
		--pc;
		util::stream_format(stream, "%-8s%Xh ; und", ".byte", 0x7c);
		break;
	}
}

void m16c_disassembler::dasm_7d(std::ostream &stream, offs_t &pc, offs_t &flags, const data_buffer &opcodes) const
{
	const u8 op2 = opcodes.r8(pc++);
	if (op2 < 0x40)
	{
		if (BIT(op2, 4))
		{
			util::stream_format(stream, "%-8s", BIT(op2, 5) ? "jsri.w" : "jsri.a");
			flags |= STEP_OVER;
		}
		else
			util::stream_format(stream, "%-8s", BIT(op2, 5) ? "jmpi.w" : "jmpi.a");
		if ((op2 & 0x0e) == 0x0c)
		{
			format_label(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc));
			util::stream_format(stream, "[%s]", s_regs[1][op2 & 0x07]);
			pc += 3;
		}
		else
		{
			if (!BIT(op2, 5) && (op2 & 0x0e) < 0x06)
				stream << s_regs[1][(op2 & 0x0e) | BIT(op2, 2) ? 1 : 2];
			dasm_ea(stream, pc, opcodes, op2 & 0x0f, true);
		}
	}
	else if (op2 < 0x60)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 4) ? "mul.w" : "mulu.w");
		const int offset = BIT(op2, 3) ? (BIT(op2, 2) ? 2 : 1) : 0;
		if (BIT(op2, 4))
			format_imm_signed(stream, opcodes.r16(pc + offset));
		else
			format_imm16(stream, opcodes.r16(pc + offset));
		stream << ", ";
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, true);
		pc += 2;
	}
	else if ((op2 & 0xf8) == 0x98)
	{
		util::stream_format(stream, "%-8s", "pusha");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, false);
	}
	else if ((op2 & 0xf8) == 0xa0)
		util::stream_format(stream, "%-8s#%d", "ldipl", op2 & 0x07);
	else if ((op2 & 0xf0) == 0xb0)
		util::stream_format(stream, "%-8s#%d, %s", "add.w", util::sext(op2, 4), s_cregs[5]);
	else if ((op2 & 0xf8) == 0xc8 && s_cnds[op2 & 0x0f][0] != '\0')
	{
		util::stream_format(stream, "j%-7s", s_cnds[op2 & 0x0f]);
		format_label(stream, pc + opcodes.r8(pc));
		++pc;
		flags |= STEP_COND;
	}
	else if ((op2 & 0xf0) == 0xd0 && s_cnds[op2 & 0x0f][0] != '\0')
		util::stream_format(stream, "bm%-6sC", s_cnds[op2 & 0x0f]);
	else if ((op2 & 0xf4) == 0xe4)
	{
		util::stream_format(stream, "%-8s", s_decimal_ops[1][op2 & 0x03]);
		if (BIT(op2, 3))
		{
			format_imm16(stream, opcodes.r16(pc));
			pc += 2;
		}
		else
			stream << s_regs[1][1];
		util::stream_format(stream, ", %s", s_regs[1][0]);
	}
	else switch (op2)
	{
	case 0xe0:
		util::stream_format(stream, "%-8s", "divu.w");
		format_imm16(stream, opcodes.r16(pc));
		pc += 2;
		break;

	case 0xe1:
		util::stream_format(stream, "%-8s", "div.w");
		format_imm_signed(stream, s16(opcodes.r16(pc)));
		pc += 2;
		break;

	case 0xe2:
		util::stream_format(stream, "%-8s", "push.w");
		format_imm16(stream, opcodes.r16(pc));
		pc += 2;
		break;

	case 0xe3:
		util::stream_format(stream, "%-8s", "divx.w");
		format_imm_signed(stream, s16(opcodes.r16(pc)));
		pc += 2;
		break;

	case 0xe8:
		stream << "smovf.w";
		break;

	case 0xe9:
		stream << "smovb.w";
		break;

	case 0xea:
		stream << "sstr.w";
		break;

	case 0xeb:
		util::stream_format(stream, "%-8s", "add.w");
		format_imm_signed(stream, s16(opcodes.r16(pc)));
		util::stream_format(stream, ", %s", s_cregs[5]);
		pc += 2;
		break;

	case 0xf0:
		util::stream_format(stream, "%-8s", "stctx");
		format_label(stream, opcodes.r16(pc));
		stream << ", ";
		format_label(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
		pc += 5;
		break;

	case 0xf1:
		stream << "rmpa.w";
		break;

	case 0xf2:
		stream << "exitd";
		flags |= STEP_OUT;
		break;

	case 0xf3:
		stream << "wait";
		break;

	default:
		--pc;
		util::stream_format(stream, "%-8s%Xh ; und", ".byte", 0x7d);
		break;
	}
}

void m16c_disassembler::dasm_7e(std::ostream &stream, offs_t &pc, const data_buffer &opcodes) const
{
	const u8 op2 = opcodes.r8(pc++);
	if (op2 < 0xe0)
	{
		if ((op2 & 0xf0) == 0x20)
		{
			const int offset = (op2 & 0x0e) == 0x06 ? 0 : (op2 & 0x0c) == 0x0c ? 2 : 1;
			const u8 cnd = opcodes.r8(pc + offset);
			if (cnd < 0x08 || (cnd >= 0xf8 && s_cnds[cnd & 0x0f][0] != '\0'))
				util::stream_format(stream, "bm%-6s", s_cnds[cnd & 0x0f]);
			else
			{
				--pc;
				util::stream_format(stream, "%-8s%Xh ; und", ".byte", 0x7e);
				return;
			}
		}
		else
			util::stream_format(stream, "%-8s", s_bit_ops[op2 >> 4]);
		if (!BIT(op2, 3))
		{
			if ((op2 & 0x0f) < 6)
			{
				const u8 pos = opcodes.r8(pc++);
				util::stream_format(stream, "%u, %s", pos, s_regs[1][op2 & 0x0f]);
			}
			else
				util::stream_format(stream, "[%s]", s_regs[1][4 + BIT(op2, 0)]);
		}
		else if (BIT(op2, 2))
		{
			if ((op2 & 0x0f) == 0x0f)
			{
				const u16 abs = opcodes.r16(pc);
				util::stream_format(stream, "%d, ", abs & 0x07);
				format_label(stream, abs >> 3);
			}
			else
				format_relative(stream, BIT(op2, 1) ? s_cregs[6] : s_regs[1][op2 & 0x07], s16(opcodes.r16(pc)));
			pc += 2;
		}
		else if (BIT(op2, 1))
		{
			const u8 dsp8 = opcodes.r8(pc++);
			util::stream_format(stream, "%d, ", dsp8 & 0x07);
			format_relative(stream, s_cregs[6 + BIT(op2, 0)], BIT(op2, 0) ? s8(dsp8) >> 3 : dsp8 >> 3);
		}
		else
			format_relative(stream, s_regs[1][4 + BIT(op2, 0)], opcodes.r8(pc++));
		if ((op2 & 0xf0) == 0x20)
			++pc;
	}
	else
	{
		--pc;
		util::stream_format(stream, "%-8s%Xh ; und", ".byte", 0x7e);
	}
}

void m16c_disassembler::dasm_eb(std::ostream &stream, offs_t &pc, offs_t &flags, const data_buffer &opcodes) const
{
	const u8 op2 = opcodes.r8(pc++);
	if (op2 >= 0xc0)
	{
		util::stream_format(stream, "%-8s", "int");
		format_imm8(stream, op2 & 0x3f);
		flags |= STEP_OVER;
	}
	else if (op2 >= 0x80)
		util::stream_format(stream, "%-8s#%d, %s%s", BIT(op2, 5) ? "sha.l" : "shl.l",
							BIT(op2, 3) ? util::sext(op2, 4) : (op2 & 0x0f) + 1,
							s_regs[1][2 + BIT(op2, 4)], s_regs[1][BIT(op2, 4)]);
	else if ((op2 & 0x4f) == 0x01)
		util::stream_format(stream, "%-8s%s, %s%s", BIT(op2, 5) ? "sha.l" : "shl.l",
							s_regs[0][3], s_regs[1][2 + BIT(op2, 4)], s_regs[1][BIT(op2, 4)]);
	else if ((op2 & 0x0e) == 0x04)
		util::stream_format(stream, "%-8s%c", BIT(op2, 0) ? "fclr" : "fset", "CDZSBOIU"[op2 >> 4]);
	else if (op2 < 0x60 && BIT(op2, 3))
	{
		util::stream_format(stream, "%-8s", "mova");
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, false);
		util::stream_format(stream, ", %s", s_regs[1][op2 >> 4]);
	}
	else if ((op2 & 0x0f) == 0 && op2 >= 0x10)
	{
		util::stream_format(stream, "%-8s", "ldc");
		format_imm16(stream, opcodes.r16(pc));
		util::stream_format(stream, ", %s", s_cregs[op2 >> 4]);
		pc += 2;
	}
	else if ((op2 & 0x0e) == 0x02 && op2 >= 0x10)
		util::stream_format(stream, "%-8s%s", BIT(op2, 0) ? "popc" : "pushc", s_cregs[op2 >> 4]);
	else
	{
		--pc;
		util::stream_format(stream, "%-8s0%Xh ; und", ".byte", 0xeb);
	}
}

offs_t m16c_disassembler::disassemble(std::ostream &stream, offs_t pc, const m16c_disassembler::data_buffer &opcodes, const m16c_disassembler::data_buffer &params)
{
	const offs_t pc0 = pc;
	offs_t flags = SUPPORTED;

	const u8 op1 = opcodes.r8(pc++);
	if (op1 < 0x40)
	{
		if (op1 == 0x00)
		{
			stream << "brk";
			flags |= STEP_OVER;
		}
		else if (op1 == 0x04)
			stream << "nop";
		else
		{
			util::stream_format(stream, "%-8s", s_byte_ops[BIT(op1, 3, 3)]);
			if (op1 < 0x08)
				util::stream_format(stream, "%s, ", s_regs[0][BIT(op1, 2)]);
			switch (op1 & 0x03)
			{
			case 0:
				util::stream_format(stream, "%s", s_regs[0][BIT(~op1, 2)]);
				break;

			case 3:
				format_label(stream, opcodes.r16(pc));
				pc += 2;
				break;

			default:
				format_relative(stream, s_cregs[5 + (op1 & 3)], BIT(op1, 1) ? s8(opcodes.r8(pc++)): opcodes.r8(pc++));
				break;
			}
			if (op1 >= 0x08)
				util::stream_format(stream, ", %s", s_regs[0][(BIT(op1, 3, 3) == 6 ? 4 : 0) + BIT(op1, 2)]);
		}
	}
	else if (op1 < 0x60)
	{
		util::stream_format(stream, "%-8s%d, ", s_bit_ops[op1 >> 3], op1 & 0x07);
		format_relative(stream, s_cregs[6], opcodes.r8(pc++));
	}
	else if (op1 < 0x68)
	{
		util::stream_format(stream, "%-8s", "jmp.s");
		format_label(stream, pc + 1 + (op1 & 0x07));
	}
	else if (op1 < 0x70)
	{
		const s8 disp = s8(opcodes.r8(pc++));
		util::stream_format(stream, "b%-7s", s_cnds[op1 & 0x07]);
		format_label(stream, pc + disp - 1);
		flags |= STEP_COND;
	}
	else switch (op1)
	{
	case 0x70: case 0x71:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "mulu.w" : "mulu.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x72: case 0x73:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "mov.w" : "mov.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x74: case 0x75:
		dasm_74(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x76: case 0x77:
		dasm_76(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x78: case 0x79:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "mul.w" : "mul.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x7a: case 0x7b:
		dasm_7a(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x7c:
		dasm_7c(stream, pc, opcodes);
		break;

	case 0x7d:
		dasm_7d(stream, pc, flags, opcodes);
		break;

	case 0x7e:
		dasm_7e(stream, pc, opcodes);
		break;

	case 0x80: case 0x81:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "tst.w" : "tst.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x82: case 0x8a: case 0x92: case 0x9a:
		util::stream_format(stream, "%-8s%s", BIT(op1, 4) ? "pop.b" : "push.b", s_regs[0][BIT(op1, 3)]);
		break;

	case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		util::stream_format(stream, "%-8s", BIT(op1, 3) ? "sub.b" : "add.b");
		format_imm_signed(stream, opcodes.r8(pc++));
		stream << ", ";
		switch (op1 & 0x07)
		{
		case 3: case 4:
			stream << s_regs[0][BIT(op1, 0)];
			break;

		case 5:
			format_relative(stream, s_cregs[6], opcodes.r8(pc++));
			break;

		case 6:
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc++)));
			break;

		case 7:
			format_label(stream, opcodes.r16(pc));
			pc += 2;
			break;
		}
		break;

	case 0x88: case 0x89:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "xor.w" : "xor.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x90: case 0x91:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "and.w" : "and.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		util::stream_format(stream, "%-8s", BIT(op1, 3) ? "or.b" : "and.b");
		format_imm8(stream, opcodes.r8(pc++));
		stream << ", ";
		switch (op1 & 0x07)
		{
		case 3: case 4:
			stream << s_regs[0][BIT(op1, 0)];
			break;

		case 5:
			format_relative(stream, s_cregs[6], opcodes.r8(pc++));
			break;

		case 6:
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc++)));
			break;

		case 7:
			format_label(stream, opcodes.r16(pc));
			pc += 2;
			break;
		}
		break;

	case 0x98: case 0x99:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "or.w" : "or.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xa0: case 0xa1:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "add.w" : "add.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xa2: case 0xaa:
		util::stream_format(stream, "%-8s", "mov.w");
		format_imm16(stream, opcodes.r16(pc));
		pc += 2;
		util::stream_format(stream, ", %s", s_regs[1][4 + BIT(op1, 3)]);
		break;

	case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		util::stream_format(stream, "%-8s", BIT(op1, 3) ? "dec.b" : "inc.b");
		switch (op1 & 0x07)
		{
		case 3: case 4:
			stream << s_regs[0][BIT(op1, 0)];
			break;

		case 5:
			format_relative(stream, s_cregs[6], opcodes.r8(pc++));
			break;

		case 6:
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc++)));
			break;

		case 7:
			format_label(stream, opcodes.r16(pc));
			pc += 2;
			break;
		}
		break;

	case 0xa8: case 0xa9:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "sub.w" : "sub.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xb0: case 0xb1:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "adc.w" : "adc.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xb2: case 0xba: case 0xf2: case 0xfa:
		util::stream_format(stream, "%-8s%s", BIT(op1, 6) ? "dec.w" : "inc.w", s_regs[1][4 + BIT(op1, 3)]);
		break;

	case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		util::stream_format(stream, "%-8s", op1 < 0xc8 ? "mov.b" : op1 < 0xd0 ? "stz" : "stnz");
		if (BIT(op1, 6))
			format_imm8(stream, opcodes.r8(pc++));
		else
			stream << "#0";
		stream << ", ";
		switch (op1 & 0x07)
		{
		case 3: case 4:
			stream << s_regs[0][BIT(op1, 0)];
			break;

		case 5:
			format_relative(stream, s_cregs[6], opcodes.r8(pc++));
			break;

		case 6:
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc++)));
			break;

		case 7:
			format_label(stream, opcodes.r16(pc));
			pc += 2;
			break;
		}
		break;

	case 0xb8: case 0xb9:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "sbb.w" : "sbb.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		util::stream_format(stream, "%-8s", "not.b");
		switch (op1 & 0x07)
		{
		case 3: case 4:
			stream << s_regs[0][BIT(op1, 0)];
			break;

		case 5:
			format_relative(stream, s_cregs[6], opcodes.r8(pc++));
			break;

		case 6:
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc++)));
			break;

		case 7:
			format_label(stream, opcodes.r16(pc));
			pc += 2;
			break;
		}
		break;

	case 0xc0: case 0xc1:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "cmp.w" : "cmp.b");
		dasm_general(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xc2: case 0xca: case 0xd2: case 0xda:
		util::stream_format(stream, "%-8s%s", BIT(op1, 4) ? "pop.w" : "push.w", s_regs[1][4 + BIT(op1, 3)]);
		break;

	case 0xc8: case 0xc9:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "add.w" : "add.b");
		dasm_quick(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xd0: case 0xd1:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "cmp.w" : "cmp.b");
		dasm_quick(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xd8: case 0xd9:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "mov.w" : "mov.b");
		dasm_quick(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		util::stream_format(stream, "%-8s", "stzx");
		format_imm8(stream, opcodes.r8(pc++));
		stream << ", ";
		switch (op1 & 0x07)
		{
		case 3: case 4:
			format_imm8(stream, opcodes.r8(pc++));
			util::stream_format(stream, ", %s", s_regs[0][BIT(op1, 0)]);
			break;

		case 5:
			format_imm8(stream, opcodes.r8(pc + 1));
			stream << ", ";
			format_relative(stream, s_cregs[6], opcodes.r8(pc));
			pc += 2;
			break;

		case 6:
			format_imm8(stream, opcodes.r8(pc + 1));
			stream << ", ";
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc)));
			pc += 2;
			break;

		case 7:
			format_imm8(stream, opcodes.r8(pc + 2));
			stream << ", ";
			format_label(stream, opcodes.r16(pc));
			pc += 3;
			break;
		}
		break;

	case 0xe0: case 0xe1:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "rot.w" : "rot.b");
		dasm_shift(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xe2: case 0xea:
		util::stream_format(stream, "%-8s", "mov.b");
		format_imm8(stream, opcodes.r8(pc++));
		util::stream_format(stream, ", %s", s_regs[1][4 + BIT(op1, 3)]);
		break;

	case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		util::stream_format(stream, "%-8s", "cmp.b");
		format_imm_signed(stream, s8(opcodes.r8(pc++)));
		stream << ", ";
		switch (op1 & 0x07)
		{
		case 3: case 4:
			stream << s_regs[0][BIT(op1, 0)];
			break;

		case 5:
			format_relative(stream, s_cregs[6], opcodes.r8(pc++));
			break;

		case 6:
			format_relative(stream, s_cregs[7], s8(opcodes.r8(pc++)));
			break;

		case 7:
			format_label(stream, opcodes.r16(pc));
			pc += 2;
			break;
		}
		break;

	case 0xe8: case 0xe9:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "shl.w" : "shl.b");
		dasm_shift(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xeb:
		dasm_eb(stream, pc, flags, opcodes);
		break;

	case 0xec:
	{
		const u8 op2 = opcodes.r8(pc++);
		util::stream_format(stream, "%-8s", "pushm");
		for (int i = 0; i < 8; i++)
		{
			if (BIT(op2, i))
			{
				stream << (i < 2 ? s_cregs[7 - i] : s_regs[1][7 - i]);
				if ((op2 >> i) != 1)
					stream << ", ";
			}
		}
		break;
	}

	case 0xed:
	{
		const u8 op2 = opcodes.r8(pc++);
		util::stream_format(stream, "%-8s", "popm");
		for (int i = 0; i < 8; i++)
		{
			if (BIT(op2, i))
			{
				stream << (i > 6 ? s_cregs[i] : s_regs[1][i]);
				if ((op2 >> i) != 1)
					stream << ", ";
			}
		}
		break;
	}

	case 0xee:
		util::stream_format(stream, "%-8s", "jmps");
		format_imm8(stream, opcodes.r8(pc++));
		break;

	case 0xef:
		util::stream_format(stream, "%-8s", "jsrs");
		format_imm8(stream, opcodes.r8(pc++));
		flags |= STEP_OVER;
		break;

	case 0xf0: case 0xf1:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "sha.w" : "sha.b");
		dasm_shift(stream, pc, opcodes, BIT(op1, 0));
		break;

	case 0xf3:
		stream << "rts";
		flags |= STEP_OUT;
		break;

	case 0xf4: case 0xf5:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "jsr.w" : "jmp.w");
		format_label(stream, pc + s16(opcodes.r16(pc)));
		pc += 2;
		if (BIT(op1, 0))
			flags |= STEP_COND;
		break;

	case 0xf6:
		stream << "into";
		flags |= STEP_OVER | STEP_COND;
		break;

	case 0xf8: case 0xf9:
	{
		const u8 op2 = opcodes.r8(pc++);
		if (BIT(op2, 3))
			util::stream_format(stream, "%-8s#%d, ", BIT(op1, 0) ? "adjnz.w" : "adjnz.b", 8 - (op2 & 0x07));
		else
			util::stream_format(stream, "%-8s#%d, ", BIT(op1, 0) ? "sbjnz.w" : "sbjnz.b", op2 & 0x07);
		dasm_ea(stream, pc, opcodes, op2 & 0x0f, BIT(op1, 0));
		format_label(stream, pc0 + 2 + s8(opcodes.r8(pc)));
		++pc;
		flags |= STEP_COND;
		break;
	}

	case 0xfb:
		stream << "reit";
		flags |= STEP_OUT;
		break;

	case 0xfc: case 0xfd:
		util::stream_format(stream, "%-8s", BIT(op1, 0) ? "jsr.a" : "jmp.a");
		format_label(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc));
		pc += 3;
		if (BIT(op1, 0))
			flags |= STEP_COND;
		break;

	case 0xfe:
		util::stream_format(stream, "%-8s", "jmp.b");
		format_label(stream, pc + s8(opcodes.r8(pc)));
		++pc;
		break;

	case 0xff:
	default:
		util::stream_format(stream, "%-8s%0*Xh ; und", ".byte", op1 >= 0xa0 ? 3 : 2, op1);
		break;
	}

	return (pc - pc0) | flags;
}
