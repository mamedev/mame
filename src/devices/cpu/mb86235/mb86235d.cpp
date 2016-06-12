// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi, Ville Linde
#include "emu.h"
#include "debugger.h"
#include "mb86235.h"

static const char *regname[128] =
{
	"MA0",	"MA1",	"MA2",	"MA3",	"MA4",	"MA5",	"MA6",	"MA7",
	"AA0",	"AA1",	"AA2",	"AA3",	"AA4",	"AA5",	"AA6",	"AA7",
	"EB",	"EBU",	"EBL",	"EO",	"SP",	"ST",	"MOD",	"LRPC",
	"AR0",	"AR1",	"AR2",	"AR3",	"AR4",	"AR5",	"AR6",	"AR7",
	"MB0",	"MB1",	"MB2",	"MB3",	"MB4",	"MB5",	"MB6",	"MB7",
	"AB0",	"AB1",	"AB2",	"AB3",	"AB4",	"AB5",	"AB6",	"AB7",
	"PR",	"FI",	"FO0",	"FO1",	"PDR",	"DDR",	"PRP",	"PWP",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???"
};

static const char *db_mnemonic[64] =
{
	"DBMN",   "DBMZ",   "DBMV",    "DBMU",    "DBZD",    "DBNR",    "DBIL",    "DBZC",
	"DBAN",   "DBAZ",   "DBAV",    "DBAU",    "DBMD",    "DBAD",    "???",     "???",
	"DBF0",   "DBF1",   "DBF2",    "???",     "DBIFF",   "DBIFE",   "DBOFF",   "DBOFE",
	"DBIF",   "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???"
};

static const char *dbn_mnemonic[64] =
{
	"DBNMN",  "DBNMZ",  "DBNMV",   "DBNMU",   "DBNZD",   "DBNNR",   "DBNIL",   "DBNZC",
	"DBNAN",  "DBNAZ",  "DBNAV",   "DBNAU",   "DBNMD",   "DBNAD",   "???",     "???",
	"DBNF0",  "DBNF1",  "DBNF2",   "???",     "DBNIFF",  "DBNIFE",  "DBNOFF",  "DBNOFE",
	"DBNIF",  "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???"
};

static const char *dc_mnemonic[64] =
{
	"DCMN",   "DCMZ",   "DCMV",    "DCMU",    "DCZD",    "DCNR",    "DCIL",    "DCZC",
	"DCAN",   "DCAZ",   "DCAV",    "DCAU",    "DCMD",    "DCAD",    "???",     "???",
	"DCF0",   "DCF1",   "DCF2",    "???",     "DCIFF",   "DCIFE",   "DCOFF",   "DCOFE",
	"DCIF",   "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???"
};

static const char *dcn_mnemonic[64] =
{
	"DCNMN",  "DCNMZ",  "DCNMV",   "DCNMU",   "DCNZD",   "DCNNR",   "DCNIL",   "DCNZC",
	"DCNAN",  "DCNAZ",  "DCNAV",   "DCNAU",   "DCNMD",   "DCNAD",   "???",     "???",
	"DCNF0",  "DCNF1",  "DCNF2",   "???",     "DCNIFF",  "DCNIFE",  "DCNOFF",  "DCNOFE",
	"DCNIF",  "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???",
	"???",    "???",    "???",     "???",     "???",     "???",     "???",     "???"
};

static const char *mi1_field[16] =
{ "MA0", "MA1", "MA2", "MA3", "MA4", "MA5", "MA6", "MA7", "MB0", "MB1", "MB2", "MB3", "MB4", "MB5", "MB6", "MB7" };

static const char *mi2_field[32] =
{ "MA0", "MA1", "MA2", "MA3", "MA4", "MA5", "MA6", "MA7", "MB0", "MB1", "MB2", "MB3", "MB4", "MB5", "MB6", "MB7",
  "PR", "PR++", "PR--", "PR#0", "???", "???", "???", "???", "-1.0E+0", "0.0E+0", "0.5E+0", "1.0E+0", "1.5E+0", "2.0E+0", "3.0E+0", "5.0E+0" };

