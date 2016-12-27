// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

enum
{
	ADDR_MODE_PC32 = 0x10,
	ADDR_MODE_ABS32 = 0x30,
	ADDR_MODE_REL32 = 0x60,
	ADDR_MODE_PC16 = 0x90,
	ADDR_MODE_REL12 = 0xA0,
	ADDR_MODE_ABS16 = 0xB0,
	ADDR_MODE_PCX = 0xD0,
	ADDR_MODE_RELX = 0xE0,
};

#define R1 ((insn[0] & 0x00F0) >> 4)
#define R2 (insn[0] & 0x000F)

#define I16 ((int16_t)insn[1])
#define I32 (*(int32_t *)&insn[1])
#define IMM_VALUE (insn[0] & 0x0080 ? I16 : I32)
#define IMM_SIZE (insn[0] & 0x0080 ? 2 : 4)

#define ADDR_MODE (insn[0] & 0x00F0)
#define ADDR_R2 ((insn[0] & 0x0050) == 0x0010 ? (insn[0] & 0x000F) : (insn[1] & 0x000F))
#define ADDR_SIZE (ADDR_MODE > ADDR_MODE_REL32 ? 2 : ADDR_MODE == ADDR_MODE_REL32 ? 6 : 4)
#define ADDR_RX ((insn[1] & 0xF0) >> 4)
#define ADDR_I12 (((int16_t)insn[1]) >> 4)

char *address (offs_t pc, uint16_t *insn)
{
	static char buffer[32];

	switch (ADDR_MODE)
	{
	case ADDR_MODE_PC32: sprintf(buffer, "0x%X", pc + I32); break;
	case ADDR_MODE_ABS32: sprintf(buffer, "0x%X", I32); break;
	case ADDR_MODE_REL32: sprintf(buffer, "%d(r%d)", *(int32_t *)&insn[2], R2); break;
	case ADDR_MODE_PC16: sprintf(buffer, "0x%X", pc + I16); break;
	case ADDR_MODE_REL12: sprintf(buffer, "%d(r%d)", ADDR_I12, R2); break;
	case ADDR_MODE_ABS16: sprintf(buffer, "0x%X", I16); break;
	case ADDR_MODE_PCX: sprintf(buffer, "[r%d](pc)", ADDR_RX); break;
	case ADDR_MODE_RELX: sprintf(buffer, "[r%d](r%d)", ADDR_RX, R2); break;
	default: sprintf(buffer, "ERROR"); break;
	}

	return buffer;
}

