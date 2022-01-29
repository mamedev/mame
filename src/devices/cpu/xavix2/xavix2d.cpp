// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Nathan Gilbert

// Xavix2 disassembler

#include "emu.h"
#include "xavix2d.h"

u32 xavix2_disassembler::opcode_alignment() const
{
	return 1;
}

const u8 xavix2_disassembler::bpo[8] = { 4, 3, 3, 2, 2, 2, 2, 1 };

const char *const xavix2_disassembler::reg_names[8] = { "r0", "r1", "r2", "r3", "r4", "r5", "sp", "lnk" };

const char *xavix2_disassembler::r1()
{
	return reg_names[(m_opcode >> 22) & 7];
}

const char *xavix2_disassembler::r2()
{
	return reg_names[(m_opcode >> 19) & 7];
}

const char *xavix2_disassembler::r3()
{
	return reg_names[(m_opcode >> 16) & 7];
}

std::string xavix2_disassembler::val22h()
{
	return util::string_format("%08x", u32(m_opcode << 10));
}

std::string xavix2_disassembler::val22s()
{
	u32 r = m_opcode & 0x3fffff;
	if(m_opcode & 0x200000)
		return util::string_format("-%06x", 0x400000 - r);
	else
		return util::string_format("%06x", r);
}

std::string xavix2_disassembler::val19s()
{
	u32 r = m_opcode & 0x7ffff;
	if(m_opcode & 0x40000)
		return util::string_format("-%06x", 0x80000 - r);
	else
		return util::string_format("%06x", r);
}

std::string xavix2_disassembler::val19u()
{
	return util::string_format("%05x", m_opcode & 0x7ffff);
}

std::string xavix2_disassembler::val14h()
{
	return util::string_format("%08x", ((m_opcode >> 8) & 0x3fff) << 18);
}

std::string xavix2_disassembler::val14u()
{
	return util::string_format("%04x", (m_opcode >> 8) & 0x3fff);
}

std::string xavix2_disassembler::val14s()
{
	u16 r = (m_opcode >> 8) & 0x3fff;
	if(r & 0x2000)
		return util::string_format("-%04x", 0x4000 - r);
	else
		return util::string_format("%04x", r);
}

std::string xavix2_disassembler::val14sa()
{
	u16 r = (m_opcode >> 8) & 0x3fff;
	if(r & 0x2000)
		return util::string_format("%08x", r - 0x4000);
	else
		return util::string_format("%04x", r);
}

std::string xavix2_disassembler::val11s()
{
	u16 r = (m_opcode >> 8) & 0x7ff;
	if(r & 0x400)
		return util::string_format("-%03x", 0x800 - r);
	else
		return util::string_format("%03x", r);
}

std::string xavix2_disassembler::val11u()
{
	return util::string_format("%03x", (m_opcode >> 8) & 0x7ff);
}

std::string xavix2_disassembler::val6u()
{
	return util::string_format("%02x", (m_opcode >> 16) & 0x3f);
}

std::string xavix2_disassembler::val6s()
{
	u16 r = (m_opcode >> 16) & 0x3f;
	if(r & 0x20)
		return util::string_format("-%02x", 0x40 - r);
	else
		return util::string_format("%02x", r);
}

std::string xavix2_disassembler::val3u()
{
	return util::string_format("%x", (m_opcode >> 16) & 0x7);
}

std::string xavix2_disassembler::off19s()
{
	u32 r = m_opcode & 0x7ffff;
	if(r & 0x40000)
		return util::string_format(" - %05x", 0x80000 - r);
	else if(r)
		return util::string_format(" + %05x", r);
	else
		return "";
}

std::string xavix2_disassembler::off14s()
{
	u16 r = (m_opcode >> 8) & 0x3fff;
	if(r & 0x2000)
		return util::string_format(" - %04x", 0x4000 - r);
	else if(r)
		return util::string_format(" + %04x", r);
	else
		return "";
}

