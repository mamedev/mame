// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PIC16 generic disassembler, with extended opcodes

#include "emu.h"
#include "pic16d.h"

u32 pic16_disassembler::opcode_alignment() const
{
	return 1;
}

char pic16_disassembler::fw(u16 opcode)
{
	return opcode & 0x80 ? 'f' : 'w';
}

std::string pic16_disassembler::freg(u16 opcode) const
{
	return util::string_format("f_%02x", opcode & 0x7f);
}

std::string pic16_disassembler::imm8(u16 opcode)
{
	if((opcode & 0xff) < 10)
		return util::string_format("%d", opcode & 0xff);
	else
		return util::string_format("'h'%02x", opcode & 0xff);
}

std::string pic16_disassembler::imm7(u16 opcode)
{
	if((opcode & 0x7f) < 10)
		return util::string_format("%d", opcode & 0x7f);
	else
		return util::string_format("'h'%02x", opcode & 0x7f);
}

std::string pic16_disassembler::imm6(u16 opcode)
{
	if((opcode & 0x3f) < 10)
		return util::string_format("%d", opcode & 0x3f);
	else
		return util::string_format("'h'%02x", opcode & 0x3f);
}

std::string pic16_disassembler::imm6s(u16 opcode)
{
	if(opcode & 0x20) {
		u16 a = (-opcode) & 0x1f;
		if(a < 10)
			return util::string_format("-%d", a);
		else
			return util::string_format("-%02x", a);
	} else {
		u16 a = opcode & 0x1f;
		if(a < 10)
			return util::string_format("%d", a);
		else
			return util::string_format("'h'%02x", a);
	}
}

std::string pic16_disassembler::imm5(u16 opcode)
{
	if((opcode & 0x1f) < 10)
		return util::string_format("%d", opcode & 0x1f);
	else
		return util::string_format("%02x", opcode & 0x1f);
}

std::string pic16_disassembler::rel9(u16 opcode, u16 pc)
{
	u16 off = opcode & 0x1ff;
	if(off & 0x100)
		off -= 0x200;
	return util::string_format("%04x", (pc + off + 1) & 0x7fff);
}

std::string pic16_disassembler::abs11(u16 opcode, u16 pc) const
{
	return util::string_format("%04x", (pc & 0x7800) | (opcode & 0x7ff));
}

