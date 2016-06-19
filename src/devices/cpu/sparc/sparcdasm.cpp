// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SPARC v7 disassembler
*/

#include "emu.h"
#include "sparcdefs.h"

static void print(char *output, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

#define INVALFMT	"%s; op:%08x rs1:%d rd:%d simm13:%d i:%d asi:%d rs2:%d"

static const char * const regnames[32] = {
	"%g0", "%g1", "%g2", "%g3", "%g4", "%g5", "%g6", "%g7",
	"%o0", "%o1", "%o2", "%o3", "%o4", "%o5", "%o6", "%o7",
	"%l0", "%l1", "%l2", "%l3", "%l4", "%l5", "%l6", "%l7",
	"%i0", "%i1", "%i2", "%i3", "%i4", "%i5", "%i6", "%i7"
};

void sparc_dasm_address(char* address, UINT32 op)
{
	char rs1[128];
	memset(rs1, 0, 128);
	bool rs1present = false;
	if (RS1 != 0)
	{
		rs1present = true;
		print(rs1, "%s", regnames[RS1]);
	}

	char rs2[128];
	memset(rs2, 0, 128);
	if (USEIMM)
	{
		if (SIMM13 != 0)
		{
			UINT32 val = SIMM13 & 0x00001fff;
			if (SIMM13 < 0)
			{
				print(rs2, "-0x%x", val);
			}
			else
			{
				print(rs2, "0x%x", val);
			}
		}
		else if (!rs1present)
		{
			print(rs2, "0x0");
		}
	}
	else
	{
		if (RS2 != 0)
		{
			if (rs1present)
			{
				print(rs2, "+%s", regnames[RS2]);
			}
			else
			{
				print(rs2, "%s", regnames[RS2]);
			}
		}
		else if (!rs1present)
		{
			print(rs2, "%s", regnames[RS2]);
		}
	}
	print(address, "[%s%s]", rs1, rs2);
}

void sparc_dasm_fpop1(char *buffer, UINT32 op)
{
	switch (OPF)
	{
		case 0x01:	print(buffer, "fmovs    %%f%d,%%f%d", RS2, RD); break;
		case 0x05:	print(buffer, "fnegs    %%f%d,%%f%d", RS2, RD); break;
		case 0x09:	print(buffer, "fabss    %%f%d,%%f%d", RS2, RD); break;
		case 0x29:	print(buffer, "fsqrts   %%f%d,%%f%d", RS2, RD); break;
		case 0x2a:	print(buffer, "fsqrtd   %%f%d,%%f%d", RS2, RD); break;
		case 0x2b:	print(buffer, "fsqrtq   %%f%d,%%f%d", RS2, RD); break;
		case 0x41:	print(buffer, "fadds    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x42:	print(buffer, "faddd    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x43:	print(buffer, "faddq    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x45:	print(buffer, "fsubs    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x46:	print(buffer, "fsubd    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x47:	print(buffer, "fsubq    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x49:	print(buffer, "fmuls    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x4a:	print(buffer, "fmuld    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x4b:	print(buffer, "fmulq    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x4d:	print(buffer, "fdivs    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x4e:	print(buffer, "fdivd    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x4f:	print(buffer, "fdivq    %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x69:	print(buffer, "fsmuld   %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0x6e:	print(buffer, "fdmulq   %%f%d,%%f%d,%%f%d", RS1, RS2, RD); break;
		case 0xc4:	print(buffer, "fitos    %%f%d,%%f%d", RS2, RD); break;
		case 0xc6:	print(buffer, "fdtos    %%f%d,%%f%d", RS2, RD); break;
		case 0xc7:	print(buffer, "fqtos    %%f%d,%%f%d", RS2, RD); break;
		case 0xc8:	print(buffer, "fitod    %%f%d,%%f%d", RS2, RD); break;
		case 0xc9:	print(buffer, "fstod    %%f%d,%%f%d", RS2, RD); break;
		case 0xcb:	print(buffer, "fqtod    %%f%d,%%f%d", RS2, RD); break;
		case 0xcc:	print(buffer, "fitoq    %%f%d,%%f%d", RS2, RD); break;
		case 0xcd:	print(buffer, "fstoq    %%f%d,%%f%d", RS2, RD); break;
		case 0xce:	print(buffer, "fdtoq    %%f%d,%%f%d", RS2, RD); break;
		case 0xd1:	print(buffer, "fstoi    %%f%d,%%f%d", RS2, RD); break;
		case 0xd2:	print(buffer, "fdtoi    %%f%d,%%f%d", RS2, RD); break;
		case 0xd3:	print(buffer, "fqtoi    %%f%d,%%f%d", RS2, RD); break;
	}
}

void sparc_dasm_ldst(char *buffer, UINT32 op)
{
	static const char * const ldstnames[64] = {
		"ld       ", "ldub     ", "lduh     ", "ldd      ", "st       ", "stb      ", "sth      ", "std      ",
		"invalid  ", "ldsb     ", "ldsh     ", "invalid  ", "invalid  ", "ldstub   ", "invalid  ", "swap     ",
		"lda      ", "lduba    ", "lduha    ", "ldda     ", "sta      ", "stba     ", "stha     ", "stda     ",
		"invalid  ", "ldsba    ", "ldsha    ", "invalid  ", "invalid  ", "ldstuba  ", "invalid  ", "swapa    ",
		"ld       ", "ld       ", "invalid  ", "ldd      ", "st       ", "st       ", "std      ", "std      ",
		"ld       ", "ld       ", "invalid  ", "ldd      ", "st       ", "st       ", "std      ", "std      ",
		"invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ",
		"invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  ", "invalid  "
	};

	static const int LOAD = 0;
	static const int STORE = 1;
	static const int LOAD_ALT = 2;
	static const int STORE_ALT = 3;
	static const int LOAD_PR = 4;
	static const int STORE_PR = 5;
	static const int LDST_SR = 6;
	static const int INVALID = 7;

	static const int ldsttype[64] = {
		LOAD,		LOAD,		LOAD,		LOAD,		STORE,		STORE,		STORE,		STORE,
		INVALID,	LOAD,		LOAD,		INVALID,	INVALID,	LOAD,		INVALID,	LOAD,
		LOAD_ALT,	LOAD_ALT,	LOAD_ALT,	LOAD_ALT,	STORE_ALT,	STORE_ALT,	STORE_ALT,	STORE_ALT,
		INVALID,	LOAD_ALT,	LOAD_ALT,	INVALID,	INVALID,	LOAD_ALT,	INVALID,	LOAD_ALT,
		LOAD_PR,	LDST_SR,	INVALID,	LOAD_PR,	STORE_PR,	LDST_SR,	LDST_SR,	STORE_PR,
		LOAD_PR,	LDST_SR,	INVALID,	LOAD_PR,	STORE_PR,	LDST_SR,	LDST_SR,	STORE_PR,
		INVALID,	INVALID,	INVALID,	INVALID,	INVALID,	INVALID,	INVALID,	INVALID,
		INVALID,	INVALID,	INVALID,	INVALID,	INVALID,	INVALID,	INVALID,	INVALID,
	};

	static const char * const ldstformats[64] = {
		"%s%s,%s",		"%s%s,%s",		"%s%s,%s",		"%s%s,%s",		"%s%s,%s",		"%s%s,%s",		"%s%s,%s",		"%s%s,%s",
		INVALFMT,		"%s%s,%s",		"%s%s,%s",		INVALFMT,		INVALFMT,		"%s%s,%s",		INVALFMT,		"%s%s,%s",
		"%s%s%d,%s",	"%s%s%d,%s",	"%s%s%d,%s",	"%s%s%d,%s",	"%s%s,%s%d",	"%s%s,%s%d",	"%s%s,%s%d",	"%s%s,%s%d",
		INVALFMT,		"%s%s%d,%s",	"%s%s%d,%s",	INVALFMT,		INVALFMT,		"%s%s%d,%s",	INVALFMT,		"%s%s%d,%s",
		"%s%s,%%f%d",	"%s%s,%%fsr",	INVALFMT,		"%s%s,%%f%d",	"%s%%f%d,%s",	"%s%%fsr,%s",	"%s%%fq,%s",	"%s%%f%d,%s",
		"%s%s,%%c%d",	"%s%s,%%csr",	INVALFMT,		"%s%s,%%c%d",	"%s%%c%d,%s",	"%s%%csr,%s",	"%s%%cq,%s",	"%s%%c%d,%s",
		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,
		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,		INVALFMT,
	};

	char address[256];
	sparc_dasm_address(address, op);

	switch (ldsttype[OP3])
	{
		case LOAD:		print(buffer, ldstformats[OP3], ldstnames[OP3], address, regnames[RD]); break;
		case STORE:		print(buffer, ldstformats[OP3], ldstnames[OP3], regnames[RD], address); break;
		case LOAD_ALT:	print(buffer, ldstformats[OP3], ldstnames[OP3], address, ASI, regnames[RD]); break;
		case STORE_ALT:	print(buffer, ldstformats[OP3], ldstnames[OP3], regnames[RD], address, ASI); break;
		case LOAD_PR:	print(buffer, ldstformats[OP3], ldstnames[OP3], address, RD); break;
		case STORE_PR:	print(buffer, ldstformats[OP3], ldstnames[OP3], RD, address); break;
		case LDST_SR:	print(buffer, ldstformats[OP3], ldstnames[OP3], address); break;
		default:		print(buffer, ldstformats[OP3], ldstnames[OP3], op, RS1, RD, SIMM13, USEIMM, ASI, RS2); break;
	}
}

offs_t sparc_dasm(char *buffer, offs_t pc, UINT32 op)
{
	static const char * const branchnames[32] = {
		"bn       ", "be       ", "ble      ", "bl       ", "bleu     ", "bcs      ", "bneg     ", "bvs      ",
		"ba       ", "bne      ", "bg       ", "bge      ", "bgu      ", "bcc      ", "bpos     ", "bvc      ",
		"bn,a     ", "be,a     ", "ble,a    ", "bl,a     ", "bleu,a   ", "bcs,a    ", "bneg,a   ", "bvs,a    ",
		"ba,a     ", "bne,a    ", "bg,a     ", "bge,a    ", "bgu,a    ", "bcc,a    ", "bpos,a   ", "bvc,a    "
	};
	static const char * const fpbranchnames[32] = {
		"fbn      ", "fbne     ", "fblg     ", "fbul     ", "fbl      ", "fbug     ", "fbg      ", "fbu      ",
		"fba      ", "fbe      ", "fbue     ", "fbge     ", "fbuge    ", "fble     ", "fbule    ", "fbo      ",
		"fbn,a    ", "fbne,a   ", "fblg,a   ", "fbul,a   ", "fbl,a    ", "fbug,a   ", "fbg,a    ", "fbu,a    ",
		"fba,a    ", "fbe,a    ", "fbue,a   ", "fbge,a   ", "fbuge,a  ", "fble,a   ", "fbule,a  ", "fbo,a    "
	};
	static const char * const copbranchnames[32] = {
		"cbn      ", "cb123    ", "cb12     ", "cb13     ", "cb1      ", "cb23     ", "cb2      ", "cb3      ",
		"cba      ", "cb0      ", "cb03     ", "cb02     ", "cb023    ", "cb01     ", "cb013    ", "cb012    ",
		"cbn,a    ", "cb123,a  ", "cb12,a   ", "cb13,a   ", "cb1,a    ", "cb23,a   ", "cb2,a    ", "cb3,a    ",
		"cba,a    ", "cb0,a    ", "cb03,a   ", "cb02,a   ", "cb023,a  ", "cb01,a   ", "cb013,a  ", "cb012,a  "
	};
	static const char * const trapnames[16] = {
		"tn       ", "te       ", "tle      ", "tl       ", "tleu     ", "tcs      ", "tneg     ", "tvs      ",
		"ta       ", "tne      ", "tg       ", "tge      ", "tgu      ", "tcc      ", "tpos     ", "tvc      "
	};

	switch (OP)
	{
	case 0:	// Bicc, SETHI, FBfcc
		switch (OP2)
		{
		case 0: // unimp
			print(buffer, "unimp    %06x", CONST22);
			break;
		case 2: // branch on integer condition codes
		case 6: // branch on floating-point condition codes
		case 7: // branch on coprocessor condition codes, SPARCv8
		{
			int index = (ANNUL << 4) | COND;
			char sign[2];
			memset(sign, 0, 2);
			INT32 disp = DISP22;
			if (disp < 0)
			{
				print(sign, "-");
				disp = -disp;
			}
			print(buffer, "%s%s0x%06x ; %08x", (OP2 == 2) ? branchnames[index] : ((OP2 == 6) ? fpbranchnames[index] : copbranchnames[index]), sign, disp, pc + DISP22);
			break;
		}
		case 4:	// SETHI
			if (IMM22 == 0 && RD == 0)
				print(buffer, "nop      ");
			else
				print(buffer, "sethi    %%hi(0x%08x),%s", IMM22, regnames[RD]);
			break;
		default:
			print(buffer, "invalid");
			break;
		}
		break;
	case 1: // CALL
		print(buffer, "call     0x%08x ; %08x", DISP30, DISP30 << 2);
		break;
	case 2:
		if (USEIMM)
		{
			switch (OP3)
			{
			case 0:
				if (RS1 == RD)
					if (SIMM13 == 1)
						print(buffer, "inc      %s", regnames[RD]);
					else
						print(buffer, "inc      %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "add      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 1:		print(buffer, "and      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 2:
				if (RS1 == 0)
					print(buffer, "mov      %d,%s", SIMM13, regnames[RD]);
				else if (RS1 == RD)
					print(buffer, "bset     %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "or       %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 3:
				if (RS1 == RD)
					print(buffer, "btog     %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "xor      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 4:
				if (RS1 == RD)
					if (SIMM13 == 1)
						print(buffer, "dec      %s", regnames[RD]);
					else
						print(buffer, "dec      %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "sub      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 5:
				if (RS1 == RD)
					print(buffer, "bclr     %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "andn     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 6:		print(buffer, "orn      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 7:
				if (SIMM13 == 0)
					if (RS1 == RD)
						print(buffer, "not      %s", regnames[RD]);
					else
						print(buffer, "not      %s,%s", regnames[RS1], regnames[RD]);
				else
					print(buffer, "xnor     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 8:		print(buffer, "addx     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 10:	print(buffer, "umul     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 11:	print(buffer, "smul     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 12:	print(buffer, "subx     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 14:	print(buffer, "udiv     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 15:	print(buffer, "sdiv     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 16:
				if (RS1 == RD)
					if (SIMM13 == 1)
						print(buffer, "inccc    %s", regnames[RD]);
					else
						print(buffer, "inccc    %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "addcc    %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 17:
				if (RD == 0)
					print(buffer, "btst     %d,%s", SIMM13, regnames[RS1]);
				else
					print(buffer, "andcc    %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 18:	print(buffer, "orcc     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 19:	print(buffer, "xorcc    %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 20:
				if (RD == 0)
					print(buffer, "cmp      %s,%d", regnames[RS1], SIMM13);
				else if (RS1 == RD)
					if (SIMM13 == 1)
						print(buffer, "deccc    %s", regnames[RD]);
					else
						print(buffer, "deccc    %d,%s", SIMM13, regnames[RD]);
				else
					print(buffer, "subcc    %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 21:	print(buffer, "andncc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 22:	print(buffer, "orncc    %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 23:	print(buffer, "xnorcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 24:	print(buffer, "addxcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 26:	print(buffer, "umulcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 27:	print(buffer, "smulcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 28:	print(buffer, "subxcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 30:	print(buffer, "udivcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 31:	print(buffer, "sdivcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break; // SPARCv8
			case 32:	print(buffer, "taddcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 33:	print(buffer, "tsubcc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 34:	print(buffer, "taddcctv %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 35:	print(buffer, "tsubcctv %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 36:	print(buffer, "mulscc   %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 37:	print(buffer, "sll      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 38:	print(buffer, "srl      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 39:	print(buffer, "sra      %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 40:
				if (RS1 == 0)
					print(buffer, "rd       %%y,%s", regnames[RD]);
				else if (RS1 == 15 && RD == 0)
					print(buffer, "stbar    "); // SPARCv8
				else
					print(buffer, "rd       %%asr%d,%s", RS1, regnames[RD]); // SPARCv8
				break;
			case 41:	print(buffer, "rd       %%psr,%s", regnames[RD]); break;
			case 42:	print(buffer, "rd       %%wim,%s", regnames[RD]); break;
			case 43:	print(buffer, "rd       %%tbr,%s", regnames[RD]); break;
			case 48:
				if (RD == 0)
					print(buffer, "wr       %s,%d,%%y", regnames[RS1], SIMM13);
				else
					print(buffer, "wr		%s,%d,%%asr%d", regnames[RS1], SIMM13, RD);
				break;
			case 49:
				if (RS1 == 0)
					print(buffer, "wr       %d,%%psr", SIMM13);
				else
					print(buffer, "wr       %s,%d,%%psr", regnames[RS1], SIMM13);
				break;
			case 50:
				if (RS1 == 0)
					print(buffer, "wr       %d,%%wim", SIMM13);
				else
					print(buffer, "wr       %s,%d,%%wim", regnames[RS1], SIMM13);
				break;
			case 51:
				if (RS1 == 0)
					print(buffer, "wr       %d,%%tbr", SIMM13);
				else
					print(buffer, "wr       %s,%d,%%tbr", regnames[RS1], SIMM13);
				break;
			case 52: // FPop1
				sparc_dasm_fpop1(buffer, op);
				break;
			case 53: // FPop2
				switch (OPF)
				{
					case 0x51:	print(buffer, "fcmps    %%f%d,%%f%d", RS1, RS2); break;
					case 0x52:	print(buffer, "fcmpd    %%f%d,%%f%d", RS1, RS2); break;
					case 0x53:	print(buffer, "fcmpq    %%f%d,%%f%d", RS1, RS2); break;
					case 0x55:	print(buffer, "fcmpes   %%f%d,%%f%d", RS1, RS2); break;
					case 0x56:	print(buffer, "fcmped   %%f%d,%%f%d", RS1, RS2); break;
					case 0x57:	print(buffer, "fcmpeq   %%f%d,%%f%d", RS1, RS2); break;
				}
				break;
			case 56:
				if (RS1 == 31 && SIMM13 == 8)
					print(buffer, "ret      ");
				else if (RS1 == 15 && SIMM13 == 8)
					print(buffer, "retl     ");
				else if (RD == 0)
					print(buffer, "jmp      %s,%d", regnames[RS1], SIMM13);
				else if (RD == 15)
					print(buffer, "call     %s,%d", regnames[RS1], SIMM13);
				else
					print(buffer, "jmpl     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]);
				break;
			case 57:
			{
				char address[256];
				sparc_dasm_address(address, op);
				print(buffer, "rett     %s", address);
				break;
			}
			case 58:	print(buffer, "%s%s,%d,%s", trapnames[COND], regnames[RS1], IMM7, regnames[RD]); break;
			case 59:
				if (RD == 0)
					print(buffer, "flush    %s.%d", regnames[RS1], SIMM13); // SPARCv8
				else
					print(buffer, "invalid  ");
				break;
			case 60:	print(buffer, "save     %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			case 61:	print(buffer, "restore  %s,%d,%s", regnames[RS1], SIMM13, regnames[RD]); break;
			default:	print(buffer, "invalid  "); break;
			}
		}
		else
		{
			switch (OP3)
			{
			case 0:		print(buffer, "add      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 1:		print(buffer, "and      %s,%d,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 2:
				if (RS1 == 0)
					print(buffer, "mov      %s,%s", regnames[RS2], regnames[RD]);
				else if (RS1 == RD)
					print(buffer, "bset     %s,%s", regnames[RS2], regnames[RD]);
				else
					print(buffer, "or       %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 3:
				if (RS1 == RD)
					print(buffer, "btog     %s,%s", regnames[RS2], regnames[RD]);
				else
					print(buffer, "xor      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 4:		print(buffer, "sub      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 5:
				if (RS1 == RD)
					print(buffer, "bclr     %s,%s", regnames[RS2], regnames[RD]);
				else
					print(buffer, "andn     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 6:		print(buffer, "orn      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 7:
				if (RS2 == 0)
					if (RS1 == RD)
						print(buffer, "not      %s", regnames[RD]);
					else
						print(buffer, "not      %s,%s", regnames[RS1], regnames[RD]);
				else
					print(buffer, "xnor     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 8:		print(buffer, "addx     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 10:	print(buffer, "umul     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 11:	print(buffer, "smul     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 12:	print(buffer, "subx     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 14:	print(buffer, "udiv     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 15:	print(buffer, "sdiv     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 16:	print(buffer, "addcc    %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 17:
				if (RD == 0)
					print(buffer, "btst     %s,%s", regnames[RS2], regnames[RS1]);
				else
					print(buffer, "andcc    %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 18:	print(buffer, "orcc     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 19:	print(buffer, "xorcc    %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 20:
				if (RD == 0)
					print(buffer, "cmp      %s,%s", regnames[RS1], regnames[RS2]);
				else
					print(buffer, "subcc    %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 21:	print(buffer, "andncc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 22:	print(buffer, "orncc    %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 23:	print(buffer, "xnorcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 24:	print(buffer, "addxcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 26:	print(buffer, "umulcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 27:	print(buffer, "smulcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 28:	print(buffer, "subxcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 30:	print(buffer, "udivcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 31:	print(buffer, "sdivcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break; // SPARCv8
			case 32:	print(buffer, "taddcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 33:	print(buffer, "tsubcc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 34:	print(buffer, "taddcctv %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 35:	print(buffer, "tsubcctv %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 36:	print(buffer, "mulscc   %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 37:	print(buffer, "sll      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 38:	print(buffer, "srl      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 39:	print(buffer, "sra      %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 40:
				if (RS1 == 0)
					print(buffer, "rd       %%y,%s", regnames[RD]);
				else if (RS1 == 15 && RD == 0)
					print(buffer, "stbar    "); // SPARCv8
				else
					print(buffer, "rd       %%asr%d,%s", RS1, regnames[RD]); // SPARCv8
				break;
			case 41:	print(buffer, "rd       %%psr,%s", regnames[RD]); break;
			case 42:	print(buffer, "rd       %%wim,%s", regnames[RD]); break;
			case 43:	print(buffer, "rd       %%tbr,%s", regnames[RD]); break;
			case 48:
				if (RD == 0)
					print(buffer, "wr       %s,%s,%%y", regnames[RS1], regnames[RS2]);
				else
					print(buffer, "wr		%s,%s,%%asr%d", regnames[RS1], regnames[RS2], RD);
				break;
			case 49:	print(buffer, "wr       %s,%s,%%psr", regnames[RS1], regnames[RS2]); break;
			case 50:	print(buffer, "wr       %s,%s,%%wim", regnames[RS1], regnames[RS2]); break;
			case 51:	print(buffer, "wr       %s,%s,%%tbr", regnames[RS1], regnames[RS2]); break;
			case 52: // FPop1
				sparc_dasm_fpop1(buffer, op);
				break;
			case 53: // FPop2
				switch (OPF)
				{
					case 0x51:	print(buffer, "fcmps    %%f%d,%%f%d", RS1, RS2); break;
					case 0x52:	print(buffer, "fcmpd    %%f%d,%%f%d", RS1, RS2); break;
					case 0x53:	print(buffer, "fcmpq    %%f%d,%%f%d", RS1, RS2); break;
					case 0x55:	print(buffer, "fcmpes   %%f%d,%%f%d", RS1, RS2); break;
					case 0x56:	print(buffer, "fcmped   %%f%d,%%f%d", RS1, RS2); break;
					case 0x57:	print(buffer, "fcmpeq   %%f%d,%%f%d", RS1, RS2); break;
				}
				break;
			case 56:
				if (RD == 0)
					if (RS1 == 0 && RS2 == 0)
						print(buffer, "jmp      %s", regnames[RS1]);
					else if (RS1 == 0)
						print(buffer, "jmp      %s", regnames[RS2]);
					else if (RS2 == 0)
						print(buffer, "jmp      %s", regnames[RS1]);
					else
						print(buffer, "jmp      %s,%s", regnames[RS1], regnames[RS2]);
				else if (RD == 15)
					print(buffer, "call     %s,%s", regnames[RS1], regnames[RS2]);
				else
					print(buffer, "jmpl     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 57:	print(buffer, "rett     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 58:	print(buffer, "%s%s,%s,%s", trapnames[COND], regnames[RS1], regnames[RS2], regnames[RD]); break;
			case 59:
				if (RD == 0)
					print(buffer, "flush    %s.%s", regnames[RS1], regnames[RS2]); // SPARCv8
				else
					print(buffer, "invalid  ");
				break;
			case 60:
				if (RS1 == RS2 && RS2 == RD && RD == 0)
					print(buffer, "save     ");
				else
					print(buffer, "save     %s,%s,%s", regnames[RS1], regnames[RS2], regnames[RD]);
				break;
			case 61:
				if (RS1 == RS2 && RS2 == RD && RD == 0)
					print(buffer, "restore  ");
				else
					print(buffer, "restore  %s,%s,%s", regnames[RS1], regnames[RS1], regnames[RD]);
				break;
			default:	print(buffer, "invalid  "); break;
			}
		}
		break;
	case 3: // loads, stores
		sparc_dasm_ldst(buffer, op);
		break;
	default:
		print(buffer, "invalid  ");
		break;
	}

	return 4 | DASMFLAG_SUPPORTED;
}

/*****************************************************************************/

CPU_DISASSEMBLE( sparc )
{
	UINT32 op = *(UINT32 *)oprom;
	op = BIG_ENDIANIZE_INT32(op);
	return sparc_dasm(buffer, pc, op);
}
