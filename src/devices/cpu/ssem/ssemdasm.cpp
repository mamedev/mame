// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Manchester Small-Scale Experimental Machine (SSEM) disassembler

    Written by Ryan Holtz
*/

#include "emu.h"

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

static offs_t ssem_dasm_one(std::ostream &stream, offs_t pc, uint32_t op)
{
	uint8_t instr = (reverse(op) >> 13) & 7;
	uint8_t addr = reverse(op) & 0x1f;

	switch (instr)
	{
		case 0: // JMP S
			util::stream_format(stream, "JMP %d", addr);
			break;
		case 1: // JRP S
			util::stream_format(stream, "JRP %d", addr);
			break;
		case 2: // LDN S
			util::stream_format(stream, "LDN %d", addr);
			break;
		case 3: // STO S
			util::stream_format(stream, "STO %d", addr);
			break;
		case 4: // SUB S
		case 5:
			util::stream_format(stream, "SUB %d", addr);
			break;
		case 6: // CMP
			util::stream_format(stream, "CMP");
			break;
		case 7: // STP
			util::stream_format(stream, "STP");
			break;
		default:
			util::stream_format(stream, "???");
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
	return ssem_dasm_one(stream, pc, op);
}
