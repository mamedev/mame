// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H16 (HD641016) disassembler

***************************************************************************/

#include "emu.h"
#include "h16dasm.h"

h16_disassembler::h16_disassembler()
	: util::disasm_interface()
{
}

u32 h16_disassembler::opcode_alignment() const
{
	return 1;
}

const char *const h16_disassembler::s_conditions[16] = {
	"T", "F",
	"HI", "LS",
	"CC", "CS",
	"NE", "EQ",
	"VC", "VS",
	"PL", "MI",
	"GE", "LT",
	"GT", "LE"
};

const char *const h16_disassembler::s_term_conditions[16] = {
	"T", "F",
	"HI", "LS",
	"HS", "LO",
	"NE", "EQ",
	"VC", "VS",
	"PL", "MI",
	"GE", "LT",
	"GT", "LE"
};

const char *const h16_disassembler::s_basic_ops[4] = {
	"ADD", "SUB", "CMP", "MOV"
};

const char *const h16_disassembler::s_logical_ops[3] = {
	"AND", "XOR", "OR"
};

const char *const h16_disassembler::s_sft_ops[8] = {
	"SHAR", "SHLR", "ROTR", "ROTXR",
	"SHAL", "SHLL", "ROTL", "ROTXL"
};

const char *const h16_disassembler::s_bit_ops[4] = {
	"BSET", "BNOT", "BCLR", "BTST"
};

const char *const h16_disassembler::s_bf_ops[3] = {
	"BFEXT", "BFINS", "BFSCH"
};

const char *const h16_disassembler::s_cr_ops[5] = {
	"ANDC", "ORC", "XORC", "LDC", "STC"
};

void h16_disassembler::format_register(std::ostream &stream, char bank, u8 n) const
{
	if (bank != 0)
		util::stream_format(stream, "%cR%d", bank, n);
	else if (n == 15)
		stream << "SP";
	else
		util::stream_format(stream, "R%d", n);
}

void h16_disassembler::format_register_to_register(std::ostream &stream, u8 rr) const
{
	format_register(stream, 0, rr >> 4);
	stream << ", ";
	format_register(stream, 0, rr & 0x0f);
}

void h16_disassembler::format_register_list(std::ostream &stream, u16 rl) const
{
	stream << "[";
	bool first = false;
	for (int n = 0; n < 16; n++)
	{
		if (BIT(rl, n))
		{
			if (first)
				stream << ",";
			else
				first = true;
			int m = n + 1;
			while (m < 16 && BIT(rl, m))
				m++;
			m--;
			if (m > n)
			{
				util::stream_format(stream, "R%d-R%d", n, m);
				n = m;
			}
			else
				util::stream_format(stream, "R%d", n);
		}
	}
	stream << "]";
}

u8 h16_disassembler::get_cr_size(u8 cr) const
{
	switch (cr)
	{
	case 0x01: case 0x80: case 0x81:
		return 0;

	case 0x20: case 0xa0:
		return 1;

	default:
		return 2;
	}
}

void h16_disassembler::format_cr(std::ostream &stream, u8 cr) const
{
	switch (cr)
	{
	case 0x01:
		stream << "VBNR";
		break;

	case 0x20:
		stream << "CCR";
		break;

	case 0x40:
		stream << "CBNR";
		break;

	case 0x41:
		stream << "BSP";
		break;

	case 0x80:
		stream << "BMR";
		break;

	case 0x81:
		stream << "GBNR";
		break;

	case 0xa0:
		stream << "SR";
		break;

	case 0xc0:
		stream << "EBR";
		break;

	case 0xc1:
		stream << "RBR";
		break;

	case 0xc2:
		stream << "USP";
		break;

	case 0xc3:
		stream << "IBR";
		break;

	default:
		stream << "<unknown CR>";
		break;
	}
}

void h16_disassembler::format_s8(std::ostream &stream, u8 data) const
{
	if (s8(data) < 0)
	{
		stream << "-";
		data = 0x100 - data;
	}
	if (data >= 0x0a)
		stream << "H'";
	util::stream_format(stream, "%X", data);
}

void h16_disassembler::format_s16(std::ostream &stream, u16 data) const
{
	if (s16(data) < 0)
	{
		stream << "-";
		data = 0x10000 - data;
	}
	if (data >= 0x000a)
		stream << "H'";
	util::stream_format(stream, "%X", data);
}

