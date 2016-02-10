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
static inline bool is_rot(UINT16 opcode)
{
	if((opcode & 0x1000) || (opcode & 0x0010))
		return false;
	else
		return true;
}

static inline bool is_src_rot(UINT16 opcode)
{
	if((opcode & 0x1000))
		return false;
	else
		return true;
}

CPU_DISASSEMBLE( n8x300 )
{
	char tmp[16];
	unsigned startpc = pc;
	UINT16 opcode = (oprom[pc - startpc] << 8) | oprom[pc+1 - startpc];
	UINT8 inst = opcode >> 13;
	pc+=2;

	// determine instruction
	switch (inst)
	{
	case 0x00:
		sprintf(buffer,"MOVE ");
		strcat(buffer,reg_names[SRC]);
		if(is_rot(opcode))
			sprintf(tmp,"(%i),",ROTLEN);
		else
			sprintf(tmp,",%i,",ROTLEN);
		strcat(buffer,tmp);
		strcat(buffer,reg_names[DST]);
		break;
	case 0x01:
		sprintf(buffer,"ADD  ");
		strcat(buffer,reg_names[SRC]);
		if(is_rot(opcode))
			sprintf(tmp,"(%i),",ROTLEN);
		else
			sprintf(tmp,",%i,",ROTLEN);
		strcat(buffer,tmp);
		strcat(buffer,reg_names[DST]);
		break;
	case 0x02:
		sprintf(buffer,"AND  ");
		strcat(buffer,reg_names[SRC]);
		if(is_rot(opcode))
			sprintf(tmp,"(%i),",ROTLEN);
		else
			sprintf(tmp,",%i,",ROTLEN);
		strcat(buffer,tmp);
		strcat(buffer,reg_names[DST]);
		break;
	case 0x03:
		sprintf(buffer,"XOR  ");
		strcat(buffer,reg_names[SRC]);
		if(is_rot(opcode))
			sprintf(tmp,"(%i),",ROTLEN);
		else
			sprintf(tmp,",%i,",ROTLEN);
		strcat(buffer,tmp);
		strcat(buffer,reg_names[DST]);
		break;
	case 0x04:
		sprintf(buffer,"XEC  ");
		strcat(buffer,reg_names[SRC]);
		if(is_src_rot(opcode))
		{
			sprintf(tmp,",%02XH",IMM8);
			strcat(buffer,tmp);
		}
		else
		{
			sprintf(tmp,",%i",ROTLEN);
			strcat(buffer,tmp);
			sprintf(tmp,",%02XH",IMM5);
			strcat(buffer,tmp);
		}
		break;
	case 0x05:
		sprintf(buffer,"NZT  ");
		strcat(buffer,reg_names[SRC]);
		if(is_src_rot(opcode))
		{
			sprintf(tmp,",%02XH",IMM8);
			strcat(buffer,tmp);
		}
		else
		{
			sprintf(tmp,",%i",ROTLEN);
			strcat(buffer,tmp);
			sprintf(tmp,",%02XH",IMM5);
			strcat(buffer,tmp);
		}
		break;
	case 0x06:
		sprintf(buffer,"XMIT ");
		if(is_src_rot(opcode))
		{
			sprintf(tmp,"%02XH,",IMM8);
			strcat(buffer,tmp);
			strcat(buffer,reg_names[SRC]);
		}
		else
		{
			sprintf(tmp,"%02XH,",IMM5);
			strcat(buffer,tmp);
			strcat(buffer,reg_names[SRC]);
			sprintf(tmp,",%i",ROTLEN);
			strcat(buffer,tmp);
		}
		break;
	case 0x07:
		sprintf(buffer,"JMP  %04XH",opcode & 0x1fff);
		break;
	}


	return (pc - startpc);
}
