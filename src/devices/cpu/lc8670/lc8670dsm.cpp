// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sanyo LC8670 disassembler

******************************************************************************/

#include "emu.h"
#include "lc8670dsm.h"

#include <locale>
#include <sstream>
#include <utility>


namespace {

enum
{
	OP_NULL,
	OP_R8,
	OP_R8RI,
	OP_R16,
	OP_RI,
	OP_A12,
	OP_A16,
	OP_I8,
	OP_B3,
	OP_D9,
	OP_D9B3,
	OP_RII8
};

// disasm table
struct dasm_entry
{
	const char *str;
	uint8_t     arg1;
	uint8_t     arg2;
	bool        inv;
};

const dasm_entry f_dasm_table[80] =
{
	{ "NOP" ,   OP_NULL,   OP_NULL, false },    // 0x0*
	{ "BR"  ,   OP_R8  ,   OP_NULL, false },
	{ "LD"  ,   OP_D9  ,   OP_NULL, false },
	{ "LD"  ,   OP_RI  ,   OP_NULL, false },
	{ "CALL",   OP_A12 ,   OP_NULL, false },
	{ "CALLR",  OP_R16 ,   OP_NULL, false },    // 0x1*
	{ "BRF" ,   OP_R16 ,   OP_NULL, false },
	{ "ST"  ,   OP_D9  ,   OP_NULL, false },
	{ "ST"  ,   OP_RI  ,   OP_NULL, false },
	{ "CALL",   OP_A12 ,   OP_NULL, false },
	{ "CALLF",  OP_A16 ,   OP_NULL, false },    // 0x2*
	{ "JMPF",   OP_A16 ,   OP_NULL, false },
	{ "MOV" ,   OP_D9  ,   OP_I8  , true  },
	{ "MOV" ,   OP_RI  ,   OP_I8  , true  },
	{ "JMP" ,   OP_A12 ,   OP_NULL, false },
	{ "MUL" ,   OP_NULL,   OP_NULL, false },    // 0x3*
	{ "BE"  ,   OP_I8  ,   OP_R8  , false },
	{ "BE"  ,   OP_D9  ,   OP_R8  , false },
	{ "BE"  ,   OP_RII8,   OP_R8  , false },
	{ "JMP" ,   OP_A12 ,   OP_NULL, false },
	{ "DIV" ,   OP_NULL,   OP_NULL, false },    // 0x4*
	{ "BNE" ,   OP_I8  ,   OP_R8  , false },
	{ "BNE" ,   OP_D9  ,   OP_R8  , false },
	{ "BNE" ,   OP_RII8,   OP_R8  , false },
	{ "BPC" ,   OP_D9B3,   OP_R8  , false },
	{ "LDF" ,   OP_NULL,   OP_NULL, false },    // 0x5*
	{ "STF" ,   OP_NULL,   OP_NULL, false },
	{ "DBNZ",   OP_D9  ,   OP_R8  , false },
	{ "DBNZ",   OP_RI  ,   OP_R8RI, false },
	{ "BPC" ,   OP_D9B3,   OP_R8  , false },
	{ "PUSH",   OP_D9  ,   OP_NULL, false },    // 0x6*
	{ "PUSH",   OP_D9  ,   OP_NULL, false },
	{ "INC" ,   OP_D9  ,   OP_NULL, false },
	{ "INC" ,   OP_RI  ,   OP_NULL, false },
	{ "BP"  ,   OP_D9B3,   OP_R8  , false },
	{ "POP" ,   OP_D9  ,   OP_NULL, false },    // 0x7*
	{ "POP" ,   OP_D9  ,   OP_NULL, false },
	{ "DEC" ,   OP_D9  ,   OP_NULL, false },
	{ "DEC" ,   OP_RI  ,   OP_NULL, false },
	{ "BP"  ,   OP_D9B3,   OP_R8  , false },
	{ "BZ"  ,   OP_R8  ,   OP_NULL, false },    // 0x8*
	{ "ADD" ,   OP_I8  ,   OP_NULL, false },
	{ "ADD" ,   OP_D9  ,   OP_NULL, false },
	{ "ADD" ,   OP_RI  ,   OP_NULL, false },
	{ "BN"  ,   OP_D9B3,   OP_R8  , false },
	{ "BNZ" ,   OP_R8  ,   OP_NULL, false },    // 0x9*
	{ "ADDC",   OP_I8  ,   OP_NULL, false },
	{ "ADDC",   OP_D9  ,   OP_NULL, false },
	{ "ADDC",   OP_RI  ,   OP_NULL, false },
	{ "BN"  ,   OP_D9B3,   OP_R8  , false },
	{ "RET" ,   OP_NULL,   OP_NULL, false },    // 0xa*
	{ "SUB" ,   OP_I8  ,   OP_NULL, false },
	{ "SUB" ,   OP_D9  ,   OP_NULL, false },
	{ "SUB" ,   OP_RI  ,   OP_NULL, false },
	{ "NOT1",   OP_D9B3,   OP_NULL, false },
	{ "RETI",   OP_NULL,   OP_NULL, false },    // 0xb*
	{ "SUBC",   OP_I8  ,   OP_NULL, false },
	{ "SUBC",   OP_D9  ,   OP_NULL, false },
	{ "SUBC",   OP_RI  ,   OP_NULL, false },
	{ "NOT1",   OP_D9B3,   OP_NULL, false },
	{ "ROR" ,   OP_NULL,   OP_NULL, false },    // 0xc*
	{ "LDC" ,   OP_NULL,   OP_NULL, false },
	{ "XCH" ,   OP_D9  ,   OP_NULL, false },
	{ "XCH" ,   OP_RI  ,   OP_NULL, false },
	{ "CLR1",   OP_D9B3,   OP_NULL, false },
	{ "RORC",   OP_NULL,   OP_NULL, false },    // 0xd*
	{ "OR"  ,   OP_I8  ,   OP_NULL, false },
	{ "OR"  ,   OP_D9  ,   OP_NULL, false },
	{ "OR"  ,   OP_RI  ,   OP_NULL, false },
	{ "CLR1",   OP_D9B3,   OP_NULL, false },
	{ "ROL" ,   OP_NULL,   OP_NULL, false },    // 0xe*
	{ "AND" ,   OP_I8  ,   OP_NULL, false },
	{ "AND" ,   OP_D9  ,   OP_NULL, false },
	{ "AND" ,   OP_RI  ,   OP_NULL, false },
	{ "SET1",   OP_D9B3,   OP_NULL, false },
	{ "ROLC",   OP_NULL,   OP_NULL, false },    // 0xf*
	{ "XOR" ,   OP_I8  ,   OP_NULL, false },
	{ "XOR" ,   OP_D9  ,   OP_NULL, false },
	{ "XOR" ,   OP_RI  ,   OP_NULL, false },
	{ "SET1",   OP_D9B3,   OP_NULL, false },
};

} // anonymous namespace