void h16_disassembler::format_s32(std::ostream &stream, u32 data) const
{
	if (s32(data) < 0)
	{
		stream << "-";
		data = u32(-data);
	}
	if (data >= 0x0000000a)
		stream << "H'";
	util::stream_format(stream, "%X", data);
}

void h16_disassembler::dasm_displacement(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, u8 sd) const
{
	switch (sd)
	{
	case 0:
		// No displacement
		break;

	case 1:
	{
		// 8-bit displacement
		format_s8(stream, opcodes.r8(pc++));
		stream << ", ";
		break;
	}

	case 2:
	{
		// 16-bit displacement
		u16 disp = opcodes.r16(pc);
		pc += 2;
		format_s16(stream, disp);
		if (disp < 0x0080 || disp >= 0xff80)
			stream << ":16";
		stream << ", ";
		break;
	}

	case 3:
	{
		// 32-bit displacement
		u32 disp = opcodes.r32(pc);
		pc += 4;
		format_s32(stream, disp);
		if (disp < 0x00008000 || disp >= 0xffff8000U)
			stream << ":32";
		stream << ", ";
		break;
	}
	}
}

void h16_disassembler::dasm_ea(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, u8 ea, char bank, h16_disassembler::ea_mode mode, u8 sz) const
{
	if (ea < 0x10)
	{
		// Register indirect
		stream << "@";
		format_register(stream, bank, ea);
	}
	else if (ea < 0x40)
	{
		// Register indirect (with displacement)
		stream << "@(";
		dasm_displacement(stream, pc, opcodes, ea >> 4);
		format_register(stream, bank, ea & 0x0f);
		stream << ")";
	}
	else if (ea < 0x50)
	{
		// Register direct
		if (mode != ea_mode::ADDRESS && mode != ea_mode::MEMORY && mode != ea_mode::PLAIN_MEMORY)
			format_register(stream, bank, ea & 0x0f);
		else
			stream << "<RIE>";
	}
	else if (ea < 0x70 && mode != ea_mode::PLAIN_MEMORY && mode != ea_mode::BITFIELD)
	{
		// Register indirect auto-increment/auto-decrement
		stream << "@";
		if (BIT(ea, 5))
			stream << "-";
		format_register(stream, bank, ea & 0x0f);
		if (BIT(ea, 4))
			stream << "+";
	}
	else switch (ea)
	{
	case 0x70:
		// Current bank
		ea = opcodes.r8(pc++);
		dasm_ea(stream, pc, opcodes, ea & 0x7f, 'C', mode, sz);
		break;

	case 0x71:
		// Immediate (Imm8)
		switch (mode)
		{
		case ea_mode::SIGNED:
			stream << "#";
			format_s8(stream, opcodes.r8(pc++));
			if (sz != 0)
				stream << ".B";
			break;

		case ea_mode::UNSIGNED:
			stream << "#";
			if (sz == 0)
				util::stream_format(stream, "H'%02X", opcodes.r8(pc++));
			else
			{
				u8 imm = opcodes.r8(pc++);
				if (imm >= 0x0a)
					stream << "H'";
				util::stream_format(stream, "%X.B", imm);
			}
			break;

		default:
			stream << "<RIE>";
			break;
		}
		break;

	case 0x72:
		// Immediate (Imm16)
		switch (mode)
		{
		case ea_mode::SIGNED:
			stream << "#";
			format_s16(stream, opcodes.r16(pc));
			pc += 2;
			if (sz != 1)
				stream << ".W";
			break;

		case ea_mode::UNSIGNED:
			stream << "#";
			if (sz == 1)
				util::stream_format(stream, "H'%04X", opcodes.r16(pc));
			else
			{
				u16 imm = opcodes.r16(pc);
				if (imm >= 0x000a)
					stream << "H'";
				util::stream_format(stream, "%X.W", imm);
			}
			pc += 2;
			break;

		default:
			stream << "<RIE>";
			break;
		}
		break;

	case 0x73:
		// Immediate (Imm32)
		switch (mode)
		{
		case ea_mode::SIGNED:
			stream << "#";
			format_s32(stream, opcodes.r32(pc));
			pc += 4;
			if (sz != 2)
				stream << ".L";
			break;

		case ea_mode::UNSIGNED:
			stream << "#";
			if (sz == 2)
				util::stream_format(stream, "H'%08X", opcodes.r32(pc));
			else
			{
				u32 imm = opcodes.r32(pc);
				if (imm >= 0x0000000a)
					stream << "H'";
				util::stream_format(stream, "%X.L", imm);
			}
			pc += 4;
			break;

		default:
			stream << "<RIE>";
			break;
		}
		break;

	case 0x74:
		// Previous bank
		ea = opcodes.r8(pc++);
		dasm_ea(stream, pc, opcodes, ea & 0x7f, 'P', mode, sz);
		break;

	case 0x75:
	{
		// Absolute address (Abs8)
		u8 aa = opcodes.r8(pc++);
		if (aa < 0x80)
			util::stream_format(stream, "@H'%02X", aa);
		else
			util::stream_format(stream, "@H'%X", aa + (mode != ea_mode::ADDRESS ? 0xffff00U : 0xffffff00U));
		break;
	}

	case 0x76:
	{
		// Absolute address (Abs16)
		u16 aa = opcodes.r16(pc);
		pc += 2;
		if (aa < 0x80)
			util::stream_format(stream, "@H'%X.W", aa);
		else if (aa < 0x8000)
			util::stream_format(stream, "@H'%04X", aa);
		else
		{
			util::stream_format(stream, "@H'%X", aa + (mode != ea_mode::ADDRESS ? 0xff0000U : 0xffff0000U));
			if (aa >= 0xff80)
				stream << ".W";
		}
		break;
	}

	case 0x77:
	{
		// Absolute address (Abs32)
		u32 aa = opcodes.r32(pc);
		pc += 4;
		if (aa < 0x8000)
			util::stream_format(stream, "@H'%X.L", aa);
		else if (mode != ea_mode::ADDRESS)
		{
			aa &= 0xffffff;
			util::stream_format(stream, "@H'%06X", aa);
			if (aa >= 0xff8000)
				stream << ".L";
		}
		else
		{
			util::stream_format(stream, "@H'%08X", aa);
			if (aa >= 0xffff8000U)
				stream << ".L";
		}
		break;
	}

	case 0x78:
	{
		// Register indirect with scale
		u8 xb = opcodes.r8(pc++);
		stream << "@";
		format_register(stream, bank, xb & 0x0f);
		util::stream_format(stream, "*%d", 1 << BIT(xb, 4, 2));
		break;
	}

	case 0x79: case 0x7a: case 0x7b:
	{
		// Register indirect with scale (and displacement)
		u8 xb = opcodes.r8(pc++);
		stream << "@(";
		dasm_displacement(stream, pc, opcodes, ea & 0x03);
		format_register(stream, bank, xb & 0x0f);
		util::stream_format(stream, "*%d)", 1 << BIT(xb, 4, 2));
		break;
	}

	case 0x7c:
	{
		u8 xb1 = opcodes.r8(pc++);
		if (xb1 < 0x80)
		{
			// Register indirect with index
			u8 xb2 = opcodes.r8(pc++);
			stream << "@(";
			dasm_displacement(stream, pc, opcodes, BIT(xb1, 4, 2));
			format_register(stream, 0, xb2 & 0x0f);
			util::stream_format(stream, ".%c", BIT(xb1, 6) ? 'L' : 'W');
			if ((xb2 & 0x30) != 0)
				util::stream_format(stream, "*%d", 1 << BIT(xb2, 4, 2));
			stream << ", ";
			format_register(stream, bank, xb1 & 0x0f);
			stream << ")";
		}
		else
			stream << "<RIE>";
		break;
	}

	case 0x7d:
	{
		u8 xb1 = opcodes.r8(pc++);
		if (xb1 < 0x80)
		{
			// Program counter relative with index
			u8 xb2 = opcodes.r8(pc++);
			stream << "@(";
			dasm_displacement(stream, pc, opcodes, BIT(xb1, 4, 2));
			format_register(stream, bank, xb2 & 0x0f);
			util::stream_format(stream, ".%c", BIT(xb1, 6) ? 'L' : 'W');
			if ((xb2 & 0x30) != 0)
				util::stream_format(stream, "*%d", 1 << BIT(xb2, 4, 2));
			stream << ", PC)";
		}
		else if (xb1 < 0x90)
			stream << "@PC";
		else if (xb1 < 0xc0)
		{
			// Program counter relative
			stream << "@(";
			dasm_displacement(stream, pc, opcodes, BIT(xb1, 4, 2));
			stream << "PC)";
		}
		else
			stream << "<RIE>";
		break;
	}

	case 0x7e:
	{
		// Register double indirect (only 8-bit and 32-bit displacements allowed)
		u8 xb = opcodes.r8(pc++);
		if ((xb & 0x50) == 0x50)
		{
			stream << "@";
			u32 ds1 = BIT(xb, 5) ? opcodes.r32(std::exchange(pc, pc + 4)) : s32(s8(opcodes.r8(pc++)));
			u32 ds2 = BIT(xb, 7) ? opcodes.r32(std::exchange(pc, pc + 4)) : s32(s8(opcodes.r8(pc++)));
			if (ds2 != 0)
			{
				stream << "(";
				format_s32(stream, ds2);
				if (BIT(xb, 7) && (ds2 < 0x00000080 || ds2 >= 0xffffff80U))
					stream << ":32";
				stream << ", ";
			}
			stream << "@";
			if (ds1 != 0)
			{
				stream << "(";
				format_s32(stream, ds1);
				if (BIT(xb, 5) && (ds1 < 0x00000080 || ds1 >= 0xffffff80U))
					stream << ":32";
				stream << ", ";
			}
			format_register(stream, bank, xb & 0x0f);
			if (ds1 != 0)
				stream << ")";
			if (ds2 != 0)
				stream << ")";
		}
		else
			stream << "<RIE>";
		break;
	}

	default:
		// Not used
		stream << "<RIE>";
		break;
	}
}

