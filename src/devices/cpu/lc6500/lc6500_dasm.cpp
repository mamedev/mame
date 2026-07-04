// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC6500 series disassembler

***************************************************************************/

#include "emu.h"
#include "lc6500_dasm.h"


#define P std::ostream &stream, u32 opcode, const data_buffer &opcodes, u32 pc

struct lc6500_disassembler::instruction
{
	using handler = uint32_t (*)(P);

	uint8_t value;
	uint8_t mask;
	handler cb;
};

const lc6500_disassembler::instruction lc6500_disassembler::s_instructions[]
{
	{ 0x00, 0xff, [](P) -> uint32_t { util::stream_format(stream, "nop"); return 1; } },
	{ 0x01, 0xff, [](P) -> uint32_t { util::stream_format(stream, "ral"); return 1; } },
	{ 0x02, 0xff, [](P) -> uint32_t { util::stream_format(stream, "s");   return 1; } },
	{ 0x03, 0xff, [](P) -> uint32_t { util::stream_format(stream, "tae"); return 1; } },
	{ 0x0c, 0xff, [](P) -> uint32_t { util::stream_format(stream, "ip");  return 1; } },
	{ 0x0d, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xae"); return 1; } },
	{ 0x0e, 0xff, [](P) -> uint32_t { util::stream_format(stream, "inc"); return 1; } },
	{ 0x0f, 0xff, [](P) -> uint32_t { util::stream_format(stream, "dec"); return 1; } },
	{ 0x20, 0xff, [](P) -> uint32_t { util::stream_format(stream, "adc"); return 1; } },
	{ 0x21, 0xff, [](P) -> uint32_t { util::stream_format(stream, "l");   return 1; } },
	{ 0x22, 0xff, [](P) -> uint32_t { util::stream_format(stream, "rti"); return 1 | STEP_OUT; } },
	{ 0x23, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xah"); return 1; } },

	// special case, need to decide on op1
	{ 0x2c, 0xff, [](P) -> uint32_t
		{
			uint8_t op1 = opcodes.r8(pc + 1);
			switch (op1 & 0xf0)
			{
				case 0x40: util::stream_format(stream, "ci #$%x",   op1 & 0x0f); break;
				case 0x50: util::stream_format(stream, "cli #$%x",  op1 & 0x0f); break;
				case 0x80: util::stream_format(stream, "sctl %x", op1 & 0x0f); break;
				case 0x90: util::stream_format(stream, "rctl %x", op1 & 0x0f); break;
				default:   util::stream_format(stream, "? $2c $%02x", op1); break;
			}
			return 2;
		}
	},

	{ 0x2e, 0xff, [](P) -> uint32_t { util::stream_format(stream, "inm"); return 1; } },
	{ 0x2f, 0xff, [](P) -> uint32_t { util::stream_format(stream, "dem"); return 1; } },

	{ 0x3c, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bntm $%03x", (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x3d, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bni $%03x",  (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x3e, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bnz $%03x",  (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x3f, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bnc $%03x",  (pc & 0xf00) | p); return 2 | STEP_COND; } },

	{ 0x60, 0xff, [](P) -> uint32_t { util::stream_format(stream, "ad");   return 1; } },
	{ 0x61, 0xff, [](P) -> uint32_t { util::stream_format(stream, "op");   return 1; } },
	{ 0x62, 0xff, [](P) -> uint32_t { util::stream_format(stream, "rt");   return 1 | STEP_OUT; } },
	{ 0x63, 0xff, [](P) -> uint32_t { util::stream_format(stream, "rtbl"); return 1; } },

	{ 0x7c, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "btm $%03x", (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x7d, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bi $%03x",  (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x7e, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bz $%03x",  (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x7f, 0xff, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bc $%03x",  (pc & 0xf00) | p); return 2 | STEP_COND; } },

	{ 0xa0, 0xff, [](P) -> uint32_t { util::stream_format(stream, "x");    return 1; } },
	{ 0xc0, 0xff, [](P) -> uint32_t { util::stream_format(stream, "cla");  return 1; } },
	{ 0xe1, 0xff, [](P) -> uint32_t { util::stream_format(stream, "clc");  return 1; } },
	{ 0xe5, 0xff, [](P) -> uint32_t { util::stream_format(stream, "or");   return 1; } },
	{ 0xe6, 0xff, [](P) -> uint32_t { util::stream_format(stream, "daa");  return 1; } },
	{ 0xe7, 0xff, [](P) -> uint32_t { util::stream_format(stream, "and");  return 1; } },
	{ 0xe9, 0xff, [](P) -> uint32_t { util::stream_format(stream, "tla");  return 1; } },
	{ 0xea, 0xff, [](P) -> uint32_t { util::stream_format(stream, "das");  return 1; } },
	{ 0xeb, 0xff, [](P) -> uint32_t { util::stream_format(stream, "cma");  return 1; } },
	{ 0xee, 0xff, [](P) -> uint32_t { util::stream_format(stream, "ind");  return 1; } },
	{ 0xef, 0xff, [](P) -> uint32_t { util::stream_format(stream, "ded");  return 1; } },
	{ 0xf0, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xl0");  return 1; } },
	{ 0xf1, 0xff, [](P) -> uint32_t { util::stream_format(stream, "stc");  return 1; } },
	{ 0xf4, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xl1");  return 1; } },
	{ 0xf5, 0xff, [](P) -> uint32_t { util::stream_format(stream, "exl");  return 1; } },
	{ 0xf6, 0xff, [](P) -> uint32_t { util::stream_format(stream, "halt"); return 1; } },
	{ 0xf7, 0xff, [](P) -> uint32_t { util::stream_format(stream, "tal");  return 1; } },
	{ 0xf8, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xh0");  return 1; } },
	{ 0xf9, 0xff, [](P) -> uint32_t { util::stream_format(stream, "wttm"); return 1; } },
	{ 0xfa, 0xff, [](P) -> uint32_t { util::stream_format(stream, "jpea"); return 1; } },
	{ 0xfb, 0xff, [](P) -> uint32_t { util::stream_format(stream, "cm");   return 1; } },
	{ 0xfc, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xh1");  return 1; } },
	{ 0xfd, 0xff, [](P) -> uint32_t { util::stream_format(stream, "bank"); return 1; } },
	{ 0xfe, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xi");   return 1; } },
	{ 0xff, 0xff, [](P) -> uint32_t { util::stream_format(stream, "xd");   return 1; } },

	{ 0x04, 0xfc, [](P) -> uint32_t { util::stream_format(stream, "spb %x", opcode & 3); return 1; } },
	{ 0x08, 0xfc, [](P) -> uint32_t { util::stream_format(stream, "smb %x", opcode & 3); return 1; } },
	{ 0x24, 0xfc, [](P) -> uint32_t { util::stream_format(stream, "rpb %x", opcode & 3); return 1; } },
	{ 0x28, 0xfc, [](P) -> uint32_t { util::stream_format(stream, "rmb %x", opcode & 3); return 1; } },

	{ 0x30, 0xfc, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bna%x $%03x", opcode & 3, (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x34, 0xfc, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bnm%x $%03x", opcode & 3, (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x38, 0xfc, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bnp%x $%03x", opcode & 3, (pc & 0xf00) | p); return 2 | STEP_COND; } },

	{ 0x64, 0xfc, [](P) -> uint32_t { util::stream_format(stream, "sb %x", opcode & 3); return 1; } },

	{ 0x70, 0xfc, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "ba%x $%03x",  opcode & 3, (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x74, 0xfc, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bm%x $%03x",  opcode & 3, (pc & 0xf00) | p); return 2 | STEP_COND; } },
	{ 0x78, 0xfc, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bp%x $%03x",  opcode & 3, (pc & 0xf00) | p); return 2 | STEP_COND; } },

	{ 0xe0, 0xf3, [](P) -> uint32_t { util::stream_format(stream, "xa%x", (opcode >> 2) & 3); return 1; } },

	{ 0x68, 0xf8, [](P) -> uint32_t { uint16_t p = (pc & 0x800) | ((opcode & 0x07) << 8) | opcodes.r8(pc + 1); util::stream_format(stream, "jmp $%03x", p); return 2; } },

	{ 0xa0, 0xf8, [](P) -> uint32_t { util::stream_format(stream, "xm #$%x", opcode & 7); return 1; } },

	{ 0xa8, 0xf8, [](P) -> uint32_t { uint16_t p = ((opcode & 0x07) << 8) | opcodes.r8(pc + 1); util::stream_format(stream, "cal $%03x", p); return 2 | STEP_OVER; } },

	{ 0x10, 0xf0, [](P) -> uint32_t { util::stream_format(stream, "rfb %x",   opcode & 0x0f); return 1; } },
	{ 0x40, 0xf0, [](P) -> uint32_t { util::stream_format(stream, "lhi #$%x", opcode & 0x0f); return 1; } },
	{ 0x50, 0xf0, [](P) -> uint32_t { util::stream_format(stream, "sfb %x",   opcode & 0x0f); return 1; } },
	{ 0x80, 0xf0, [](P) -> uint32_t { util::stream_format(stream, "ldz #$%x", opcode & 0x0f); return 1; } },

	{ 0x90, 0xf0, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bnf%u $%03x", opcode & 0x0f, (pc & 0xf00) | p); return 2 | STEP_COND; } },

	{ 0xb0, 0xf0, [](P) -> uint32_t { util::stream_format(stream, "czp $%03x", opcode & 0x0f); return 1 | STEP_OVER; } },
	{ 0xc0, 0xf0, [](P) -> uint32_t { util::stream_format(stream, "li #$%x",   opcode & 0x0f); return 1; } },

	{ 0xd0, 0xf0, [](P) -> uint32_t { uint8_t p = opcodes.r8(pc + 1); util::stream_format(stream, "bf%u $%03x",  opcode & 0x0f, (pc & 0xf00) | p); return 2 | STEP_COND; } },

	// fallback
	{ 0x00, 0x00, [](P) -> uint32_t { util::stream_format(stream, "? $%02x", opcode); return 1; } },
};

#undef P


uint32_t lc6500_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t lc6500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint8_t opcode = opcodes.r8(pc);

	for (unsigned i = 0; i < std::size(s_instructions); i++)
		if ((opcode & s_instructions[i].mask) == s_instructions[i].value)
			return s_instructions[i].cb(stream, opcode, opcodes, pc) | SUPPORTED;

	return 0;
}
