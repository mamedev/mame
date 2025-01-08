// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// upd177X disassembler

#include "emu.h"
#include "upd177xd.h"

u32 upd177x_disassembler::opcode_alignment() const
{
	return 1;
}

std::string upd177x_disassembler::reg4(u16 opcode)
{
	return util::string_format("r%02x", (opcode >> 4) & 0x1f);
}

std::string upd177x_disassembler::reg8(u16 opcode)
{
	return util::string_format("r%02x", (opcode >> 8) & 0x1f);
}

std::string upd177x_disassembler::imm8(u16 opcode)
{
	if((opcode & 0xff) < 10)
		return util::string_format("%d", opcode & 0xff);
	else
		return util::string_format("%02x", opcode & 0xff);
}

std::string upd177x_disassembler::imm7(u16 opcode)
{
	if((opcode & 0x7f) < 10)
		return util::string_format("%d", opcode & 0x7f);
	else
		return util::string_format("%02x", opcode & 0x7f);
}

std::string upd177x_disassembler::imm6(u16 opcode)
{
	if((opcode & 0x3f) < 10)
		return util::string_format("%d", opcode & 0x3f);
	else
		return util::string_format("%02x", opcode & 0x3f);
}

std::string upd177x_disassembler::imm4(u16 opcode)
{
	return util::string_format("%x", opcode & 0xf);
}

std::string upd177x_disassembler::imm3_5(u16 opcode)
{
	return util::string_format("%02x", opcode & 0xe0);
}

std::string upd177x_disassembler::abs12(u16 opcode, u16 pc)
{
	return util::string_format("%04x", (pc & 0xf000) | (opcode & 0xfff));
}

std::string upd177x_disassembler::abs8(u16 opcode, u16 pc)
{
	return util::string_format("%04x", (pc & 0xff00) | (opcode & 0xff));
}

std::string upd177x_disassembler::abs4_4(u16 opcode, u16 pc)
{
	return util::string_format("%04x", (pc & 0x0fff) | ((opcode << 8) & 0xf000));
}