void h16_disassembler::dasm_eas_ead(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, h16_disassembler::ea_mode smode, h16_disassembler::ea_mode dmode, u8 sz) const
{
	dasm_eas_ead(stream, pc, opcodes, smode, dmode, sz, sz);
}

void h16_disassembler::dasm_eas_ead(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, h16_disassembler::ea_mode smode, h16_disassembler::ea_mode dmode, u8 szs, u8 szd) const
{
	const offs_t pcs = pc;
	u8 eas = opcodes.r8(pc++);
	dasm_ea(stream, pc, opcodes, eas & 0x7f, 0, smode, szs);
	stream << ", ";
	u8 ead = BIT(eas, 7) ? 0x40 : opcodes.r8(pc++);
	dasm_ea(stream, pc, opcodes, ead & 0x7f, 0, dmode, szd);

	// Reinterpret PC-relative source offsets by working backwards
	if ((eas & 0x7f) == 0x7d)
	{
		u8 xb1 = opcodes.r8(pcs + 1);
		const offs_t disppc = pcs + (BIT(xb1, 7) ? 2 : 3);
		switch (xb1 & 0xf0)
		{
		case 0x00: case 0x40: case 0x80:
			util::stream_format(stream, " ; H'%06X", disppc & 0xffffff);
			break;

		case 0x10: case 0x50: case 0x90:
			util::stream_format(stream, " ; H'%06X", (disppc + 1 + s8(opcodes.r8(disppc))) & 0xffffff);
			break;

		case 0x20: case 0x60: case 0xa0:
			util::stream_format(stream, " ; H'%06X", (disppc + 2 + s16(opcodes.r16(disppc))) & 0xffffff);
			break;

		case 0x30: case 0x70: case 0xb0:
			util::stream_format(stream, " ; H'%06X", (disppc + 4 + opcodes.r32(disppc)) & 0xffffff);
			break;
		}
	}
}

