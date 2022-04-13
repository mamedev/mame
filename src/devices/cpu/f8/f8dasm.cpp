// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#include "emu.h"
#include "f8dasm.h"

const char *const f8_disassembler::rname[16] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"R8", "J",  "HU", "HL", "KU", "KL", "QU", "QL"
};

u32 f8_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t f8_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t base_pc = pc;
	uint8_t op = opcodes.r8(pc++);
	offs_t flags = SUPPORTED;

	switch( op )
	{
	/* opcode  bitmask */
	case 0x00: /* 0000 0000 */
	case 0x01: /* 0000 0001 */
	case 0x02: /* 0000 0010 */
	case 0x03: /* 0000 0011 */
		util::stream_format(stream, "LR   A,%s", rname[op + 12]);
		break;
	case 0x04: /* 0000 0100 */
	case 0x05: /* 0000 0101 */
	case 0x06: /* 0000 0110 */
	case 0x07: /* 0000 0111 */
		util::stream_format(stream, "LR   %s,A", rname[(op - 0x04) + 12]);
		break;

	case 0x08: /* 0000 1000 */
		util::stream_format(stream, "LR   K,P");
		break;
	case 0x09: /* 0000 1001 */
		util::stream_format(stream, "LR   P,K");
		break;
	case 0x0a: /* 0000 1010 */
		util::stream_format(stream, "LR   A,IS");
		break;
	case 0x0b: /* 0000 1011 */
		util::stream_format(stream, "LR   IS,A");
		break;

	case 0x0c: /* 0000 1100 */
		util::stream_format(stream, "PK");
		flags |= STEP_OUT;
		break;
	case 0x0d: /* 0000 1101 */
		util::stream_format(stream, "LR   P0,Q");
		break;
	case 0x0e: /* 0000 1110 */
		util::stream_format(stream, "LR   Q,DC");
		break;
	case 0x0f: /* 0000 1111 */
		util::stream_format(stream, "LR   DC,Q");
		break;
	case 0x10: /* 0001 0000 */
		util::stream_format(stream, "LR   DC,H");
		break;
	case 0x11: /* 0001 0001 */
		util::stream_format(stream, "LR   H,DC");
		break;
	case 0x12: /* 0001 0010 */
		util::stream_format(stream, "SR   1");
		break;
	case 0x13: /* 0001 0011 */
		util::stream_format(stream, "SL   1");
		break;
	case 0x14: /* 0001 0100 */
		util::stream_format(stream, "SR   4");
		break;
	case 0x15: /* 0001 0101 */
		util::stream_format(stream, "SL   4");
		break;
	case 0x16: /* 0001 0110 */
		util::stream_format(stream, "LM") ;
		break;
	case 0x17: /* 0001 0111 */
		util::stream_format(stream, "ST");
		break;
	case 0x18: /* 0001 1000 */
		util::stream_format(stream, "COM");
		break;
	case 0x19: /* 0001 1001 */
		util::stream_format(stream, "LNK");
		break;
	case 0x1a: /* 0001 1010 */
		util::stream_format(stream, "DI");
		break;
	case 0x1b: /* 0001 1011 */
		util::stream_format(stream, "EI");
		break;
	case 0x1c: /* 0001 1100 */
		util::stream_format(stream, "POP");
		flags |= STEP_OUT;
		break;
	case 0x1d: /* 0001 1101 */
		util::stream_format(stream, "LR   W,J");
		break;
	case 0x1e: /* 0001 1110 */
		util::stream_format(stream, "LR   J,W");
		break;
	case 0x1f: /* 0001 1111 */
		util::stream_format(stream, "INC");
		break;
	case 0x20: /* 0010 0000 */
		util::stream_format(stream, "LI   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x21: /* 0010 0001 */
		util::stream_format(stream, "NI   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x22: /* 0010 0010 */
		util::stream_format(stream, "OI   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x23: /* 0010 0011 */
		util::stream_format(stream, "XI   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x24: /* 0010 0100 */
		util::stream_format(stream, "AI   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x25: /* 0010 0101 */
		util::stream_format(stream, "CI   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x26: /* 0010 0110 */
		util::stream_format(stream, "IN   H'%02X'", opcodes.r8(pc++));
		break;
	case 0x27: /* 0010 0111 */
		util::stream_format(stream, "OUT  H'%02X'", opcodes.r8(pc++));
		break;
	case 0x28: /* 0010 1000 */
		util::stream_format(stream, "PI   H'%04X'", opcodes.r16(pc));
		pc += 2;
		flags |= STEP_OVER;
		break;
	case 0x29: /* 0010 1001 */
		util::stream_format(stream, "JMP  H'%04X'", opcodes.r16(pc));
		pc += 2;
		break;
	case 0x2a: /* 0010 1010 */
		util::stream_format(stream, "DCI  H'%04X'", opcodes.r16(pc));
		pc += 2;
		break;
	case 0x2b: /* 0010 1011 */
		util::stream_format(stream, "NOP");
		break;
	case 0x2c: /* 0010 1100 */
		util::stream_format(stream, "XDC");
		break;
	/* case 0x2d: 0010 1101 */
	/* case 0x2e: 0010 1110 */
	/* case 0x2f: 0010 1111 */

	case 0x30: /* 0011 0000 */
	case 0x31: /* 0011 0001 */
	case 0x32: /* 0011 0010 */
	case 0x33: /* 0011 0011 */
	case 0x34: /* 0011 0100 */
	case 0x35: /* 0011 0101 */
	case 0x36: /* 0011 0110 */
	case 0x37: /* 0011 0111 */
	case 0x38: /* 0011 1000 */
	case 0x39: /* 0011 1001 */
	case 0x3a: /* 0011 1010 */
	case 0x3b: /* 0011 1011 */
		util::stream_format(stream, "DS   %s", rname[op & 15]);
		break;
	case 0x3c: /* 0011 1100 */
		util::stream_format(stream, "DS   S");
		break;
	case 0x3d: /* 0011 1101 */
		util::stream_format(stream, "DS   I");
		break;
	case 0x3e: /* 0011 1110 */
		util::stream_format(stream, "DS   D");
		break;
	/* case 0x3f: 0011 1111 */

	case 0x40: /* 0100 0000 */
	case 0x41: /* 0100 0001 */
	case 0x42: /* 0100 0010 */
	case 0x43: /* 0100 0011 */
	case 0x44: /* 0100 0100 */
	case 0x45: /* 0100 0101 */
	case 0x46: /* 0100 0110 */
	case 0x47: /* 0100 0111 */
	case 0x48: /* 0100 1000 */
	case 0x49: /* 0100 1001 */
	case 0x4a: /* 0100 1010 */
	case 0x4b: /* 0100 1011 */
		util::stream_format(stream, "LR   A,%s", rname[op & 15]);
		break;
	case 0x4c: /* 0100 1100 */
		util::stream_format(stream, "LR   A,S");
		break;
	case 0x4d: /* 0100 1101 */
		util::stream_format(stream, "LR   A,I");
		break;
	case 0x4e: /* 0100 1110 */
		util::stream_format(stream, "LR   A,D");
		break;
	/* case 0x4f: 0100 1111 */

	case 0x50: /* 0101 0000 */
	case 0x51: /* 0101 0001 */
	case 0x52: /* 0101 0010 */
	case 0x53: /* 0101 0011 */
	case 0x54: /* 0101 0100 */
	case 0x55: /* 0101 0101 */
	case 0x56: /* 0101 0110 */
	case 0x57: /* 0101 0111 */
	case 0x58: /* 0101 1000 */
	case 0x59: /* 0101 1001 */
	case 0x5a: /* 0101 1010 */
	case 0x5b: /* 0101 1011 */
		util::stream_format(stream, "LR   %s,A", rname[op & 15]);
		break;
	case 0x5c: /* 0101 1100 */
		util::stream_format(stream, "LR   S,A");
		break;
	case 0x5d: /* 0101 1101 */
		util::stream_format(stream, "LR   I,A");
		break;
	case 0x5e: /* 0101 1110 */
		util::stream_format(stream, "LR   D,A");
		break;
	/* case 0x5f: 0101 1111 */

	case 0x60: /* 0110 0000 */
	case 0x61: /* 0110 0001 */
	case 0x62: /* 0110 0010 */
	case 0x63: /* 0110 0011 */
	case 0x64: /* 0110 0100 */
	case 0x65: /* 0110 0101 */
	case 0x66: /* 0110 0110 */
	case 0x67: /* 0110 0111 */
		util::stream_format(stream, "LISU %d", op & 0x07);
		break;
	case 0x68: /* 0110 1000 */
	case 0x69: /* 0110 1001 */
	case 0x6a: /* 0110 1010 */
	case 0x6b: /* 0110 1011 */
	case 0x6c: /* 0110 1100 */
	case 0x6d: /* 0110 1101 */
	case 0x6e: /* 0110 1110 */
	case 0x6f: /* 0110 1111 */
		util::stream_format(stream, "LISL %d", op & 0x07);
		break;

	case 0x70: /* 0111 0000 */
		util::stream_format(stream, "CLR");
		break;
	case 0x71: /* 0111 0001 */
	case 0x72: /* 0111 0010 */
	case 0x73: /* 0111 0011 */
	case 0x74: /* 0111 0100 */
	case 0x75: /* 0111 0101 */
	case 0x76: /* 0111 0110 */
	case 0x77: /* 0111 0111 */
	case 0x78: /* 0111 1000 */
	case 0x79: /* 0111 1001 */
	case 0x7a: /* 0111 1010 */
	case 0x7b: /* 0111 1011 */
	case 0x7c: /* 0111 1100 */
	case 0x7d: /* 0111 1101 */
	case 0x7e: /* 0111 1110 */
	case 0x7f: /* 0111 1111 */
		util::stream_format(stream, "LIS  H'%02X'", op & 0x0f);
		break;

	case 0x81: /* 1000 0001 */
	case 0x85: /* 1000 0101 */
		util::stream_format(stream, "BP   H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x82: /* 1000 0010 */
		util::stream_format(stream, "BC   H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x84: /* 1000 0100 */
		util::stream_format(stream, "BZ   H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x80: /* 1000 0000 */
	case 0x83: /* 1000 0011 */
	case 0x86: /* 1000 0110 */
	case 0x87: /* 1000 0111 */
		util::stream_format(stream, "BT   H'%02X',H'%04X'", op & 0x07, base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x88: /* 1000 1000 */
		util::stream_format(stream, "AM");
		break;

	case 0x89: /* 1000 1001 */
		util::stream_format(stream, "AMD");
		break;

	case 0x8a: /* 1000 1010 */
		util::stream_format(stream, "NM");
		break;

	case 0x8b: /* 1000 1011 */
		util::stream_format(stream, "OM");
		break;

	case 0x8c: /* 1000 1100 */
		util::stream_format(stream, "XM");
		break;

	case 0x8d: /* 1000 1101 */
		util::stream_format(stream, "CM");
		break;

	case 0x8e: /* 1000 1110 */
		util::stream_format(stream, "ADC");
		break;

	case 0x8f: /* 1000 1111 */
		util::stream_format(stream, "BR7  H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x90: /* 1001 0000 */
		util::stream_format(stream, "BR   H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		break;

	case 0x91: /* 1001 0001 */
	case 0x95: /* 1001 0101 */
		util::stream_format(stream, "BM   H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x92: /* 1001 0010 */
		util::stream_format(stream, "BNC  H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x94: /* 1001 0100 */
		util::stream_format(stream, "BNZ  H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x98: /* 1001 1000 */
		util::stream_format(stream, "BNO  H'%04X'", base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0x93: /* 1001 0011 */
	case 0x96: /* 1001 0110 */
	case 0x97: /* 1001 0111 */
	case 0x99: /* 1001 1001 */
	case 0x9a: /* 1001 1010 */
	case 0x9b: /* 1001 1011 */
	case 0x9c: /* 1001 1100 */
	case 0x9d: /* 1001 1101 */
	case 0x9e: /* 1001 1110 */
	case 0x9f: /* 1001 1111 */
		util::stream_format(stream, "BF   H'%02X',H'%04X'", op & 0x0f, base_pc + (int8_t)opcodes.r8(pc++) + 1);
		flags |= STEP_COND;
		break;

	case 0xa0: /* 1010 0000 */
	case 0xa1: /* 1010 0001 */
		util::stream_format(stream, "INS  %d", op & 0x0f);
		break;

	/* case 0xa2: 1010 0010 */
	/* case 0xa3: 1010 0011 */

	case 0xa4: /* 1010 0100 */
	case 0xa5: /* 1010 0101 */
	case 0xa6: /* 1010 0110 */
	case 0xa7: /* 1010 0111 */
	case 0xa8: /* 1010 1000 */
	case 0xa9: /* 1010 1001 */
	case 0xaa: /* 1010 1010 */
	case 0xab: /* 1010 1011 */
	case 0xac: /* 1010 1100 */
	case 0xad: /* 1010 1101 */
	case 0xae: /* 1010 1110 */
	case 0xaf: /* 1010 1111 */
		util::stream_format(stream, "INS  H'%02X'", op & 0x0f);
		break;

	case 0xb0: /* 1011 0000 */
	case 0xb1: /* 1011 0001 */
		util::stream_format(stream, "OUTS %d", op & 0x0f);
		break;

	/* case 0xb2: 1011 0010 */
	/* case 0xb3: 1011 0011 */

	case 0xb4: /* 1011 0100 */
	case 0xb5: /* 1011 0101 */
	case 0xb6: /* 1011 0110 */
	case 0xb7: /* 1011 0111 */
	case 0xb8: /* 1011 1000 */
	case 0xb9: /* 1011 1001 */
	case 0xba: /* 1011 1010 */
	case 0xbb: /* 1011 1011 */
	case 0xbc: /* 1011 1100 */
	case 0xbd: /* 1011 1101 */
	case 0xbe: /* 1011 1110 */
	case 0xbf: /* 1011 1111 */
		util::stream_format(stream, "OUTS H'%02X'", op & 0x0f);
		break;

	case 0xc0: /* 1100 0000 */
	case 0xc1: /* 1100 0001 */
	case 0xc2: /* 1100 0010 */
	case 0xc3: /* 1100 0011 */
	case 0xc4: /* 1100 0100 */
	case 0xc5: /* 1100 0101 */
	case 0xc6: /* 1100 0110 */
	case 0xc7: /* 1100 0111 */
	case 0xc8: /* 1100 1000 */
	case 0xc9: /* 1100 1001 */
	case 0xca: /* 1100 1010 */
	case 0xcb: /* 1100 1011 */
		util::stream_format(stream, "AS   %s", rname[op & 15]);
		break;
	case 0xcc: /* 1100 1100 */
		util::stream_format(stream, "AS   S");
		break;
	case 0xcd: /* 1100 1101 */
		util::stream_format(stream, "AS   I");
		break;
	case 0xce: /* 1100 1110 */
		util::stream_format(stream, "AS   D");
		break;
	/* case 0xcf: 1100 1111 */

	case 0xd0: /* 1101 0000 */
	case 0xd1: /* 1101 0001 */
	case 0xd2: /* 1101 0010 */
	case 0xd3: /* 1101 0011 */
	case 0xd4: /* 1101 0100 */
	case 0xd5: /* 1101 0101 */
	case 0xd6: /* 1101 0110 */
	case 0xd7: /* 1101 0111 */
	case 0xd8: /* 1101 1000 */
	case 0xd9: /* 1101 1001 */
	case 0xda: /* 1101 1010 */
	case 0xdb: /* 1101 1011 */
		util::stream_format(stream, "ASD  %s", rname[op & 15]);
		break;
	case 0xdc: /* 1101 1100 */
		util::stream_format(stream, "ASD  S");
		break;
	case 0xdd: /* 1101 1101 */
		util::stream_format(stream, "ASD  I");
		break;
	case 0xde: /* 1101 1110 */
		util::stream_format(stream, "ASD  D");
		break;
	/* case 0xdf: 1101 1111 */

	case 0xe0: /* 1110 0000 */
	case 0xe1: /* 1110 0001 */
	case 0xe2: /* 1110 0010 */
	case 0xe3: /* 1110 0011 */
	case 0xe4: /* 1110 0100 */
	case 0xe5: /* 1110 0101 */
	case 0xe6: /* 1110 0110 */
	case 0xe7: /* 1110 0111 */
	case 0xe8: /* 1110 1000 */
	case 0xe9: /* 1110 1001 */
	case 0xea: /* 1110 1010 */
	case 0xeb: /* 1110 1011 */
		util::stream_format(stream, "XS   %s", rname[op & 15]);
		break;
	case 0xec: /* 1110 1100 */
		util::stream_format(stream, "XS   S");
		break;
	case 0xed: /* 1110 1101 */
		util::stream_format(stream, "XS   I");
		break;
	case 0xee: /* 1110 1110 */
		util::stream_format(stream, "XS   D");
		break;
	/* case 0xef: 1110 1111 */


	case 0xf0: /* 1111 0000 */
	case 0xf1: /* 1111 0001 */
	case 0xf2: /* 1111 0010 */
	case 0xf3: /* 1111 0011 */
	case 0xf4: /* 1111 0100 */
	case 0xf5: /* 1111 0101 */
	case 0xf6: /* 1111 0110 */
	case 0xf7: /* 1111 0111 */
	case 0xf8: /* 1111 1000 */
	case 0xf9: /* 1111 1001 */
	case 0xfa: /* 1111 1010 */
	case 0xfb: /* 1111 1011 */
		util::stream_format(stream, "NS   %s", rname[op & 15]);
		break;
	case 0xfc: /* 1111 1100 */
		util::stream_format(stream, "NS   S");
		break;
	case 0xfd: /* 1111 1101 */
		util::stream_format(stream, "NS   I");
		break;
	case 0xfe: /* 1111 1110 */
		util::stream_format(stream, "NS   D");
		break;
	/* case 0xff: 1111 1111 */


	default:
		util::stream_format(stream, "DC   H'%02X' (?)", op);
		break;
	}

	return (pc - base_pc) | flags;
}