#define P std::ostream &stream, u16 opcode, u16 pc
const upd177x_disassembler::instruction upd177x_disassembler::instructions[] {
	{ 0x4000, 0xe000, [](P) -> u32 { util::stream_format(stream, "mvi %s, %s", reg8(opcode), imm8(opcode)); return 1; } },
	{ 0x3200, 0xff00, [](P) -> u32 { util::stream_format(stream, "mvi (h), %s", imm8(opcode)); return 1; } },
	{ 0x3400, 0xff00, [](P) -> u32 { util::stream_format(stream, "mvi a, %s", imm8(opcode)); return 1; } },
	{ 0x3800, 0xffc0, [](P) -> u32 { util::stream_format(stream, "mvi h, %s", imm6(opcode)); return 1; } },
	{ 0x3100, 0xff1f, [](P) -> u32 { util::stream_format(stream, "mvi md1, %s", imm3_5(opcode)); return 1; } },
	{ 0x2100, 0xff80, [](P) -> u32 { util::stream_format(stream, "mvi md0, %s", imm7(opcode)); return 1; } },

	{ 0x1201, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "mov %s, a", reg4(opcode)); return 1; } },
	{ 0x1005, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "mov a, %s", reg4(opcode)); return 1; } },
	{ 0x1205, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "xchg %s, a", reg4(opcode)); return 1; } },
	{ 0x1202, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "mov %s, h", reg4(opcode)); return 1; } },
	{ 0x100a, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "mov h, %s", reg4(opcode)); return 1; } },
	{ 0x120a, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "xchg %s, a", reg4(opcode)); return 1; } },
	{ 0x1000, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "mov y, %s", reg4(opcode)); return 1; } },
	{ 0x0008, 0xffff, [](P) -> u32 { util::stream_format(stream, "mov x, rg"); return 1; } },

	{ 0x0404, 0xffff, [](P) -> u32 { util::stream_format(stream, "rar"); return 1; } },
	{ 0x0408, 0xffff, [](P) -> u32 { util::stream_format(stream, "ral"); return 1; } },

	{ 0x0401, 0xffff, [](P) -> u32 { util::stream_format(stream, "in pa"); return 1; } },
	{ 0x0402, 0xffff, [](P) -> u32 { util::stream_format(stream, "in pb"); return 1; } },
	{ 0x0002, 0xffff, [](P) -> u32 { util::stream_format(stream, "out pa"); return 1; } },
	{ 0x0004, 0xffff, [](P) -> u32 { util::stream_format(stream, "out pb"); return 1; } },
	{ 0x0502, 0xffff, [](P) -> u32 { util::stream_format(stream, "out da"); return 1; } },

	{ 0x0504, 0xffff, [](P) -> u32 { util::stream_format(stream, "mul 1"); return 1; } },
	{ 0x050c, 0xffff, [](P) -> u32 { util::stream_format(stream, "mul 2"); return 1; } },

	{ 0x1409, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "mix %s", reg4(opcode)); return 1; } },

	{ 0x1801, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tblo a, (%s)", reg4(opcode)); return 1; } },
	{ 0x1802, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tblo x, (%s)", reg4(opcode)); return 1; } },
	{ 0x1804, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tblo y, (%s)", reg4(opcode)); return 1; } },
	{ 0x1808, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "callo (%s)", reg4(opcode)); return 1 | STEP_OVER; } },

	{ 0x6000, 0xf000, [](P) -> u32 { util::stream_format(stream, "jmp %s", abs12(opcode, pc)); return 1; } },
	{ 0x2000, 0xff0f, [](P) -> u32 { util::stream_format(stream, "jpp %s", abs4_4(opcode, pc)); return 1; } },
	{ 0x0501, 0xffff, [](P) -> u32 { util::stream_format(stream, "jmpa"); return 1; } },
	{ 0x2400, 0xff00, [](P) -> u32 { util::stream_format(stream, "jmpfz %s", abs8(opcode, pc)); return 1; } },

	{ 0x7000, 0xf000, [](P) -> u32 { util::stream_format(stream, "call %s", abs12(opcode, pc)); return 1 | STEP_OVER; } },

	{ 0x0800, 0xffff, [](P) -> u32 { util::stream_format(stream, "ret"); return 1 | STEP_OUT; } },
	{ 0x0801, 0xffff, [](P) -> u32 { util::stream_format(stream, "rets"); return 1 | STEP_OUT; } },
	{ 0x090f, 0xffff, [](P) -> u32 { util::stream_format(stream, "reti"); return 1 | STEP_OUT; } },

	{ 0x0005, 0xffff, [](P) -> u32 { util::stream_format(stream, "stf"); return 1; } },
	{ 0x0602, 0xffff, [](P) -> u32 { util::stream_format(stream, "off"); return 1; } },
	{ 0x0000, 0xffff, [](P) -> u32 { util::stream_format(stream, "nop"); return 1; } },

	{ 0xe000, 0xfe00, [](P) -> u32 { util::stream_format(stream, "adi %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xe200, 0xfe00, [](P) -> u32 { util::stream_format(stream, "adis %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xe400, 0xfe00, [](P) -> u32 { util::stream_format(stream, "sbi %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xe600, 0xfe00, [](P) -> u32 { util::stream_format(stream, "sbis %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xe800, 0xfe00, [](P) -> u32 { util::stream_format(stream, "tadinc %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xea00, 0xfe00, [](P) -> u32 { util::stream_format(stream, "tadic %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xec00, 0xfe00, [](P) -> u32 { util::stream_format(stream, "tsbinc %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xee00, 0xfe00, [](P) -> u32 { util::stream_format(stream, "tsbic %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xf000, 0xfe00, [](P) -> u32 { util::stream_format(stream, "adi5 %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xf200, 0xfe00, [](P) -> u32 { util::stream_format(stream, "adims %s, %s", reg4(opcode), imm4(opcode)); return 1; } },
	{ 0xf800, 0xfe00, [](P) -> u32 { util::stream_format(stream, "tadi5 %s, %s", reg4(opcode), imm4(opcode)); return 1; } },

	{ 0x8000, 0xff00, [](P) -> u32 { util::stream_format(stream, "adi a, %s", imm8(opcode)); return 1; } },
	{ 0x8200, 0xff00, [](P) -> u32 { util::stream_format(stream, "andi a, %s", imm8(opcode)); return 1; } },
	{ 0x8400, 0xff00, [](P) -> u32 { util::stream_format(stream, "sbi a, %s", imm8(opcode)); return 1; } },
	{ 0x8600, 0xff00, [](P) -> u32 { util::stream_format(stream, "ori a, %s", imm8(opcode)); return 1; } },
	{ 0x8800, 0xff00, [](P) -> u32 { util::stream_format(stream, "adis a, %s", imm8(opcode)); return 1; } },
	{ 0x8a00, 0xff00, [](P) -> u32 { util::stream_format(stream, "andis a, %s", imm8(opcode)); return 1; } },
	{ 0x8c00, 0xff00, [](P) -> u32 { util::stream_format(stream, "sbis a, %s", imm8(opcode)); return 1; } },
	{ 0x8e00, 0xff00, [](P) -> u32 { util::stream_format(stream, "xori a, %s", imm8(opcode)); return 1; } },
	{ 0x9000, 0xff00, [](P) -> u32 { util::stream_format(stream, "tadinc a, %s", imm8(opcode)); return 1; } },
	{ 0x9200, 0xff00, [](P) -> u32 { util::stream_format(stream, "tandinz a, %s", imm8(opcode)); return 1; } },
	{ 0x9400, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbinc a, %s", imm8(opcode)); return 1; } },
	{ 0x9600, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbinz a, %s", imm8(opcode)); return 1; } },
	{ 0x9800, 0xff00, [](P) -> u32 { util::stream_format(stream, "tadic a, %s", imm8(opcode)); return 1; } },
	{ 0x9a00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tandiz a, %s", imm8(opcode)); return 1; } },
	{ 0x9c00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbic a, %s", imm8(opcode)); return 1; } },
	{ 0x9e00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbiz a, %s", imm8(opcode)); return 1; } },

	{ 0x8100, 0xff1f, [](P) -> u32 { util::stream_format(stream, "adi mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8300, 0xff1f, [](P) -> u32 { util::stream_format(stream, "andi mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8500, 0xff1f, [](P) -> u32 { util::stream_format(stream, "sbi mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8700, 0xff1f, [](P) -> u32 { util::stream_format(stream, "ori mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8900, 0xff1f, [](P) -> u32 { util::stream_format(stream, "adis mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8b00, 0xff1f, [](P) -> u32 { util::stream_format(stream, "andis mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8d00, 0xff1f, [](P) -> u32 { util::stream_format(stream, "sbis mdi, %s", imm8(opcode)); return 1; } },
	{ 0x8f00, 0xff1f, [](P) -> u32 { util::stream_format(stream, "xori mdi, %s", imm8(opcode)); return 1; } },
	{ 0x9100, 0xff00, [](P) -> u32 { util::stream_format(stream, "tadinc md, %s", imm8(opcode)); return 1; } },
	{ 0x9300, 0xff00, [](P) -> u32 { util::stream_format(stream, "tandinz md, %s", imm8(opcode)); return 1; } },
	{ 0x9500, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbinc md, %s", imm8(opcode)); return 1; } },
	{ 0x9700, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbinz md, %s", imm8(opcode)); return 1; } },
	{ 0x9900, 0xff00, [](P) -> u32 { util::stream_format(stream, "tadic md, %s", imm8(opcode)); return 1; } },
	{ 0x9b00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tandiz md, %s", imm8(opcode)); return 1; } },
	{ 0x9d00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbic md, %s", imm8(opcode)); return 1; } },
	{ 0x9f00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbiz md, %s", imm8(opcode)); return 1; } },

	{ 0xa000, 0xff00, [](P) -> u32 { util::stream_format(stream, "adi (h), %s", imm8(opcode)); return 1; } },
	{ 0xa200, 0xff00, [](P) -> u32 { util::stream_format(stream, "andi (h), %s", imm8(opcode)); return 1; } },
	{ 0xa400, 0xff00, [](P) -> u32 { util::stream_format(stream, "sbi (h), %s", imm8(opcode)); return 1; } },
	{ 0xa600, 0xff00, [](P) -> u32 { util::stream_format(stream, "ori (h), %s", imm8(opcode)); return 1; } },
	{ 0xa800, 0xff00, [](P) -> u32 { util::stream_format(stream, "adis (h), %s", imm8(opcode)); return 1; } },
	{ 0xaa00, 0xff00, [](P) -> u32 { util::stream_format(stream, "andis (h), %s", imm8(opcode)); return 1; } },
	{ 0xac00, 0xff00, [](P) -> u32 { util::stream_format(stream, "sbis (h), %s", imm8(opcode)); return 1; } },
	{ 0xae00, 0xff00, [](P) -> u32 { util::stream_format(stream, "xori (h), %s", imm8(opcode)); return 1; } },
	{ 0xb000, 0xff00, [](P) -> u32 { util::stream_format(stream, "tadinc (h), %s", imm8(opcode)); return 1; } },
	{ 0xb200, 0xff00, [](P) -> u32 { util::stream_format(stream, "tandinz (h), %s", imm8(opcode)); return 1; } },
	{ 0xb400, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbinc (h), %s", imm8(opcode)); return 1; } },
	{ 0xb600, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbinz (h), %s", imm8(opcode)); return 1; } },
	{ 0xb800, 0xff00, [](P) -> u32 { util::stream_format(stream, "tadic (h), %s", imm8(opcode)); return 1; } },
	{ 0xba00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tandiz (h), %s", imm8(opcode)); return 1; } },
	{ 0xbc00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbic (h), %s", imm8(opcode)); return 1; } },
	{ 0xbe00, 0xff00, [](P) -> u32 { util::stream_format(stream, "tsbiz (h), %s", imm8(opcode)); return 1; } },

	{ 0xa100, 0xffc0, [](P) -> u32 { util::stream_format(stream, "adi h, %s", imm6(opcode)); return 1; } },
	{ 0xa300, 0xffc0, [](P) -> u32 { util::stream_format(stream, "andi h, %s", imm6(opcode)); return 1; } },
	{ 0xa500, 0xffc0, [](P) -> u32 { util::stream_format(stream, "sbi h, %s", imm6(opcode)); return 1; } },
	{ 0xa700, 0xffc0, [](P) -> u32 { util::stream_format(stream, "ori h, %s", imm6(opcode)); return 1; } },
	{ 0xa900, 0xffc0, [](P) -> u32 { util::stream_format(stream, "adis h, %s", imm6(opcode)); return 1; } },
	{ 0xabc0, 0xffc0, [](P) -> u32 { util::stream_format(stream, "andis h, %s", imm6(opcode)); return 1; } },
	{ 0xad00, 0xffc0, [](P) -> u32 { util::stream_format(stream, "sbis h, %s", imm6(opcode)); return 1; } },
	{ 0xaf00, 0xffc0, [](P) -> u32 { util::stream_format(stream, "xori h, %s", imm6(opcode)); return 1; } },
	{ 0xb1c0, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tadinc h, %s", imm6(opcode)); return 1; } },
	{ 0xb300, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tandinz h, %s", imm6(opcode)); return 1; } },
	{ 0xb500, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tsbinc h, %s", imm6(opcode)); return 1; } },
	{ 0xb700, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tsbinz h, %s", imm6(opcode)); return 1; } },
	{ 0xb9c0, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tadic h, %s", imm6(opcode)); return 1; } },
	{ 0xbb00, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tandiz h, %s", imm6(opcode)); return 1; } },
	{ 0xbd00, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tsbic h, %s", imm6(opcode)); return 1; } },
	{ 0xbf00, 0xffc0, [](P) -> u32 { util::stream_format(stream, "tsbiz h, %s", imm6(opcode)); return 1; } },

	{ 0x0201, 0xffff, [](P) -> u32 { util::stream_format(stream, "mov n, a"); return 1; } },
	{ 0x0208, 0xffff, [](P) -> u32 { util::stream_format(stream, "mov x, a"); return 1; } },
	{ 0x1601, 0xffff, [](P) -> u32 { util::stream_format(stream, "mov (h), a"); return 1; } },
	{ 0x1405, 0xffff, [](P) -> u32 { util::stream_format(stream, "mov a, (h)"); return 1; } },
	{ 0x1605, 0xffff, [](P) -> u32 { util::stream_format(stream, "xchg (h), a"); return 1; } },

	{ 0xc000, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "ad a, %s", reg4(opcode)); return 1; } },
	{ 0xc200, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "and a, %s", reg4(opcode)); return 1; } },
	{ 0xc400, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "sb a, %s", reg4(opcode)); return 1; } },
	{ 0xc600, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "aor a, %s", reg4(opcode)); return 1; } },
	{ 0xc800, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "ads a, %s", reg4(opcode)); return 1; } },
	{ 0xca00, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "ands a, %s", reg4(opcode)); return 1; } },
	{ 0xcc00, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "sbs a, %s", reg4(opcode)); return 1; } },
	{ 0xce00, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "xor a, %s", reg4(opcode)); return 1; } },
	{ 0xd000, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tadnc a, %s", reg4(opcode)); return 1; } },
	{ 0xd200, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tandnz a, %s", reg4(opcode)); return 1; } },
	{ 0xd400, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbnc a, %s", reg4(opcode)); return 1; } },
	{ 0xd600, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbnz a, %s", reg4(opcode)); return 1; } },
	{ 0xd800, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tadc a, %s", reg4(opcode)); return 1; } },
	{ 0xda00, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tandz a, %s", reg4(opcode)); return 1; } },
	{ 0xdc00, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbc a, %s", reg4(opcode)); return 1; } },
	{ 0xde00, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbz a, %s", reg4(opcode)); return 1; } },

	{ 0xc008, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "ad %s, a", reg4(opcode)); return 1; } },
	{ 0xc208, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "and %s, a", reg4(opcode)); return 1; } },
	{ 0xc408, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "sb %s, a", reg4(opcode)); return 1; } },
	{ 0xc608, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "aor %s, a", reg4(opcode)); return 1; } },
	{ 0xc808, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "ads %s, a", reg4(opcode)); return 1; } },
	{ 0xca08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "ands %s, a", reg4(opcode)); return 1; } },
	{ 0xcc08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "sbs %s, a", reg4(opcode)); return 1; } },
	{ 0xce08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "xor %s, a", reg4(opcode)); return 1; } },
	{ 0xd008, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tadnc %s, a", reg4(opcode)); return 1; } },
	{ 0xd208, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tandnz %s, a", reg4(opcode)); return 1; } },
	{ 0xd408, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbnc %s, a", reg4(opcode)); return 1; } },
	{ 0xd608, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbnz %s, a", reg4(opcode)); return 1; } },
	{ 0xd808, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tadc %s, a", reg4(opcode)); return 1; } },
	{ 0xda08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tandz %s, a", reg4(opcode)); return 1; } },
	{ 0xdc08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbc %s, a", reg4(opcode)); return 1; } },
	{ 0xde08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tsbz %s, a", reg4(opcode)); return 1; } },

	{ 0xc001, 0xffff, [](P) -> u32 { util::stream_format(stream, "ad a, (h)"); return 1; } },
	{ 0xc201, 0xffff, [](P) -> u32 { util::stream_format(stream, "and a, (h)"); return 1; } },
	{ 0xc401, 0xffff, [](P) -> u32 { util::stream_format(stream, "sb a, (h)"); return 1; } },
	{ 0xc601, 0xffff, [](P) -> u32 { util::stream_format(stream, "aor a, (h)"); return 1; } },
	{ 0xc801, 0xffff, [](P) -> u32 { util::stream_format(stream, "ads a, (h)"); return 1; } },
	{ 0xca01, 0xffff, [](P) -> u32 { util::stream_format(stream, "ands a, (h)"); return 1; } },
	{ 0xcc01, 0xffff, [](P) -> u32 { util::stream_format(stream, "sbs a, (h)"); return 1; } },
	{ 0xce01, 0xffff, [](P) -> u32 { util::stream_format(stream, "xor a, (h)"); return 1; } },
	{ 0xd001, 0xffff, [](P) -> u32 { util::stream_format(stream, "tadnc a, (h)"); return 1; } },
	{ 0xd201, 0xffff, [](P) -> u32 { util::stream_format(stream, "tandnz a, (h)"); return 1; } },
	{ 0xd401, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbnc a, (h)"); return 1; } },
	{ 0xd601, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbnz a, (h)"); return 1; } },
	{ 0xd801, 0xffff, [](P) -> u32 { util::stream_format(stream, "tadc a, (h)"); return 1; } },
	{ 0xda01, 0xffff, [](P) -> u32 { util::stream_format(stream, "tandz a, (h)"); return 1; } },
	{ 0xdc01, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbc a, (h)"); return 1; } },
	{ 0xde01, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbz a, (h)"); return 1; } },

	{ 0xc009, 0xffff, [](P) -> u32 { util::stream_format(stream, "ad (h), a"); return 1; } },
	{ 0xc209, 0xffff, [](P) -> u32 { util::stream_format(stream, "and (h), a"); return 1; } },
	{ 0xc409, 0xffff, [](P) -> u32 { util::stream_format(stream, "sb (h), a"); return 1; } },
	{ 0xc609, 0xffff, [](P) -> u32 { util::stream_format(stream, "aor (h), a"); return 1; } },
	{ 0xc809, 0xffff, [](P) -> u32 { util::stream_format(stream, "ads (h), a"); return 1; } },
	{ 0xca09, 0xffff, [](P) -> u32 { util::stream_format(stream, "ands (h), a"); return 1; } },
	{ 0xcc09, 0xffff, [](P) -> u32 { util::stream_format(stream, "sbs (h), a"); return 1; } },
	{ 0xce09, 0xffff, [](P) -> u32 { util::stream_format(stream, "xor (h), a"); return 1; } },
	{ 0xd009, 0xffff, [](P) -> u32 { util::stream_format(stream, "tadnc (h), a"); return 1; } },
	{ 0xd209, 0xffff, [](P) -> u32 { util::stream_format(stream, "tandnz (h), a"); return 1; } },
	{ 0xd409, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbnc (h), a"); return 1; } },
	{ 0xd609, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbnz (h), a"); return 1; } },
	{ 0xd809, 0xffff, [](P) -> u32 { util::stream_format(stream, "tadc (h), a"); return 1; } },
	{ 0xda09, 0xffff, [](P) -> u32 { util::stream_format(stream, "tandz (h), a"); return 1; } },
	{ 0xdc09, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbc (h), a"); return 1; } },
	{ 0xde09, 0xffff, [](P) -> u32 { util::stream_format(stream, "tsbz (h), a"); return 1; } },

	{ 0x1a01, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tbli a, (%s)", reg4(opcode)); return 1; } },
	{ 0x1a02, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tbli x, (%s)", reg4(opcode)); return 1; } },
	{ 0x1a04, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "tbli y, (%s)", reg4(opcode)); return 1; } },
	{ 0x1a08, 0xfe0f, [](P) -> u32 { util::stream_format(stream, "calli (%s)", reg4(opcode)); return 1 | STEP_OVER; } },

	{ 0x0101, 0xffff, [](P) -> u32 { util::stream_format(stream, "mon"); return 1; } },

	{ 0x0000, 0x0000, [](P) -> u32 { util::stream_format(stream, "?%04x",   opcode); return 1; } },
};

#undef P

offs_t upd177x_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, opcode, pc) | SUPPORTED;
	return 0;
}
