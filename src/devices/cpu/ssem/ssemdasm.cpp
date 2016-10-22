// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Manchester Small-Scale Experimental Machine (SSEM) disassembler

    Written by Ryan Holtz
*/

#include "emu.h"

static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static inline uint32_t reverse(uint32_t v)
{
	// Taken from http://www-graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ...
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = ( v >> 16             ) | ( v               << 16);

	return v;
}

offs_t ssem_dasm_one(char *buffer, offs_t pc, uint32_t op)
{
	uint8_t instr = (reverse(op) >> 13) & 7;
	uint8_t addr = reverse(op) & 0x1f;

	output = buffer;

	switch (instr)
	{
		case 0: // JMP S
			print("JMP %d", addr);
			break;
		case 1: // JRP S
			print("JRP %d", addr);
			break;
		case 2: // LDN S
			print("LDN %d", addr);
			break;
		case 3: // STO S
			print("STO %d", addr);
			break;
		case 4: // SUB S
		case 5:
			print("SUB %d", addr);
			break;
		case 6: // CMP
			print("CMP");
			break;
		case 7: // STP
			print("STP");
			break;
		default:
			print("???");
			break;
	}

	return 4 | DASMFLAG_SUPPORTED;
}

/*****************************************************************************/

CPU_DISASSEMBLE( ssem )
{
	uint32_t op = (*(uint8_t *)(opram + 0) << 24) |
				(*(uint8_t *)(opram + 1) << 16) |
				(*(uint8_t *)(opram + 2) <<  8) |
				(*(uint8_t *)(opram + 3) <<  0);
	return ssem_dasm_one(buffer, pc, op);
}
