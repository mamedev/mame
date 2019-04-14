// license:GPL-2.0+
// copyright-holders:Segher Boessenkool
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#include "emu.h"
#include "unspdasm.h"

char const *const unsp_disassembler::regs[] =
{
	"sp", "r1", "r2", "r3", "r4", "bp", "sr", "pc"
};

char const *const unsp_disassembler::jumps[] =
{
	"jb", "jae", "jge", "jl", "jne", "je", "jpl", "jmi",
	"jbe", "ja", "jle", "jg", "jvc", "jvs", "jmp", "<inv>"
};

#define UNSP_DASM_OK (len | SUPPORTED)

u32 unsp_disassembler::opcode_alignment() const
{
	return 1;
}

void unsp_disassembler::print_alu_op_start(std::ostream &stream, uint8_t op0, uint8_t opA)
{
	static const char* const alu_op_start[] =
	{
		"%s += ", "%s += ", "%s -= ", "%s -= ",
		"cmp %s, ", "<BAD>", "%s =- ", "<BAD>",
		"%s ^= ", "%s = ", "%s |= ", "%s &= ",
		"test %s, "
	};

	util::stream_format(stream, alu_op_start[op0], regs[opA]);
}

void unsp_disassembler::print_alu_op3(std::ostream &stream, uint8_t op0, uint8_t opB)
{
	static const char* const alu_op3[] =
	{
		"%s + ", "%s + ", "%s - ", "%s - ",
		"cmp %s, ", "<BAD>", "-", "<BAD>",
		"%s ^ ", "", "%s | ", "%s & ",
		"test %s, "
	};

	util::stream_format(stream, alu_op3[op0], regs[opB]);
}

void unsp_disassembler::print_alu_op_end(std::ostream &stream, uint8_t op0)
{
	if (op0 == 1 || op0 == 3)
		util::stream_format(stream, ", carry");
}

void unsp_disassembler::print_indirect_op(std::ostream &stream, uint8_t opN, uint8_t opB)
{
	static const char* const forms[] = { "[%s]", "[%s--]", "[%s++]", "[++%s]" };

	if (opN & 4)
		util::stream_format(stream, "ds:");
	util::stream_format(stream, forms[opN & 3], regs[opB]);
}

offs_t unsp_disassembler::disassemble_f_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, uint8_t opN, uint8_t opA, uint8_t opB, uint32_t len)
{
	switch (opN)
	{
	case 1:
		if (opA == 7)
		{
			util::stream_format(stream, "<DUNNO f group>");
			return UNSP_DASM_OK;
		}
		util::stream_format(stream, "mr = %s*%s, us", regs[opA], regs[opB]);
		return UNSP_DASM_OK;
	default:
		util::stream_format(stream, "<DUNNO f group>");
		return UNSP_DASM_OK;
	}
}

offs_t unsp_newer_disassembler::disassemble_f_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, uint8_t opN, uint8_t opA, uint8_t opB, uint32_t len)
{
	// these are new opcodes on the later core

	switch (opN)
	{
	case 1:
		if (opA == 7)
		{
			util::stream_format(stream, "<EXTENDED f group>");
			return UNSP_DASM_OK;
		}
		util::stream_format(stream, "mr = %s*%s, us", regs[opA], regs[opB]);
		return UNSP_DASM_OK;
	default:
		util::stream_format(stream, "<EXTENDED f group>");
		return UNSP_DASM_OK;
	}
}



