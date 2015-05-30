// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "e0c6200.h"

// opcode mnemonics
enum e_mnemonics
{
	em_JP, em_RETD, em_CALL, em_CALZ,
	em_LD, em_LBPX, em_ADC, em_CP, em_ADD, em_SUB, em_SBC, em_AND, em_OR, em_XOR,
	em_RLC, em_FAN, em_PSET, em_LDPX, em_LDPY, em_SET, em_RST, em_INC, em_DEC,
	em_RRC, em_ACPX, em_ACPY, em_SCPX, em_SCPY, em_PUSH, em_POP,
	em_RETS, em_RET, em_JPBA, em_HALT, em_SLP, em_NOP5, em_NOP7,
	em_NOT, em_SCF, em_SZF, em_SDF, em_EI, em_DI, em_RDF, em_RZF, em_RCF, em_ILL
};

static const char *const em_name[] =
{
	"JP", "RETD", "CALL", "CALZ",
	"LD", "LBPX", "ADC", "CP", "ADD", "SUB", "SBC", "AND", "OR", "XOR",
	"RLC", "FAN", "PSET", "LDPX", "LDPY", "SET", "RST", "INC", "DEC",
	"RRC", "ACPX", "ACPY", "SCPX", "SCPY", "PUSH", "POP",
	"RETS", "RET", "JPBA", "HALT", "SLP", "NOP5", "NOP7",
	"NOT", "SCF", "SZF", "SDF", "EI", "DI", "RDF", "RZF", "RCF", "?"
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 em_flags[] =
{
	0, _OUT, _OVER, _OVER,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	_OUT, _OUT, 0, _OVER, _OVER, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


// opcode params
enum e_params
{
	ep_S, ep_E, ep_I, ep_R0, ep_R2, ep_R4, ep_Q,
	ep_cC, ep_cNC, ep_cZ, ep_cNZ,
	ep_A, ep_B, ep_X, ep_Y, ep_MX, ep_MY, ep_XP, ep_XH, ep_XL, ep_YP, ep_YH, ep_YL,
	ep_P, ep_F, ep_MN, ep_SP, ep_SPH, ep_SPL
};

// 0-digit is number of bits per opcode parameter, 0 bits is literal,
// 0x10-digit is for shift-right, 0x100-digit is special flag for r/q param
static const UINT16 ep_bits[] =
{
	8, 8, 4, 0x102, 0x122, 0x142, 0x102,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 4, 0, 0, 0
};

// redirect for r/q param
static const UINT8 ep_redirect_r[4] = { ep_A, ep_B, ep_MX, ep_MY };

// literal opcode parameter
static const char *const ep_name[] =
{
	" ", " ", " ", " ", " ", " ", " ",
	"C", "NC", "Z", "NZ",
	"A", "B", "X", "Y", "MX", "MY", "XP", "XH", "XL", "YP", "YH", "YL",
	" ", "F", " ", "SP", "SPH", "SPL"
};


static char* decode_param(UINT16 opcode, int param, char* buffer)
{
	int bits = ep_bits[param] & 0xf;
	int shift = ep_bits[param] >> 4 & 0xf;
	UINT16 opmask = opcode >> shift & ((1 << bits) - 1);

	// redirect r/q to A/B/MX/MY
	if (ep_bits[param] & 0x100)
		param = ep_redirect_r[opmask];

	// literal param
	if (ep_bits[param] == 0)
	{
		strcpy(buffer, ep_name[param]);
		return buffer;
	}

	// print value like how it's documented in the manual
	char val[10];
	if (bits > 4 || opmask > 9)
		sprintf(val, "%02XH", opmask);
	else
		sprintf(val, "%d", opmask);

	if (param == ep_MN)
		sprintf(buffer, "M%s", val);
	else
		strcpy(buffer, val);

	return buffer;
}


CPU_DISASSEMBLE(e0c6200)
{
	UINT16 op = (oprom[1] | oprom[0] << 8) & 0xfff;

	int m = -1;
	int p1 = -1;
	int p2 = -1;

	// determine id for mnemonic and param(s)
	switch (op & 0xf00)
	{
		// JP s
		case 0x000:
			m = em_JP; p1 = ep_S;
			break;

		// RETD e
		case 0x100:
			m = em_RETD; p1 = ep_E;
			break;

		// JP C,s
		case 0x200:
			m = em_JP; p1 = ep_cC; p2 = ep_S;
			break;

		// JP NC,s
		case 0x300:
			m = em_JP; p1 = ep_cNC; p2 = ep_S;
			break;

		// CALL s
		case 0x400:
			m = em_CALL; p1 = ep_S;
			break;

		// CALZ s
		case 0x500:
			m = em_CALZ; p1 = ep_S;
			break;

		// JP Z,s
		case 0x600:
			m = em_JP; p1 = ep_cZ; p2 = ep_S;
			break;

		// JP NZ,s
		case 0x700:
			m = em_JP; p1 = ep_cNZ; p2 = ep_S;
			break;

		// LD Y,e
		case 0x800:
			m = em_LD; p1 = ep_Y; p2 = ep_E;
			break;

		// LBPX MX,e
		case 0x900:
			m = em_LBPX; p1 = ep_MX; p2 = ep_E;
			break;

		// LD X,e
		case 0xb00:
			m = em_LD; p1 = ep_X; p2 = ep_E;
			break;


		default:
			switch (op)
			{
		// RLC r
		case 0xaf0: case 0xaf5: case 0xafa: case 0xaff:
		m = em_RLC; p1 = ep_R0;
		break;

		// NOT r
		case 0xd0f: case 0xd1f: case 0xd2f: case 0xd3f:
			m = em_NOT; p1 = ep_R4;
			break;

		// LD XP,r
		case 0xe80: case 0xe81: case 0xe82: case 0xe83:
			m = em_LD; p1 = ep_XP; p2 = ep_R0;
			break;

		// LD XH,r
		case 0xe84: case 0xe85: case 0xe86: case 0xe87:
			m = em_LD; p1 = ep_XH; p2 = ep_R0;
			break;

		// LD XL,r
		case 0xe88: case 0xe89: case 0xe8a: case 0xe8b:
			m = em_LD; p1 = ep_XL; p2 = ep_R0;
			break;

		// RRC r
		case 0xe8c: case 0xe8d: case 0xe8e: case 0xe8f:
			m = em_RRC; p1 = ep_R0;
			break;

		// LD YP,r
		case 0xe90: case 0xe91: case 0xe92: case 0xe93:
			m = em_LD; p1 = ep_YP; p2 = ep_R0;
			break;

		// LD YH,r
		case 0xe94: case 0xe95: case 0xe96: case 0xe97:
			m = em_LD; p1 = ep_YH; p2 = ep_R0;
			break;

		// LD YL,r
		case 0xe98: case 0xe99: case 0xe9a: case 0xe9b:
			m = em_LD; p1 = ep_YL; p2 = ep_R0;
			break;

		// LD r,XP
		case 0xea0: case 0xea1: case 0xea2: case 0xea3:
			m = em_LD; p1 = ep_R0; p2 = ep_XP;
			break;

		// LD r,XH
		case 0xea4: case 0xea5: case 0xea6: case 0xea7:
			m = em_LD; p1 = ep_R0; p2 = ep_XH;
			break;

		// LD r,XL
		case 0xea8: case 0xea9: case 0xeaa: case 0xeab:
			m = em_LD; p1 = ep_R0; p2 = ep_XL;
			break;

		// LD r,YP
		case 0xeb0: case 0xeb1: case 0xeb2: case 0xeb3:
			m = em_LD; p1 = ep_R0; p2 = ep_YP;
			break;

		// LD r,YH
		case 0xeb4: case 0xeb5: case 0xeb6: case 0xeb7:
			m = em_LD; p1 = ep_R0; p2 = ep_YH;
			break;

		// LD r,YL
		case 0xeb8: case 0xeb9: case 0xeba: case 0xebb:
			m = em_LD; p1 = ep_R0; p2 = ep_YL;
			break;

		// INC X
		case 0xee0:
			m = em_INC; p1 = ep_X;
			break;

		// INC Y
		case 0xef0:
			m = em_INC; p1 = ep_Y;
			break;

		// ACPX MX,r
		case 0xf28: case 0xf29: case 0xf2a: case 0xf2b:
			m = em_ACPX; p1 = ep_MX; p2 = ep_R0;
			break;

		// ACPY MY,r
		case 0xf2c: case 0xf2d: case 0xf2e: case 0xf2f:
			m = em_ACPY; p1 = ep_MY; p2 = ep_R0;
			break;

		// SCPX MX,r
		case 0xf38: case 0xf39: case 0xf3a: case 0xf3b:
			m = em_SCPX; p1 = ep_MX; p2 = ep_R0;
			break;

		// SCPY MY,r
		case 0xf3c: case 0xf3d: case 0xf3e: case 0xf3f:
			m = em_SCPY; p1 = ep_MY; p2 = ep_R0;
			break;

		// SCF
		case 0xf41:
			m = em_SCF;
			break;

		// SZF
		case 0xf42:
			m = em_SZF;
			break;

		// SDF
		case 0xf44:
			m = em_SDF;
			break;

		// EI
		case 0xf48:
			m = em_EI;
			break;

		// DI
		case 0xf57:
			m = em_DI;
			break;

		// RDF
		case 0xf5b:
			m = em_RDF;
			break;

		// RZF
		case 0xf5d:
			m = em_RZF;
			break;

		// RCF
		case 0xf5e:
			m = em_RCF;
			break;

		// PUSH r
		case 0xfc0: case 0xfc1: case 0xfc2: case 0xfc3:
			m = em_PUSH; p1 = ep_R0;
			break;

		// PUSH XP
		case 0xfc4:
			m = em_PUSH; p1 = ep_XP;
			break;

		// PUSH XH
		case 0xfc5:
			m = em_PUSH; p1 = ep_XH;
			break;

		// PUSH XL
		case 0xfc6:
			m = em_PUSH; p1 = ep_XL;
			break;

		// PUSH YP
		case 0xfc7:
			m = em_PUSH; p1 = ep_YP;
			break;

		// PUSH YH
		case 0xfc8:
			m = em_PUSH; p1 = ep_YH;
			break;

		// PUSH YL
		case 0xfc9:
			m = em_PUSH; p1 = ep_YL;
			break;

		// PUSH F
		case 0xfca:
			m = em_PUSH; p1 = ep_F;
			break;

		// DEC SP
		case 0xfcb:
			m = em_DEC; p1 = ep_SP;
			break;

		// POP r
		case 0xfd0: case 0xfd1: case 0xfd2: case 0xfd3:
			m = em_POP; p1 = ep_R0;
			break;

		// POP XP
		case 0xfd4:
			m = em_POP; p1 = ep_XP;
			break;

		// POP XH
		case 0xfd5:
			m = em_POP; p1 = ep_XH;
			break;

		// POP XL
		case 0xfd6:
			m = em_POP; p1 = ep_XL;
			break;

		// POP YP
		case 0xfd7:
			m = em_POP; p1 = ep_YP;
			break;

		// POP YH
		case 0xfd8:
			m = em_POP; p1 = ep_YH;
			break;

		// POP YL
		case 0xfd9:
			m = em_POP; p1 = ep_YL;
			break;

		// POP F
		case 0xfda:
			m = em_POP; p1 = ep_F;
			break;

		// INC SP
		case 0xfdb:
			m = em_INC; p1 = ep_SP;
			break;

		// RETS
		case 0xfde:
			m = em_RETS;
			break;

		// RET
		case 0xfdf:
			m = em_RET;
			break;

		// LD SPH,r
		case 0xfe0: case 0xfe1: case 0xfe2: case 0xfe3:
			m = em_LD; p1 = ep_SPH; p2 = ep_R0;
			break;

		// LD r,SPH
		case 0xfe4: case 0xfe5: case 0xfe6: case 0xfe7:
			m = em_LD; p1 = ep_R0; p2 = ep_SPH;
			break;

		// JPBA
		case 0xfe8:
			m = em_JPBA;
			break;

		// LD SPL,r
		case 0xff0: case 0xff1: case 0xff2: case 0xff3:
			m = em_LD; p1 = ep_SPL; p2 = ep_R0;
			break;

		// LD r,SPL
		case 0xff4: case 0xff5: case 0xff6: case 0xff7:
			m = em_LD; p1 = ep_R0; p2 = ep_SPL;
			break;

		// HALT
		case 0xff8:
			m = em_HALT;
			break;

		// SLP
		case 0xff9:
			m = em_SLP;
			break;

		// NOP5
		case 0xffb:
			m = em_NOP5;
			break;

		// NOP7
		case 0xfff:
			m = em_NOP7;
			break;


		default:
			switch (op & 0xff0)
			{
		// ADC XH,i
		case 0xa00:
			m = em_ADC; p1 = ep_XH; p2 = ep_I;
			break;

		// ADC XL,i
		case 0xa10:
			m = em_ADC; p1 = ep_XL; p2 = ep_I;
			break;

		// ADC YH,i
		case 0xa20:
			m = em_ADC; p1 = ep_YH; p2 = ep_I;
			break;

		// ADC YL,i
		case 0xa30:
			m = em_ADC; p1 = ep_YL; p2 = ep_I;
			break;

		// CP XH,i
		case 0xa40:
			m = em_CP; p1 = ep_XH; p2 = ep_I;
			break;

		// CP XL,i
		case 0xa50:
			m = em_CP; p1 = ep_XL; p2 = ep_I;
			break;

		// CP YH,i
		case 0xa60:
			m = em_CP; p1 = ep_YH; p2 = ep_I;
			break;

		// CP YL,i
		case 0xa70:
			m = em_CP; p1 = ep_YL; p2 = ep_I;
			break;

		// ADD r,q
		case 0xa80:
			m = em_ADD; p1 = ep_R2; p2 = ep_Q;
			break;

		// ADC r,q
		case 0xa90:
			m = em_ADC; p1 = ep_R2; p2 = ep_Q;
			break;

		// SUB r,q
		case 0xaa0:
			m = em_SUB; p1 = ep_R2; p2 = ep_Q;
			break;

		// SBC r,q
		case 0xab0:
			m = em_SBC; p1 = ep_R2; p2 = ep_Q;
			break;

		// AND r,q
		case 0xac0:
			m = em_AND; p1 = ep_R2; p2 = ep_Q;
			break;

		// OR r,q
		case 0xad0:
			m = em_OR; p1 = ep_R2; p2 = ep_Q;
			break;

		// XOR r,q
		case 0xae0:
			m = em_XOR; p1 = ep_R2; p2 = ep_Q;
			break;

		// ADD r,i
		case 0xc00: case 0xc10: case 0xc20: case 0xc30:
			m = em_ADD; p1 = ep_R4; p2 = ep_I;
			break;

		// ADC r,i
		case 0xc40: case 0xc50: case 0xc60: case 0xc70:
			m = em_ADC; p1 = ep_R4; p2 = ep_I;
			break;

		// AND r,i
		case 0xc80: case 0xc90: case 0xca0: case 0xcb0:
			m = em_AND; p1 = ep_R4; p2 = ep_I;
			break;

		// OR r,i
		case 0xcc0: case 0xcd0: case 0xce0: case 0xcf0:
			m = em_OR; p1 = ep_R4; p2 = ep_I;
			break;

		// XOR r,i
		case 0xd00: case 0xd10: case 0xd20: case 0xd30:
			m = em_XOR; p1 = ep_R4; p2 = ep_I;
			break;

		// SBC r,i
		case 0xd40: case 0xd50: case 0xd60: case 0xd70:
			m = em_SBC; p1 = ep_R4; p2 = ep_I;
			break;

		// FAN r,i
		case 0xd80: case 0xd90: case 0xda0: case 0xdb0:
			m = em_FAN; p1 = ep_R4; p2 = ep_I;
			break;

		// CP r,i
		case 0xdc0: case 0xdd0: case 0xde0: case 0xdf0:
			m = em_CP; p1 = ep_R4; p2 = ep_I;
			break;

		// LD r,i
		case 0xe00: case 0xe10: case 0xe20: case 0xe30:
			m = em_LD; p1 = ep_R4; p2 = ep_I;
			break;

		// PSET p
		case 0xe40: case 0xe50:
			m = em_PSET; p1 = ep_P;
			break;

		// LDPX MX,i
		case 0xe60:
			m = em_LDPX; p1 = ep_MX; p2 = ep_I;
			break;

		// LDPY MY,i
		case 0xe70:
			m = em_LDPY; p1 = ep_MY; p2 = ep_I;
			break;

		// LD r,q
		case 0xec0:
			m = em_LD; p1 = ep_R2; p2 = ep_Q;
			break;

		// LDPX r,q
		case 0xee0:
			m = em_LDPX; p1 = ep_R2; p2 = ep_Q;
			break;

		// LDPY r,q
		case 0xef0:
			m = em_LDPY; p1 = ep_R2; p2 = ep_Q;
			break;

		// CP r,q
		case 0xf00:
			m = em_CP; p1 = ep_R2; p2 = ep_Q;
			break;

		// FAN r,q
		case 0xf10:
			m = em_FAN; p1 = ep_R2; p2 = ep_Q;
			break;

		// SET F,i
		case 0xf40:
			m = em_SET; p1 = ep_F; p2 = ep_I;
			break;

		// RST F,i
		case 0xf50:
			m = em_RST; p1 = ep_F; p2 = ep_I;
			break;

		// INC Mn
		case 0xf60:
			m = em_INC; p1 = ep_MN;
			break;

		// DEC Mn
		case 0xf70:
			m = em_DEC; p1 = ep_MN;
			break;

		// LD Mn,A
		case 0xf80:
			m = em_LD; p1 = ep_MN; p2 = ep_A;
			break;

		// LD Mn,B
		case 0xf90:
			m = em_LD; p1 = ep_MN; p2 = ep_B;
			break;

		// LD A,Mn
		case 0xfa0:
			m = em_LD; p1 = ep_A; p2 = ep_MN;
			break;

		// LD B,Mn
		case 0xfb0:
			m = em_LD; p1 = ep_B; p2 = ep_MN;
			break;


		// illegal opcode
		default:
			m = em_ILL;
			break;

			} // 0xff0
			break;

			} // 0xfff
			break;

	} // 0xf00 (big switch)


	// fetch mnemonic
	char *dst = buffer;
	dst += sprintf(dst, "%-6s", em_name[m]);

	// fetch param(s)
	char pbuffer[10];
	if (p1 != -1)
	{
		dst += sprintf(dst, "%s", decode_param(op, p1, pbuffer));
		if (p2 != -1)
		{
			dst += sprintf(dst, ",%s", decode_param(op, p2, pbuffer));
		}
	}

	return 1 | em_flags[m] | DASMFLAG_SUPPORTED;
}
