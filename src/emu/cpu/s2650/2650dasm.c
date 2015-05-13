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

static const UINT8 *rambase;
static offs_t pcbase;

#define readarg(A)  (rambase[(A) - pcbase])

/* Set this to 1 to disassemble using Z80 style mnemonics */
#define HJB     0

/* Set this to 1 to give names to condition codes and flag bits */
#define MNEMO   1

/* handy table to build relative offsets from HR (holding register) */
static const int rel[0x100] = {
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

typedef char* (*callback) (int addr);
static callback find_symbol = 0;

static char *SYM(int addr)
{
	static char buff[8+1];
	char * s = NULL;

	if (find_symbol) s = (*find_symbol)(addr);
	if (s) return s;

	sprintf(buff, "$%04x", addr);
	return buff;
}

/* format an immediate */
static char *IMM(int pc)
{
	static char buff[32];

	sprintf(buff, "$%02x", readarg(pc));
	return buff;
}

#if MNEMO
static const char cc[4] = { 'z', 'p', 'm', 'a' };

/* format an immediate for PSL */
static char *IMM_PSL(int pc)
{
	static char buff[32];
	char *p = buff;
	int v = readarg(pc);

	if (v == 0xff) {
		p += sprintf(p, "all");
	} else {
		switch (v & 0xc0)
		{
			case 0x40: p += sprintf(p, "p+"); break;
			case 0x80: p += sprintf(p, "m+"); break;
			case 0xc0: p += sprintf(p, "cc+"); break;
		}
		if (v & 0x20)   /* inter digit carry */
			p += sprintf(p, "idc+");
		if (v & 0x10)   /* register select */
			p += sprintf(p, "rs+");
		if (v & 0x08)   /* with carry */
			p += sprintf(p, "wc+");
		if (v & 0x04)   /* overflow */
			p += sprintf(p, "ovf+");
		if (v & 0x02)   /* 2's complement comparisons */
			p += sprintf(p, "com+");
		if (v & 0x01)   /* carry */
			p += sprintf(p, "c+");
		if (p > buff)
			*--p = '\0';
	}
	return buff;
}

/* format an immediate for PSU (processor status upper) */
static char *IMM_PSU(int pc)
{
	static char buff[32];
	char *p = buff;
	int v = readarg(pc);

	if (v == 0xff) {
		p += sprintf(p, "all");

	} else {
		if (v & 0x80)   /* sense input */
			p += sprintf(p, "si+");
		if (v & 0x40)   /* flag output */
			p += sprintf(p, "fo+");
		if (v & 0x20)   /* interrupt inhibit */
			p += sprintf(p, "ii+");
		if (v & 0x10)   /* unused bit 4 */
			p += sprintf(p, "4+");
		if (v & 0x08)   /* unused bit 3 */
			p += sprintf(p, "3+");
		if (v & 0x04)   /* stack pointer bit 2 */
			p += sprintf(p, "sp2+");
		if (v & 0x02)   /* stack pointer bit 1 */
			p += sprintf(p, "sp1+");
		if (v & 0x01)   /* stack pointer bit 0 */
			p += sprintf(p, "sp0+");
		if (p > buff)
			*--p = '\0';
	}
	return buff;
}
#else
static const char cc[4] = { '0', '1', '2', '3' };
#define IMM_PSL IMM
#define IMM_PSU IMM
#endif

/* format an relative address */
static char *REL(int pc)
{
static char buff[32];
int o = readarg(pc);
	sprintf(buff, "%s%s", (o&0x80)?"*":"", SYM((pc&0x6000)+((pc+1+rel[o])&0x1fff)));
	return buff;
}

/* format an relative address (implicit page 0) */
static char *REL0(int pc)
{
static char buff[32];
int o = readarg(pc);
	sprintf(buff, "%s%s", (o&0x80)?"*":"", SYM((rel[o]) & 0x1fff));
	return buff;
}

/* format a destination register and an absolute address */
static char *ABS(int load, int r, int pc)
{
	static char buff[32];
	int h = readarg(pc);
	int l = readarg((pc&0x6000)+((pc+1)&0x1fff));
	int a = (pc & 0x6000) + ((h & 0x1f) << 8) + l;

#if HJB
	if (load) {
		switch (h >> 5) {
			case 0: sprintf(buff, "r%d,(%s)",       r, SYM(a)); break;
			case 1: sprintf(buff, "r0,(%s,r%d++)",  SYM(a), r); break;
			case 2: sprintf(buff, "r0,(%s,r%d--)",  SYM(a), r); break;
			case 3: sprintf(buff, "r0,(%s,r%d)",    SYM(a), r); break;
			case 4: sprintf(buff, "r%d,*(%s)",      r, SYM(a)); break;
			case 5: sprintf(buff, "r0,*(%s,r%d++)", SYM(a), r); break;
			case 6: sprintf(buff, "r0,*(%s,r%d--)", SYM(a), r); break;
			case 7: sprintf(buff, "r0,*(%s,r%d)",   SYM(a), r); break;
		}
	} else {
		switch (h >> 5) {
			case 0: sprintf(buff, "(%s),r%d",       SYM(a), r); break;
			case 1: sprintf(buff, "(%s,r%d++),r0",  SYM(a), r); break;
			case 2: sprintf(buff, "(%s,r%d--),r0",  SYM(a), r); break;
			case 3: sprintf(buff, "(%s,r%d),r0",    SYM(a), r); break;
			case 4: sprintf(buff, "*(%s),r%d",      SYM(a), r); break;
			case 5: sprintf(buff, "*(%s,r%d++),r0", SYM(a), r); break;
			case 6: sprintf(buff, "*(%s,r%d--),r0", SYM(a), r); break;
			case 7: sprintf(buff, "*(%s,r%d),r0",   SYM(a), r); break;
		}
	}
#else
	switch (h >> 5) {
		case 0: sprintf(buff, "%d %s",      r, SYM(a)); break;
		case 1: sprintf(buff, "0 %s,r%d+",  SYM(a), r); break;
		case 2: sprintf(buff, "0 %s,r%d-",  SYM(a), r); break;
		case 3: sprintf(buff, "0 %s,r%d",   SYM(a), r); break;
		case 4: sprintf(buff, "%d *%s",     r, SYM(a)); break;
		case 5: sprintf(buff, "0 *%s,r%d+", SYM(a), r); break;
		case 6: sprintf(buff, "0 *%s,r%d-", SYM(a), r); break;
		case 7: sprintf(buff, "0 *%s,r%d",  SYM(a), r); break;
	}
#endif
	return buff;
}

/* format an (branch) absolute address */
static char *ADR(int pc)
{
	static char buff[32];
	int h = readarg(pc);
	int l = readarg((pc&0x6000)+((pc+1)&0x1fff));
	int a = ((h & 0x7f) << 8) + l;
	if (h & 0x80)
		sprintf(buff, "*%s", SYM(a));
	else
		sprintf(buff, "%s", SYM(a));
	return buff;
}

/* disassemble one instruction at PC into buff. return byte size of instr */
CPU_DISASSEMBLE( s2650 )
{
	UINT32 flags = 0;
	int PC = pc;
	int op = oprom[0];
	int rv = op & 3;

	rambase = opram;
	pcbase = PC;

	pc += 1;
	switch (op)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
#if HJB
			sprintf(buffer, "ld   r0,r%d", rv);
#else
			sprintf(buffer, "lodz,%d", rv);
#endif
			break;
		case 0x04: case 0x05: case 0x06: case 0x07:
#if HJB
			sprintf(buffer, "ld   r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "lodi,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
#if HJB
			sprintf(buffer, "ld   r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "lodr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
#if HJB
			sprintf(buffer, "ld   %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "loda,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0x10: case 0x11:
#if HJB
			sprintf(buffer, "**** $%02X",op);
#else
			sprintf(buffer, "****   $%02X",op);
#endif
			break;
		case 0x12:
#if HJB
			sprintf(buffer, "ld   r0,psu");
#else
			sprintf(buffer, "spsu");
#endif
			break;
		case 0x13:
#if HJB
			sprintf(buffer, "ld   r0,psl");
#else
			sprintf(buffer, "spsl");
#endif
			break;
		case 0x14: case 0x15: case 0x16: case 0x17:
#if HJB
			if (rv == 3)
				sprintf(buffer, "ret");
			else
				sprintf(buffer, "ret  %c", cc[rv]);
#else
			sprintf(buffer, "retc   %c", cc[rv]);
#endif
			flags = DASMFLAG_STEP_OUT;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
#if HJB
			if (rv == 3)
				sprintf(buffer, "jr   %s", REL(pc));
			else
				sprintf(buffer, "jr   %c,%s", cc[rv], REL(pc));
#else
			sprintf(buffer, "bctr,%c %s", cc[rv], REL(pc));
#endif
			pc+=1;
			break;
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
#if HJB
			if (rv == 3)
				sprintf(buffer, "jp   %s", ADR(pc));
			else
				sprintf(buffer, "jp   %c,%s", cc[rv], ADR(pc));
#else
			sprintf(buffer, "bcta,%c %s", cc[rv], ADR(pc));
#endif
			pc+=2;
			break;
		case 0x20: case 0x21: case 0x22: case 0x23:
#if HJB
			sprintf(buffer, "xor  r0,r%d", rv);
#else
			sprintf(buffer, "eorz,%d", rv);
#endif
			break;
		case 0x24: case 0x25: case 0x26: case 0x27:
#if HJB
			sprintf(buffer, "xor  r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "eori,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
#if HJB
			sprintf(buffer, "xor  r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "eorr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
#if HJB
			sprintf(buffer, "xor  %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "eora,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
#if HJB
			sprintf(buffer, "in   r%d,(ctrl)", rv);
#else
			sprintf(buffer, "redc,%d", rv);
#endif
			break;
		case 0x34: case 0x35: case 0x36: case 0x37:
#if HJB
			if (rv == 3)
				sprintf(buffer, "iret");
			else
				sprintf(buffer, "iret %c", cc[rv]);
#else
			sprintf(buffer, "rete   %c", cc[rv]);
#endif
			flags = DASMFLAG_STEP_OUT;
			break;
		case 0x38: case 0x39: case 0x3a: case 0x3b:
#if HJB
			if (rv == 3)
				sprintf(buffer, "calr %s", REL(pc));
			else
				sprintf(buffer, "calr %c,%s", cc[rv], REL(pc));
#else
			sprintf(buffer, "bstr,%c %s", cc[rv], REL(pc));
#endif
			pc+=1;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
#if HJB
			if (rv == 3)
				sprintf(buffer, "call %s", ADR(pc));
			else
				sprintf(buffer, "call %c,%s", cc[rv], ADR(pc));
#else
			sprintf(buffer, "bsta,%c %s", cc[rv], ADR(pc));
#endif
			pc+=2;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0x40:
			sprintf(buffer, "halt");
			break;
		case 0x41: case 0x42: case 0x43:
#if HJB
			sprintf(buffer, "and  r0,r%d", rv);
#else
			sprintf(buffer, "andz,%d", rv);
#endif
			break;
		case 0x44: case 0x45: case 0x46: case 0x47:
#if HJB
			sprintf(buffer, "and  r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "andi,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0x48: case 0x49: case 0x4a: case 0x4b:
#if HJB
			sprintf(buffer, "and  r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "andr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
#if HJB
			sprintf(buffer, "and  %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "anda,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0x50: case 0x51: case 0x52: case 0x53:
#if HJB
			sprintf(buffer, "ror  r%d", rv);
#else
			sprintf(buffer, "rrr,%d", rv);
#endif
			break;
		case 0x54: case 0x55: case 0x56: case 0x57:
#if HJB
			sprintf(buffer, "in   r%d,(%s)", rv, IMM(pc));
#else
			sprintf(buffer, "rede,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0x58: case 0x59: case 0x5a: case 0x5b:
#if HJB
			sprintf(buffer, "jrnz r%d,%s", rv, REL(pc));
#else
			sprintf(buffer, "brnr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
#if HJB
			sprintf(buffer, "jpnz r%d,%s", rv, ADR(pc));
#else
			sprintf(buffer, "brna,%d %s", rv, ADR(pc));
#endif
			pc+=2;
			break;
		case 0x60: case 0x61: case 0x62: case 0x63:
#if HJB
			sprintf(buffer, "or   r0,r%d", rv);
#else
			sprintf(buffer, "iorz,%d", rv);
#endif
			break;
		case 0x64: case 0x65: case 0x66: case 0x67:
#if HJB
			sprintf(buffer, "or   r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "iori,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0x68: case 0x69: case 0x6a: case 0x6b:
#if HJB
			sprintf(buffer, "or   r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "iorr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
#if HJB
			sprintf(buffer, "or   %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "iora,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0x70: case 0x71: case 0x72: case 0x73:
#if HJB
			sprintf(buffer, "in   r%d,(data)", rv);
#else
			sprintf(buffer, "redd,%d", rv);
#endif
			break;
		case 0x74:
#if HJB
			sprintf(buffer, "res  psu,%s", IMM_PSU(pc));
#else
			sprintf(buffer, "cpsu   %s", IMM_PSU(pc));
#endif
			pc+=1;
			break;
		case 0x75:
#if HJB
			sprintf(buffer, "res  psl,%s", IMM_PSL(pc));
#else
			sprintf(buffer, "cpsl   %s", IMM_PSL(pc));
#endif
			pc+=1;
			break;
		case 0x76:
#if HJB
			sprintf(buffer, "set  psu,%s", IMM_PSU(pc));
#else
			sprintf(buffer, "ppsu   %s", IMM_PSU(pc));
#endif
			pc+=1;
			break;
		case 0x77:
#if HJB
			sprintf(buffer, "set  psl,%s", IMM_PSL(pc));
#else
			sprintf(buffer, "ppsl   %s", IMM_PSL(pc));
#endif
			pc+=1;
			break;
		case 0x78: case 0x79: case 0x7a: case 0x7b:
#if HJB
			sprintf(buffer, "call r%d-nz,%s", rv, REL(pc));
#else
			sprintf(buffer, "bsnr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
#if HJB
			sprintf(buffer, "call r%d-nz,%s", rv, ADR(pc));
#else
			sprintf(buffer, "bsna,%d %s", rv, ADR(pc));
#endif
			pc+=2;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0x80: case 0x81: case 0x82: case 0x83:
#if HJB
			sprintf(buffer, "add  r0,r%d", rv);
#else
			sprintf(buffer, "addz,%d", rv);
#endif
			break;
		case 0x84: case 0x85: case 0x86: case 0x87:
#if HJB
			sprintf(buffer, "add  r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "addi,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0x88: case 0x89: case 0x8a: case 0x8b:
#if HJB
			sprintf(buffer, "add  r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "addr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
#if HJB
			sprintf(buffer, "add  %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "adda,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0x90: case 0x91:
#if HJB
			sprintf(buffer, "**** $%02X",op);
#else
			sprintf(buffer, "****   $%02X",op);
#endif
			break;
		case 0x92:
#if HJB
			sprintf(buffer, "ld   psu,r0");
#else
			sprintf(buffer, "lpsu");
#endif
			break;
		case 0x93:
#if HJB
			sprintf(buffer, "ld   psl,r0");
#else
			sprintf(buffer, "lpsl");
#endif
			break;
		case 0x94: case 0x95: case 0x96: case 0x97:
#if HJB
			sprintf(buffer, "daa  r%d", rv);
#else
			sprintf(buffer, "dar,%d", rv);
#endif
			break;
		case 0x98: case 0x99: case 0x9a:
#if HJB
			sprintf(buffer, "jr   n%c,%s", cc[rv], REL(pc));
#else
			sprintf(buffer, "bcfr,%c %s", cc[rv], REL(pc));
#endif
			pc+=1;
			break;
		case 0x9b:
#if HJB
			sprintf(buffer, "jr0  %s", REL0(pc));
#else
			sprintf(buffer, "zbrr   %s", REL0(pc));
#endif
			pc+=1;
			break;
		case 0x9c: case 0x9d: case 0x9e:
#if HJB
			sprintf(buffer, "jp   n%c,%s", cc[rv], ADR(pc));
#else
			sprintf(buffer, "bcfa,%c %s", cc[rv], ADR(pc));
#endif
			pc+=2;
			break;
		case 0x9f:
#if HJB
			sprintf(buffer, "jp   %s+r3", ADR(pc));
#else
			sprintf(buffer, "bxa    %s", ADR(pc));
#endif
			pc+=2;
			break;
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
#if HJB
			sprintf(buffer, "sub  r0,r%d", rv);
#else
			sprintf(buffer, "subz,%d", rv);
#endif
			break;
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
#if HJB
			sprintf(buffer, "sub  r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "subi,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
#if HJB
			sprintf(buffer, "sub  r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "subr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0xac: case 0xad: case 0xae: case 0xaf:
#if HJB
			sprintf(buffer, "sub  %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "suba,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
#if HJB
			sprintf(buffer, "out  (ctrl),r%d", rv);
#else
			sprintf(buffer, "wrtc,%d", rv);
#endif
			break;
		case 0xb4:
#if HJB
			sprintf(buffer, "bit  psu,%s", IMM_PSU(pc));
#else
			sprintf(buffer, "tpsu   %s", IMM_PSU(pc));
#endif
			pc+=1;
			break;
		case 0xb5:
#if HJB
			sprintf(buffer, "bit  psl,%s", IMM_PSL(pc));
#else
			sprintf(buffer, "tpsl   %s", IMM_PSL(pc));
#endif
			pc+=1;
			break;
		case 0xb6: case 0xb7:
#if HJB
			sprintf(buffer, "**** $%02X",op);
#else
			sprintf(buffer, "****   $%02X",op);
#endif
			break;
		case 0xb8: case 0xb9: case 0xba:
#if HJB
			sprintf(buffer, "calr n%c,%s", cc[rv], REL(pc));
#else
			sprintf(buffer, "bsfr,%c %s", cc[rv], REL(pc));
#endif
			pc+=1;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xbb:
#if HJB
			sprintf(buffer, "cal0 %s", REL0(pc));
#else
			sprintf(buffer, "zbsr   %s", REL0(pc));
#endif
			pc+=1;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xbc: case 0xbd: case 0xbe:
#if HJB
			sprintf(buffer, "call n%c,%s", cc[rv], ADR(pc));
#else
			sprintf(buffer, "bsfa,%c %s", cc[rv], ADR(pc));
#endif
			pc+=2;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xbf:
#if HJB
			sprintf(buffer, "call %s+r3", ADR(pc));
#else
			sprintf(buffer, "bsxa   %s", ADR(pc));
#endif
			pc+=2;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xc0:
			sprintf(buffer, "nop");
			break;
		case 0xc1: case 0xc2: case 0xc3:
#if HJB
			sprintf(buffer, "ld   r%d,r0", rv);
#else
			sprintf(buffer, "strz,%d", rv);
#endif
			break;
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
#if HJB
			sprintf(buffer, "**** $%02X",op);
#else
			sprintf(buffer, "****   $%02X",op);
#endif
			break;
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
#if HJB
			sprintf(buffer, "ld   (%s),r%d", REL(pc), rv);
#else
			sprintf(buffer, "strr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
#if HJB
			sprintf(buffer, "ld   %s", ABS(0,rv,pc));
#else
			sprintf(buffer, "stra,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
#if HJB
			sprintf(buffer, "rol  r%d", rv);
#else
			sprintf(buffer, "rrl,%d", rv);
#endif
			break;
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
#if HJB
			sprintf(buffer, "out  (%s),r%d", IMM(pc), rv);
#else
			sprintf(buffer, "wrte,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
#if HJB
			sprintf(buffer, "ijnz r%d,%s", rv, REL(pc));
#else
			sprintf(buffer, "birr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
#if HJB
			sprintf(buffer, "ijnz r%d,%s", rv, ADR(pc));
#else
			sprintf(buffer, "bira,%d %s", rv, ADR(pc));
#endif
			pc+=2;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
#if HJB
			sprintf(buffer, "cp   r0,%d", rv);
#else
			sprintf(buffer, "comz,%d", rv);
#endif
			break;
		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
#if HJB
			sprintf(buffer, "cp   r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "comi,%d %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0xe8: case 0xe9: case 0xea: case 0xeb:
#if HJB
			sprintf(buffer, "cp   r%d,(%s)", rv, REL(pc));
#else
			sprintf(buffer, "comr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			break;
		case 0xec: case 0xed: case 0xee: case 0xef:
#if HJB
			sprintf(buffer, "cp   %s", ABS(1,rv,pc));
#else
			sprintf(buffer, "coma,%s", ABS(1,rv,pc));
#endif
			pc+=2;
			break;
		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
#if HJB
			sprintf(buffer, "out  (data),r%d", rv);
#else
			sprintf(buffer, "wrtd,%d", rv);
#endif
			break;
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
#if HJB
			sprintf(buffer, "test r%d,%s", rv, IMM(pc));
#else
			sprintf(buffer, "tmi,%d  %s", rv, IMM(pc));
#endif
			pc+=1;
			break;
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
#if HJB
			sprintf(buffer, "djnz r%d,%s", rv, REL(pc));
#else
			sprintf(buffer, "bdrr,%d %s", rv, REL(pc));
#endif
			pc+=1;
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0xfc: case 0xfd: case 0xfe: case 0xff:
#if HJB
			sprintf(buffer, "djnz r%d,%s", rv, ADR(pc));
#else
			sprintf(buffer, "bdra,%d %s", rv, ADR(pc));
#endif
			pc+=2;
			flags = DASMFLAG_STEP_OVER;
			break;
	}
	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}
