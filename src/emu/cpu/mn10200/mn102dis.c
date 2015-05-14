// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 disassembler
*/

#include "emu.h"

#include <stdio.h>

static const UINT8 *sOpROM; // current opROM pointer
static UINT32 sBasePC;

static UINT8 program_read_byte(offs_t pc)
{
	return sOpROM[pc - sBasePC];
}

static UINT32 r16u(offs_t pc)
{
	return sOpROM[pc - sBasePC] | (sOpROM[pc - sBasePC + 1]<<8);
}

static INT32 r16s(offs_t pc)
{
	return (INT16)(sOpROM[pc - sBasePC] | (sOpROM[pc - sBasePC + 1]<<8));
}

static UINT32 r24u(offs_t pc)
{
	return sOpROM[pc - sBasePC] | (sOpROM[pc - sBasePC + 1]<<8) | (sOpROM[pc - sBasePC + 2]<<16);
}

static INT32 r24s(offs_t pc)
{
	return sOpROM[pc - sBasePC] | (sOpROM[pc - sBasePC + 1]<<8) | ((INT8)sOpROM[pc - sBasePC + 2]<<16);
}

static const char *i8str(INT8 v)
{
	static char res[0x10];
	if(v>=0)
	sprintf(res, "$%x", v);
	else
	sprintf(res, "-$%x", (UINT8)(-v));
	return res;
}

static const char *i16str(INT16 v)
{
	static char res[0x10];
	if(v>=0)
	sprintf(res, "$%x", v);
	else
	sprintf(res, "-$%x", (UINT16)(-v));
	return res;
}

static const char *i24str(INT32 v)
{
	static char res[0x10];
	if(v>=0)
	sprintf(res, "$%x", v);
	else
	sprintf(res, "-$%x", -v);
	return res;
}