u32 lc8670_disassembler::opcode_alignment() const
{
	return 1;
}

void lc8670_disassembler::dasm_arg(uint8_t op, std::ostream &buffer, offs_t pc, int arg, const data_buffer &opcodes, offs_t &pos)
{
	switch( arg )
	{
		case OP_NULL:
			break;
		case OP_R8:
			pc++;
			[[fallthrough]];
		case OP_R8RI:
			util::stream_format(buffer, "%04x", (pc + 1 + opcodes.r8(pos) - (opcodes.r8(pos)&0x80 ? 0x100 : 0)) & 0xffff);
			pos++;
			break;
		case OP_R16:
			util::stream_format(buffer, "%04x", (pc + 2 + ((opcodes.r8(pos+1)<<8) | opcodes.r8(pos))) & 0xffff);
			pos += 2;
			break;
		case OP_RI:
			util::stream_format(buffer, "@%x", op & 0x03);
			break;
		case OP_A12:
			util::stream_format(buffer, "%04x", ((pc + 2) & 0xf000) | ((op & 0x10)<<7) | ((op & 0x07)<<8) | opcodes.r8(pos));
			pos++;
			break;
		case OP_A16:
			util::stream_format(buffer, "%04x", (opcodes.r8(pos)<<8) | opcodes.r8(pos+1));
			pos += 2;
			break;
		case OP_I8:
			util::stream_format(buffer, "#$%02x", opcodes.r8(pos++));
			break;
		case OP_B3:
			util::stream_format(buffer, "%x", op & 0x07);
			break;
		case OP_D9:
			util::stream_format(buffer, "($%03x)", ((op & 0x01)<<8) | opcodes.r8(pos));
			pos++;
			break;
		case OP_D9B3:
			util::stream_format(buffer, "($%03x)", ((op & 0x10)<<4) | opcodes.r8(pos));
			util::stream_format(buffer, ",%x", op & 0x07);
			pos++;
			break;
		case OP_RII8:
			util::stream_format(buffer, "@%x", op & 0x03);
			util::stream_format(buffer, ",#$%02x", opcodes.r8(pos));
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
	const dasm_entry &inst = f_dasm_table[op_idx];

	util::stream_format(stream, "%-8s", inst.str);

	if (!inst.inv)
	{
		dasm_arg(op, stream, pc + 0, inst.arg1, opcodes, pos);
		if (inst.arg2 != OP_NULL)
		{
			stream << ',';
			dasm_arg(op, stream, pc + 1, inst.arg2, opcodes, pos);
		}
	}
	else
	{
		std::ostringstream arg2;
		arg2.imbue(std::locale::classic());
		dasm_arg(op, arg2, pc + 0, inst.arg1, opcodes, pos);
		dasm_arg(op, stream, pc + 1, inst.arg2, opcodes, pos);
		stream << ',' << std::move(arg2).str();
	}

	return pos - pc;
}
