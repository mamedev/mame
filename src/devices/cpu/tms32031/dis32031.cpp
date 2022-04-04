// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dis32031.cpp
    Disassembler for the portable TMS320C3x emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "dis32031.h"

/***************************************************************************
    CODE CODE
***************************************************************************/

const char *const tms32031_disassembler::regname[32] =
{
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"AR0", "AR1", "AR2", "AR3", "AR4", "AR5", "AR6", "AR7",
	"DP", "IR0", "IR1", "BK", "SP", "ST", "IE", "IF",
	"IOF", "RS", "RE", "RC", "??", "??", "??", "??"
};

const char *const tms32031_disassembler::condition[32] =
{
	"U", "LO", "LS", "HI", "HS", "EQ", "NE", "LT",
	"LE", "GT", "GE", "??", "NV", "V", "NUF", "UF",
	"NLV", "LV", "NLUF", "LUF", "ZUF", "??", "??", "??",
	"??", "??", "??", "??", "??", "??", "??", "??"
};

//
// for instructions 0x000-0x3f
//  G = (op >> 21) & 3;
//  G == 0 -> register  -> 000ooooo o00ddddd 00000000 000sssss
//  G == 1 -> direct    -> 000ooooo o01ddddd DDDDDDDD DDDDDDDD
//  G == 2 -> indirect  -> 000ooooo o10ddddd mmmmmaaa DDDDDDDD
//  G == 3 -> immediate -> 000ooooo o11ddddd iiiiiiii iiiiiiii
//
// for instructions 0x040-0x7f
//  T = (op >> 21) & 3;                       (src1)   (src2)
//  T == 0 -> reg reg   -> 001ooooo o00ddddd 000sssss 000SSSSS
//  T == 1 -> ind reg   -> 001ooooo o01ddddd mmmmmaaa 000SSSSS
//  T == 2 -> reg ind   -> 001ooooo o10ddddd 000sssss MMMMMAAA
//  T == 3 -> ind ind   -> 001ooooo o11ddddd mmmmmaaa MMMMMAAA
//
// for instructions 0x100-0x1ff
//  10ooooPP dDsssSSS mmmmmaaa MMMMMAAA
//  10ooooPP dDsssSSS 111rrrrr 111RRRRR
//
// conditional branches
//  xxxxxxBa aaDccccc 00000000 000sssss
//  xxxxxxBa aaDccccc iiiiiiii iiiiiiii
//      D=0(standard) or 1(delayed)
//      B=0(register) or 1(immediate)
//
// status register
//  31-14 = 0
//  13 = GIE
//  12 = CC
//  11 = CE
//  10 = CF
//  9 = 0
//  8 = RM
//  7 = OVM
//  6 = LUF (latched FP underflow)
//  5 = LV (latched overflow)
//  4 = UF (FP underflow)
//  3 = N
//  2 = Z
//  1 = V
//  0 = C
//
// conditions:
//  0 = U (unconditional)
//  1 = LO (C)
//  2 = LS (C | Z)
//  3 = HI (~C & ~Z)
//  4 = HS (~C)
//  5 = EQ (Z)
//  6 = NE (~Z)
//  7 = LT (N)
//  8 = LE (N | Z)
//  9 = GT (~N & ~Z)
//  10 = GE (~N)
//  12 = NV (~V)
//  13 = V (V)
//  14 = NUF (~UF)
//  15 = UF (UF)
//  16 = NLV (~LV)
//  17 = LV (LV)
//  18 = NLUF (~LUF)
//  19 = LUF (LUF)
//  20 = ZUF (Z | UF)