CPU_DISASSEMBLE(clipper)
{
	uint16_t *insn = (uint16_t *)oprom;
	offs_t bytes;

	// TODO: substitute for 'fp' and 'sp' register names?
	// TODO: branch conditions

	switch (insn[0] >> 8)
	{
	case 0x00: 
		if (oprom[0] == 0)
			util::stream_format(stream, "noop");
		else
			util::stream_format(stream, "noop $%d", oprom[0]);
		bytes = 2; 
		break;

	case 0x10: util::stream_format(stream, "movwp r%d,%s", R2, R1 == 0 ? "psw" : R1 == 1 ? "ssw" : "sswf"); bytes = 2; break;
	case 0x11: util::stream_format(stream, "movpw %s,r%d", R1 == 0 ? "psw" : "ssw", R2); bytes = 2; break; 
	case 0x12: util::stream_format(stream, "calls $%d", insn[0] & 0x7F); bytes = 2; break;
	case 0x13: util::stream_format(stream, "ret r%d", R2); bytes = 2; break;
	case 0x14: util::stream_format(stream, "pushw r%d,r%d", R2, R1); bytes = 2; break;

	case 0x16: util::stream_format(stream, "popw r%d,r%d", R1, R2); bytes = 2; break;

	case 0x20: util::stream_format(stream, "adds f%d,f%d", R1, R2); bytes = 2; break;
	case 0x21: util::stream_format(stream, "subs f%d,f%d", R1, R2); bytes = 2; break;
	case 0x22: util::stream_format(stream, "addd f%d,f%d", R1, R2); bytes = 2; break;
	case 0x23: util::stream_format(stream, "subd f%d,f%d", R1, R2); bytes = 2; break;
	case 0x24: util::stream_format(stream, "movs f%d,f%d", R1, R2); bytes = 2; break;
	case 0x25: util::stream_format(stream, "cmps f%d,f%d", R1, R2); bytes = 2; break;
	case 0x26: util::stream_format(stream, "movd f%d,f%d", R1, R2); bytes = 2; break;
	case 0x27: util::stream_format(stream, "cmpd f%d,f%d", R1, R2); bytes = 2; break;
	case 0x28: util::stream_format(stream, "muls f%d,f%d", R1, R2); bytes = 2; break;
	case 0x29: util::stream_format(stream, "divs f%d,f%d", R1, R2); bytes = 2; break;
	case 0x2A: util::stream_format(stream, "muld f%d,f%d", R1, R2); bytes = 2; break;
	case 0x2B: util::stream_format(stream, "divd f%d,f%d", R1, R2); bytes = 2; break;
	case 0x2C: util::stream_format(stream, "movsw f%d,r%d", R1, R2); bytes = 2; break;
	case 0x2D: util::stream_format(stream, "movws r%d,f%d", R1, R2); bytes = 2; break;
	case 0x2E: util::stream_format(stream, "movdl f%d,r%d:%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x2F: util::stream_format(stream, "movld r%d:r%d,f%d", R1 + 0, R1 + 1, R2); bytes = 2; break;

	case 0x30: util::stream_format(stream, "shaw r%d,r%d", R1, R2); bytes = 2; break;
	case 0x31: util::stream_format(stream, "shal r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x32: util::stream_format(stream, "shlw r%d,r%d", R1, R2); bytes = 2; break;
	case 0x33: util::stream_format(stream, "shll r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x34: util::stream_format(stream, "rotw r%d,r%d", R1, R2); bytes = 2; break;
	case 0x35: util::stream_format(stream, "rotl r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;

	case 0x38: util::stream_format(stream, "shai $%d,r%d", I16, R2); bytes = 4; break;
	case 0x39: util::stream_format(stream, "shali $%d,r%d:r%d", I16, R2 + 0, R2 + 1); bytes = 4; break;
	case 0x3A: util::stream_format(stream, "shli $%d,r%d", I16, R2); bytes = 4; break;
	case 0x3B: util::stream_format(stream, "shlli $%d,r%d:r%d", I16, R2 + 0, R2 + 1); bytes = 4; break;
	case 0x3C: util::stream_format(stream, "roti $%d,r%d", I16, R2); bytes = 4; break;
	case 0x3D: util::stream_format(stream, "rotli $%d,r%d:r%d", I16, R2 + 0, R2 + 1); bytes = 4; break;

	case 0x44: util::stream_format(stream, "call r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x45: util::stream_format(stream, "call r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x48: util::stream_format(stream, "b* (r%d)", R1); bytes = 2; break;
	case 0x49: util::stream_format(stream, "b* %s", address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x4C: util::stream_format(stream, "bf* (r%d)", R1); bytes = 2; break;
	case 0x4D: util::stream_format(stream, "bf* %s", address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x60: util::stream_format(stream, "loadw (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x61: util::stream_format(stream, "loadw %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x62: util::stream_format(stream, "loada (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x63: util::stream_format(stream, "loada %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x64: util::stream_format(stream, "loads (r%d),f%d", R1, R2); bytes = 2; break;
	case 0x65: util::stream_format(stream, "loads %s,f%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x66: util::stream_format(stream, "loadd (r%d),f%d", R1, R2); bytes = 2; break;
	case 0x67: util::stream_format(stream, "loadd %s,f%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x68: util::stream_format(stream, "loadb (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x69: util::stream_format(stream, "loadb %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x6A: util::stream_format(stream, "loadbu (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x6B: util::stream_format(stream, "loadbu %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x6C: util::stream_format(stream, "loadh (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x6D: util::stream_format(stream, "loadh %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x6E: util::stream_format(stream, "loadhu (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x6F: util::stream_format(stream, "loadhu %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;

	case 0x70: util::stream_format(stream, "storw r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x71: util::stream_format(stream, "storw r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;
	case 0x72: util::stream_format(stream, "tsts (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x73: // tsts
	case 0x74: util::stream_format(stream, "stors f%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x75: util::stream_format(stream, "stors f%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;
	case 0x76: util::stream_format(stream, "stord f%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x77: util::stream_format(stream, "stord f%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;
	case 0x78: util::stream_format(stream, "storb r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x79: util::stream_format(stream, "storb r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x7C: util::stream_format(stream, "storh r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x7D: util::stream_format(stream, "storh r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x80: util::stream_format(stream, "addw r%d,r%d", R1, R2); bytes = 2; break;
		
	case 0x82: util::stream_format(stream, "addq $%d,r%d", R1, R2); bytes = 2; break;
	case 0x83: util::stream_format(stream, "addi $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0x84: util::stream_format(stream, "movw r%d,r%d", R1, R2); bytes = 2; break;

	case 0x86: util::stream_format(stream, "loadq $%d,r%d", R1, R2); bytes = 2; break;
	case 0x87: util::stream_format(stream, "loadi $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0x88: util::stream_format(stream, "andw r%d,r%d", R1, R2); bytes = 2; break;

	case 0x8B: util::stream_format(stream, "andi $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0x8C: util::stream_format(stream, "orw r%d,r%d", R1, R2); bytes = 2; break;

	case 0x8F: util::stream_format(stream, "ori $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;

	case 0x90: util::stream_format(stream, "addwc r%d,r%d", R1, R2); bytes = 2; break;
	case 0x91: util::stream_format(stream, "subwc r%d,r%d", R1, R2); bytes = 2; break;

	case 0x93: util::stream_format(stream, "negw r%d,r%d", R1, R2); bytes = 2; break;

	case 0x98: util::stream_format(stream, "mulw r%d,r%d", R1, R2); bytes = 2; break;
	case 0x99: util::stream_format(stream, "mulwx r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x9A: util::stream_format(stream, "mulwu r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9B: util::stream_format(stream, "mulwux r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x9C: util::stream_format(stream, "divw r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9D: util::stream_format(stream, "modw r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9E: util::stream_format(stream, "divwu r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9F: util::stream_format(stream, "modwu r%d,r%d", R1, R2); bytes = 2; break;

	case 0xA0: util::stream_format(stream, "subw r%d,r%d", R1, R2); bytes = 2; break;

	case 0xA2: util::stream_format(stream, "subq $%d,r%d", R1, R2); bytes = 2; break;
	case 0xA3: util::stream_format(stream, "subi $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0xA4: util::stream_format(stream, "cmpw r%d,r%d", R1, R2); bytes = 2; break;

	case 0xA6: util::stream_format(stream, "cmpq $%d,r%d", R1, R2); bytes = 2; break;
	case 0xA7: util::stream_format(stream, "cmpi $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0xA8: util::stream_format(stream, "xorw r%d,r%d", R1, R2); bytes = 2; break;

	case 0xAB: util::stream_format(stream, "xori $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0xAC: util::stream_format(stream, "notw r%d,r%d", R1, R2); bytes = 2; break;

	case 0xAE: util::stream_format(stream, "notq $%d,r%d", R1, R2); bytes = 2; break;

	case 0xB4: // macro
	case 0xB5: // macro
		switch (insn[0] & 0xff)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C:
			util::stream_format(stream, "savew%d", R2);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C:
			util::stream_format(stream, "restw%d", R2);
			break;

		default:
			util::stream_format(stream, "macro 0x%04X %04X", insn[0], insn[1]);
			break;
		}
		bytes = 4;
		break;
	case 0xB6: // macro
	case 0xB7: // macro
		switch (insn[0] & 0xff)
		{
		case 0x00: util::stream_format(stream, "movus r%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x01: util::stream_format(stream, "movsu r%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x04: util::stream_format(stream, "reti r%d", (insn[1] & 0xf0) >> 4); break;

		default:
			util::stream_format(stream, "macro 0x%04X %04X", insn[0], insn[1]);
			break;
		}
		bytes = 4;
		break;

	default:
		util::stream_format(stream, ".word 0x%04X ; invalid", insn[0]);
		bytes = 2;
		break;
	}

	return bytes;
}