// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, David Haywood
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#include "emu.h"
#include "unspdasm.h"

offs_t unsp_disassembler::disassemble_remaining(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes)
{
	uint32_t len = 1;
	uint8_t op0 = (op >> 12);
	uint8_t opA = (op >> 9) & 7;
	uint8_t op1 = (op >> 6) & 7;
	uint8_t opN = (op >> 3) & 7;
	uint8_t opB = op & 7;
	uint8_t opimm = op & 63;

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
		else if (opA + 1 < 8 && opA + opN < 8)
			util::stream_format(stream, "pop %s, %s from [%s]",
				regs[opA + 1], regs[opA + opN], regs[opB]);
		else
			util::stream_format(stream, "<BAD>");
		return UNSP_DASM_OK;


		// push insns
	case 0x2d:
		if (opA + 1 >= opN && opA < opN + 7)
			util::stream_format(stream, "push %s, %s to [%s]",
				regs[opA + 1 - opN], regs[opA], regs[opB]);
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
			len = 2;
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
			len = 2;
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
			len = 2;
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
			len = 2;
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

	default:
		util::stream_format(stream, "<UNHANDLED>");
		return UNSP_DASM_OK;
	}
}
