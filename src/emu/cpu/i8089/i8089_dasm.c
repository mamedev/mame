/***************************************************************************

    Intel 8089 I/O Processor

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    Disassembler

    Note: Incomplete

***************************************************************************/

#include "emu.h"

INT16 displacement(offs_t &pc, int wb, const UINT8 *oprom)
{
	INT16 result = 0;

	switch (wb)
	{
	case 1:
		return oprom[2];
		pc += 1;
		break;
	case 2:
		return oprom[2] | (oprom[3] << 8);
		pc += 2;
		break;
	}

	return result;
}

void offset(char *buffer, offs_t &pc, int aa, int mm, const UINT8 *oprom)
{
	const char *mm_name[]  = { "ga", "gb", "gc", "pp" };

	switch (aa)
	{
	case 0: sprintf(buffer, "[%s]", mm_name[mm]); break;
	case 1: sprintf(buffer, "[%s].%x", mm_name[mm], oprom[2]); pc++; break;
	case 2: sprintf(buffer, "[%s+ix]", mm_name[mm]); break;
	case 3: sprintf(buffer, "[%s+ix+]", mm_name[mm]); break;
	}
}

UINT8 imm8(offs_t &pc, const UINT8 *oprom)
{
	pc += 1;
	return oprom[2];
}

UINT16 imm16(offs_t &pc, const UINT8 *oprom)
{
	pc += 2;
	return oprom[2] | (oprom[3] << 8);
}

#define BRP brp_name[brp]
#define SDISP displacement(pc, wb, oprom)
#define OFFSET(x) offset(x, pc, aa, mm, oprom)
#define IMM8 imm8(pc, oprom)
#define IMM16 imm16(pc, oprom)

CPU_DISASSEMBLE( i8089 )
{
	const char *brp_name[] = { "ga", "gb", "gc", "bc", "tp", "ix", "cc", "mc" };

	UINT32 flags = 0;
	offs_t ppc = pc;

	// temporary storage
	char o[10], o2[10];
	memset(o, 0, sizeof(o));
	memset(o2, 0, sizeof(o2));
	UINT16 off, seg;

	// decode instruction
	int brp = (oprom[0] >> 5) & 0x07;
	int wb  = (oprom[0] >> 3) & 0x03;
	int aa  = (oprom[0] >> 1) & 0x03;
	int w   = (oprom[0] >> 0) & 0x01;
	int opc = (oprom[1] >> 2) & 0x3f;
	int mm  = (oprom[1] >> 0) & 0x03;

	pc += 2;

	switch (opc)
	{
	case 0x00:
		switch (brp)
		{
		case 0: sprintf(buffer, "nop"); break;
		case 1: sprintf(buffer, "???"); break;
		case 2: sprintf(buffer, "sintr"); break;
		case 3: sprintf(buffer, "xfer"); break;
		default: sprintf(buffer, "wid %d, %d", BIT(brp, 1) ? 16 : 8, BIT(brp, 0) ? 16 : 8); break;
		}
		break;
	case 0x02:
		off = IMM16;
		seg = IMM16;
		sprintf(buffer, "lpdi %s, %4x %4x", BRP, off, seg);
		break;
	case 0x08:
		if (w) sprintf(buffer, "addi %s, %04x", BRP, IMM16);
		else   sprintf(buffer, "addbi %s, %02x", BRP, IMM8);
		break;
	case 0x0a:
		if (w) sprintf(buffer, "andi %s, %04x", BRP, IMM16);
		else   sprintf(buffer, "andbi %s, %02x", BRP, IMM8);
		break;
	case 0x0c:
		if (w) sprintf(buffer, "movi %s, %04x", BRP, IMM16);
		else   sprintf(buffer, "movbi %s, %02x", BRP, IMM8);
		break;
	case 0x0f:
		sprintf(buffer, "dec %s", BRP);
		break;
	case 0x12:
		sprintf(buffer, "hlt");
		break;
	case 0x22:
		OFFSET(o);
		sprintf(buffer, "lpd %s, %s", BRP, o);
		break;
	case 0x28:
		OFFSET(o);
		if (w) sprintf(buffer, "add %s, %s", BRP, o);
		else   sprintf(buffer, "addb %s, %s", BRP, o);
		break;
	case 0x2a:
		OFFSET(o);
		if (w) sprintf(buffer, "and %s, %s", BRP, o);
		else   sprintf(buffer, "andb %s, %s", BRP, o);
		break;
	case 0x27:
		OFFSET(o);
		sprintf(buffer, "call %s, %06x", o, ppc + SDISP);
		flags = DASMFLAG_STEP_OVER;
		break;
	case 0x30:
		OFFSET(o);
		if (w) sprintf(buffer, "addi %s, %04x", o, IMM16);
		else   sprintf(buffer, "addbi %s, %02x", o, IMM8);
		break;
	case 0x32:
		OFFSET(o);
		if (w) sprintf(buffer, "andi %s, %04x", o, IMM16);
		else   sprintf(buffer, "andbi %s, %02x", o, IMM8);
		break;
	case 0x34:
		OFFSET(o);
		if (w) sprintf(buffer, "add %s, %s", o, BRP);
		else   sprintf(buffer, "addb %s, %s", o, BRP);
		break;
	case 0x36:
		OFFSET(o);
		if (w) sprintf(buffer, "and %s, %s", o, BRP);
		else   sprintf(buffer, "andb %s, %s", o, BRP);
		break;
	case 0x3b:
		OFFSET(o);
		if (w) sprintf(buffer, "dec %s", o);
		else   sprintf(buffer, "decb %s", o);
		break;
	case 0x3e:
		OFFSET(o);
		sprintf(buffer, "clr %s, %d", o, brp);
		break;
	default:
		sprintf(buffer, "???");
	}

	return (pc - ppc) | flags | DASMFLAG_SUPPORTED;
}
