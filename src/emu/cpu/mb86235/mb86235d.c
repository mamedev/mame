// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
#include "emu.h"
#include "debugger.h"
#include "mb86235.h"

static const char *const alu_opcode_string[] =
{
	"FADD",
	"FADDZ",
	"FSUB",
	"FSUBZ",

	"FCMP",
	"FABS",
	"FABC",
	"ALUNOP",

	"FEA",
	"FES",
	"FRCP",
	"FRSQ",

	"FLOG",
	"CIF",
	"CFI",
	"CFIB",

	"ADD",
	"ADDZ",
	"SUB",
	"SUBZ",

	"CMP",
	"ABS",
	"ATR",
	"ATRZ",

	"AND",
	"OR",
	"XOR",
	"NOT",

	"LSR",
	"LSL",
	"ASR",
	"ASL"
};

static const char *const ctrl_opcode_string[] =
{
	"NOP",
	"REP",
	"SETL",
	"CLRF",
	"PUSH",
	"POP",
	"???",
	"???",
	"SETM",
	"SETMCBSA",
	"SETMCBSB",
	"SETMRF",
	"SETMRDY",
	"SETMWAIT",
	"???",
	"???",
	"DBcc", /* TODO */
	"DBNcc", /* TODO */
	"DJMP",
	"DBLP",
	"DBBC",
	"DBBS",
	"???",
	"???",
	"DCcc", /* TODO */
	"DCNcc", /* TODO */
	"DCALL",
	"DRET",
	"???",
	"???",
	"???",
	"???"
};

static unsigned dasm_mb86235(char *buffer, UINT32 opcode, UINT32 opcode2)
{
	char *p = buffer;
	UINT32 grp = ( opcode2 >> 29 ) & 0x7;
	UINT32 aluop = (opcode2 >> (24)) & 0x1f;

	switch(grp)
	{
		case 0: // ALU2

			p += sprintf(p,"%s TRANS2_1",alu_opcode_string[aluop]);
			break;
		case 1: // ALU2
			p += sprintf(p,"%s TRANS1_1",alu_opcode_string[aluop]);
			break;
		case 2: // ALU2 + CTRL
		{
			UINT32 ctrlop = (opcode >> (22)) & 0x1f;
			//UINT32 ef1 = (opcode >> 16) & 0x3f;
			//UINT32 ef2 = (opcode >> 0) & 0xffff;

			p += sprintf(p,"%s %s",alu_opcode_string[aluop],ctrl_opcode_string[ctrlop]);
		}
		break;
		case 4: // ALU1
			p += sprintf(p,"%s TRANS2_2",alu_opcode_string[aluop]);
			break;
		case 5: // ALU1
			p += sprintf(p,"%s TRANS1_2",alu_opcode_string[aluop]);
			break;
		case 6: // ALU1
		{
			UINT32 ctrlop = (opcode >> (22)) & 0x1f;
			//UINT32 ef1 = (opcode >> 16) & 0x3f;
			//UINT32 ef2 = (opcode >> 0) & 0xffff;

			p += sprintf(p,"%s %s",alu_opcode_string[aluop],ctrl_opcode_string[ctrlop]);
			break;
		}
		case 7:
			p += sprintf(p,"TRANS1_3");
			break;
		default:
			p += sprintf(p,"UNK ???");
			break;
	}

	return (2 | DASMFLAG_SUPPORTED);
}

CPU_DISASSEMBLE( mb86235 )
{
	UINT32 op = *(UINT32 *)oprom;
	UINT32 op2 = *(UINT32 *)(oprom + 4);
	op = LITTLE_ENDIANIZE_INT32(op);
	op2 = LITTLE_ENDIANIZE_INT32(op2);

	return dasm_mb86235(buffer, op, op2);
}
