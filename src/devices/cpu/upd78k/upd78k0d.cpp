// license:BSD-3-Clause
// copyright-holders:AJR

// Note: FFD0–FFDF defined as "External access area" on various models but *cannot* use SFR addressing!

#include "emu.h"
#include "upd78k0d.h"

upd78k0_disassembler::upd78k0_disassembler(const char *const sfr_names[], const char *const sfrp_names[])
	: upd78k_8reg_disassembler(sfr_names, sfrp_names)
{
}

const char *const upd78k0_disassembler::s_alu_ops[8] =
{
	"ADD",
	"SUB",
	"ADDC",
	"SUBC",
	"CMP",
	"AND",
	"OR",
	"XOR"
};

const char *const upd78k0_disassembler::s_bool_ops[4] =
{
	"MOV1",
	"AND1",
	"OR1",
	"XOR1"
};

offs_t upd78k0_disassembler::dasm_31(std::ostream &stream, offs_t pc, const upd78k0_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if (op2 < 0x80)
	{
		if ((op2 & 0x0d) == 0x01)
		{
			util::stream_format(stream, "%-8s", BIT(op2, 1) ? "BF" : "BTCLR");
			u8 saddr = opcodes.r8(pc + 2);
			if (saddr == 0x1e)
				util::stream_format(stream, "%s,", s_psw_bits[(op2 & 0x70) >> 4]);
			else
			{
				format_saddr(stream, saddr);
				util::stream_format(stream, ".%d,", (op2 & 0x70) >> 4);
			}
			format_jdisp8(stream, pc + 4, opcodes.r8(pc + 3));
			return 4 | STEP_COND | SUPPORTED;
		}
		else switch (op2 & 0x0f)
		{
		case 0x05:
			util::stream_format(stream, "%-8s", "BTCLR");
			format_sfr(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, ".%d,", (op2 & 0x70) >> 4);
			format_jdisp8(stream, pc + 4, opcodes.r8(pc + 3));
			return 4 | STEP_COND | SUPPORTED;

		case 0x06:
			util::stream_format(stream, "%-8s", "BT");
			format_sfr(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, ".%d,", (op2 & 0x70) >> 4);
			format_jdisp8(stream, pc + 4, opcodes.r8(pc + 3));
			return 4 | STEP_COND | SUPPORTED;

		case 0x07:
			util::stream_format(stream, "%-8s", "BF");
			format_sfr(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, ".%d,", (op2 & 0x70) >> 4);
			format_jdisp8(stream, pc + 4, opcodes.r8(pc + 3));
			return 4 | STEP_COND | SUPPORTED;

		case 0x0d:
			util::stream_format(stream, "%-8sA.%d,", "BTCLR", (op2 & 0x70) >> 4);
			format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
			return 3 | STEP_COND | SUPPORTED;

		case 0x0e:
			util::stream_format(stream, "%-8sA.%d,", "BT", (op2 & 0x70) >> 4);
			format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
			return 3 | STEP_COND | SUPPORTED;

		case 0x0f:
			util::stream_format(stream, "%-8sA.%d,", "BF", (op2 & 0x70) >> 4);
			format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
			return 3 | STEP_COND | SUPPORTED;

		default:
			return dasm_illegal2(stream, 0x31, op2);
		}
	}
	else if ((op2 & 0xef) == 0x80)
	{
		util::stream_format(stream, "%-8s[HL]", BIT(op2, 4) ? "ROR4" : "ROL4");
		return 2 | SUPPORTED;
	}
	else if (op2 == 0x82)
	{
		util::stream_format(stream, "%-8sC", "DIVUW");
		return 2 | SUPPORTED;
	}
	else if (op2 == 0x88)
	{
		util::stream_format(stream, "%-8sX", "MULU");
		return 2 | SUPPORTED;
	}
	else if (op2 == 0x98)
	{
		util::stream_format(stream, "%-8sAX", "BR");
		return 2 | SUPPORTED;
	}
	else switch (op2 & 0x0f)
	{
	case 0x05:
		util::stream_format(stream, "%-8s[HL].%d,", "BTCLR", (op2 & 0x70) >> 4);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;

	case 0x06:
		util::stream_format(stream, "%-8s[HL].%d,", "BT", (op2 & 0x70) >> 4);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;

	case 0x07:
		util::stream_format(stream, "%-8s[HL].%d,", "BF", (op2 & 0x70) >> 4);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;

	case 0x0a: case 0x0b:
		if (op2 < 0x90)
		{
			util::stream_format(stream, "%-8sA,[HL+%s]", op2 < 0x80 ? s_alu_ops[(op2 & 0x70) >> 4] : "XCH", s_r_names[op2 & 0x07]);
			return 2 | SUPPORTED;
		}
		else
			return dasm_illegal2(stream, 0x31, op2);

	default:
		return dasm_illegal2(stream, 0x31, op2);
	}
}

