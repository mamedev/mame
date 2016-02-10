// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

#include "arcompactdasm_dispatch.h"
#include "arcompactdasm_ops.h"


/*****************************************************************************/



/*****************************************************************************/


#define ARCOMPACT_OPERATION ((op & 0xf800) >> 11)

extern char *output;

CPU_DISASSEMBLE(arcompact)
{
	int size;

	UINT32 op = oprom[0] | (oprom[1] << 8);
	output = buffer;

	UINT8 instruction = ARCOMPACT_OPERATION;

	if (instruction < 0x0c)
	{
		size = 4;
		op <<= 16;
		op |= oprom[2] | (oprom[3] << 8);

		op &= ~0xf8000000;

		switch (instruction) // 32-bit instructions (with optional extra dword for immediate data)
		{
			case 0x00: size = arcompact_handle00_dasm(DASM_PARAMS); break; // Bcc
			case 0x01: size = arcompact_handle01_dasm(DASM_PARAMS); break; // BLcc/BRcc
			case 0x02: size = arcompact_handle02_dasm(DASM_PARAMS); break; // LD r+o
			case 0x03: size = arcompact_handle03_dasm(DASM_PARAMS); break; // ST r+o
			case 0x04: size = arcompact_handle04_dasm(DASM_PARAMS); break; // op a,b,c (basecase)
			case 0x05: size = arcompact_handle05_dasm(DASM_PARAMS); break; // op a,b,c (05 ARC ext)
			case 0x06: size = arcompact_handle06_dasm(DASM_PARAMS); break; // op a,b,c (06 ARC ext)
			case 0x07: size = arcompact_handle07_dasm(DASM_PARAMS); break; // op a,b,c (07 User ext)
			case 0x08: size = arcompact_handle08_dasm(DASM_PARAMS); break; // op a,b,c (08 User ext)
			case 0x09: size = arcompact_handle09_dasm(DASM_PARAMS); break; // op a,b,c (09 Market ext)
			case 0x0a: size = arcompact_handle0a_dasm(DASM_PARAMS); break; // op a,b,c (0a Market ext)
			case 0x0b: size = arcompact_handle0b_dasm(DASM_PARAMS); break; // op a,b,c (0b Market ext)
		}
	}
	else
	{
		size = 2;
		op &= ~0xf800;


		switch (instruction) // 16-bit instructions
		{
			case 0x0c: size = arcompact_handle0c_dasm(DASM_PARAMS); break; // Load/Add reg-reg
			case 0x0d: size = arcompact_handle0d_dasm(DASM_PARAMS); break; // Add/Sub/Shft imm
			case 0x0e: size = arcompact_handle0e_dasm(DASM_PARAMS); break; // Mov/Cmp/Add
			case 0x0f: size = arcompact_handle0f_dasm(DASM_PARAMS); break; // op_S b,b,c (single 16-bit ops)
			case 0x10: size = arcompact_handle10_dasm(DASM_PARAMS); break; // LD_S
			case 0x11: size = arcompact_handle11_dasm(DASM_PARAMS); break; // LDB_S
			case 0x12: size = arcompact_handle12_dasm(DASM_PARAMS); break; // LDW_S
			case 0x13: size = arcompact_handle13_dasm(DASM_PARAMS); break; // LSW_S.X
			case 0x14: size = arcompact_handle14_dasm(DASM_PARAMS); break; // ST_S
			case 0x15: size = arcompact_handle15_dasm(DASM_PARAMS); break; // STB_S
			case 0x16: size = arcompact_handle16_dasm(DASM_PARAMS); break; // STW_S
			case 0x17: size = arcompact_handle17_dasm(DASM_PARAMS); break; // Shift/Sub/Bit
			case 0x18: size = arcompact_handle18_dasm(DASM_PARAMS); break; // Stack Instr
			case 0x19: size = arcompact_handle19_dasm(DASM_PARAMS); break; // GP Instr
			case 0x1a: size = arcompact_handle1a_dasm(DASM_PARAMS); break; // PCL Instr
			case 0x1b: size = arcompact_handle1b_dasm(DASM_PARAMS); break; // MOV_S
			case 0x1c: size = arcompact_handle1c_dasm(DASM_PARAMS); break; // ADD_S/CMP_S
			case 0x1d: size = arcompact_handle1d_dasm(DASM_PARAMS); break; // BRcc_S
			case 0x1e: size = arcompact_handle1e_dasm(DASM_PARAMS); break; // Bcc_S
			case 0x1f: size = arcompact_handle1f_dasm(DASM_PARAMS); break; // BL_S
		}
	}

	return size | DASMFLAG_SUPPORTED;
}