void h16_disassembler::dasm_branch_disp(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, u8 sz) const
{
	switch (sz)
	{
	case 0:
	{
		u8 disp = opcodes.r8(pc++);
		util::stream_format(stream, "H'%06X", (pc + s8(disp)) & 0xffffff);
		break;
	}

	case 1:
	{
		u16 disp = opcodes.r16(pc);
		pc += 2;
		util::stream_format(stream, "H'%06X", (pc + s16(disp)) & 0xffffff);
		break;
	}

	case 2:
	{
		u32 disp = opcodes.r32(pc);
		pc += 4;
		util::stream_format(stream, "H'%06X", (pc + disp) & 0xffffff);
		break;
	}
	}
}

offs_t h16_disassembler::disassemble(std::ostream &stream, offs_t pc, const h16_disassembler::data_buffer &opcodes, const h16_disassembler::data_buffer &params)
{
	const offs_t pc0 = pc;
	offs_t flags = SUPPORTED;
	u8 op = opcodes.r8(pc++);
	switch (op)
	{
	case 0x00: case 0x01: case 0x02:
	case 0x04: case 0x05: case 0x06:
	case 0x08: case 0x09: case 0x0a:
	case 0x0c: case 0x0d: case 0x0e:
		util::stream_format(stream, "%-9s", string_format("%s.%c", s_basic_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		dasm_eas_ead(stream, pc, opcodes,
						(op & 0x0c) == 0x0c ? ea_mode::UNSIGNED : ea_mode::SIGNED,
						(op & 0x0c) == 0x08 ? ea_mode::SIGNED : ea_mode::DESTINATION,
						op & 0x03);
		break;

	case 0x10: case 0x11: case 0x12:
	case 0x18: case 0x19: case 0x1a:
	case 0x1c: case 0x1d: case 0x1e:
		util::stream_format(stream, "%-9s#", string_format("%s.%c", s_basic_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		if (op == 0x1c)
			util::stream_format(stream, "H'%02X", opcodes.r8(pc++));
		else
			format_s8(stream, opcodes.r8(pc++));
		stream << ", ";
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, (op & 0x0c) == 0x08 ? ea_mode::SIGNED : ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x14: case 0x15: case 0x16:
		util::stream_format(stream, "%-9s", string_format("CLR.%c", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x20: case 0x21: case 0x22:
	case 0x24: case 0x25: case 0x26:
	case 0x28: case 0x29: case 0x2a:
	case 0x2c: case 0x2d: case 0x2e:
		util::stream_format(stream, "%-9s", string_format("%s.%c", s_basic_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		format_register_to_register(stream, opcodes.r8(pc++));
		break;

	case 0x30: case 0x31: case 0x32:
	case 0x34: case 0x35: case 0x36:
	case 0x38: case 0x39: case 0x3a:
	case 0x3c: case 0x3d: case 0x3e:
	{
		u8 xb = opcodes.r8(pc++);
		util::stream_format(stream, "%-9s", string_format("%s.%c", s_basic_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		util::stream_format(stream, "#%d, ", !BIT(op, 3) || !BIT(xb, 3) ? xb & 0x0f : int(xb & 0x0f) - 16);
		format_register(stream, 0, xb >> 4);
		break;
	}

	case 0x40: case 0x41: case 0x42:
	case 0x44: case 0x45: case 0x46:
	case 0x48: case 0x49: case 0x4a:
	case 0x4c: case 0x4d: case 0x4e:
		util::stream_format(stream, "%-9s", string_format("%sS.%c", s_basic_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::SIGNED, ea_mode::DESTINATION, op & 0x03, 2);
		break;

	case 0x50: case 0x51: case 0x52:
	case 0x54: case 0x55: case 0x56:
		util::stream_format(stream, "%-9s", string_format("%sX.%c", s_basic_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::UNSIGNED, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x58: case 0x59: case 0x5a:
		util::stream_format(stream, "%-9s", string_format("TST.%c", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::SIGNED, op & 0x03);
		break;

	case 0x5c: case 0x5d: case 0x5e:
		util::stream_format(stream, "%-9s", string_format("MOVF.%c", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x60: case 0x61: case 0x62:
	case 0x64: case 0x65: case 0x66:
	case 0x68: case 0x69: case 0x6a:
	case 0x6c: case 0x6d: case 0x6e:
	{
		u8 xb = opcodes.r8(pc++);
		if (BIT(op, 3))
			util::stream_format(stream, "%-9s", string_format("%s.%c", s_bit_ops[BIT(xb, 5, 2)], "BWL"[op & 0x03]));
		else
			util::stream_format(stream, "%-9s", string_format("%s.%c", s_sft_ops[BIT(xb, 5, 3)], "BWL"[op & 0x03]));
		if (BIT(op, 2))
			util::stream_format(stream, "#%d", xb & 0x1f);
		else
			format_register(stream, 0, xb & 0x0f);
		stream << ", ";
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, op & 0x03);
		break;
	}

	case 0x70: case 0x71: case 0x72:
	{
		util::stream_format(stream, "%-9s", string_format("STM.%c", "BWL"[op & 0x03]));
		std::ostringstream eadbuf;
		dasm_ea(eadbuf, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::MEMORY, op & 0x03);
		format_register_list(stream, opcodes.r16(pc));
		pc += 2;
		util::stream_format(stream, ", %s", eadbuf.str());
		break;
	}

	case 0x74: case 0x75: case 0x76:
		util::stream_format(stream, "%-9s", string_format("LDM.%c", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::MEMORY, op & 0x03);
		stream << ", ";
		format_register_list(stream, opcodes.r16(pc));
		pc += 2;
		break;

	case 0x78: case 0x79: case 0x7a:
		util::stream_format(stream, "%-9s", string_format("MOVTPE.%c", "BWL"[op & 0x03]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::UNSIGNED, ea_mode::PLAIN_MEMORY, op & 0x03);
		break;

	case 0x7c: case 0x7d: case 0x7e:
		util::stream_format(stream, "%-9s", string_format("MOVFPE.%c", "BWL"[op & 0x03]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::PLAIN_MEMORY, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x80: case 0x81: case 0x82:
	case 0x84: case 0x85: case 0x86:
	case 0x88: case 0x89: case 0x8a:
		util::stream_format(stream, "%-9s", string_format("%s.%c", s_logical_ops[BIT(op, 2, 2)], "BWL"[op & 0x03]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::UNSIGNED, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x8c: case 0x8d: case 0x8e:
	case 0x9c: case 0x9d: case 0x9e:
		util::stream_format(stream, "%-9s", string_format("NEG%s.%c", BIT(op, 4) ? "X" : "", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x90: case 0x91: case 0x92:
		util::stream_format(stream, "%-9s", string_format("NOT.%c", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, op & 0x03);
		break;

	case 0x94: case 0x95:
	{
		u8 xb1 = opcodes.r8(pc++);
		if ((xb1 & 0x30) == 0 || (xb1 & 0x70) == 0x20 || (xb1 & 0x70) == 0x50)
		{
			u8 xb2 = opcodes.r8(pc++);
			if (BIT(xb1, 7))
			{
				u8 xb3 = opcodes.r8(pc++);
				util::stream_format(stream, "S%s/%s/%c.%c ", (xb1 & 0x30) == 0 ? "SCH" : "CMP", s_term_conditions[xb3 & 0x0f], "FB"[BIT(xb1, 6)], "BW"[BIT(op, 0)]);
				format_register_to_register(stream, xb2);
				stream << ", ";
				format_register(stream, 0, xb1 & 0x0f);
				stream << ", ";
				format_register(stream, 0, xb3 >> 4);
			}
			else
			{
				util::stream_format(stream, "S%s/%c.%c ", (xb1 & 0x30) == 0 ? "STR" : "MOV", "FB"[BIT(xb1, 6)], "BW"[BIT(op, 0)]);
				format_register_to_register(stream, xb2);
				stream << ", ";
				format_register(stream, 0, xb1 & 0x0f);
			}
		}
		else
			stream << "{RIE}";
		break;
	}

	case 0x98: case 0x99: case 0x9a:
		util::stream_format(stream, "%-9s", string_format("BRA.%c", "BWL"[op & 0x03]));
		dasm_branch_disp(stream, pc, opcodes, op & 0x03);
		break;

	case 0x9b:
		util::stream_format(stream, "%-9s", "JMP");
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::MEMORY, 2);
		break;

	case 0xa0: case 0xa1: case 0xa2: case 0xb0: case 0xb1: case 0xb2:
		util::stream_format(stream, "%-9s", string_format("B%s.%c", BIT(op, 4) ? "NE" : "EQ", "BWL"[op & 0x03]));
		dasm_branch_disp(stream, pc, opcodes, op & 0x03);
		break;

	case 0xa3: case 0xa7:
		util::stream_format(stream, "%-9s", BIT(op, 2) ? "DSUB" : "DADD");
		dasm_eas_ead(stream, pc, opcodes, ea_mode::SIGNED, ea_mode::DESTINATION, 0);
		break;

	case 0xa4: case 0xa5: case 0xa6:
	{
		u8 xb = opcodes.r8(pc++);
		util::stream_format(stream, "%-9s", string_format("B%s.%c", s_conditions[xb & 0x0f], "BWL"[op & 0x03]));
		dasm_branch_disp(stream, pc, opcodes, op & 0x03);
		if ((xb & 0x0e) != 0)
			flags |= STEP_COND;
		break;
	}

	case 0xa8: case 0xa9: case 0xaa:
		util::stream_format(stream, "%-9s", string_format("BSR.%c", "BWL"[op & 0x03]));
		dasm_branch_disp(stream, pc, opcodes, op & 0x03);
		flags |= STEP_OVER;
		break;

	case 0xab:
		util::stream_format(stream, "%-9s", "JSR");
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::MEMORY, 2);
		flags |= STEP_OVER;
		break;

	case 0xac: case 0xad: case 0xae: case 0xbc: case 0xbd: case 0xbe:
		util::stream_format(stream, "%-9s", string_format("EXT%c.%c", "US"[BIT(op, 4)], "WLB"[op & 0x03]));
		format_register(stream, 0, opcodes.r8(pc++) & 0x0f);
		break;

	case 0xb3:
		util::stream_format(stream, "%-9s", "XCH");
		format_register_to_register(stream, opcodes.r8(pc++));
		break;

	case 0xb4: case 0xb5: case 0xb6:
	{
		u8 xb = opcodes.r8(pc++);
		util::stream_format(stream, "%-9s", string_format("SCB/%s.%c", s_conditions[xb & 0x0f], "BWL"[op & 0x03]));
		format_register(stream, 0, xb >> 4);
		stream << ", ";
		dasm_branch_disp(stream, pc, opcodes, op & 0x03);
		flags |= STEP_COND;
		break;
	}

	case 0xb7:
		util::stream_format(stream, "%-9s", string_format("SET/%s", s_conditions[opcodes.r8(pc++) & 0x0f]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, 0);
		break;

	case 0xb8: case 0xb9: case 0xba:
		util::stream_format(stream, "%-9s", string_format("RTD.%c", "BWL"[op & 0x03]));
		dasm_ea(stream, pc, opcodes, 0x71 + (op & 0x03), 0, ea_mode::SIGNED, op & 0x03);
		flags |= STEP_OUT;
		break;

	case 0xbb:
		stream << "RTS";
		flags |= STEP_OUT;
		break;

	case 0xbf:
		util::stream_format(stream, "%-9s", "MOVA");
		dasm_eas_ead(stream, pc, opcodes, ea_mode::ADDRESS, ea_mode::DESTINATION, 2);
		break;

	case 0xd0: case 0xd1: case 0xd2:
		util::stream_format(stream, "%-9s", string_format("LINK", "BWL"[op & 0x03]));
		format_register(stream, 0, opcodes.r8(pc++) & 0x0f);
		stream << ", ";
		dasm_ea(stream, pc, opcodes, 0x71 + (op & 0x03), 0, ea_mode::SIGNED, op & 0x03);
		break;

	case 0xd3:
		util::stream_format(stream, "%-9s", "UNLK");
		format_register(stream, 0, opcodes.r8(pc++) & 0x0f);
		break;

	case 0xd4: case 0xd5: case 0xd6:
		util::stream_format(stream, "%-9s", s_bf_ops[op & 0x03]);
		format_register_to_register(stream, opcodes.r8(pc++));
		stream << ", ";
		if (op == 0xd5)
			dasm_eas_ead(stream, pc, opcodes, ea_mode::BITFIELD, ea_mode::BITFIELD, 1, 2);
		else
			dasm_eas_ead(stream, pc, opcodes, ea_mode::BITFIELD, ea_mode::BITFIELD, 2, 1);
		break;

	case 0xd7:
		util::stream_format(stream, "%-9s", "BFMOV");
		format_register_to_register(stream, opcodes.r8(pc++));
		stream << ", ";
		format_register_to_register(stream, opcodes.r8(pc++));
		stream << ", ";
		format_register_to_register(stream, opcodes.r8(pc++));
		break;

	case 0xe0: case 0xe1:
		util::stream_format(stream, "%-9s", string_format("MOVTP.%c", "WL"[BIT(op, 0)]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::UNSIGNED, ea_mode::PLAIN_MEMORY, BIT(op, 0) ? 2 : 1);
		break;

	case 0xe2: case 0xe3:
		util::stream_format(stream, "%-9s", string_format("MOVFP.%c", "WL"[BIT(op, 0)]));
		dasm_eas_ead(stream, pc, opcodes, ea_mode::PLAIN_MEMORY, ea_mode::DESTINATION, BIT(op, 0) ? 2 : 1);
		break;

	case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		util::stream_format(stream, "%-9s", "CGBN");
		if (BIT(op, 1))
			util::stream_format(stream, "#H'%02X", opcodes.r8(pc++));
		else
			format_register(stream, 0, opcodes.r8(pc++) & 0x0f);
		if (BIT(op, 0))
		{
			stream << ", ";
			format_register_list(stream, opcodes.r16(pc));
			pc += 2;
		}
		break;

	case 0xe8:
		stream << "PGBN";
		break;

	case 0xe9:
		util::stream_format(stream, "%-9s", "PGBN");
		format_register_list(stream, opcodes.r16(pc));
		pc += 2;
		break;

	case 0xea: case 0xeb:
		util::stream_format(stream, "%-9s", string_format("SWAP.%c", "BW"[BIT(op, 0)]));
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, BIT(op, 0) + 1);
		break;

	case 0xec:
		util::stream_format(stream, "%-9s", "TAS");
		dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, 0);
		break;

	case 0xee: case 0xef:
	{
		u8 xb = opcodes.r8(pc++);
		if ((xb & 0xa0) == 0)
		{
			util::stream_format(stream, "%-9s", string_format("%sX%c.%c ", BIT(op, 0) ? "DIV" : "MUL", "SU"[BIT(xb, 6)], "BW"[BIT(xb, 4)]));
			if (BIT(xb, 4))
				dasm_eas_ead(stream, pc, opcodes, BIT(xb, 6) ? ea_mode::UNSIGNED : ea_mode::SIGNED, ea_mode::DESTINATION, 1, 2);
			else
				dasm_eas_ead(stream, pc, opcodes, BIT(xb, 6) ? ea_mode::UNSIGNED : ea_mode::SIGNED, ea_mode::DESTINATION, 0, 1);
		}
		else
			stream << "{RIE}";
		break;
	}

	case 0xf0:
		stream << "RESET";
		break;

	case 0xf1:
		stream << "RTE";
		flags |= STEP_OUT;
		break;

	case 0xf2:
		util::stream_format(stream, "%-9s#%d", "TRAPA", opcodes.r8(pc++) & 0x0f);
		flags |= STEP_OVER;
		break;

	case 0xf3:
	{
		u8 xb = opcodes.r8(pc++);
		util::stream_format(stream, "TRAP/%s", s_conditions[xb & 0x0f]);
		flags |= STEP_OVER;
		if ((xb & 0x0e) != 0)
			flags |= STEP_COND;
		break;
	}

	case 0xf4:
		stream << "RTR";
		flags |= STEP_OUT;
		break;

	case 0xf5:
		stream << "SLEEP";
		break;

	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc:
	{
		u8 cr = opcodes.r8(pc++);
		util::stream_format(stream, "%-9s", s_cr_ops[op & 0x07]);
		if (op >= 0xfb)
		{
			format_cr(stream, cr);
			stream << ", ";
		}
		if (op == 0xfc)
			dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::DESTINATION, get_cr_size(cr));
		else
		{
			dasm_ea(stream, pc, opcodes, opcodes.r8(pc++) & 0x7f, 0, ea_mode::UNSIGNED, get_cr_size(cr));
			if (op < 0xfb)
			{
				stream << ", ";
				format_cr(stream, cr);
			}
		}
		break;
	}

	case 0xfd:
		stream << "ICBN";
		break;

	case 0xfe:
		stream << "DCBN";
		break;

	case 0xff:
		stream << "NOP";
		break;

	default:
		stream << "{RIE}";
		break;
	}

	return ((pc - pc0) & LENGTHMASK) | flags;
}