static const char *mo_field[32] = 
{ "MA0", "MA1", "MA2", "MA3", "MA4", "MA5", "MA6", "MA7", "MB0", "MB1", "MB2", "MB3", "MB4", "MB5", "MB6", "MB7",
  "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7" };

static const char *ai1_field[16] = 
{ "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7" };

static const char *ai2_field[32] =
{ "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7",
  "PR", "PR++", "PR--", "PR#0", "???", "???", "???", "???", "0", "1", "-1", "???", "???", "???", "???", "???" };

static const char *ai2f_field[32] =
{ "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7",
  "PR", "PR++", "PR--", "PR#0", "???", "???", "???", "???", "-1.0E+0", "0.0E+0", "0.5E+0", "1.0E+0", "1.5E+0", "2.0E+0", "3.0E+0", "5.0E+0" };

static char* get_ea(int md, int arx, int ary, int disp)
{
	static char buffer[40];
	char *p = buffer;

	switch (md)
	{
		case 0x0: p += sprintf(p, "@AR%d", arx); break;
		case 0x1: p += sprintf(p, "@AR%d++", arx); break;
		case 0x2: p += sprintf(p, "@AR%d--", arx); break;
		case 0x3: p += sprintf(p, "@AR%d++%04X", arx, disp); break;
		case 0x4: p += sprintf(p, "@AR%d+AR%d", arx, ary); break;
		case 0x5: p += sprintf(p, "@AR%d+AR%d++", arx, ary); break;
		case 0x6: p += sprintf(p, "@AR%d+AR%d--", arx, ary); break;
		case 0x7: p += sprintf(p, "@AR%d+AR%d++%04X", arx, ary, disp); break;
		case 0x8: p += sprintf(p, "@AR%d+AR%dU", arx, ary); break;
		case 0x9: p += sprintf(p, "@AR%d+AR%dL", arx, ary); break;
		case 0xa: p += sprintf(p, "@AR%d+%04X", arx, disp); break;
		case 0xb: p += sprintf(p, "@AR%d+AR%d+%04X", arx, ary, disp); break;
		case 0xc: p += sprintf(p, "%04X", disp); break;
		case 0xd: p += sprintf(p, "@AR%d+[AR%d++]", arx, ary); break;
		case 0xe: p += sprintf(p, "@AR%d+[AR%d--]", arx, ary); break;
		case 0xf: p += sprintf(p, "@AR%d+[AR%d++%04X]", arx, ary, disp); break;
	}

	return buffer;
}