offs_t unsp_disassembler::disassemble(std::ostream &stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	// the top four bits are the alu op or the branch condition, or E or F
	uint8_t op0 = (op >> 12);

	// the next three are usually the destination register
	uint8_t opA = (op >> 9) & 7;

	// and the next three the addressing mode
	uint8_t op1 = (op >> 6) & 7;

	// the next three can be anything
	uint8_t opN = (op >> 3) & 7;

	// and the last three usually the second register (source register)
	uint8_t opB = op & 7;

	// the last six sometimes are a single immediate number
	uint8_t opimm = op & 63;

	uint32_t len = 1;
	if ((op0 < 14 && op1 == 4 && (opN == 1 || opN == 2 || opN == 3)) || (op0 == 15 && (op1 == 1 || op1 == 2)))
	{
		len = 2;
	}

	// all-zero and all-one are invalid insns:
	if (op == 0 || op == 0xffff)
	{
		util::stream_format(stream, "--");
		return UNSP_DASM_OK;
	}

	// first, check for the conditional branch insns
	if (op0 < 15 && opA == 7 && op1 == 0)
	{
		util::stream_format(stream, "%s %04x", jumps[op0], pc+1+opimm);
		return UNSP_DASM_OK;
	}
	if (op0 < 15 && opA == 7 && op1 == 1)
	{
		util::stream_format(stream, "%s %04x", jumps[op0], pc+1-opimm);
		return UNSP_DASM_OK;
	}

	switch ((op1 << 4) | op0)
	{
	case 0x05: case 0x15: case 0x25: case 0x35:
	case 0x45: case 0x55: case 0x65: case 0x75:
	case 0x85: case 0x95: case 0xa5: case 0xb5:
	case 0xc5: case 0xd5:
	case 0x07: case 0x17: case 0x27: case 0x37:
	case 0x47: case 0x57: case 0x67: case 0x77:
	case 0x87: case 0x97: case 0xa7: case 0xb7:
	case 0xc7: case 0xd7:
	case 0x1d: case 0x5d: case 0x6d:
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x24: case 0x26: case 0x28: case 0x2a:
	case 0x2b: case 0x2c:
		util::stream_format(stream, "<BAD>");
		return UNSP_DASM_OK;


	// alu, base+displacement
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x06: case 0x08: case 0x09:
	case 0x0a: case 0x0b: case 0x0c:
		print_alu_op_start(stream, op0, opA);
		util::stream_format(stream, "[bp+%02x]", opimm);
		print_alu_op_end(stream, op0);
		return UNSP_DASM_OK;
	case 0x0d:
		util::stream_format(stream, "[bp+%02x] = %s", opimm, regs[opA]);
		return UNSP_DASM_OK;


	// alu, 6-bit immediate
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x16: case 0x18: case 0x19:
	case 0x1a: case 0x1b: case 0x1c:
		print_alu_op_start(stream, op0, opA);
		util::stream_format(stream, "%02x", opimm);
		print_alu_op_end(stream, op0);
		return UNSP_DASM_OK;


	// pop insns
	case 0x29:
		if (op == 0x9a90)
			util::stream_format(stream, "retf");
		else if (op == 0x9a98)
			util::stream_format(stream, "reti");
		else if (opA+1 < 8 && opA+opN < 8)
			util::stream_format(stream, "pop %s, %s from [%s]",
				   regs[opA+1], regs[opA+opN], regs[opB]);
		else
			util::stream_format(stream, "<BAD>");
		return UNSP_DASM_OK;


	// push insns
	case 0x2d:
		if (opA+1 >= opN && opA < opN+7)
			util::stream_format(stream, "push %s, %s to [%s]",
				   regs[opA+1-opN], regs[opA], regs[opB]);
		else
			util::stream_format(stream, "<BAD>");
		return UNSP_DASM_OK;


	// alu, indirect memory
	case 0x30: case 0x31: case 0x32: case 0x33:
	case 0x34: case 0x36: case 0x38: case 0x39:
	case 0x3a: case 0x3b: case 0x3c:
		print_alu_op_start(stream, op0, opA);
		print_indirect_op(stream, opN, opB);
		print_alu_op_end(stream, op0);
		return UNSP_DASM_OK;
	case 0x3d:
		print_indirect_op(stream, opN, opB);
		util::stream_format(stream, " = %s", regs[opA]);
		return UNSP_DASM_OK;


	case 0x40: case 0x41: case 0x42: case 0x43:
	case 0x44: case 0x46: case 0x48: case 0x49:
	case 0x4a: case 0x4b: case 0x4c:
		switch (opN)
		{
		// alu, register
		case 0:
			print_alu_op_start(stream, op0, opA);
			util::stream_format(stream, "%s", regs[opB]);
			print_alu_op_end(stream, op0);
			return UNSP_DASM_OK;

		// alu, 16-bit immediate
		case 1:
			if ((op0 == 4 || op0 == 12 || op0 == 6 || op0 == 9) && opA != opB)
			{
				util::stream_format(stream, "<BAD>");
				return UNSP_DASM_OK;
			}
			if (op0 != 4 && op0 != 12)
				util::stream_format(stream, "%s = ", regs[opA]);
			print_alu_op3(stream, op0, opB);
			util::stream_format(stream, "%04x", ximm);
			print_alu_op_end(stream, op0);
			return UNSP_DASM_OK;

		// alu, direct memory
		case 2:
			if ((op0 == 4 || op0 == 12 || op0 == 6 || op0 == 9) && opA != opB)
			{
				util::stream_format(stream, "<BAD>");
				return UNSP_DASM_OK;
			}
			if (op0 != 4 && op0 != 12)
				util::stream_format(stream, "%s = ", regs[opA]);
			print_alu_op3(stream, op0, opB);
			util::stream_format(stream, "[%04x]", ximm);
			print_alu_op_end(stream, op0);
			return UNSP_DASM_OK;

		// alu, direct memory
		case 3:
			if (op0 == 4 || op0 == 12)
			{
				util::stream_format(stream, "<BAD>");
				return UNSP_DASM_OK;
			}
			if ((op0 == 6 || op0 == 9) && opA != opB)
			{
				util::stream_format(stream, "<BAD>");
				return UNSP_DASM_OK;
			}
			util::stream_format(stream, "[%04x] = ", ximm);
			print_alu_op3(stream, op0, opB);
			util::stream_format(stream, "%s", regs[opA]);
			print_alu_op_end(stream, op0);
			return UNSP_DASM_OK;

		// alu, with shift
		default:
			print_alu_op_start(stream, op0, opA);
			util::stream_format(stream, "%s asr %x", regs[opB], (opN & 3) + 1);
			print_alu_op_end(stream, op0);
			return UNSP_DASM_OK;
		}

	case 0x4d:
		switch (opN)
		{
		// alu, direct memory
		case 3:
			if (opA != opB)
			{
				util::stream_format(stream, "<BAD>");
				return UNSP_DASM_OK;
			}
			util::stream_format(stream, "[%04x] = %s", ximm, regs[opB]);
			return UNSP_DASM_OK;
		default:
			util::stream_format(stream, "<BAD>");
			return UNSP_DASM_OK;
		}


	// alu, with shift
	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x56: case 0x58: case 0x59:
	case 0x5a: case 0x5b: case 0x5c:
		print_alu_op_start(stream, op0, opA);
		if ((opN & 4) == 0)
			util::stream_format(stream, "%s lsl %x", regs[opB], (opN & 3) + 1);
		else
			util::stream_format(stream, "%s lsr %x", regs[opB], (opN & 3) + 1);
		print_alu_op_end(stream, op0);
		return UNSP_DASM_OK;


	// alu, with shift
	case 0x60: case 0x61: case 0x62: case 0x63:
	case 0x64: case 0x66: case 0x68: case 0x69:
	case 0x6a: case 0x6b: case 0x6c:
		print_alu_op_start(stream, op0, opA);
		if ((opN & 4) == 0)
			util::stream_format(stream, "%s rol %x", regs[opB], (opN & 3) + 1);
		else
			util::stream_format(stream, "%s ror %x", regs[opB], (opN & 3) + 1);
		print_alu_op_end(stream, op0);
		return UNSP_DASM_OK;


	// alu, direct memory
	case 0x70: case 0x71: case 0x72: case 0x73:
	case 0x74: case 0x76: case 0x78: case 0x79:
	case 0x7a: case 0x7b: case 0x7c:
		print_alu_op_start(stream, op0, opA);
		util::stream_format(stream, "[%02x]", opimm);
		print_alu_op_end(stream, op0);
		return UNSP_DASM_OK;
	case 0x7d:
		util::stream_format(stream, "[%02x] = %s", opimm, regs[opA]);
		return UNSP_DASM_OK;


	case 0x1f:
		if (opA == 0)
		{
			util::stream_format(stream, "call %04x", (opimm << 16) | ximm);
			return UNSP_DASM_OK;
		}
		util::stream_format(stream, "<DUNNO>");
		return UNSP_DASM_OK;

	case 0x2f: case 0x3f: case 0x6f: case 0x7f:
		if (opA == 7 && op1 == 2)
		{
			util::stream_format(stream, "goto %04x", (opimm << 16) | ximm);
			return UNSP_DASM_OK;
		}
		if (opA == 7 && op1 == 3)
		{
			util::stream_format(stream, "<DUNNO>");
			return UNSP_DASM_OK;
		}
		util::stream_format(stream, "<DUNNO>");
		return UNSP_DASM_OK;


	case 0x0f:
		return disassemble_f_group(stream, pc, op, ximm, opN, opA, opB, len);

	case 0x4f:
		switch (opN)
		{
		case 1:
			if (opA == 7)
			{
				util::stream_format(stream, "<DUNNO>");
				return UNSP_DASM_OK;
			}
			util::stream_format(stream, "mr = %s*%s", regs[opA], regs[opB]);
			return UNSP_DASM_OK;
		default:
			util::stream_format(stream, "<DUNNO>");
			return UNSP_DASM_OK;
		}

	case 0x5f:
		if (opA != 0)
		{
			util::stream_format(stream, "<DUNNO>");
			return UNSP_DASM_OK;
		}
		switch (opimm)
		{
		case 0x00:
			util::stream_format(stream, "int off");
			return UNSP_DASM_OK;
		case 0x01:
			util::stream_format(stream, "int irq");
			return UNSP_DASM_OK;
		case 0x02:
			util::stream_format(stream, "int fiq");
			return UNSP_DASM_OK;
		case 0x03:
			util::stream_format(stream, "int fiq,irq");
			return UNSP_DASM_OK;
		case 0x04:
			util::stream_format(stream, "fir_mov on");
			return UNSP_DASM_OK;
		case 0x05:
			util::stream_format(stream, "fir_mov off");
			return UNSP_DASM_OK;
		case 0x08:
			util::stream_format(stream, "irq off");
			return UNSP_DASM_OK;
		case 0x09:
			util::stream_format(stream, "irq on");
			return UNSP_DASM_OK;
		case 0x0c:
			util::stream_format(stream, "fiq off");
			return UNSP_DASM_OK;
		case 0x0e:
			util::stream_format(stream, "fiq on");
			return UNSP_DASM_OK;
		case 0x25:
			util::stream_format(stream, "nop");
			return UNSP_DASM_OK;
		default:
			util::stream_format(stream, "<DUNNO>");
			return UNSP_DASM_OK;
		}

	case 0x0e: case 0x1e: case 0x2e: case 0x3e:
	case 0x4e: case 0x5e: case 0x6e: case 0x7e:
		util::stream_format(stream, "<DUNNO>");
		return UNSP_DASM_OK;

	default:
		util::stream_format(stream, "<UNHANDLED>");
		return UNSP_DASM_OK;
	}
	return UNSP_DASM_OK;
}

offs_t unsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint16_t op = opcodes.r16(pc);
	uint16_t imm16 = opcodes.r16(pc+1);

	return disassemble(stream, pc, op, imm16);
}
