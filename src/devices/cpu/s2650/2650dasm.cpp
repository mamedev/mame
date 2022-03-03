// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************
 *
 *  Portable Signetics 2650 disassembler
 *
 *  Written by J. Buchmueller (pullmoll@t-online.de)
 *  for the MAME project
 *
 **************************************************************************/

#include "emu.h"
#include "2650dasm.h"

/* handy table to build relative offsets from HR (holding register) */
const int s2650_disassembler::rel[0x100] = {
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,
	-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,
	-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,
	-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,
	-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,
	-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,
	-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
};

std::string s2650_disassembler::SYM(int addr)
{
	return util::string_format("$%04x", addr);
}

/* format an immediate */
std::string s2650_disassembler::IMM(offs_t pc, const data_buffer &params)
{
	return util::string_format("$%02x", params.r8(pc));
}

const char s2650_disassembler::cc[4] = { 'z', 'p', 'm', 'a' };

void s2650_disassembler::add(std::string &buf, const std::string &str)
{
	if(!buf.empty())
		buf += '+';
	buf += str;
}

/* format an immediate for PSL */
std::string s2650_disassembler::IMM_PSL(offs_t pc, const data_buffer &params)
{
	u8 v = params.r8(pc);

	if (v == 0xff) {
		return "all";

	} else {
		std::string buff;
		switch (v & 0xc0)
		{
			case 0x40: add(buff, "p"); break;
			case 0x80: add(buff, "m"); break;
			case 0xc0: add(buff, "cc"); break;
		}
		if (v & 0x20)   /* inter digit carry */
			add(buff, "idc");
		if (v & 0x10)   /* register select */
			add(buff, "rs");
		if (v & 0x08)   /* with carry */
			add(buff, "wc");
		if (v & 0x04)   /* overflow */
			add(buff, "ovf");
		if (v & 0x02)   /* 2's complement comparisons */
			add(buff, "com");
		if (v & 0x01)   /* carry */
			add(buff, "c");
		return buff;
	}
}

/* format an immediate for PSU (processor status upper) */
std::string s2650_disassembler::IMM_PSU(offs_t pc, const data_buffer &params)
{
	int v = params.r8(pc);

	if (v == 0xff) {
		return "all";

	} else {
		std::string buff;
		if (v & 0x80)   /* sense input */
			add(buff, "si");
		if (v & 0x40)   /* flag output */
			add(buff, "fo");
		if (v & 0x20)   /* interrupt inhibit */
			add(buff, "ii");
		if (v & 0x10)   /* unused bit 4 */
			add(buff, "4");
		if (v & 0x08)   /* unused bit 3 */
			add(buff, "3");
		if (v & 0x04)   /* stack pointer bit 2 */
			add(buff, "sp2");
		if (v & 0x02)   /* stack pointer bit 1 */
			add(buff, "sp1");
		if (v & 0x01)   /* stack pointer bit 0 */
			add(buff, "sp0");
		return buff;
	}
}

/* format an relative address */
std::string s2650_disassembler::REL(offs_t pc, const data_buffer &params)
{
	int o = params.r8(pc);
	return util::string_format("%s%s", (o&0x80)?"*":"", SYM((pc&0x6000)+((pc+1+rel[o])&0x1fff)));
}

/* format an relative address (implicit page 0) */
std::string s2650_disassembler::REL0(offs_t pc, const data_buffer &params)
{
	int o = params.r8(pc);
	return util::string_format("%s%s", (o&0x80)?"*":"", SYM((rel[o]) & 0x1fff));
}

