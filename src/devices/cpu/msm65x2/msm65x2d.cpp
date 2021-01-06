// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// MSM65X2 generic disassembler

#include "emu.h"
#include "msm65x2d.h"

u32 msm65x2_disassembler::opcode_alignment() const
{
	return 1;
}

#define P std::ostream &stream, const data_buffer &opcodes, u8 opcode, u16 pc
const msm65x2_disassembler::instruction msm65x2_disassembler::instructions[] {
	{ 0x80, 0xf0, [](P) -> u32 { util::stream_format(stream, "lai %x", opcode & 0xf); return 1; } },
	{ 0x70, 0xf0, [](P) -> u32 { util::stream_format(stream, "lli %x", opcode & 0xf); return 1; } },
	{ 0x6a, 0xff, [](P) -> u32 { util::stream_format(stream, "lhli %02x", opcodes.r8(pc+1)); return 2; } },
	{ 0x69, 0xff, [](P) -> u32 { util::stream_format(stream, "lxi %02x", opcodes.r8(pc+1)); return 2; } },
	{ 0xb0, 0xff, [](P) -> u32 { util::stream_format(stream, "lam"); return 1; } },
	{ 0xb1, 0xff, [](P) -> u32 { util::stream_format(stream, "lal"); return 1; } },
	{ 0xb2, 0xff, [](P) -> u32 { util::stream_format(stream, "lah"); return 1; } },
	{ 0xb3, 0xff, [](P) -> u32 { util::stream_format(stream, "lma"); return 1; } },
	{ 0xb4, 0xff, [](P) -> u32 { util::stream_format(stream, "lla"); return 1; } },
	{ 0xb5, 0xff, [](P) -> u32 { util::stream_format(stream, "lha"); return 1; } },
	{ 0x1b, 0xff, [](P) -> u32 { util::stream_format(stream, "lmad %02x", opcodes.r8(pc+1)); return 2; } },
	{ 0x1a, 0xff, [](P) -> u32 { util::stream_format(stream, "lmda %02x", opcodes.r8(pc+1)); return 2; } },
	{ 0x67, 0xff, [](P) -> u32 { util::stream_format(stream, "lmt"); return 1; } },
	{ 0xbc, 0xff, [](P) -> u32 { util::stream_format(stream, "push hl"); return 1; } },
	{ 0xbd, 0xff, [](P) -> u32 { util::stream_format(stream, "push ca"); return 1; } },
	{ 0xbe, 0xff, [](P) -> u32 { util::stream_format(stream, "pop hl"); return 1; } },
	{ 0xbf, 0xff, [](P) -> u32 { util::stream_format(stream, "pop ca"); return 1; } },

	{ 0xb8, 0xff, [](P) -> u32 { util::stream_format(stream, "xam"); return 1; } },
	{ 0x1c, 0xff, [](P) -> u32 { util::stream_format(stream, "xamd %02x", opcodes.r8(pc+1)); return 2; } },
	{ 0x3f, 0xff, [](P) -> u32 { util::stream_format(stream, "xhs"); return 1; } },

	{ 0x10, 0xff, [](P) -> u32 { util::stream_format(stream, "ina"); return 1; } },
	{ 0x11, 0xff, [](P) -> u32 { util::stream_format(stream, "inl"); return 1; } },
	{ 0x12, 0xff, [](P) -> u32 { util::stream_format(stream, "inm"); return 1; } },
	{ 0x5c, 0xff, [](P) -> u32 { util::stream_format(stream, "inx"); return 1; } },
	{ 0x14, 0xff, [](P) -> u32 { util::stream_format(stream, "dca"); return 1; } },
	{ 0x15, 0xff, [](P) -> u32 { util::stream_format(stream, "dcl"); return 1; } },
	{ 0x16, 0xff, [](P) -> u32 { util::stream_format(stream, "dcm"); return 1; } },
	{ 0x5d, 0xff, [](P) -> u32 { util::stream_format(stream, "dcx"); return 1; } },

	{ 0x60, 0xff, [](P) -> u32 { util::stream_format(stream, "dsc"); return 1; } },
	{ 0x61, 0xff, [](P) -> u32 { util::stream_format(stream, "dac"); return 1; } },
	{ 0x62, 0xff, [](P) -> u32 { util::stream_format(stream, "ads"); return 1; } },
	{ 0x63, 0xff, [](P) -> u32 { util::stream_format(stream, "adcs"); return 1; } },
	{ 0x00, 0xf0, [](P) -> u32 { util::stream_format(stream, "ais %x", opcode & 0xf); return 1; } },
	{ 0x65, 0xff, [](P) -> u32 { util::stream_format(stream, "cma"); return 1; } },
	{ 0x66, 0xff, [](P) -> u32 { util::stream_format(stream, "eor"); return 1; } },
	{ 0x4c, 0xff, [](P) -> u32 { util::stream_format(stream, "and"); return 1; } },
	{ 0x4d, 0xff, [](P) -> u32 { util::stream_format(stream, "or"); return 1; } },
	{ 0x6b, 0xff, [](P) -> u32 { util::stream_format(stream, "cam"); return 1; } },
	{ 0x6c, 0xff, [](P) -> u32 { util::stream_format(stream, "cpal"); return 1; } },
	{ 0x6d, 0xff, [](P) -> u32 { util::stream_format(stream, "caxl"); return 1; } },
	{ 0x6e, 0xff, [](P) -> u32 { util::stream_format(stream, "caxh"); return 1; } },
	{ 0x1f, 0xff, [](P) -> u32 { util::stream_format(stream, "sc"); return 1; } },
	{ 0x1e, 0xff, [](P) -> u32 { util::stream_format(stream, "rc"); return 1; } },
	{ 0x18, 0xff, [](P) -> u32 { util::stream_format(stream, "ral"); return 1; } },
	{ 0x19, 0xff, [](P) -> u32 { util::stream_format(stream, "rar"); return 1; } },

	{ 0x30, 0xfc, [](P) -> u32 { util::stream_format(stream, "smb %x", opcode & 0x3); return 1; } },
	{ 0x50, 0xfc, [](P) -> u32 { util::stream_format(stream, "smbd %02x, %x", opcodes.r8(pc+1), opcode & 3); return 2; } },
	{ 0x20, 0xfc, [](P) -> u32 { util::stream_format(stream, "spb %x", opcode & 0x3); return 1; } },
	{ 0x34, 0xfc, [](P) -> u32 { util::stream_format(stream, "rmb %x", opcode & 0x3); return 1; } },
	{ 0x54, 0xfc, [](P) -> u32 { util::stream_format(stream, "rmbd %02x, %x", opcodes.r8(pc+1), opcode & 3); return 2; } },
	{ 0x24, 0xfc, [](P) -> u32 { util::stream_format(stream, "rpb %x", opcode & 0x3); return 1; } },
	{ 0x38, 0xfc, [](P) -> u32 { util::stream_format(stream, "tmb %x", opcode & 0x3); return 1; } },
	{ 0x58, 0xfc, [](P) -> u32 { util::stream_format(stream, "tmbd %02x, %x", opcodes.r8(pc+1), opcode & 3); return 2; } },
	{ 0x28, 0xfc, [](P) -> u32 { util::stream_format(stream, "tpb %x", opcode & 0x3); return 1; } },
	{ 0x2c, 0xfc, [](P) -> u32 { util::stream_format(stream, "tab %x", opcode & 0x3); return 1; } },
	{ 0x49, 0xfc, [](P) -> u32 { util::stream_format(stream, "tirb %x", opcode & 0x3); return 1; } },

	{ 0x5e, 0xff, [](P) -> u32 { util::stream_format(stream, "ei"); return 1; } },
	{ 0x5f, 0xff, [](P) -> u32 { util::stream_format(stream, "di"); return 1; } },

	{ 0x90, 0xf8, [](P) -> u32 { util::stream_format(stream, "j %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0xa0, 0xf8, [](P) -> u32 { util::stream_format(stream, "cal %x%02x", opcode & 7, opcodes.r8(pc+1)); return 2; } },
	{ 0xc0, 0xc0, [](P) -> u32 { util::stream_format(stream, "jcp %03x", (pc & 0x7c0) | (opcode & 0x3f)); return 1; } },
	{ 0x3c, 0xff, [](P) -> u32 { util::stream_format(stream, "rt"); return 1; } },
	{ 0x3d, 0xff, [](P) -> u32 { util::stream_format(stream, "rts"); return 1; } },

	{ 0xb3, 0xff, [](P) -> u32 { util::stream_format(stream, "ip"); return 1; } },
	{ 0xb7, 0xff, [](P) -> u32 { util::stream_format(stream, "op"); return 1; } },

	{ 0x4f, 0xff, [](P) -> u32 { util::stream_format(stream, "halt"); return 1; } },
	{ 0x00, 0xff, [](P) -> u32 { util::stream_format(stream, "nop"); return 1; } },

	{ 0x00, 0x00, [](P) -> u32 { util::stream_format(stream, "?%0x", opcode); return 1; } },
};

#undef P

offs_t msm65x2_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 opcode = opcodes.r8(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, opcodes, opcode, pc) | SUPPORTED;
	return 0;
}
