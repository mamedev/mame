// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi, Ville Linde
#include "emu.h"
#include "mb86235d.h"

char const *const mb86235_disassembler::regname[128] =
{
	"MA0",  "MA1",  "MA2",  "MA3",  "MA4",  "MA5",  "MA6",  "MA7",
	"AA0",  "AA1",  "AA2",  "AA3",  "AA4",  "AA5",  "AA6",  "AA7",
	"EB",   "EBU",  "EBL",  "EO",   "SP",   "ST",   "MOD",  "LRPC",
	"AR0",  "AR1",  "AR2",  "AR3",  "AR4",  "AR5",  "AR6",  "AR7",
	"MB0",  "MB1",  "MB2",  "MB3",  "MB4",  "MB5",  "MB6",  "MB7",
	"AB0",  "AB1",  "AB2",  "AB3",  "AB4",  "AB5",  "AB6",  "AB7",
	"PR",   "FI",   "FO0",  "FO1",  "PDR",  "DDR",  "PRP",  "PWP",
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???"
};

char const *const mb86235_disassembler::db_mnemonic[64] =
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

char const *const mb86235_disassembler::dbn_mnemonic[64] =
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

char const *const mb86235_disassembler::dc_mnemonic[64] =
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

char const *const mb86235_disassembler::dcn_mnemonic[64] =
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

char const *const mb86235_disassembler::mi1_field[16] =
{ "MA0", "MA1", "MA2", "MA3", "MA4", "MA5", "MA6", "MA7", "MB0", "MB1", "MB2", "MB3", "MB4", "MB5", "MB6", "MB7" };

char const *const mb86235_disassembler::mi2_field[32] =
{ "MA0", "MA1", "MA2", "MA3", "MA4", "MA5", "MA6", "MA7", "MB0", "MB1", "MB2", "MB3", "MB4", "MB5", "MB6", "MB7",
	"PR", "PR++", "PR--", "PR#0", "???", "???", "???", "???", "-1.0E+0", "0.0E+0", "0.5E+0", "1.0E+0", "1.5E+0", "2.0E+0", "3.0E+0", "5.0E+0" };

char const *const mb86235_disassembler::mo_field[32] =
{ "MA0", "MA1", "MA2", "MA3", "MA4", "MA5", "MA6", "MA7", "MB0", "MB1", "MB2", "MB3", "MB4", "MB5", "MB6", "MB7",
	"AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7" };

char const *const mb86235_disassembler::ai1_field[16] =
{ "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7" };

char const *const mb86235_disassembler::ai2_field[32] =
{ "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7",
	"PR", "PR++", "PR--", "PR#0", "???", "???", "???", "???", "0", "1", "-1", "???", "???", "???", "???", "???" };

char const *const mb86235_disassembler::ai2f_field[32] =
{ "AA0", "AA1", "AA2", "AA3", "AA4", "AA5", "AA6", "AA7", "AB0", "AB1", "AB2", "AB3", "AB4", "AB5", "AB6", "AB7",
	"PR", "PR++", "PR--", "PR#0", "???", "???", "???", "???", "-1.0E+0", "0.0E+0", "0.5E+0", "1.0E+0", "1.5E+0", "2.0E+0", "3.0E+0", "5.0E+0" };

void mb86235_disassembler::dasm_ea(std::ostream &stream, int md, int arx, int ary, int disp)
{
	if (arx & 0x20)
		stream << "B(";
	else
		stream << "A(";

	arx &= 7;
	switch (md)
	{
		case 0x0: util::stream_format(stream, "@AR%d", arx); break;
		case 0x1: util::stream_format(stream, "@AR%d++", arx); break;
		case 0x2: util::stream_format(stream, "@AR%d--", arx); break;
		case 0x3: util::stream_format(stream, "@AR%d++%04X", arx, disp); break;
		case 0x4: util::stream_format(stream, "@AR%d+AR%d", arx, ary); break;
		case 0x5: util::stream_format(stream, "@AR%d+AR%d++", arx, ary); break;
		case 0x6: util::stream_format(stream, "@AR%d+AR%d--", arx, ary); break;
		case 0x7: util::stream_format(stream, "@AR%d+AR%d++%04X", arx, ary, disp); break;
		case 0x8: util::stream_format(stream, "@AR%d+AR%dU", arx, ary); break;
		case 0x9: util::stream_format(stream, "@AR%d+AR%dL", arx, ary); break;
		case 0xa: util::stream_format(stream, "@AR%d+%04X", arx, disp); break;
		case 0xb: util::stream_format(stream, "@AR%d+AR%d+%04X", arx, ary, disp); break;
		case 0xc: util::stream_format(stream, "%04X", disp); break;
		case 0xd: util::stream_format(stream, "@AR%d+[AR%d++]", arx, ary); break;
		case 0xe: util::stream_format(stream, "@AR%d+[AR%d--]", arx, ary); break;
		case 0xf: util::stream_format(stream, "@AR%d+[AR%d++%04X]", arx, ary, disp); break;
	}

	stream << ')';
}