/* format a destination register and an absolute address */
std::string s2650_disassembler::ABS(int load, int r, offs_t pc, const data_buffer &params)
{
	int h = params.r8(pc);
	int l = params.r8((pc&0x6000)+((pc+1)&0x1fff));
	int a = (pc & 0x6000) + ((h & 0x1f) << 8) + l;

	if (m_config->get_z80_mnemonics_mode()) {
		if (load) {
			switch (h >> 5) {
				case 0: return util::string_format("r%d,(%s)",       r, SYM(a));
				case 1: return util::string_format("r0,(%s,r%d++)",  SYM(a), r);
				case 2: return util::string_format("r0,(%s,r%d--)",  SYM(a), r);
				case 3: return util::string_format("r0,(%s,r%d)",    SYM(a), r);
				case 4: return util::string_format("r%d,*(%s)",      r, SYM(a));
				case 5: return util::string_format("r0,*(%s,r%d++)", SYM(a), r);
				case 6: return util::string_format("r0,*(%s,r%d--)", SYM(a), r);
				case 7: return util::string_format("r0,*(%s,r%d)",   SYM(a), r);
			}
		} else {
			switch (h >> 5) {
				case 0: return util::string_format("(%s),r%d",       SYM(a), r);
				case 1: return util::string_format("(%s,r%d++),r0",  SYM(a), r);
				case 2: return util::string_format("(%s,r%d--),r0",  SYM(a), r);
				case 3: return util::string_format("(%s,r%d),r0",    SYM(a), r);
				case 4: return util::string_format("*(%s),r%d",      SYM(a), r);
				case 5: return util::string_format("*(%s,r%d++),r0", SYM(a), r);
				case 6: return util::string_format("*(%s,r%d--),r0", SYM(a), r);
				case 7: return util::string_format("*(%s,r%d),r0",   SYM(a), r);
			}
		}
	} else {
		switch (h >> 5) {
			case 0: return util::string_format("%d %s",      r, SYM(a));
			case 1: return util::string_format("0 %s,r%d+",  SYM(a), r);
			case 2: return util::string_format("0 %s,r%d-",  SYM(a), r);
			case 3: return util::string_format("0 %s,r%d",   SYM(a), r);
			case 4: return util::string_format("%d *%s",     r, SYM(a));
			case 5: return util::string_format("0 *%s,r%d+", SYM(a), r);
			case 6: return util::string_format("0 *%s,r%d-", SYM(a), r);
			case 7: return util::string_format("0 *%s,r%d",  SYM(a), r);
		}
	}
	return "";
}

/* format an (branch) absolute address */
std::string s2650_disassembler::ADR(offs_t pc, const data_buffer &params)
{
	int h = params.r8(pc);
	int l = params.r8((pc&0x6000)+((pc+1)&0x1fff));
	int a = ((h & 0x7f) << 8) + l;
	if (h & 0x80)
		return util::string_format("*%s", SYM(a));
	else
		return util::string_format("%s", SYM(a));
}

s2650_disassembler::s2650_disassembler(config *conf) : m_config(conf)
{
}

