// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Fujitsu F²MC-16 family disassembler

***************************************************************************/

#include "emu.h"
#include "f2mc16d.h"

f2mc16_disassembler::f2mc16_disassembler()
	: util::disasm_interface()
{
}

const std::string_view f2mc16_disassembler::s_segment_prefixes[4] =
{
	"PCB", "DTB", "ADB", "SPB"
};

const std::string_view f2mc16_disassembler::s_segment_registers[6] =
{
	"DTB", "ADB", "SSB", "USB", "DPR", "SPB"
};

const std::string_view f2mc16_disassembler::s_bit_ops[8] =
{
	"MOVB", "MOVB",
	"CLRB", "SETB",
	"BBC", "BBS",
	"WBTS", "WBTC"
};

const std::string_view f2mc16_disassembler::s_movs_ops[4] =
{
	"MOVSI", "MOVSD", "MOVSWI", "MOVSWD"
};

const std::string_view f2mc16_disassembler::s_sceq_ops[4] =
{
	"SCEQI", "SCEQD", "SCWEQI", "SCWEQD"
};

const std::string_view f2mc16_disassembler::s_shift_ops[3][4] =
{
	{ "LSLW", "???", "ASRW", "LSRW" },
	{ "LSLL", "???", "ASRL", "LSRL" },
	{ "LSL", "NRML", "ASR", "LSR" }
};

const std::string_view f2mc16_disassembler::s_movp_ops[4] =
{
	"MOVP", "MOVPX", "MOVPW", "MOVPL"
};

const std::string_view f2mc16_disassembler::s_bcc_ops[16] =
{
	"BZ", "BNZ",
	"BC", "BNC",
	"BN", "BP",
	"BV", "BNV",
	"BT", "BNT",
	"BLT", "BGE",
	"BLE", "BGT",
	"BLS", "BHI"
};

offs_t f2mc16_disassembler::opcode_alignment() const
{
	return 1;
}

u32 f2mc16_disassembler::interface_flags() const
{
	return PAGED;
}

u32 f2mc16_disassembler::page_address_bits() const
{
	return 16;
}

void f2mc16_disassembler::format_imm4(std::ostream &stream, u8 x)
{
	if (x > 9)
		util::stream_format(stream, "#0x%X", x);
	else
		util::stream_format(stream, "#%d", x);
}

void f2mc16_disassembler::format_imm8(std::ostream &stream, u8 x)
{
	util::stream_format(stream, "#0x%02X", x);
}

void f2mc16_disassembler::format_imm16(std::ostream &stream, u16 x)
{
	util::stream_format(stream, "#0x%04X", x);
}

void f2mc16_disassembler::format_imm32(std::ostream &stream, u32 x)
{
	util::stream_format(stream, "#0x%08X", x);
}

void f2mc16_disassembler::format_imm_signed(std::ostream &stream, s32 x)
{
	stream << '#';
	if (x < 0)
	{
		stream << '-';
		x = -x;
	}
	if (u32(x) > 9)
		util::stream_format(stream, "0x%X", u32(x));
	else
		util::stream_format(stream, "%d", x);
}

void f2mc16_disassembler::format_dir(std::ostream &stream, u8 segm, u8 addr)
{
	if (segm != 0)
		util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
	util::stream_format(stream, "S:0x%02X", addr);
}

void f2mc16_disassembler::format_io(std::ostream &stream, u8 addr)
{
	util::stream_format(stream, "I:0x%02X", addr);
}

void f2mc16_disassembler::format_addr16(std::ostream &stream, u8 segm, u16 addr)
{
	if (segm != 0)
		util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
	else
		stream << "D:";
	util::stream_format(stream, "0x%04X", addr);
}

void f2mc16_disassembler::format_dst16(std::ostream &stream, u16 addr)
{
	util::stream_format(stream, "0x%04X", addr);
}

void f2mc16_disassembler::format_addr24(std::ostream &stream, u32 addr)
{
	util::stream_format(stream, "0x%06X", addr);
}

void f2mc16_disassembler::format_rel(std::ostream &stream, offs_t pc, s8 disp)
{
	util::stream_format(stream, "0x%04X", (pc + disp) & 0xffff);
}

void f2mc16_disassembler::format_rwdisp8(std::ostream &stream, u8 r, u8 segm, s8 disp)
{
	if (segm != 0)
		util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
	util::stream_format(stream, "@RW%d", r);
	if (disp < 0)
	{
		stream << '-';
		disp = -disp;
	}
	else
		stream << '+';
	if (u8(disp) > 9)
		stream << "0x";
	util::stream_format(stream, "%X", u8(disp));
}

void f2mc16_disassembler::format_rwdisp16(std::ostream &stream, u8 r, u8 segm, u16 disp)
{
	if (segm != 0)
		util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
	util::stream_format(stream, "@RW%d+0x%04X", r, disp);
}

void f2mc16_disassembler::format_rldisp8(std::ostream &stream, u8 r, s8 disp)
{
	util::stream_format(stream, "@RL%d", r);
	if (disp < 0)
	{
		stream << '-';
		disp = -disp;
	}
	else
		stream << '+';
	if (u8(disp) > 9)
		stream << "0x";
	util::stream_format(stream, "%X", u8(disp));
}

void f2mc16_disassembler::format_spdisp8(std::ostream &stream, s8 disp)
{
	stream << "@SP";
	if (disp < 0)
	{
		stream << '-';
		disp = -disp;
	}
	else
		stream << '+';
	if (u8(disp) > 9)
		stream << "0x";
	util::stream_format(stream, "%X", u8(disp));
}

