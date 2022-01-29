// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Fujitsu FR family disassembler

    While Fujitsu's manuals also refer to R13, R14 and R15 by function as
    AC (accumulator), FP (frame pointer) and SP (stack pointer), these
    aliases do not appear to be commonly used. All indexed addressing modes
    are based on one of these three registers.

    The PS (program status) register can only be manipulated with special
    instructions, some of which affect only the sections known as CCR
    (condition code register) and ILM (interrupt level mask).

    TODO: The FR81 family has extra opcodes (primarily floating-point
    instructions) which are not included here.

***************************************************************************/

#include "emu.h"
#include "frdasm.h"

fr_disassembler::fr_disassembler()
	: util::disasm_interface()
{
}

u32 fr_disassembler::opcode_alignment() const
{
	return 2;
}

void fr_disassembler::format_u8(std::ostream &stream, u8 value)
{
	util::stream_format(stream, "#0x%02X", value);
}

void fr_disassembler::format_i8(std::ostream &stream, u8 value)
{
	if (value >= 0x0a)
		util::stream_format(stream, "#0x%X", value);
	else
		util::stream_format(stream, "#%d", value);
}

void fr_disassembler::format_disp(std::ostream &stream, u8 offs, unsigned shift)
{
	// 10-bit (word), 9-bit (half-word) or 8-bit (byte) signed displacement
	u16 disp;
	if (BIT(offs, 7))
	{
		disp = u16(0x100 - offs) << shift;
		stream << "-";
	}
	else
		disp = u16(offs) << shift;

	if (disp >= 0x0a)
		util::stream_format(stream, "0x%X", disp);
	else
		util::stream_format(stream, "%d", disp);
}

void fr_disassembler::format_u10(std::ostream &stream, u16 value)
{
	if (value >= 0x0a)
		util::stream_format(stream, "#0x%X", value);
	else
		util::stream_format(stream, "#%d", value);
}

void fr_disassembler::format_i20(std::ostream &stream, u32 value)
{
	util::stream_format(stream, "#0x%05X", value);
}

void fr_disassembler::format_i32(std::ostream &stream, u32 value)
{
	util::stream_format(stream, "#0x%08X", value);
}

void fr_disassembler::format_label(std::ostream &stream, offs_t addr)
{
	util::stream_format(stream, "0x%X", u32(addr));
}

void fr_disassembler::format_dir(std::ostream &stream, u16 addr)
{
	if (addr >= 0x100)
		util::stream_format(stream, "@0x%03X", addr);
	else
		util::stream_format(stream, "@0x%02X", addr);
}

void fr_disassembler::format_ac_rdisp(std::ostream &stream, u8 rj)
{
	util::stream_format(stream, "@(R13, R%d)", rj);
}

void fr_disassembler::format_sp_udisp(std::ostream &stream, u8 disp)
{
	// unsigned displacement
	util::stream_format(stream, "@(R15, %d)", disp);
}

void fr_disassembler::format_rs(std::ostream &stream, u8 reg)
{
	switch (reg)
	{
	case 0: // Table Base Register
		stream << "TBR";
		break;

	case 1: // Return Pointer
		stream << "RP";
		break;

	case 2: // System Stack Pointer (R15 when S = 0)
		stream << "SSP";
		break;

	case 3: // User Stack Pointer (R15 when S = 1)
		stream << "USP";
		break;

	case 4: // Multiply/Divide Register (high 32 bits)
		stream << "MDH";
		break;

	case 5: // Multiply/Divide Register (low 32 bits)
		stream << "MDL";
		break;

	default:
		stream << "reserved";
		break;
	}
}

