// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sanyo LC8670 disassembler

******************************************************************************/

#include "emu.h"
#include "lc8670dsm.h"

u32 lc8670_disassembler::opcode_alignment() const
{
	return 1;
}

const lc8670_disassembler::dasm_entry lc8670_disassembler::s_dasm_table[] =
{
	{ "NOP" ,   OP_NULL,   OP_NULL, 0 },        // 0x0*
	{ "BR"  ,   OP_R8  ,   OP_NULL, 0 },
	{ "LD"  ,   OP_D9  ,   OP_NULL, 0 },
	{ "LD"  ,   OP_RI  ,   OP_NULL, 0 },
	{ "CALL",   OP_A12 ,   OP_NULL, 0 },
	{ "CALLR",  OP_R16 ,   OP_NULL, 0 },        // 0x1*
	{ "BRF" ,   OP_R16 ,   OP_NULL, 0 },
	{ "ST"  ,   OP_D9  ,   OP_NULL, 0 },
	{ "ST"  ,   OP_RI  ,   OP_NULL, 0 },
	{ "CALL",   OP_A12 ,   OP_NULL, 0 },
	{ "CALLF",  OP_A16 ,   OP_NULL, 0 },        // 0x2*
	{ "JMPF",   OP_A16 ,   OP_NULL, 0 },
	{ "MOV" ,   OP_D9  ,   OP_I8  , 1 },
	{ "MOV" ,   OP_RI  ,   OP_I8  , 1 },
	{ "JMP" ,   OP_A12 ,   OP_NULL, 0 },
	{ "MUL" ,   OP_NULL,   OP_NULL, 0 },        // 0x3*
	{ "BE"  ,   OP_I8  ,   OP_R8  , 0 },
	{ "BE"  ,   OP_D9  ,   OP_R8  , 0 },
	{ "BE"  ,   OP_RII8,   OP_R8  , 0 },
	{ "JMP" ,   OP_A12 ,   OP_NULL, 0 },
	{ "DIV" ,   OP_NULL,   OP_NULL, 0 },        // 0x4*
	{ "BNE" ,   OP_I8  ,   OP_R8  , 0 },
	{ "BNE" ,   OP_D9  ,   OP_R8  , 0 },
	{ "BNE" ,   OP_RII8,   OP_R8  , 0 },
	{ "BPC" ,   OP_D9B3,   OP_R8  , 0 },
	{ "LDF" ,   OP_NULL,   OP_NULL, 0 },        // 0x5*
	{ "STF" ,   OP_NULL,   OP_NULL, 0 },
	{ "DBNZ",   OP_D9  ,   OP_R8  , 0 },
	{ "DBNZ",   OP_RI  ,   OP_R8RI, 0 },
	{ "BPC" ,   OP_D9B3,   OP_R8  , 0 },
	{ "PUSH",   OP_D9  ,   OP_NULL, 0 },        // 0x6*
	{ "PUSH",   OP_D9  ,   OP_NULL, 0 },
	{ "INC" ,   OP_D9  ,   OP_NULL, 0 },
	{ "INC" ,   OP_RI  ,   OP_NULL, 0 },
	{ "BP"  ,   OP_D9B3,   OP_R8  , 0 },
	{ "POP" ,   OP_D9  ,   OP_NULL, 0 },        // 0x7*
	{ "POP" ,   OP_D9  ,   OP_NULL, 0 },
	{ "DEC" ,   OP_D9  ,   OP_NULL, 0 },
	{ "DEC" ,   OP_RI  ,   OP_NULL, 0 },
	{ "BP"  ,   OP_D9B3,   OP_R8  , 0 },
	{ "BZ"  ,   OP_R8  ,   OP_NULL, 0 },        // 0x8*
	{ "ADD" ,   OP_I8  ,   OP_NULL, 0 },
	{ "ADD" ,   OP_D9  ,   OP_NULL, 0 },
	{ "ADD" ,   OP_RI  ,   OP_NULL, 0 },
	{ "BN"  ,   OP_D9B3,   OP_R8  , 0 },
	{ "BNZ" ,   OP_R8  ,   OP_NULL, 0 },        // 0x9*
	{ "ADDC",   OP_I8  ,   OP_NULL, 0 },
	{ "ADDC",   OP_D9  ,   OP_NULL, 0 },
	{ "ADDC",   OP_RI  ,   OP_NULL, 0 },
	{ "BN"  ,   OP_D9B3,   OP_R8  , 0 },
	{ "RET" ,   OP_NULL,   OP_NULL, 0 },        // 0xa*
	{ "SUB" ,   OP_I8  ,   OP_NULL, 0 },
	{ "SUB" ,   OP_D9  ,   OP_NULL, 0 },
	{ "SUB" ,   OP_RI  ,   OP_NULL, 0 },
	{ "NOT1",   OP_D9B3,   OP_NULL, 0 },
	{ "RETI",   OP_NULL,   OP_NULL, 0 },        // 0xb*
	{ "SUBC",   OP_I8  ,   OP_NULL, 0 },
	{ "SUBC",   OP_D9  ,   OP_NULL, 0 },
	{ "SUBC",   OP_RI  ,   OP_NULL, 0 },
	{ "NOT1",   OP_D9B3,   OP_NULL, 0 },
	{ "ROR" ,   OP_NULL,   OP_NULL, 0 },        // 0xc*
	{ "LDC" ,   OP_NULL,   OP_NULL, 0 },
	{ "XCH" ,   OP_D9  ,   OP_NULL, 0 },
	{ "XCH" ,   OP_RI  ,   OP_NULL, 0 },
	{ "CLR1",   OP_D9B3,   OP_NULL, 0 },
	{ "RORC",   OP_NULL,   OP_NULL, 0 },        // 0xd*
	{ "OR"  ,   OP_I8  ,   OP_NULL, 0 },
	{ "OR"  ,   OP_D9  ,   OP_NULL, 0 },
	{ "OR"  ,   OP_RI  ,   OP_NULL, 0 },
	{ "CLR1",   OP_D9B3,   OP_NULL, 0 },
	{ "ROL" ,   OP_NULL,   OP_NULL, 0 },        // 0xe*
	{ "AND" ,   OP_I8  ,   OP_NULL, 0 },
	{ "AND" ,   OP_D9  ,   OP_NULL, 0 },
	{ "AND" ,   OP_RI  ,   OP_NULL, 0 },
	{ "SET1",   OP_D9B3,   OP_NULL, 0 },
	{ "ROLC",   OP_NULL,   OP_NULL, 0 },        // 0xf*
	{ "XOR" ,   OP_I8  ,   OP_NULL, 0 },
	{ "XOR" ,   OP_D9  ,   OP_NULL, 0 },
	{ "XOR" ,   OP_RI  ,   OP_NULL, 0 },
	{ "SET1",   OP_D9B3,   OP_NULL, 0 },
};