void tms32031_disassembler::append_indirect(uint8_t ma, int8_t disp, std::ostream &stream)
{
	std::string dispstr;
	int mode = (ma >> 3) & 0x1f;
	int ar = ma & 7;

	if (disp < 0)
		dispstr = util::string_format("(-%X)", -disp & 0xffff);
	else if (disp > 0)
		dispstr = util::string_format("(%X)", disp);

	switch (mode)
	{
		case 0:     util::stream_format(stream, "*+AR%d%s", ar, dispstr);  break;
		case 1:     util::stream_format(stream, "*-AR%d%s", ar, dispstr);  break;
		case 2:     util::stream_format(stream, "*++AR%d%s", ar, dispstr); break;
		case 3:     util::stream_format(stream, "*--AR%d%s", ar, dispstr); break;
		case 4:     util::stream_format(stream, "*AR%d++%s", ar, dispstr); break;
		case 5:     util::stream_format(stream, "*AR%d--%s", ar, dispstr); break;
		case 6:     util::stream_format(stream, "*AR%d++%s%%", ar, dispstr);   break;
		case 7:     util::stream_format(stream, "*AR%d--%s%%", ar, dispstr);   break;

		case 8:     util::stream_format(stream, "*+AR%d(IR0)", ar);    break;
		case 9:     util::stream_format(stream, "*-AR%d(IR0)", ar);    break;
		case 10:    util::stream_format(stream, "*++AR%d(IR0)", ar);   break;
		case 11:    util::stream_format(stream, "*--AR%d(IR0)", ar);   break;
		case 12:    util::stream_format(stream, "*AR%d++(IR0)", ar);   break;
		case 13:    util::stream_format(stream, "*AR%d--(IR0)", ar);   break;
		case 14:    util::stream_format(stream, "*AR%d++(IR0)%%", ar); break;
		case 15:    util::stream_format(stream, "*AR%d--(IR0)%%", ar); break;

		case 16:    util::stream_format(stream, "*+AR%d(IR1)", ar);    break;
		case 17:    util::stream_format(stream, "*-AR%d(IR1)", ar);    break;
		case 18:    util::stream_format(stream, "*++AR%d(IR1)", ar);   break;
		case 19:    util::stream_format(stream, "*--AR%d(IR1)", ar);   break;
		case 20:    util::stream_format(stream, "*AR%d++(IR1)", ar);   break;
		case 21:    util::stream_format(stream, "*AR%d--(IR1)", ar);   break;
		case 22:    util::stream_format(stream, "*AR%d++(IR1)%%", ar); break;
		case 23:    util::stream_format(stream, "*AR%d--(IR1)%%", ar); break;

		case 24:    util::stream_format(stream, "*AR%d", ar);          break;
		case 25:    util::stream_format(stream, "*AR%d++(IR0)B", ar);  break;
		case 28:
		case 29:
		case 30:
		case 31:    stream << regname[ma & 31];      break;
		default:    util::stream_format(stream, "(unknown mode)");     break;
	}
}

std::string tms32031_disassembler::get_indirect(uint8_t ma, int8_t disp)
{
	std::ostringstream stream;
	append_indirect(ma, disp, stream);
	return stream.str();
}

void tms32031_disassembler::append_immediate(uint16_t data, int is_float, int is_unsigned, std::ostream &stream)
{
	if (is_float)
	{
		int exp = ((int16_t)data >> 12) + 127;
		uint32_t expanded_data;
		float float_val;

		expanded_data = ((data & 0x0800) << 20) + ((exp << 23) & 0x7f800000);
		if (data == 0x8000)
			*(float *)&expanded_data = 0;
		else if (!(data & 0x0800))
			expanded_data += ((data & 0x0fff) << 12);
		else
			expanded_data += ((-data & 0x0fff) << 12);
		float_val = *(float *)&expanded_data;
		util::stream_format(stream, "%8f", (double) float_val);
	}
	else if (!is_unsigned && (int16_t)data < 0)
		util::stream_format(stream, "-$%04X", -data & 0xffff);
	else
		util::stream_format(stream, "$%04X", data);
}

void tms32031_disassembler::disasm_general(const char *opstring, uint32_t op, int flags, std::ostream &stream)
{
	util::stream_format(stream, "%-6s", opstring);

	if (flags & SWAPSRCDST)
	{
		stream << regname[(op >> 16) & 31] << ',';
	}

	/* switch off of G */
	if (!(flags & NOSOURCE))
	{
		switch ((op >> 21) & 3)
		{
			case 0:
				stream << regname[op & 31];
				break;

			case 1:
				util::stream_format(stream, "($%04X)", op & 0xffff);
				break;

			case 2:
				append_indirect((op >> 8) & 0xff, op, stream);
				break;

			case 3:
				append_immediate(op & 0xffff, (flags & FLOAT), (flags & UNSIGNED), stream);
				break;
		}
	}

	/* add destination op */
	if (!(flags & NODEST) && !(flags & SWAPSRCDST))
	{
		if (!(flags & NOSOURCE))
			stream << ',';
		stream << regname[(op >> 16) & 31];
	}
}