/* disassemble one instruction at PC into buff. return byte size of instr */
offs_t s2650_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	int PC = pc;
	int op = opcodes.r8(pc);
	int rv = op & 3;

	bool z80 = m_config->get_z80_mnemonics_mode();

	pc += 1;
	switch (op)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
			util::stream_format(stream, z80 ? "ld   r0,r%d" : "lodz,%d", rv);
			break;
		case 0x04: case 0x05: case 0x06: case 0x07:
			util::stream_format(stream, z80 ? "ld   r%d,%s" : "lodi,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
			util::stream_format(stream, z80 ? "ld   r%d,(%s)" : "lodr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, z80 ? "ld   %s" : "loda,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0x10: case 0x11:
			util::stream_format(stream, z80 ? "**** $%02X" : "****   $%02X",op);
			break;
		case 0x12:
			util::stream_format(stream, z80 ? "ld   r0,psu" : "spsu");
			break;
		case 0x13:
			util::stream_format(stream, z80 ? "ld   r0,psl" : "spsl");
			break;
		case 0x14: case 0x15: case 0x16: case 0x17:
			if (z80) {
				if (rv == 3)
					util::stream_format(stream, "ret");
				else
					util::stream_format(stream, "ret  %c", cc[rv]);
			} else
				util::stream_format(stream, "retc   %c", cc[rv]);
			flags = STEP_OUT;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			if (z80) {
				if (rv == 3)
					util::stream_format(stream, "jr   %s", REL(pc, params));
				else
					util::stream_format(stream, "jr   %c,%s", cc[rv], REL(pc, params));
			} else
				util::stream_format(stream, "bctr,%c %s", cc[rv], REL(pc, params));
			pc+=1;
			break;
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			if (z80) {
				if (rv == 3)
					util::stream_format(stream, "jp   %s", ADR(pc, params));
				else
					util::stream_format(stream, "jp   %c,%s", cc[rv], ADR(pc, params));
			} else
				util::stream_format(stream, "bcta,%c %s", cc[rv], ADR(pc, params));
			pc+=2;
			break;
		case 0x20: case 0x21: case 0x22: case 0x23:
			util::stream_format(stream, z80 ? "xor  r0,r%d" : "eorz,%d", rv);
			break;
		case 0x24: case 0x25: case 0x26: case 0x27:
			util::stream_format(stream, z80 ? "xor  r%d,%s" : "eori,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
			util::stream_format(stream, z80 ? "xor  r%d,(%s)" : "eorr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			util::stream_format(stream, z80 ? "xor  %s" : "eora,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
			util::stream_format(stream, z80 ? "in   r%d,(ctrl)" : "redc,%d", rv);
			break;
		case 0x34: case 0x35: case 0x36: case 0x37:
			if (z80) {
				if (rv == 3)
					util::stream_format(stream, "iret");
				else
					util::stream_format(stream, "iret %c", cc[rv]);
			} else
				util::stream_format(stream, "rete   %c", cc[rv]);
			flags = STEP_OUT;
			break;
		case 0x38: case 0x39: case 0x3a: case 0x3b:
			if (z80) {
				if (rv == 3)
					util::stream_format(stream, "calr %s", REL(pc, params));
				else
					util::stream_format(stream, "calr %c,%s", cc[rv], REL(pc, params));
			} else
				util::stream_format(stream, "bstr,%c %s", cc[rv], REL(pc, params));
			pc+=1;
			flags = STEP_OVER;
			break;
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			if (z80) {
				if (rv == 3)
					util::stream_format(stream, "call %s", ADR(pc, params));
				else
					util::stream_format(stream, "call %c,%s", cc[rv], ADR(pc, params));
			} else
				util::stream_format(stream, "bsta,%c %s", cc[rv], ADR(pc, params));
			pc+=2;
			flags = STEP_OVER;
			break;
		case 0x40:
			util::stream_format(stream, "halt");
			break;
		case 0x41: case 0x42: case 0x43:
			util::stream_format(stream, z80 ? "and  r0,r%d" : "andz,%d", rv);
			break;
		case 0x44: case 0x45: case 0x46: case 0x47:
			util::stream_format(stream, z80 ? "and  r%d,%s" : "andi,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0x48: case 0x49: case 0x4a: case 0x4b:
			util::stream_format(stream, z80 ? "and  r%d,(%s)" : "andr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			util::stream_format(stream, z80 ? "and  %s" : "anda,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0x50: case 0x51: case 0x52: case 0x53:
			util::stream_format(stream, z80 ? "ror  r%d" : "rrr,%d", rv);
			break;
		case 0x54: case 0x55: case 0x56: case 0x57:
			util::stream_format(stream, z80 ? "in   r%d,(%s)" : "rede,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0x58: case 0x59: case 0x5a: case 0x5b:
			util::stream_format(stream, z80 ? "jrnz r%d,%s" : "brnr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			util::stream_format(stream, z80 ? "jpnz r%d,%s" : "brna,%d %s", rv, ADR(pc, params));
			pc+=2;
			break;
		case 0x60: case 0x61: case 0x62: case 0x63:
			util::stream_format(stream, z80 ? "or   r0,r%d" : "iorz,%d", rv);
			break;
		case 0x64: case 0x65: case 0x66: case 0x67:
			util::stream_format(stream, z80 ? "or   r%d,%s" : "iori,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0x68: case 0x69: case 0x6a: case 0x6b:
			util::stream_format(stream, z80 ? "or   r%d,(%s)" : "iorr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			util::stream_format(stream, z80 ? "or   %s" : "iora,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0x70: case 0x71: case 0x72: case 0x73:
			util::stream_format(stream, z80 ? "in   r%d,(data)" : "redd,%d", rv);
			break;
		case 0x74:
			util::stream_format(stream, z80 ? "res  psu,%s" : "cpsu   %s", IMM_PSU(pc, params));
			pc+=1;
			break;
		case 0x75:
			util::stream_format(stream, z80 ? "res  psl,%s" : "cpsl   %s", IMM_PSL(pc, params));
			pc+=1;
			break;
		case 0x76:
			util::stream_format(stream, z80 ? "set  psu,%s" : "ppsu   %s", IMM_PSU(pc, params));
			pc+=1;
			break;
		case 0x77:
			util::stream_format(stream, z80 ? "set  psl,%s" : "ppsl   %s", IMM_PSL(pc, params));
			pc+=1;
			break;
		case 0x78: case 0x79: case 0x7a: case 0x7b:
			util::stream_format(stream, z80 ? "call r%d-nz,%s" : "bsnr,%d %s", rv, REL(pc, params));
			pc+=1;
			flags = STEP_OVER;
			break;
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			util::stream_format(stream, z80 ? "call r%d-nz,%s" : "bsna,%d %s", rv, ADR(pc, params));
			pc+=2;
			flags = STEP_OVER;
			break;
		case 0x80: case 0x81: case 0x82: case 0x83:
			util::stream_format(stream, z80 ? "add  r0,r%d" : "addz,%d", rv);
			break;
		case 0x84: case 0x85: case 0x86: case 0x87:
			util::stream_format(stream, z80 ? "add  r%d,%s" : "addi,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0x88: case 0x89: case 0x8a: case 0x8b:
			util::stream_format(stream, z80 ? "add  r%d,(%s)" : "addr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			util::stream_format(stream, z80 ? "add  %s" : "adda,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0x90: case 0x91:
			util::stream_format(stream, z80 ? "**** $%02X" : "****   $%02X",op);
			break;
		case 0x92:
			util::stream_format(stream, z80 ? "ld   psu,r0" : "lpsu");
			break;
		case 0x93:
			util::stream_format(stream, z80 ? "ld   psl,r0" : "lpsl");
			break;
		case 0x94: case 0x95: case 0x96: case 0x97:
			util::stream_format(stream, z80 ? "daa  r%d" : "dar,%d", rv);
			break;
		case 0x98: case 0x99: case 0x9a:
			util::stream_format(stream, z80 ? "jr   n%c,%s" : "bcfr,%c %s", cc[rv], REL(pc, params));
			pc+=1;
			break;
		case 0x9b:
			util::stream_format(stream, z80 ? "jr0  %s" : "zbrr   %s", REL0(pc, params));
			pc+=1;
			break;
		case 0x9c: case 0x9d: case 0x9e:
			util::stream_format(stream, z80 ? "jp   n%c,%s" : "bcfa,%c %s", cc[rv], ADR(pc, params));
			pc+=2;
			break;
		case 0x9f:
			util::stream_format(stream, z80 ? "jp   %s+r3" : "bxa    %s", ADR(pc, params));
			pc+=2;
			break;
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
			util::stream_format(stream, z80 ? "sub  r0,r%d" : "subz,%d", rv);
			break;
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			util::stream_format(stream, z80 ? "sub  r%d,%s" : "subi,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
			util::stream_format(stream, z80 ? "sub  r%d,(%s)" : "subr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, z80 ? "sub  %s" : "suba,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
			util::stream_format(stream, z80 ? "out  (ctrl),r%d" : "wrtc,%d", rv);
			break;
		case 0xb4:
			util::stream_format(stream, z80 ? "bit  psu,%s" : "tpsu   %s", IMM_PSU(pc, params));
			pc+=1;
			break;
		case 0xb5:
			util::stream_format(stream, z80 ? "bit  psl,%s" : "tpsl   %s", IMM_PSL(pc, params));
			pc+=1;
			break;
		case 0xb6: case 0xb7:
			util::stream_format(stream, z80 ? "**** $%02X" : "****   $%02X",op);
			break;
		case 0xb8: case 0xb9: case 0xba:
			util::stream_format(stream, z80 ? "calr n%c,%s" : "bsfr,%c %s", cc[rv], REL(pc, params));
			pc+=1;
			flags = STEP_OVER;
			break;
		case 0xbb:
			util::stream_format(stream, z80 ? "cal0 %s" : "zbsr   %s", REL0(pc, params));
			pc+=1;
			flags = STEP_OVER;
			break;
		case 0xbc: case 0xbd: case 0xbe:
			util::stream_format(stream, z80 ? "call n%c,%s" : "bsfa,%c %s", cc[rv], ADR(pc, params));
			pc+=2;
			flags = STEP_OVER;
			break;
		case 0xbf:
			util::stream_format(stream, z80 ? "call %s+r3" : "bsxa   %s", ADR(pc, params));
			pc+=2;
			flags = STEP_OVER;
			break;
		case 0xc0:
			util::stream_format(stream, "nop");
			break;
		case 0xc1: case 0xc2: case 0xc3:
			util::stream_format(stream, z80 ? "ld   r%d,r0" : "strz,%d", rv);
			break;
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			util::stream_format(stream, z80 ? "**** $%02X" : "****   $%02X",op);
			break;
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			util::stream_format(stream, z80 ? "ld   (%2$s),r%1$d" : "strr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			util::stream_format(stream, z80 ? "ld   %s" : "stra,%s", ABS(0,rv,pc, params));
			pc+=2;
			break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			util::stream_format(stream, z80 ? "rlca r%d" : "rrl,%d", rv);
			break;
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			util::stream_format(stream, z80 ? "out  (%s),r%d" : "wrte,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
			util::stream_format(stream, z80 ? "ijnz r%d,%s" : "birr,%d %s", rv, REL(pc, params));
			pc+=1;
			flags = STEP_OVER;
			break;
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
			util::stream_format(stream, z80 ? "ijnz r%d,%s" : "bira,%d %s", rv, ADR(pc, params));
			pc+=2;
			flags = STEP_OVER;
			break;
		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
			util::stream_format(stream, z80 ? "cp   r0,%d" : "comz,%d", rv);
			break;
		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			util::stream_format(stream, z80 ? "cp   r%d,%s" : "comi,%d %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0xe8: case 0xe9: case 0xea: case 0xeb:
			util::stream_format(stream, z80 ? "cp   r%d,(%s)" : "comr,%d %s", rv, REL(pc, params));
			pc+=1;
			break;
		case 0xec: case 0xed: case 0xee: case 0xef:
			util::stream_format(stream, z80 ? "cp   %s" : "coma,%s", ABS(1,rv,pc, params));
			pc+=2;
			break;
		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
			util::stream_format(stream, z80 ? "out  (data),r%d" : "wrtd,%d", rv);
			break;
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
			util::stream_format(stream, z80 ? "test r%d,%s" : "tmi,%d  %s", rv, IMM(pc, params));
			pc+=1;
			break;
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
			util::stream_format(stream, z80 ? "djnz r%d,%s" : "bdrr,%d %s", rv, REL(pc, params));
			pc+=1;
			flags = STEP_OVER;
			break;
		case 0xfc: case 0xfd: case 0xfe: case 0xff:
			util::stream_format(stream, z80 ? "djnz r%d,%s" : "bdra,%d %s", rv, ADR(pc, params));
			pc+=2;
			flags = STEP_OVER;
			break;
	}
	return (pc - PC) | flags | SUPPORTED;
}

u32 s2650_disassembler::opcode_alignment() const
{
	return 1;
}

u32 s2650_disassembler::interface_flags() const
{
	return PAGED;
}

u32 s2650_disassembler::page_address_bits() const
{
	return 13;
}
