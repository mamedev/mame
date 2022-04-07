// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PIC1670 disassembler

#include "emu.h"
#include "pic1670d.h"

u32 pic1670_disassembler::opcode_alignment() const
{
	return 1;
}

char pic1670_disassembler::fw(u16 opcode)
{
	return opcode & 0x40 ? 'f' : 'w';
}

std::string pic1670_disassembler::freg(u16 opcode) const
{
	static const char *const fnames[0x10] = {
		"(fsr)", "w", "pc", "asr", "fsr", "isr", "rtcca", "rtccb",
		"riopa", "iopa", "riopb", "iopb", "riorpc", "iopc", "riopd", "iopd"
	};
	u16 reg = opcode & 0x3f;
	if(reg < 0x10)
		return fnames[reg];
	return util::string_format("f_%02x", reg);
}

std::string pic1670_disassembler::imm8(u16 opcode)
{
	if((opcode & 0xff) < 10)
		return util::string_format("%d", opcode & 0xff);
	else
		return util::string_format("'h'%02x", opcode & 0xff);
}

// Note: the decodes have more don't care than the manual, they reflect the
// reality of the opcode decode of the die, e.g. bits 3,4,5 are not used there

#define P std::ostream &stream, const pic1670_disassembler *d, u16 opcode, u16 pc
const pic1670_disassembler::instruction pic1670_disassembler::instructions[] {
	{ 0x0004, 0x1fc7, [](P) -> u32 { util::stream_format(stream, "daw"                                       ); return 1; } },
	{ 0x0040, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "movwf %s",      d->freg(opcode)            ); return 1; } },
	{ 0x0080, 0x1f80, [](P) -> u32 { util::stream_format(stream, "subbwf %s, %c", d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0100, 0x1f80, [](P) -> u32 { util::stream_format(stream, "subwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0180, 0x1f80, [](P) -> u32 { util::stream_format(stream, "decf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0200, 0x1f80, [](P) -> u32 { util::stream_format(stream, "iorwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0280, 0x1f80, [](P) -> u32 { util::stream_format(stream, "andwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0300, 0x1f80, [](P) -> u32 { util::stream_format(stream, "xorwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0380, 0x1f80, [](P) -> u32 { util::stream_format(stream, "addwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0400, 0x1f80, [](P) -> u32 { util::stream_format(stream, "adcwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0480, 0x1f80, [](P) -> u32 { util::stream_format(stream, "comf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0500, 0x1f80, [](P) -> u32 { util::stream_format(stream, "incf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0580, 0x1f80, [](P) -> u32 { util::stream_format(stream, "decfsz %s, %c", d->freg(opcode), fw(opcode)); return 1 | STEP_COND; } },
	{ 0x0600, 0x1f80, [](P) -> u32 { util::stream_format(stream, "rlcf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0680, 0x1f80, [](P) -> u32 { util::stream_format(stream, "rrcf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0700, 0x1f80, [](P) -> u32 { util::stream_format(stream, "swapf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0780, 0x1f80, [](P) -> u32 { util::stream_format(stream, "incfsz %s, %c", d->freg(opcode), fw(opcode)); return 1 | STEP_COND; } },

	{ 0x1000, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "movfw %s",      d->freg(opcode)            ); return 1; } },
	{ 0x1040, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "clrf %s",       d->freg(opcode)            ); return 1; } },
	{ 0x1080, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "rrncf %s",      d->freg(opcode)            ); return 1; } },
	{ 0x10c0, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "rlncf %s",      d->freg(opcode)            ); return 1; } },
	{ 0x1100, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "cpfslt %s",     d->freg(opcode)            ); return 1 | STEP_COND; } },
	{ 0x1140, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "cpfseq %s",     d->freg(opcode)            ); return 1 | STEP_COND; } },
	{ 0x1180, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "cpfsgt %s",     d->freg(opcode)            ); return 1 | STEP_COND; } },
	{ 0x11c0, 0x1fc0, [](P) -> u32 { util::stream_format(stream, "testf %s",      d->freg(opcode)            ); return 1; } },

	{ 0x0800, 0x1e00, [](P) -> u32 { util::stream_format(stream, "bcf %d, %s",   (opcode >> 6) & 7, d->freg(opcode)); return 1; } },
	{ 0x0a00, 0x1e00, [](P) -> u32 { util::stream_format(stream, "bsf %d, %s",   (opcode >> 6) & 7, d->freg(opcode)); return 1; } },
	{ 0x0c00, 0x1e00, [](P) -> u32 { util::stream_format(stream, "btfsc %d, %s", (opcode >> 6) & 7, d->freg(opcode)); return 1 | STEP_COND; } },
	{ 0x0e00, 0x1e00, [](P) -> u32 { util::stream_format(stream, "btfss %d, %s", (opcode >> 6) & 7, d->freg(opcode)); return 1 | STEP_COND; } },

	{ 0x0000, 0x1fc7, [](P) -> u32 { util::stream_format(stream, "nop"                   ); return 1; } },
	{ 0x0001, 0x1fc7, [](P) -> u32 { util::stream_format(stream, "halt"                  ); return 1; } },
	{ 0x0002, 0x1fc7, [](P) -> u32 { util::stream_format(stream, "retfi"                 ); return 1 | STEP_OUT; } },
	{ 0x0003, 0x1fc7, [](P) -> u32 { util::stream_format(stream, "retfs"                 ); return 1 | STEP_OUT; } },
	{ 0x1200, 0x1f00, [](P) -> u32 { util::stream_format(stream, "movlw %s", imm8(opcode)); return 1; } },
	{ 0x1300, 0x1f00, [](P) -> u32 { util::stream_format(stream, "addlw %s", imm8(opcode)); return 1; } },
	{ 0x1400, 0x1f00, [](P) -> u32 { util::stream_format(stream, "iorlw %s", imm8(opcode)); return 1; } },
	{ 0x1500, 0x1f00, [](P) -> u32 { util::stream_format(stream, "andlw %s", imm8(opcode)); return 1; } },
	{ 0x1600, 0x1f00, [](P) -> u32 { util::stream_format(stream, "xorlw %s", imm8(opcode)); return 1; } },
	{ 0x1700, 0x1f00, [](P) -> u32 { util::stream_format(stream, "retlw %s", imm8(opcode)); return 1 | STEP_OUT; } },

	{ 0x1800, 0x1c00, [](P) -> u32 { util::stream_format(stream, "goto %03x", opcode & 0x3ff); return 1; } },
	{ 0x1c00, 0x1c00, [](P) -> u32 { util::stream_format(stream, "call %03x", opcode & 0x3ff); return 1 | STEP_OVER; } },

	{ 0x0000, 0x0000, [](P) -> u32 { util::stream_format(stream, "?%04x",   opcode); return 1; } },
};

#undef P

offs_t pic1670_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, this, opcode, pc) | SUPPORTED;
	return 0;
}
