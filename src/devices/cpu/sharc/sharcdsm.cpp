// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
   Analog Devices ADSP-2106x SHARC Disassembler

   Written by Ville Linde for use in MAME
*/

#include "emu.h"
#include "sharcdsm.h"

#include <stdexcept>


const char sharc_disassembler::ureg_names[256][16] =
{
	"R0",       "R1",       "R2",       "R3",       "R4",       "R5",       "R6",       "R7",
	"R8",       "R9",       "R10",      "R11",      "R12",      "R13",      "R14",      "R15",
	"I0",       "I1",       "I2",       "I3",       "I4",       "I5",       "I6",       "I7",
	"I8",       "I9",       "I10",      "I11",      "I12",      "I13",      "I14",      "I15",
	"M0",       "M1",       "M2",       "M3",       "M4",       "M5",       "M6",       "M7",
	"M8",       "M9",       "M10",      "M11",      "M12",      "M13",      "M14",      "M15",
	"L0",       "L1",       "L2",       "L3",       "L4",       "L5",       "L6",       "L7",
	"L8",       "L9",       "L10",      "L11",      "L12",      "L13",      "L14",      "L15",
	"B0",       "B1",       "B2",       "B3",       "B4",       "B5",       "B6",       "B7",
	"B8",       "B9",       "B10",      "B11",      "B12",      "B13",      "B14",      "B15",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"FADDR",    "DADDR",    "???",      "PC",       "PCSTK",    "PCSTKP",   "LADDR",    "CURLCNTR",
	"LCNTR",    "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"USTAT1",   "USTAT2",   "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "IRPTL",    "MODE2",    "MODE1",    "ASTAT",    "IMASK",    "STKY",     "IMASKP",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "PX",       "PX1",      "PX2",      "TPERIOD",  "TCOUNT",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???"
};

const char sharc_disassembler::bopnames[8][8] =
{
	"SET",      "CLEAR",    "TOGGLE",   "???",      "TEST",     "XOR",      "???",      "???"
};

const char sharc_disassembler::condition_codes_if[32][32] =
{
	"EQ",           "LT",           "LE",           "AC",
	"AV",           "MV",           "MS",           "SV",
	"SZ",           "FLAG0_IN",     "FLAG1_IN",     "FLAG2_IN",
	"FLAG3_IN",     "TF",           "BM",           "NOT LCE",
	"NE",           "GE",           "GT",           "NOT AC",
	"NOT AV",       "NOT MV",       "NOT MS",       "NOT SV",
	"NOT SZ",       "NOT FLAG0_IN", "NOT FLAG1_IN", "NOT FLAG2_IN",
	"NOT FLAG3_IN", "NOT TF",       "NBM",          ""
};

const char sharc_disassembler::condition_codes_do[32][32] =
{
	"EQ",           "LT",           "LE",           "AC",
	"AV",           "MV",           "MS",           "SV",
	"SZ",           "FLAG0_IN",     "FLAG1_IN",     "FLAG2_IN",
	"FLAG3_IN",     "TF",           "BM",           "LCE",
	"NE",           "GE",           "GT",           "NOT AC",
	"NOT AV",       "NOT MV",       "NOT MS",       "NOT SV",
	"NOT SZ",       "NOT FLAG0_IN", "NOT FLAG1_IN", "NOT FLAG2_IN",
	"NOT FLAG3_IN", "NOT TF",       "NBM",          "FOREVER"
};

const char sharc_disassembler::mr_regnames[16][8] =
{
	"MR0F", "MR1F", "MR2F", "MR0B", "MR1B", "MR2B", "???",  "???",
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???"
};

#define GET_UREG(x)     (ureg_names[x])
#define GET_SREG(x)     (GET_UREG(0x70 | (x & 0xf)))
#define GET_DREG(x)     (GET_UREG(0x00 | (x & 0xf)))
#define GET_DAG1_I(x)   (GET_UREG(0x10 | (x & 0x7)))
#define GET_DAG1_M(x)   (GET_UREG(0x20 | (x & 0x7)))
#define GET_DAG1_L(x)   (GET_UREG(0x30 | (x & 0x7)))
#define GET_DAG1_B(x)   (GET_UREG(0x40 | (x & 0x7)))
#define GET_DAG2_I(x)   (GET_UREG(0x10 | (8 + (x & 0x7))))
#define GET_DAG2_M(x)   (GET_UREG(0x20 | (8 + (x & 0x7))))
#define GET_DAG2_L(x)   (GET_UREG(0x30 | (8 + (x & 0x7))))
#define GET_DAG2_B(x)   (GET_UREG(0x40 | (8 + (x & 0x7))))

#define SIGN_EXTEND6(x)     ((x & 0x20) ? (0xffffffc0 | x) : x)
#define SIGN_EXTEND24(x)    ((x & 0x800000) ? (0xff000000 | x) : x)