void tms32031_disassembler::disasm_3op(const char *opstring, uint32_t op, int flags, std::ostream &stream)
{
	util::stream_format(stream, "%-6s", opstring);

	/* switch off of T */
	if (!(flags & NOSOURCE1))
	{
		switch ((op >> 21) & 1)
		{
			case 0:
				stream << regname[(op >> 8) & 31];
				break;

			case 1:
				append_indirect(op >> 8, 1, stream);
				break;
		}
	}

	/* switch off of T */
	if (!(flags & NOSOURCE2))
	{
		if (!(flags & NOSOURCE1))
			stream << ',';
		switch ((op >> 22) & 1)
		{
			case 0:
				stream << regname[op & 31];
				break;

			case 1:
				append_indirect(op, 1, stream);
				break;
		}
	}

	/* add destination op */
	if (!(flags & NODEST))
	{
		if (!(flags & (NOSOURCE1 | NOSOURCE2)))
			stream << ',';
		stream << regname[(op >> 16) & 31];
	}
}

void tms32031_disassembler::disasm_conditional(const char *opstring, uint32_t op, int flags, std::ostream &stream)
{
	char temp[10];
	sprintf(temp, "%s%s", opstring, condition[(op >> 23) & 31]);
	disasm_general(temp, op, flags, stream);
}


void tms32031_disassembler::disasm_parallel_3op3op(const char *opstring1, const char *opstring2, uint32_t op, int flags, const uint8_t *srctable, std::ostream &stream)
{
	const uint8_t *s = &srctable[((op >> 24) & 3) * 4];
	int d1 = (op >> 23) & 1;
	int d2 = 2 + ((op >> 22) & 1);
	std::string src[5];

	src[1].assign(regname[(op >> 19) & 7]);
	src[2].assign(regname[(op >> 16) & 7]);
	src[3] = get_indirect(op >> 8, 1);
	src[4] = get_indirect(op, 1);

	util::stream_format(stream, "%s %s,%s,R%d || %s %s,%s,R%d",
			opstring1, src[s[0]], src[s[1]], d1,
			opstring2, src[s[2]], src[s[3]], d2);
}


void tms32031_disassembler::disasm_parallel_3opstore(const char *opstring1, const char *opstring2, uint32_t op, int flags, std::ostream &stream)
{
	int d1 = (op >> 22) & 7;
	int s1 = (op >> 19) & 7;
	int s3 = (op >> 16) & 7;

	std::string dst2 = get_indirect(op >> 8, 1);
	std::string src2 = get_indirect(op, 1);

	if (!(flags & NOSOURCE1))
		util::stream_format(stream, "%s R%d,%s,R%d || %s R%d,%s",
				opstring1, s1, src2, d1,
				opstring2, s3, dst2);
	else
		util::stream_format(stream, "%s %s,R%d || %s R%d,%s",
				opstring1, src2, d1,
				opstring2, s3, dst2);
}


void tms32031_disassembler::disasm_parallel_loadload(const char *opstring1, const char *opstring2, uint32_t op, int flags, std::ostream &stream)
{
	int d2 = (op >> 22) & 7;
	int d1 = (op >> 19) & 7;

	std::string src1 = get_indirect(op >> 8, 1);
	std::string src2 = get_indirect(op >> 0, 1);

	util::stream_format(stream, "%s %s,R%d || %s %s,R%d",
			opstring1, src2, d2,
			opstring2, src1, d1);
}


void tms32031_disassembler::disasm_parallel_storestore(const char *opstring1, const char *opstring2, uint32_t op, int flags, std::ostream &stream)
{
	int s2 = (op >> 22) & 7;
	int s1 = (op >> 16) & 7;

	std::string dst1 = get_indirect(op >> 8, 1);
	std::string dst2 = get_indirect(op, 1);

	util::stream_format(stream, "%s R%d,%s || %s R%d,%s",
			opstring1, s2, dst2,
			opstring2, s1, dst1);
}



