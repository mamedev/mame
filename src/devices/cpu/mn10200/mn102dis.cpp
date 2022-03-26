// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 disassembler
*/

#include "emu.h"
#include "mn102dis.h"

u32 mn10200_disassembler::r24(const data_buffer &opcodes, offs_t pc)
{
	return opcodes.r16(pc) | (opcodes.r8(pc+2) << 16);
}

std::string mn10200_disassembler::i8str(s8 v)
{
	if(v>=0)
		return util::string_format("$%x", v);
	else
		return util::string_format("-$%x", u8(-v));
}

std::string mn10200_disassembler::i16str(int16_t v)
{
	if(v>=0)
		return util::string_format("$%x", v);
	else
		return util::string_format("-$%x", u16(-v));
}

std::string mn10200_disassembler::i24str(u32 v)
{
	if(!(v & 0x800000))
		return util::string_format("$%x", v & 0xffffff);
	else
		return util::string_format("-$%x", (-v) & 0xffffff);
}

u32 mn10200_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t mn10200_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 opcode;

	opcode = opcodes.r8(pc);
	switch(opcode)
	{
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		util::stream_format(stream, "mov d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
		return 1 | SUPPORTED;

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		util::stream_format(stream, "movb d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
		return 1 | SUPPORTED;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		util::stream_format(stream, "mov (a%d), d%d", (opcode>>2) & 3, opcode & 3);
		return 1 | SUPPORTED;

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		util::stream_format(stream, "movbu (a%d), d%d", (opcode>>2) & 3, opcode & 3);
		return 1 | SUPPORTED;

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		util::stream_format(stream, "mov d%d, (%s, a%d)", opcode & 3, i8str(opcodes.r8(pc+1)), (opcode>>2) & 3);
		return 2;

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		util::stream_format(stream, "mov a%d, (%s, a%d)", opcode & 3, i8str(opcodes.r8(pc+1)), (opcode>>2) & 3);
		return 2;

	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		util::stream_format(stream, "mov (%s, a%d), d%d", i8str(opcodes.r8(pc+1)), (opcode>>2) & 3, opcode & 3);
		return 2;

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		util::stream_format(stream, "mov (%s, a%d), a%d", i8str(opcodes.r8(pc+1)), (opcode>>2) & 3, opcode & 3);
		return 2;

	case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
		util::stream_format(stream, "mov d%d, d%d", (opcode>>2) & 3, opcode & 3);
		return 1 | SUPPORTED;

	case 0x80: case 0x85: case 0x8a: case 0x8f:
		util::stream_format(stream, "mov %s, d%d", i8str(opcodes.r8(pc+1)), opcode & 3);
		return 2;

	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		util::stream_format(stream, "add d%d, d%d", (opcode>>2) & 3, opcode & 3);
		return 1 | SUPPORTED;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		util::stream_format(stream, "sub d%d, d%d", (opcode>>2) & 3, opcode & 3);
		return 1 | SUPPORTED;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		util::stream_format(stream, "extx d%d", opcode & 3);
		return 1 | SUPPORTED;

	case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		util::stream_format(stream, "extxu d%d", opcode & 3);
		return 1 | SUPPORTED;

	case 0xb8: case 0xb9: case 0xba: case 0xbb:
		util::stream_format(stream, "extxb d%d", opcode & 3);
		return 1 | SUPPORTED;

	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		util::stream_format(stream, "extxbu d%d", opcode & 3);
		return 1 | SUPPORTED;

	case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		util::stream_format(stream, "mov d%d, ($%04x)", opcode & 3, opcodes.r16(pc+1));
		return 3 | SUPPORTED;

	case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		util::stream_format(stream, "movb d%d, ($%04x)", opcode & 3, opcodes.r16(pc+1));
		return 3 | SUPPORTED;

	case 0xc8: case 0xc9: case 0xca: case 0xcb:
		util::stream_format(stream, "mov ($%04x), d%d", opcodes.r16(pc+1), opcode & 3);
		return 3 | SUPPORTED;

	case 0xcc: case 0xcd: case 0xce: case 0xcf:
		util::stream_format(stream, "movbu ($%04x), d%d", opcodes.r16(pc+1), opcode & 3);
		return 3 | SUPPORTED;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		util::stream_format(stream, "add %s, a%d", i8str(opcodes.r8(pc+1)), opcode & 3);
		return 2 | SUPPORTED;

	case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		util::stream_format(stream, "add %s, d%d", i8str(opcodes.r8(pc+1)), opcode & 3);
		return 2 | SUPPORTED;

	case 0xd8: case 0xd9: case 0xda: case 0xdb:
		util::stream_format(stream, "cmp %s, d%d", i8str(opcodes.r8(pc+1)), opcode & 3);
		return 2 | SUPPORTED;

	case 0xdc: case 0xdd: case 0xde: case 0xdf:
		util::stream_format(stream, "move $%04x, a%d", opcodes.r16(pc+1), opcode & 3);
		return 3 | SUPPORTED;

	case 0xe0:
		util::stream_format(stream, "blt $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe1:
		util::stream_format(stream, "bgt $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe2:
		util::stream_format(stream, "bge $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe3:
		util::stream_format(stream, "ble $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe4:
		util::stream_format(stream, "bcs $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe5:
		util::stream_format(stream, "bhi $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe6:
		util::stream_format(stream, "bcc $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe7:
		util::stream_format(stream, "bls $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe8:
		util::stream_format(stream, "beq $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xe9:
		util::stream_format(stream, "bne $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xea:
		util::stream_format(stream, "bra $%x", (pc+2+(s8)opcodes.r8(pc+1)) & 0xffffff);
		return 2 | SUPPORTED;

	case 0xeb:
		util::stream_format(stream, "rti");
		return 1 | STEP_OUT | SUPPORTED;

	case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "cmp $%04x, a%d", opcodes.r16(pc+1), opcode & 3);
		return 3 | SUPPORTED;

	case 0xf0:
		opcode = opcodes.r8(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x04: case 0x08: case 0x0c:
			util::stream_format(stream, "jmp (a%d)", (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0x01: case 0x05: case 0x09: case 0x0d:
			util::stream_format(stream, "jsr (a%d)", (opcode>>2) & 3);
			return 2 | STEP_OVER | SUPPORTED;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			util::stream_format(stream, "bset d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			util::stream_format(stream, "bclr d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			util::stream_format(stream, "movb (d%d, a%d), d%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			util::stream_format(stream, "movbu (d%d, a%d), d%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
			util::stream_format(stream, "movb d%d, (d%d, a%d)", opcode & 3, (opcode>>4) & 3, (opcode>>2) & 3);
			return 2 | SUPPORTED;

		default:
			goto illegal2;
		}

	case 0xf1:
		opcode = opcodes.r8(pc+1);
		switch(opcode&0xc0)
		{
		case 0x00:
			util::stream_format(stream, "mov (d%d, a%d), a%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x40:
			util::stream_format(stream, "mov (d%d, a%d), d%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x80:
			util::stream_format(stream, "mov a%d, (d%d, a%d)", opcode & 3, (opcode>>4) & 3, (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0xc0:
			util::stream_format(stream, "mov d%d, (d%d, a%d)", opcode & 3, (opcode>>4) & 3, (opcode>>2) & 3);
			return 2 | SUPPORTED;
		}
		break;

	case 0xf2:
		opcode = opcodes.r8(pc+1);
		switch(opcode&0xf0)
		{
		case 0x00:
			util::stream_format(stream, "add d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x10:
			util::stream_format(stream, "sub d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x20:
			util::stream_format(stream, "cmp d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x30:
			util::stream_format(stream, "mov d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x40:
			util::stream_format(stream, "add a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x50:
			util::stream_format(stream, "sub a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x60:
			util::stream_format(stream, "cmp a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x70:
			util::stream_format(stream, "mov a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x80:
			util::stream_format(stream, "addc d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x90:
			util::stream_format(stream, "subc d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0xc0:
			util::stream_format(stream, "add a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0xd0:
			util::stream_format(stream, "sub a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0xe0:
			util::stream_format(stream, "cmp a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0xf0:
			util::stream_format(stream, "mov a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		default:
			goto illegal2;
		}

	case 0xf3:
		opcode = opcodes.r8(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "and d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			util::stream_format(stream, "or d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			util::stream_format(stream, "xor d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x30: case 0x31: case 0x32: case 0x33:
			util::stream_format(stream, "rol d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0x34: case 0x35: case 0x36: case 0x37:
			util::stream_format(stream, "ror d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0x38: case 0x39: case 0x3a: case 0x3b:
			util::stream_format(stream, "asr d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			util::stream_format(stream, "lsr d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			util::stream_format(stream, "mul d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			util::stream_format(stream, "mulu d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			util::stream_format(stream, "divu d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			util::stream_format(stream, "cmp d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2 | SUPPORTED;

		case 0xc0: case 0xc4: case 0xc8: case 0xcc:
			util::stream_format(stream, "mov d%d, mdr", (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0xc1: case 0xc5: case 0xc9: case 0xcd:
			util::stream_format(stream, "ext d%d", (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0xd0: case 0xd4: case 0xd8: case 0xdc:
			util::stream_format(stream, "mov d%d, psw", (opcode>>2) & 3);
			return 2 | SUPPORTED;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
			util::stream_format(stream, "mov mdr, d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			util::stream_format(stream, "not d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
			util::stream_format(stream, "mov psw, d%d", opcode & 3);
			return 2 | SUPPORTED;

		case 0xfc:
			util::stream_format(stream, "pxst");
			return 2 | SUPPORTED;

		case 0xfe:
			opcode = opcodes.r8(pc+2);
			switch(opcode)
			{
			case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				util::stream_format(stream, "tbz ($%x) %d, $%x", r24(opcodes, pc+3), opcode & 7,
					(pc+7+(s8)opcodes.r8(pc+6)) & 0xffffff);
				return 7 | SUPPORTED;

			case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
				util::stream_format(stream, "tbnz ($%x) %d, $%x", r24(opcodes, pc+3), opcode & 7,
					(pc+7+(s8)opcodes.r8(pc+6)) & 0xffffff);
				return 7 | SUPPORTED;

			case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
				util::stream_format(stream, "bset ($%x) %d", r24(opcodes, pc+2), opcode & 7);
				return 6 | SUPPORTED;

			case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
				util::stream_format(stream, "bclr ($%x) %d", r24(opcodes, pc+2), opcode & 7);
				return 6 | SUPPORTED;

			default:
				goto illegal3;
			}

		case 0xff:
			opcode = opcodes.r8(pc+2);
			switch(opcode)
			{
			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				util::stream_format(stream, "tbz (%s, a%d) %d, $%x", i8str(opcodes.r8(pc+3)), 2+((opcode>>3)&1), opcode & 7,
					(pc+5+(s8)opcodes.r8(pc+4)) & 0xffffff);
				return 5 | SUPPORTED;

			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				util::stream_format(stream, "bset (%s, a%d) %d", i8str(opcodes.r8(pc+3)), 2+((opcode>>3)&1), opcode & 7);
				return 4 | SUPPORTED;

			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				util::stream_format(stream, "tbnz (%s, a%d) %d, $%x", i8str(opcodes.r8(pc+3)), 2+((opcode>>3)&1), opcode & 7,
					(pc+5+(s8)opcodes.r8(pc+4)) & 0xffffff);
				return 5 | SUPPORTED;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				util::stream_format(stream, "bclr (%s, a%d) %d", i8str(opcodes.r8(pc+3)), 2+((opcode>>3)&1), opcode & 7);
				return 4 | SUPPORTED;

			default:
				goto illegal3;
			}

		default:
			goto illegal2;
		}

	case 0xf4:
		opcode = opcodes.r8(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "mov d%d, (%s, a%d)", opcode & 3, i24str(r24(opcodes, pc+2)), (opcode>>2) & 3);
			return 5 | SUPPORTED;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			util::stream_format(stream, "mov a%d, (%s, a%d)", opcode & 3, i24str(r24(opcodes, pc+2)), (opcode>>2) & 3);
			return 5 | SUPPORTED;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			util::stream_format(stream, "movb d%d, (%s, a%d)", opcode & 3, i24str(r24(opcodes, pc+2)), (opcode>>2) & 3);
			return 5 | SUPPORTED;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			util::stream_format(stream, "movx d%d, (%s, a%d)", opcode & 3, i24str(r24(opcodes, pc+2)), (opcode>>2) & 3);
			return 5 | SUPPORTED;

		case 0x40: case 0x41: case 0x42: case 0x43:
			util::stream_format(stream, "mov d%d, ($%06x)", opcode & 3, r24(opcodes, pc+2));
			return 5 | SUPPORTED;

		case 0x44: case 0x45: case 0x46: case 0x47:
			util::stream_format(stream, "movb d%d, ($%06x)", opcode & 3, r24(opcodes, pc+2));
			return 5 | SUPPORTED;

		case 0x4b:
			util::stream_format(stream, "bset %02x, ($%06x)", opcodes.r8(pc+5), r24(opcodes, pc+2));
			return 6 | SUPPORTED;

		case 0x4f:
			util::stream_format(stream, "bclr %02x, ($%06x)", opcodes.r8(pc+5), r24(opcodes, pc+2));
			return 6 | SUPPORTED;

		case 0x50: case 0x51: case 0x52: case 0x53:
			util::stream_format(stream, "mov a%d, ($%06x)", opcode & 3, r24(opcodes, pc+2));
			return 5 | SUPPORTED;

		case 0x60: case 0x61: case 0x62: case 0x63:
			util::stream_format(stream, "add %s, d%d", i24str(r24(opcodes, pc+2)), opcode & 3);
			return 5 | SUPPORTED;

		case 0x64: case 0x65: case 0x66: case 0x67:
			util::stream_format(stream, "add %s, a%d", i24str(r24(opcodes, pc+2)), opcode & 3);
			return 5 | SUPPORTED;

		case 0x68: case 0x69: case 0x6a: case 0x6b:
			util::stream_format(stream, "sub %s, d%d", i24str(r24(opcodes, pc+2)), opcode & 3);
			return 5 | SUPPORTED;

		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			util::stream_format(stream, "sub %s, a%d", i24str(r24(opcodes, pc+2)), opcode & 3);
			return 5 | SUPPORTED;

		case 0x70: case 0x71: case 0x72: case 0x73:
			util::stream_format(stream, "mov %s, d%d", i24str(r24(opcodes, pc+2)), opcode & 3);
			return 5 | SUPPORTED;

		case 0x74: case 0x75: case 0x76: case 0x77:
			util::stream_format(stream, "mov $%06x, a%d", r24(opcodes, pc+2), opcode & 3);
			return 5 | SUPPORTED;

		case 0x78: case 0x79: case 0x7a: case 0x7b:
			util::stream_format(stream, "cmp %s, d%d", i24str(r24(opcodes, pc+2)), opcode & 3);
			return 5 | SUPPORTED;

		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			util::stream_format(stream, "cmp $%06x, a%d", r24(opcodes, pc+2), opcode & 3);
			return 5 | SUPPORTED;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			util::stream_format(stream, "mov (%s, a%d), d%d", i24str(r24(opcodes, pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5 | SUPPORTED;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			util::stream_format(stream, "movbu (%s, a%d), d%d", i24str(r24(opcodes, pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5 | SUPPORTED;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, "movb (%s, a%d), d%d", i24str(r24(opcodes, pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5 | SUPPORTED;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			util::stream_format(stream, "movx (%s, a%d), d%d", i24str(r24(opcodes, pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5 | SUPPORTED;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
			util::stream_format(stream, "mov ($%06x), d%d", r24(opcodes, pc+2), opcode & 3);
			return 5 | SUPPORTED;

		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			util::stream_format(stream, "movb ($%06x), d%d", r24(opcodes, pc+2), opcode & 3);
			return 5 | SUPPORTED;

		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			util::stream_format(stream, "movbu ($%06x), d%d", r24(opcodes, pc+2), opcode & 3);
			return 5 | SUPPORTED;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			util::stream_format(stream, "mov ($%06x), a%d", r24(opcodes, pc+2), opcode & 3);
			return 5 | SUPPORTED;

		case 0xe0:
			util::stream_format(stream, "jmp $%x", (pc+5+r24(opcodes, pc+2)) & 0xffffff);
			return 5 | SUPPORTED;

		case 0xe1:
			util::stream_format(stream, "jsr $%x", (pc+5+r24(opcodes, pc+2)) & 0xffffff);
			return 5 | STEP_OVER | SUPPORTED;

		case 0xe3:
			util::stream_format(stream, "bset $%02x, ($%x)", opcodes.r8(pc+4), opcodes.r16(pc+2));
			return 6 | SUPPORTED;

		case 0xe7:
			util::stream_format(stream, "bclr $%02x, ($%x)", opcodes.r8(pc+4), opcodes.r16(pc+2));
			return 6 | SUPPORTED;

		case 0xe8: case 0xe9: case 0xea: case 0xeb:
			util::stream_format(stream, "bset $%02x, (%s, a%d)", opcodes.r8(pc+3), i8str(opcodes.r8(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0xec: case 0xed: case 0xee: case 0xef:
			util::stream_format(stream, "bclr $%02x, (%s, a%d)", opcodes.r8(pc+3), i8str(opcodes.r8(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
			util::stream_format(stream, "mov (%s, a%d), a%d", i24str(r24(opcodes, pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5 | SUPPORTED;

		default:
			goto illegal2;
		}

	case 0xf5:
		opcode = opcodes.r8(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
			util::stream_format(stream, "and $%02x, d%d", opcodes.r8(pc+2), opcode & 3);
			return 3 | SUPPORTED;

		case 0x04: case 0x05: case 0x06: case 0x07:
			util::stream_format(stream, "btst $%02x, d%d", opcodes.r8(pc+2), opcode & 3);
			return 3 | SUPPORTED;

		case 0x08: case 0x09: case 0x0a: case 0x0b:
			util::stream_format(stream, "or $%02x, d%d", opcodes.r8(pc+2), opcode & 3);
			return 3 | SUPPORTED;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "addnf %s, a%d", i8str(opcodes.r8(pc+2)), opcode & 3);
			return 3 | SUPPORTED;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			util::stream_format(stream, "movb d%d, (%s, a%d)", opcode & 3, i8str(opcodes.r8(pc+2)), (opcode>>2) & 3);
			return 3 | SUPPORTED;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			util::stream_format(stream, "movb (%s, a%d), d%d", i8str(opcodes.r8(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 3 | SUPPORTED;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			util::stream_format(stream, "movbu (%s, a%d), d%d", i8str(opcodes.r8(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 3 | SUPPORTED;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		{
			u8 opcode2 = opcodes.r8(pc+2);
			switch(opcode2)
			{
			case 0x00:
				util::stream_format(stream, "mulql d%d, d%d", (opcode>>3) & 3, opcode & 3);
				return 3 | SUPPORTED;

			case 0x01:
				util::stream_format(stream, "mulqh d%d, d%d", (opcode>>3) & 3, opcode & 3);
				return 3 | SUPPORTED;

			default:
				goto illegal3;
			}
		}

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			util::stream_format(stream, "movx d%d, (%s, a%d)", opcode & 3, i8str(opcodes.r8(pc+2)), (opcode>>2) & 3);
			return 3 | SUPPORTED;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		{
			u8 opcode2 = opcodes.r8(pc+2);
			switch(opcode2)
			{
			case 0x10:
				util::stream_format(stream, "mulq d%d, d%d", (opcode>>3) & 3, opcode & 3);
				return 3 | SUPPORTED;

			default:
				goto illegal3;
			}
		}

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			util::stream_format(stream, "movx (%s, a%d), d%d", i8str(opcodes.r8(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 3 | SUPPORTED;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			util::stream_format(stream, "tbz (%s, a%d) %d, $%x", i8str(opcodes.r8(pc+2)), (opcode>>3)&1, opcode & 7,
					(pc+4+(s8)opcodes.r8(pc+3)) & 0xffffff);
			return 4 | SUPPORTED;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			util::stream_format(stream, "bset (%s, a%d) %d", i8str(opcodes.r8(pc+2)), (opcode>>3)&1, opcode & 7);
			return 3 | SUPPORTED;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, "tbnz (%s, a%d) %d, $%x", i8str(opcodes.r8(pc+2)), (opcode>>3)&1, opcode & 7,
					(pc+4+(s8)opcodes.r8(pc+3)) & 0xffffff);
			return 4 | SUPPORTED;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			util::stream_format(stream, "bclr (%s, a%d) %d", i8str(opcodes.r8(pc+2)), (opcode>>3)&1, opcode & 7);
			return 3 | SUPPORTED;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			util::stream_format(stream, "tbz ($%x) %d, $%x", opcodes.r16(pc+2), opcode & 7,
					(pc+5+(s8)opcodes.r8(pc+4)) & 0xffffff);
			return 5 | SUPPORTED;

		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
			util::stream_format(stream, "tbnz ($%x) %d, $%x", opcodes.r16(pc+2), opcode & 7,
					(pc+5+(s8)opcodes.r8(pc+4)) & 0xffffff);
			return 5 | SUPPORTED;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			util::stream_format(stream, "bset ($%x) %d", opcodes.r16(pc+2), opcode & 7);
			return 4 | SUPPORTED;

		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
			util::stream_format(stream, "bclr ($%x) %d", opcodes.r16(pc+2), opcode & 7);
			return 4 | SUPPORTED;

		case 0xe0:
			util::stream_format(stream, "bltx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe1:
			util::stream_format(stream, "bgtx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe2:
			util::stream_format(stream, "bgex $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe3:
			util::stream_format(stream, "blex $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe4:
			util::stream_format(stream, "bcsx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe5:
			util::stream_format(stream, "bhix $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe6:
			util::stream_format(stream, "bccx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe7:
			util::stream_format(stream, "blsx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe8:
			util::stream_format(stream, "beqx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xe9:
			util::stream_format(stream, "bnex $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xec:
			util::stream_format(stream, "bvcx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xed:
			util::stream_format(stream, "bvsx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xee:
			util::stream_format(stream, "bncx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xef:
			util::stream_format(stream, "bnsx $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		{
			u8 opcode2 = opcodes.r8(pc+2);
			switch(opcode2)
			{
			case 0x04:
				util::stream_format(stream, "mulql %s, d%d", i8str(opcodes.r8(pc+3)), opcode & 3);
				return 4 | SUPPORTED;

			case 0x05:
				util::stream_format(stream, "mulqh %s, d%d", i8str(opcodes.r8(pc+3)), opcode & 3);
				return 4 | SUPPORTED;

			case 0x08:
				util::stream_format(stream, "mulql %s, d%d", i16str(opcodes.r16(pc+3)), opcode & 3);
				return 5 | SUPPORTED;

			case 0x09:
				util::stream_format(stream, "mulqh %s, d%d", i16str(opcodes.r16(pc+3)), opcode & 3);
				return 5 | SUPPORTED;

			default:
				goto illegal3;
			}
		}

		case 0xfc:
			util::stream_format(stream, "bvc $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xfd:
			util::stream_format(stream, "bvs $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xfe:
			util::stream_format(stream, "bnc $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		case 0xff:
			util::stream_format(stream, "bns $%x", (pc+3+s8(opcodes.r8(pc+2))) & 0xffffff);
			return 3 | SUPPORTED;

		default:
			goto illegal2;
		}

	case 0xf6:
		util::stream_format(stream, "nop");
		return 1 | SUPPORTED;

	case 0xf7:
		opcode = opcodes.r8(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
			util::stream_format(stream, "and $%04x, d%d", opcodes.r16(pc+2), opcode & 3);
			return 4 | SUPPORTED;

		case 0x04: case 0x05: case 0x06: case 0x07:
			util::stream_format(stream, "btst $%04x, d%d", opcodes.r16(pc+2), opcode & 3);
			return 4 | SUPPORTED;

		case 0x08: case 0x09: case 0x0a: case 0x0b:
			util::stream_format(stream, "add %s, a%d", i16str(opcodes.r16(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "sub %s, a%d", i16str(opcodes.r16(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0x10:
			util::stream_format(stream, "and $%04x, psw", opcodes.r16(pc+2));
			return 4 | SUPPORTED;

		case 0x14:
			util::stream_format(stream, "or $%04x, psw", opcodes.r16(pc+2));
			return 4 | SUPPORTED;

		case 0x18: case 0x19: case 0x1a: case 0x1b:
			util::stream_format(stream, "add %s, d%d", i16str(opcodes.r16(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			util::stream_format(stream, "sub %s, d%d", i16str(opcodes.r16(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0x20: case 0x21: case 0x22: case 0x23:
			util::stream_format(stream, "mov a%d, ($%04x)", opcode & 3, opcodes.r16(pc+2));
			return 4 | SUPPORTED;

		case 0x30: case 0x31: case 0x32: case 0x33:
			util::stream_format(stream, "mov ($%04x), a%d", opcodes.r16(pc+2), opcode & 3);
			return 4 | SUPPORTED;

		case 0x40: case 0x41: case 0x42: case 0x43:
			util::stream_format(stream, "or $%04x, d%d", opcodes.r16(pc+2), opcode & 3);
			return 4 | SUPPORTED;

		case 0x48: case 0x49: case 0x4a: case 0x4b:
			util::stream_format(stream, "cmp %s, d%d", i16str(opcodes.r16(pc+2)), opcode & 3);
			return 4 | SUPPORTED;

		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			util::stream_format(stream, "xor $%04x, d%d", opcodes.r16(pc+2), opcode & 3);
			return 4 | SUPPORTED;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			util::stream_format(stream, "movbu (%s, a%d), d%d", i16str(opcodes.r16(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4 | SUPPORTED;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			util::stream_format(stream, "movx d%d, (%s, a%d)", opcode & 3, i16str(opcodes.r16(pc+2)), (opcode>>2) & 3);
			return 4 | SUPPORTED;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			util::stream_format(stream, "movx (%s, a%d), d%d", i16str(opcodes.r16(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4 | SUPPORTED;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			util::stream_format(stream, "mov d%d, (%s, a%d)", opcode & 3, i16str(opcodes.r16(pc+2)), (opcode>>2) & 3);
			return 4 | SUPPORTED;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			util::stream_format(stream, "movb d%d, (%s, a%d)", opcode & 3, i16str(opcodes.r16(pc+2)), (opcode>>2) & 3);
			return 4 | SUPPORTED;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, "mov a%d, (%s, a%d)", opcode & 3, i16str(opcodes.r16(pc+2)), (opcode>>2) & 3);
			return 4 | SUPPORTED;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			util::stream_format(stream, "mov (%s, a%d), a%d", i16str(opcodes.r16(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4 | SUPPORTED;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
			util::stream_format(stream, "mov (%s, a%d), d%d", i16str(opcodes.r16(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4 | SUPPORTED;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
			util::stream_format(stream, "movb (%s, a%d), d%d", i16str(opcodes.r16(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4 | SUPPORTED;

		default:
			goto illegal2;
		}

	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		util::stream_format(stream, "mov %s, d%d", i16str(opcodes.r16(pc+1)), opcode & 3);
		return 3 | SUPPORTED;

	case 0xfc:
		util::stream_format(stream, "jmp $%x", (pc+3+s16(opcodes.r16(pc+1))) & 0xffffff);
		return 3 | SUPPORTED;

	case 0xfd:
		util::stream_format(stream, "jsr $%x", (pc+3+s16(opcodes.r16(pc+1))) & 0xffffff);
		return 3 | STEP_OVER | SUPPORTED;

	case 0xfe:
		util::stream_format(stream, "rts");
		return 1 | STEP_OUT | SUPPORTED;

	default:
		goto illegal1;
	};

	illegal1:
		util::stream_format(stream, "dc.b $%02x", opcodes.r8(pc));
		return 1 | SUPPORTED;

	illegal2:
		util::stream_format(stream, "dc.b $%02x $%02x", opcodes.r8(pc), opcodes.r8(pc+1));
		return 2 | SUPPORTED;

	illegal3:
		util::stream_format(stream, "dc.b $%02x $%02x $%02x", opcodes.r8(pc), opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;
}