offs_t fr_disassembler::dasm_invalid(std::ostream &stream, u16 opcode)
{
	util::stream_format(stream, "%-8s0x%04X", ".DATA.H", opcode);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_i4op(std::ostream &stream, u16 opcode, const char *inst)
{
	// Arithmetic instructions with 4-bit (effectively 5-bit) positive or negative immediate offset
	util::stream_format(stream, "%-8s#", inst);
	u8 value = (opcode & 0x00f0) >> 4;
	if (BIT(opcode, 9))
	{
		stream << "-";
		value = 0x10 - value;
	}
	if (value >= 0x0a)
		util::stream_format(stream, "0x%X", value);
	else
		util::stream_format(stream, "%d", value);

	util::stream_format(stream, ", R%d", opcode & 0x000f);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_shift(std::ostream &stream, u16 opcode, const char *inst)
{
	// Register shifts with 4-bit (effectively 5-bit) immediate offset
	util::stream_format(stream, "%-8s#%d, R%d", inst, (opcode & 0x01f0) >> 4, opcode & 0x000f);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_rrop(std::ostream &stream, u16 opcode, const char *inst)
{
	// Simple register-to-register operations (and multiplication)
	util::stream_format(stream, "%-8sR%d, R%d", inst, (opcode & 0x00f0) >> 4, opcode & 0x000f);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_ld_fp_disp(std::ostream &stream, u16 opcode, const char *inst, unsigned shift)
{
	// Load (zero extended) from memory at frame pointer ± 10-bit/9-bit/8-bit displacement
	util::stream_format(stream, "%-8s@(R14, ", inst);
	format_disp(stream, (opcode & 0x0ff0) >> 4, shift);
	util::stream_format(stream, "), R%d", opcode & 0x000f);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_st_fp_disp(std::ostream &stream, u16 opcode, const char *inst, unsigned shift)
{
	// Store to memory at frame pointer ± 10-bit/9-bit/8-bit displacement
	util::stream_format(stream, "%-8sR%d, @(R14, ", inst, opcode & 0x000f);
	format_disp(stream, (opcode & 0x0ff0) >> 4, shift);
	stream << ")";
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_ldstm(std::ostream &stream, u16 opcode, const char *inst)
{
	// Load/store register list in (R0-R7) or (R8-R15)
	util::stream_format(stream, "%-8s(", inst);
	int base = BIT(opcode, 8) ? 8 : 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(opcode, i))
		{
			util::stream_format(stream, "R%d", BIT(opcode, 9) ? base + 7 - i : base + i);
			opcode &= ~(1 << i);
			if ((opcode & 0x00ff) != 0)
				stream << ", ";
			else
				break;
		}
	}
	stream << ")";
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_bop(std::ostream &stream, u16 opcode, const char *inst)
{
	// Operands are 4-bit immediate mask and upper or lower nibble of memory
	util::stream_format(stream, "%-8s#0x%X, @R%d", inst, (opcode & 0x00f0) >> 4, opcode & 0x000f);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_cop(std::ostream &stream, u16 op1, u16 op2, const char *inst, bool crj, bool cri)
{
	// Coprocessor instructions
	util::stream_format(stream, "%-8s#%d, ", inst, op1 & 0x000f);
	format_u8(stream, (op2 & 0xff00) >> 8);
	util::stream_format(stream, ", %sR%d, %sR%d", crj ? "C" : "", (op2 & 0x00f0) >> 4, cri ? "C" : "", op2 & 0x000f);
	return 4 | SUPPORTED;
}

offs_t fr_disassembler::dasm_call(std::ostream &stream, offs_t pc, const char *inst, u16 disp)
{
	// 12-bit relative calls, with or without delay slot
	util::stream_format(stream, "%-8s", inst);
	if (disp >= 0x800)
		format_label(stream, pc + 2 - 0x1000 + disp);
	else
		format_label(stream, pc + 2 + disp);
	return 2 | SUPPORTED | STEP_OVER;
}

offs_t fr_disassembler::dasm_branch(std::ostream &stream, offs_t pc, const char *inst, u16 disp)
{
	// Conditional/unconditional 9-bit relative branches, with or without delay slot
	util::stream_format(stream, "%-8s", inst);
	if (disp >= 0x100)
		format_label(stream, pc + 2 - 0x200 + disp);
	else
		format_label(stream, pc + 2 + disp);
	return 2 | SUPPORTED;
}

offs_t fr_disassembler::dasm_07(std::ostream &stream, offs_t pc, const fr_disassembler::data_buffer &opcodes, u16 opcode)
{
	switch (opcode & 0x00f0)
	{
	case 0x00:
		util::stream_format(stream, "%-8s@R15+, R%d", "LD", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x10:
		util::stream_format(stream, "%-8sR%d, PS", "MOV", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x80:
		util::stream_format(stream, "%-8s@R15+, ", "LD");
		format_rs(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x90:
		util::stream_format(stream, "%-8s@R15+, PS", "MOV");
		return 2 | SUPPORTED;

	default:
		return dasm_invalid(stream, opcode);
	}
}

offs_t fr_disassembler::dasm_17(std::ostream &stream, offs_t pc, const fr_disassembler::data_buffer &opcodes, u16 opcode)
{
	switch (opcode & 0x00f0)
	{
	case 0x00:
		util::stream_format(stream, "%-8sR%d, @-R15", "ST", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x10:
		util::stream_format(stream, "%-8sPS, R%d", "MOV", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x80:
		util::stream_format(stream, "%-8s", "ST");
		format_rs(stream, opcode & 0x000f);
		stream << ", @-R15";
		return 2 | SUPPORTED;

	case 0x90:
		util::stream_format(stream, "%-8sPS, @-R15", "ST");
		return 2 | SUPPORTED;

	default:
		return dasm_invalid(stream, opcode);
	}
}

offs_t fr_disassembler::dasm_97(std::ostream &stream, offs_t pc, const fr_disassembler::data_buffer &opcodes, u16 opcode)
{
	switch (opcode & 0x00f0)
	{
	case 0x00:
		util::stream_format(stream, "%-8s@R%d", "JMP", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x10:
		util::stream_format(stream, "%-8s@R%d", "CALL", opcode & 0x000f);
		return 2 | SUPPORTED | STEP_OVER;

	case 0x20:
		if ((opcode & 0x000f) == 0)
		{
			stream << "RET";
			return 2 | SUPPORTED | STEP_OUT;
		}
		else
			return dasm_invalid(stream, opcode);

	case 0x30:
		if ((opcode & 0x000f) == 0)
		{
			stream << "RETI";
			return 2 | SUPPORTED | STEP_OUT;
		}
		else
			return dasm_invalid(stream, opcode);

	case 0x40:
		util::stream_format(stream, "%-8sR%d", "DIV0S", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x50:
		util::stream_format(stream, "%-8sR%d", "DIV0U", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x60:
		util::stream_format(stream, "%-8sR%d", "DIV1", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x70:
		util::stream_format(stream, "%-8sR%d", "DIV2", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x80:
		util::stream_format(stream, "%-8sR%d", "EXTSB", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x90:
		util::stream_format(stream, "%-8sR%d", "EXTUB", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xa0:
		util::stream_format(stream, "%-8sR%d", "EXTSH", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xb0:
		util::stream_format(stream, "%-8sR%d", "EXTUH", opcode & 0x000f);
		return 2 | SUPPORTED;

	default:
		return dasm_invalid(stream, opcode);
	}
}

offs_t fr_disassembler::dasm_9f(std::ostream &stream, offs_t pc, const fr_disassembler::data_buffer &opcodes, u16 opcode)
{
	switch (opcode & 0x00f0)
	{
	case 0x00:
		util::stream_format(stream, "%-8s@R%d", "JMP:D", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x10:
		util::stream_format(stream, "%-8s@R%d", "CALL:D", opcode & 0x000f);
		return 2 | SUPPORTED | STEP_OVER | step_over_extra(1);

	case 0x20:
		if ((opcode & 0x000f) == 0)
		{
			stream << "RET:D";
			return 2 | SUPPORTED | STEP_OUT | step_over_extra(1);
		}
		else
			return dasm_invalid(stream, opcode);


	case 0x30:
		if ((opcode & 0x000f) == 0)
		{
			// Emulator software interrupt (#0x09)
			stream << "INTE";
			return 2 | SUPPORTED | STEP_OVER;
		}
		else
			return dasm_invalid(stream, opcode);

	case 0x60:
		if ((opcode & 0x000f) == 0)
		{
			stream << "DIV3";
			return 2 | SUPPORTED;
		}
		else
			return dasm_invalid(stream, opcode);

	case 0x70:
		if ((opcode & 0x000f) == 0)
		{
			stream << "DIV4S";
			return 2 | SUPPORTED;
		}
		else
			return dasm_invalid(stream, opcode);


	case 0x80: // LDI:32
		util::stream_format(stream, "%-8s", "LDI");
		format_i32(stream, opcodes.r32(pc + 2));
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 6 | SUPPORTED;

	case 0x90:
		if ((opcode & 0x000f) == 0)
		{
			stream << "LEAVE";
			return 2 | SUPPORTED;
		}
		else
			return dasm_invalid(stream, opcode);

	case 0xa0:
		if ((opcode & 0x000f) == 0)
		{
			stream << "NOP";
			return 2 | SUPPORTED;
		}
		else
			return dasm_invalid(stream, opcode);

	case 0xc0:
		return dasm_cop(stream, opcode, opcodes.r16(pc + 2), "COPOP", true, true);

	case 0xd0:
		return dasm_cop(stream, opcode, opcodes.r16(pc + 2), "COPLD", false, true);

	case 0xe0:
		return dasm_cop(stream, opcode, opcodes.r16(pc + 2), "COPST", true, false);

	case 0xf0:
		return dasm_cop(stream, opcode, opcodes.r16(pc + 2), "COPSV", true, false);

	default:
		return dasm_invalid(stream, opcode);
	}
}

offs_t fr_disassembler::disassemble(std::ostream &stream, offs_t pc, const fr_disassembler::data_buffer &opcodes, const fr_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	switch (opcode >> 8)
	{
	case 0x00:
		util::stream_format(stream, "%-8s", "LD");
		format_ac_rdisp(stream, (opcode & 0x00f0) >> 4);
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x01:
		util::stream_format(stream, "%-8s", "LDUH");
		format_ac_rdisp(stream, (opcode & 0x00f0) >> 4);
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x02:
		util::stream_format(stream, "%-8s", "LDUB");
		format_ac_rdisp(stream, (opcode & 0x00f0) >> 4);
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x03:
		util::stream_format(stream, "%-8s", "LD");
		format_sp_udisp(stream, (opcode & 0x00f0) >> 2);
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x04:
		util::stream_format(stream, "%-8s@R%d, R%d", "LD", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x05:
		util::stream_format(stream, "%-8s@R%d, R%d", "LDUH", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x06:
		util::stream_format(stream, "%-8s@R%d, R%d", "LDUB", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x07:
		return dasm_07(stream, pc, opcodes, opcode);

	case 0x08:
		util::stream_format(stream, "%-8s", "DMOV");
		format_dir(stream, (opcode & 0x00ff) << 2);
		stream << ", R13";
		return 2 | SUPPORTED;

	case 0x09:
		util::stream_format(stream, "%-8s", "DMOVH");
		format_dir(stream, (opcode & 0x00ff) << 1);
		stream << ", R13";
		return 2 | SUPPORTED;

	case 0x0a:
		util::stream_format(stream, "%-8s", "DMOVB");
		format_dir(stream, opcode & 0x00ff);
		stream << ", R13";
		return 2 | SUPPORTED;

	case 0x0b:
		util::stream_format(stream, "%-8s", "DMOV");
		format_dir(stream, (opcode & 0x00ff) << 2);
		stream << ", @-R15";
		return 2 | SUPPORTED;

	case 0x0c:
		util::stream_format(stream, "%-8s", "DMOV");
		format_dir(stream, (opcode & 0x00ff) << 2);
		stream << ", @R13+";
		return 2 | SUPPORTED;

	case 0x0d:
		util::stream_format(stream, "%-8s", "DMOVH");
		format_dir(stream, (opcode & 0x00ff) << 1);
		stream << ", @R13+";
		return 2 | SUPPORTED;

	case 0x0e:
		util::stream_format(stream, "%-8s", "DMOVB");
		format_dir(stream, opcode & 0x00ff);
		stream << ", @R13+";
		return 2 | SUPPORTED;

	case 0x0f:
		util::stream_format(stream, "%-8s", "ENTER");
		format_u10(stream, (opcode & 0x00ff) << 2);
		return 2 | SUPPORTED;

	case 0x10:
		util::stream_format(stream, "%-8sR%d, ", "ST", opcode & 0x000f);
		format_ac_rdisp(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x11:
		util::stream_format(stream, "%-8sR%d, ", "STH", opcode & 0x000f);
		format_ac_rdisp(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x12:
		util::stream_format(stream, "%-8sR%d, ", "STB", opcode & 0x000f);
		format_ac_rdisp(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x13:
		util::stream_format(stream, "%-8sR%d, ", "ST", opcode & 0x000f);
		format_sp_udisp(stream, (opcode & 0x00f0) >> 2);
		return 2 | SUPPORTED;

	case 0x14:
		util::stream_format(stream, "%-8sR%d, @R%d", "ST", opcode & 0x000f, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x15:
		util::stream_format(stream, "%-8sR%d, @R%d", "STH", opcode & 0x000f, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x16:
		util::stream_format(stream, "%-8sR%d, @R%d", "STB", opcode & 0x000f, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x17:
		return dasm_17(stream, pc, opcodes, opcode);

	case 0x18:
		util::stream_format(stream, "%-8sR13, ", "DMOV");
		format_dir(stream, (opcode & 0x00ff) << 2);
		return 2 | SUPPORTED;

	case 0x19:
		util::stream_format(stream, "%-8sR13, ", "DMOVH");
		format_dir(stream, (opcode & 0x00ff) << 1);
		return 2 | SUPPORTED;

	case 0x1a:
		util::stream_format(stream, "%-8sR13, ", "DMOVB");
		format_dir(stream, opcode & 0x00ff);
		return 2 | SUPPORTED;

	case 0x1b:
		util::stream_format(stream, "%-8s@R15+, ", "DMOV");
		format_dir(stream, (opcode & 0x00ff) << 2);
		return 2 | SUPPORTED;

	case 0x1c:
		util::stream_format(stream, "%-8s@R13+, ", "DMOV");
		format_dir(stream, (opcode & 0x00ff) << 2);
		return 2 | SUPPORTED;

	case 0x1d:
		util::stream_format(stream, "%-8s@R13+, ", "DMOVH");
		format_dir(stream, (opcode & 0x00ff) << 1);
		return 2 | SUPPORTED;

	case 0x1e:
		util::stream_format(stream, "%-8s@R13+, ", "DMOVB");
		format_dir(stream, opcode & 0x00ff);
		return 2 | SUPPORTED;

	case 0x1f:
		util::stream_format(stream, "%-8s", "INT");
		format_u8(stream, opcode & 0x00ff);
		return 2 | SUPPORTED | STEP_OVER;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		return dasm_ld_fp_disp(stream, opcode, "LD", 2);

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		return dasm_st_fp_disp(stream, opcode, "ST", 2);

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		return dasm_ld_fp_disp(stream, opcode, "LDUH", 1);

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		return dasm_st_fp_disp(stream, opcode, "STH", 1);

	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		return dasm_ld_fp_disp(stream, opcode, "LDUB", 0);

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		return dasm_st_fp_disp(stream, opcode, "STB", 0);

	case 0x80:
		return dasm_bop(stream, opcode, "BANDL");

	case 0x81:
		return dasm_bop(stream, opcode, "BANDH");

	case 0x82:
		return dasm_rrop(stream, opcode, "AND");

	case 0x83:
		util::stream_format(stream, "%-8s", "ANDCCR");
		format_u8(stream, opcode & 0x00ff);
		return 2 | SUPPORTED;

	case 0x84:
		util::stream_format(stream, "%-8sR%d, @R%d", "AND", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x85:
		util::stream_format(stream, "%-8sR%d, @R%d", "ANDH", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x86:
		util::stream_format(stream, "%-8sR%d, @R%d", "ANDB", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x87:
		util::stream_format(stream, "%-8s", "STILM");
		format_u8(stream, opcode & 0x00ff);
		return 2 | SUPPORTED;

	case 0x88:
		return dasm_bop(stream, opcode, "BTSTL");

	case 0x89:
		return dasm_bop(stream, opcode, "BTSTH");

	case 0x8a:
		util::stream_format(stream, "%-8s@R%d, R%d", "XCHB", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x8b:
		return dasm_rrop(stream, opcode, "MOV");

	case 0x8c:
		return dasm_ldstm(stream, opcode, "LDM0");

	case 0x8d:
		return dasm_ldstm(stream, opcode, "LDM1");

	case 0x8e:
		return dasm_ldstm(stream, opcode, "STM0");

	case 0x8f:
		return dasm_ldstm(stream, opcode, "STM1");

	case 0x90:
		return dasm_bop(stream, opcode, "BORL");

	case 0x91:
		return dasm_bop(stream, opcode, "BORH");

	case 0x92:
		return dasm_rrop(stream, opcode, "OR");

	case 0x93:
		util::stream_format(stream, "%-8s", "ORCCR");
		format_u8(stream, opcode & 0x00ff);
		return 2 | SUPPORTED;

	case 0x94:
		util::stream_format(stream, "%-8sR%d, @R%d", "OR", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x95:
		util::stream_format(stream, "%-8sR%d, @R%d", "ORH", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x96:
		util::stream_format(stream, "%-8sR%d, @R%d", "ORB", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x97:
		return dasm_97(stream, pc, opcodes, opcode);

	case 0x98:
		return dasm_bop(stream, opcode, "BEORL");

	case 0x99:
		return dasm_bop(stream, opcode, "BEORH");

	case 0x9a:
		return dasm_rrop(stream, opcode, "EOR");

	case 0x9b: // LDI:20
		util::stream_format(stream, "%-8s", "LDI");
		format_i20(stream, u32(opcode & 0x00f0) << 12 | opcodes.r16(pc + 2));
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 4 | SUPPORTED;

	case 0x9c:
		util::stream_format(stream, "%-8sR%d, @R%d", "EOR", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x9d:
		util::stream_format(stream, "%-8sR%d, @R%d", "EORH", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x9e:
		util::stream_format(stream, "%-8sR%d, @R%d", "EORB", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x9f:
		return dasm_9f(stream, pc, opcodes, opcode);

	case 0xa0:
	case 0xa1: // ADDN2
		return dasm_i4op(stream, opcode, "ADDN");

	case 0xa2:
		return dasm_rrop(stream, opcode, "ADDN");

	case 0xa3:
		util::stream_format(stream, "%-8s#", "ADDSP");
		format_disp(stream, opcode & 0x00ff, 2);
		return 2 | SUPPORTED;

	case 0xa4:
	case 0xa5: // ADD2
		return dasm_i4op(stream, opcode, "ADD");

	case 0xa6:
		return dasm_rrop(stream, opcode, "ADD");

	case 0xa7:
		return dasm_rrop(stream, opcode, "ADDC");

	case 0xa8:
	case 0xa9: // CMP2
		return dasm_i4op(stream, opcode, "CMP");

	case 0xaa:
		return dasm_rrop(stream, opcode, "CMP");

	case 0xab:
		return dasm_rrop(stream, opcode, "MULH");

	case 0xac:
		return dasm_rrop(stream, opcode, "SUB");

	case 0xad:
		return dasm_rrop(stream, opcode, "SUBC");

	case 0xae:
		return dasm_rrop(stream, opcode, "SUBN");

	case 0xaf:
		return dasm_rrop(stream, opcode, "MUL");

	case 0xb0:
	case 0xb1: // LSR2
		return dasm_shift(stream, opcode, "LSR");

	case 0xb2:
		return dasm_rrop(stream, opcode, "LSR");

	case 0xb3:
		util::stream_format(stream, "%-8sR%d, ", "MOV", opcode & 0x000f);
		format_rs(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0xb4:
	case 0xb5: // LSL2
		return dasm_shift(stream, opcode, "LSL");

	case 0xb6:
		return dasm_rrop(stream, opcode, "LSL");

	case 0xb7:
		util::stream_format(stream, "%-8s", "MOV");
		format_rs(stream, (opcode & 0x00f0) >> 4);
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xb8:
	case 0xb9: // ASR2
		return dasm_shift(stream, opcode, "ASR");

	case 0xba:
		return dasm_rrop(stream, opcode, "ASR");

	case 0xbb:
		return dasm_rrop(stream, opcode, "MULUH");

	case 0xbc:
		util::stream_format(stream, "%-8s@R%d+, #%d", "LDRES", opcode & 0x000f, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0xbd:
		util::stream_format(stream, "%-8s#%d, @R%d+", "STRES", (opcode & 0x00f0) >> 4, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xbf:
		return dasm_rrop(stream, opcode, "MULU");

	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: // LDI:8
		util::stream_format(stream, "%-8s", "LDI");
		format_i8(stream, (opcode & 0x0ff0) >> 4);
		util::stream_format(stream, ", R%d", opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		return dasm_call(stream, pc, "CALL", (opcode & 0x07ff) << 1);

	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		return dasm_call(stream, pc, "CALL:D", (opcode & 0x07ff) << 1) | step_over_extra(1);

	case 0xe0:
		return dasm_branch(stream, pc, "BRA", (opcode & 0x00ff) << 1);

	case 0xe1:
		return dasm_branch(stream, pc, "BNO", (opcode & 0x00ff) << 1);

	case 0xe2:
		return dasm_branch(stream, pc, "BEQ", (opcode & 0x00ff) << 1);

	case 0xe3:
		return dasm_branch(stream, pc, "BNE", (opcode & 0x00ff) << 1);

	case 0xe4:
		return dasm_branch(stream, pc, "BC", (opcode & 0x00ff) << 1);

	case 0xe5:
		return dasm_branch(stream, pc, "BNC", (opcode & 0x00ff) << 1);

	case 0xe6:
		return dasm_branch(stream, pc, "BN", (opcode & 0x00ff) << 1);

	case 0xe7:
		return dasm_branch(stream, pc, "BP", (opcode & 0x00ff) << 1);

	case 0xe8:
		return dasm_branch(stream, pc, "BV", (opcode & 0x00ff) << 1);

	case 0xe9:
		return dasm_branch(stream, pc, "BNV", (opcode & 0x00ff) << 1);

	case 0xea:
		return dasm_branch(stream, pc, "BLT", (opcode & 0x00ff) << 1);

	case 0xeb:
		return dasm_branch(stream, pc, "BGE", (opcode & 0x00ff) << 1);

	case 0xec:
		return dasm_branch(stream, pc, "BLE", (opcode & 0x00ff) << 1);

	case 0xed:
		return dasm_branch(stream, pc, "BGT", (opcode & 0x00ff) << 1);

	case 0xee:
		return dasm_branch(stream, pc, "BLS", (opcode & 0x00ff) << 1);

	case 0xef:
		return dasm_branch(stream, pc, "BHI", (opcode & 0x00ff) << 1);

	case 0xf0:
		return dasm_branch(stream, pc, "BRA:D", (opcode & 0x00ff) << 1);

	case 0xf1:
		return dasm_branch(stream, pc, "BNO:D", (opcode & 0x00ff) << 1);

	case 0xf2:
		return dasm_branch(stream, pc, "BEQ:D", (opcode & 0x00ff) << 1);

	case 0xf3:
		return dasm_branch(stream, pc, "BNE:D", (opcode & 0x00ff) << 1);

	case 0xf4:
		return dasm_branch(stream, pc, "BC:D", (opcode & 0x00ff) << 1);

	case 0xf5:
		return dasm_branch(stream, pc, "BNC:D", (opcode & 0x00ff) << 1);

	case 0xf6:
		return dasm_branch(stream, pc, "BN:D", (opcode & 0x00ff) << 1);

	case 0xf7:
		return dasm_branch(stream, pc, "BP:D", (opcode & 0x00ff) << 1);

	case 0xf8:
		return dasm_branch(stream, pc, "BV:D", (opcode & 0x00ff) << 1);

	case 0xf9:
		return dasm_branch(stream, pc, "BNV:D", (opcode & 0x00ff) << 1);

	case 0xfa:
		return dasm_branch(stream, pc, "BLT:D", (opcode & 0x00ff) << 1);

	case 0xfb:
		return dasm_branch(stream, pc, "BGE:D", (opcode & 0x00ff) << 1);

	case 0xfc:
		return dasm_branch(stream, pc, "BLE:D", (opcode & 0x00ff) << 1);

	case 0xfd:
		return dasm_branch(stream, pc, "BGT:D", (opcode & 0x00ff) << 1);

	case 0xfe:
		return dasm_branch(stream, pc, "BLS:D", (opcode & 0x00ff) << 1);

	case 0xff:
		return dasm_branch(stream, pc, "BHI:D", (opcode & 0x00ff) << 1);

	default:
		return dasm_invalid(stream, opcode);
	}
}