void f2mc16_disassembler::format_pcdisp16(std::ostream &stream, s16 disp)
{
	if (disp < 0)
	{
		stream << "@PC-";
		disp = -disp;
	}
	else
		stream << "@PC+";
	if (u16(disp) > 9)
		stream << "0x";
	util::stream_format(stream, "%X", u16(disp));
}

void f2mc16_disassembler::format_rlist(std::ostream &stream, u8 rlst)
{
	stream << '(';

	bool first = true;
	int last = -1;
	for (int i = 0; i <= 8; i++)
	{
		if (i != 8 && BIT(rlst, i))
		{
			if (last == -1)
				last = i;
		}
		else if (last != -1)
		{
			if (first)
				first = false;
			else
				stream << ',';
			util::stream_format(stream, "RW%d", last);
			if (last != i - 1)
				util::stream_format(stream, "-RW%d", i - 1);
			last = -1;
		}
	}

	stream << ')';
}

u32 f2mc16_disassembler::dasm_ea8(std::ostream &stream, offs_t pc, u8 op2, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	op2 &= 0x1f;
	if (op2 < 0x08)
	{
		util::stream_format(stream, "R%d", op2);
		return 0;
	}
	else if (op2 < 0x10)
	{
		if (segm != 0)
			util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
		util::stream_format(stream, "@RW%d", op2 & 0x03);
		if (op2 >= 0x0c)
			stream << '+';
		return 0;
	}
	else if (op2 < 0x18)
	{
		format_rwdisp8(stream, op2 & 0x07, segm, opcodes.r8(pc));
		return 1;
	}
	else if (op2 < 0x1c)
	{
		format_rwdisp16(stream, op2 & 0x03, segm, opcodes.r16(pc));
		return 2;
	}
	else if (op2 < 0x1e)
	{
		if (segm != 0)
			util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
		util::stream_format(stream, "@RW%d+RW7", op2 & 0x01);
		return 0;
	}
	else if (op2 == 0x1e)
	{
		format_pcdisp16(stream, s16(opcodes.r16(pc)));
		return 2;
	}
	else
	{
		format_addr16(stream, segm, opcodes.r16(pc));
		return 2;
	}
}

u32 f2mc16_disassembler::dasm_ea16(std::ostream &stream, offs_t pc, u8 op2, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x18) == 0)
	{
		util::stream_format(stream, "RW%d", op2 & 0x07);
		return 0;
	}
	else
		return dasm_ea8(stream, pc, op2, segm, opcodes);
}

u32 f2mc16_disassembler::dasm_ea32(std::ostream &stream, offs_t pc, u8 op2, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x18) == 0)
	{
		util::stream_format(stream, "RL%d", BIT(op2, 1, 2));
		return 0;
	}
	else
		return dasm_ea8(stream, pc, op2, segm, opcodes);
}

offs_t f2mc16_disassembler::dasm_bitop(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + bytes++);
	if (op2 >= 0xf8)
	{
		util::stream_format(stream, "%-8s", "SBBS");
		format_addr16(stream, segm, opcodes.r16(pc + bytes));
		util::stream_format(stream, ":%d, ", op2 & 0x07);
		format_rel(stream, pc + bytes + 3, opcodes.r8(pc + bytes + 2));
		bytes += 3;
	}
	else if ((op2 & 0x18) != 0x10 && (op2 & 0xc8) != 0xc8)
	{
		util::stream_format(stream, "%-8s", s_bit_ops[BIT(op2, 5, 3)]);
		if (op2 < 0x20)
			stream << "A, ";
		if (BIT(op2, 4))
		{
			format_addr16(stream, segm, opcodes.r16(pc + bytes));
			bytes += 2;
		}
		else if (BIT(op2, 3))
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		else
			format_io(stream, opcodes.r8(pc + bytes++));
		util::stream_format(stream, ":%d", op2 & 0x07);
		if ((op2 & 0xc0) == 0x80)
		{
			stream << ", ";
			format_rel(stream, pc + bytes + 1, opcodes.r8(pc + bytes));
			return (bytes + 1) | STEP_COND | SUPPORTED;
		}
		else if ((op2 & 0xe0) == 0x20)
			stream << ", A";
	}
	else
	{
		util::stream_format(stream, "%-8s0x%02X", ".DATA.B", segm != 0 ? segm : 0x6c);
		return 1 | SUPPORTED;
	}
	return bytes | SUPPORTED;
}

offs_t f2mc16_disassembler::dasm_movm(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	// 16F only?
	u8 op2 = opcodes.r8(pc + bytes++);
	util::stream_format(stream, "%-8s", BIT(op2, 6) ? "MOVMW" : "MOVM");
	if (op2 < 0x80)
	{
		if (BIT(op2, 5))
			stream << "@A";
		else
		{
			format_addr16(stream, segm, opcodes.r16(pc + bytes));
			bytes += 2;
		}
		stream << ", ";
	}
	if ((op2 & 0x18) == 0)
		util::stream_format(stream, "@RL%d", BIT(op2, 1, 2));
	else
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
	if (op2 >= 0x80)
	{
		stream << ", ";
		if (BIT(op2, 5))
			stream << "@A";
		else
		{
			format_addr16(stream, segm, opcodes.r16(pc + bytes));
			bytes += 2;
		}
	}
	stream << ", ";
	format_imm8(stream, opcodes.r8(pc + bytes++));
	return bytes | SUPPORTED;
}