offs_t upd78k0_disassembler::dasm_61(std::ostream &stream, offs_t pc, const upd78k0_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if (op2 < 0x80)
	{
		if ((op2 & 0x0f) != 0x09)
		{
			util::stream_format(stream, "%-8s", s_alu_ops[(op2 & 0x70) >> 4]);
			if (BIT(op2, 3))
				util::stream_format(stream, "A,%s", s_r_names[op2 & 0x07]);
			else
				util::stream_format(stream, "%s,A", s_r_names[op2 & 0x07]);
			return 2 | SUPPORTED;
		}
		else
			return dasm_illegal2(stream, 0x61, op2);
	}
	else if ((op2 & 0xef) == 0x80)
	{
		util::stream_format(stream, "ADJB%c", BIT(op2, 4) ? 'S' : 'A');
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xd7) == 0xd0)
	{
		util::stream_format(stream, "%-8sRB%d", "SEL", (op2 & 0x20) >> 4 | (op2 & 0x08) >> 3);
		return 2 | SUPPORTED;
	}
	else switch (op2 & 0x0f)
	{
	case 0x09:
		util::stream_format(stream, "%-8sA.%d,CY", "MOV1", (op2 & 0x70) >> 4);
		return 2 | SUPPORTED;

	case 0x0a: case 0x0b:
		util::stream_format(stream, "%-8sA.%d", BIT(op2, 0) ? "CLR1" : "SET1", (op2 & 0x70) >> 4);
		return 2 | SUPPORTED;

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		util::stream_format(stream, "%-8sCY,A.%d", s_bool_ops[op2 & 0x03], (op2 & 0x70) >> 4);
		return 2 | SUPPORTED;

	default:
		return dasm_illegal2(stream, 0x61, op2);
	}
}

offs_t upd78k0_disassembler::dasm_71(std::ostream &stream, offs_t pc, const upd78k0_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if (op2 < 0x80)
	{
		if (op2 == 0x00)
		{
			stream << "STOP";
			return 2 | SUPPORTED;
		}
		else if (op2 == 0x10)
		{
			stream << "HALT";
			return 2 | SUPPORTED;
		}
		else switch (op2 & 0x0f)
		{
		case 0x01:
		{
			util::stream_format(stream, "%-8s", "MOV1");
			u8 saddr = opcodes.r8(pc + 2);
			if (saddr == 0x1e)
				stream << s_psw_bits[(op2 & 0x70) >> 4];
			else
			{
				format_saddr(stream, saddr);
				util::stream_format(stream, ".%d", (op2 & 0x70) >> 4);
			}
			stream << ",CY";
			return 3 | SUPPORTED;
		}

		case 0x04: case 0x05: case 0x06: case 0x07:
		{
			util::stream_format(stream, "%-8sCY,", s_bool_ops[op2 & 0x03]);
			u8 saddr = opcodes.r8(pc + 2);
			if (saddr == 0x1e)
				stream << s_psw_bits[(op2 & 0x70) >> 4];
			else
			{
				format_saddr(stream, saddr);
				util::stream_format(stream, ".%d", (op2 & 0x70) >> 4);
			}
			return 3 | SUPPORTED;
		}

		case 0x09:
			util::stream_format(stream, "%-8s", "MOV1");
			format_sfr(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, ".%d,CY", (op2 & 0x70) >> 4);
			return 3 | SUPPORTED;

		case 0x0a: case 0x0b:
			util::stream_format(stream, "%-8s", BIT(op2, 0) ? "CLR1" : "SET1");
			format_sfr(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, ".%d", (op2 & 0x70) >> 4);
			return 3 | SUPPORTED;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "%-8sCY,", s_bool_ops[op2 & 0x03]);
			format_sfr(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, ".%d", (op2 & 0x70) >> 4);
			return 3 | SUPPORTED;

		default:
			return dasm_illegal2(stream, 0x71, op2);
		}
	}
	else switch (op2 & 0x0f)
	{
	case 0x01:
		util::stream_format(stream, "%-8s[HL].%d,CY", "MOV1", (op2 & 0x70) >> 4);
		return 2 | SUPPORTED;

	case 0x02: case 0x03:
		util::stream_format(stream, "%-8s[HL].%d", BIT(op2, 0) ? "CLR1" : "SET1", (op2 & 0x70) >> 4);
		return 2 | SUPPORTED;

	case 0x04: case 0x05: case 0x06: case 0x07:
		util::stream_format(stream, "%-8sCY,[HL].%d", s_bool_ops[op2 & 0x03], (op2 & 0x70) >> 4);
		return 2 | SUPPORTED;

	default:
		return dasm_illegal2(stream, 0x71, op2);
	}
}