void sharc_disassembler::compute(std::ostream &stream, uint32_t opcode)
{
	int op = (opcode >> 12) & 0xff;
	int cu = (opcode >> 20) & 0x3;
	int rn = (opcode >> 8) & 0xf;
	int rx = (opcode >> 4) & 0xf;
	int ry = (opcode >> 0) & 0xf;
	int rs = (opcode >> 12) & 0xf;
	int ra = rn;
	int rm = rs;

	if (opcode & 0x400000)      /* Multi-function opcode */
	{
		int multiop = (opcode >> 16) & 0x3f;
		int rxm = (opcode >> 6) & 0x3;
		int rym = (opcode >> 4) & 0x3;
		int rxa = (opcode >> 2) & 0x3;
		int rya = (opcode >> 0) & 0x3;

		switch(multiop)
		{
			case 0x04:      util::stream_format(stream, "R%d = R%d * R%d (SSFR),  R%d = R%d + R%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x05:      util::stream_format(stream, "R%d = R%d * R%d (SSFR),  R%d = R%d - R%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x06:      util::stream_format(stream, "R%d = R%d * R%d (SSFR),  R%d = (R%d + R%d)/2", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x08:      util::stream_format(stream, "MRF = MRF + R%d * R%d (SSF),  R%d = R%d + R%d", rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x09:      util::stream_format(stream, "MRF = MRF + R%d * R%d (SSF),  R%d = R%d - R%d", rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x0a:      util::stream_format(stream, "MRF = MRF + R%d * R%d (SSF),  R%d = (R%d + R%d)/2", rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x0c:      util::stream_format(stream, "R%d = MRF + R%d * R%d (SSFR),  R%d = R%d + R%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x0d:      util::stream_format(stream, "R%d = MRF + R%d * R%d (SSFR),  R%d = R%d - R%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x0e:      util::stream_format(stream, "R%d = MRF + R%d * R%d (SSFR),  R%d = (R%d + R%d)/2", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x10:      util::stream_format(stream, "MRF = MRF - R%d * R%d (SSF),  R%d = R%d + R%d", rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x11:      util::stream_format(stream, "MRF = MRF - R%d * R%d (SSF),  R%d = R%d - R%d", rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x12:      util::stream_format(stream, "MRF = MRF - R%d * R%d (SSF),  R%d = (R%d + R%d)/2", rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x14:      util::stream_format(stream, "R%d = MRF - R%d * R%d (SSFR),  R%d = R%d + R%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x15:      util::stream_format(stream, "R%d = MRF - R%d * R%d (SSFR),  R%d = R%d - R%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x16:      util::stream_format(stream, "R%d = MRF - R%d * R%d (SSFR),  R%d = (R%d + R%d)/2", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x18:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = F%d + F%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x19:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = F%d - F%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x1a:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = FLOAT F%d BY F%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x1b:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = FIX F%d BY F%d", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x1c:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = (F%d + F%d)/2", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x1d:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = ABS F%d", rm, rxm, rym+4, ra, rxa+8); break;
			case 0x1e:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = MAX(F%d, F%d)", rm, rxm, rym+4, ra, rxa+8, rya+12); break;
			case 0x1f:      util::stream_format(stream, "F%d = F%d * F%d,  F%d = MIN(F%d, F%d)", rm, rxm, rym+4, ra, rxa+8, rya+12); break;

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			{
				util::stream_format(stream, "R%d = R%d * R%d (SSFR),   R%d = R%d + R%d,   R%d = R%d - R%d", rm, rxm, rym+4, ra, rxa+8, rya+12, (opcode >> 16) & 0xf, rxa+8, rya+12);
				break;
			}

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			{
				util::stream_format(stream, "F%d = F%d * F%d,   F%d = F%d + F%d,   F%d = F%d - F%d", rm, rxm, rym+4, ra, rxa+8, rya+12, (opcode >> 16) & 0xf, rxa+8, rya+12);
				break;
			}

			case 0x00:
			{
				int rk = (opcode >> 8) & 0xf;
				int ai = (opcode >> 12) & 0xf;
				util::stream_format(stream, "R%d = %s", rk, mr_regnames[ai]);
				break;
			}
			case 0x01:
			{
				int rk = (opcode >> 8) & 0xf;
				int ai = (opcode >> 12) & 0xf;
				util::stream_format(stream, "%s = R%d", mr_regnames[ai], rk);
				break;
			}

			default:
			{
				util::stream_format(stream, "??? (COMPUTE, MULTIOP)");
				break;
			}
		}
	}
	else                        /* Single-function */
	{
		switch(cu)
		{
			/******************/
			/* ALU operations */
			/******************/
			case 0:
			{
				switch(op)
				{
					/* Fixed-point */
					case 0x01:  util::stream_format(stream, "R%d = R%d + R%d", rn, rx, ry); break;
					case 0x02:  util::stream_format(stream, "R%d = R%d - R%d", rn, rx, ry); break;
					case 0x05:  util::stream_format(stream, "R%d = R%d + R%d + CI", rn, rx, ry); break;
					case 0x06:  util::stream_format(stream, "R%d = R%d - R%d + CI - 1", rn, rx, ry); break;
					case 0x09:  util::stream_format(stream, "R%d = (R%d + R%d)/2", rn, rx, ry); break;
					case 0x0a:  util::stream_format(stream, "COMP(R%d, R%d)", rx, ry); break;
					case 0x25:  util::stream_format(stream, "R%d = R%d + CI", rn, rx); break;
					case 0x26:  util::stream_format(stream, "R%d = R%d + CI - 1", rn, rx); break;
					case 0x29:  util::stream_format(stream, "R%d = R%d + 1", rn, rx); break;
					case 0x2a:  util::stream_format(stream, "R%d = R%d - 1", rn, rx); break;
					case 0x22:  util::stream_format(stream, "R%d = -R%d", rn, rx); break;
					case 0x30:  util::stream_format(stream, "R%d = ABS R%d", rn, rx); break;
					case 0x21:  util::stream_format(stream, "R%d = PASS R%d", rn, rx); break;
					case 0x40:  util::stream_format(stream, "R%d = R%d AND R%d", rn, rx, ry); break;
					case 0x41:  util::stream_format(stream, "R%d = R%d OR R%d", rn, rx, ry); break;
					case 0x42:  util::stream_format(stream, "R%d = R%d XOR R%d", rn, rx, ry); break;
					case 0x43:  util::stream_format(stream, "R%d = NOT R%d", rn, rx); break;
					case 0x61:  util::stream_format(stream, "R%d = MIN(R%d, R%d)", rn, rx, ry); break;
					case 0x62:  util::stream_format(stream, "R%d = MAX(R%d, R%d)", rn, rx, ry); break;
					case 0x63:  util::stream_format(stream, "R%d = CLIP R%d BY R%d", rn, rx, ry); break;
					/* Floating-point */
					case 0x81:  util::stream_format(stream, "F%d = F%d + F%d", rn, rx, ry); break;
					case 0x82:  util::stream_format(stream, "F%d = F%d - F%d", rn, rx, ry); break;
					case 0x91:  util::stream_format(stream, "F%d = ABS(F%d + F%d)", rn, rx, ry); break;
					case 0x92:  util::stream_format(stream, "F%d = ABS(F%d - F%d)", rn, rx, ry); break;
					case 0x89:  util::stream_format(stream, "F%d = (F%d + F%d)/2", rn, rx, ry); break;
					case 0x8a:  util::stream_format(stream, "COMP(F%d, F%d)", rx, ry); break;
					case 0xa2:  util::stream_format(stream, "F%d = -F%d", rn, rx); break;
					case 0xb0:  util::stream_format(stream, "F%d = ABS F%d", rn, rx); break;
					case 0xa1:  util::stream_format(stream, "F%d = PASS F%d", rn, rx); break;
					case 0xa5:  util::stream_format(stream, "F%d = RND R%d", rn, rx); break;
					case 0xbd:  util::stream_format(stream, "F%d = SCALB F%d BY R%d", rn, rx, ry); break;
					case 0xad:  util::stream_format(stream, "R%d = MANT F%d", rn, rx); break;
					case 0xc1:  util::stream_format(stream, "R%d = LOGB F%d", rn, rx); break;
					case 0xd9:  util::stream_format(stream, "R%d = FIX F%d BY R%d", rn, rx, ry); break;
					case 0xc9:  util::stream_format(stream, "R%d = FIX F%d", rn, rx); break;
					case 0xdd:  util::stream_format(stream, "R%d = TRUNC F%d BY R%d", rn, rx, ry); break;
					case 0xcd:  util::stream_format(stream, "R%d = TRUNC F%d", rn, rx); break;
					case 0xda:  util::stream_format(stream, "F%d = FLOAT R%d BY R%d", rn, rx, ry); break;
					case 0xca:  util::stream_format(stream, "F%d = FLOAT R%d", rn, rx); break;
					case 0xc4:  util::stream_format(stream, "F%d = RECIPS F%d", rn, rx); break;
					case 0xc5:  util::stream_format(stream, "F%d = RSQRTS F%d", rn, rx); break;
					case 0xe0:  util::stream_format(stream, "F%d = F%d COPYSIGN F%d", rn, rx, ry); break;
					case 0xe1:  util::stream_format(stream, "F%d = MIN(F%d, F%d)", rn, rx, ry); break;
					case 0xe2:  util::stream_format(stream, "F%d = MAX(F%d, F%d)", rn, rx, ry); break;
					case 0xe3:  util::stream_format(stream, "F%d = CLIP F%d BY F%d", rn, rx, ry); break;

					case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
					case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					{
						util::stream_format(stream, "R%d = R%d + R%d,   R%d = R%d - R%d", ra, rx, ry, rs, rx, ry);
						break;
					}
					case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
					case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
					{
						util::stream_format(stream, "F%d = F%d + F%d,   F%d = F%d - F%d", ra, rx, ry, rs, rx, ry);
						break;
					}
					default:
					{
						util::stream_format(stream, "??? (COMPUTE, ALU)");
						break;
					}
				}
				break;
			}

			/*************************/
			/* Multiplier operations */
			/*************************/
			case 1:
			{
				if( op == 0x30 ) {
					util::stream_format(stream, "F%d = F%d * F%d", rn, rx, ry);
					return;
				}

				switch((op >> 1) & 0x3)
				{
					case 0:
					case 1:     util::stream_format(stream, "R%d = ", rn); break;
					case 2:     util::stream_format(stream, "MRF = "); break;
					case 3:     util::stream_format(stream, "MRB = "); break;
				}
				switch((op >> 6) & 0x3)
				{
					case 0:
						switch((op >> 4) & 0x3)
						{
							case 0:     util::stream_format(stream, "SAT %s", (op & 0x2) ? "MRB" : "MRF"); break;
							case 1:
								if (op & 0x8)
								{
									util::stream_format(stream, "RND %s", (op & 0x2) ? "MRB" : "MRF");
								}
								else
								{
									util::stream_format(stream, "0");
								}
								break;
						}
						break;

					case 1:
						util::stream_format(stream, "R%d * R%d", rx, ry); break;

					case 2:
						util::stream_format(stream, "%s +(R%d * R%d)", (op & 0x2) ? "MRB" : "MRF", rx, ry); break;

					case 3:
						util::stream_format(stream, "%s -(R%d * R%d)", (op & 0x2) ? "MRB" : "MRF", rx, ry); break;
				}
				break;
			}

			/**********************/
			/* Shifter operations */
			/**********************/
			case 2:
			{
				switch(op)
				{
					case 0x00:      util::stream_format(stream, "R%d = LSHIFT R%d BY R%d", rn, rx, ry); break;
					case 0x20:      util::stream_format(stream, "R%d = R%d OR LSHIFT R%d BY R%d", rn, rn, rx, ry); break;
					case 0x04:      util::stream_format(stream, "R%d = ASHIFT R%d BY R%d", rn, rx, ry); break;
					case 0x24:      util::stream_format(stream, "R%d = R%d OR ASHIFT R%d BY R%d", rn, rn, rx, ry); break;
					case 0x08:      util::stream_format(stream, "R%d = ROT R%d BY R%d", rn, rx, ry); break;
					case 0xc4:      util::stream_format(stream, "R%d = BCLR R%d BY R%d", rn, rx, ry); break;
					case 0xc0:      util::stream_format(stream, "R%d = BSET R%d BY R%d", rn, rx, ry); break;
					case 0xc8:      util::stream_format(stream, "R%d = BTGL R%d BY R%d", rn, rx, ry); break;
					case 0xcc:      util::stream_format(stream, "BTST R%d BY R%d", rx, ry); break;
					case 0x44:      util::stream_format(stream, "R%d = FDEP R%d BY R%d", rn, rx, ry); break;
					case 0x64:      util::stream_format(stream, "R%d = R%d OR FDEP R%d BY R%d", rn, rn, rx, ry); break;
					case 0x4c:      util::stream_format(stream, "R%d = FDEP R%d BY R%d (SE)", rn, rx, ry); break;
					case 0x6c:      util::stream_format(stream, "R%d = R%d OR FDEP R%d BY R%d (SE)", rn, rn, rx, ry); break;
					case 0x40:      util::stream_format(stream, "R%d = FEXT R%d BY R%d", rn, rx, ry); break;
					case 0x48:      util::stream_format(stream, "R%d = FEXT R%d BY R%d (SE)", rn, rx, ry); break;
					case 0x80:      util::stream_format(stream, "R%d = EXP R%d", rn, rx); break;
					case 0x84:      util::stream_format(stream, "R%d = EXP R%d (EX)", rn, rx); break;
					case 0x88:      util::stream_format(stream, "R%d = LEFTZ R%d", rn, rx); break;
					case 0x8c:      util::stream_format(stream, "R%d = LEFTO R%d", rn, rx); break;
					case 0x90:      util::stream_format(stream, "R%d = FPACK F%d", rn, rx); break;
					case 0x94:      util::stream_format(stream, "F%d = FUNPACK R%d", rn, rx); break;
					default:        util::stream_format(stream, "??? (COMPUTE, SHIFT)"); break;
				}
				break;
			}

			default:
			{
				util::stream_format(stream, "??? (COMPUTE)");
				break;
			}
		}
	}
}

void sharc_disassembler::get_if_condition(std::ostream &stream, int cond)
{
	if (cond != 31)
	{
		util::stream_format(stream, "IF %s, ", condition_codes_if[cond]);
	}
}

void sharc_disassembler::pm_dm_ureg(std::ostream &stream, int g, int d, int i, int m, int ureg, int update)
{
	if (update)     // post-modify
	{
		if (d)
		{
			if (g)
			{
				util::stream_format(stream, "PM(%s, %s) = %s", GET_DAG2_I(i), GET_DAG2_M(m), GET_UREG(ureg));
			}
			else
			{
				util::stream_format(stream, "DM(%s, %s) = %s", GET_DAG1_I(i), GET_DAG1_M(m), GET_UREG(ureg));
			}
		}
		else
		{
			if (g)
			{
				util::stream_format(stream, "%s = PM(%s, %s)", GET_UREG(ureg), GET_DAG2_I(i), GET_DAG2_M(m));
			}
			else
			{
				util::stream_format(stream, "%s = DM(%s, %s)", GET_UREG(ureg), GET_DAG1_I(i), GET_DAG1_M(m));
			}
		}

	}
	else            // pre-modify
	{
		if (d)
		{
			if (g)
			{
				util::stream_format(stream, "PM(%s, %s) = %s", GET_DAG2_M(m), GET_DAG2_I(i), GET_UREG(ureg));
			}
			else
			{
				util::stream_format(stream, "DM(%s, %s) = %s", GET_DAG1_M(m), GET_DAG1_I(i), GET_UREG(ureg));
			}
		}
		else
		{
			if (g)
			{
				util::stream_format(stream, "%s = PM(%s, %s)", GET_UREG(ureg), GET_DAG2_M(m), GET_DAG2_I(i));
			}
			else
			{
				util::stream_format(stream, "%s = DM(%s, %s)", GET_UREG(ureg), GET_DAG1_M(m), GET_DAG1_I(i));
			}
		}
	}
}

void sharc_disassembler::pm_dm_imm_dreg(std::ostream &stream, int g, int d, int i, int data, int dreg, int update)
{
	const char *sign = "";
	if (data & 0x20)
	{
		/* negative sign */
		data = (data ^ 0x3f) + 1;
		sign = "-";
	}
	if (update)     // post-modify
	{
		if (d)
		{
			if (g)
			{
				util::stream_format(stream, "PM(%s, %s0x%02X) = %s", GET_DAG2_I(i), sign, data, GET_DREG(dreg));
			}
			else
			{
				util::stream_format(stream, "DM(%s, %s0x%02X) = %s", GET_DAG1_I(i), sign, data, GET_DREG(dreg));
			}
		}
		else
		{
			if (g)
			{
				util::stream_format(stream, "%s = PM(%s, %s0x%02X)", GET_DREG(dreg), GET_DAG2_I(i), sign, data);
			}
			else
			{
				util::stream_format(stream, "%s = DM(%s, %s0x%02X)", GET_DREG(dreg), GET_DAG1_I(i), sign, data);
			}
		}
	}
	else            // pre-modify
	{
		if (d)
		{
			if (g)
			{
				util::stream_format(stream, "PM(%s0x%02X, %s) = %s", sign, data, GET_DAG2_I(i), GET_DREG(dreg));
			}
			else
			{
				util::stream_format(stream, "DM(%s0x%02X, %s) = %s", sign, data, GET_DAG1_I(i), GET_DREG(dreg));
			}
		}
		else
		{
			if (g)
			{
				util::stream_format(stream, "%s = PM(%s0x%02X, %s)", GET_DREG(dreg), sign, data, GET_DAG2_I(i));
			}
			else
			{
				util::stream_format(stream, "%s = DM(%s0x%02X, %s)", GET_DREG(dreg), sign, data, GET_DAG1_I(i));
			}
		}
	}
}

void sharc_disassembler::pm_dm_dreg(std::ostream &stream, int g, int d, int i, int m, int dreg)
{
	if (d)
	{
		if (g)
		{
			util::stream_format(stream, "PM(%s, %s) = %s", GET_DAG2_I(i), GET_DAG2_M(m), GET_DREG(dreg));
		}
		else
		{
			util::stream_format(stream, "DM(%s, %s) = %s", GET_DAG1_I(i), GET_DAG1_M(m), GET_DREG(dreg));
		}
	}
	else
	{
		if (g)
		{
			util::stream_format(stream, "%s = PM(%s, %s)", GET_DREG(dreg), GET_DAG2_I(i), GET_DAG2_M(m));
		}
		else
		{
			util::stream_format(stream, "%s = DM(%s, %s)", GET_DREG(dreg), GET_DAG1_I(i), GET_DAG1_M(m));
		}
	}
}

void sharc_disassembler::shiftop(std::ostream &stream, int shift, int data, int rn, int rx)
{
	int8_t data8 = data & 0xff;
	int bit6 = data & 0x3f;
	int len = (data >> 6) & 0x3f;

	switch(shift)
	{
		case 0x00:      util::stream_format(stream, "R%d = LSHIFT R%d BY %d", rn, rx, data8); break;
		case 0x08:      util::stream_format(stream, "R%d = R%d OR LSHIFT R%d BY %d", rn, rn, rx, data8); break;
		case 0x01:      util::stream_format(stream, "R%d = ASHIFT R%d BY %d", rn, rx, data8); break;
		case 0x09:      util::stream_format(stream, "R%d = R%d OR ASHIFT R%d BY %d", rn, rn, rx, data8); break;
		case 0x02:      util::stream_format(stream, "R%d = ROT R%d BY %d", rn, rx, data8); break;
		case 0x31:      util::stream_format(stream, "R%d = BCLR R%d BY %d", rn, rx, data8); break;
		case 0x30:      util::stream_format(stream, "R%d = BSET R%d BY %d", rn, rx, data8); break;
		case 0x32:      util::stream_format(stream, "R%d = BTGL R%d BY %d", rn, rx, data8); break;
		case 0x33:      util::stream_format(stream, "BTST R%d BY %d", rx, data8); break;
		case 0x11:      util::stream_format(stream, "R%d = FDEP R%d BY %d:%d", rn, rx, bit6, len); break;
		case 0x19:      util::stream_format(stream, "R%d = R%d OR FDEP R%d BY %d:%d", rn, rn, rx, bit6, len); break;
		case 0x13:      util::stream_format(stream, "R%d = FDEP R%d BY %d:%d (SE)", rn, rx, bit6, len); break;
		case 0x1b:      util::stream_format(stream, "R%d = R%d OR FDEP R%d BY %d:%d (SE)", rn, rn, rx, bit6, len); break;
		case 0x10:      util::stream_format(stream, "R%d = FEXT R%d BY %d:%d", rn, rx, bit6, len); break;
		case 0x12:      util::stream_format(stream, "R%d = FEXT R%d BY %d:%d (SE)", rn, rx, bit6, len); break;
		case 0x20:      util::stream_format(stream, "R%d = EXP R%d", rn, rx); break;
		case 0x21:      util::stream_format(stream, "R%d = EXP R%d (EX)", rn, rx); break;
		case 0x22:      util::stream_format(stream, "R%d = LEFTZ R%d", rn, rx); break;
		case 0x23:      util::stream_format(stream, "R%d = LEFTO R%d", rn, rx); break;
		case 0x24:      util::stream_format(stream, "R%d = FPACK F%d", rn, rx); break;
		case 0x25:      util::stream_format(stream, "F%d = FUNPACK R%d", rn, rx); break;
		default:        util::stream_format(stream, "??? (SHIFTOP)"); break;
	}
}

uint32_t sharc_disassembler::dasm_compute_dreg_dmpm(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int dmi = (opcode >> 41) & 0x7;
	int dmm = (opcode >> 38) & 0x7;
	int pmi = (opcode >> 30) & 0x7;
	int pmm = (opcode >> 27) & 0x7;
	int dmdreg = (opcode >> 33) & 0xf;
	int pmdreg = (opcode >> 23) & 0xf;
	int comp = opcode & 0x7fffff;
	int dmd = (opcode >> 44) & 0x1;
	int pmd = (opcode >> 37) & 0x1;

	if (comp)
	{
		compute(stream, comp);
		util::stream_format(stream, ",  ");
	}
	if (dmd)
	{
		util::stream_format(stream, "DM(%s, %s) = R%d, ", GET_DAG1_I(dmi), GET_DAG1_M(dmm), dmdreg);
	}
	else
	{
		util::stream_format(stream, "R%d = DM(%s, %s), ", dmdreg, GET_DAG1_I(dmi), GET_DAG1_M(dmm));
	}
	if (pmd)
	{
		util::stream_format(stream, "PM(%s, %s) = R%d", GET_DAG2_I(pmi), GET_DAG2_M(pmm), pmdreg);
	}
	else
	{
		util::stream_format(stream, "R%d = PM(%s, %s)", pmdreg, GET_DAG2_I(pmi), GET_DAG2_M(pmm));
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_compute(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 33) & 0x1f;
	int comp = opcode & 0x7fffff;

	if (comp)
	{
		get_if_condition(stream, cond);
		compute(stream, comp);
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_compute_uregdmpm_regmod(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 33) & 0x1f;
	int g = (opcode >> 32) & 0x1;
	int d = (opcode >> 31) & 0x1;
	int i = (opcode >> 41) & 0x7;
	int m = (opcode >> 38) & 0x7;
	int u = (opcode >> 44) & 0x1;
	int ureg = (opcode >> 23) & 0xff;
	int comp = opcode & 0x7fffff;

	get_if_condition(stream, cond);
	if (comp)
	{
		compute(stream, comp);
		util::stream_format(stream, ",  ");
	}
	pm_dm_ureg(stream, g,d,i,m, ureg, u);
	return 0;
}

uint32_t sharc_disassembler::dasm_compute_dregdmpm_immmod(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 33) & 0x1f;
	int g = (opcode >> 40) & 0x1;
	int d = (opcode >> 39) & 0x1;
	int i = (opcode >> 41) & 0x7;
	int u = (opcode >> 38) & 0x1;
	int dreg = (opcode >> 23) & 0xf;
	int data = (opcode >> 27) & 0x3f;
	int comp = opcode & 0x7fffff;

	get_if_condition(stream, cond);
	if (comp)
	{
		compute(stream, comp);
		util::stream_format(stream, ",  ");
	}
	pm_dm_imm_dreg(stream, g,d,i, data, dreg, u);
	return 0;
}

uint32_t sharc_disassembler::dasm_compute_ureg_ureg(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 31) & 0x1f;
	int uregs = (opcode >> 36) & 0xff;
	int uregd = (opcode >> 23) & 0xff;
	int comp = opcode & 0x7fffff;

	get_if_condition(stream, cond);
	if (comp)
	{
		compute(stream, comp);
		util::stream_format(stream, ",  ");
	}
	util::stream_format(stream, "%s = %s", GET_UREG(uregd), GET_UREG(uregs));
	return 0;
}

uint32_t sharc_disassembler::dasm_immshift_dregdmpm(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 33) & 0x1f;
	int g = (opcode >> 32) & 0x1;
	int d = (opcode >> 31) & 0x1;
	int i = (opcode >> 41) & 0x7;
	int m = (opcode >> 38) & 0x7;
	int rn = (opcode >> 4) & 0xf;
	int rx = (opcode >> 0) & 0xf;
	int shift = (opcode >> 16) & 0x3f;
	int dreg = (opcode >> 23) & 0xf;
	int data = (((opcode >> 27) & 0xf) << 8) | ((opcode >> 8) & 0xff);

	get_if_condition(stream, cond);
	shiftop(stream, shift, data, rn, rx);
	util::stream_format(stream, ",  ");
	pm_dm_dreg(stream, g,d,i,m, dreg);
	return 0;
}

uint32_t sharc_disassembler::dasm_immshift_dregdmpm_nodata(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 33) & 0x1f;
	int rn = (opcode >> 4) & 0xf;
	int rx = (opcode >> 0) & 0xf;
	int shift = (opcode >> 16) & 0x3f;
	int data = (((opcode >> 27) & 0xf) << 8) | ((opcode >> 8) & 0xff);

	get_if_condition(stream, cond);
	shiftop(stream, shift, data, rn, rx);
	return 0;
}

uint32_t sharc_disassembler::dasm_compute_modify(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int cond = (opcode >> 33) & 0x1f;
	int g = (opcode >> 38) & 0x7;
	int i = (opcode >> 30) & 0x7;
	int m = (opcode >> 27) & 0x7;
	int comp = opcode & 0x7fffff;

	get_if_condition(stream, cond);
	if (comp)
	{
		compute(stream, comp);
		util::stream_format(stream, ",  ");
	}
	util::stream_format(stream, "MODIFY(I%d, M%d)", (g ? 8+i : i), (g ? 8+m : m));
	return 0;
}

uint32_t sharc_disassembler::dasm_direct_jump(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int j = (opcode >> 26) & 0x1;
	int cond = (opcode >> 33) & 0x1f;
	int ci = (opcode >> 24) & 0x1;
	uint32_t addr = opcode & 0xffffff;
	uint32_t flags = 0;

	get_if_condition(stream, cond);
	if (opcode & 0x8000000000U)
	{
		util::stream_format(stream, "CALL");
		flags = STEP_OVER;
	}
	else
	{
		util::stream_format(stream, "JUMP");
	}
	if (cond != 31)
		flags |= STEP_COND;

	if (opcode & 0x10000000000U)    /* PC-relative branch */
	{
		util::stream_format(stream, " (0x%08X)", pc + SIGN_EXTEND24(addr));
	}
	else                                /* Indirect branch */
	{
		util::stream_format(stream, " (0x%08X)", addr);
	}
	if (j)
	{
		util::stream_format(stream, " (DB)");
		flags |= step_over_extra(2);
	}
	if (ci)
	{
		util::stream_format(stream, " (CI)");
	}
	return flags;
}

uint32_t sharc_disassembler::dasm_indirect_jump_compute(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int b = (opcode >> 39) & 0x1;
	int j = (opcode >> 26) & 0x1;
	int e = (opcode >> 25) & 0x1;
	int ci = (opcode >> 24) & 0x1;
	int cond = (opcode >> 33) & 0x1f;
	int pmi = (opcode >> 30) & 0x7;
	int pmm = (opcode >> 27) & 0x7;
	int reladdr = (opcode >> 27) & 0x3f;
	int comp = opcode & 0x7fffff;
	uint32_t flags = 0;

	get_if_condition(stream, cond);
	if (b)
	{
		util::stream_format(stream, "CALL");
		flags = STEP_OVER;
	}
	else
	{
		util::stream_format(stream, "JUMP");
	}
	if (cond != 31)
		flags |= STEP_COND;

	if (opcode & 0x10000000000U)    /* PC-relative branch */
	{
		util::stream_format(stream, " (0x%08X)", pc + SIGN_EXTEND6(reladdr));
	}
	else                                /* Indirect branch */
	{
		util::stream_format(stream, " (%s, %s)", GET_DAG2_M(pmm), GET_DAG2_I(pmi));
	}
	if (j)
	{
		util::stream_format(stream, " (DB)");
		flags |= step_over_extra(2);
	}
	if (ci)
	{
		util::stream_format(stream, " (CI)");
	}

	if (comp)
	{
		util::stream_format(stream, ", ");
		if (e)
		{
			util::stream_format(stream, "ELSE ");
		}

		compute(stream, comp);
	}
	return flags;
}

uint32_t sharc_disassembler::dasm_indirect_jump_compute_dregdm(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int d = (opcode >> 44) & 0x1;
	int cond = (opcode >> 33) & 0x1f;
	int pmi = (opcode >> 30) & 0x7;
	int pmm = (opcode >> 27) & 0x7;
	int dmi = (opcode >> 41) & 0x7;
	int dmm = (opcode >> 38) & 0x7;
	int reladdr = (opcode >> 27) & 0x3f;
	int dreg = (opcode >> 23) & 0xf;
	int comp = opcode & 0x7fffff;
	uint32_t flags = cond != 31 ? STEP_COND : 0;

	get_if_condition(stream, cond);
	util::stream_format(stream, "JUMP");

	if (opcode & 0x200000000000U)   /* PC-relative branch */
	{
		util::stream_format(stream, " (0x%08X)", pc + SIGN_EXTEND6(reladdr));
	}
	else                                /* Indirect branch */
	{
		util::stream_format(stream, " (%s, %s)", GET_DAG2_M(pmm), GET_DAG2_I(pmi));
	}
	util::stream_format(stream, ", ELSE ");

	if (comp)
	{
		compute(stream, comp);
		util::stream_format(stream, ",  ");
	}
	if (d)
	{
		util::stream_format(stream, "%s = DM(%s, %s)", GET_DREG(dreg), GET_DAG1_I(dmi), GET_DAG1_M(dmm));
	}
	else
	{
		util::stream_format(stream, "DM(%s, %s) = %s", GET_DAG1_I(dmi), GET_DAG1_M(dmm), GET_DREG(dreg));
	}
	return flags;
}

uint32_t sharc_disassembler::dasm_rts_compute(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int j = (opcode >> 26) & 0x1;
	int e = (opcode >> 25) & 0x1;
	int lr = (opcode >> 24) & 0x1;
	int cond = (opcode >> 33) & 0x1f;
	int comp = opcode & 0x7fffff;
	uint32_t flags = STEP_OUT;

	get_if_condition(stream, cond);
	if (cond != 31)
		flags |= STEP_COND;

	if (opcode & 0x10000000000U)
	{
		util::stream_format(stream, "RTI");
	}
	else
	{
		util::stream_format(stream, "RTS");
	}

	if (j)
	{
		util::stream_format(stream, " (DB)");
		flags |= step_over_extra(2);
	}
	if (lr)
	{
		util::stream_format(stream, " (LR)");
	}

	if (comp)
	{
		util::stream_format(stream, ", ");
		if (e)
		{
			util::stream_format(stream, "ELSE ");
		}

		compute(stream, comp);
	}
	return flags;
}

uint32_t sharc_disassembler::dasm_do_until_counter(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int data = (opcode >> 24) & 0xffff;
	int ureg = (opcode >> 32) & 0xff;
	uint32_t addr = opcode & 0xffffff;

	if (opcode & 0x10000000000U)    /* Loop counter from universal register */
	{
		util::stream_format(stream, "LCNTR = %s, ", GET_UREG(ureg));
		util::stream_format(stream, "DO (0x%08X)", pc + SIGN_EXTEND24(addr));
	}
	else                                /* Loop counter from immediate */
	{
		util::stream_format(stream, "LCNTR = 0x%04X, ", data);
		util::stream_format(stream, "DO (0x%08X) UNTIL LCE", pc + SIGN_EXTEND24(addr));
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_do_until(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int term = (opcode >> 33) & 0x1f;
	uint32_t addr = opcode & 0xffffff;

	util::stream_format(stream, "DO (0x%08X) UNTIL %s", pc + SIGN_EXTEND24(addr), condition_codes_do[term]);
	return 0;
}

uint32_t sharc_disassembler::dasm_immmove_uregdmpm(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int d = (opcode >> 40) & 0x1;
	int g = (opcode >> 41) & 0x1;
	int ureg = (opcode >> 32) & 0xff;
	uint32_t addr = opcode & 0xffffffff;

	if (g)
	{
		if (d)
		{
			util::stream_format(stream, "PM(0x%08X) = %s", addr, GET_UREG(ureg));
		}
		else
		{
			util::stream_format(stream, "%s = PM(0x%08X)", GET_UREG(ureg), addr);
		}
	}
	else
	{
		if (d)
		{
			util::stream_format(stream, "DM(0x%08X) = %s", addr, GET_UREG(ureg));
		}
		else
		{
			util::stream_format(stream, "%s = DM(0x%08X)", GET_UREG(ureg), addr);
		}
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_immmove_uregdmpm_indirect(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int d = (opcode >> 40) & 0x1;
	int g = (opcode >> 44) & 0x1;
	int i = (opcode >> 41) & 0x7;
	int ureg = (opcode >> 32) & 0xff;
	uint32_t addr = opcode & 0xffffffff;

	if (g)
	{
		if (d)
		{
			util::stream_format(stream, "PM(0x%08X, %s) = %s", addr, GET_DAG2_I(i), GET_UREG(ureg));
		}
		else
		{
			util::stream_format(stream, "%s = PM(0x%08X, %s)", GET_UREG(ureg), addr, GET_DAG2_I(i));
		}
	}
	else
	{
		if (d)
		{
			util::stream_format(stream, "DM(0x%08X, %s) = %s", addr, GET_DAG1_I(i), GET_UREG(ureg));
		}
		else
		{
			util::stream_format(stream, "%s = DM(0x%08X, %s)", GET_UREG(ureg), addr, GET_DAG1_I(i));
		}
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_immmove_immdata_dmpm(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int g = (opcode >> 37) & 0x1;
	int i = (opcode >> 41) & 0x7;
	int m = (opcode >> 38) & 0x7;
	uint32_t data = opcode & 0xffffffff;

	if (g)
	{
		util::stream_format(stream, "PM(%s, %s) = 0x%08X", GET_DAG2_I(i), GET_DAG2_M(m), data);
	}
	else
	{
		util::stream_format(stream, "DM(%s, %s) = 0x%08X", GET_DAG1_I(i), GET_DAG1_M(m), data);
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_immmove_immdata_ureg(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int ureg = (opcode >> 32) & 0xff;
	uint32_t data = opcode & 0xffffffff;

	util::stream_format(stream, "%s = 0x%08X", GET_UREG(ureg), data);
	return 0;
}

uint32_t sharc_disassembler::dasm_sysreg_bitop(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int bop = (opcode >> 37) & 0x7;
	int sreg = (opcode >> 32) & 0xf;
	uint32_t data = opcode & 0xffffffff;

	util::stream_format(stream, "BIT ");
	util::stream_format(stream, "%s ", bopnames[bop]);
	util::stream_format(stream, "%s ", GET_SREG(sreg));
	util::stream_format(stream, "0x%08X", data);
	return 0;
}

uint32_t sharc_disassembler::dasm_ireg_modify(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int g = (opcode >> 38) & 0x1;
	int i = (opcode >> 32) & 0x7;
	uint32_t data = opcode & 0xffffffff;

	if (opcode & 0x8000000000U)     /* with bit-reverse */
	{
		if (g)
		{
			util::stream_format(stream, "BITREV (%s, 0x%08X)", GET_DAG2_I(i), data);
		}
		else
		{
			util::stream_format(stream, "BITREV (%s, 0x%08X)", GET_DAG1_I(i), data);
		}
	}
	else                                /* without bit-reverse */
	{
		if (g)
		{
			util::stream_format(stream, "MODIFY (%s, 0x%08X)", GET_DAG2_I(i), data);
		}
		else
		{
			util::stream_format(stream, "MODIFY (%s, 0x%08X)", GET_DAG1_I(i), data);
		}
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_misc(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	int bits = (opcode >> 33) & 0x7f;
	int lpu = (opcode >> 39) & 0x1;
	int lpo = (opcode >> 38) & 0x1;
	int spu = (opcode >> 37) & 0x1;
	int spo = (opcode >> 36) & 0x1;
	int ppu = (opcode >> 35) & 0x1;
	int ppo = (opcode >> 34) & 0x1;
	int fc = (opcode >> 33) & 0x1;

	if (lpu)
	{
		util::stream_format(stream, "PUSH LOOP");
		if (bits & 0x3f)
		{
			util::stream_format(stream, ", ");
		}
	}
	if (lpo)
	{
		util::stream_format(stream, "POP LOOP");
		if (bits & 0x1f)
		{
			util::stream_format(stream, ", ");
		}
	}
	if (spu)
	{
		util::stream_format(stream, "PUSH STS");
		if (bits & 0xf)
		{
			util::stream_format(stream, ", ");
		}
	}
	if (spo)
	{
		util::stream_format(stream, "POP STS");
		if (bits & 0x7)
		{
			util::stream_format(stream, ", ");
		}
	}
	if (ppu)
	{
		util::stream_format(stream, "PUSH PCSTK");
		if (bits & 0x3)
		{
			util::stream_format(stream, ", ");
		}
	}
	if (ppo)
	{
		util::stream_format(stream, "POP PCSTK");
		if (bits & 0x1)
		{
			util::stream_format(stream, ", ");
		}
	}
	if (fc)
	{
		util::stream_format(stream, "FLUSH CACHE");
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_idlenop(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	if (opcode & 0x8000000000U)
	{
		util::stream_format(stream, "IDLE");
	}
	else
	{
		util::stream_format(stream, "NOP");
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_cjump_rframe(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	/* TODO */
	if (opcode & 0x10000000000U)    /* RFRAME */
	{
		util::stream_format(stream, "TODO: RFRAME");
	}
	else
	{
		util::stream_format(stream, "TODO: CJUMP");
	}
	return 0;
}

uint32_t sharc_disassembler::dasm_invalid(std::ostream &stream, uint32_t pc, uint64_t opcode)
{
	util::stream_format(stream, "?");
	return 0;
}

const sharc_disassembler::SHARC_DASM_OP sharc_disassembler::sharc_dasm_ops[] =
{
	//  |0 0 1|
	{   0xe000,     0x2000,     &sharc_disassembler::dasm_compute_dreg_dmpm                              },

	//  |0 0 0|0 0 0 0 1|
	{   0xff00,     0x0100,     &sharc_disassembler::dasm_compute                                        },

	//  |0 1 0|
	{   0xe000,     0x4000,     &sharc_disassembler::dasm_compute_uregdmpm_regmod                        },

	//  |0 1 1|0|
	{   0xf000,     0x6000,     &sharc_disassembler::dasm_compute_dregdmpm_immmod                        },

	//  |0 1 1|1|
	{   0xf000,     0x7000,     &sharc_disassembler::dasm_compute_ureg_ureg                              },

	//  |1 0 0|0|
	{   0xf000,     0x8000,     &sharc_disassembler::dasm_immshift_dregdmpm                              },

	//  |0 0 0|0 0 0 1 0|
	{   0xff00,     0x0200,     &sharc_disassembler::dasm_immshift_dregdmpm_nodata                       },

	//  |0 0 0|0 0 1 0 0|
	{   0xff00,     0x0400,     &sharc_disassembler::dasm_compute_modify                                 },

	//  |0 0 0|0 0 1 1 x|
	{   0xfe00,     0x0600,     &sharc_disassembler::dasm_direct_jump                                    },

	//  |0 0 0|0 1 0 0 x|
	{   0xfe00,     0x0800,     &sharc_disassembler::dasm_indirect_jump_compute                          },

	//  |1 1 x|
	{   0xc000,     0xc000,     &sharc_disassembler::dasm_indirect_jump_compute_dregdm                   },

	//  |0 0 0|0 1 0 1 x|
	{   0xfe00,     0x0a00,     &sharc_disassembler::dasm_rts_compute                                    },

	//  |0 0 0|0 1 1 0 x|
	{   0xfe00,     0x0c00,     &sharc_disassembler::dasm_do_until_counter                               },

	//  |0 0 0|0 1 1 1 0|
	{   0xff00,     0x0e00,     &sharc_disassembler::dasm_do_until                                       },

	//  |0 0 0|1 0 0|x|x|
	{   0xfc00,     0x1000,     &sharc_disassembler::dasm_immmove_uregdmpm                               },

	//  |1 0 1|x|x x x|x|
	{   0xe000,     0xa000,     &sharc_disassembler::dasm_immmove_uregdmpm_indirect                      },

	//  |1 0 0|1|
	{   0xf000,     0x9000,     &sharc_disassembler::dasm_immmove_immdata_dmpm                           },

	//  |0 0 0|0 1 1 1 1|
	{   0xff00,     0x0f00,     &sharc_disassembler::dasm_immmove_immdata_ureg                           },

	//  |0 0 0|1 0 1 0 0|
	{   0xff00,     0x1400,     &sharc_disassembler::dasm_sysreg_bitop                                   },

	//  |0 0 0|1 0 1 1 0|
	{   0xff00,     0x1600,     &sharc_disassembler::dasm_ireg_modify                                    },

	//  |0 0 0|1 0 1 1 1|
	{   0xff00,     0x1700,     &sharc_disassembler::dasm_misc                                           },

	//  |0 0 0|0 0 0 0 0|
	{   0xff00,     0x0000,     &sharc_disassembler::dasm_idlenop                                        },
};

sharc_disassembler::sharc_disassembler()
{
	int i, j;
	int num_ops = sizeof(sharc_dasm_ops) / sizeof(SHARC_DASM_OP);

	for (i=0; i < 256; i++)
	{
		sharcdasm_table[i] = &sharc_disassembler::dasm_invalid;
	}

	for (i=0; i < 256; i++)
	{
		uint16_t op = i << 8;

		for (j=0; j < num_ops; j++)
		{
			if ((sharc_dasm_ops[j].op_mask & op) == sharc_dasm_ops[j].op_bits)
			{
				if (sharcdasm_table[i] != &sharc_disassembler::dasm_invalid)
				{
					throw std::logic_error(util::string_format("build_dasm_table: table already filled! (i=%04X, j=%d)\n", i, j));
				}
				else
				{
					sharcdasm_table[i] = sharc_dasm_ops[j].handler;
				}
			}
		}
	}
}

offs_t sharc_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u64 opcode = opcodes.r64(pc);
	int op = (opcode >> 40) & 0xff;

	return 1 | (this->*sharcdasm_table[op])(stream, pc, opcode) | SUPPORTED;
}

u32 sharc_disassembler::opcode_alignment() const
{
	return 1;
}