static int mn102_disassemble(char *buffer, UINT32 pc, const UINT8 *oprom)
{
	UINT8 opcode;

	sOpROM = oprom;
	sBasePC = pc;

	opcode = program_read_byte(pc);
	switch(opcode)
	{
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		sprintf(buffer, "mov d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
		return 1;

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		sprintf(buffer, "movb d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
		return 1;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		sprintf(buffer, "mov (a%d), d%d", (opcode>>2) & 3, opcode & 3);
		return 1;

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		sprintf(buffer, "movbu (a%d), d%d", (opcode>>2) & 3, opcode & 3);
		return 1;

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		sprintf(buffer, "mov d%d, (%s, a%d)", opcode & 3, i8str(program_read_byte(pc+1)), (opcode>>2) & 3);
		return 2;

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		sprintf(buffer, "mov a%d, (%s, a%d)", opcode & 3, i8str(program_read_byte(pc+1)), (opcode>>2) & 3);
		return 2;

	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		sprintf(buffer, "mov (%s, a%d), d%d", i8str(program_read_byte(pc+1)), (opcode>>2) & 3, opcode & 3);
		return 2;

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		sprintf(buffer, "mov (%s, a%d), a%d", i8str(program_read_byte(pc+1)), (opcode>>2) & 3, opcode & 3);
		return 2;

	case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
		sprintf(buffer, "mov d%d, d%d", (opcode>>2) & 3, opcode & 3);
		return 1;

	case 0x80: case 0x85: case 0x8a: case 0x8f:
		sprintf(buffer, "mov %s, d%d", i8str(program_read_byte(pc+1)), opcode & 3);
		return 2;

	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		sprintf(buffer, "add d%d, d%d", (opcode>>2) & 3, opcode & 3);
		return 1;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		sprintf(buffer, "sub d%d, d%d", (opcode>>2) & 3, opcode & 3);
		return 1;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		sprintf(buffer, "extx d%d", opcode & 3);
		return 1;

	case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		sprintf(buffer, "extxu d%d", opcode & 3);
		return 1;

	case 0xb8: case 0xb9: case 0xba: case 0xbb:
		sprintf(buffer, "extxb d%d", opcode & 3);
		return 1;

	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		sprintf(buffer, "extxbu d%d", opcode & 3);
		return 1;

	case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		sprintf(buffer, "mov d%d, ($%04x)", opcode & 3, r16u(pc+1));
		return 3;

	case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		sprintf(buffer, "movb d%d, ($%04x)", opcode & 3, r16u(pc+1));
		return 3;

	case 0xc8: case 0xc9: case 0xca: case 0xcb:
		sprintf(buffer, "mov ($%04x), d%d", r16u(pc+1), opcode & 3);
		return 3;

	case 0xcc: case 0xcd: case 0xce: case 0xcf:
		sprintf(buffer, "movbu ($%04x), d%d", r16u(pc+1), opcode & 3);
		return 3;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		sprintf(buffer, "add %s, a%d", i8str(program_read_byte(pc+1)), opcode & 3);
		return 2;

	case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		sprintf(buffer, "add %s, d%d", i8str(program_read_byte(pc+1)), opcode & 3);
		return 2;

	case 0xd8: case 0xd9: case 0xda: case 0xdb:
		sprintf(buffer, "cmp %s, d%d", i8str(program_read_byte(pc+1)), opcode & 3);
		return 2;

	case 0xdc: case 0xdd: case 0xde: case 0xdf:
		sprintf(buffer, "move $%04x, a%d", r16u(pc+1), opcode & 3);
		return 3;

	case 0xe0:
		sprintf(buffer, "blt $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe1:
		sprintf(buffer, "bgt $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe2:
		sprintf(buffer, "bge $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe3:
		sprintf(buffer, "ble $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe4:
		sprintf(buffer, "bcs $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe5:
		sprintf(buffer, "bhi $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe6:
		sprintf(buffer, "bcc $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe7:
		sprintf(buffer, "bls $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe8:
		sprintf(buffer, "beq $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xe9:
		sprintf(buffer, "bne $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xea:
		sprintf(buffer, "bra $%x", (pc+2+(INT8)program_read_byte(pc+1)) & 0xffffff);
		return 2;

	case 0xeb:
		sprintf(buffer, "rti");
		return 1;

	case 0xec: case 0xed: case 0xee: case 0xef:
		sprintf(buffer, "cmp $%04x, a%d", r16u(pc+1), opcode & 3);
		return 3;

	case 0xf0:
		opcode = program_read_byte(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x04: case 0x08: case 0x0c:
			sprintf(buffer, "jmp (a%d)", (opcode>>2) & 3);
			return 2;

		case 0x01: case 0x05: case 0x09: case 0x0d:
			sprintf(buffer, "jsr (a%d)", (opcode>>2) & 3);
			return 2;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			sprintf(buffer, "bset d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
			return 2;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			sprintf(buffer, "bclr d%d, (a%d)", opcode & 3, (opcode>>2) & 3);
			return 2;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			sprintf(buffer, "movb (d%d, a%d), d%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			sprintf(buffer, "movbu (d%d, a%d), d%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
			sprintf(buffer, "movb d%d, (d%d, a%d)", opcode & 3, (opcode>>4) & 3, (opcode>>2) & 3);
			return 2;

		default:
			goto illegal2;
		}

	case 0xf1:
		opcode = program_read_byte(pc+1);
		switch(opcode&0xc0)
		{
		case 0x00:
			sprintf(buffer, "mov (d%d, a%d), a%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x40:
			sprintf(buffer, "mov (d%d, a%d), d%d", (opcode>>4) & 3, (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x80:
			sprintf(buffer, "mov a%d, (d%d, a%d)", opcode & 3, (opcode>>4) & 3, (opcode>>2) & 3);
			return 2;

		case 0xc0:
			sprintf(buffer, "mov d%d, (d%d, a%d)", opcode & 3, (opcode>>4) & 3, (opcode>>2) & 3);
			return 2;
		}
		break;

	case 0xf2:
		opcode = program_read_byte(pc+1);
		switch(opcode&0xf0)
		{
		case 0x00:
			sprintf(buffer, "add d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x10:
			sprintf(buffer, "sub d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x20:
			sprintf(buffer, "cmp d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x30:
			sprintf(buffer, "mov d%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x40:
			sprintf(buffer, "add a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x50:
			sprintf(buffer, "sub a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x60:
			sprintf(buffer, "cmp a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x70:
			sprintf(buffer, "mov a%d, a%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x80:
			sprintf(buffer, "addc d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x90:
			sprintf(buffer, "subc d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0xc0:
			sprintf(buffer, "add a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0xd0:
			sprintf(buffer, "sub a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0xe0:
			sprintf(buffer, "cmp a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0xf0:
			sprintf(buffer, "mov a%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		default:
			goto illegal2;
		}

	case 0xf3:
		opcode = program_read_byte(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			sprintf(buffer, "and d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			sprintf(buffer, "or d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			sprintf(buffer, "xor d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x30: case 0x31: case 0x32: case 0x33:
			sprintf(buffer, "rol d%d", opcode & 3);
			return 2;

		case 0x34: case 0x35: case 0x36: case 0x37:
			sprintf(buffer, "ror d%d", opcode & 3);
			return 2;

		case 0x38: case 0x39: case 0x3a: case 0x3b:
			sprintf(buffer, "asr d%d", opcode & 3);
			return 2;

		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			sprintf(buffer, "lsr d%d", opcode & 3);
			return 2;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			sprintf(buffer, "mul d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			sprintf(buffer, "mulu d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			sprintf(buffer, "divu d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			sprintf(buffer, "cmp d%d, d%d", (opcode>>2) & 3, opcode & 3);
			return 2;

		case 0xc0: case 0xc4: case 0xc8: case 0xcc:
			sprintf(buffer, "mov d%d, mdr", (opcode>>2) & 3);
			return 2;

		case 0xc1: case 0xc5: case 0xc9: case 0xcd:
			sprintf(buffer, "ext d%d", (opcode>>2) & 3);
			return 2;

		case 0xd0: case 0xd4: case 0xd8: case 0xdc:
			sprintf(buffer, "mov d%d, psw", (opcode>>2) & 3);
			return 2;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
			sprintf(buffer, "mov mdr, d%d", opcode & 3);
			return 2;

		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			sprintf(buffer, "not d%d", opcode & 3);
			return 2;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
			sprintf(buffer, "mov psw, d%d", opcode & 3);
			return 2;

		case 0xfc:
			sprintf(buffer, "pxst");
			return 2;

		case 0xfe:
			opcode = program_read_byte(pc+2);
			switch(opcode)
			{
			case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				sprintf(buffer, "tbz ($%x) %d, $%x", r24u(pc+3), opcode & 7,
					(pc+7+(INT8)program_read_byte(pc+6)) & 0xffffff);
				return 7;

			case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
				sprintf(buffer, "tbnz ($%x) %d, $%x", r24u(pc+3), opcode & 7,
					(pc+7+(INT8)program_read_byte(pc+6)) & 0xffffff);
				return 7;

			case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
				sprintf(buffer, "bset ($%x) %d", r24u(pc+2), opcode & 7);
				return 6;

			case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
				sprintf(buffer, "bclr ($%x) %d", r24u(pc+2), opcode & 7);
				return 6;

			default:
				goto illegal3;
			}

		case 0xff:
			opcode = program_read_byte(pc+2);
			switch(opcode)
			{
			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				sprintf(buffer, "tbz (%s, a%d) %d, $%x", i8str(program_read_byte(pc+3)), 2+((opcode>>3)&1), opcode & 7,
					(pc+5+(INT8)program_read_byte(pc+4)) & 0xffffff);
				return 5;

			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				sprintf(buffer, "bset (%s, a%d) %d", i8str(program_read_byte(pc+3)), 2+((opcode>>3)&1), opcode & 7);
				return 4;

			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				sprintf(buffer, "tbnz (%s, a%d) %d, $%x", i8str(program_read_byte(pc+3)), 2+((opcode>>3)&1), opcode & 7,
					(pc+5+(INT8)program_read_byte(pc+4)) & 0xffffff);
				return 5;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				sprintf(buffer, "bclr (%s, a%d) %d", i8str(program_read_byte(pc+3)), 2+((opcode>>3)&1), opcode & 7);
				return 4;

			default:
				goto illegal3;
			}

		default:
			goto illegal2;
		}

	case 0xf4:
		opcode = program_read_byte(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			sprintf(buffer, "mov d%d, (%s, a%d)", opcode & 3, i24str(r24s(pc+2)), (opcode>>2) & 3);
			return 5;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			sprintf(buffer, "mov a%d, (%s, a%d)", opcode & 3, i24str(r24s(pc+2)), (opcode>>2) & 3);
			return 5;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			sprintf(buffer, "movb d%d, (%s, a%d)", opcode & 3, i24str(r24s(pc+2)), (opcode>>2) & 3);
			return 5;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			sprintf(buffer, "movx d%d, (%s, a%d)", opcode & 3, i24str(r24s(pc+2)), (opcode>>2) & 3);
			return 5;

		case 0x40: case 0x41: case 0x42: case 0x43:
			sprintf(buffer, "mov d%d, ($%06x)", opcode & 3, r24u(pc+2));
			return 5;

		case 0x44: case 0x45: case 0x46: case 0x47:
			sprintf(buffer, "movb d%d, ($%06x)", opcode & 3, r24u(pc+2));
			return 5;

		case 0x4b:
			sprintf(buffer, "bset %02x, ($%06x)", program_read_byte(pc+5), r24u(pc+2));
			return 6;

		case 0x4f:
			sprintf(buffer, "bclr %02x, ($%06x)", program_read_byte(pc+5), r24u(pc+2));
			return 6;

		case 0x50: case 0x51: case 0x52: case 0x53:
			sprintf(buffer, "mov a%d, ($%06x)", opcode & 3, r24u(pc+2));
			return 5;

		case 0x60: case 0x61: case 0x62: case 0x63:
			sprintf(buffer, "add %s, d%d", i24str(r24s(pc+2)), opcode & 3);
			return 5;

		case 0x64: case 0x65: case 0x66: case 0x67:
			sprintf(buffer, "add %s, a%d", i24str(r24s(pc+2)), opcode & 3);
			return 5;

		case 0x68: case 0x69: case 0x6a: case 0x6b:
			sprintf(buffer, "sub %s, d%d", i24str(r24s(pc+2)), opcode & 3);
			return 5;

		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			sprintf(buffer, "sub %s, a%d", i24str(r24s(pc+2)), opcode & 3);
			return 5;

		case 0x70: case 0x71: case 0x72: case 0x73:
			sprintf(buffer, "mov %s, d%d", i24str(r24s(pc+2)), opcode & 3);
			return 5;

		case 0x74: case 0x75: case 0x76: case 0x77:
			sprintf(buffer, "mov $%06x, a%d", r24u(pc+2), opcode & 3);
			return 5;

		case 0x78: case 0x79: case 0x7a: case 0x7b:
			sprintf(buffer, "cmp %s, d%d", i24str(r24s(pc+2)), opcode & 3);
			return 5;

		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			sprintf(buffer, "cmp $%06x, a%d", r24u(pc+2), opcode & 3);
			return 5;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			sprintf(buffer, "mov (%s, a%d), d%d", i24str(r24s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			sprintf(buffer, "movbu (%s, a%d), d%d", i24str(r24s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			sprintf(buffer, "movb (%s, a%d), d%d", i24str(r24s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			sprintf(buffer, "movx (%s, a%d), d%d", i24str(r24s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
			sprintf(buffer, "mov ($%06x), d%d", r24u(pc+2), opcode & 3);
			return 5;

		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			sprintf(buffer, "movb ($%06x), d%d", r24u(pc+2), opcode & 3);
			return 5;

		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			sprintf(buffer, "movbu ($%06x), d%d", r24u(pc+2), opcode & 3);
			return 5;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			sprintf(buffer, "mov ($%06x), a%d", r24u(pc+2), opcode & 3);
			return 5;

		case 0xe0:
			sprintf(buffer, "jmp $%x", (pc+5+r24s(pc+2)) & 0xffffff);
			return 5;

		case 0xe1:
			sprintf(buffer, "jsr $%x", (pc+5+r24s(pc+2)) & 0xffffff);
			return 5;

		case 0xe3:
			sprintf(buffer, "bset $%02x, ($%x)", program_read_byte(pc+4), r16u(pc+2));
			return 6;

		case 0xe7:
			sprintf(buffer, "bclr $%02x, ($%x)", program_read_byte(pc+4), r16u(pc+2));
			return 6;

		case 0xe8: case 0xe9: case 0xea: case 0xeb:
			sprintf(buffer, "bset $%02x, (%s, a%d)", program_read_byte(pc+3), i8str(program_read_byte(pc+2)), opcode & 3);
			return 4;

		case 0xec: case 0xed: case 0xee: case 0xef:
			sprintf(buffer, "bclr $%02x, (%s, a%d)", program_read_byte(pc+3), i8str(program_read_byte(pc+2)), opcode & 3);
			return 4;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
			sprintf(buffer, "mov (%s, a%d), a%d", i24str(r24s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 5;

		default:
			goto illegal2;
		}

	case 0xf5:
		opcode = program_read_byte(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
			sprintf(buffer, "and $%02x, d%d", program_read_byte(pc+2), opcode & 3);
			return 3;

		case 0x04: case 0x05: case 0x06: case 0x07:
			sprintf(buffer, "btst $%02x, d%d", program_read_byte(pc+2), opcode & 3);
			return 3;

		case 0x08: case 0x09: case 0x0a: case 0x0b:
			sprintf(buffer, "or $%02x, d%d", program_read_byte(pc+2), opcode & 3);
			return 3;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			sprintf(buffer, "addnf %s, a%d", i8str(program_read_byte(pc+2)), opcode & 3);
			return 3;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			sprintf(buffer, "movb d%d, (%s, a%d)", opcode & 3, i8str(program_read_byte(pc+2)), (opcode>>2) & 3);
			return 3;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			sprintf(buffer, "movb (%s, a%d), d%d", i8str(program_read_byte(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 3;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			sprintf(buffer, "movbu (%s, a%d), d%d", i8str(program_read_byte(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 3;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		{
			UINT8 opcode2 = program_read_byte(pc+2);
			switch(opcode2)
			{
			case 0x00:
				sprintf(buffer, "mulql d%d, d%d", (opcode>>3) & 3, opcode & 3);
				return 3;

			case 0x01:
				sprintf(buffer, "mulqh d%d, d%d", (opcode>>3) & 3, opcode & 3);
				return 3;

			default:
				goto illegal3;
			}
		}

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			sprintf(buffer, "movx d%d, (%s, a%d)", opcode & 3, i8str(program_read_byte(pc+2)), (opcode>>2) & 3);
			return 3;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		{
			UINT8 opcode2 = program_read_byte(pc+2);
			switch(opcode2)
			{
			case 0x10:
				sprintf(buffer, "mulq d%d, d%d", (opcode>>3) & 3, opcode & 3);
				return 3;

			default:
				goto illegal3;
			}
		}

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			sprintf(buffer, "movx (%s, a%d), d%d", i8str(program_read_byte(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 3;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			sprintf(buffer, "tbz (%s, a%d) %d, $%x", i8str(program_read_byte(pc+2)), (opcode>>3)&1, opcode & 7,
					(pc+4+(INT8)program_read_byte(pc+3)) & 0xffffff);
			return 4;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			sprintf(buffer, "bset (%s, a%d) %d", i8str(program_read_byte(pc+2)), (opcode>>3)&1, opcode & 7);
			return 3;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			sprintf(buffer, "tbnz (%s, a%d) %d, $%x", i8str(program_read_byte(pc+2)), (opcode>>3)&1, opcode & 7,
					(pc+4+(INT8)program_read_byte(pc+3)) & 0xffffff);
			return 4;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			sprintf(buffer, "bclr (%s, a%d) %d", i8str(program_read_byte(pc+2)), (opcode>>3)&1, opcode & 7);
			return 3;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			sprintf(buffer, "tbz ($%x) %d, $%x", r16u(pc+2), opcode & 7,
					(pc+5+(INT8)program_read_byte(pc+4)) & 0xffffff);
			return 5;

		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
			sprintf(buffer, "tbnz ($%x) %d, $%x", r16u(pc+2), opcode & 7,
					(pc+5+(INT8)program_read_byte(pc+4)) & 0xffffff);
			return 5;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			sprintf(buffer, "bset ($%x) %d", r16u(pc+2), opcode & 7);
			return 4;

		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
			sprintf(buffer, "bclr ($%x) %d", r16u(pc+2), opcode & 7);
			return 4;

		case 0xe0:
			sprintf(buffer, "bltx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe1:
			sprintf(buffer, "bgtx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe2:
			sprintf(buffer, "bgex $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe3:
			sprintf(buffer, "blex $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe4:
			sprintf(buffer, "bcsx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe5:
			sprintf(buffer, "bhix $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe6:
			sprintf(buffer, "bccx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe7:
			sprintf(buffer, "blsx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe8:
			sprintf(buffer, "beqx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xe9:
			sprintf(buffer, "bnex $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xec:
			sprintf(buffer, "bvcx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xed:
			sprintf(buffer, "bvsx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xee:
			sprintf(buffer, "bncx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xef:
			sprintf(buffer, "bnsx $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		{
			UINT8 opcode2 = program_read_byte(pc+2);
			switch(opcode2)
			{
			case 0x04:
				sprintf(buffer, "mulql %s, d%d", i8str(program_read_byte(pc+3)), opcode & 3);
				return 4;

			case 0x05:
				sprintf(buffer, "mulqh %s, d%d", i8str(program_read_byte(pc+3)), opcode & 3);
				return 4;

			case 0x08:
				sprintf(buffer, "mulql %s, d%d", i16str(r16s(pc+3)), opcode & 3);
				return 5;

			case 0x09:
				sprintf(buffer, "mulqh %s, d%d", i16str(r16s(pc+3)), opcode & 3);
				return 5;

			default:
				goto illegal3;
			}
		}

		case 0xfc:
			sprintf(buffer, "bvc $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xfd:
			sprintf(buffer, "bvs $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xfe:
			sprintf(buffer, "bnc $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		case 0xff:
			sprintf(buffer, "bns $%x", (pc+3+(INT8)program_read_byte(pc+2)) & 0xffffff);
			return 3;

		default:
			goto illegal2;
		}

	case 0xf6:
		sprintf(buffer, "nop");
		return 1;

	case 0xf7:
		opcode = program_read_byte(pc+1);
		switch(opcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
			sprintf(buffer, "and $%04x, d%d", r16u(pc+2), opcode & 3);
			return 4;

		case 0x04: case 0x05: case 0x06: case 0x07:
			sprintf(buffer, "btst $%04x, d%d", r16u(pc+2), opcode & 3);
			return 4;

		case 0x08: case 0x09: case 0x0a: case 0x0b:
			sprintf(buffer, "add %s, a%d", i16str(r16s(pc+2)), opcode & 3);
			return 4;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			sprintf(buffer, "sub %s, a%d", i16str(r16s(pc+2)), opcode & 3);
			return 4;

		case 0x10:
			sprintf(buffer, "and $%04x, psw", r16u(pc+2));
			return 4;

		case 0x14:
			sprintf(buffer, "or $%04x, psw", r16u(pc+2));
			return 4;

		case 0x18: case 0x19: case 0x1a: case 0x1b:
			sprintf(buffer, "add %s, d%d", i16str(r16u(pc+2)), opcode & 3);
			return 4;

		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			sprintf(buffer, "sub %s, d%d", i16str(r16s(pc+2)), opcode & 3);
			return 4;

		case 0x20: case 0x21: case 0x22: case 0x23:
			sprintf(buffer, "mov a%d, ($%04x)", opcode & 3, r16u(pc+2));
			return 4;

		case 0x30: case 0x31: case 0x32: case 0x33:
			sprintf(buffer, "mov ($%04x), a%d", r16u(pc+2), opcode & 3);
			return 4;

		case 0x40: case 0x41: case 0x42: case 0x43:
			sprintf(buffer, "or $%04x, d%d", r16u(pc+2), opcode & 3);
			return 4;

		case 0x48: case 0x49: case 0x4a: case 0x4b:
			sprintf(buffer, "cmp %s, d%d", i16str(r16u(pc+2)), opcode & 3);
			return 4;

		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			sprintf(buffer, "xor $%04x, d%d", r16u(pc+2), opcode & 3);
			return 4;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			sprintf(buffer, "movbu (%s, a%d), d%d", i16str(r16s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			sprintf(buffer, "movx d%d, (%s, a%d)", opcode & 3, i16str(r16s(pc+2)), (opcode>>2) & 3);
			return 4;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			sprintf(buffer, "movx (%s, a%d), d%d", i16str(r16s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			sprintf(buffer, "mov d%d, (%s, a%d)", opcode & 3, i16str(r16s(pc+2)), (opcode>>2) & 3);
			return 4;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			sprintf(buffer, "movb d%d, (%s, a%d)", opcode & 3, i16str(r16s(pc+2)), (opcode>>2) & 3);
			return 4;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			sprintf(buffer, "mov a%d, (%s, a%d)", opcode & 3, i16str(r16s(pc+2)), (opcode>>2) & 3);
			return 4;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			sprintf(buffer, "mov (%s, a%d), a%d", i16str(r16s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
			sprintf(buffer, "mov (%s, a%d), d%d", i16str(r16s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
			sprintf(buffer, "movb (%s, a%d), d%d", i16str(r16s(pc+2)), (opcode>>2) & 3, opcode & 3);
			return 4;

		default:
			goto illegal2;
		}

	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		sprintf(buffer, "mov %s, d%d", i16str(r16s(pc+1)), opcode & 3);
		return 3;

	case 0xfc:
		sprintf(buffer, "jmp $%x", (pc+3+r16s(pc+1)) & 0xffffff);
		return 3;

	case 0xfd:
		sprintf(buffer, "jsr $%x", (pc+3+r16s(pc+1)) & 0xffffff);
		return 3;

	case 0xfe:
		sprintf(buffer, "rts");
		return 1;

	default:
		goto illegal1;
	};

	illegal1:
		sprintf(buffer, "dc.b $%02x", program_read_byte(pc));
		return 1;

	illegal2:
		sprintf(buffer, "dc.b $%02x $%02x", program_read_byte(pc), program_read_byte(pc+1));
		return 2;

	illegal3:
		sprintf(buffer, "dc.b $%02x $%02x $%02x", program_read_byte(pc), program_read_byte(pc+1), program_read_byte(pc+2));
		return 3;
}

CPU_DISASSEMBLE( mn10200 )
{
	return mn102_disassemble(buffer, pc, oprom);
}
