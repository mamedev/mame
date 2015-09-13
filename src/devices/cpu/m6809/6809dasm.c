// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Sean Riddle
/*****************************************************************************

    6809dasm.c - a 6809 opcode disassembler
    Version 1.4 1-MAR-95
    Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    Freely distributable on any medium given all copyrights are retained
    by the author and no charge greater than $7.00 is made for obtaining
    this software

    Please send all bug reports, update ideas and data files to:
    sriddle@ionet.net

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "m6809.h"
#include "m6809inl.h"

// Opcode structure
struct opcodeinfo
{
	UINT8   opcode;     // 8-bit opcode value
	UINT8   length;     // Opcode length in bytes
	char    name[6];    // Opcode name
	UINT8   mode;       // Addressing mode
	unsigned flags;     // Disassembly flags
};

enum m6809_addressing_modes
{
	INH,                // Inherent
	DIR,                // Direct
	IND,                // Indexed
	REL,                // Relative (8 bit)
	LREL,               // Long relative (16 bit)
	EXT,                // Extended
	IMM,                // Immediate
	IMM_RR,             // Register-to-register
	PG1,                // Switch to page 1 opcodes
	PG2                 // Switch to page 2 opcodes
};

// Page 0 opcodes (single byte)
static const opcodeinfo m6809_pg0opcodes[] =
{
	{ 0x00, 2, "NEG",   DIR    },
	{ 0x03, 2, "COM",   DIR    },
	{ 0x04, 2, "LSR",   DIR    },
	{ 0x06, 2, "ROR",   DIR    },
	{ 0x07, 2, "ASR",   DIR    },
	{ 0x08, 2, "ASL",   DIR    },
	{ 0x09, 2, "ROL",   DIR    },
	{ 0x0A, 2, "DEC",   DIR    },
	{ 0x0C, 2, "INC",   DIR    },
	{ 0x0D, 2, "TST",   DIR    },
	{ 0x0E, 2, "JMP",   DIR    },
	{ 0x0F, 2, "CLR",   DIR    },

	{ 0x10, 1, "page1", PG1    },
	{ 0x11, 1, "page2", PG2    },
	{ 0x12, 1, "NOP",   INH    },
	{ 0x13, 1, "SYNC",  INH    },
	{ 0x16, 3, "LBRA",  LREL   },
	{ 0x17, 3, "LBSR",  LREL    , DASMFLAG_STEP_OVER },
	{ 0x19, 1, "DAA",   INH    },
	{ 0x1A, 2, "ORCC",  IMM    },
	{ 0x1C, 2, "ANDCC", IMM    },
	{ 0x1D, 1, "SEX",   INH    },
	{ 0x1E, 2, "EXG",   IMM_RR },
	{ 0x1F, 2, "TFR",   IMM_RR },

	{ 0x20, 2, "BRA",   REL    },
	{ 0x21, 2, "BRN",   REL    },
	{ 0x22, 2, "BHI",   REL    },
	{ 0x23, 2, "BLS",   REL    },
	{ 0x24, 2, "BCC",   REL    },
	{ 0x25, 2, "BCS",   REL    },
	{ 0x26, 2, "BNE",   REL    },
	{ 0x27, 2, "BEQ",   REL    },
	{ 0x28, 2, "BVC",   REL    },
	{ 0x29, 2, "BVS",   REL    },
	{ 0x2A, 2, "BPL",   REL    },
	{ 0x2B, 2, "BMI",   REL    },
	{ 0x2C, 2, "BGE",   REL    },
	{ 0x2D, 2, "BLT",   REL    },
	{ 0x2E, 2, "BGT",   REL    },
	{ 0x2F, 2, "BLE",   REL    },

	{ 0x30, 2, "LEAX",  IND    },
	{ 0x31, 2, "LEAY",  IND    },
	{ 0x32, 2, "LEAS",  IND    },
	{ 0x33, 2, "LEAU",  IND    },
	{ 0x34, 2, "PSHS",  INH    },
	{ 0x35, 2, "PULS",  INH    },
	{ 0x36, 2, "PSHU",  INH    },
	{ 0x37, 2, "PULU",  INH    },
	{ 0x39, 1, "RTS",   INH    },
	{ 0x3A, 1, "ABX",   INH    },
	{ 0x3B, 1, "RTI",   INH    },
	{ 0x3C, 2, "CWAI",  IMM    },
	{ 0x3D, 1, "MUL",   INH    },
	{ 0x3F, 1, "SWI",   INH    },

	{ 0x40, 1, "NEGA",  INH    },
	{ 0x43, 1, "COMA",  INH    },
	{ 0x44, 1, "LSRA",  INH    },
	{ 0x46, 1, "RORA",  INH    },
	{ 0x47, 1, "ASRA",  INH    },
	{ 0x48, 1, "ASLA",  INH    },
	{ 0x49, 1, "ROLA",  INH    },
	{ 0x4A, 1, "DECA",  INH    },
	{ 0x4C, 1, "INCA",  INH    },
	{ 0x4D, 1, "TSTA",  INH    },
	{ 0x4F, 1, "CLRA",  INH    },

	{ 0x50, 1, "NEGB",  INH    },
	{ 0x53, 1, "COMB",  INH    },
	{ 0x54, 1, "LSRB",  INH    },
	{ 0x56, 1, "RORB",  INH    },
	{ 0x57, 1, "ASRB",  INH    },
	{ 0x58, 1, "ASLB",  INH    },
	{ 0x59, 1, "ROLB",  INH    },
	{ 0x5A, 1, "DECB",  INH    },
	{ 0x5C, 1, "INCB",  INH    },
	{ 0x5D, 1, "TSTB",  INH    },
	{ 0x5F, 1, "CLRB",  INH    },

	{ 0x60, 2, "NEG",   IND    },
	{ 0x63, 2, "COM",   IND    },
	{ 0x64, 2, "LSR",   IND    },
	{ 0x66, 2, "ROR",   IND    },
	{ 0x67, 2, "ASR",   IND    },
	{ 0x68, 2, "ASL",   IND    },
	{ 0x69, 2, "ROL",   IND    },
	{ 0x6A, 2, "DEC",   IND    },
	{ 0x6C, 2, "INC",   IND    },
	{ 0x6D, 2, "TST",   IND    },
	{ 0x6E, 2, "JMP",   IND    },
	{ 0x6F, 2, "CLR",   IND    },

	{ 0x70, 3, "NEG",   EXT    },
	{ 0x73, 3, "COM",   EXT    },
	{ 0x74, 3, "LSR",   EXT    },
	{ 0x76, 3, "ROR",   EXT    },
	{ 0x77, 3, "ASR",   EXT    },
	{ 0x78, 3, "ASL",   EXT    },
	{ 0x79, 3, "ROL",   EXT    },
	{ 0x7A, 3, "DEC",   EXT    },
	{ 0x7C, 3, "INC",   EXT    },
	{ 0x7D, 3, "TST",   EXT    },
	{ 0x7E, 3, "JMP",   EXT    },
	{ 0x7F, 3, "CLR",   EXT    },

	{ 0x80, 2, "SUBA",  IMM    },
	{ 0x81, 2, "CMPA",  IMM    },
	{ 0x82, 2, "SBCA",  IMM    },
	{ 0x83, 3, "SUBD",  IMM    },
	{ 0x84, 2, "ANDA",  IMM    },
	{ 0x85, 2, "BITA",  IMM    },
	{ 0x86, 2, "LDA",   IMM    },
	{ 0x88, 2, "EORA",  IMM    },
	{ 0x89, 2, "ADCA",  IMM    },
	{ 0x8A, 2, "ORA",   IMM    },
	{ 0x8B, 2, "ADDA",  IMM    },
	{ 0x8C, 3, "CMPX",  IMM    },
	{ 0x8D, 2, "BSR",   REL     , DASMFLAG_STEP_OVER },
	{ 0x8E, 3, "LDX",   IMM    },

	{ 0x90, 2, "SUBA",  DIR    },
	{ 0x91, 2, "CMPA",  DIR    },
	{ 0x92, 2, "SBCA",  DIR    },
	{ 0x93, 2, "SUBD",  DIR    },
	{ 0x94, 2, "ANDA",  DIR    },
	{ 0x95, 2, "BITA",  DIR    },
	{ 0x96, 2, "LDA",   DIR    },
	{ 0x97, 2, "STA",   DIR    },
	{ 0x98, 2, "EORA",  DIR    },
	{ 0x99, 2, "ADCA",  DIR    },
	{ 0x9A, 2, "ORA",   DIR    },
	{ 0x9B, 2, "ADDA",  DIR    },
	{ 0x9C, 2, "CMPX",  DIR    },
	{ 0x9D, 2, "JSR",   DIR     , DASMFLAG_STEP_OVER },
	{ 0x9E, 2, "LDX",   DIR    },
	{ 0x9F, 2, "STX",   DIR    },

	{ 0xA0, 2, "SUBA",  IND    },
	{ 0xA1, 2, "CMPA",  IND    },
	{ 0xA2, 2, "SBCA",  IND    },
	{ 0xA3, 2, "SUBD",  IND    },
	{ 0xA4, 2, "ANDA",  IND    },
	{ 0xA5, 2, "BITA",  IND    },
	{ 0xA6, 2, "LDA",   IND    },
	{ 0xA7, 2, "STA",   IND    },
	{ 0xA8, 2, "EORA",  IND    },
	{ 0xA9, 2, "ADCA",  IND    },
	{ 0xAA, 2, "ORA",   IND    },
	{ 0xAB, 2, "ADDA",  IND    },
	{ 0xAC, 2, "CMPX",  IND    },
	{ 0xAD, 2, "JSR",   IND     , DASMFLAG_STEP_OVER },
	{ 0xAE, 2, "LDX",   IND    },
	{ 0xAF, 2, "STX",   IND    },

	{ 0xB0, 3, "SUBA",  EXT    },
	{ 0xB1, 3, "CMPA",  EXT    },
	{ 0xB2, 3, "SBCA",  EXT    },
	{ 0xB3, 3, "SUBD",  EXT    },
	{ 0xB4, 3, "ANDA",  EXT    },
	{ 0xB5, 3, "BITA",  EXT    },
	{ 0xB6, 3, "LDA",   EXT    },
	{ 0xB7, 3, "STA",   EXT    },
	{ 0xB8, 3, "EORA",  EXT    },
	{ 0xB9, 3, "ADCA",  EXT    },
	{ 0xBA, 3, "ORA",   EXT    },
	{ 0xBB, 3, "ADDA",  EXT    },
	{ 0xBC, 3, "CMPX",  EXT    },
	{ 0xBD, 3, "JSR",   EXT     , DASMFLAG_STEP_OVER },
	{ 0xBE, 3, "LDX",   EXT    },
	{ 0xBF, 3, "STX",   EXT    },

	{ 0xC0, 2, "SUBB",  IMM    },
	{ 0xC1, 2, "CMPB",  IMM    },
	{ 0xC2, 2, "SBCB",  IMM    },
	{ 0xC3, 3, "ADDD",  IMM    },
	{ 0xC4, 2, "ANDB",  IMM    },
	{ 0xC5, 2, "BITB",  IMM    },
	{ 0xC6, 2, "LDB",   IMM    },
	{ 0xC8, 2, "EORB",  IMM    },
	{ 0xC9, 2, "ADCB",  IMM    },
	{ 0xCA, 2, "ORB",   IMM    },
	{ 0xCB, 2, "ADDB",  IMM    },
	{ 0xCC, 3, "LDD",   IMM    },
	{ 0xCE, 3, "LDU",   IMM    },

	{ 0xD0, 2, "SUBB",  DIR    },
	{ 0xD1, 2, "CMPB",  DIR    },
	{ 0xD2, 2, "SBCB",  DIR    },
	{ 0xD3, 2, "ADDD",  DIR    },
	{ 0xD4, 2, "ANDB",  DIR    },
	{ 0xD5, 2, "BITB",  DIR    },
	{ 0xD6, 2, "LDB",   DIR    },
	{ 0xD7, 2, "STB",   DIR    },
	{ 0xD8, 2, "EORB",  DIR    },
	{ 0xD9, 2, "ADCB",  DIR    },
	{ 0xDA, 2, "ORB",   DIR    },
	{ 0xDB, 2, "ADDB",  DIR    },
	{ 0xDC, 2, "LDD",   DIR    },
	{ 0xDD, 2, "STD",   DIR    },
	{ 0xDE, 2, "LDU",   DIR    },
	{ 0xDF, 2, "STU",   DIR    },

	{ 0xE0, 2, "SUBB",  IND    },
	{ 0xE1, 2, "CMPB",  IND    },
	{ 0xE2, 2, "SBCB",  IND    },
	{ 0xE3, 2, "ADDD",  IND    },
	{ 0xE4, 2, "ANDB",  IND    },
	{ 0xE5, 2, "BITB",  IND    },
	{ 0xE6, 2, "LDB",   IND    },
	{ 0xE7, 2, "STB",   IND    },
	{ 0xE8, 2, "EORB",  IND    },
	{ 0xE9, 2, "ADCB",  IND    },
	{ 0xEA, 2, "ORB",   IND    },
	{ 0xEB, 2, "ADDB",  IND    },
	{ 0xEC, 2, "LDD",   IND    },
	{ 0xED, 2, "STD",   IND    },
	{ 0xEE, 2, "LDU",   IND    },
	{ 0xEF, 2, "STU",   IND    },

	{ 0xF0, 3, "SUBB",  EXT    },
	{ 0xF1, 3, "CMPB",  EXT    },
	{ 0xF2, 3, "SBCB",  EXT    },
	{ 0xF3, 3, "ADDD",  EXT    },
	{ 0xF4, 3, "ANDB",  EXT    },
	{ 0xF5, 3, "BITB",  EXT    },
	{ 0xF6, 3, "LDB",   EXT    },
	{ 0xF7, 3, "STB",   EXT    },
	{ 0xF8, 3, "EORB",  EXT    },
	{ 0xF9, 3, "ADCB",  EXT    },
	{ 0xFA, 3, "ORB",   EXT    },
	{ 0xFB, 3, "ADDB",  EXT    },
	{ 0xFC, 3, "LDD",   EXT    },
	{ 0xFD, 3, "STD",   EXT    },
	{ 0xFE, 3, "LDU",   EXT    },
	{ 0xFF, 3, "STU",   EXT    }
};

// Page 1 opcodes (0x10 0x..)
static const opcodeinfo m6809_pg1opcodes[] =
{
	{ 0x21, 4, "LBRN",  LREL   },
	{ 0x22, 4, "LBHI",  LREL   },
	{ 0x23, 4, "LBLS",  LREL   },
	{ 0x24, 4, "LBCC",  LREL   },
	{ 0x25, 4, "LBCS",  LREL   },
	{ 0x26, 4, "LBNE",  LREL   },
	{ 0x27, 4, "LBEQ",  LREL   },
	{ 0x28, 4, "LBVC",  LREL   },
	{ 0x29, 4, "LBVS",  LREL   },
	{ 0x2A, 4, "LBPL",  LREL   },
	{ 0x2B, 4, "LBMI",  LREL   },
	{ 0x2C, 4, "LBGE",  LREL   },
	{ 0x2D, 4, "LBLT",  LREL   },
	{ 0x2E, 4, "LBGT",  LREL   },
	{ 0x2F, 4, "LBLE",  LREL   },
	{ 0x3F, 2, "SWI2",  INH    },
	{ 0x83, 4, "CMPD",  IMM    },
	{ 0x8C, 4, "CMPY",  IMM    },
	{ 0x8E, 4, "LDY",   IMM    },
	{ 0x93, 3, "CMPD",  DIR    },
	{ 0x9C, 3, "CMPY",  DIR    },
	{ 0x9E, 3, "LDY",   DIR    },
	{ 0x9F, 3, "STY",   DIR    },
	{ 0xA3, 3, "CMPD",  IND    },
	{ 0xAC, 3, "CMPY",  IND    },
	{ 0xAE, 3, "LDY",   IND    },
	{ 0xAF, 3, "STY",   IND    },
	{ 0xB3, 4, "CMPD",  EXT    },
	{ 0xBC, 4, "CMPY",  EXT    },
	{ 0xBE, 4, "LDY",   EXT    },
	{ 0xBF, 4, "STY",   EXT    },
	{ 0xCE, 4, "LDS",   IMM    },
	{ 0xDE, 3, "LDS",   DIR    },
	{ 0xDF, 3, "STS",   DIR    },
	{ 0xEE, 3, "LDS",   IND    },
	{ 0xEF, 3, "STS",   IND    },
	{ 0xFE, 4, "LDS",   EXT    },
	{ 0xFF, 4, "STS",   EXT    }
};

// Page 2 opcodes (0x11 0x..)
static const opcodeinfo m6809_pg2opcodes[] =
{
	{ 0x3F, 2, "SWI3",  INH    },
	{ 0x83, 4, "CMPU",  IMM    },
	{ 0x8C, 4, "CMPS",  IMM    },
	{ 0x93, 3, "CMPU",  DIR    },
	{ 0x9C, 3, "CMPS",  DIR    },
	{ 0xA3, 3, "CMPU",  IND    },
	{ 0xAC, 3, "CMPS",  IND    },
	{ 0xB3, 4, "CMPU",  EXT    },
	{ 0xBC, 4, "CMPS",  EXT    }
};

static const opcodeinfo *const m6809_pgpointers[3] =
{
	m6809_pg0opcodes, m6809_pg1opcodes, m6809_pg2opcodes
};

static const int m6809_numops[3] =
{
	ARRAY_LENGTH(m6809_pg0opcodes),
	ARRAY_LENGTH(m6809_pg1opcodes),
	ARRAY_LENGTH(m6809_pg2opcodes)
};

static const char *const m6809_regs[5] = { "X", "Y", "U", "S", "PC" };

static const char *const m6809_regs_te[16] =
{
	"D", "X",  "Y",  "U",   "S",  "PC", "inv", "inv",
	"A", "B", "CC", "DP", "inv", "inv", "inv", "inv"
};

CPU_DISASSEMBLE( m6809 )
{
	UINT8 opcode, mode, pb, pbm, reg;
	const UINT8 *operandarray;
	unsigned int ea, flags;
	int numoperands, offset, indirect;
	int i, p = 0, page = 0, opcode_found = FALSE;

	do
	{
		opcode = oprom[p++];

		for (i = 0; i < m6809_numops[page]; i++)
			if (m6809_pgpointers[page][i].opcode == opcode)
				break;

		if (i < m6809_numops[page])
			opcode_found = TRUE;
		else
		{
			strcpy(buffer, "Illegal Opcode");
			return p | DASMFLAG_SUPPORTED;
		}

		if (m6809_pgpointers[page][i].mode >= PG1)
		{
			page = m6809_pgpointers[page][i].mode - PG1 + 1;
			opcode_found = FALSE;
		}
	} while (!opcode_found);

	if (page == 0)
		numoperands = m6809_pgpointers[page][i].length - 1;
	else
		numoperands = m6809_pgpointers[page][i].length - 2;

	operandarray = &opram[p];
	p += numoperands;
	pc += p;
	mode = m6809_pgpointers[page][i].mode;
	flags = m6809_pgpointers[page][i].flags;

	buffer += sprintf(buffer, "%-6s", m6809_pgpointers[page][i].name);

	switch (mode)
	{
	case INH:
		switch (opcode)
		{
		case 0x34:  // PSHS
		case 0x36:  // PSHU
			pb = operandarray[0];
			if (pb & 0x80)
				buffer += sprintf(buffer, "PC");
			if (pb & 0x40)
				buffer += sprintf(buffer, "%s%s", (pb&0x80)?",":"", (opcode==0x34)?"U":"S");
			if (pb & 0x20)
				buffer += sprintf(buffer, "%sY",  (pb&0xc0)?",":"");
			if (pb & 0x10)
				buffer += sprintf(buffer, "%sX",  (pb&0xe0)?",":"");
			if (pb & 0x08)
				buffer += sprintf(buffer, "%sDP", (pb&0xf0)?",":"");
			if (pb & 0x04)
				buffer += sprintf(buffer, "%sB",  (pb&0xf8)?",":"");
			if (pb & 0x02)
				buffer += sprintf(buffer, "%sA",  (pb&0xfc)?",":"");
			if (pb & 0x01)
				buffer += sprintf(buffer, "%sCC", (pb&0xfe)?",":"");
			break;
		case 0x35:  // PULS
		case 0x37:  // PULU
			pb = operandarray[0];
			if (pb & 0x01)
				buffer += sprintf(buffer, "CC");
			if (pb & 0x02)
				buffer += sprintf(buffer, "%sA",  (pb&0x01)?",":"");
			if (pb & 0x04)
				buffer += sprintf(buffer, "%sB",  (pb&0x03)?",":"");
			if (pb & 0x08)
				buffer += sprintf(buffer, "%sDP", (pb&0x07)?",":"");
			if (pb & 0x10)
				buffer += sprintf(buffer, "%sX",  (pb&0x0f)?",":"");
			if (pb & 0x20)
				buffer += sprintf(buffer, "%sY",  (pb&0x1f)?",":"");
			if (pb & 0x40)
				buffer += sprintf(buffer, "%s%s", (pb&0x3f)?",":"", (opcode==0x35)?"U":"S");
			if (pb & 0x80)
				buffer += sprintf(buffer, "%sPC ; (PUL? PC=RTS)", (pb&0x7f)?",":"");
			break;
		default:
			// No operands
			break;
		}
		break;

	case DIR:
		ea = operandarray[0];
		buffer += sprintf(buffer, "$%02X", ea);
		break;

	case REL:
		offset = (INT8)operandarray[0];
		buffer += sprintf(buffer, "$%04X", (pc + offset) & 0xffff);
		break;

	case LREL:
		offset = (INT16)((operandarray[0] << 8) + operandarray[1]);
		buffer += sprintf(buffer, "$%04X", (pc + offset) & 0xffff);
		break;

	case EXT:
		ea = (operandarray[0] << 8) + operandarray[1];
		buffer += sprintf(buffer, "$%04X", ea);
		break;

	case IND:
		pb = operandarray[0];
		reg = (pb >> 5) & 3;
		pbm = pb & 0x8f;
		indirect = ((pb & 0x90) == 0x90 )? TRUE : FALSE;

		// open brackets if indirect
		if (indirect && pbm != 0x80 && pbm != 0x82)
			buffer += sprintf(buffer, "[");

		switch (pbm)
		{
		case 0x80:  // ,R+
			if (indirect)
				strcpy(buffer, "Illegal Postbyte");
			else
				buffer += sprintf(buffer, ",%s+", m6809_regs[reg]);
			break;

		case 0x81:  // ,R++
			buffer += sprintf(buffer, ",%s++", m6809_regs[reg]);
			break;

		case 0x82:  // ,-R
			if (indirect)
				strcpy(buffer, "Illegal Postbyte");
			else
				buffer += sprintf(buffer, ",-%s", m6809_regs[reg]);
			break;

		case 0x83:  // ,--R
			buffer += sprintf(buffer, ",--%s", m6809_regs[reg]);
			break;

		case 0x84:  // ,R
			buffer += sprintf(buffer, ",%s", m6809_regs[reg]);
			break;

		case 0x85:  // (+/- B),R
			buffer += sprintf(buffer, "B,%s", m6809_regs[reg]);
			break;

		case 0x86:  // (+/- A),R
			buffer += sprintf(buffer, "A,%s", m6809_regs[reg]);
			break;

		case 0x87:
			strcpy(buffer, "Illegal Postbyte");
			break;

		case 0x88:  // (+/- 7 bit offset),R
			offset = (INT8)opram[p++];
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%02X,", (offset < 0) ? -offset : offset);
			buffer += sprintf(buffer, "%s", m6809_regs[reg]);
			break;

		case 0x89:  // (+/- 15 bit offset),R
			offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%04X,", (offset < 0) ? -offset : offset);
			buffer += sprintf(buffer, "%s", m6809_regs[reg]);
			break;

		case 0x8a:
			strcpy(buffer, "Illegal Postbyte");
			break;

		case 0x8b:  // (+/- D),R
			buffer += sprintf(buffer, "D,%s", m6809_regs[reg]);
			break;

		case 0x8c:  // (+/- 7 bit offset),PC
			offset = (INT8)opram[p++];
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%02X,PC", (offset < 0) ? -offset : offset);
			break;

		case 0x8d:  // (+/- 15 bit offset),PC
			offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%04X,PC", (offset < 0) ? -offset : offset);
			break;

		case 0x8e:
			strcpy(buffer, "Illegal Postbyte");
			break;

		case 0x8f:  // address
			ea = (UINT16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += sprintf(buffer, "$%04X", ea);
			break;

		default:    // (+/- 4 bit offset),R
			offset = pb & 0x1f;
			if (offset > 15)
				offset = offset - 32;
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%X,", (offset < 0) ? -offset : offset);
			buffer += sprintf(buffer, "%s", m6809_regs[reg]);
			break;
		}

		// close brackets if indirect
		if (indirect && pbm != 0x80 && pbm != 0x82)
			buffer += sprintf(buffer, "]");
		break;

	case IMM:
		if (numoperands == 2)
		{
			ea = (operandarray[0] << 8) + operandarray[1];
			buffer += sprintf(buffer, "#$%04X", ea);
		}
		else
		if (numoperands == 1)
		{
			ea = operandarray[0];
			buffer += sprintf(buffer, "#$%02X", ea);
		}
		break;

	case IMM_RR:
		pb = operandarray[0];
		buffer += sprintf(buffer, "%s,%s", m6809_regs_te[(pb >> 4) & 0xf], m6809_regs_te[pb & 0xf]);
		break;
	}

	return p | flags | DASMFLAG_SUPPORTED;
}
