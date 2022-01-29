// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// LC57 generic disassembler

#include "emu.h"
#include "lc57d.h"

u32 lc57_disassembler::opcode_alignment() const
{
	return 1;
}

#define P std::ostream &stream, const data_buffer &opcodes, u8 opcode, u16 pc
const lc57_disassembler::instruction lc57_disassembler::instructions[] {
	{ 0xf0, 0xff, [](P) -> u32 { util::stream_format(stream, "rcf"); return 1; } },
	{ 0xf1, 0xff, [](P) -> u32 { util::stream_format(stream, "scf"); return 1; } },
	{ 0x18, 0xff, [](P) -> u32 { util::stream_format(stream, "asr0"); return 1; } },
	{ 0x19, 0xff, [](P) -> u32 { util::stream_format(stream, "asr1"); return 1; } },
	{ 0x1a, 0xff, [](P) -> u32 { util::stream_format(stream, "asl0"); return 1; } },
	{ 0x1b, 0xff, [](P) -> u32 { util::stream_format(stream, "asl1"); return 1; } },
	{ 0x98, 0xff, [](P) -> u32 { util::stream_format(stream, "inc"); return 1; } },
	{ 0x99, 0xff, [](P) -> u32 { util::stream_format(stream, "dec"); return 1; } },
	{ 0x01, 0xff, [](P) -> u32 { util::stream_format(stream, "taat"); return 1; } },
	{ 0x12, 0xff, [](P) -> u32 { util::stream_format(stream, "mtr"); return 1; } },

	{ 0x80, 0xff, [](P) -> u32 { util::stream_format(stream, "adc"); return 1; } },
	{ 0x88, 0xff, [](P) -> u32 { util::stream_format(stream, "adc*"); return 1; } },
	{ 0x81, 0xff, [](P) -> u32 { util::stream_format(stream, "sbc"); return 1; } },
	{ 0x89, 0xff, [](P) -> u32 { util::stream_format(stream, "sbc*"); return 1; } },
	{ 0x82, 0xff, [](P) -> u32 { util::stream_format(stream, "add"); return 1; } },
	{ 0x8a, 0xff, [](P) -> u32 { util::stream_format(stream, "add*"); return 1; } },
	{ 0x83, 0xff, [](P) -> u32 { util::stream_format(stream, "sub"); return 1; } },
	{ 0x8b, 0xff, [](P) -> u32 { util::stream_format(stream, "sub*"); return 1; } },
	{ 0x84, 0xff, [](P) -> u32 { util::stream_format(stream, "adn"); return 1; } },
	{ 0x8c, 0xff, [](P) -> u32 { util::stream_format(stream, "adn*"); return 1; } },
	{ 0x85, 0xff, [](P) -> u32 { util::stream_format(stream, "and"); return 1; } },
	{ 0x8d, 0xff, [](P) -> u32 { util::stream_format(stream, "and*"); return 1; } },
	{ 0x86, 0xff, [](P) -> u32 { util::stream_format(stream, "eor"); return 1; } },
	{ 0x8e, 0xff, [](P) -> u32 { util::stream_format(stream, "eor*"); return 1; } },
	{ 0x87, 0xff, [](P) -> u32 { util::stream_format(stream, "or"); return 1; } },
	{ 0x8f, 0xff, [](P) -> u32 { util::stream_format(stream, "or*"); return 1; } },
	{ 0x90, 0xff, [](P) -> u32 { util::stream_format(stream, "adci %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x91, 0xff, [](P) -> u32 { util::stream_format(stream, "sbci %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x92, 0xff, [](P) -> u32 { util::stream_format(stream, "addi %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x93, 0xff, [](P) -> u32 { util::stream_format(stream, "subi %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x94, 0xff, [](P) -> u32 { util::stream_format(stream, "adni %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x95, 0xff, [](P) -> u32 { util::stream_format(stream, "andi %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x96, 0xff, [](P) -> u32 { util::stream_format(stream, "eori %x", opcodes.r8(pc+1) & 0xf); return 2; } },
	{ 0x97, 0xff, [](P) -> u32 { util::stream_format(stream, "ori %x", opcodes.r8(pc+1) & 0xf); return 2; } },

	{ 0x1c, 0xff, [](P) -> u32 { util::stream_format(stream, "sdpl"); return 1; } },
	{ 0x13, 0xff, [](P) -> u32 { util::stream_format(stream, "sdph"); return 1; } },
	{ 0x1e, 0xff, [](P) -> u32 { util::stream_format(stream, "edpl"); return 1; } },
	{ 0x1f, 0xff, [](P) -> u32 { util::stream_format(stream, "edph"); return 1; } },
	{ 0x9a, 0xff, [](P) -> u32 { util::stream_format(stream, "idpl"); return 1; } },
	{ 0x9b, 0xff, [](P) -> u32 { util::stream_format(stream, "ddpl"); return 1; } },
	{ 0x9c, 0xff, [](P) -> u32 { util::stream_format(stream, "idph"); return 1; } },
	{ 0x9d, 0xff, [](P) -> u32 { util::stream_format(stream, "ddph"); return 1; } },
	{ 0xfd, 0xff, [](P) -> u32 { util::stream_format(stream, "ldpl"); return 1; } },
	{ 0xfe, 0xff, [](P) -> u32 { util::stream_format(stream, "ldph"); return 1; } },
	{ 0xb0, 0xf0, [](P) -> u32 { util::stream_format(stream, "mdpl %x", opcode & 0xf); return 1; } },
	{ 0xc0, 0xf0, [](P) -> u32 { util::stream_format(stream, "mdph %x", opcode & 0xf); return 1; } },

	{ 0xab, 0xff, [](P) -> u32 { util::stream_format(stream, "lhlt"); return 1; } },
	{ 0xac, 0xff, [](P) -> u32 { util::stream_format(stream, "l500"); return 1; } },
	{ 0x04, 0xff, [](P) -> u32 { util::stream_format(stream, "csp"); return 1; } },
	{ 0x05, 0xff, [](P) -> u32 { util::stream_format(stream, "cst"); return 1; } },
	{ 0x06, 0xff, [](P) -> u32 { util::stream_format(stream, "rc5"); return 1; } },
	{ 0x07, 0xff, [](P) -> u32 { util::stream_format(stream, "rc5"); return 1; } },

	{ 0x9e, 0xff, [](P) -> u32 { util::stream_format(stream, "isp"); return 1; } },
	{ 0x9f, 0xff, [](P) -> u32 { util::stream_format(stream, "dsp"); return 1; } },
	{ 0xaa, 0xff, [](P) -> u32 { util::stream_format(stream, "lsp"); return 1; } },
	{ 0xae, 0xff, [](P) -> u32 { util::stream_format(stream, "ssp"); return 1; } },
	{ 0xe0, 0xf0, [](P) -> u32 { util::stream_format(stream, "msp %x", opcode & 0xf); return 1; } },

	{ 0xa9, 0xff, [](P) -> u32 { util::stream_format(stream, "lda"); return 1; } },
	{ 0xad, 0xff, [](P) -> u32 { util::stream_format(stream, "sta"); return 1; } },
	{ 0x20, 0xf0, [](P) -> u32 { util::stream_format(stream, "mvi %x", opcode & 0xf); return 1; } },
	{ 0x30, 0xf0, [](P) -> u32 { util::stream_format(stream, "ldi %x", opcode & 0xf); return 1; } },

	{ 0x00, 0xff, [](P) -> u32 { util::stream_format(stream, "halt"); return 1; } },
	{ 0xd0, 0xf0, [](P) -> u32 { util::stream_format(stream, "sic %x", opcode & 0xf); return 1; } },
	{ 0xff, 0xff, [](P) -> u32 { util::stream_format(stream, "nop"); return 1; } },
	{ 0xf8, 0xff, [](P) -> u32 { util::stream_format(stream, "rbak"); return 1; } },
	{ 0xf9, 0xff, [](P) -> u32 { util::stream_format(stream, "sbak"); return 1; } },

	{ 0xaf, 0xff, [](P) -> u32 { util::stream_format(stream, "ips"); return 1; } },
	{ 0xa8, 0xff, [](P) -> u32 { util::stream_format(stream, "ipm"); return 1; } },
	{ 0xfa, 0xff, [](P) -> u32 { util::stream_format(stream, "sas %02x", opcodes.r8(pc+1)); return 2; } },
	{ 0xf4, 0xfc, [](P) -> u32 { util::stream_format(stream, "spdr %x", opcode & 0x3); return 1; } },
	{ 0xfc, 0xff, [](P) -> u32 { util::stream_format(stream, "out"); return 1; } },
	{ 0x17, 0xff, [](P) -> u32 { util::stream_format(stream, "in"); return 1; } },
	{ 0x02, 0xff, [](P) -> u32 { util::stream_format(stream, "twrt"); return 1; } },
	{ 0x03, 0xff, [](P) -> u32 { util::stream_format(stream, "tmel"); return 1; } },

	{ 0x08, 0xf8, [](P) -> u32 { util::stream_format(stream, "jmp %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x48, 0xf8, [](P) -> u32 { util::stream_format(stream, "bab0 %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x58, 0xf8, [](P) -> u32 { util::stream_format(stream, "bab1 %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x68, 0xf8, [](P) -> u32 { util::stream_format(stream, "bab2 %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x78, 0xf8, [](P) -> u32 { util::stream_format(stream, "bab3 %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x50, 0xf8, [](P) -> u32 { util::stream_format(stream, "banz %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x40, 0xf8, [](P) -> u32 { util::stream_format(stream, "baz %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x60, 0xf8, [](P) -> u32 { util::stream_format(stream, "bcnh %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x70, 0xf8, [](P) -> u32 { util::stream_format(stream, "bch %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x11, 0xff, [](P) -> u32 { util::stream_format(stream, "page"); return 1; } },
	{ 0x10, 0xff, [](P) -> u32 { util::stream_format(stream, "jmp*"); return 1; } },

	{ 0xa0, 0xf8, [](P) -> u32 { util::stream_format(stream, "jsr %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0x13, 0xff, [](P) -> u32 { util::stream_format(stream, "rts"); return 1; } },

	{ 0x14, 0xff, [](P) -> u32 { util::stream_format(stream, "mpcl"); return 1; } },
	{ 0x15, 0xff, [](P) -> u32 { util::stream_format(stream, "mpcm"); return 1; } },
	{ 0x16, 0xff, [](P) -> u32 { util::stream_format(stream, "mpch"); return 1; } },
	{ 0xfb, 0xff, [](P) -> u32 { util::stream_format(stream, "csec"); return 1; } },

	{ 0x00, 0x00, [](P) -> u32 { util::stream_format(stream, "?%0x", opcode); return 1; } },
};

#undef P

offs_t lc57_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r8(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, opcodes, opcode, pc) | SUPPORTED;
	return 0;
}