std::string xavix2_disassembler::off11s()
{
	u16 r = (m_opcode >> 8) & 0x7ff;
	if(r & 0x400)
		return util::string_format(" - %03x", 0x800 - r);
	else if(r)
		return util::string_format(" + %03x", r);
	else
		return "";
}

std::string xavix2_disassembler::off6s()
{
	u16 r = (m_opcode >> 16) & 0x3f;
	if(r & 0x20)
		return util::string_format(" - %02x", 0x40 - r);
	else if(r)
		return util::string_format(" + %02x", r);
	else
		return "";
}

std::string xavix2_disassembler::off3s()
{
	u16 r = (m_opcode >> 16) & 0x7;
	if(r & 0x4)
		return util::string_format(" - %x", 8 - r);
	else if(r)
		return util::string_format(" + %x", r);
	else
		return "";
}

std::string xavix2_disassembler::adr24()
{
	return util::string_format("%06x", m_opcode & 0xffffff);
}

std::string xavix2_disassembler::adr16()
{
	return util::string_format("%06x", (m_pc & 0xffff0000) | ((m_opcode >> 8) & 0xffff));
}

std::string xavix2_disassembler::rel16()
{
	return util::string_format("%06x", m_pc + static_cast<s16>(m_opcode >> 8));
}

std::string xavix2_disassembler::rel8()
{
	return util::string_format("%06x", m_pc + static_cast<s8>(m_opcode >> 16));
}