void lc8670_disassembler::dasm_arg(uint8_t op, char *buffer, offs_t pc, int arg, const data_buffer &opcodes, offs_t &pos)
{
	switch( arg )
	{
		case OP_NULL:
			buffer[0] = '\0';
			break;
		case OP_R8:
			pc++;
			// fall through
		case OP_R8RI:
			buffer += sprintf(buffer, "%04x", (pc + 1 + opcodes.r8(pos) - (opcodes.r8(pos)&0x80 ? 0x100 : 0)) & 0xffff);
			pos++;
			break;
		case OP_R16:
			buffer += sprintf(buffer, "%04x", (pc + 2 + ((opcodes.r8(pos+1)<<8) | opcodes.r8(pos))) & 0xffff);
			pos += 2;
			break;
		case OP_RI:
			buffer += sprintf(buffer, "@%x", op & 0x03);
			break;
		case OP_A12:
			buffer += sprintf(buffer, "%04x", ((pc + 2) & 0xf000) | ((op & 0x10)<<7) | ((op & 0x07)<<8) | opcodes.r8(pos));
			pos++;
			break;
		case OP_A16:
			buffer += sprintf(buffer, "%04x", (opcodes.r8(pos)<<8) | opcodes.r8(pos+1));
			pos += 2;
			break;
		case OP_I8:
			buffer += sprintf(buffer, "#$%02x", opcodes.r8(pos++));
			break;
		case OP_B3:
			buffer += sprintf(buffer, "%x", op & 0x07);
			break;
		case OP_D9:
			buffer += sprintf(buffer, "($%03x)", ((op & 0x01)<<8) | opcodes.r8(pos));
			pos++;
			break;
		case OP_D9B3:
			buffer += sprintf(buffer, "($%03x)", ((op & 0x10)<<4) | opcodes.r8(pos));
			buffer += sprintf(buffer, ",%x", op & 0x07);
			pos++;
			break;
		case OP_RII8:
			buffer += sprintf(buffer, "@%x", op & 0x03);
			buffer += sprintf(buffer, ",#$%02x", opcodes.r8(pos));
			pos++;
			break;
	}
}

//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t lc8670_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t pos = pc;
	char arg1[16], arg2[16];

	uint8_t op = opcodes.r8(pos++);

	int idx;
	switch (op & 0x0f)
	{
		case 0: case 1:
			idx =  op & 0x0f;
			break;
		case 2: case 3:
			idx = 2;
			break;
		case 4: case 5: case 6: case 7:
			idx = 3;
			break;
		default:
			idx = 4;
			break;
	}

	int op_idx = ((op>>4) & 0x0f) * 5 + idx;
	const dasm_entry *inst = &s_dasm_table[op_idx];

	util::stream_format(stream, "%-8s", inst->str);

	dasm_arg(op, inst->inv ? arg2 : arg1, pc+0, inst->arg1, opcodes, pos);
	dasm_arg(op, inst->inv ? arg1 : arg2, pc+1, inst->arg2, opcodes, pos);

	stream << arg1;

	if (inst->arg2 != OP_NULL)
		stream << "," << arg2;

	return pos - pc;
}