#define P std::ostream &stream, const pic16_disassembler *d, u16 opcode, u16 pc
const pic16_disassembler::instruction pic16_disassembler::instructions[] {
	{ 0x0700, 0x3f00, [](P) -> u32 { util::stream_format(stream, "addwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x3d00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "addwfc %s, %c", d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0500, 0x3f00, [](P) -> u32 { util::stream_format(stream, "andwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x3700, 0x3f00, [](P) -> u32 { util::stream_format(stream, "asrf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x3500, 0x3f00, [](P) -> u32 { util::stream_format(stream, "lslf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x3600, 0x3f00, [](P) -> u32 { util::stream_format(stream, "lsrf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0180, 0x3f80, [](P) -> u32 { util::stream_format(stream, "clrf %s",       d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0100, 0x3ffc, [](P) -> u32 { util::stream_format(stream, "clrw"                                      ); return 1; } },
	{ 0x0900, 0x3f00, [](P) -> u32 { util::stream_format(stream, "comf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0300, 0x3f00, [](P) -> u32 { util::stream_format(stream, "decf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0a00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "incf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0400, 0x3f00, [](P) -> u32 { util::stream_format(stream, "iorwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0800, 0x3f00, [](P) -> u32 { util::stream_format(stream, "movf %s, %c",   d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0080, 0x3f80, [](P) -> u32 { util::stream_format(stream, "movwf %s",      d->freg(opcode)            ); return 1; } },
	{ 0x0d00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "rlf %s, %c",    d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0c00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "rrf %s, %c",    d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0200, 0x3f00, [](P) -> u32 { util::stream_format(stream, "subwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x3b00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "subwfb %s, %c", d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0e00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "swapf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },
	{ 0x0600, 0x3f00, [](P) -> u32 { util::stream_format(stream, "xorwf %s, %c",  d->freg(opcode), fw(opcode)); return 1; } },

	{ 0x0b00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "decfsz %s, %c", d->freg(opcode), fw(opcode)); return 1 | STEP_COND; } },
	{ 0x0f00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "incfsz %s, %c", d->freg(opcode), fw(opcode)); return 1 | STEP_COND; } },

	{ 0x1000, 0x3c00, [](P) -> u32 { util::stream_format(stream, "bcf %d, %s",   (opcode >> 7) & 7, d->freg(opcode)); return 1; } },
	{ 0x1400, 0x3c00, [](P) -> u32 { util::stream_format(stream, "bsf %d, %s",   (opcode >> 7) & 7, d->freg(opcode)); return 1; } },

	{ 0x1800, 0x3c00, [](P) -> u32 { util::stream_format(stream, "btfsc %d, %s", (opcode >> 7) & 7, d->freg(opcode)); return 1 | STEP_COND; } },
	{ 0x1c00, 0x3c00, [](P) -> u32 { util::stream_format(stream, "btfss %d, %s", (opcode >> 7) & 7, d->freg(opcode)); return 1 | STEP_COND; } },

	{ 0x3e00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "addlw %s", imm8(opcode)); return 1; } },
	{ 0x3900, 0x3f00, [](P) -> u32 { util::stream_format(stream, "andlw %s", imm8(opcode)); return 1; } },
	{ 0x3800, 0x3f00, [](P) -> u32 { util::stream_format(stream, "iorlw %s", imm8(opcode)); return 1; } },
	{ 0x0020, 0x3fe0, [](P) -> u32 { util::stream_format(stream, "movlb %s", imm5(opcode)); return 1; } },
	{ 0x3180, 0x3f80, [](P) -> u32 { util::stream_format(stream, "movlp %s", imm7(opcode)); return 1; } },
	{ 0x3000, 0x3f00, [](P) -> u32 { util::stream_format(stream, "movlw %s", imm8(opcode)); return 1; } },
	{ 0x3c00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "sublw %s", imm8(opcode)); return 1; } },
	{ 0x3a00, 0x3f00, [](P) -> u32 { util::stream_format(stream, "xorlw %s", imm8(opcode)); return 1; } },

	{ 0x3200, 0x3e00, [](P) -> u32 { util::stream_format(stream, "bra %s",  rel9(opcode,  pc)   ); return 1; } },
	{ 0x000b, 0x3fff, [](P) -> u32 { util::stream_format(stream, "brw"                          ); return 1; } },
	{ 0x2000, 0x3800, [](P) -> u32 { util::stream_format(stream, "call %s", d->abs11(opcode, pc)); return 1 | STEP_OVER; } },
	{ 0x000a, 0x3fff, [](P) -> u32 { util::stream_format(stream, "callw"                        ); return 1 | STEP_OVER; } },
	{ 0x2800, 0x3800, [](P) -> u32 { util::stream_format(stream, "goto %s", d->abs11(opcode, pc)); return 1; } },
	{ 0x0009, 0x3fff, [](P) -> u32 { util::stream_format(stream, "retfie"                       ); return 1 | STEP_OUT; } },
	{ 0x3400, 0x3f00, [](P) -> u32 { util::stream_format(stream, "retlw %s", imm8(opcode)       ); return 1 | STEP_OUT; } },
	{ 0x0008, 0x3fff, [](P) -> u32 { util::stream_format(stream, "return"                       ); return 1 | STEP_OUT; } },

	{ 0x0064, 0x3fff, [](P) -> u32 { util::stream_format(stream, "clrwdt"                    ); return 1; } },
	{ 0x0000, 0x3fff, [](P) -> u32 { util::stream_format(stream, "nop"                       ); return 1; } },
	{ 0x0062, 0x3fff, [](P) -> u32 { util::stream_format(stream, "option"                    ); return 1; } },
	{ 0x0001, 0x3fff, [](P) -> u32 { util::stream_format(stream, "reset"                     ); return 1; } },
	{ 0x0063, 0x3fff, [](P) -> u32 { util::stream_format(stream, "sleep"                     ); return 1; } },
	{ 0x0065, 0x3fff, [](P) -> u32 { util::stream_format(stream, "tris a"                    ); return 1; } },
	{ 0x0066, 0x3fff, [](P) -> u32 { util::stream_format(stream, "tris b"                    ); return 1; } },
	{ 0x0067, 0x3fff, [](P) -> u32 { util::stream_format(stream, "tris c"                    ); return 1; } },

	{ 0x3100, 0x3fc0, [](P) -> u32 { util::stream_format(stream, "addfsr fsr0, %s", imm6(opcode)); return 1; } },
	{ 0x3140, 0x3fc0, [](P) -> u32 { util::stream_format(stream, "addfsr fsr1, %s", imm6(opcode)); return 1; } },
	{ 0x0010, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw ++fsr0"                 ); return 1; } },
	{ 0x0011, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw --fsr0"                 ); return 1; } },
	{ 0x0012, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw fsr0++"                 ); return 1; } },
	{ 0x0013, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw fsr0--"                 ); return 1; } },
	{ 0x0014, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw ++fsr1"                 ); return 1; } },
	{ 0x0015, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw --fsr1"                 ); return 1; } },
	{ 0x0016, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw fsr1++"                 ); return 1; } },
	{ 0x0017, 0xffff, [](P) -> u32 { util::stream_format(stream, "moviw fsr1--"                 ); return 1; } },
	{ 0x3f00, 0xffc0, [](P) -> u32 { util::stream_format(stream, "moviw %s[fsr0]", imm6s(opcode)); return 1; } },
	{ 0x3f40, 0xffc0, [](P) -> u32 { util::stream_format(stream, "moviw %s[fsr1]", imm6s(opcode)); return 1; } },
	{ 0x0018, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi ++fsr0"                 ); return 1; } },
	{ 0x0019, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi --fsr0"                 ); return 1; } },
	{ 0x001a, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi fsr0++"                 ); return 1; } },
	{ 0x001b, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi fsr0--"                 ); return 1; } },
	{ 0x001c, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi ++fsr1"                 ); return 1; } },
	{ 0x001d, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi --fsr1"                 ); return 1; } },
	{ 0x001e, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi fsr1++"                 ); return 1; } },
	{ 0x001f, 0xffff, [](P) -> u32 { util::stream_format(stream, "movwi fsr1--"                 ); return 1; } },
	{ 0x3f80, 0xffc0, [](P) -> u32 { util::stream_format(stream, "movwi %s[fsr0]", imm6s(opcode)); return 1; } },
	{ 0x3fc0, 0xffc0, [](P) -> u32 { util::stream_format(stream, "movwi %s[fsr1]", imm6s(opcode)); return 1; } },

	{ 0x0000, 0x0000, [](P) -> u32 { util::stream_format(stream, "?%04x",   opcode); return 1; } },
};

#undef P

offs_t pic16_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, this, opcode, pc) | SUPPORTED;
	return 0;
}