offs_t f2mc16_disassembler::dasm_cstrop(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + bytes++);
	if (op2 < 0x40)
		util::stream_format(stream, "%-8s%s, %s", s_movs_ops[BIT(op2, 4, 2)], s_segment_prefixes[BIT(op2, 2, 2)], s_segment_prefixes[BIT(op2, 0, 2)]);
	else if (op2 < 0x60)
	{
		// 16F only?
		util::stream_format(stream, "%-8s", BIT(op2, 4) ? "MOVMW" : "MOVM");
		format_addr16(stream, 0x04 | BIT(op2, 2, 2), opcodes.r16(pc + bytes));
		stream << ", ";
		format_addr16(stream, 0x04 | BIT(op2, 0, 2), opcodes.r16(pc + bytes + 2));
		stream << ", ";
		format_imm8(stream, opcodes.r16(pc + bytes + 4));
		bytes += 5;
	}
	else if ((op2 & 0xcc) == 0x80)
		util::stream_format(stream, "%-8s%s", s_sceq_ops[BIT(op2, 4, 2)], s_segment_prefixes[BIT(op2, 0, 2)]);
	else if ((op2 & 0xdc) == 0xc0)
		util::stream_format(stream, "%-8s%s", op2 >= 0xe0 ? "FILSWI" : "FILSI", s_segment_prefixes[BIT(op2, 0, 2)]);
	else
	{
		util::stream_format(stream, "%-8s0x%02X", ".DATA.B", segm != 0 ? segm : 0x6e);
		return 1 | SUPPORTED;
	}
	return bytes | SUPPORTED;
}

offs_t f2mc16_disassembler::dasm_op6f(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + bytes++);
	switch (op2)
	{
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04:
		util::stream_format(stream, "%-8sA, %s", "MOV", s_segment_registers[op2]);
		break;

	case 0x05: case 0x0d:
		util::stream_format(stream, "%-8sA, ", BIT(op2, 3) ? "MOVW" : "MOV");
		if (segm != 0)
			util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
		stream << "@A";
		break;

	case 0x06:
		util::stream_format(stream, "%-8sA, PCB", "MOV");
		break;

	case 0x07:
		util::stream_format(stream, "%-8sA", "ROLC");
		break;

	case 0x08: case 0x0a: case 0x0b:
	case 0x18: case 0x1a: case 0x1b:
	case 0x28: case 0x2a: case 0x2b: // all 16F only?
		util::stream_format(stream, "%-8sA, ", s_shift_ops[BIT(op2, 4, 2)][BIT(op2, 0, 2)]);
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x09: // 16F only?
		util::stream_format(stream, "%-8sA, SPB", "MOV");
		break;

	case 0x0c: case 0x0e: case 0x0f:
	case 0x1c: case 0x1e: case 0x1f:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		util::stream_format(stream, "%-8sA, R0", s_shift_ops[BIT(op2, 4, 2)][BIT(op2, 0, 2)]);
		break;

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
		util::stream_format(stream, "%-8s%s, A", "MOV", s_segment_registers[op2 & 0x07]);
		break;

	case 0x15: case 0x1d:
		util::stream_format(stream, "%-8sAL, ", BIT(op2, 3) ? "MOVW" : "MOV");
		if (segm != 0)
			util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
		stream << "@AH";
		break;

	case 0x16:
		util::stream_format(stream, "%-8sA, ", "MOVX");
		if (segm != 0)
			util::stream_format(stream, "%s:", s_segment_prefixes[segm & 0x03]);
		stream << "@A";
		break;

	case 0x17:
		util::stream_format(stream, "%-8sA", "RORC");
		break;

	case 0x19: // 16F only?
		util::stream_format(stream, "%-8sSPB, A", "MOV");
		break;

	case 0x20: case 0x22: case 0x24: case 0x26:
	case 0x21: case 0x23: case 0x25: case 0x27: // defined as aliases on 16F only?
		util::stream_format(stream, "%-8sA, ", "MOVX");
		format_rldisp8(stream, BIT(op2, 1, 2), opcodes.r8(pc + bytes++));
		break;

	case 0x29: // 16F only?
		stream << "RETIQ";
		return bytes | STEP_OUT | SUPPORTED;

	case 0x30: case 0x32: case 0x34: case 0x36: case 0x38: case 0x3a: case 0x3c: case 0x3e:
	case 0x31: case 0x33: case 0x35: case 0x37: case 0x39: case 0x3b: case 0x3d: case 0x3f: // defined as aliases on 16F only?
		util::stream_format(stream, "%-8s", BIT(op2, 3) ? "MOVW" : "MOV");
		format_rldisp8(stream, BIT(op2, 1, 2), opcodes.r8(pc + bytes++));
		stream << ", A";
		break;

	case 0x40: case 0x42: case 0x44: case 0x46: case 0x48: case 0x4a: case 0x4c: case 0x4e:
	case 0x41: case 0x43: case 0x45: case 0x47: case 0x49: case 0x4b: case 0x4d: case 0x4f: // defined as aliases on 16F only?
		util::stream_format(stream, "%-8sA, ", BIT(op2, 3) ? "MOVW" : "MOV");
		format_rldisp8(stream, BIT(op2, 1, 2), opcodes.r8(pc + bytes++));
		break;

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: // 16F only?
		util::stream_format(stream, "%-8s%s, ", "MOV", s_segment_registers[op2 & 0x07]);
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x56: case 0x57: // 16F only?
		util::stream_format(stream, "%-8sSPC%c, ", "MOVW", BIT(op2, 0) ? 'L' : 'U');
		format_imm8(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x58: case 0x59: case 0x5a: case 0x5b: // 16F only?
		util::stream_format(stream, "%-8sA, @A", s_movp_ops[op2 & 0x03]);
		break;

	case 0x5c: // 16F only?
		util::stream_format(stream, "%-8sA", "BTSCN");
		break;

	case 0x5d: // 16F only?
		stream << "SETSPC";
		break;

	case 0x5e: // 16F only?
		util::stream_format(stream, "%-8sA", "BTSCNS");
		break;

	case 0x5f: // 16F only?
		util::stream_format(stream, "%-8sA", "BTSCND");
		break;

	case 0x60: // 16F only?
		util::stream_format(stream, "%-8sA, ", "MOV");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x61: // 16F only?
		util::stream_format(stream, "%-8sA, ", "MOVX");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x62: // 16F only?
		util::stream_format(stream, "%-8sA, ", "MOVW");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x63: // 16F only?
		util::stream_format(stream, "%-8sA, ", "MOVL");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x64: // 16F only?
		util::stream_format(stream, "%-8s", "MOV");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		stream << ", A";
		break;

	case 0x65: // 16F only?
		util::stream_format(stream, "%-8sSP, ", "MOVW");
		format_imm16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x66: // 16F only?
		util::stream_format(stream, "%-8s", "MOVW");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		stream << ", A";
		break;

	case 0x67: // 16F only?
		util::stream_format(stream, "%-8s", "MOVL");
		format_spdisp8(stream, opcodes.r8(pc + bytes++));
		stream << ", A";
		break;

	case 0x68: case 0x69: case 0x6a: case 0x6b: // 16F only?
		util::stream_format(stream, "%-8sA, ", s_movp_ops[op2 & 0x03]);
		format_addr24(stream, opcodes.r16(pc + bytes) | u32(opcodes.r8(pc + bytes + 2)) << 16);
		bytes += 3;
		break;

	case 0x6c: case 0x6e: case 0x6f: // 16F only?
		util::stream_format(stream, "%-8s", s_movp_ops[op2 & 0x03]);
		format_addr24(stream, opcodes.r16(pc + bytes) | u32(opcodes.r8(pc + bytes + 2)) << 16);
		stream << ", A";
		bytes += 3;
		break;

	case 0x6d: // 16F only?
		stream << "CLRSPC";
		break;

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: // 16F only?
		util::stream_format(stream, "%-8s@A, RL%d", "MOVPL", BIT(op2, 1, 2));
		break;

	case 0x78: // 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA", "MUL");
		break;

	case 0x79: // 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA", "MULW");
		break;

	case 0x7a: // 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA", "DIV");
		break;

	case 0x7b: // 16F only?
		util::stream_format(stream, "%-8sA", "ABS");
		break;

	case 0x7c: // 16F only?
		util::stream_format(stream, "%-8sA", "ABSW");
		break;

	case 0x7d: // 16F only?
		util::stream_format(stream, "%-8sA", "ABSL");
		break;

	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: // 16F only?
		util::stream_format(stream, "%-8s@A, R%d", "MOVP", op2 & 0x07);
		break;

	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f: // 16F only?
		util::stream_format(stream, "%-8s@A, RW%d", "MOVPW", op2 & 0x07);
		break;

	default:
		util::stream_format(stream, "%-8s0x%02X", ".DATA.B", segm != 0 ? segm : 0x6f);
		return 1 | SUPPORTED;
	}

	return bytes | SUPPORTED;
}

