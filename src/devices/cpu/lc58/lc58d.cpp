// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// LC58 generic disassembler

#include "emu.h"
#include "lc58d.h"

u32 lc58_disassembler::opcode_alignment() const
{
	return 1;
}

#define P std::ostream &stream, const lc58_disassembler *d, u16 opcode
const lc58_disassembler::instruction lc58_disassembler::instructions[] {
	{ 0x5a00, 0xffff, [](P) -> u32 { util::stream_format(stream, "cla"); return 1; } },
	{ 0xec01, 0xffff, [](P) -> u32 { util::stream_format(stream, "rcf"); return 1; } },
	{ 0xe801, 0xffff, [](P) -> u32 { util::stream_format(stream, "scf"); return 1; } },
	{ 0x6000, 0xfc00, [](P) -> u32 { util::stream_format(stream, "mrw wr%x, %02x", (opcode >> 7) & 7, opcode & 0x7f); return 1; } },
	{ 0x6400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "mwr %02x, wr%x", opcode & 0x7f, (opcode >> 7) & 7); return 1; } },
	{ 0x6800, 0xff80, [](P) -> u32 { util::stream_format(stream, "sr0 %02x", opcode & 0x7f); return 1; } },
	{ 0x6900, 0xff80, [](P) -> u32 { util::stream_format(stream, "sr1 %02x", opcode & 0x7f); return 1; } },
	{ 0x6a00, 0xff80, [](P) -> u32 { util::stream_format(stream, "sl0 %02x", opcode & 0x7f); return 1; } },
	{ 0x6b00, 0xff80, [](P) -> u32 { util::stream_format(stream, "sl1 %02x", opcode & 0x7f); return 1; } },
	{ 0x6c00, 0xff80, [](P) -> u32 { util::stream_format(stream, "rar %02x", opcode & 0x7f); return 1; } },
	{ 0x6e00, 0xff80, [](P) -> u32 { util::stream_format(stream, "ral %02x", opcode & 0x7f); return 1; } },
	{ 0x7400, 0xff80, [](P) -> u32 { util::stream_format(stream, "maf %02x", opcode & 0x7f); return 1; } },
	{ 0x0f80, 0xff80, [](P) -> u32 { util::stream_format(stream, "mra %02x", opcode & 0x7f); return 1; } },

	{ 0x4000, 0xff80, [](P) -> u32 { util::stream_format(stream, "adc %02x", opcode & 0x7f); return 1; } },
	{ 0x4100, 0xff80, [](P) -> u32 { util::stream_format(stream, "adc* %02x", opcode & 0x7f); return 1; } },
	{ 0x4200, 0xff80, [](P) -> u32 { util::stream_format(stream, "sbc %02x", opcode & 0x7f); return 1; } },
	{ 0x4300, 0xff80, [](P) -> u32 { util::stream_format(stream, "sbc* %02x", opcode & 0x7f); return 1; } },
	{ 0x4400, 0xff80, [](P) -> u32 { util::stream_format(stream, "add %02x", opcode & 0x7f); return 1; } },
	{ 0x4500, 0xff80, [](P) -> u32 { util::stream_format(stream, "add* %02x", opcode & 0x7f); return 1; } },
	{ 0x4600, 0xff80, [](P) -> u32 { util::stream_format(stream, "sub %02x", opcode & 0x7f); return 1; } },
	{ 0x4700, 0xff80, [](P) -> u32 { util::stream_format(stream, "sub* %02x", opcode & 0x7f); return 1; } },
	{ 0x4800, 0xff80, [](P) -> u32 { util::stream_format(stream, "adn %02x", opcode & 0x7f); return 1; } },
	{ 0x4900, 0xff80, [](P) -> u32 { util::stream_format(stream, "adn* %02x", opcode & 0x7f); return 1; } },
	{ 0x4a00, 0xff80, [](P) -> u32 { util::stream_format(stream, "and %02x", opcode & 0x7f); return 1; } },
	{ 0x4b00, 0xff80, [](P) -> u32 { util::stream_format(stream, "and* %02x", opcode & 0x7f); return 1; } },
	{ 0x4c00, 0xff80, [](P) -> u32 { util::stream_format(stream, "eor %02x", opcode & 0x7f); return 1; } },
	{ 0x4d00, 0xff80, [](P) -> u32 { util::stream_format(stream, "eor* %02x", opcode & 0x7f); return 1; } },
	{ 0x4e00, 0xff80, [](P) -> u32 { util::stream_format(stream, "or %02x", opcode & 0x7f); return 1; } },
	{ 0x4f00, 0xff80, [](P) -> u32 { util::stream_format(stream, "or* %02x", opcode & 0x7f); return 1; } },
	{ 0x5000, 0xff00, [](P) -> u32 { util::stream_format(stream, "adci #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5100, 0xff00, [](P) -> u32 { util::stream_format(stream, "adci* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5200, 0xff00, [](P) -> u32 { util::stream_format(stream, "sbci #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5300, 0xff00, [](P) -> u32 { util::stream_format(stream, "sbci* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5400, 0xff00, [](P) -> u32 { util::stream_format(stream, "addi #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5500, 0xff00, [](P) -> u32 { util::stream_format(stream, "addi* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5600, 0xff00, [](P) -> u32 { util::stream_format(stream, "subi #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5700, 0xff00, [](P) -> u32 { util::stream_format(stream, "subi* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5800, 0xff00, [](P) -> u32 { util::stream_format(stream, "adni #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5900, 0xff00, [](P) -> u32 { util::stream_format(stream, "adni* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5a00, 0xff00, [](P) -> u32 { util::stream_format(stream, "andi #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5b00, 0xff00, [](P) -> u32 { util::stream_format(stream, "andi* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5c00, 0xff00, [](P) -> u32 { util::stream_format(stream, "eori #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5d00, 0xff00, [](P) -> u32 { util::stream_format(stream, "eori* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5e00, 0xff00, [](P) -> u32 { util::stream_format(stream, "ori #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },
	{ 0x5f00, 0xff00, [](P) -> u32 { util::stream_format(stream, "ori* #%x, %02x", (opcode >> 4) & 0xf, 0x70 + (opcode & 0xf)); return 1; } },

	{ 0x7700, 0xff80, [](P) -> u32 { util::stream_format(stream, "mdpl %02x", opcode & 0x7f); return 1; } },
	{ 0x7680, 0xff80, [](P) -> u32 { util::stream_format(stream, "mdph %02x", opcode & 0x7f); return 1; } },
	{ 0x6b80, 0xff80, [](P) -> u32 { util::stream_format(stream, "mrdl %02x", opcode & 0x7f); return 1; } },
	{ 0x6980, 0xff80, [](P) -> u32 { util::stream_format(stream, "mrdh %02x", opcode & 0x7f); return 1; } },
	{ 0x6a80, 0xff80, [](P) -> u32 { util::stream_format(stream, "mrsp %02x", opcode & 0x7f); return 1; } },
	{ 0xe800, 0xfc00, [](P) -> u32 { util::stream_format(stream, "sf1 %03x", opcode & 0x3ff); return 1; } },
	{ 0xec00, 0xfc00, [](P) -> u32 { util::stream_format(stream, "rf1 %03x", opcode & 0x3ff); return 1; } },
	{ 0xf000, 0xfc00, [](P) -> u32 { util::stream_format(stream, "sf2 %03x", opcode & 0x3ff); return 1; } },
	{ 0xf400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "rf2 %03x", opcode & 0x3ff); return 1; } },
	{ 0xe880, 0xffff, [](P) -> u32 { util::stream_format(stream, "sdpf"); return 1; } },
	{ 0xec80, 0xffff, [](P) -> u32 { util::stream_format(stream, "rdpf"); return 1; } },

	{ 0x7780, 0xff80, [](P) -> u32 { util::stream_format(stream, "sta %02x", opcode & 0x7f); return 1; } },
	{ 0x7800, 0xf800, [](P) -> u32 { util::stream_format(stream, "lds %02x, #%x", opcode & 0x7f, (opcode >> 7) & 0xf); return 1; } },
	{ 0x6880, 0xff80, [](P) -> u32 { util::stream_format(stream, "lda %02x", opcode & 0x7f); return 1; } },

	{ 0xff00, 0xffff, [](P) -> u32 { util::stream_format(stream, "halt"); return 1; } },
	{ 0xe000, 0xffc0, [](P) -> u32 { util::stream_format(stream, "ssw %02x", opcode & 0x3f); return 1; } },
	{ 0xfa00, 0xfe00, [](P) -> u32 { util::stream_format(stream, "sie %03x", opcode & 0x1ff); return 1 | STEP_OVER; } },
	{ 0xfe00, 0xfff0, [](P) -> u32 { util::stream_format(stream, "sie* %x", opcode & 0xf); return 1 | STEP_OVER; } },
	{ 0x7580, 0xff80, [](P) -> u32 { util::stream_format(stream, "msb %02x", opcode & 0x7f); return 1; } },
	{ 0x7600, 0xff80, [](P) -> u32 { util::stream_format(stream, "msc %02x", opcode & 0x7f); return 1; } },
	{ 0x0000, 0xffff, [](P) -> u32 { util::stream_format(stream, "nop"); return 1; } },
	{ 0xf018, 0xffff, [](P) -> u32 { util::stream_format(stream, "lon"); return 1; } },
	{ 0xf418, 0xffff, [](P) -> u32 { util::stream_format(stream, "loff"); return 1; } },
	{ 0xf004, 0xffff, [](P) -> u32 { util::stream_format(stream, "sbak"); return 1; } },
	{ 0xf404, 0xffff, [](P) -> u32 { util::stream_format(stream, "rbak"); return 1; } },

	{ 0x7480, 0xff80, [](P) -> u32 { util::stream_format(stream, "mcd %02x", opcode & 0x7f); return 1; } },
	{ 0x7500, 0xff80, [](P) -> u32 { util::stream_format(stream, "mcf %02x", opcode & 0x7f); return 1; } },
	{ 0xfc22, 0xffff, [](P) -> u32 { util::stream_format(stream, "ccc"); return 1; } },
	{ 0xfc40, 0xffff, [](P) -> u32 { util::stream_format(stream, "rlp"); return 1; } },
	{ 0xfc80, 0xffff, [](P) -> u32 { util::stream_format(stream, "csp"); return 1; } },
	{ 0xfb00, 0xffff, [](P) -> u32 { util::stream_format(stream, "cst"); return 1; } },
	{ 0xe802, 0xffff, [](P) -> u32 { util::stream_format(stream, "scex"); return 1; } },
	{ 0xec02, 0xffff, [](P) -> u32 { util::stream_format(stream, "rcex"); return 1; } },
	{ 0xf000, 0xfffc, [](P) -> u32 { util::stream_format(stream, "schf %x", opcode & 3); return 1; } },
	{ 0xf400, 0xfffc, [](P) -> u32 { util::stream_format(stream, "rchf %x", opcode & 3); return 1; } },

	{ 0x7180, 0xff80, [](P) -> u32 { util::stream_format(stream, "ipp %02x", opcode & 0x7f); return 1; } },
	{ 0x7000, 0xff80, [](P) -> u32 { util::stream_format(stream, "ips %02x", opcode & 0x7f); return 1; } },
	{ 0x7080, 0xff80, [](P) -> u32 { util::stream_format(stream, "ipm %02x", opcode & 0x7f); return 1; } },
	{ 0x7100, 0xff80, [](P) -> u32 { util::stream_format(stream, "ipk %02x", opcode & 0x7f); return 1; } },
	{ 0x0080, 0xff80, [](P) -> u32 { util::stream_format(stream, "opp %02x", opcode & 0x7f); return 1; } },
	{ 0x0f00, 0xff80, [](P) -> u32 { util::stream_format(stream, "opm %02x", opcode & 0x7f); return 1; } },
	{ 0xe804, 0xffff, [](P) -> u32 { util::stream_format(stream, "sct1"); return 1; } },
	{ 0xec04, 0xffff, [](P) -> u32 { util::stream_format(stream, "rct1"); return 1; } },
	{ 0xf200, 0xffff, [](P) -> u32 { util::stream_format(stream, "sct2"); return 1; } },
	{ 0xf600, 0xffff, [](P) -> u32 { util::stream_format(stream, "rct2"); return 1; } },
	{ 0xf010, 0xffff, [](P) -> u32 { util::stream_format(stream, "slgt"); return 1; } },
	{ 0xf410, 0xffff, [](P) -> u32 { util::stream_format(stream, "rlgt"); return 1; } },
	{ 0xf800, 0xffff, [](P) -> u32 { util::stream_format(stream, "ras"); return 1; } },
	{ 0xf800, 0xfe00, [](P) -> u32 { util::stream_format(stream, "sas %03x", opcode & 0x1ff); return 1; } },
	{ 0xe800, 0xff9f, [](P) -> u32 { util::stream_format(stream, "comd %x", (opcode >> 5) & 3); return 1; } },
	{ 0xec00, 0xff9f, [](P) -> u32 { util::stream_format(stream, "cimd %x", (opcode >> 5) & 3); return 1; } },
	{ 0xf000, 0xfe1f, [](P) -> u32 { util::stream_format(stream, "spdf %x", (opcode >> 5) & 3); return 1; } },
	{ 0xf400, 0xfe1f, [](P) -> u32 { util::stream_format(stream, "rpdf %x", (opcode >> 5) & 3); return 1; } },
	{ 0xe808, 0xffff, [](P) -> u32 { util::stream_format(stream, "sfsp"); return 1; } },
	{ 0xec08, 0xffff, [](P) -> u32 { util::stream_format(stream, "rfsp"); return 1; } },
	{ 0x0000, 0xf000, [](P) -> u32 { util::stream_format(stream, "wrt %02x, %02x", (opcode >> 7) & 0x1f, opcode & 0x7f); return 1; } },
	{ 0x1000, 0xf000, [](P) -> u32 { util::stream_format(stream, "wrb %02x, %02x", (opcode >> 7) & 0x1f, opcode & 0x7f); return 1; } },
	{ 0x2000, 0xf000, [](P) -> u32 { util::stream_format(stream, "wrc %02x, %02x", (opcode >> 7) & 0x1f, opcode & 0x7f); return 1; } },
	{ 0x3000, 0xf000, [](P) -> u32 { util::stream_format(stream, "wrp %02x, %02x", (opcode >> 7) & 0x1f, opcode & 0x7f); return 1; } },

	{ 0xc000, 0xf800, [](P) -> u32 { util::stream_format(stream, "jmp %03x", opcode & 0x7ff); return 1; } },
	{ 0x8000, 0xf800, [](P) -> u32 { util::stream_format(stream, "bab0 %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0x8800, 0xf800, [](P) -> u32 { util::stream_format(stream, "bab1 %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0x9000, 0xf800, [](P) -> u32 { util::stream_format(stream, "bab2 %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0x9800, 0xf800, [](P) -> u32 { util::stream_format(stream, "bab3 %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0xa000, 0xf800, [](P) -> u32 { util::stream_format(stream, "banz %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0xb000, 0xf800, [](P) -> u32 { util::stream_format(stream, "baz %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0xa800, 0xf800, [](P) -> u32 { util::stream_format(stream, "bcnh %03x", opcode & 0x7ff); return 1 | STEP_COND; } },
	{ 0xb800, 0xf800, [](P) -> u32 { util::stream_format(stream, "bch %03x", opcode & 0x7ff); return 1 | STEP_COND; } },

	{ 0xc800, 0xf800, [](P) -> u32 { util::stream_format(stream, "call %03x", opcode & 0x7ff); return 1 | STEP_OVER; } },
	{ 0xd000, 0xffff, [](P) -> u32 { util::stream_format(stream, "rts"); return 1 | STEP_OUT; } },
	{ 0xd800, 0xffff, [](P) -> u32 { util::stream_format(stream, "pop"); return 1; } },

	{ 0xe400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "stm %03x", opcode & 0x7ff); return 1; } },
	{ 0xfc04, 0xffff, [](P) -> u32 { util::stream_format(stream, "rtm"); return 1 | STEP_OUT; } },
	{ 0xf810, 0xffff, [](P) -> u32 { util::stream_format(stream, "sfpd"); return 1 | STEP_OUT; } },
	{ 0xfc10, 0xffff, [](P) -> u32 { util::stream_format(stream, "rfpd"); return 1 | STEP_OUT; } },
	{ 0xfc00, 0xfe00, [](P) -> u32 { util::stream_format(stream, "plc %03x", opcode & 0x1ff); return 1 | STEP_OVER; } },

	{ 0x0000, 0x0000, [](P) -> u32 { util::stream_format(stream, "?%04x",   opcode); return 1; } },
};

#undef P

offs_t lc58_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, this, opcode) | SUPPORTED;
	return 0;
}
