// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Tim Lindner
/*****************************************************************************

    6309dasm.c - a 6309 opcode disassembler
    Version 1.0 5-AUG-2000
    Copyright Tim Lindner

    Based on:
    6809dasm.c - a 6809 opcode disassembler
    Version 1.4 1-MAR-95
    Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    Freely distributable on any medium given all copyrights are retained
    by the author and no charge greater than $7.00 is made for obtaining
    this software

    Please send all bug reports, update ideas and data files to:
    tlindner@ix.netcom.com

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "hd6309.h"

// Opcode structure
struct opcodeinfo
{
	UINT8   opcode;     // 8-bit opcode value
	UINT8   length;     // Opcode length in bytes
	char    name[6];    // Opcode name
	UINT8   mode;       // Addressing mode
	unsigned flags;     // Disassembly flags
};

enum hd6309_addressing_modes
{
	INH,                // Inherent
	DIR,                // Direct
	DIR_IM,             // Direct in memory (6309 only)
	IND,                // Indexed
	REL,                // Relative (8 bit)
	LREL,               // Long relative (16 bit)
	EXT,                // Extended
	IMM,                // Immediate
	IMM_RR,             // Register-to-register
	IMM_BW,             // Bitwise operations (6309 only)
	IMM_TFM,            // Transfer from memory (6309 only)
	PG1,                // Switch to page 1 opcodes
	PG2                 // Switch to page 2 opcodes
};

// Page 0 opcodes (single byte)
static const opcodeinfo hd6309_pg0opcodes[] =
{
	{ 0x00, 2, "NEG",   DIR    },
	{ 0x01, 3, "OIM",   DIR_IM },
	{ 0x02, 3, "AIM",   DIR_IM },
	{ 0x03, 2, "COM",   DIR    },
	{ 0x04, 2, "LSR",   DIR    },
	{ 0x05, 3, "EIM",   DIR_IM },
	{ 0x06, 2, "ROR",   DIR    },
	{ 0x07, 2, "ASR",   DIR    },
	{ 0x08, 2, "ASL",   DIR    },
	{ 0x09, 2, "ROL",   DIR    },
	{ 0x0A, 2, "DEC",   DIR    },
	{ 0x0B, 3, "TIM",   DIR_IM },
	{ 0x0C, 2, "INC",   DIR    },
	{ 0x0D, 2, "TST",   DIR    },
	{ 0x0E, 2, "JMP",   DIR    },
	{ 0x0F, 2, "CLR",   DIR    },

	{ 0x10, 1, "page1", PG1    },
	{ 0x11, 1, "page2", PG2    },
	{ 0x12, 1, "NOP",   INH    },
	{ 0x13, 1, "SYNC",  INH    },
	{ 0x14, 1, "SEXW",  INH    },
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
	{ 0x61, 3, "OIM",   IND    },
	{ 0x62, 3, "AIM",   IND    },
	{ 0x63, 2, "COM",   IND    },
	{ 0x64, 2, "LSR",   IND    },
	{ 0x65, 3, "EIM",   IND    },
	{ 0x66, 2, "ROR",   IND    },
	{ 0x67, 2, "ASR",   IND    },
	{ 0x68, 2, "ASL",   IND    },
	{ 0x69, 2, "ROL",   IND    },
	{ 0x6A, 2, "DEC",   IND    },
	{ 0x6B, 3, "TIM",   IND    },
	{ 0x6C, 2, "INC",   IND    },
	{ 0x6D, 2, "TST",   IND    },
	{ 0x6E, 2, "JMP",   IND    },
	{ 0x6F, 2, "CLR",   IND    },

	{ 0x70, 3, "NEG",   EXT    },
	{ 0x71, 4, "OIM",   EXT    },
	{ 0x72, 4, "AIM",   EXT    },
	{ 0x73, 3, "COM",   EXT    },
	{ 0x74, 3, "LSR",   EXT    },
	{ 0x75, 4, "EIM",   EXT    },
	{ 0x76, 3, "ROR",   EXT    },
	{ 0x77, 3, "ASR",   EXT    },
	{ 0x78, 3, "ASL",   EXT    },
	{ 0x79, 3, "ROL",   EXT    },
	{ 0x7A, 3, "DEC",   EXT    },
	{ 0x7B, 4, "TIM",   EXT    },
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
	{ 0xCD, 5, "LDQ",   IMM    },
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
static const opcodeinfo hd6309_pg1opcodes[] =
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

	{ 0x30, 3, "ADDR",  IMM_RR },
	{ 0x31, 3, "ADCR",  IMM_RR },
	{ 0x32, 3, "SUBR",  IMM_RR },
	{ 0x33, 3, "SBCR",  IMM_RR },
	{ 0x34, 3, "ANDR",  IMM_RR },
	{ 0x35, 3, "ORR",   IMM_RR },
	{ 0x36, 3, "EORR",  IMM_RR },
	{ 0x37, 3, "CMPR",  IMM_RR },

	{ 0x38, 2, "PSHSW", INH    },
	{ 0x39, 2, "PULSW", INH    },
	{ 0x3A, 2, "PSHUW", INH    },
	{ 0x3B, 2, "PULUW", INH    },

	{ 0x3F, 2, "SWI2",  INH    },

	{ 0x40, 2, "NEGD",  INH    },
	{ 0x43, 2, "COMD",  INH    },
	{ 0x44, 2, "LSRD",  INH    },
	{ 0x46, 2, "RORD",  INH    },
	{ 0x47, 2, "ASRD",  INH    },
	{ 0x48, 2, "ASLD",  INH    },
	{ 0x49, 2, "ROLD",  INH    },

	{ 0x4A, 2, "DECD",  INH    },
	{ 0x4C, 2, "INCD",  INH    },
	{ 0x4D, 2, "TSTD",  INH    },
	{ 0x4f, 2, "CLRD",  INH    },

	{ 0x53, 2, "COMW",  INH    },
	{ 0x54, 2, "LSRW",  INH    },
	{ 0x56, 2, "RORW",  INH    },
	{ 0x59, 2, "ROLW",  INH    },
	{ 0x5A, 2, "DECW",  INH    },
	{ 0x5C, 2, "INCW",  INH    },
	{ 0x5D, 2, "TSTW",  INH    },
	{ 0x5F, 2, "CLRW",  INH    },
	{ 0x80, 4, "SUBW",  IMM    },
	{ 0x81, 4, "CMPW",  IMM    },
	{ 0x82, 4, "SBCD",  IMM    },

	{ 0x83, 4, "CMPD",  IMM    },

	{ 0x84, 4, "ANDD",  IMM    },
	{ 0x85, 4, "BITD",  IMM    },
	{ 0x86, 4, "LDW",   IMM    },
	{ 0x88, 4, "EORD",  IMM    },
	{ 0x89, 4, "ADCD",  IMM    },
	{ 0x8A, 4, "ORD",   IMM    },
	{ 0x8B, 4, "ADDW",  IMM    },

	{ 0x8C, 4, "CMPY",  IMM    },
	{ 0x8E, 4, "LDY",   IMM    },

	{ 0x90, 3, "SUBW",  DIR    },
	{ 0x91, 3, "CMPW",  DIR    },
	{ 0x92, 3, "SBCD",  DIR    },

	{ 0x93, 3, "CMPD",  DIR    },

	{ 0x94, 3, "ANDD",  DIR    },
	{ 0x95, 3, "BITD",  DIR    },
	{ 0x96, 3, "LDW",   DIR    },
	{ 0x97, 3, "STW",   DIR    },
	{ 0x98, 3, "EORD",  DIR    },
	{ 0x99, 3, "ADCD",  DIR    },
	{ 0x9A, 3, "ORD",   DIR    },
	{ 0x9B, 3, "ADDW",  DIR    },

	{ 0x9C, 3, "CMPY",  DIR    },
	{ 0x9E, 3, "LDY",   DIR    },
	{ 0x9F, 3, "STY",   DIR    },

	{ 0xA0, 3, "SUBW",  IND    },
	{ 0xA1, 3, "CMPW",  IND    },
	{ 0xA2, 3, "SBCD",  IND    },

	{ 0xA3, 3, "CMPD",  IND    },

	{ 0xA4, 3, "ANDD",  IND    },
	{ 0xA5, 3, "BITD",  IND    },

	{ 0xA6, 3, "LDW",   IND    },
	{ 0xA7, 3, "STW",   IND    },
	{ 0xA8, 3, "EORD",  IND    },
	{ 0xA9, 3, "ADCD",  IND    },
	{ 0xAA, 3, "ORD",   IND    },
	{ 0xAB, 3, "ADDW",  IND    },

	{ 0xAC, 3, "CMPY",  IND    },
	{ 0xAE, 3, "LDY",   IND    },
	{ 0xAF, 3, "STY",   IND    },

	{ 0xB0, 4, "SUBW",  EXT    },
	{ 0xB1, 4, "CMPW",  EXT    },
	{ 0xB2, 4, "SBCD",  EXT    },

	{ 0xB3, 4, "CMPD",  EXT    },

	{ 0xB4, 4, "ANDD",  EXT    },
	{ 0xB5, 4, "BITD",  EXT    },
	{ 0xB6, 4, "LDW",   EXT    },
	{ 0xB7, 4, "STW",   EXT    },
	{ 0xB8, 4, "EORD",  EXT    },
	{ 0xB9, 4, "ADCD",  EXT    },
	{ 0xBA, 4, "ORD",   EXT    },
	{ 0xBB, 4, "ADDW",  EXT    },

	{ 0xBC, 4, "CMPY",  EXT    },
	{ 0xBE, 4, "LDY",   EXT    },
	{ 0xBF, 4, "STY",   EXT    },
	{ 0xCE, 4, "LDS",   IMM    },

	{ 0xDC, 3, "LDQ",   DIR    },
	{ 0xDD, 3, "STQ",   DIR    },

	{ 0xDE, 3, "LDS",   DIR    },
	{ 0xDF, 3, "STS",   DIR    },

	{ 0xEC, 3, "LDQ",   IND    },
	{ 0xED, 3, "STQ",   IND    },
	{ 0xEE, 3, "LDS",   IND    },

	{ 0xEE, 3, "LDS",   IND    },
	{ 0xEF, 3, "STS",   IND    },

	{ 0xFC, 4, "LDQ",   EXT    },
	{ 0xFD, 4, "STQ",   EXT    },

	{ 0xFE, 4, "LDS",   EXT    },
	{ 0xFF, 4, "STS",   EXT    }
};

// Page 2 opcodes (0x11 0x..)
static const opcodeinfo hd6309_pg2opcodes[] =
{
	{ 0x30, 4, "BAND",  IMM_BW },
	{ 0x31, 4, "BIAND", IMM_BW },
	{ 0x32, 4, "BOR",   IMM_BW },
	{ 0x33, 4, "BIOR",  IMM_BW },
	{ 0x34, 4, "BEOR",  IMM_BW },
	{ 0x35, 4, "BIEOR", IMM_BW },

	{ 0x36, 4, "LDBT",  IMM_BW },
	{ 0x37, 4, "STBT",  IMM_BW },

	{ 0x38, 3, "TFM",   IMM_TFM },
	{ 0x39, 3, "TFM",   IMM_TFM },
	{ 0x3A, 3, "TFM",   IMM_TFM },
	{ 0x3B, 3, "TFM",   IMM_TFM },

	{ 0x3C, 3, "BITMD", IMM     },
	{ 0x3D, 3, "LDMD",  IMM     },

	{ 0x3F, 2, "SWI3",  INH     },

	{ 0x43, 2, "COME",  INH     },
	{ 0x4A, 2, "DECE",  INH     },
	{ 0x4C, 2, "INCE",  INH     },
	{ 0x4D, 2, "TSTE",  INH     },
	{ 0x4F, 2, "CLRE",  INH     },
	{ 0x53, 2, "COMF",  INH     },
	{ 0x5A, 2, "DECF",  INH     },
	{ 0x5C, 2, "INCF",  INH     },
	{ 0x5D, 2, "TSTF",  INH     },
	{ 0x5F, 2, "CLRF",  INH     },

	{ 0x80, 3, "SUBE",  IMM     },
	{ 0x81, 3, "CMPE",  IMM     },

	{ 0x83, 4, "CMPU",  IMM     },

	{ 0x86, 3, "LDE",   IMM     },
	{ 0x8b, 3, "ADDE",  IMM     },

	{ 0x8C, 4, "CMPS",  IMM     },

	{ 0x8D, 3, "DIVD",  IMM     },
	{ 0x8E, 4, "DIVQ",  IMM     },
	{ 0x8F, 4, "MULD",  IMM     },
	{ 0x90, 3, "SUBE",  DIR     },
	{ 0x91, 3, "CMPE",  DIR     },

	{ 0x93, 3, "CMPU",  DIR     },

	{ 0x96, 3, "LDE",   DIR     },
	{ 0x97, 3, "STE",   DIR     },
	{ 0x9B, 3, "ADDE",  DIR     },

	{ 0x9C, 3, "CMPS",  DIR     },

	{ 0x9D, 3, "DIVD",  DIR     },
	{ 0x9E, 3, "DIVQ",  DIR     },
	{ 0x9F, 3, "MULD",  DIR     },

	{ 0xA0, 3, "SUBE",  IND     },
	{ 0xA1, 3, "CMPE",  IND     },

	{ 0xA3, 3, "CMPU",  IND     },

	{ 0xA6, 3, "LDE",   IND     },
	{ 0xA7, 3, "STE",   IND     },
	{ 0xAB, 3, "ADDE",  IND     },

	{ 0xAC, 3, "CMPS",  IND     },

	{ 0xAD, 3, "DIVD",  IND     },
	{ 0xAE, 3, "DIVQ",  IND     },
	{ 0xAF, 3, "MULD",  IND     },
	{ 0xB0, 4, "SUBE",  EXT     },
	{ 0xB1, 4, "CMPE",  EXT     },

	{ 0xB3, 4, "CMPU",  EXT     },

	{ 0xB6, 4, "LDE",   EXT     },
	{ 0xB7, 4, "STE",   EXT     },

	{ 0xBB, 4, "ADDE",  EXT     },
	{ 0xBC, 4, "CMPS",  EXT     },

	{ 0xBD, 4, "DIVD",  EXT     },
	{ 0xBE, 4, "DIVQ",  EXT     },
	{ 0xBF, 4, "MULD",  EXT     },

	{ 0xC0, 3, "SUBF",  IMM     },
	{ 0xC1, 3, "CMPF",  IMM     },
	{ 0xC6, 3, "LDF",   IMM     },
	{ 0xCB, 3, "ADDF",  IMM     },

	{ 0xD0, 3, "SUBF",  DIR     },
	{ 0xD1, 3, "CMPF",  DIR     },
	{ 0xD6, 3, "LDF",   DIR     },
	{ 0xD7, 3, "STF",   DIR     },
	{ 0xDB, 3, "ADDF",  DIR     },

	{ 0xE0, 3, "SUBF",  IND     },
	{ 0xE1, 3, "CMPF",  IND     },
	{ 0xE6, 3, "LDF",   IND     },
	{ 0xE7, 3, "STF",   IND     },
	{ 0xEB, 3, "ADDF",  IND     },

	{ 0xF0, 4, "SUBF",  EXT     },
	{ 0xF1, 4, "CMPF",  EXT     },
	{ 0xF6, 4, "LDF",   EXT     },
	{ 0xF7, 4, "STF",   EXT     },
	{ 0xFB, 4, "ADDF",  EXT     }
};

static const opcodeinfo *const hd6309_pgpointers[3] =
{
	hd6309_pg0opcodes, hd6309_pg1opcodes, hd6309_pg2opcodes
};

static const int hd6309_numops[3] =
{
	ARRAY_LENGTH(hd6309_pg0opcodes),
	ARRAY_LENGTH(hd6309_pg1opcodes),
	ARRAY_LENGTH(hd6309_pg2opcodes)
};

static const char *const hd6309_regs[5] = { "X", "Y", "U", "S", "PC" };

static const char *const hd6309_btwregs[5] = { "CC", "A", "B", "inv" };

static const char *const hd6309_teregs[16] =
{
	"D", "X",  "Y",  "U", "S", "PC", "W", "V",
	"A", "B", "CC", "DP", "0",  "0", "E", "F"
};

static const char *const hd6309_tfmregs[16] = {
		"D",   "X",   "Y",   "U",   "S", "inv", "inv", "inv",
	"inv", "inv", "inv", "inv", "inv", "inv", "inv", "inv"
};

static const char *const tfm_s[] = { "%s+,%s+", "%s-,%s-", "%s+,%s", "%s,%s+" };

CPU_DISASSEMBLE( hd6309 )
{
	UINT8 opcode, mode, pb, pbm, reg;
	const UINT8 *operandarray;
	unsigned int ea, flags;
	int numoperands, offset, indirect;

	int i, p = 0, page = 0, opcode_found = FALSE;

	do
	{
		opcode = oprom[p++];
		for (i = 0; i < hd6309_numops[page]; i++)
			if (hd6309_pgpointers[page][i].opcode == opcode)
				break;

		if (i < hd6309_numops[page])
			opcode_found = TRUE;
		else
		{
			strcpy(buffer, "Illegal Opcode");
			return p | DASMFLAG_SUPPORTED;
		}

		if (hd6309_pgpointers[page][i].mode >= PG1)
		{
			page = hd6309_pgpointers[page][i].mode - PG1 + 1;
			opcode_found = FALSE;
		}
	} while (!opcode_found);

	if (page == 0)
		numoperands = hd6309_pgpointers[page][i].length - 1;
	else
		numoperands = hd6309_pgpointers[page][i].length - 2;

	operandarray = &opram[p];
	p += numoperands;
	pc += p;
	mode = hd6309_pgpointers[page][i].mode;
	flags = hd6309_pgpointers[page][i].flags;

	buffer += sprintf(buffer, "%-6s", hd6309_pgpointers[page][i].name);

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

	case DIR_IM:
		buffer += sprintf(buffer, "#$%02X,", operandarray[0]);
		buffer += sprintf(buffer, "$%02X", operandarray[1]);
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
		if (numoperands == 3)
		{
			pb = operandarray[0];
			ea = (operandarray[1] << 8) + operandarray[2];
			buffer += sprintf(buffer, "#$%02X,$%04X", pb, ea);
		}
		else
		if (numoperands == 2)
		{
			ea = (operandarray[0] << 8) + operandarray[1];
			buffer += sprintf(buffer, "$%04X", ea);
		}
		break;

	case IND:
		if (numoperands == 2)
		{
			buffer += sprintf(buffer, "#$%02X,", operandarray[0]);
			pb = operandarray[1];
		}
		else
		{
			pb = operandarray[0];
		}

		reg = (pb >> 5) & 3;
		pbm = pb & 0x8f;
		indirect = ((pb & 0x90) == 0x90 )? TRUE : FALSE;

		// open brackets if indirect
		if (indirect && pbm != 0x82)
			buffer += sprintf(buffer, "[");

		switch (pbm)
		{
		case 0x80:  // ,R+ or operations relative to W
			if (indirect)
			{
				switch (reg)
				{
				case 0x00:
					buffer += sprintf(buffer, ",W");
					break;
				case 0x01:
					offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
					p += 2;
					buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
					buffer += sprintf(buffer, "$%04X,W", (offset < 0) ? -offset : offset);
					break;
				case 0x02:
					buffer += sprintf(buffer, ",W++");
					break;
				case 0x03:
					buffer += sprintf(buffer, ",--W");
					break;
				}
			}
			else
				buffer += sprintf(buffer, ",%s+", hd6309_regs[reg]);
			break;

		case 0x81:  // ,R++
			buffer += sprintf(buffer, ",%s++", hd6309_regs[reg]);
			break;

		case 0x82:  // ,-R
			if (indirect)
				strcpy(buffer, "Illegal Postbyte");
			else
				buffer += sprintf(buffer, ",-%s", hd6309_regs[reg]);
			break;

		case 0x83:  // ,--R
			buffer += sprintf(buffer, ",--%s", hd6309_regs[reg]);
			break;

		case 0x84:  // ,R
			buffer += sprintf(buffer, ",%s", hd6309_regs[reg]);
			break;

		case 0x85:  // (+/- B),R
			buffer += sprintf(buffer, "B,%s", hd6309_regs[reg]);
			break;

		case 0x86:  // (+/- A),R
			buffer += sprintf(buffer, "A,%s", hd6309_regs[reg]);
			break;

		case 0x87:  // (+/- E),R
			buffer += sprintf(buffer, "E,%s", hd6309_regs[reg]);
			break;

		case 0x88:  // (+/- 7 bit offset),R
			offset = (INT8)opram[p++];
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%02X,", (offset < 0) ? -offset : offset);
			buffer += sprintf(buffer, "%s", hd6309_regs[reg]);
			break;

		case 0x89:  // (+/- 15 bit offset),R
			offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%04X,", (offset < 0) ? -offset : offset);
			buffer += sprintf(buffer, "%s", hd6309_regs[reg]);
			break;

		case 0x8a:  // (+/- F),R
			buffer += sprintf(buffer, "F,%s", hd6309_regs[reg]);
			break;

		case 0x8b:  // (+/- D),R
			buffer += sprintf(buffer, "D,%s", hd6309_regs[reg]);
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

		case 0x8e:  // (+/- W),R
			buffer += sprintf(buffer, "W,%s", hd6309_regs[reg]);
			break;

		case 0x8f:  // address or operations relative to W
			if (indirect)
			{
				ea = (UINT16)((opram[p+0] << 8) + opram[p+1]);
				p += 2;
				buffer += sprintf(buffer, "$%04X", ea);
				break;
			}
			else
			{
				switch (reg)
				{
				case 0x00:
					buffer += sprintf(buffer, ",W");
					break;
				case 0x01:
					offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
					p += 2;
					buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
					buffer += sprintf(buffer, "$%04X,W", (offset < 0) ? -offset : offset);
					break;
				case 0x02:
					buffer += sprintf(buffer, ",W++");
					break;
				case 0x03:
					buffer += sprintf(buffer, ",--W");
					break;
				}
			}
			break;

		default:    // (+/- 4 bit offset),R
			offset = pb & 0x1f;
			if (offset > 15)
				offset = offset - 32;
			buffer += sprintf(buffer, "%s", (offset < 0) ? "-" : "");
			buffer += sprintf(buffer, "$%X,", (offset < 0) ? -offset : offset);
			buffer += sprintf(buffer, "%s", hd6309_regs[reg]);
			break;
		}

		// close brackets if indirect
		if (indirect && pbm != 0x82)
			buffer += sprintf(buffer, "]");
		break;

	case IMM:
		if (numoperands == 4)
		{
			ea = (operandarray[0] << 24) + (operandarray[1] << 16) + (operandarray[2] << 8) + operandarray[3];
			buffer += sprintf(buffer, "#$%08X", ea);
		}
		else
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
		buffer += sprintf(buffer, "%s,%s", hd6309_teregs[(pb >> 4) & 0xf], hd6309_teregs[pb & 0xf]);
		break;

	case IMM_BW:
		pb = operandarray[0];
		buffer += sprintf(buffer, "%s,", hd6309_btwregs[((pb & 0xc0) >> 6)]);
		buffer += sprintf(buffer, "%d,", (pb & 0x38) >> 3);
		buffer += sprintf(buffer, "%d,", (pb & 0x07));
		buffer += sprintf(buffer, "$%02X", operandarray[1]);
		break;

	case IMM_TFM:
		pb = operandarray[0];
		buffer += sprintf(buffer, tfm_s[opcode & 0x07], hd6309_tfmregs[(pb >> 4) & 0xf], hd6309_tfmregs[pb & 0xf]);
		break;
	}

	return p | flags | DASMFLAG_SUPPORTED;
}