offs_t upd78k0_disassembler::disassemble(std::ostream &stream, offs_t pc, const upd78k0_disassembler::data_buffer &opcodes, const upd78k0_disassembler::data_buffer &params)
{
	u8 op = opcodes.r8(pc);
	switch (op)
	{
	case 0x00:
		stream << "NOP";
		return 1 | SUPPORTED;

	case 0x01:
		util::stream_format(stream, "%-8sCY", "NOT1");
		return 1 | SUPPORTED;

	case 0x02:
		util::stream_format(stream, "%-8sAX,", "MOVW");
		format_abs16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x03:
		util::stream_format(stream, "%-8s", "MOVW");
		format_abs16(stream, opcodes.r16(pc + 1));
		stream << ",AX";
		return 3 | SUPPORTED;

	case 0x04:
		util::stream_format(stream, "%-8s", "DBNZ");
		format_saddr(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;

	case 0x05: case 0x07:
		util::stream_format(stream, "%-8sA,[%s]", "XCH", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0x08: case 0x18: case 0x28: case 0x38: case 0x48: case 0x58: case 0x68: case 0x78:
		util::stream_format(stream, "%-8sA,", s_alu_ops[(op & 0x70) >> 4]);
		format_abs16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x09: case 0x19: case 0x29: case 0x39: case 0x49: case 0x59: case 0x69: case 0x79:
		util::stream_format(stream, "%-8sA,", s_alu_ops[(op & 0x70) >> 4]);
		format_ix_disp8(stream, "HL", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x0a: case 0x1a: case 0x2a: case 0x3a: case 0x4a: case 0x5a: case 0x6a: case 0x7a:
	case 0x0b: case 0x1b: case 0x2b: case 0x3b: case 0x4b: case 0x5b: case 0x6b: case 0x7b:
	{
		u8 saddr = opcodes.r8(pc + 1);
		if ((op & 0x70) == 0x70 && saddr == 0x1e)
			util::stream_format(stream, "%cI", BIT(op, 0) ? 'D' : 'E');
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "CLR1" : "SET1");
			if (saddr == 0x1e)
				stream << s_psw_bits[(op & 0x70) >> 4];
			else
			{
				format_saddr(stream, saddr);
				util::stream_format(stream, ".%d", (op & 0x70) >> 4);
			}
		}
		return 2 | SUPPORTED;
	}

	case 0x0c: case 0x1c: case 0x2c: case 0x3c: case 0x4c: case 0x5c: case 0x6c: case 0x7c:
		util::stream_format(stream, "%-8s", "CALLF");
		format_abs16(stream, 0x0800 | u16(op & 0x70) << 4 | opcodes.r8(pc + 1));
		return 2 | STEP_OVER | SUPPORTED;

	case 0x0d: case 0x1d: case 0x2d: case 0x3d: case 0x4d: case 0x5d: case 0x6d: case 0x7d:
		util::stream_format(stream, "%-8sA,", s_alu_ops[(op & 0x70) >> 4]);
		format_imm8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x0e: case 0x1e: case 0x2e: case 0x3e: case 0x4e: case 0x5e: case 0x6e: case 0x7e:
		util::stream_format(stream, "%-8sA,", s_alu_ops[(op & 0x70) >> 4]);
		format_saddr(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x0f: case 0x1f: case 0x2f: case 0x3f: case 0x4f: case 0x5f: case 0x6f: case 0x7f:
		util::stream_format(stream, "%-8sA,[HL]", s_alu_ops[(op & 0x70) >> 4]);
		return 1 | SUPPORTED;

	case 0x10: case 0x12: case 0x14: case 0x16:
		util::stream_format(stream, "%-8s%s,", "MOVW", s_rp_names[(op & 0x06) >> 1]);
		format_imm16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x11:
	{
		util::stream_format(stream, "%-8s", "MOV");
		u8 saddr = opcodes.r8(pc + 1);
		if (saddr == 0x1e)
			stream << "PSW";
		else
			format_saddr(stream, saddr);
		stream << ",";
		format_imm8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}

	case 0x13:
		util::stream_format(stream, "%-8s", "MOV");
		format_sfr(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_imm8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x20: case 0x21:
		util::stream_format(stream, "%-8sCY", BIT(op, 0) ? "CLR1" : "SET1");
		return 1 | SUPPORTED;

	case 0x22: case 0x23:
		util::stream_format(stream, "%-8sPSW", BIT(op, 0) ? "POP" : "PUSH");
		return 1 | SUPPORTED;

	case 0x24: case 0x26:
		util::stream_format(stream, "%-8sA,1", BIT(op, 1) ? "ROL" : "ROR");
		return 1 | SUPPORTED;

	case 0x25: case 0x27:
		util::stream_format(stream, "%-8sA,1", BIT(op, 1) ? "ROLC" : "RORC");
		return 1 | SUPPORTED;

	case 0x30: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		util::stream_format(stream, "%-8sA,%s", "XCH", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0x31:
		return dasm_31(stream, pc, opcodes);

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		util::stream_format(stream, "%-8s%s", BIT(op, 4) ? "DEC" : "INC", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0x60: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		util::stream_format(stream, "%-8sA,%s", "MOV", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0x61:
		return dasm_61(stream, pc, opcodes);

	case 0x70: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		util::stream_format(stream, "%-8s%s,A", "MOV", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0x71:
		return dasm_71(stream, pc, opcodes);

	case 0x80: case 0x82: case 0x84: case 0x86:
	case 0x90: case 0x92: case 0x94: case 0x96:
		util::stream_format(stream, "%-8s%s", BIT(op, 4) ? "DECW" : "INCW", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0x81: case 0x91:
		util::stream_format(stream, "%-8s", BIT(op, 4) ? "DEC" : "INC");
		format_saddr(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x83: case 0x93:
		util::stream_format(stream, "%-8sA,", "XCH");
		if (BIT(op, 4))
			format_sfr(stream, opcodes.r8(pc + 1));
		else
			format_saddr(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x85: case 0x87:
		util::stream_format(stream, "%-8sA,[%s]", "MOV", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0x88: case 0x98: case 0xa8: case 0xb8: case 0xc8: case 0xd8: case 0xe8: case 0xf8:
		util::stream_format(stream, "%-8s", s_alu_ops[(op & 0x70) >> 4]);
		format_saddr(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_imm8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x89:
	{
		util::stream_format(stream, "%-8sAX,", "MOVW");
		u8 saddrp = opcodes.r8(pc + 1);
		if (saddrp == 0x1c)
			stream << "SP";
		else
			format_saddrp(stream, saddrp);
		return 2 | SUPPORTED;
	}

	case 0x8a: case 0x8b:
		util::stream_format(stream, "%-8s%s,", "DBNZ", s_r_names[op & 0x07]);
		format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
		return 2 | STEP_COND | SUPPORTED;

	case 0x8c: case 0x9c: case 0xac: case 0xbc: case 0xcc: case 0xdc: case 0xec: case 0xfc:
	{
		util::stream_format(stream, "%-8s", "BT");
		u8 saddr = opcodes.r8(pc + 1);
		if (saddr == 0x1e)
			util::stream_format(stream, "%s,", s_psw_bits[(op & 0x70) >> 4]);
		else
		{
			format_saddr(stream, saddr);
			util::stream_format(stream, ".%d,", (op & 0x70) >> 4);
		}
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;
	}

	case 0x8d: case 0x9d:
		util::stream_format(stream, "%-8s", BIT(op, 4) ? "BNC" : "BC");
		format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
		return 2 | STEP_COND | SUPPORTED;

	case 0x8e:
		util::stream_format(stream, "%-8sA,", "MOV");
		format_abs16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x8f: case 0x9f:
		util::stream_format(stream, "RET%c", BIT(op, 4) ? 'B' : 'I');
		return 2 | STEP_OUT | SUPPORTED;

	case 0x95: case 0x97:
		util::stream_format(stream, "%-8s[%s],A", "MOV", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0x99:
	{
		util::stream_format(stream, "%-8s", "MOVW");
		u8 saddrp = opcodes.r8(pc + 1);
		if (saddrp == 0x1c)
			stream << "SP";
		else
			format_saddrp(stream, saddrp);
		stream << ",AX";
		return 2 | SUPPORTED;
	}

	case 0x9a:
		util::stream_format(stream, "%-8s", "CALL");
		format_abs16(stream, opcodes.r16(pc + 1));
		return 3 | STEP_OVER | SUPPORTED;

	case 0x9b:
		util::stream_format(stream, "%-8s", "BR");
		format_abs16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x9e:
		util::stream_format(stream, "%-8s", "MOV");
		format_abs16(stream, opcodes.r16(pc + 1));
		stream << ",A";
		return 3 | SUPPORTED;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		util::stream_format(stream, "%-8s%s,", "MOV", s_r_names[op & 0x07]);
		format_imm8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xa9:
		util::stream_format(stream, "%-8sAX,", "MOVW");
		format_sfrp(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xaa: case 0xab:
		util::stream_format(stream, "%-8sA,[HL+%s]", "MOV", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0xad: case 0xbd:
		util::stream_format(stream, "%-8s", BIT(op, 4) ? "BNZ" : "BZ");
		format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
		return 2 | STEP_COND | SUPPORTED;

	case 0xae:
		util::stream_format(stream, "%-8sA,", "MOV");
		format_ix_disp8(stream, "HL", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xaf:
		stream << "RET";
		return 1 | STEP_OUT | SUPPORTED;

	case 0xb0: case 0xb2: case 0xb4: case 0xb6:
		util::stream_format(stream, "%-8s%s", "POP", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0xb1: case 0xb3: case 0xb5: case 0xb7:
		util::stream_format(stream, "%-8s%s", "PUSH", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0xb9:
		util::stream_format(stream, "%-8s", "MOVW");
		format_sfrp(stream, opcodes.r8(pc + 1));
		stream << ",AX";
		return 2 | SUPPORTED;

	case 0xba: case 0xbb:
		util::stream_format(stream, "%-8s[HL+%s],A", "MOV", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0xbe:
		util::stream_format(stream, "%-8s", "MOV");
		format_ix_disp8(stream, "HL", opcodes.r8(pc + 1));
		stream << ",A";
		return 2 | SUPPORTED;

	case 0xbf:
		stream << "BRK";
		return 1 | STEP_OVER | SUPPORTED;

	case 0xc1: case 0xc3: case 0xc5: case 0xc7: case 0xc9: case 0xcb: case 0xcd: case 0xcf:
	case 0xd1: case 0xd3: case 0xd5: case 0xd7: case 0xd9: case 0xdb: case 0xdd: case 0xdf:
	case 0xe1: case 0xe3: case 0xe5: case 0xe7: case 0xe9: case 0xeb: case 0xed: case 0xef:
	case 0xf1: case 0xf3: case 0xf5: case 0xf7: case 0xf9: case 0xfb: case 0xfd: case 0xff:
		util::stream_format(stream, "%-8s[%04XH]", "CALLT", u16(op & 0x7e));
		return 1 | STEP_OVER | SUPPORTED;

	case 0xc2: case 0xc4: case 0xc6:
	case 0xe2: case 0xe4: case 0xe6:
		util::stream_format(stream, "%-8sAX,%s", BIT(op, 5) ? "XCHW" : "MOVW", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0xca:
		util::stream_format(stream, "%-8sAX,", "ADDW");
		format_imm16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0xce:
		util::stream_format(stream, "%-8sA,", "XCH");
		format_abs16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0xd2: case 0xd4: case 0xd6:
		util::stream_format(stream, "%-8s%s,AX", "MOVW", s_rp_names[(op & 0x06) >> 1]);
		return 1 | SUPPORTED;

	case 0xda:
		util::stream_format(stream, "%-8sAX,", "SUBW");
		format_imm16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0xde:
		util::stream_format(stream, "%-8s", "XCH");
		format_ix_disp8(stream, "HL", opcodes.r8(pc + 1));
		stream << ",A";
		return 2 | SUPPORTED;

	case 0xea:
		util::stream_format(stream, "%-8sAX,", "CMPW");
		format_imm16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0xee:
	{
		util::stream_format(stream, "%-8s", "MOVW");
		u8 saddrp = opcodes.r8(pc + 1);
		if (saddrp == 0x1c)
			stream << "SP";
		else
			format_saddrp(stream, saddrp);
		stream << ",";
		format_imm16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;
	}

	case 0xf0:
	{
		util::stream_format(stream, "%-8sA,", "MOV");
		u8 saddr = opcodes.r8(pc + 1);
		if (saddr == 0x1e)
			stream << "PSW";
		else
			format_saddr(stream, saddr);
		return 2 | SUPPORTED;
	}

	case 0xf2:
	{
		util::stream_format(stream, "%-8s", "MOV");
		u8 saddr = opcodes.r8(pc + 1);
		if (saddr == 0x1e)
			stream << "PSW";
		else
			format_saddr(stream, saddr);
		stream << ",A";
		return 2 | SUPPORTED;
	}

	case 0xf4:
		util::stream_format(stream, "%-8sA,", "MOV");
		format_sfr(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xf6:
		util::stream_format(stream, "%-8s", "MOV");
		format_sfr(stream, opcodes.r8(pc + 1));
		stream << ",A";
		return 2 | SUPPORTED;

	case 0xfa:
		util::stream_format(stream, "%-8s", "BR");
		format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xfe:
		util::stream_format(stream, "%-8s", "MOVW");
		format_sfrp(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_imm16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	default:
		return dasm_illegal(stream, op);
	}
}

const char *const upd78k0_disassembler::s_common_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", "TM0", nullptr, "TMS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78014_disassembler::upd78014_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_common_sfrp_names)
{
}

const char *const upd78014_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CR10", "CR20",
	"TM1", "TM2", "SIO0", "SIO1", nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", "PM2", "PM3", nullptr, "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TCL0", "TCL1", "TCL2", "TCL3", nullptr, nullptr, nullptr, "SCS",
	"TMC0", "TMC1", "TMC2", nullptr, nullptr, nullptr, "TOC0", "TOC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "SBIC", "SVA", "SINT", nullptr, nullptr, nullptr, nullptr,
	"CSIM1", "ADTC", "ADTP", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "INTM0", nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, nullptr, nullptr, "KRM", "PUO",
	"MM", "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

upd78024_disassembler::upd78024_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_common_sfrp_names)
{
}

const char *const upd78024_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", nullptr, nullptr, nullptr, nullptr,
	"P8", "P9", "P10", "P11", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CR10", "CR20",
	"TM1", "TM2", "SIO0", "SIO1", nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", "PM2", "PM3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "PM11", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TCL0", "TCL1", "TCL2", "TCL3", nullptr, nullptr, nullptr, "SCS",
	"TMC0", "TMC1", "TMC2", nullptr, nullptr, nullptr, "TOC0", "TOC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "SBIC", "SVA", "SINT", nullptr, nullptr, nullptr, nullptr,
	"CSIM1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"DSPM0", "DSPM1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "INTM0", nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PUO",
	nullptr, "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

upd78044a_disassembler::upd78044a_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_common_sfrp_names)
{
}

const char *const upd78044a_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", nullptr, nullptr, nullptr, "P7",
	"P8", "P9", "P10", "P11", "P12", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CR10", "CR20",
	"TM1", "TM2", "SIO0", "SIO1", nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", "PM2", "PM3", nullptr, nullptr, nullptr, "PM7",
	nullptr, nullptr, nullptr, "PM11", "PM12", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TCL0", "TCL1", "TCL2", "TCL3", nullptr, nullptr, nullptr, "SCS",
	"TMC0", "TMC1", "TMC2", nullptr, nullptr, nullptr, "TOC0", "TOC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "SBIC", "SVA", "SINT", nullptr, nullptr, nullptr, nullptr,
	"CSIM1", "ADTC", "ADTP", "ADTI", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"DSPM0", "DSPM1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"UDM", "UDC", "UDCC", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "INTM0", nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, "IXS", nullptr, nullptr, "PUO",
	nullptr, "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

upd78054_disassembler::upd78054_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_common_sfrp_names)
{
}

const char *const upd78054_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, nullptr, nullptr, "P12", "P13", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CR10", "CR20",
	"TM1", "TM2", "SIO0", "SIO1", nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", "PM2", "PM3", nullptr, "PM5", "PM6", "PM7",
	nullptr, nullptr, nullptr, nullptr, "PM12", "PM13", nullptr, nullptr,
	"RTBL", "RTBH", nullptr, nullptr, "RTPM", nullptr, "RTPC", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TCL0", "TCL1", "TCL2", "TCL3", nullptr, nullptr, nullptr, "SCS",
	"TMC0", "TMC1", "TMC2", nullptr, "CRC0", nullptr, "TOC0", "TOC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "SBIC", "SVA", "SINT", nullptr, nullptr, nullptr, nullptr,
	"CSIM1", "ADTC", "ADTP", "ADTI", nullptr, nullptr, nullptr, nullptr,
	"ASIM", "ASIS", "CSIM2", "BRGC", "SIO2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"DACS0", "DACS1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"DAM", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IMS", nullptr, "OSMS", "PUOH", nullptr, nullptr, "KRM", "PUOL",
	"MM", "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

upd78064_disassembler::upd78064_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_common_sfrp_names)
{
}

const char *const upd78064_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", nullptr, nullptr, nullptr, "P7",
	"P8", "P9", "P10", "P11", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CR10", "CR20",
	"TM1", "TM2", "SIO0", "SIO1", nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", "PM2", "PM3", nullptr, nullptr, nullptr, "PM7",
	"PM8", "PM9", "PM10", "PM11", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TCL0", "TCL1", "TCL2", "TCL3", nullptr, nullptr, nullptr, "SCS",
	"TMC0", "TMC1", "TMC2", nullptr, "CRC0", nullptr, "TOC0", "TOC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "SBIC", "SVA", "SINT", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ASIM", "ASIS", "CSIM2", "BRGC", "SIO2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"LDCM", nullptr, "LCDC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"KRM", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IMS", nullptr, "OSMS", "PUOH", nullptr, nullptr, nullptr, "PUOL",
	nullptr, "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

upd78078_disassembler::upd78078_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78078_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", "P10", nullptr, "P12", "P13", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CR10", "CR20",
	"TM1", "TM2", "SIO0", "SIO1", nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", "PM2", "PM3", nullptr, "PM5", "PM6", "PM7",
	"PM8", "PM9", "PM10", nullptr, "PM12", "PM13", nullptr, nullptr,
	"RTBL", "RTBH", nullptr, nullptr, "RTPM", nullptr, "RTPC", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "EBTS",
	"TCL0", "TCL1", "TCL2", "TCL3", nullptr, nullptr, nullptr, "SCS",
	"TMC0", "TMC1", "TMC2", nullptr, "CRC0", nullptr, "TOC0", "TOC1",
	"CR50", "TM5", "TCL5", "TMC5", "CR60", "TM6", "TCL6", "TMC6",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "SBIC", "SVA", "SINT", nullptr, nullptr, nullptr, nullptr,
	"CSIM1", "ADTC", "ADTP", "ADTI", nullptr, nullptr, nullptr, nullptr,
	"ASIM", "ASIS", "CSIM2", "BRGC", "SIO2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, "CORCN", nullptr, nullptr, nullptr, nullptr, nullptr,
	"DACS0", "DACS1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"DAM", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IMS", nullptr, "OSMS", "PUOH", "IXS", nullptr, "KRM", "PUOL",
	"MM", "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd78078_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", "TM0", nullptr, "TMS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CORAD0", "CORAD1", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78083_disassembler::upd78083_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78083_disassembler::s_sfr_names[256] =
{
	"P0", "P1", nullptr, "P3", nullptr, "P5", nullptr, "P7",
	nullptr, nullptr, "P10", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ADCR",
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", nullptr, "PM7",
	nullptr, nullptr, "PM10", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TCL0", nullptr, "TCL2", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR50", "TM5", "TCL5", "TMC5", "CR60", "TM6", "TCL6", "TMC6",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ASIM", "ASIS", "CSIM2", "BRGC", "SIO2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, "ADIS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IMS", nullptr, "OSMS", "PUOH", nullptr, nullptr, nullptr, "PUOL",
	nullptr, "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd78083_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd780024a_disassembler::upd780024a_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd780024a_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", nullptr, "P7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR50", "CR51", "TM50", "TM51", nullptr, nullptr, nullptr, nullptr,
	"SIO0", nullptr, "SIO30", "SIO31", nullptr, nullptr, nullptr, nullptr,
	"PM0", nullptr, "PM2", "PM3", "PM4", "PM5", nullptr, "PM7",
	nullptr, nullptr, nullptr, "PM11", nullptr, nullptr, nullptr, nullptr,
	"PU0", nullptr, "PU2", "PU3", "PU4", "PU5", nullptr, "PU7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CKS", "WTM", "WDCS", nullptr, nullptr, nullptr, nullptr, "MEM",
	"EGP", "EGN", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", "PRM0", "CRC0", "TOC0", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC50", "TCL50", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC51", "TCL51", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM0", "ADS0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ASIM0", "ASIM1", "BRGC0", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM30", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM31", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, nullptr, nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd780024a_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, "CR00", "CR01", "TM0",
	nullptr, "TM5", nullptr, "ADCR0", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd780065_disassembler::upd780065_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd780065_disassembler::s_sfr_names[256] =
{
	"P0", nullptr, "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR50", "CR51", "TM50", "TM51", nullptr, "ADCR0", nullptr, "SIO1",
	"SIO30", "SIO31", "SIO0", nullptr, nullptr, nullptr, nullptr, "ADCR",
	"PM0", nullptr, "PM2", "PM3", "PM4", "PM5", "PM6", "PM7",
	"PM8", "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PU0", nullptr, "PU2", "PU3", "PU4", "PU5", "PU6", "PU7",
	"PU8", "PU9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CKS", "WTM", "WDCS", nullptr, nullptr, nullptr, nullptr, "MEM",
	"EGP", "EGN", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", "PRM0", "CRC0", "TOC0", nullptr, nullptr, nullptr, nullptr,
	"CSIM1", "ADTC0", "ADTP0", "ADTI0", nullptr, nullptr, nullptr, nullptr,
	"TMC50", "TCL50", "TMC51", "TCL51", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM0", "ADS0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ASIM0", "ASIS1", "BRGC0", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM0", "CSIM31", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, nullptr, nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, "IXS", nullptr, nullptr, nullptr,
	"MM", "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd780065_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, "CR00", "CR01", "TM0",
	nullptr, "TM5", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CORAD0", "CORAD1", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd780988_disassembler::upd780988_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd780988_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "TM52",
	"BFCM0L", nullptr, "BFCM1L", nullptr, "BFCM2L", nullptr, "BFCM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "TM50", "TM51", "CR50", "CR51",
	nullptr, nullptr, "SIO00", "SIO01", nullptr, nullptr, nullptr, "SIO3",
	"PM0", nullptr, "PM2", "PM3", "PM4", "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PU0", nullptr, "PU2", "PU3", "PU4", "PU5", "PU6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "WDCS", nullptr, nullptr, nullptr, nullptr, "MEM",
	"EGP", "EGN", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC00", "PRM00", "CRC00", "TOC00", nullptr, nullptr, nullptr, nullptr,
	"TMC01", "PRM01", "CRC01", "TOC01", nullptr, nullptr, nullptr, nullptr,
	"TMC50", "TCL50", nullptr, nullptr, "TMC51", "TCL51", nullptr, nullptr,
	"TMC52", "TCL52", "CR52", nullptr, "EGP5", "EGN5", nullptr, nullptr,
	"ADM0", "ADS0", nullptr, nullptr, "RTBL00", "RTBH00", "RTPM00", "RTPC00",
	nullptr, "FLPMC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC7", "TMM7", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "DTIME", nullptr, "RTBL01", "RTBH01", "RTPM01", "RTPC01",
	"ASIM00", "ASIS00", "BRGC00", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ASIM01", "ASIS01", "BRGC01", nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM3", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"DCCTL0", nullptr, nullptr, nullptr, "DCCTL1", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, nullptr, nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, "IXS", nullptr, nullptr, nullptr,
	"MM", "WDTM", "OSTS", "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd780988_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, "BFCM0", "BFCM1", "BFCM2", "BFCM3",
	"TM00", "TM01", "TM5", "CR5", "ADCR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "CR000", "CR010", nullptr, nullptr, "CR001", "CR011",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "CM0", "CM1", "CM2", "CM3", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78k0kx1_disassembler::upd78k0kx1_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78k0kx1_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, "RXB6", "TXB6", "P12", "P13", "P14", "SIO10",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TM50", "CR50",
	"CMP00", "CMP10", "CMP01", "CMP11", nullptr, nullptr, nullptr, "TM51",
	"PM0", "PM1", nullptr, "PM3", "PM4", "PM5", "PM6", "PM7",
	"ADM", "ADS", "PFM", "PFT", "PM12", nullptr, "PM14", nullptr,
	"PU0", "PU1", nullptr, "PU3", "PU4", "PU5", "PU6", "PU7",
	nullptr, nullptr, nullptr, nullptr, "PU12", nullptr, "PU14", nullptr,
	"CKS", "CR51", nullptr, "TMC51", nullptr, nullptr, nullptr, "MEM",
	"EGP", "EGN", "SIO11", nullptr, "SOTB11", nullptr, nullptr, "ISC",
	"ASIM6", nullptr, nullptr, "ASIS6", nullptr, "ASIF6", "CKSR6", "BRGC6",
	"ASICL6", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"SDR0L", "SDR0H", "MDA0LL", "MDA0LH", "MDA0HL", "MDA0HH", "MDB0L", "MDB0H",
	"DMUC0", "TMHMD0", "TCL50", "TMC50", "TMHMD1", "TMCYC1", "KRM", "WTM",
	"ASIM0", "BRGC0", "RXB0", "ASIS0", "TXS0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM10", "CSIC10", nullptr, nullptr, "SOTB10", nullptr, nullptr, nullptr,
	"CSIM11", "CSIC11", nullptr, nullptr, "TCL51", nullptr, nullptr, nullptr,
	"CSIMA0", "CSIS0", "CSIT0", "BRGAC0", "ADTP0", "ADTI0", "SIOA0", "ADTC3",
	"WDTM", "WDTE", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"RCM", "MCM", "MOC", "OSTC", "OSTS", nullptr, nullptr, nullptr,
	nullptr, "CLM", nullptr, nullptr, "RESF", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TMC01", "PRM01",
	"CRC01", "TOC01", "TMC00", "PRM00", "CRC00", "TOC00", "LVIM", "LVIS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", "IF1H", "MK0L", "MK0H", "MK1L", "MK1H",
	"PR0L", "PR0H", "PR1L", "PR1H", nullptr, nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, nullptr, "IXS", nullptr, nullptr, nullptr,
	"MM", nullptr, nullptr, "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd78k0kx1_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, "ADCR", nullptr, nullptr, nullptr,
	"TM00", "CR000", "CR010", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"SDR0", "MDA0L", "MDA0H", "MDB0", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM01", "CR001", "CR011", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", "IF1", "MK0", "MK1", "PR0", "PR1", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78k0kx2_disassembler::upd78k0kx2_disassembler()
	: upd78k0_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78k0kx2_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, "ADCRH", "RXB6", "TXB6", "P12", "P13", "P14", "SIO10",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TM50", "CR50",
	"CMP00", "CMP10", "CMP01", "CMP11", nullptr, nullptr, nullptr, "TM51",
	"PM0", "PM1", "PM2", "PM3", "PM4", "PM5", "PM6", "PM7",
	"ADM", "ADS", nullptr, nullptr, "PM12", nullptr, "PM14", "ADPC",
	"PU0", "PU1", nullptr, "PU3", "PU4", "PU5", "PU6", "PU7",
	nullptr, nullptr, nullptr, nullptr, "PU12", nullptr, "PU14", nullptr,
	"CKS", "CR51", nullptr, "TMC51", nullptr, nullptr, nullptr, nullptr,
	"EGP", "EGN", "SIO11", nullptr, "SOTB11", nullptr, nullptr, "ISC",
	"ASIM6", nullptr, nullptr, "ASIS6", nullptr, "ASIF6", "CKSR6", "BRGC6",
	"ASICL6", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"SDR0L", "SDR0H", "MDA0LL", "MDA0LH", "MDA0HL", "MDA0HH", "MDB0L", "MDB0H",
	"DMUC0", "TMHMD0", "TCL50", "TMC50", "TMHMD1", "TMCYC1", "KRM", "WTM",
	"ASIM0", "BRGC0", "RXB0", "ASIS0", "TXS0", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM10", "CSIC10", nullptr, nullptr, "SOTB10", nullptr, nullptr, nullptr,
	"CSIM11", "CSIC11", nullptr, nullptr, "TCL51", nullptr, nullptr, nullptr,
	"CSIMA0", "CSIS0", "CSIT0", "BRGAC0", "ADTP0", "ADTI0", "SIOA0", "ADTC3",
	nullptr, "WDTE", nullptr, nullptr, nullptr, nullptr, nullptr, "OSCCTL",
	"RCM", "MCM", "MOC", "OSTC", "OSTS", "IIC0", "IICC0", "SVA0",
	"IICCL0", "IICX0", "IICS0", "IICF0", "RESF", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TMC01", "PRM01",
	"CRC01", "TOC01", "TMC00", "PRM00", "CRC00", "TOC00", "LVIM", "LVIS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", "IF1H", "MK0L", "MK0H", "MK1L", "MK1H",
	"PR0L", "PR0H", "PR1L", "PR1H", nullptr, nullptr, nullptr, nullptr,
	"IMS", nullptr, nullptr, "BANK", "IXS", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "PCC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd78k0kx2_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, "ADCR", nullptr, nullptr, nullptr,
	"TM00", "CR000", "CR010", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"SDR0", "MDA0L", "MDA0H", "MDB0", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM01", "CR001", "CR011", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", "IF1", "MK0", "MK1", "PR0", "PR1", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};