void mb86235_disassembler::dasm_alu_mul(std::ostream &stream, uint64_t opcode, bool twoop)
{
	int ma = (opcode & ((uint64_t)(1) << 41)) ? 1 : 0;
	int o = (opcode >> 42) & 0x1f;
	int i2 = (opcode >> 47) & 0x1f;
	int i1 = (opcode >> 52) & 0xf;

	if (twoop || ma)
	{
		// ALU op
		int aluop = (opcode >> 56) & 0x1f;
		switch (aluop)
		{
			case 0x00: util::stream_format(stream, "FADD %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x01: util::stream_format(stream, "FADDZ %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x02: util::stream_format(stream, "FSUB %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x03: util::stream_format(stream, "FSUBZ %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x04: util::stream_format(stream, "FCMP %s, %s", ai1_field[i1], ai2f_field[i2]);
				break;
			case 0x05: util::stream_format(stream, "FABS %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x06: util::stream_format(stream, "FABC %s, %s, %s", ai1_field[i1], ai2f_field[i2], mo_field[o]);
				break;
			case 0x07: stream << "NOP";
				break;
			case 0x08: util::stream_format(stream, "FEA %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x09: util::stream_format(stream, "FES %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x0a: util::stream_format(stream, "FRCP %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0b: util::stream_format(stream, "FRSQ %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0c: util::stream_format(stream, "FLOG %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0d: util::stream_format(stream, "CIF %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0e: util::stream_format(stream, "CFI %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x0f: util::stream_format(stream, "CFIB %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x10: util::stream_format(stream, "ADD %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x11: util::stream_format(stream, "ADDZ %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x12: util::stream_format(stream, "SUB %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x13: util::stream_format(stream, "SUBZ %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x14: util::stream_format(stream, "CMP %s, %s", ai1_field[i1], ai2_field[i2]);
				break;
			case 0x15: util::stream_format(stream, "ABS %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x16: util::stream_format(stream, "ATR %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x17: util::stream_format(stream, "ATRZ %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x18: util::stream_format(stream, "AND %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x19: util::stream_format(stream, "OR %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x1a: util::stream_format(stream, "XOR %s, %s, %s", ai1_field[i1], ai2_field[i2], mo_field[o]);
				break;
			case 0x1b: util::stream_format(stream, "NOT %s, %s", ai1_field[i1], mo_field[o]);
				break;
			case 0x1c: util::stream_format(stream, "LSR %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x1d: util::stream_format(stream, "LSL %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x1e: util::stream_format(stream, "ASR %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
			case 0x1f: util::stream_format(stream, "ASL %s, #%02X, %s", ai1_field[i1], i2, mo_field[o]);
				break;
		}
	}

	// multiplication
	if (twoop)
	{
		int mi2 = (opcode >> 32) & 0x1f;
		int mi1 = (opcode >> 37) & 0xf;
		int mo = (opcode >> 27) & 0x1f;
		if (opcode & ((uint64_t)(1) << 41))
			util::stream_format(stream, " : FMUL %s, %s, %s", mi1_field[mi1], mi2_field[mi2], mo_field[mo]);
		else
			util::stream_format(stream, " : MUL %s, %s, %s", mi1_field[mi1], mi2_field[mi2], mo_field[mo]);
	}
	else
	{
		if (ma == 0)
		{
			if (opcode & ((uint64_t)(1) << 56))
				util::stream_format(stream, "FMUL %s, %s, %s", mi1_field[i1], mi2_field[i2], mo_field[o]);
			else
				util::stream_format(stream, "MUL %s, %s, %s", mi1_field[i1], mi2_field[i2], mo_field[o]);
		}
	}
}

offs_t mb86235_disassembler::dasm_control(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int ef1 = (opcode >> 16) & 0x3f;
	int ef2 = opcode & 0xffff;

	int cop = (opcode >> 22) & 0x1f;

	int rel12 = (opcode & 0x800) ? (0xfffff000 | (opcode & 0xfff)) : (opcode & 0xfff);

	offs_t flags = 0;

	switch (cop)
	{
		case 0x00: stream << "NOP";
			break;
		case 0x01:
			if (ef1 == 0)
				util::stream_format(stream, "REP #%04X", ef2);
			else
				util::stream_format(stream, "REP AR%d", (ef2 >> 12) & 7);
			break;
		case 0x02:
			if (ef1 == 0)
				util::stream_format(stream, "SETL #%04X", ef2);
			else
				util::stream_format(stream, "SETL AR%d", (ef2 >> 12) & 7);
			break;
		case 0x03:
			if (ef1 == 1)
				stream << "CLRFI";
			else if (ef1 == 2)
				stream << "CLRFO";
			else if (ef1 == 3)
				stream << "CLRF";
			break;
		case 0x04:
			util::stream_format(stream, "PUSH %s", regname[(ef2 >> 6) & 0x3f]);
			break;
		case 0x05:
			util::stream_format(stream, "POP %s", regname[(ef2 >> 6) & 0x3f]);
			break;
		case 0x08:
			util::stream_format(stream, "SETM #%04X", ef2);
			break;
		case 0x09:
			util::stream_format(stream, "SETM #%01X, CBSA", (ef2 >> 12) & 7);
			break;
		case 0x0a:
			util::stream_format(stream, "SETM #%01X, CBSB", (ef2 >> 8) & 7);
			break;
		case 0x0b:
			util::stream_format(stream, "SETM #%d, RF", (ef2 >> 7) & 1);
			break;
		case 0x0c:
			util::stream_format(stream, "SETM #%d, RDY", (ef2 >> 4) & 1);
			break;
		case 0x0d:
			util::stream_format(stream, "SETM #%01X, WAIT", ef2 & 7);
			break;
		case 0x13:
			util::stream_format(stream, "DBLP %04X", pc + rel12);
			break;
		case 0x14:
			util::stream_format(stream, "DBBC AR%d:%d, %04X", (uint32_t)((opcode >> 13) & 7), (uint32_t)((opcode >> 16) & 0xf), pc + rel12);
			flags = STEP_COND | step_over_extra(1);
			break;
		case 0x15:
			util::stream_format(stream, "DBBS AR%d:%d, %04X", (uint32_t)((opcode >> 13) & 7), (uint32_t)((opcode >> 16) & 0xf), pc + rel12);
			flags = STEP_COND | step_over_extra(1);
			break;
		case 0x1b:
			stream << "DRET";
			flags = STEP_OUT | step_over_extra(1);
			break;

		case 0x10:      // DBcc
		case 0x11:      // DBNcc
		case 0x18:      // DCcc
		case 0x19:      // DCNcc
		case 0x1a:      // DCALL
		case 0x12:      // DJMP
		{
			if (cop == 0x10)
			{
				util::stream_format(stream, "%s ", db_mnemonic[ef1]);
				flags = STEP_COND | step_over_extra(1);
			}
			else if (cop == 0x11)
			{
				util::stream_format(stream, "%s ", dbn_mnemonic[ef1]);
				flags = STEP_COND | step_over_extra(1);
			}
			else if (cop == 0x18)
			{
				util::stream_format(stream, "%s ", dc_mnemonic[ef1]);
				flags = STEP_OVER | STEP_COND | step_over_extra(1);
			}
			else if (cop == 0x19)
			{
				util::stream_format(stream, "%s ", dcn_mnemonic[ef1]);
				flags = STEP_OVER | STEP_COND | step_over_extra(1);
			}
			else if (cop == 0x12)
				stream << "DJMP ";
			else if (cop == 0x1a)
			{
				stream << "DCALL ";
				flags = STEP_OVER | step_over_extra(1);
			}

			switch ((opcode >> 12) & 0xf)
			{
				case 0x0: util::stream_format(stream, "%03X", ef2 & 0xfff); break;
				case 0x1: util::stream_format(stream, "%04X", pc + rel12); break;
				case 0x2: util::stream_format(stream, "%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x3: util::stream_format(stream, "+%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x4: util::stream_format(stream, "%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x5: util::stream_format(stream, "+%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x6: util::stream_format(stream, "%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x7: util::stream_format(stream, "+%s", regname[(ef2 >> 6) & 0x3f]); break;
				case 0x8: util::stream_format(stream, "A(%03X)", ef2 & 0x3ff); break;
				case 0x9: util::stream_format(stream, "+A(%03X)", ef2 & 0x3ff); break;
				case 0xa: util::stream_format(stream, "B(%03X)", ef2 & 0x3ff); break;
				case 0xb: util::stream_format(stream, "+B(%03X)", ef2 & 0x3ff); break;
				case 0xc: util::stream_format(stream, "A(%s)", regname[(ef2 >> 6) & 0x3f]); break;
				case 0xd: util::stream_format(stream, "+A(%s)", regname[(ef2 >> 6) & 0x3f]); break;
				case 0xe: util::stream_format(stream, "B(%s)", regname[(ef2 >> 6) & 0x3f]); break;
				case 0xf: util::stream_format(stream, "+B(%s)", regname[(ef2 >> 6) & 0x3f]); break;
			}

			break;
		}
	}

	return flags;
}

void mb86235_disassembler::dasm_double_xfer1(std::ostream &stream, uint64_t opcode)
{
	int sd = (opcode >> 25) & 3;

	stream << "MVD1 ";

	switch (sd)
	{
		case 0:
		{
			int as = (opcode >> 20) & 0x1f;
			int ad = (opcode >> 15) & 0x1f;
			int bs = (opcode >> 10) & 0x1f;
			int bd = (opcode >> 5) & 0x1f;
			util::stream_format(stream, "%s, %s, %s, %s", regname[as], regname[ad], regname[bs], regname[bd]);
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

			switch (md)
			{
				case 0x0: util::stream_format(stream, "%s, A(@AR%d), %s, B(@AR%d)", regname[areg], aarx, regname[breg], barx); break;
				case 0x1: util::stream_format(stream, "%s, A(@AR%d++), %s, B(@AR%d++)", regname[areg], aarx, regname[breg], barx); break;
				case 0x2: util::stream_format(stream, "%s, A(@AR%d--), %s, B(@AR%d--)", regname[areg], aarx, regname[breg], barx); break;
				case 0x4: util::stream_format(stream, "%s, A(@AR%d+AR%d), %s, B(@AR%d+AR%d)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x5: util::stream_format(stream, "%s, A(@AR%d+AR%d++), %s, B(@AR%d+AR%d++)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x6: util::stream_format(stream, "%s, A(@AR%d+AR%d--), %s, B(@AR%d+AR%d--)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x8: util::stream_format(stream, "%s, A(@AR%d+AR%dU), %s, B(@AR%d+AR%dL)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0x9: util::stream_format(stream, "%s, A(@AR%d+AR%dL), %s, B(@AR%d+AR%dU)", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0xd: util::stream_format(stream, "%s, A(@AR%d+[AR%d++]), %s, B(@AR%d+[AR%d++])", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				case 0xe: util::stream_format(stream, "%s, A(@AR%d+[AR%d--]), %s, B(@AR%d+[AR%d--])", regname[areg], aarx, aary, regname[breg], barx, bary); break;
				default: stream << "???"; break;
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

			switch (md)
			{
				case 0x0: util::stream_format(stream, "A(@AR%d), %s, B(@AR%d), %s", aarx, regname[areg], barx, regname[breg]); break;
				case 0x1: util::stream_format(stream, "A(@AR%d++), %s, B(@AR%d++), %s", aarx, regname[areg], barx, regname[breg]); break;
				case 0x2: util::stream_format(stream, "A(@AR%d--), %s, B(@AR%d--), %s", aarx, regname[areg], barx, regname[breg]); break;
				case 0x4: util::stream_format(stream, "A(@AR%d+AR%d), %s, B(@AR%d+AR%d), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x5: util::stream_format(stream, "A(@AR%d+AR%d++), %s, B(@AR%d+AR%d++), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x6: util::stream_format(stream, "A(@AR%d+AR%d--), %s, B(@AR%d+AR%d--), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x8: util::stream_format(stream, "A(@AR%d+AR%dU), %s, B(@AR%d+AR%dL), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0x9: util::stream_format(stream, "A(@AR%d+AR%dL), %s, B(@AR%d+AR%dU), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0xd: util::stream_format(stream, "A(@AR%d+[AR%d++]), %s, B(@AR%d+[AR%d++]), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				case 0xe: util::stream_format(stream, "A(@AR%d+[AR%d--]), %s, B(@AR%d+[AR%d--]), %s", aarx, aary, regname[areg], barx, bary, regname[breg]); break;
				default: stream << "???"; break;
			}
			break;
		}

		case 3:
			stream << "???";
			break;
	}
}

void mb86235_disassembler::dasm_xfer1(std::ostream &stream, uint64_t opcode)
{
	int dr = (opcode >> 12) & 0x7f;
	int sr = (opcode >> 19) & 0x7f;
	int md = opcode & 0xf;
	int ary = (opcode >> 4) & 7;
	int disp5 = (opcode >> 7) & 0x1f;
	int trm = (opcode >> 26) & 1;
	int dir = (opcode >> 25) & 1;

	stream << "MOV1 ";

	if (trm == 0)
	{
		if (sr == 0x58)
		{
			util::stream_format(stream, "#%03X, %s", (uint32_t)(opcode & 0xfff), regname[dr]);
		}
		else
		{
			if ((sr & 0x40) == 0)
				stream << regname[sr];
			else
				dasm_ea(stream, md, sr, ary, disp5);

			stream << ", ";

			if ((dr & 0x40) == 0)
				stream << regname[dr];
			else
				dasm_ea(stream, md, dr, ary, disp5);
		}
	}
	else
	{
		if (dir == 0)
		{
			if ((dr & 0x40) == 0)
				util::stream_format(stream, "%s, ", regname[dr]);
			else
				dasm_ea(stream, md, dr, ary, disp5);

			util::stream_format(stream, "E(@EB+EO++%02X)", sr);
		}
		else
		{
			util::stream_format(stream, "E(@EB+EO++%02X), ", sr);

			if ((dr & 0x40) == 0)
				stream << regname[dr];
			else
				dasm_ea(stream, md, dr, ary, disp5);
		}
	}
}

void mb86235_disassembler::dasm_double_xfer2_field(std::ostream &stream, int sd, bool isbbus, uint32_t field)
{
	switch (sd)
	{
		case 0:
		{
			int s = (field >> 13) & 0x1f;
			int d = (field >> 8) & 0x1f;
			if(isbbus == true)
			{
				s |= 0x20;
				d |= 0x20;
			}
			util::stream_format(stream, "%s, %s", regname[s], regname[d]);
			break;
		}

		case 1:
		{
			int reg = (field >> 13) & 0x1f;
			int arx = (field >> 10) & 7;
			int ary = (field >> 7) & 7;
			//int disp3 = (field >> 4) & 7;
			int md = field & 0xf;

			switch (md)
			{
				case 0x0: util::stream_format(stream, "%s, A(@AR%d)", regname[reg], arx); break;
				case 0x1: util::stream_format(stream, "%s, A(@AR%d++)", regname[reg], arx); break;
				case 0x2: util::stream_format(stream, "%s, A(@AR%d--)", regname[reg], arx); break;
				case 0x4: util::stream_format(stream, "%s, A(@AR%d+AR%d))", regname[reg], arx, ary); break;
				case 0x5: util::stream_format(stream, "%s, A(@AR%d+AR%d++)", regname[reg], arx, ary); break;
				case 0x6: util::stream_format(stream, "%s, A(@AR%d+AR%d--)", regname[reg], arx, ary); break;
				case 0x8: util::stream_format(stream, "%s, A(@AR%d+AR%dU)", regname[reg], arx, ary); break;
				case 0x9: util::stream_format(stream, "%s, A(@AR%d+AR%dL)", regname[reg], arx, ary); break;
				case 0xd: util::stream_format(stream, "%s, A(@AR%d+[AR%d++])", regname[reg], arx, ary); break;
				case 0xe: util::stream_format(stream, "%s, A(@AR%d+[AR%d--]", regname[reg], arx, ary); break;
				default: stream << "???"; break;
			}
			break;
		}

		case 2:
		{
			int reg = (field >> 13) & 0x1f;
			int arx = (field >> 10) & 7;
			int ary = (field >> 7) & 7;
			//int disp3 = (field >> 4) & 7;
			int md = field & 0xf;

			switch (md)
			{
				case 0x0: util::stream_format(stream, "A(@AR%d), %s", arx, regname[reg]); break;
				case 0x1: util::stream_format(stream, "A(@AR%d++), %s", arx, regname[reg]); break;
				case 0x2: util::stream_format(stream, "A(@AR%d--), %s", arx, regname[reg]); break;
				case 0x4: util::stream_format(stream, "A(@AR%d+AR%d), %s", arx, ary, regname[reg]); break;
				case 0x5: util::stream_format(stream, "A(@AR%d+AR%d++), %s", arx, ary, regname[reg]); break;
				case 0x6: util::stream_format(stream, "A(@AR%d+AR%d--), %s", arx, ary, regname[reg]); break;
				case 0x8: util::stream_format(stream, "A(@AR%d+AR%dU), %s", arx, ary, regname[reg]); break;
				case 0x9: util::stream_format(stream, "A(@AR%d+AR%dL), %s", arx, ary, regname[reg]); break;
				case 0xd: util::stream_format(stream, "A(@AR%d+[AR%d++]), %s", arx, ary, regname[reg]); break;
				case 0xe: util::stream_format(stream, "A(@AR%d+[AR%d--]), %s", arx, ary, regname[reg]); break;
				default: stream << "???"; break;
			}
			break;
		}

		case 3:
			stream << "???";
			break;
	}
}

void mb86235_disassembler::dasm_double_xfer2(std::ostream &stream, uint64_t opcode)
{
	int asd = (opcode >> 38) & 3;
	int bsd = (opcode >> 18) & 3;

	if (asd == 3)
	{
		int barx = (opcode >> 10) & 7;
		int bary = (opcode >> 7) & 7;
		int disp3 = (opcode >> 4) & 7;
		int bmd = opcode & 0xf;
		int arx = (opcode >> 30) & 7;

		stream << "MOVI ";

		switch (bmd)
		{
			case 0x0: util::stream_format(stream, "B(@BAR%d)", barx); break;
			case 0x1: util::stream_format(stream, "B(@BAR%d++)", barx); break;
			case 0x2: util::stream_format(stream, "B(@BAR%d--)", barx); break;
			case 0x3: util::stream_format(stream, "B(@BAR%d++%02X)", barx, disp3); break;
			case 0x4: util::stream_format(stream, "B(@BAR%d+BAR%d)", barx, bary); break;
			case 0x5: util::stream_format(stream, "B(@BAR%d+BAR%d++)", barx, bary); break;
			case 0x6: util::stream_format(stream, "B(@BAR%d+BAR%d--)", barx, bary); break;
			case 0x7: util::stream_format(stream, "B(@BAR%d+BAR%d++%02X)", barx, bary, disp3); break;
			case 0x8: util::stream_format(stream, "B(@BAR%d+BAR%dU)", barx, bary); break;
			case 0x9: util::stream_format(stream, "B(@BAR%d+BAR%dL)", barx, bary); break;
			case 0xa: util::stream_format(stream, "B(@BAR%d+%02X)", barx, disp3); break;
			case 0xb: util::stream_format(stream, "B(@BAR%d+BAR%d+%02X)", barx, bary, disp3); break;
			case 0xc: stream << "???"; break;
			case 0xd: util::stream_format(stream, "B(@BAR%d+[BAR%d++])", barx, bary); break;
			case 0xe: util::stream_format(stream, "B(@BAR%d+[BAR%d--])", barx, bary); break;
			case 0xf: util::stream_format(stream, "B(@BAR%d+[BAR%d++%02X])", barx, bary, disp3); break;
		}

		util::stream_format(stream, ", I(@AR%d++)", arx);
	}
	else
	{
		stream << "MVD2 ";
		dasm_double_xfer2_field(stream, asd, false, (opcode >> 20) & 0x3ffff);
		stream << ", ";
		dasm_double_xfer2_field(stream, bsd, true, opcode & 0x3ffff);
	}
}

void mb86235_disassembler::dasm_xfer2(std::ostream &stream, uint64_t opcode)
{
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
		stream << "MOV2 ";

		if (trm == 0)
		{
			if (sr == 0x58)
			{
				util::stream_format(stream, "#%06X, %s", (uint32_t)(opcode & 0xffffff), regname[dr]);
			}
			else
			{
				if ((sr & 0x40) == 0)
					stream << regname[sr];
				else
					dasm_ea(stream, md, sr, ary, disp14);

				stream << ", ";

				if ((dr & 0x40) == 0)
					stream << regname[dr];
				else
					dasm_ea(stream, md, dr, ary, disp14);
			}
		}
		else
		{
			if (dir == 0)
				util::stream_format(stream, "%s, E(@EB+EO++%02X)", regname[dr], sr);
			else
				util::stream_format(stream, "E(@EB+EO++%02X), %s", sr, regname[dr]);
		}
	}
	else if (op == 2)
	{
		stream << "MOV4 ";

		if (trm == 0)
		{
			if ((sr & 0x40) == 0)
				stream << regname[sr];
			else if (sr == 0x58)
				util::stream_format(stream, "#%06X", (uint32_t)(opcode & 0xffffff));
			else
				dasm_ea(stream, md, sr, ary, disp14);

			util::stream_format(stream, ", ICDTR%d", dr & 7);
		}
		else
		{
			if (dir == 0)
				util::stream_format(stream, "ICDTR%d, E(@EB+EO++%02X)", dr & 7, sr);
			else
				util::stream_format(stream, "E(@EB+EO++%02X), ICDTR%d", sr, dr & 7);
		}
	}
}

void mb86235_disassembler::dasm_xfer3(std::ostream &stream, uint64_t opcode)
{
	uint32_t imm = (uint32_t)(opcode >> 27);
	int dr = (opcode >> 19) & 0x7f;
	int disp = (opcode >> 7) & 0xfff;
	int ary = (opcode >> 4) & 7;
	int md = opcode & 0xf;

	util::stream_format(stream, "MOV3 #%08X, ", imm);

	if ((dr & 0x40) == 0)
		stream << regname[dr];
	else
		dasm_ea(stream, md, dr, ary, disp);
}

offs_t mb86235_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u64 opcode = opcodes.r64(pc);
	switch ((opcode >> 61) & 7)
	{
		case 0:     // ALU / MUL / double transfer (type 1)
			dasm_alu_mul(stream, opcode, true);
			stream << " : ";
			dasm_double_xfer1(stream, opcode);
			break;
		case 1:     // ALU / MYL / transfer (type 1)
			dasm_alu_mul(stream, opcode, true);
			stream << " : ";
			dasm_xfer1(stream, opcode);
			break;
		case 2:     // ALU / MUL / control
			dasm_alu_mul(stream, opcode, true);
			stream << " : ";
			return 1 | dasm_control(stream, pc, opcode) | SUPPORTED;
		case 4:     // ALU or MUL / double transfer (type 2)
			dasm_alu_mul(stream, opcode, false);
			stream << " : ";
			dasm_double_xfer2(stream, opcode);
			break;
		case 5:     // ALU or MUL / transfer (type 2)
			dasm_alu_mul(stream, opcode, false);
			stream << " : ";
			dasm_xfer2(stream, opcode);
			break;
		case 6:     // ALU or MUL / control
			dasm_alu_mul(stream, opcode, false);
			stream << " : ";
			return 1 | dasm_control(stream, pc, opcode) | SUPPORTED;
		case 7:     // transfer (type 3)
			dasm_xfer3(stream, opcode);
			break;

		default:
			stream << "???";
			break;
	}

	return 1 | SUPPORTED;
}

u32 mb86235_disassembler::opcode_alignment() const
{
	return 1;
}