offs_t f2mc16_disassembler::dasm_eainst(std::ostream &stream, offs_t pc, u32 bytes, u8 op1, u8 segm, const f2mc16_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + bytes++);
	switch ((op1 & 0x0f) | (op2 & 0xe0))
	{
	case 0x00:
		util::stream_format(stream, "%-8sA, ", "ADDL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x20:
		util::stream_format(stream, "%-8sA, ", "SUBL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x40:
		util::stream_format(stream, "%-8s", "CWBNE");
		if ((op2 & 0x1e) == 0x0e)
			stream << "(use prohibited)";
		else
			bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", ";
		format_imm16(stream, opcodes.r16(pc + bytes));
		stream << ", ";
		format_rel(stream, pc + bytes + 3, opcodes.r8(pc + bytes + 2));
		bytes += 3;
		break;

	case 0x60:
		util::stream_format(stream, "%-8sA, ", "CMPL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x80:
		util::stream_format(stream, "%-8sA, ", "ANDL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa0:
		util::stream_format(stream, "%-8sA, ", "ORL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xc0:
		util::stream_format(stream, "%-8sA, ", "XORL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xe0:
		util::stream_format(stream, "%-8s", "CBNE");
		if ((op2 & 0x1e) == 0x0e)
			stream << "(use prohibited)";
		else
			bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", ";
		format_imm8(stream, opcodes.r8(pc + bytes));
		stream << ", ";
		format_rel(stream, pc + bytes + 2, opcodes.r8(pc + bytes + 1));
		return (bytes + 2) | STEP_COND | SUPPORTED;

	case 0x01:
		util::stream_format(stream, "%-8s@", "JMPP");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x21:
		util::stream_format(stream, "%-8s@", "CALLP");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		return bytes | STEP_OVER | SUPPORTED;

	case 0x41:
		util::stream_format(stream, "%-8s", "INCL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x61:
		util::stream_format(stream, "%-8s", "DECL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x81:
		util::stream_format(stream, "%-8sA, ", "MOVL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa1:
		util::stream_format(stream, "%-8s", "MOVL");
		bytes += dasm_ea32(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xc1:
		util::stream_format(stream, "%-8s", "MOV");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", ";
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0xe1:
		util::stream_format(stream, "%-8sA, ", "MOVEA");
		bytes += dasm_ea16(stream, pc + bytes, op2, 0, opcodes);
		break;

	case 0x02:
		util::stream_format(stream, "%-8s", "ROLC");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x22:
		util::stream_format(stream, "%-8s", "RORC");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x42:
		util::stream_format(stream, "%-8s", "INC");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x62:
		util::stream_format(stream, "%-8s", "DEC");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x82:
		util::stream_format(stream, "%-8sA, ", "MOV");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa2:
		util::stream_format(stream, "%-8s", "MOV");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xc2:
		util::stream_format(stream, "%-8sA, ", "MOVX");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xe2:
		util::stream_format(stream, "%-8sA, ", "XCH");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x03:
		util::stream_format(stream, "%-8s@", "JMP");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x23:
		util::stream_format(stream, "%-8s@", "CALL");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		return bytes | STEP_OVER | SUPPORTED;

	case 0x43:
		util::stream_format(stream, "%-8s", "INCW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x63:
		util::stream_format(stream, "%-8s", "DECW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x83:
		util::stream_format(stream, "%-8sA, ", "MOVW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa3:
		util::stream_format(stream, "%-8s", "MOVW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xc3:
		util::stream_format(stream, "%-8s", "MOVW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", ";
		format_imm16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0xe3:
		util::stream_format(stream, "%-8sA, ", "XCHW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x04:
		util::stream_format(stream, "%-8sA, ", "ADD");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x24:
		util::stream_format(stream, "%-8sA, ", "SUB");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x44:
		util::stream_format(stream, "%-8sA, ", "ADDC");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x64:
		util::stream_format(stream, "%-8sA, ", "CMP");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x84:
		util::stream_format(stream, "%-8sA, ", "AND");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa4:
		util::stream_format(stream, "%-8sA, ", "OR");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xc4:
		util::stream_format(stream, "%-8sA, ", "XOR");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xe4:
		util::stream_format(stream, "%-8s", "DBNZ");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", ";
		format_rel(stream, pc + bytes + 1, opcodes.r8(pc + bytes));
		return (bytes + 1) | STEP_COND | SUPPORTED;

	case 0x05:
		util::stream_format(stream, "%-8s", "ADD");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0x25:
		util::stream_format(stream, "%-8s", "SUB");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0x45:
		util::stream_format(stream, "%-8sA, ", "SUBC");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x65:
		util::stream_format(stream, "%-8s", "NEG");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x85:
		util::stream_format(stream, "%-8s", "AND");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xa5:
		util::stream_format(stream, "%-8s", "OR");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xc5:
		util::stream_format(stream, "%-8s", "XOR");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xe5:
		util::stream_format(stream, "%-8s", "NOT");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x06:
		util::stream_format(stream, "%-8sA, ", "ADDW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x26:
		util::stream_format(stream, "%-8sA, ", "SUBW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x46:
		util::stream_format(stream, "%-8sA, ", "ADDCW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x66:
		util::stream_format(stream, "%-8sA, ", "CMPW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x86:
		util::stream_format(stream, "%-8sA, ", "ANDW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa6:
		util::stream_format(stream, "%-8sA, ", "ORW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xc6:
		util::stream_format(stream, "%-8sA, ", "XORW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xe6:
		util::stream_format(stream, "%-8s", "DWBNZ");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", ";
		format_rel(stream, pc + bytes + 1, opcodes.r8(pc + bytes));
		return (bytes + 1) | STEP_COND | SUPPORTED;

	case 0x07:
		util::stream_format(stream, "%-8s", "ADDW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0x27:
		util::stream_format(stream, "%-8s", "SUBW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0x47:
		util::stream_format(stream, "%-8sA, ", "SUBCW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x67:
		util::stream_format(stream, "%-8s", "NEGW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x87:
		util::stream_format(stream, "%-8s", "ANDW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xa7:
		util::stream_format(stream, "%-8s", "ORW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xc7:
		util::stream_format(stream, "%-8s", "XORW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		stream << ", A";
		break;

	case 0xe7:
		util::stream_format(stream, "%-8s", "NOTW");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x08: case 0x48: // MUL is 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA, ", BIT(op2, 6) ? "MUL" : "MULU");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x28: case 0x68: // MULW is 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA, ", BIT(op2, 6) ? "MULW" : "MULUW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0x88: case 0xc8: // DIV is 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA, ", BIT(op2, 6) ? "DIV" : "DIVU");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;

	case 0xa8: case 0xe8: // DIVW is 16F, 16FX, 16LX only?
		util::stream_format(stream, "%-8sA, ", BIT(op2, 6) ? "DIVW" : "DIVUW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;

	default:
		util::stream_format(stream, "%-8s0x%02X", ".DATA.B", segm != 0 ? segm : op1);
		return 1 | SUPPORTED;
	}

	return bytes | SUPPORTED;
}

offs_t f2mc16_disassembler::disassemble(std::ostream &stream, offs_t pc, const f2mc16_disassembler::data_buffer &opcodes, const f2mc16_disassembler::data_buffer &params)
{
	u8 op = opcodes.r8(pc);
	u8 segm = 0;
	u32 bytes = 1;
	if ((op & 0xfc) == 0x04)
	{
		segm = op;
		op = opcodes.r8(pc + 1);
		++bytes;
	}

	if (op >= 0xf0)
	{
		util::stream_format(stream, "%-8s", s_bcc_ops[op & 0x0f]);
		format_rel(stream, pc + bytes + 1, opcodes.r8(pc + bytes));
		return (bytes + 1) | STEP_COND | SUPPORTED;
	}
	else if (op >= 0xe0)
	{
		util::stream_format(stream, "%-8s#%d", "CALLV", op & 0x0f);
		return bytes | STEP_OVER | SUPPORTED;
	}
	else if (op >= 0xd0)
	{
		util::stream_format(stream, "%-8sA, ", "MOVN");
		format_imm4(stream, op & 0x0f);
	}
	else if (op >= 0xc8)
	{
		util::stream_format(stream, "%-8s", "MOVW");
		format_rwdisp8(stream, op & 0x07, segm, opcodes.r8(pc + bytes++));
		stream << ", A";
	}
	else if (op >= 0xc0)
	{
		util::stream_format(stream, "%-8sA, ", "MOVX");
		format_rwdisp8(stream, op & 0x07, segm, opcodes.r8(pc + bytes++));
	}
	else if (op >= 0xb8)
	{
		util::stream_format(stream, "%-8sA, ", "MOVW");
		format_rwdisp8(stream, op & 0x07, segm, opcodes.r8(pc + bytes++));
	}
	else if (op >= 0xb0)
		util::stream_format(stream, "%-8sA, R%d", "MOVX", op & 0x07);
	else if (op >= 0xa8)
	{
		util::stream_format(stream, "%-8sRW%d, ", "MOVW", op & 0x07);
		format_imm16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
	}
	else if (op >= 0xa0)
	{
		util::stream_format(stream, "%-8sR%d, ", "MOV", op & 0x07);
		format_imm8(stream, opcodes.r8(pc + bytes++));
	}
	else if (op >= 0x98)
		util::stream_format(stream, "%-8sRW%d, A", "MOVW", op & 0x07);
	else if (op >= 0x90)
		util::stream_format(stream, "%-8sR%d, A", "MOV", op & 0x07);
	else if (op >= 0x88)
		util::stream_format(stream, "%-8sA, RW%d", "MOVW", op & 0x07);
	else if (op >= 0x80)
		util::stream_format(stream, "%-8sA, R%d", "MOV", op & 0x07);
	else switch (op)
	{
	case 0x00:
		stream << "NOP";
		break;

	case 0x01:
		stream << "INT9";
		return bytes | STEP_OVER | SUPPORTED;

	case 0x02:
		util::stream_format(stream, "%-8sA", "ADDDC");
		break;

	case 0x03:
		util::stream_format(stream, "%-8sA", "NEG");
		break;

	case 0x08:
		util::stream_format(stream, "%-8s", "LINK");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x09:
		stream << "UNLINK";
		break;

	case 0x0a:
		util::stream_format(stream, "%-8sRP, ", "MOV");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x0b:
		util::stream_format(stream, "%-8sA", "NEGW");
		break;

	case 0x0c: case 0x0e: case 0x0f:
		util::stream_format(stream, "%-8sA", s_shift_ops[0][BIT(op, 0, 2)]);
		break;

	case 0x0d:
		stream << "INTE";
		return bytes | STEP_OVER | SUPPORTED;

	case 0x10:
		stream << "CMR";
		break;

	case 0x11:
		stream << "NCC";
		break;

	case 0x12:
		util::stream_format(stream, "%-8sA", "SUBDC");
		break;

	case 0x13:
		util::stream_format(stream, "%-8s@A", "JCTX");
		break;

	case 0x14: case 0x1c:
		stream << "EXT";
		if (BIT(op, 3))
			stream << 'W';
		break;

	case 0x15: case 0x1d:
		stream << "ZEXT";
		if (BIT(op, 3))
			stream << 'W';
		break;

	case 0x16: case 0x1e:
		stream << "SWAP";
		if (BIT(op, 3))
			stream << 'W';
		break;

	case 0x17: case 0x1f:
		util::stream_format(stream, "%-8s", "ADDSP");
		if (BIT(op, 3))
		{
			format_imm_signed(stream, s32(s16(opcodes.r16(pc + bytes))));
			bytes += 2;
		}
		else
			format_imm_signed(stream, s32(s8(opcodes.r8(pc + bytes++))));
		break;

	case 0x18: case 0x19:
	{
		u32 operand = opcodes.r32(pc + bytes);
		if (operand == 0x00000001)
			util::stream_format(stream, "%-8sA", BIT(op, 0) ? "DECL" : "INCL");
		else
		{
			util::stream_format(stream, "%-8sA, ", BIT(op, 0) ? "SUBL" : "ADDL");
			format_imm_signed(stream, s32(operand));
		}
		bytes += 4;
		break;
	}

	case 0x1a:
		util::stream_format(stream, "%-8sILM, ", "MOV");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x1b:
		util::stream_format(stream, "%-8sA, ", "CMPL");
		format_imm_signed(stream, s32(opcodes.r32(pc + bytes)));
		bytes += 4;
		break;

	case 0x20:
		util::stream_format(stream, "%-8sA, ", "ADD");
		format_dir(stream, segm, opcodes.r8(pc + bytes++));
		break;

	case 0x21:
		util::stream_format(stream, "%-8sA, ", "SUB");
		format_dir(stream, segm, opcodes.r8(pc + bytes++));
		break;

	case 0x22:
		util::stream_format(stream, "%-8sA", "ADDC");
		break;

	case 0x23: case 0x33:
		util::stream_format(stream, "%-8sA", "CMP");
		if (BIT(op, 4))
		{
			stream << ", ";
			format_imm_signed(stream, s32(s8(opcodes.r8(pc + bytes++))));
		}
		break;

	case 0x24:
		util::stream_format(stream, "%-8sCCR, ", "AND");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x25:
		util::stream_format(stream, "%-8sCCR, ", "OR");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x26:
		util::stream_format(stream, "%-8sA", "DIVU");
		break;

	case 0x27:
		util::stream_format(stream, "%-8sA", "MULU");
		break;

	case 0x28:
		util::stream_format(stream, "%-8sA", "ADDW");
		break;

	case 0x29:
		util::stream_format(stream, "%-8sA", "SUBW");
		break;

	case 0x2a:
		util::stream_format(stream, "%-8sA, ", "CBNE");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		stream << ", ";
		format_rel(stream, pc + bytes + 1, opcodes.r8(pc + bytes));
		return (bytes + 1) | STEP_COND | SUPPORTED;

	case 0x2b: case 0x3b:
		util::stream_format(stream, "%-8sA", "CMPW");
		if (BIT(op, 4))
		{
			stream << ", ";
			format_imm_signed(stream, s32(s16(opcodes.r16(pc + bytes))));
			bytes += 2;
		}
		break;

	case 0x2c: case 0x3c:
		util::stream_format(stream, "%-8sA", "ANDW");
		if (BIT(op, 4))
		{
			stream << ", ";
			format_imm16(stream, opcodes.r16(pc + bytes));
			bytes += 2;
		}
		break;

	case 0x2d: case 0x3d:
		util::stream_format(stream, "%-8sA", "ORW");
		if (BIT(op, 4))
		{
			stream << ", ";
			format_imm16(stream, opcodes.r16(pc + bytes));
			bytes += 2;
		}
		break;

	case 0x2e: case 0x3e:
		util::stream_format(stream, "%-8sA", "XORW");
		if (BIT(op, 4))
		{
			stream << ", ";
			format_imm16(stream, opcodes.r16(pc + bytes));
			bytes += 2;
		}
		break;

	case 0x2f:
		util::stream_format(stream, "%-8sA", "MULUW");
		break;

	case 0x30: case 0x31:
		util::stream_format(stream, "%-8sA, ", BIT(op, 0) ? "SUB" : "ADD");
		format_imm_signed(stream, s32(s8(opcodes.r8(pc + bytes++))));
		break;

	case 0x32:
		util::stream_format(stream, "%-8sA", "SUBC");
		break;

	case 0x34:
		util::stream_format(stream, "%-8sA, ", "AND");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x35:
		util::stream_format(stream, "%-8sA, ", "OR");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x36:
		util::stream_format(stream, "%-8sA, ", "XOR");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x37:
		util::stream_format(stream, "%-8sA", "NOT");
		break;

	case 0x38: case 0x39:
	{
		u16 operand = opcodes.r16(pc + bytes);
		if (operand == 0x0001)
			util::stream_format(stream, "%-8sA", BIT(op, 0) ? "DECW" : "INCW");
		else
		{
			util::stream_format(stream, "%-8sA, ", BIT(op, 0) ? "SUBW" : "ADDW");
			format_imm_signed(stream, s32(s16(operand)));
		}
		bytes += 2;
		break;
	}

	case 0x3a:
		util::stream_format(stream, "%-8sA, ", "CWBNE");
		format_imm16(stream, opcodes.r16(pc + bytes));
		stream << ", ";
		format_rel(stream, pc + bytes + 3, opcodes.r8(pc + bytes + 2));
		bytes += 3;
		break;

	case 0x3f:
		util::stream_format(stream, "%-8sA", "NOTW");
		break;

	case 0x40: case 0x50:
		util::stream_format(stream, "%-8sA, ", "MOV");
		if (BIT(op, 4))
			format_io(stream, opcodes.r8(pc + bytes++));
		else
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		break;

	case 0x41: case 0x51:
		util::stream_format(stream, "%-8s", "MOV");
		if (BIT(op, 4))
			format_io(stream, opcodes.r8(pc + bytes++));
		else
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		stream << ", A";
		break;

	case 0x42:
		util::stream_format(stream, "%-8sA, ", "MOV");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x43:
		util::stream_format(stream, "%-8sA, ", "MOVX");
		format_imm_signed(stream, s32(s8(opcodes.r8(pc + bytes++))));
		break;

	case 0x44: case 0x54:
		util::stream_format(stream, "%-8s", "MOV");
		if (BIT(op, 4))
			format_io(stream, opcodes.r8(pc + bytes++));
		else
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		stream << ", ";
		format_imm8(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x45: case 0x55:
		util::stream_format(stream, "%-8sA, ", "MOVX");
		if (BIT(op, 4))
			format_io(stream, opcodes.r8(pc + bytes++));
		else
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		break;

	case 0x46:
		util::stream_format(stream, "%-8sA, SP", "MOVW");
		break;

	case 0x47:
		util::stream_format(stream, "%-8sSP, A", "MOVW");
		break;

	case 0x48: case 0x58:
		util::stream_format(stream, "%-8sA, ", "MOVW");
		if (BIT(op, 4))
			format_io(stream, opcodes.r8(pc + bytes++));
		else
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		break;

	case 0x49: case 0x59:
		util::stream_format(stream, "%-8s", "MOVW");
		if (BIT(op, 4))
			format_io(stream, opcodes.r8(pc + bytes++));
		else
			format_dir(stream, segm, opcodes.r8(pc + bytes++));
		stream << ", A";
		break;

	case 0x4a:
		util::stream_format(stream, "%-8sA, ", "MOVW");
		format_imm16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x4b:
		util::stream_format(stream, "%-8sA, ", "MOVL");
		format_imm32(stream, opcodes.r32(pc + bytes));
		bytes += 4;
		break;

	case 0x4c: case 0x4d:
		util::stream_format(stream, "%-8sA", "PUSHW");
		if (BIT(op, 0))
			stream << 'H';
		break;

	case 0x4e:
		util::stream_format(stream, "%-8sPS", "PUSHW");
		break;

	case 0x4f:
		util::stream_format(stream, "%-8s", "PUSHW");
		format_rlist(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x52: case 0x5a:
		util::stream_format(stream, "%-8sA, ", BIT(op, 3) ? "MOVW" : "MOV");
		format_addr16(stream, segm, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x53: case 0x5b:
		util::stream_format(stream, "%-8s", BIT(op, 3) ? "MOVW" : "MOV");
		format_addr16(stream, segm, opcodes.r16(pc + bytes));
		stream << ", A";
		bytes += 2;
		break;

	case 0x56:
		util::stream_format(stream, "%-8s", "MOVW");
		format_io(stream, opcodes.r8(pc + bytes++));
		stream << ", ";
		format_imm16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x57:
		util::stream_format(stream, "%-8sA, ", "MOVX");
		format_addr16(stream, segm, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x5c: case 0x5d:
		util::stream_format(stream, "%-8sA", "POPW");
		if (BIT(op, 0))
			stream << 'H';
		break;

	case 0x5e:
		util::stream_format(stream, "%-8sPS", "POPW");
		break;

	case 0x5f:
		util::stream_format(stream, "%-8s", "POPW");
		format_rlist(stream, opcodes.r8(pc + bytes++));
		break;

	case 0x60:
		util::stream_format(stream, "%-8s", "BRA");
		format_rel(stream, pc + bytes + 1, opcodes.r8(pc + bytes));
		++bytes;
		break;

	case 0x61:
		util::stream_format(stream, "%-8s@A", "JMP");
		break;

	case 0x62:
		util::stream_format(stream, "%-8s", "JMP");
		format_dst16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		break;

	case 0x63:
		util::stream_format(stream, "%-8s", "JMPP");
		format_addr24(stream, opcodes.r16(pc + bytes) | u32(opcodes.r8(pc + bytes + 2)) << 16);
		bytes += 3;
		break;

	case 0x64:
		util::stream_format(stream, "%-8s", "CALL");
		format_dst16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		return bytes | STEP_OVER | SUPPORTED;

	case 0x65:
		util::stream_format(stream, "%-8s", "CALLP");
		format_addr24(stream, opcodes.r16(pc + bytes) | u32(opcodes.r8(pc + bytes + 2)) << 16);
		bytes += 3;
		return bytes | STEP_OVER | SUPPORTED;

	case 0x66: case 0x67:
		stream << "RET";
		if (!BIT(op, 0))
			stream << 'P';
		return bytes | STEP_OUT | SUPPORTED;

	case 0x68:
		util::stream_format(stream, "%-8s", "INT");
		format_imm8(stream, opcodes.r8(pc + bytes++));
		return bytes | STEP_OVER | SUPPORTED;

	case 0x69:
		util::stream_format(stream, "%-8s", "INT");
		format_dst16(stream, opcodes.r16(pc + bytes));
		bytes += 2;
		return bytes | STEP_OVER | SUPPORTED;

	case 0x6a:
		util::stream_format(stream, "%-8s", "INTP");
		format_addr24(stream, opcodes.r16(pc + bytes) | u32(opcodes.r8(pc + bytes + 2)) << 16);
		bytes += 3;
		return bytes | STEP_OVER | SUPPORTED;

	case 0x6b:
		stream << "RETI";
		return bytes | STEP_OUT | SUPPORTED;

	case 0x6c:
		return dasm_bitop(stream, pc, bytes, segm, opcodes);

	case 0x6d:
		return dasm_movm(stream, pc, bytes, segm, opcodes);

	case 0x6e:
		return dasm_cstrop(stream, pc, bytes, segm, opcodes);

	case 0x6f:
		return dasm_op6f(stream, pc, bytes, segm, opcodes);

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78:
		return dasm_eainst(stream, pc, bytes, op, segm, opcodes);

	case 0x79:
	{
		u8 op2 = opcodes.r8(pc + bytes++);
		util::stream_format(stream, "%-8sRW%d, ", "MOVEA", BIT(op2, 5, 3));
		bytes += dasm_ea16(stream, pc + bytes, op2, 0, opcodes);
		break;
	}

	case 0x7a: case 0x7e:
	{
		u8 op2 = opcodes.r8(pc + bytes++);
		util::stream_format(stream, "%-8sR%d, ", BIT(op, 2) ? "XCH" : "MOV", BIT(op2, 5, 3));
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		break;
	}

	case 0x7b: case 0x7f:
	{
		u8 op2 = opcodes.r8(pc + bytes++);
		util::stream_format(stream, "%-8sRW%d, ", BIT(op, 2) ? "XCHW" : "MOVW", BIT(op2, 5, 3));
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		break;
	}

	case 0x7c:
	{
		u8 op2 = opcodes.r8(pc + bytes++);
		util::stream_format(stream, "%-8s", "MOV");
		bytes += dasm_ea8(stream, pc + bytes, op2, segm, opcodes);
		util::stream_format(stream, ", R%d", BIT(op2, 5, 3));
		break;
	}

	case 0x7d:
	{
		u8 op2 = opcodes.r8(pc + bytes++);
		util::stream_format(stream, "%-8s", "MOVW");
		bytes += dasm_ea16(stream, pc + bytes, op2, segm, opcodes);
		util::stream_format(stream, ", RW%d", BIT(op2, 5, 3));
		break;
	}

	default:
		util::stream_format(stream, "%-8s0x%02X", ".DATA.B", segm != 0 ? segm : op);
		return 1 | SUPPORTED;
	}

	return bytes | SUPPORTED;
}
