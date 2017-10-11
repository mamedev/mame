// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * 8x300dasm.c
 *  Implementation of the Scientific Micro Systems SMS300 / Signetics 8X300 Microcontroller
 *
 *  Created on: 18/12/2013
 */

#include "emu.h"
#include "8x300.h"

#define SRC    ((opcode & 0x1f00) >> 8)
#define DST    (opcode & 0x001f)
#define ROTLEN ((opcode & 0x00e0) >> 5)
#define IMM8   (opcode & 0x00ff)
#define IMM5   (opcode & 0x001f)

static const char *reg_names[32] =
{
	"AUX", "R1", "R2", "R3", "R4", "R5", "R6", "IVL", "OVF", "R11",
	"Unused12", "Unused13", "Unused14", "Unused15", "Unused16", "IVR",
	"LIV0", "LIV1", "LIV2", "LIV3", "LIV4", "LIV5", "LIV6", "LIV7",
	"RIV0", "RIV1", "RIV2", "RIV3", "RIV4", "RIV5", "RIV6", "RIV7"
};

// determines if right rotate or I/O field length is to be used
static inline bool is_rot(uint16_t opcode)
{
	if((opcode & 0x1000) || (opcode & 0x0010))
		return false;
	else
		return true;
}

static inline bool is_src_rot(uint16_t opcode)
{
	if((opcode & 0x1000))
		return false;
	else
		return true;
}

CPU_DISASSEMBLE(n8x300)
{
	unsigned startpc = pc;
	uint16_t opcode = (oprom[pc - startpc] << 8) | oprom[pc+1 - startpc];
	uint8_t inst = opcode >> 13;
	pc+=2;

	// determine instruction
	switch (inst)
	{
	case 0x00:
		stream << "MOVE " << reg_names[SRC];
		if(is_rot(opcode))
			util::stream_format(stream, "(%i),", ROTLEN);
		else
			util::stream_format(stream, ",%i,", ROTLEN);
		stream << reg_names[DST];
		break;
	case 0x01:
		stream << "ADD  " << reg_names[SRC];
		if(is_rot(opcode))
			util::stream_format(stream, "(%i),", ROTLEN);
		else
			util::stream_format(stream, ",%i,", ROTLEN);
		stream << reg_names[DST];
		break;
	case 0x02:
		stream << "AND  " << reg_names[SRC];
		if(is_rot(opcode))
			util::stream_format(stream, "(%i),", ROTLEN);
		else
			util::stream_format(stream, ",%i,", ROTLEN);
		stream << reg_names[DST];
		break;
	case 0x03:
		stream << "XOR  " << reg_names[SRC];
		if(is_rot(opcode))
			util::stream_format(stream, "(%i),", ROTLEN);
		else
			util::stream_format(stream, ",%i,", ROTLEN);
		stream << reg_names[DST];
		break;
	case 0x04:
		stream << "XEC  " << reg_names[SRC];
		if(is_src_rot(opcode))
		{
			util::stream_format(stream, ",%02XH", IMM8);
		}
		else
		{
			util::stream_format(stream, ",%i", ROTLEN);
			util::stream_format(stream, ",%02XH", IMM5);
		}
		break;
	case 0x05:
		stream << "NZT  " << reg_names[SRC];
		if(is_src_rot(opcode))
		{
			util::stream_format(stream, ",%02XH", IMM8);
		}
		else
		{
			util::stream_format(stream, ",%i", ROTLEN);
			util::stream_format(stream, ",%02XH", IMM5);
		}
		break;
	case 0x06:
		stream << "XMIT ";
		if(is_src_rot(opcode))
		{
			util::stream_format(stream, "%02XH,", IMM8);
			stream << reg_names[SRC];
		}
		else
		{
			util::stream_format(stream, "%02XH,", IMM5);
			stream << reg_names[SRC];
			util::stream_format(stream, ",%i", ROTLEN);
		}
		break;
	case 0x07:
		util::stream_format(stream, "JMP  %04XH", (opcode & 0x1fff) << 1);
		break;
	}


	return (pc - startpc);
}
