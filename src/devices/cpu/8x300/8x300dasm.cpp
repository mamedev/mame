// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * 8x300dasm.c
 *  Implementation of the Scientific Micro Systems SMS300 / Signetics 8X300 Microcontroller
 *
 *  Created on: 18/12/2013
 */

#include "emu.h"
#include "8x300dasm.h"

#define SRC    ((opcode & 0x1f00) >> 8)
#define DST    (opcode & 0x001f)
#define ROTLEN ((opcode & 0x00e0) >> 5)
#define IMM8   (opcode & 0x00ff)
#define IMM5   (opcode & 0x001f)

const char *const n8x300_disassembler::reg_names[32] =
{
	"AUX", "R1", "R2", "R3", "R4", "R5", "R6", "IVL", "OVF", "R11",
	"R12", "R13", "R14", "R15", "R16", "IVR",
	"LIV0", "LIV1", "LIV2", "LIV3", "LIV4", "LIV5", "LIV6", "LIV7",
	"RIV0", "RIV1", "RIV2", "RIV3", "RIV4", "RIV5", "RIV6", "RIV7"
};

// determines if right rotate or I/O field length is to be used
bool n8x300_disassembler::is_rot(uint16_t opcode)
{
	if((opcode & 0x1000) || (opcode & 0x0010))
		return false;
	else
		return true;
}

bool n8x300_disassembler::is_src_rot(uint16_t opcode)
{
	if((opcode & 0x1000))
		return false;
	else
		return true;
}

u32 n8x300_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t n8x300_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned startpc = pc;
	offs_t flags = 0;
	uint16_t opcode = opcodes.r16(pc);
	uint8_t inst = opcode >> 13;
	pc+=1;

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
		flags = STEP_COND;
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
		util::stream_format(stream, "JMP  %04XH", (opcode & 0x1fff));
		break;
	}


	return (pc - startpc) | flags | SUPPORTED;
}
