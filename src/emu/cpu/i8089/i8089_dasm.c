/***************************************************************************

    Intel 8089 I/O Processor

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    Disassembler

    Note: Incomplete

***************************************************************************/

#include "emu.h"

INT16 displacement(offs_t &pc, int wb, const UINT8 *oprom, bool aa1)
{
	INT16 result = 0;

	switch (wb)
	{
	case 1:
		result = oprom[2 + aa1];
		pc += 1;
		break;
	case 2:
		result = oprom[2 + aa1] | (oprom[3 + aa1] << 8);
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

UINT8 imm8(offs_t &pc, const UINT8 *oprom, bool aa1)
{
	pc += 1;
	return oprom[2 + aa1];
}

UINT16 imm16(offs_t &pc, const UINT8 *oprom, bool aa1)
{
	pc += 2;
	return oprom[2 + aa1] | (oprom[3 + aa1] << 8);
}

#define BRP brp_name[brp]
#define SDISP displacement(pc, wb, oprom, (aa == 1))
#define OFFSET(x) offset(x, pc, aa, mm, oprom)
#define IMM8 imm8(pc, oprom, (aa == 1))
#define IMM16 imm16(pc, oprom, (aa == 1))

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
	case 0x09:
		if (w) sprintf(buffer, "ori %s, %04x", BRP, IMM16);
		else   sprintf(buffer, "orbi %s, %02x", BRP, IMM8);
		break;
	case 0x0a:
		if (w) sprintf(buffer, "andi %s, %04x", BRP, IMM16);
		else   sprintf(buffer, "andbi %s, %02x", BRP, IMM8);
		break;
	case 0x0b:
		sprintf(buffer, "not %s", BRP);
		break;
	case 0x0c:
		if (w) sprintf(buffer, "movi %s, %04x", BRP, IMM16);
		else   sprintf(buffer, "movbi %s, %02x", BRP, IMM8);
		break;
	case 0x0e:
		sprintf(buffer, "inc %s", BRP);
		break;
	case 0x0f:
		sprintf(buffer, "dec %s", BRP);
		break;
	case 0x10:
		sprintf(buffer, "jnz %s, %06x", BRP, pc + SDISP);
		break;
	case 0x11:
		sprintf(buffer, "jz %s, %06x", BRP, pc + SDISP);
		break;
	case 0x12:
		sprintf(buffer, "hlt");
		break;
	case 0x13:
		OFFSET(o);
		if (w) sprintf(buffer, "movi %s, %04x", o, IMM16);
		else   sprintf(buffer, "movbi %s, %02x", o, IMM8);
		break;
	case 0x20:
		OFFSET(o);
		if (w) sprintf(buffer, "mov %s, %s", BRP, o);
		else   sprintf(buffer, "movb %s, %s", BRP, o);
		break;
	case 0x21:
		OFFSET(o);
		if (w) sprintf(buffer, "mov %s, %s", o, BRP);
		else   sprintf(buffer, "movb %s, %s", o, BRP);
		break;
	case 0x22:
		OFFSET(o);
		sprintf(buffer, "lpd %s, %s", BRP, o);
		break;
	case 0x23:
		OFFSET(o);
		sprintf(buffer, "movp %s, %s", BRP, o);
		break;
	case 0x24:
	{
		char buf[20];
		OFFSET(o);
		pc += cpu_disassemble_i8089(device, buf, pc, oprom + (pc - ppc), opram, options) & 0xffff;
		if(w) sprintf(buffer, "mov %s, %s", buf, o);
		else sprintf(buffer, "movb %s, %s", buf, o);
		break;
	}
	case 0x26:
		OFFSET(o);
		sprintf(buffer, "movp %s, %s", o, BRP);
		break;
	case 0x27:
		OFFSET(o);
		sprintf(buffer, "call %s, %06x", o, pc + SDISP);
		flags = DASMFLAG_STEP_OVER;
		break;
	case 0x28:
		OFFSET(o);
		if (w) sprintf(buffer, "add %s, %s", BRP, o);
		else   sprintf(buffer, "addb %s, %s", BRP, o);
		break;
	case 0x29:
		OFFSET(o);
		if (w) sprintf(buffer, "or %s, %s", BRP, o);
		else   sprintf(buffer, "orb %s, %s", BRP, o);
		break;
	case 0x2a:
		OFFSET(o);
		if (w) sprintf(buffer, "and %s, %s", BRP, o);
		else   sprintf(buffer, "andb %s, %s", BRP, o);
		break;
	case 0x2b:
		OFFSET(o);
		if(w) sprintf(buffer, "not %s, %s", BRP, o);
		else sprintf(buffer, "notb %s, %s", BRP, o);
		break;
	case 0x2c:
		OFFSET(o);
		sprintf(buffer, "jmce %s, %06x", o, pc + SDISP);
		break;
	case 0x2d:
		OFFSET(o);
		sprintf(buffer, "jmcne %s, %06x", o, pc + SDISP);
		break;
	case 0x2e:
		OFFSET(o);
		sprintf(buffer, "jnbt %s, %d, %06x", o, brp, pc + SDISP);
		break;
	case 0x2f:
		OFFSET(o);
		sprintf(buffer, "jbt %s, %d, %06x", o, brp, pc + SDISP);
		break;
	case 0x30:
		OFFSET(o);
		if (w) sprintf(buffer, "addi %s, %04x", o, IMM16);
		else   sprintf(buffer, "addbi %s, %02x", o, IMM8);
		break;
	case 0x31:
		OFFSET(o);
		if (w) sprintf(buffer, "ori %s, %04x", o, IMM16);
		else   sprintf(buffer, "ori %s, %02x", o, IMM8);
		break;
	case 0x32:
		OFFSET(o);
		if (w) sprintf(buffer, "andi %s, %04x", o, IMM16);
		else   sprintf(buffer, "andbi %s, %02x", o, IMM8);
		break;
	case 0x33:
		OFFSET(o);
		sprintf(buffer, "%s", o);
		break;
	case 0x34:
		OFFSET(o);
		if (w) sprintf(buffer, "add %s, %s", o, BRP);
		else   sprintf(buffer, "addb %s, %s", o, BRP);
		break;
	case 0x35:
		OFFSET(o);
		if (w) sprintf(buffer, "or %s, %s", o, BRP);
		else   sprintf(buffer, "orb %s, %s", o, BRP);
		break;
	case 0x36:
		OFFSET(o);
		if (w) sprintf(buffer, "and %s, %s", o, BRP);
		else   sprintf(buffer, "andb %s, %s", o, BRP);
		break;
	case 0x37:
		OFFSET(o);
		if(w) sprintf(buffer, "not %s", o);
		else sprintf(buffer, "notb %s", o);
		break;
	case 0x38:
		OFFSET(o);
		if(w) sprintf(buffer, "jnz %s, %06x", o, pc + SDISP);
		else sprintf(buffer, "jnzb %s, %06x", o, pc + SDISP);
		break;
	case 0x39:
		OFFSET(o);
		if(w) sprintf(buffer, "jz %s, %06x", o, pc + SDISP);
		else sprintf(buffer, "jzb %s, %06x", o, pc + SDISP);
		break;
	case 0x3a:
		OFFSET(o);
		if (w) sprintf(buffer, "inc %s", o);
		else   sprintf(buffer, "incb %s", o);
		break;
	case 0x3b:
		OFFSET(o);
		if (w) sprintf(buffer, "dec %s", o);
		else   sprintf(buffer, "decb %s", o);
		break;
	case 0x3d:
		OFFSET(o);
		sprintf(buffer, "set %s, %d", o, brp);
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