offs_t xavix2_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	m_pc = pc;
	m_opcode = opcodes.r8(m_pc) << 24;
	u8 nb = bpo[m_opcode >> 29];
	for(u8 i=1; i != nb; i++)
		m_opcode |= opcodes.r8(m_pc + i) << (24 - 8*i);

	u32 flags = 0;
	switch(m_opcode >> 24) {
	case 0x00: case 0x01: util::stream_format(stream, "%s = %s + %s", r1(), r2(), val19s()); break;
	case 0x02: case 0x03: util::stream_format(stream, "%s = %s", r1(), val22h()); break;
	case 0x04: case 0x05: util::stream_format(stream, "%s = %s - %s", r1(), r2(), val19s()); break;
	case 0x06: case 0x07: util::stream_format(stream, "%s = %s", r1(), val22s()); break;
	case 0x08:            util::stream_format(stream, "jmp %s", adr24()); break;
	case 0x09:            util::stream_format(stream, "jsr %s", adr24()); flags = STEP_OVER; break;
	case 0x0a: case 0x0b: util::stream_format(stream, "%s = %s & %s", r1(), r2(), val19u()); break;
	case 0x0c: case 0x0d: util::stream_format(stream, "%s = %s | %s", r1(), r2(), val19u()); break;
	case 0x0e: case 0x0f: util::stream_format(stream, "%s = %s ^ %s", r1(), r2(), val19u()); break;

	case 0x10: case 0x11: util::stream_format(stream, "%s = (%s%s).bs", r1(), r2(), off19s()); break;
	case 0x12: case 0x13: util::stream_format(stream, "%s = (%s%s).bu", r1(), r2(), off19s()); break;
	case 0x14: case 0x15: util::stream_format(stream, "%s = (%s%s).ws", r1(), r2(), off19s()); break;
	case 0x16: case 0x17: util::stream_format(stream, "%s = (%s%s).wu", r1(), r2(), off19s()); break;
	case 0x18: case 0x19: util::stream_format(stream, "%s = (%s%s).l", r1(), r2(), off19s()); break;
	case 0x1a: case 0x1b: util::stream_format(stream, "(%s%s).b = %s", r2(), off19s(), r1()); break;
	case 0x1c: case 0x1d: util::stream_format(stream, "(%s%s).w = %s", r2(), off19s(), r1()); break;
	case 0x1e: case 0x1f: util::stream_format(stream, "(%s%s).l = %s", r2(), off19s(), r1()); break;

	case 0x20: case 0x21: util::stream_format(stream, "%s = %s + %s", r1(), r2(), val11s()); break;
	case 0x22: case 0x23: util::stream_format(stream, "%s = %s", r1(), val14h()); break;
	case 0x24: case 0x25: util::stream_format(stream, "%s = %s - %s", r1(), r2(), val11s()); break;
	case 0x26: case 0x27: util::stream_format(stream, "cmp %s, %s", r1(), val14s()); break;
	case 0x28:            util::stream_format(stream, "bra %s", rel16()); break;
	case 0x29:            util::stream_format(stream, "bsr %s", rel16()); flags = STEP_OVER; break;
	case 0x2a: case 0x2b: util::stream_format(stream, "%s = %s & %s", r1(), r2(), val11u()); break;
	case 0x2c: case 0x2d: util::stream_format(stream, "%s = %s | %s", r1(), r2(), val11u()); break;
	case 0x2e: case 0x2f: util::stream_format(stream, "%s = %s ^ %s", r1(), r2(), val11u()); break;

	case 0x30: case 0x31: util::stream_format(stream, "%s = (sp%s).bs", r1(), off14s()); break;
	case 0x32: case 0x33: util::stream_format(stream, "%s = (sp%s).bu", r1(), off14s()); break;
	case 0x34: case 0x35: util::stream_format(stream, "%s = (sp%s).ws", r1(), off14s()); break;
	case 0x36: case 0x37: util::stream_format(stream, "%s = (sp%s).wu", r1(), off14s()); break;
	case 0x38: case 0x39: util::stream_format(stream, "%s = (sp%s).l", r1(), off14s()); break;
	case 0x3a: case 0x3b: util::stream_format(stream, "(sp%s).b = %s", off14s(), r1()); break;
	case 0x3c: case 0x3d: util::stream_format(stream, "(sp%s).w = %s", off14s(), r1()); break;
	case 0x3e: case 0x3f: util::stream_format(stream, "(sp%s).l = %s", off14s(), r1()); break;

	case 0x40: case 0x41: util::stream_format(stream, "%s = (%s%s).bs", r1(), r2(), off11s()); break;
	case 0x42: case 0x43: util::stream_format(stream, "%s = (%s%s).bu", r1(), r2(), off11s()); break;
	case 0x44: case 0x45: util::stream_format(stream, "%s = (%s%s).ws", r1(), r2(), off11s()); break;
	case 0x46: case 0x47: util::stream_format(stream, "%s = (%s%s).wu", r1(), r2(), off11s()); break;
	case 0x48: case 0x49: util::stream_format(stream, "%s = (%s%s).l", r1(), r2(), off11s()); break;
	case 0x4a: case 0x4b: util::stream_format(stream, "(%s%s).b = %s", r2(), off11s(), r1()); break;
	case 0x4c: case 0x4d: util::stream_format(stream, "(%s%s).w = %s", r2(), off11s(), r1()); break;
	case 0x4e: case 0x4f: util::stream_format(stream, "(%s%s).l = %s", r2(), off11s(), r1()); break;

	case 0x50: case 0x51: util::stream_format(stream, "%s = %s.bs", r1(), val14sa()); break;
	case 0x52: case 0x53: util::stream_format(stream, "%s = %s.bu", r1(), val14sa()); break;
	case 0x54: case 0x55: util::stream_format(stream, "%s = %s.ws", r1(), val14sa()); break;
	case 0x56: case 0x57: util::stream_format(stream, "%s = %s.wu", r1(), val14sa()); break;
	case 0x58: case 0x59: util::stream_format(stream, "%s = %s.l", r1(), val14sa()); break;
	case 0x5a: case 0x5b: util::stream_format(stream, "%s.b = %s", val14sa(), r1()); break;
	case 0x5c: case 0x5d: util::stream_format(stream, "%s.w = %s", val14sa(), r1()); break;
	case 0x5e: case 0x5f: util::stream_format(stream, "%s.l = %s", val14sa(), r1()); break;

	case 0x60: case 0x61: util::stream_format(stream, "%s += %s", r1(), val6s()); break;
	case 0x62: case 0x63: util::stream_format(stream, "%s = %s", r1(), val6s()); break;
	case 0x64: case 0x65: util::stream_format(stream, "%s -= %s", r1(), val6s()); break;
	case 0x66: case 0x67: util::stream_format(stream, "cmp %s, %s", r1(), val6s()); break;
		// 68-69
	case 0x6a: case 0x6b: util::stream_format(stream, "%s = %s >>s %s", r1(), r2(), val3u()); break;
	case 0x6c: case 0x6d: util::stream_format(stream, "%s = %s >> %s", r1(), r2(), val3u()); break;
	case 0x6e: case 0x6f: util::stream_format(stream, "%s = %s << %s", r1(), r2(), val3u()); break;

	case 0x70: case 0x71: util::stream_format(stream, "%s = (sp%s).bs", r1(), off6s()); break;
	case 0x72: case 0x73: util::stream_format(stream, "%s = (sp%s).bu", r1(), off6s()); break;
	case 0x74: case 0x75: util::stream_format(stream, "%s = (sp%s).ws", r1(), off6s()); break;
	case 0x76: case 0x77: util::stream_format(stream, "%s = (sp%s).wu", r1(), off6s()); break;
	case 0x78: case 0x79: util::stream_format(stream, "%s = (sp%s).l", r1(), off6s()); break;
	case 0x7a: case 0x7b: util::stream_format(stream, "(sp%s).b = %s", off6s(), r1()); break;
	case 0x7c: case 0x7d: util::stream_format(stream, "(sp%s).w = %s", off6s(), r1()); break;
	case 0x7e: case 0x7f: util::stream_format(stream, "(sp%s).l = %s", off6s(), r1()); break;

	case 0x80: case 0x81: util::stream_format(stream, "%s = %s + %s", r1(), r2(), r3()); break;
		// 82-83
	case 0x84: case 0x85: util::stream_format(stream, "%s = %s - %s", r1(), r2(), r3()); break;
		// 86-87
	case 0x88:            util::stream_format(stream, "jmp (%s)", r2()); break;
		// 89
	case 0x8a: case 0x8b: util::stream_format(stream, "%s = %s & %s", r1(), r2(), r3()); break;
	case 0x8c: case 0x8d: util::stream_format(stream, "%s = %s | %s", r1(), r2(), r3()); break;
	case 0x8e: case 0x8f: util::stream_format(stream, "%s = %s ^ %s", r1(), r2(), r3()); break;

	case 0x90: case 0x91: util::stream_format(stream, "%s = (%s%s).bs", r1(), r2(), off3s()); break;
	case 0x92: case 0x93: util::stream_format(stream, "%s = (%s%s).bu", r1(), r2(), off3s()); break;
	case 0x94: case 0x95: util::stream_format(stream, "%s = (%s%s).ws", r1(), r2(), off3s()); break;
	case 0x96: case 0x97: util::stream_format(stream, "%s = (%s%s).wu", r1(), r2(), off3s()); break;
	case 0x98: case 0x99: util::stream_format(stream, "%s = (%s%s).l", r1(), r2(), off3s()); break;
	case 0x9a: case 0x9b: util::stream_format(stream, "(%s%s).b = %s", r2(), off3s(), r1()); break;
	case 0x9c: case 0x9d: util::stream_format(stream, "(%s%s).w = %s", r2(), off3s(), r1()); break;
	case 0x9e: case 0x9f: util::stream_format(stream, "(%s%s).l = %s", r2(), off3s(), r1()); break;

	case 0xa0: case 0xa1: util::stream_format(stream, "%s = ~%s", r1(), r2()); break;
	case 0xa2: case 0xa3: util::stream_format(stream, "%s = %s", r1(), r2()); break;
	case 0xa4: case 0xa5: util::stream_format(stream, "%s = -%s", r1(), r2()); break;
	case 0xa6: case 0xa7: util::stream_format(stream, "cmp %s, %s", r1(), r2()); break;
	case 0xa8:            util::stream_format(stream, "jsr (%s)", r2()); flags = STEP_OVER; break;
		// a9
	case 0xaa: case 0xab: util::stream_format(stream, "%s = %s >>s %s", r1(), r2(), r3()); break;
	case 0xac: case 0xad: util::stream_format(stream, "%s = %s >> %s", r1(), r2(), r3()); break;
	case 0xae: case 0xaf: util::stream_format(stream, "%s = %s << %s", r1(), r2(), r3()); break;

	case 0xb0: case 0xb1: util::stream_format(stream, "hreg[00] = %s *u %s", r1(), r2()); break;
	case 0xb2: case 0xb3: util::stream_format(stream, "hreg[00] = %s *s %s", r1(), r2()); break;
	case 0xb4: case 0xb5: util::stream_format(stream, "hreg[01:00] = %s *u %s", r1(), r2()); break;
	case 0xb6: case 0xb7: util::stream_format(stream, "hreg[01:00] = %s *s %s", r1(), r2()); break;
	case 0xbc: case 0xbd: util::stream_format(stream, "hreg[03], hreg[02] = %s /s %s", r1(), r2()); break;
	case 0xbe: case 0xbf: util::stream_format(stream, "hreg[03], hreg[02] = %s /u %s", r1(), r2()); break;
		// b6-c7

	case 0xc8: case 0xc9: util::stream_format(stream, "%s = hreg[%s]", r1(), val6u()); break;
	case 0xca: case 0xcb: util::stream_format(stream, "hreg[%s] = %s", val6u(), r1()); break;
		// cc-cf

	case 0xd0:            util::stream_format(stream, "bvs %s", rel8()); break;
	case 0xd1:            util::stream_format(stream, "bltu %s", rel8()); break;
	case 0xd2:            util::stream_format(stream, "beq %s", rel8()); break;
	case 0xd3:            util::stream_format(stream, "bleu %s", rel8()); break;
	case 0xd4:            util::stream_format(stream, "bmi %s", rel8()); break;
	case 0xd5:            util::stream_format(stream, "bra %s", rel8()); break;
	case 0xd6:            util::stream_format(stream, "blts %s", rel8()); break;
	case 0xd7:            util::stream_format(stream, "bles %s", rel8()); break;
	case 0xd8:            util::stream_format(stream, "bvc %s", rel8()); break;
	case 0xd9:            util::stream_format(stream, "bgeu %s", rel8()); break;
	case 0xda:            util::stream_format(stream, "bne %s", rel8()); break;
	case 0xdb:            util::stream_format(stream, "bgtu %s", rel8()); break;
	case 0xdc:            util::stream_format(stream, "bpl %s", rel8()); break;
	case 0xdd:            util::stream_format(stream, "bnv %s", rel8()); break;
	case 0xde:            util::stream_format(stream, "bges %s", rel8()); break;
	case 0xdf:            util::stream_format(stream, "bgts %s", rel8()); break;

	case 0xe0:            util::stream_format(stream, "jmp lr"); flags = STEP_OUT; break;
	case 0xe1:            util::stream_format(stream, "rti1"); flags = STEP_OUT; break;
	case 0xe2:            util::stream_format(stream, "rti2"); flags = STEP_OUT; break;
	case 0xe3:            util::stream_format(stream, "rti3"); flags = STEP_OUT; break;
		// e4-ef

	case 0xf0:            util::stream_format(stream, "stc"); break;
	case 0xf1:            util::stream_format(stream, "clc"); break;
	case 0xf2:            util::stream_format(stream, "stz"); break;
	case 0xf3:            util::stream_format(stream, "clz"); break;
	case 0xf4:            util::stream_format(stream, "stn"); break;
	case 0xf5:            util::stream_format(stream, "cln"); break;
	case 0xf6:            util::stream_format(stream, "stv"); break;
	case 0xf7:            util::stream_format(stream, "clv"); break;
	case 0xf8:            util::stream_format(stream, "di"); break;
	case 0xf9:            util::stream_format(stream, "ei"); break;
		// fa-fb
	case 0xfc:            util::stream_format(stream, "nop"); break;
		// fd
	case 0xfe:            util::stream_format(stream, "wait"); break;
		// ff
	default:              util::stream_format(stream, "?%02x", m_opcode >> 24);
	}

	return nb | flags | SUPPORTED;
}