static char* dasm_alu_mul(UINT64 opcode, bool twoop)
{
	static char buffer[80];
	char *p = buffer;

	int ma = (opcode & ((UINT64)(1) << 41)) ? 1 : 0;
	int o = (opcode >> 42) & 0x1f;
	int i2 = (opcode >> 47) & 0x1f;
	int i1 = (opcode >> 52) & 0xf;

	if (twoop || ma)
	{
		// ALU op
		int aluop = (opcode >> 56) & 0x1f;
		switch (aluop)
		{
			case 0x00: p += sprintf(p, "FADD %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x01: p += sprintf(p, "FADDZ %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x02: p += sprintf(p, "FSUB %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x03: p += sprintf(p, "FSUBZ %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x04: p += sprintf(p, "FCMP %s, %s", ai1_field[i1], ai2f_field[i2]);
				break;
			case 0x05: p += sprintf(p, "FABS %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x06: p += sprintf(p, "FABC %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x07: p += sprintf(p, "NOP");
				break;
			case 0x08: p += sprintf(p, "FEA %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x09: p += sprintf(p, "FES %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x0a: p += sprintf(p, "FRCP %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0b: p += sprintf(p, "FRSQ %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0c: p += sprintf(p, "FLOG %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0d: p += sprintf(p, "CIF %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0e: p += sprintf(p, "CFI %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0f: p += sprintf(p, "CFIB %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x10: p += sprintf(p, "ADD %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x11: p += sprintf(p, "ADDZ %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x12: p += sprintf(p, "SUB %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x13: p += sprintf(p, "SUBZ %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x14: p += sprintf(p, "CMP %s, %s", ai1_field[i1], ai2_field[i2]);
				break;
			case 0x15: p += sprintf(p, "ABS %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x16: p += sprintf(p, "ATR %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x17: p += sprintf(p, "ATRZ %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x18: p += sprintf(p, "AND %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x19: p += sprintf(p, "OR %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x1a: p += sprintf(p, "XOR %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x1b: p += sprintf(p, "NOT %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x1c: p += sprintf(p, "LSR %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x1d: p += sprintf(p, "LSL %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x1e: p += sprintf(p, "ASR %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x1f: p += sprintf(p, "ASL %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
		}
	}
	
	// multiplication
	if (twoop)
	{
		int mi2 = (opcode >> 32) & 0x1f;
		int mi1 = (opcode >> 37) & 0xf;
		int mo = (opcode >> 27) & 0x1f;
		if (opcode & ((UINT64)(1) << 41))
			p += sprintf(p, " : FMUL %s, %s, %s", mi1_field[mi1], mi2_field[mi2], mo_field[mo]);
		else
			p += sprintf(p, " : MUL %s, %s, %s", mi1_field[mi1], mi2_field[mi2], mo_field[mo]);
	}
	else
	{
		if (ma == 0)
		{			
			if (opcode & ((UINT64)(1) << 56))
				p += sprintf(p, "FMUL %s, %s, %s", mi1_field[i1], mi2_field[i2], mo_field[o]);
			else
				p += sprintf(p, "MUL %s, %s, %s", mi1_field[i1], mi2_field[i2], mo_field[o]);
		}
	}

	return buffer;
}

static char* dasm_control(UINT32 pc, UINT64 opcode)
{
	static char buffer[80];
	char *p = buffer;

	int ef1 = (opcode >> 16) & 0x3f;
	int ef2 = opcode & 0xffff;

	int cop = (opcode >> 22) & 0x1f;

	int rel12 = (opcode & 0x800) ? (0xfffff000 | (opcode & 0xfff)) : (opcode & 0xfff);


	switch (cop)
	{
		case 0x00: p += sprintf(p, "NOP");
			break;
		case 0x01:
			if (ef1 == 0)
				p += sprintf(p, "REP #%04X", ef2);
			else
				p += sprintf(p, "REP AR%d", (ef2 >> 12) & 7);
			break;
		case 0x02:
			if (ef1 == 0)
				p += sprintf(p, "SETL #%04X", ef2);
			else
				p += sprintf(p, "SETL AR%d", (ef2 >> 12) & 7);
			break;
		case 0x03:
			if (ef1 == 1)
				p += sprintf(p, "CLRFI");
			else if (ef1 == 2)
				p += sprintf(p, "CLRFO");
			else if (ef1 == 3)
				p += sprintf(p, "CLRF");
			break;
		case 0x04:
			p += sprintf("PUSH %s", regname[(ef2 >> 6) & 0x3f]);
			break;
		case 0x05:
			p += sprintf("POP %s", regname[(ef2 >> 6) & 0x3f]);
			break;
		case 0x08:
			p += sprintf(p, "SETM #%04X", ef2);
			break;
		case 0x09:
			p += sprintf(p, "SETM #%01X, CBSA", (ef2 >> 12) & 7);
			break;
		case 0x0a:
			p += sprintf(p, "SETM #%01X, CBSB", (ef2 >> 8) & 7);
			break;
		case 0x0b:
			p += sprintf(p, "SETM #%d, RF", (ef2 >> 7) & 1);
			break;
		case 0x0c:
			p += sprintf(p, "SETM #%d, RDY", (ef2 >> 4) & 1);
			break;
		case 0x0d:
			p += sprintf(p, "SETM #%01X, WAIT", ef2 & 7);
			break;
		case 0x13:
			p += sprintf(p, "DBLP %04X", pc + rel12);
			break;
		case 0x14:
			p += sprintf(p, "DBBC AR%d:%d, %04X", (UINT32)((opcode >> 13) & 7), (UINT32)((opcode >> 16) & 0xf), pc + rel12);
			break;
		case 0x15:
			p += sprintf(p, "DBBS AR%d:%d, %04X", (UINT32)((opcode >> 13) & 7), (UINT32)((opcode >> 16) & 0xf), pc + rel12);
			break;
		case 0x1b:
			p += sprintf(p, "DRET");
			break;

		case 0x10:		// DBcc
		case 0x11:		// DBNcc
		case 0x18:		// DCcc
		case 0x19:		// DCNcc
		case 0x1a:		// DCALL
		case 0x12:		// DJMP
		{
			if (cop == 0x10)
				p += sprintf(p, "%s ", db_mnemonic[ef1]);
			else if (cop == 0x11)
				p += sprintf(p, "%s ", dbn_mnemonic[ef1]);
			else if (cop == 0x18)
				p += sprintf(p, "%s ", dc_mnemonic[ef1]);
			else if (cop == 0x19)
				p += sprintf(p, "%s ", dcn_mnemonic[ef1]);
			else if (cop == 0x12)
				p += sprintf(p, "DJMP ");
			else if (cop == 0x1a)
				p += sprintf(p, "DCALL ");

			switch ((opcode >> 12) & 0xf)
			{
				case 0x0: p += sprintf(p, "%03X", ef2 & 0xfff); break;
				case 0x1: p += sprintf(p, "%04X", pc + rel12); break;
				case 0x2: p += sprintf(p, "%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x3: p += sprintf(p, "+%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x4: p += sprintf(p, "%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x5: p += sprintf(p, "+%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x6: p += sprintf(p, "%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x7: p += sprintf(p, "+%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x8: p += sprintf(p, "A(%03X)", ef2 & 0x3ff); break;
				case 0x9: p += sprintf(p, "+A(%03X)", ef2 & 0x3ff); break;
				case 0xa: p += sprintf(p, "B(%03X)", ef2 & 0x3ff); break;
				case 0xb: p += sprintf(p, "+B(%03X)", ef2 & 0x3ff); break;
				case 0xc: p += sprintf(p, "A(%s)", regname[(ef2 >> 6) & 0x3f]); break;
				case 0xd: p += sprintf(p, "+A(%s)", regname[(ef2 >> 6) & 0x3f]); break;
				case 0xe: p += sprintf(p, "B(%s)", regname[(ef2 >> 6) & 0x3f]); break;
				case 0xf: p += sprintf(p, "+B(%s)", regname[(ef2 >> 6) & 0x3f]); break;
			}

			break;
		}
	}

	return buffer;
}

static char* dasm_double_xfer1(UINT64 opcode)
{
	static char buffer[80];
	char *p = buffer;

	int sd = (opcode >> 25) & 3;

	switch (sd)
	{
		case 0:
		{
			int as = (opcode >> 20) & 0x1f;
			int ad = (opcode >> 15) & 0x1f;
			int bs = (opcode >> 10) & 0x1f;
			int bd = (opcode >> 5) & 0x1f;
			p += sprintf(p, "MVD1 %s, %s, %s, %s", regname[as], regname[ad], regname[bs], regname[bd]);
			break;
		}

		case 1:
		{
			int areg = (opcode >> 20) & 0x1f;
			int aarx = (opcode >> 17) & 7;
			int aary = (opcode >> 14) & 7;
			int breg = (opcode >> 10) & 0xf;
			int barx = (opcode >> 7) & 7;
			int bary = (opcode >> 4) & 7;
			int md = opcode & 0xf;

			p += sprintf(p, "MVD1 ");

			switch (md)
			{
				case 0x0: p += sprintf(p, "%s, A(@AR%d), %s, B(@AR%d)", regname[areg], aarx, regname[breg], barx); break;
				case 0x1: p += sprintf(p, "%s, A(@AR%d++), %s, B(@AR%d++)", regname[areg], aarx, regname[breg], barx); break;
				case 0x2: p += sprintf(p, "%s, A(@AR%d--), %s, B(@AR%d--)", regname[areg], aarx, regname[breg], barx); break;
				case 0x4: p += sprintf(p, "%s, A(@AR%d+AR%d), %s, B(@AR%d+AR%d)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x5: p += sprintf(p, "%s, A(@AR%d+AR%d++), %s, B(@AR%d+AR%d++)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x6: p += sprintf(p, "%s, A(@AR%d+AR%d--), %s, B(@AR%d+AR%d--)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x8: p += sprintf(p, "%s, A(@AR%d+AR%dU), %s, B(@AR%d+AR%dL)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x9: p += sprintf(p, "%s, A(@AR%d+AR%dL), %s, B(@AR%d+AR%dU)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0xd: p += sprintf(p, "%s, A(@AR%d+[AR%d++]), %s, B(@AR%d+[AR%d++])", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0xe: p += sprintf(p, "%s, A(@AR%d+[AR%d--]), %s, B(@AR%d+[AR%d--])", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				default: p += sprintf(p, "???"); break;
			}
			break;
		}

		case 2:
		{
			int areg = (opcode >> 20) & 0x1f;
			int aarx = (opcode >> 17) & 7;
			int aary = (opcode >> 14) & 7;
			int breg = (opcode >> 10) & 0xf;
			int barx = (opcode >> 7) & 7;
			int bary = (opcode >> 4) & 7;
			int md = opcode & 0xf;

			p += sprintf(p, "MVD1 ");

			switch (md)
			{
				case 0x0: p += sprintf(p, "A(@AR%d), %s, B(@AR%d), %s", aarx, regname[areg], barx, regname[breg]); break;
				case 0x1: p += sprintf(p, "A(@AR%d++), %s, B(@AR%d++), %s", aarx, regname[areg], barx, regname[breg]); break;
				case 0x2: p += sprintf(p, "A(@AR%d--), %s, B(@AR%d--), %s", aarx, regname[areg], barx, regname[breg]); break;
				case 0x4: p += sprintf(p, "A(@AR%d+AR%d), %s, B(@AR%d+AR%d), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x5: p += sprintf(p, "A(@AR%d+AR%d++), %s, B(@AR%d+AR%d++), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x6: p += sprintf(p, "A(@AR%d+AR%d--), %s, B(@AR%d+AR%d--), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x8: p += sprintf(p, "A(@AR%d+AR%dU), %s, B(@AR%d+AR%dL), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x9: p += sprintf(p, "A(@AR%d+AR%dL), %s, B(@AR%d+AR%dU), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0xd: p += sprintf(p, "A(@AR%d+[AR%d++]), %s, B(@AR%d+[AR%d++]), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0xe: p += sprintf(p, "A(@AR%d+[AR%d--]), %s, B(@AR%d+[AR%d--]), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				default: p += sprintf(p, "???"); break;
			}
			break;
		}

		case 3:
			p += sprintf(p, "???");
			break;
	}

	return buffer;
}

static char* dasm_xfer1(UINT64 opcode)
{
	static char buffer[80];
	char *p = buffer;

	int dr = (opcode >> 12) & 0x7f;
	int sr = (opcode >> 19) & 0x7f;
	int md = opcode & 0xf;
	int ary = (opcode >> 4) & 7;
	int disp5 = (opcode >> 7) & 0x1f;
	int trm = (opcode >> 26) & 1;
	int dir = (opcode >> 25) & 1;

	if (trm == 0)
	{
		if (sr == 0x58)
		{
			p += sprintf(p, "MOV1 #%03X, %s", (UINT32)(opcode & 0xfff), regname[dr]);
		}
		else
		{
			p += sprintf(p, "MOV1 ");

			if ((sr & 0x40) == 0)
			{
				p += sprintf(p, "%s", regname[sr]);
			}
			else
			{
				if (sr & 0x20)
					p += sprintf(p, "B");
				else
					p += sprintf(p, "A");

				p += sprintf(p, "(%s)", get_ea(md, sr & 7, ary, disp5));
			}

			p += sprintf(p, ", ");

			if ((dr & 0x40) == 0)
			{
				p += sprintf(p, "%s", regname[dr]);
			}
			else
			{
				if (dr & 0x20)
					p += sprintf(p, "B");
				else
					p += sprintf(p, "A");

				p += sprintf(p, "(%s)", get_ea(md, dr & 7, ary, disp5));
			}
		}
	}
	else
	{
		if (dir == 0)
		{
			p += sprintf(p, "MOV1 ");

			if ((dr & 0x40) == 0)
			{
				p += sprintf(p, "%s, ", regname[dr]);
			}
			else
			{
				if (dr & 0x20)
					p += sprintf(p, "B");
				else
					p += sprintf(p, "A");

				p += sprintf(p, "(%s)", get_ea(md, dr & 7, ary, disp5));
			}

			p += sprintf(p, "E(@EB+EO++%02X)", sr);
		}
		else
		{
			p += sprintf(p, "MOV1 E(@EB+EO++%02X), ", sr);

			if ((dr & 0x40) == 0)
			{
				p += sprintf(p, "%s", regname[dr]);
			}
			else
			{
				if (dr & 0x20)
					p += sprintf(p, "B");
				else
					p += sprintf(p, "A");

				p += sprintf(p, "(%s)", get_ea(md, dr & 7, ary, disp5));
			}
		}
	}

	return buffer;
}

static char* double_xfer2_field(int sd, UINT32 field)
{
	static char buffer[40];
	char *p = buffer;

	switch (sd)
	{
		case 0:
		{
			int s = (field >> 13) & 0x1f;
			int d = (field >> 8) & 0x1f;
			p += sprintf(p, "%s, %s", regname[s], regname[d]);
			break;
		}

		case 1:
		{
			int reg = (field >> 13) & 0x1f;
			int arx = (field >> 10) & 7;
			int ary = (field >> 7) & 7;
			int disp3 = (field >> 4) & 7;
			int md = field & 0xf;

			switch (md)
			{
				case 0x0: p += sprintf(p, "%s, A(@AR%d)", regname[reg], arx); break;
				case 0x1: p += sprintf(p, "%s, A(@AR%d++)", regname[reg], arx); break;
				case 0x2: p += sprintf(p, "%s, A(@AR%d--)", regname[reg], arx); break;
				case 0x4: p += sprintf(p, "%s, A(@AR%d+AR%d))", regname[reg], arx, ary); break;
				case 0x5: p += sprintf(p, "%s, A(@AR%d+AR%d++)", regname[reg], arx, ary); break;
				case 0x6: p += sprintf(p, "%s, A(@AR%d+AR%d--)", regname[reg], arx, ary); break;
				case 0x8: p += sprintf(p, "%s, A(@AR%d+AR%dU)", regname[reg], arx, ary); break;
				case 0x9: p += sprintf(p, "%s, A(@AR%d+AR%dL)", regname[reg], arx, ary); break;
				case 0xd: p += sprintf(p, "%s, A(@AR%d+[AR%d++])", regname[reg], arx, ary); break;
				case 0xe: p += sprintf(p, "%s, A(@AR%d+[AR%d--]", regname[reg], arx, ary); break;
				default: p += sprintf(p, "???"); break;
			}
			break;
		}

		case 2:
		{
			int reg = (field >> 13) & 0x1f;
			int arx = (field >> 10) & 7;
			int ary = (field >> 7) & 7;
			int disp3 = (field >> 4) & 7;
			int md = field & 0xf;

			switch (md)
			{
				case 0x0: p += sprintf(p, "A(@AR%d), %s", arx, regname[reg]); break;
				case 0x1: p += sprintf(p, "A(@AR%d++), %s", arx, regname[reg]); break;
				case 0x2: p += sprintf(p, "A(@AR%d--), %s", arx, regname[reg]); break;
				case 0x4: p += sprintf(p, "A(@AR%d+AR%d), %s", arx, ary, regname[reg]); break;
				case 0x5: p += sprintf(p, "A(@AR%d+AR%d++), %s", arx, ary, regname[reg]); break;
				case 0x6: p += sprintf(p, "A(@AR%d+AR%d--), %s", arx, ary, regname[reg]); break;
				case 0x8: p += sprintf(p, "A(@AR%d+AR%dU), %s", arx, ary, regname[reg]); break;
				case 0x9: p += sprintf(p, "A(@AR%d+AR%dL), %s", arx, ary, regname[reg]); break;
				case 0xd: p += sprintf(p, "A(@AR%d+[AR%d++]), %s", arx, ary, regname[reg]); break;
				case 0xe: p += sprintf(p, "A(@AR%d+[AR%d--]), %s", arx, ary, regname[reg]); break;
				default: p += sprintf(p, "???"); break;
			}
			break;
		}

		case 3:
			p += sprintf(p, "???");
			break;
	}

	return buffer;
}

static char* dasm_double_xfer2(UINT64 opcode)
{
	static char buffer[80];
	char *p = buffer;

	int asd = (opcode >> 38) & 3;
	int bsd = (opcode >> 18) & 3;

	if (asd == 3)
	{
		int barx = (opcode >> 10) & 7;
		int bary = (opcode >> 7) & 7;
		int disp3 = (opcode >> 4) & 7;
		int bmd = opcode & 0xf;
		int arx = (opcode >> 30) & 7;

		p += sprintf(p, "MOVI ");

		switch (bmd)
		{
			case 0x0: p += sprintf(p, "B(@BAR%d)", barx); break;
			case 0x1: p += sprintf(p, "B(@BAR%d++)", barx); break;
			case 0x2: p += sprintf(p, "B(@BAR%d--)", barx); break;
			case 0x3: p += sprintf(p, "B(@BAR%d++%02X)", barx, disp3); break;
			case 0x4: p += sprintf(p, "B(@BAR%d+BAR%d)", barx, bary); break;
			case 0x5: p += sprintf(p, "B(@BAR%d+BAR%d++)", barx, bary); break;
			case 0x6: p += sprintf(p, "B(@BAR%d+BAR%d--)", barx, bary); break;
			case 0x7: p += sprintf(p, "B(@BAR%d+BAR%d++%02X)", barx, bary, disp3); break;
			case 0x8: p += sprintf(p, "B(@BAR%d+BAR%dU)", barx, bary); break;
			case 0x9: p += sprintf(p, "B(@BAR%d+BAR%dL)", barx, bary); break;
			case 0xa: p += sprintf(p, "B(@BAR%d+%02X)", barx, disp3); break;
			case 0xb: p += sprintf(p, "B(@BAR%d+BAR%d+%02X)", barx, bary, disp3); break;
			case 0xc: p += sprintf(p, "???"); break;
			case 0xd: p += sprintf(p, "B(@BAR%d+[BAR%d++])", barx, bary); break;
			case 0xe: p += sprintf(p, "B(@BAR%d+[BAR%d--])", barx, bary); break;
			case 0xf: p += sprintf(p, "B(@BAR%d+[BAR%d++%02X])", barx, bary, disp3); break;
		}

		p += sprintf(p, ", I(@AR%d++)", arx);
	}
	else
	{
		p += sprintf(p, "MVD2 %s, %s", double_xfer2_field(asd, (opcode >> 20) & 0x3ffff), double_xfer2_field(bsd, opcode & 0x3ffff));
	}

	return buffer;
}

static char* dasm_xfer2(UINT64 opcode)
{
	static char buffer[80];
	char *p = buffer;

	int op = (opcode >> 39) & 3;
	int trm = (opcode >> 38) & 1;
	int dir = (opcode >> 37) & 1;
	int sr = (opcode >> 31) & 0x7f;
	int dr = (opcode >> 24) & 0x7f;
	int ary = (opcode >> 4) & 7;
	int md = opcode & 0xf;
	int disp14 = (opcode >> 7) & 0x3fff;

	if (op == 0)
	{
		if (trm == 0)
		{
			if (sr == 0x58)
			{
				p += sprintf(p, "MOV2 #%06X, %s", (UINT32)(opcode & 0xffffff), regname[dr]);
			}
			else
			{
				p += sprintf(p, "MOV2 ");

				if ((sr & 0x40) == 0)
				{
					p += sprintf(p, "%s", regname[sr]);
				}
				else
				{
					if (sr & 0x20)
						p += sprintf(p, "B");
					else
						p += sprintf(p, "A");

					p += sprintf(p, "(%s)", get_ea(md, sr & 7, ary, disp14));
				}

				p += sprintf(p, ", ");

				if ((dr & 0x40) == 0)
				{
					p += sprintf(p, "%s", regname[dr]);
				}
				else
				{
					if (dr & 0x20)
						p += sprintf(p, "B");
					else
						p += sprintf(p, "A");

					p += sprintf(p, "(%s)", get_ea(md, dr & 7, ary, disp14));
				}
			}
		}
		else
		{
			if (dir == 0)
			{
				p += sprintf(p, "MOV2 %s, E(@EB+EO++%02X)", regname[dr], sr);
			}
			else
			{
				p += sprintf(p, "MOV2 E(@EB+EO++%02X), %s", sr, regname[dr]);
			}
		}
	}
	else if (op == 2)
	{
		if (trm == 0)
		{
			if ((sr & 0x40) == 0)
			{
				p += sprintf(p, "MOV4 %s, ICDTR%d", regname[sr], dr & 7);
			}
			else
			{
				if (sr == 0x58)
				{
					p += sprintf(p, "MOV4 #%06X, ICDTR%d", (UINT32)(opcode & 0xffffff), dr & 7);
				}
				else
				{
					p += sprintf(p, "MOV4 ");

					if (sr & 0x20)
						p += sprintf(p, "B");
					else
						p += sprintf(p, "A");

					p += sprintf(p, "(%s), ICDTR%d", get_ea(md, sr & 7, ary, disp14), dr & 7);
				}
			}
		}
		else
		{
			if (dir == 0)
			{
				p += sprintf(p, "MOV4 ICDTR%d, E(@EB+EO++%02X)", dr & 7, sr);
			}
			else
			{
				p += sprintf(p, "MOV4 E(@EB+EO++%02X), ICDTR%d", sr, dr & 7);
			}
		}
	}

	return buffer;
}

static char* dasm_xfer3(UINT64 opcode)
{
	static char buffer[80];
	char *p = buffer;

	UINT32 imm = (UINT32)(opcode >> 27);
	int dr = (opcode >> 19) & 0x7f;
	int disp = (opcode >> 7) & 0xfff;
	int ary = (opcode >> 4) & 7;
	int md = opcode & 0xf;

	p += sprintf(p, "MOV3 #%08X, ", imm);

	if ((dr & 0x40) == 0)
	{
		p += sprintf(p, "%s", regname[dr]);
	}
	else
	{
		if (dr & 0x20)
			p += sprintf(p, "B");
		else
			p += sprintf(p, "A");

		p += sprintf(p, "(%s)", get_ea(md, dr & 7, ary, disp));
	}

	return buffer;
}

static unsigned dasm_mb86235(char *buffer, UINT32 pc, UINT64 opcode)
{
	char *p = buffer;

	p[0] = 0;

	switch ((opcode >> 61) & 7)
	{
		case 0:		// ALU / MUL / double transfer (type 1)
			p += sprintf(p, "%s : %s", dasm_alu_mul(opcode, true), dasm_double_xfer1(opcode));
			break;
		case 1:		// ALU / MYL / transfer (type 1)
			p += sprintf(p, "%s : %s", dasm_alu_mul(opcode, true), dasm_xfer1(opcode));
			break;
		case 2:		// ALU / MUL / control
			p += sprintf(p, "%s : %s", dasm_alu_mul(opcode, true), dasm_control(pc, opcode));
			break;
		case 4:		// ALU or MUL / double transfer (type 2)
			p += sprintf(p, "%s : %s", dasm_alu_mul(opcode, false), dasm_double_xfer2(opcode));
			break;
		case 5:		// ALU or MUL / transfer (type 2)
			p += sprintf(p, "%s : %s", dasm_alu_mul(opcode, false), dasm_xfer2(opcode));
			break;
		case 6:		// ALU or MUL / control
			p += sprintf(p, "%s : %s", dasm_alu_mul(opcode, false), dasm_control(pc, opcode));
			break;
		case 7:		// transfer (type 3)
			p += sprintf(p, "%s", dasm_xfer3(opcode));
			break;

		default:
			p += sprintf(p, "???");
			break;
	}

	return (1 | DASMFLAG_SUPPORTED);
}



CPU_DISASSEMBLE( mb86235 )
{
	UINT64 op = *(UINT64*)oprom;
	op = LITTLE_ENDIANIZE_INT64(op);

	return dasm_mb86235(buffer, pc, op);
}