offs_t tms32031_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	uint32_t op = opcodes.r32(pc);

	switch (op >> 23)
	{
		case 0x000: disasm_general("ABSF", op, FLOAT, stream);          break;
		case 0x001: disasm_general("ABSI", op, INTEGER, stream);        break;
		case 0x002: disasm_general("ADDC", op, INTEGER, stream);        break;
		case 0x003: disasm_general("ADDF", op, FLOAT, stream);          break;
		case 0x004: disasm_general("ADDI", op, INTEGER, stream);        break;
		case 0x005: disasm_general("AND", op, INTEGER | UNSIGNED, stream);  break;
		case 0x006: disasm_general("ANDN", op, INTEGER | UNSIGNED, stream); break;
		case 0x007: disasm_general("ASH", op, INTEGER, stream);         break;

		case 0x008: disasm_general("CMPF", op, FLOAT, stream);          break;
		case 0x009: disasm_general("CMPI", op, INTEGER, stream);        break;
		case 0x00a: disasm_general("FIX", op, FLOAT, stream);           break;
		case 0x00b: disasm_general("FLOAT", op, INTEGER, stream);       break;
		case 0x00c: disasm_general((op & 1) ? "IDLE2" : "IDLE", op, NOSOURCE | NODEST, stream); break;
		case 0x00d: disasm_general("LDE", op, FLOAT, stream);           break;
		case 0x00e: disasm_general("LDF", op, FLOAT, stream);           break;
		case 0x00f: disasm_general("LDFI", op, FLOAT, stream);          break;

		case 0x010: disasm_general("LDI", op, INTEGER, stream);         break;
		case 0x011: disasm_general("LDII", op, INTEGER, stream);        break;
		case 0x012: disasm_general("LDM", op, FLOAT, stream);           break;
		case 0x013: disasm_general("LSH", op, INTEGER, stream);         break;
		case 0x014: disasm_general("MPYF", op, FLOAT, stream);          break;
		case 0x015: disasm_general("MPYI", op, INTEGER, stream);        break;
		case 0x016: disasm_general("NEGB", op, INTEGER, stream);        break;
		case 0x017: disasm_general("NEGF", op, FLOAT, stream);          break;

		case 0x018: disasm_general("NEGI", op, INTEGER, stream);        break;
		case 0x019: disasm_general("NOP", op, NODEST, stream);          break;
		case 0x01a: disasm_general("NORM", op, FLOAT, stream);          break;
		case 0x01b: disasm_general("NOT", op, INTEGER, stream);         break;
		case 0x01c: disasm_general("POP", op, NOSOURCE, stream);        break;
		case 0x01d: disasm_general("POPF", op, NOSOURCE, stream);       break;
		case 0x01e: disasm_general("PUSH", op, NOSOURCE, stream);       break;
		case 0x01f: disasm_general("PUSHF", op, NOSOURCE, stream);      break;

		case 0x020: disasm_general("OR", op, INTEGER | UNSIGNED, stream);   break;
		case 0x021: disasm_general((op & 1) ? "LOPOWER" : "MAXSPEED", op, NOSOURCE | NODEST, stream); break;
		case 0x022: disasm_general("RND", op, FLOAT, stream);           break;
		case 0x023: disasm_general("ROL", op, INTEGER, stream);         break;
		case 0x024: disasm_general("ROLC", op, INTEGER, stream);        break;
		case 0x025: disasm_general("ROR", op, INTEGER, stream);         break;
		case 0x026: disasm_general("RORC", op, INTEGER, stream);        break;
		case 0x027: disasm_general("RPTS", op, INTEGER | NODEST, stream); break;

		case 0x028: disasm_general("STF", op, FLOAT | SWAPSRCDST, stream);  break;
		case 0x029: disasm_general("STFI", op, FLOAT | SWAPSRCDST, stream); break;
		case 0x02a: disasm_general("STI", op, INTEGER | SWAPSRCDST, stream); break;
		case 0x02b: disasm_general("STII", op, INTEGER | SWAPSRCDST, stream); break;
		case 0x02c: disasm_general("SIGI", op, NOSOURCE | NODEST, stream); break;
		case 0x02d: disasm_general("SUBB", op, INTEGER, stream);        break;
		case 0x02e: disasm_general("SUBC", op, INTEGER, stream);        break;
		case 0x02f: disasm_general("SUBF", op, FLOAT, stream);          break;

		case 0x030: disasm_general("SUBI", op, INTEGER, stream);        break;
		case 0x031: disasm_general("SUBRB", op, INTEGER, stream);       break;
		case 0x032: disasm_general("SUBRF", op, FLOAT, stream);         break;
		case 0x033: disasm_general("SUBRI", op, INTEGER, stream);       break;
		case 0x034: disasm_general("TSTB", op, INTEGER, stream);        break;
		case 0x035: disasm_general("XOR", op, INTEGER | UNSIGNED, stream);  break;
		case 0x036: disasm_general("IACK", op, INTEGER | NODEST, stream); break;

		case 0x040: disasm_3op("ADDC3", op, INTEGER, stream);           break;
		case 0x041: disasm_3op("ADDF3", op, FLOAT, stream);             break;
		case 0x042: disasm_3op("ADDI3", op, INTEGER, stream);           break;
		case 0x043: disasm_3op("AND3", op, INTEGER, stream);            break;
		case 0x044: disasm_3op("ANDN3", op, INTEGER, stream);           break;
		case 0x045: disasm_3op("ASH3", op, INTEGER, stream);            break;
		case 0x046: disasm_3op("CMPF3", op, FLOAT | NODEST, stream);    break;
		case 0x047: disasm_3op("CMPI3", op, INTEGER | NODEST, stream);  break;

		case 0x048: disasm_3op("LSH3", op, INTEGER, stream);            break;
		case 0x049: disasm_3op("MPYF3", op, FLOAT, stream);             break;
		case 0x04a: disasm_3op("MPYI3", op, INTEGER, stream);           break;
		case 0x04b: disasm_3op("OR3", op, INTEGER, stream);             break;
		case 0x04c: disasm_3op("SUBB3", op, INTEGER, stream);           break;
		case 0x04d: disasm_3op("SUBF3", op, FLOAT, stream);             break;
		case 0x04e: disasm_3op("SUBI3", op, INTEGER, stream);           break;
		case 0x04f: disasm_3op("TSTB3", op, INTEGER, stream);           break;

		case 0x050: disasm_3op("XOR3", op, INTEGER, stream);            break;

		case 0x080: case 0x081: case 0x082: case 0x083:
		case 0x084: case 0x085: case 0x086: case 0x087:
		case 0x088: case 0x089: case 0x08a: case 0x08b:
		case 0x08c: case 0x08d: case 0x08e: case 0x08f:
		case 0x090: case 0x091: case 0x092: case 0x093:
		case 0x094: case 0x095: case 0x096: case 0x097:
		case 0x098: case 0x099: case 0x09a: case 0x09b:
		case 0x09c: case 0x09d: case 0x09e: case 0x09f:
			disasm_conditional("LDF", op, FLOAT, stream);
			break;

		case 0x0a0: case 0x0a1: case 0x0a2: case 0x0a3:
		case 0x0a4: case 0x0a5: case 0x0a6: case 0x0a7:
		case 0x0a8: case 0x0a9: case 0x0aa: case 0x0ab:
		case 0x0ac: case 0x0ad: case 0x0ae: case 0x0af:
		case 0x0b0: case 0x0b1: case 0x0b2: case 0x0b3:
		case 0x0b4: case 0x0b5: case 0x0b6: case 0x0b7:
		case 0x0b8: case 0x0b9: case 0x0ba: case 0x0bb:
		case 0x0bc: case 0x0bd: case 0x0be: case 0x0bf:
			disasm_conditional("LDI", op, INTEGER, stream);
			break;


		case 0x0c0: case 0x0c1:
			util::stream_format(stream, "BR    $%06X", op & 0xffffff);
			break;

		case 0x0c2: case 0x0c3:
			util::stream_format(stream, "BRD   $%06X", op & 0xffffff);
			break;

		case 0x0c4: case 0x0c5:
			util::stream_format(stream, "CALL  $%06X", op & 0xffffff);
			flags = STEP_OVER;
			break;


		case 0x0c8: case 0x0c9:
			util::stream_format(stream, "RPTB  $%06X", op & 0xffffff);
			break;

		case 0x0cc: case 0x0cd: case 0x0ce: case 0x0cf:
			util::stream_format(stream, "SWI");
			break;


		case 0x0d0:
		{
			char temp[10];
			sprintf(temp, "B%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			util::stream_format(stream, "%-6s%s", temp, regname[op & 31]);
			if (((op >> 16) & 31) != 0)
				flags = STEP_COND | ((op >> 21) & 1 ? step_over_extra(1) : 0);
			break;
		}

		case 0x0d4:
		{
			char temp[10];
			sprintf(temp, "B%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			util::stream_format(stream, "%-6s$%06X", temp, (pc + (((op >> 21) & 1) ? 3 : 1) + (int16_t)op) & 0xffffff);
			if (((op >> 16) & 31) != 0)
				flags = STEP_COND | ((op >> 21) & 1 ? step_over_extra(1) : 0);
			break;
		}


		case 0x0d8: case 0x0d9: case 0x0da: case 0x0db:
		{
			char temp[10];
			sprintf(temp, "DB%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			util::stream_format(stream, "%-6sAR%d,%s", temp, (op >> 22) & 7, regname[op & 31]);
			flags = STEP_COND | ((op >> 21) & 1 ? step_over_extra(1) : 0);
			break;
		}

		case 0x0dc: case 0x0dd: case 0x0de: case 0x0df:
		{
			char temp[10];
			sprintf(temp, "DB%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			util::stream_format(stream, "%-6sAR%d,$%06X", temp, (op >> 22) & 7, (pc + (((op >> 21) & 1) ? 3 : 1) + (int16_t)op) & 0xffffff);
			flags = STEP_COND | ((op >> 21) & 1 ? step_over_extra(1) : 0);
			break;
		}


		case 0x0e0:
		{
			char temp[10];
			sprintf(temp, "CALL%s", condition[(op >> 16) & 31]);
			util::stream_format(stream, "%-6s%s", temp, regname[op & 31]);
			flags = STEP_OVER;
			break;
		}

		case 0x0e4:
		{
			char temp[10];
			sprintf(temp, "CALL%s", condition[(op >> 16) & 31]);
			util::stream_format(stream, "%-6s$%06X", temp, (pc + 1 + (int16_t)op) & 0xffffff);
			flags = STEP_OVER;
			break;
		}


		case 0x0e8: case 0x0e9: case 0x0ea: case 0x0eb:
		{
			char temp[10];
			sprintf(temp, "TRAP%s", condition[(op >> 16) & 31]);
			util::stream_format(stream, "%-6s$%02X", temp, op & 31);
			flags = STEP_OVER;
			break;
		}


		case 0x0f0:
			util::stream_format(stream, "RETI%s", condition[(op >> 16) & 31]);
			flags = STEP_OUT | (((op >> 16) & 31) != 0 ? STEP_COND : 0);
			break;

		case 0x0f1:
			util::stream_format(stream, "RETS%s", condition[(op >> 16) & 31]);
			flags = STEP_OUT | (((op >> 16) & 31) != 0 ? STEP_COND : 0);
			break;


		case 0x100: case 0x101: case 0x102: case 0x103:
		case 0x104: case 0x105: case 0x106: case 0x107: // MPYF3||ADDF3
		{
			static const uint8_t srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYF3", "ADDF3", op, FLOAT, srctable, stream);
			break;
		}


		case 0x108: case 0x109: case 0x10a: case 0x10b:
		case 0x10c: case 0x10d: case 0x10e: case 0x10f: // MPYF3||SUBF3
		{
			static const uint8_t srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYF3", "SUBF3", op, FLOAT, srctable, stream);
			break;
		}


		case 0x110: case 0x111: case 0x112: case 0x113:
		case 0x114: case 0x115: case 0x116: case 0x117: // MPYI3||ADDI3
		{
			static const uint8_t srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYI3", "ADDI3", op, INTEGER, srctable, stream);
			break;
		}


		case 0x118: case 0x119: case 0x11a: case 0x11b:
		case 0x11c: case 0x11d: case 0x11e: case 0x11f: // MPYI3||SUBI3
		{
			static const uint8_t srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYI3", "SUBI3", op, INTEGER, srctable, stream);
			break;
		}


		case 0x180: case 0x181: case 0x182: case 0x183: // STF||STF
			disasm_parallel_storestore("STF", "STF", op, FLOAT, stream);
			break;

		case 0x184: case 0x185: case 0x186: case 0x187: // STI||STI
			disasm_parallel_storestore("STI", "STI", op, INTEGER, stream);
			break;


		case 0x188: case 0x189: case 0x18a: case 0x18b: // LDF||LDF
			disasm_parallel_loadload("LDF", "LDF", op, FLOAT, stream);
			break;

		case 0x18c: case 0x18d: case 0x18e: case 0x18f: // LDI||LDI
			disasm_parallel_loadload("LDI", "LDI", op, INTEGER, stream);
			break;


		case 0x190: case 0x191: case 0x192: case 0x193: // ABSF||STF
			disasm_parallel_3opstore("ABSF", "STF", op, FLOAT | NOSOURCE1, stream);
			break;

		case 0x194: case 0x195: case 0x196: case 0x197: // ABSI||STI
			disasm_parallel_3opstore("ABSI", "STI", op, INTEGER | NOSOURCE1, stream);
			break;


		case 0x198: case 0x199: case 0x19a: case 0x19b: // ADDF3||STF
			disasm_parallel_3opstore("ADDF3", "STF", op, FLOAT, stream);
			break;

		case 0x19c: case 0x19d: case 0x19e: case 0x19f: // ADDI3||STI
			disasm_parallel_3opstore("ADDI3", "STI", op, INTEGER, stream);
			break;


		case 0x1a0: case 0x1a1: case 0x1a2: case 0x1a3: // AND3||STI
			disasm_parallel_3opstore("AND3", "STI", op, INTEGER, stream);
			break;

		case 0x1a4: case 0x1a5: case 0x1a6: case 0x1a7: // ASH3||STI
			disasm_parallel_3opstore("ASH3", "STI", op, INTEGER, stream);
			break;


		case 0x1a8: case 0x1a9: case 0x1aa: case 0x1ab: // FIX||STI
			disasm_parallel_3opstore("FIX", "STF", op, FLOAT | NOSOURCE1, stream);
			break;

		case 0x1ac: case 0x1ad: case 0x1ae: case 0x1af: // FLOAT||STF
			disasm_parallel_3opstore("FLOAT", "STF", op, FLOAT | NOSOURCE1, stream);
			break;


		case 0x1b0: case 0x1b1: case 0x1b2: case 0x1b3: // LDF||STF
			disasm_parallel_3opstore("LDF", "STF", op, FLOAT | NOSOURCE1, stream);
			break;

		case 0x1b4: case 0x1b5: case 0x1b6: case 0x1b7: // LDI||STI
			disasm_parallel_3opstore("LDI", "STI", op, INTEGER | NOSOURCE1, stream);
			break;


		case 0x1b8: case 0x1b9: case 0x1ba: case 0x1bb: // LSH3||STI
			disasm_parallel_3opstore("LSH3", "STI", op, INTEGER, stream);
			break;

		case 0x1bc: case 0x1bd: case 0x1be: case 0x1bf: // MPYF3||STF
			disasm_parallel_3opstore("MPYF3", "STF", op, FLOAT, stream);
			break;


		case 0x1c0: case 0x1c1: case 0x1c2: case 0x1c3: // MPYI3||STI
			disasm_parallel_3opstore("MPYI3", "STI", op, INTEGER, stream);
			break;

		case 0x1c4: case 0x1c5: case 0x1c6: case 0x1c7: // NEGF||STF
			disasm_parallel_3opstore("NEGF", "STF", op, FLOAT | NOSOURCE1, stream);
			break;


		case 0x1c8: case 0x1c9: case 0x1ca: case 0x1cb: // NEGI||STI
			disasm_parallel_3opstore("NEGI", "STI", op, INTEGER | NOSOURCE1, stream);
			break;

		case 0x1cc: case 0x1cd: case 0x1ce: case 0x1cf: // NOT||STI
			disasm_parallel_3opstore("NOT", "STI", op, INTEGER | NOSOURCE1, stream);
			break;


		case 0x1d0: case 0x1d1: case 0x1d2: case 0x1d3: // OR3||STI
			disasm_parallel_3opstore("OR3", "STI", op, INTEGER, stream);
			break;

		case 0x1d4: case 0x1d5: case 0x1d6: case 0x1d7: // SUBF3||STF
			disasm_parallel_3opstore("SUBF3", "STF", op, FLOAT, stream);
			break;


		case 0x1d8: case 0x1d9: case 0x1da: case 0x1db: // SUBI3||STI
			disasm_parallel_3opstore("SUBI3", "STI", op, INTEGER, stream);
			break;

		case 0x1dc: case 0x1dd: case 0x1de: case 0x1df: // XOR3||STI
			disasm_parallel_3opstore("XOR3", "STI", op, INTEGER, stream);
			break;


		default:
			break;
	}

	return 1 | flags | SUPPORTED;
}

u32 tms32031_disassembler::opcode_alignment() const
{
	return 1;
}

