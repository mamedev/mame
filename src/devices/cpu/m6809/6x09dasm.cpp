// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Sean Riddle,Tim Lindner
/*****************************************************************************

    6x09dasm.cpp - a 6809/6309 opcode disassembler
	
	Based on:
		6809dasm.c - a 6809 opcode disassembler
		Version 1.0 5-AUG-2000
	    Copyright Tim Lindner

	and

	    6809dasm.c - a 6809 opcode disassembler
	    Version 1.4 1-MAR-95
	    Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    Freely distributable on any medium given all copyrights are retained
    by the author and no charge greater than $7.00 is made for obtaining
    this software

    Please send all bug reports, update ideas and data files to:
    sriddle@ionet.net and tlindner@ix.netcom.com

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "hd6309.h"

enum m6x09_addressing_mode
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

// General, or 6309 only?
enum m6x09_instruction_level
{
	M6x09_GENERAL,
	HD6309_EXCLUSIVE
};

// Opcode structure
struct opcodeinfo
{
	uint8_t					opcode;			// 8-bit opcode value
	uint8_t					length;			// Opcode length in bytes
	char					name[6];		// Opcode name
	m6x09_addressing_mode	mode : 4;       // Addressing mode
	m6x09_instruction_level	level : 1;		// General, or 6309 only?
	unsigned				flags;			// Disassembly flags
};

// Page 0 opcodes (single byte)
static const opcodeinfo m6x09_pg0opcodes[] =
{
	{ 0x00, 2, "NEG",   DIR,	M6x09_GENERAL },
	{ 0x01, 3, "OIM",   DIR_IM,	HD6309_EXCLUSIVE },
	{ 0x02, 3, "AIM",   DIR_IM,	HD6309_EXCLUSIVE },
	{ 0x03, 2, "COM",   DIR,	M6x09_GENERAL },
	{ 0x04, 2, "LSR",   DIR,	M6x09_GENERAL },
	{ 0x05, 3, "EIM",   DIR_IM,	HD6309_EXCLUSIVE },
	{ 0x06, 2, "ROR",   DIR,	M6x09_GENERAL },
	{ 0x07, 2, "ASR",   DIR,	M6x09_GENERAL },
	{ 0x08, 2, "ASL",   DIR,	M6x09_GENERAL },
	{ 0x09, 2, "ROL",   DIR,	M6x09_GENERAL },
	{ 0x0A, 2, "DEC",   DIR,	M6x09_GENERAL },
	{ 0x0B, 3, "TIM",   DIR_IM,	HD6309_EXCLUSIVE },
	{ 0x0C, 2, "INC",   DIR,	M6x09_GENERAL },
	{ 0x0D, 2, "TST",   DIR,	M6x09_GENERAL },
	{ 0x0E, 2, "JMP",   DIR,	M6x09_GENERAL },
	{ 0x0F, 2, "CLR",   DIR,	M6x09_GENERAL },

	{ 0x10, 1, "page1", PG1,	M6x09_GENERAL },
	{ 0x11, 1, "page2", PG2,	M6x09_GENERAL },
	{ 0x12, 1, "NOP",   INH,	M6x09_GENERAL },
	{ 0x13, 1, "SYNC",  INH,	M6x09_GENERAL },
	{ 0x14, 1, "SEXW",  INH,	HD6309_EXCLUSIVE },
	{ 0x16, 3, "LBRA",  LREL,	M6x09_GENERAL },
	{ 0x17, 3, "LBSR",  LREL,	M6x09_GENERAL, DASMFLAG_STEP_OVER },
	{ 0x19, 1, "DAA",   INH,	M6x09_GENERAL },
	{ 0x1A, 2, "ORCC",  IMM,	M6x09_GENERAL },
	{ 0x1C, 2, "ANDCC", IMM,	M6x09_GENERAL },
	{ 0x1D, 1, "SEX",   INH,	M6x09_GENERAL },
	{ 0x1E, 2, "EXG",   IMM_RR,	M6x09_GENERAL },
	{ 0x1F, 2, "TFR",   IMM_RR,	M6x09_GENERAL },

	{ 0x20, 2, "BRA",   REL,	M6x09_GENERAL },
	{ 0x21, 2, "BRN",   REL,	M6x09_GENERAL },
	{ 0x22, 2, "BHI",   REL,	M6x09_GENERAL },
	{ 0x23, 2, "BLS",   REL,	M6x09_GENERAL },
	{ 0x24, 2, "BCC",   REL,	M6x09_GENERAL },
	{ 0x25, 2, "BCS",   REL,	M6x09_GENERAL },
	{ 0x26, 2, "BNE",   REL,	M6x09_GENERAL },
	{ 0x27, 2, "BEQ",   REL,	M6x09_GENERAL },
	{ 0x28, 2, "BVC",   REL,	M6x09_GENERAL },
	{ 0x29, 2, "BVS",   REL,	M6x09_GENERAL },
	{ 0x2A, 2, "BPL",   REL,	M6x09_GENERAL },
	{ 0x2B, 2, "BMI",   REL,	M6x09_GENERAL },
	{ 0x2C, 2, "BGE",   REL,	M6x09_GENERAL },
	{ 0x2D, 2, "BLT",   REL,	M6x09_GENERAL },
	{ 0x2E, 2, "BGT",   REL,	M6x09_GENERAL },
	{ 0x2F, 2, "BLE",   REL,	M6x09_GENERAL },

	{ 0x30, 2, "LEAX",  IND,	M6x09_GENERAL },
	{ 0x31, 2, "LEAY",  IND,	M6x09_GENERAL },
	{ 0x32, 2, "LEAS",  IND,	M6x09_GENERAL },
	{ 0x33, 2, "LEAU",  IND,	M6x09_GENERAL },
	{ 0x34, 2, "PSHS",  INH,	M6x09_GENERAL },
	{ 0x35, 2, "PULS",  INH,	M6x09_GENERAL },
	{ 0x36, 2, "PSHU",  INH,	M6x09_GENERAL },
	{ 0x37, 2, "PULU",  INH,	M6x09_GENERAL },
	{ 0x39, 1, "RTS",   INH ,	M6x09_GENERAL },
	{ 0x3A, 1, "ABX",   INH,	M6x09_GENERAL },
	{ 0x3B, 1, "RTI",   INH,	M6x09_GENERAL },
	{ 0x3C, 2, "CWAI",  IMM,	M6x09_GENERAL },
	{ 0x3D, 1, "MUL",   INH,	M6x09_GENERAL },
	{ 0x3F, 1, "SWI",   INH,	M6x09_GENERAL },

	{ 0x40, 1, "NEGA",  INH,	M6x09_GENERAL },
	{ 0x43, 1, "COMA",  INH,	M6x09_GENERAL },
	{ 0x44, 1, "LSRA",  INH,	M6x09_GENERAL },
	{ 0x46, 1, "RORA",  INH,	M6x09_GENERAL },
	{ 0x47, 1, "ASRA",  INH,	M6x09_GENERAL },
	{ 0x48, 1, "ASLA",  INH,	M6x09_GENERAL },
	{ 0x49, 1, "ROLA",  INH,	M6x09_GENERAL },
	{ 0x4A, 1, "DECA",  INH,	M6x09_GENERAL },
	{ 0x4C, 1, "INCA",  INH,	M6x09_GENERAL },
	{ 0x4D, 1, "TSTA",  INH,	M6x09_GENERAL },
	{ 0x4F, 1, "CLRA",  INH,	M6x09_GENERAL },

	{ 0x50, 1, "NEGB",  INH,	M6x09_GENERAL },
	{ 0x53, 1, "COMB",  INH,	M6x09_GENERAL },
	{ 0x54, 1, "LSRB",  INH,	M6x09_GENERAL },
	{ 0x56, 1, "RORB",  INH,	M6x09_GENERAL },
	{ 0x57, 1, "ASRB",  INH,	M6x09_GENERAL },
	{ 0x58, 1, "ASLB",  INH,	M6x09_GENERAL },
	{ 0x59, 1, "ROLB",  INH,	M6x09_GENERAL },
	{ 0x5A, 1, "DECB",  INH,	M6x09_GENERAL },
	{ 0x5C, 1, "INCB",  INH,	M6x09_GENERAL },
	{ 0x5D, 1, "TSTB",  INH,	M6x09_GENERAL },
	{ 0x5F, 1, "CLRB",  INH,	M6x09_GENERAL },

	{ 0x60, 2, "NEG",   IND,	M6x09_GENERAL },
	{ 0x61, 3, "OIM",   IND,	HD6309_EXCLUSIVE },
	{ 0x62, 3, "AIM",   IND,	HD6309_EXCLUSIVE },
	{ 0x63, 2, "COM",   IND,	M6x09_GENERAL },
	{ 0x64, 2, "LSR",   IND,	M6x09_GENERAL },
	{ 0x65, 3, "EIM",   IND,	HD6309_EXCLUSIVE },
	{ 0x66, 2, "ROR",   IND,	M6x09_GENERAL },
	{ 0x67, 2, "ASR",   IND,	M6x09_GENERAL },
	{ 0x68, 2, "ASL",   IND,	M6x09_GENERAL },
	{ 0x69, 2, "ROL",   IND,	M6x09_GENERAL },
	{ 0x6A, 2, "DEC",   IND,	M6x09_GENERAL },
	{ 0x6B, 3, "TIM",   IND,	HD6309_EXCLUSIVE },
	{ 0x6C, 2, "INC",   IND,	M6x09_GENERAL },
	{ 0x6D, 2, "TST",   IND,	M6x09_GENERAL },
	{ 0x6E, 2, "JMP",   IND,	M6x09_GENERAL },
	{ 0x6F, 2, "CLR",   IND,	M6x09_GENERAL },

	{ 0x70, 3, "NEG",   EXT,	M6x09_GENERAL },
	{ 0x71, 4, "OIM",   EXT,	HD6309_EXCLUSIVE },
	{ 0x72, 4, "AIM",   EXT,	HD6309_EXCLUSIVE },
	{ 0x73, 3, "COM",   EXT,	M6x09_GENERAL },
	{ 0x74, 3, "LSR",   EXT,	M6x09_GENERAL },
	{ 0x75, 4, "EIM",   EXT,	HD6309_EXCLUSIVE },
	{ 0x76, 3, "ROR",   EXT,	M6x09_GENERAL },
	{ 0x77, 3, "ASR",   EXT,	M6x09_GENERAL },
	{ 0x78, 3, "ASL",   EXT,	M6x09_GENERAL },
	{ 0x79, 3, "ROL",   EXT,	M6x09_GENERAL },
	{ 0x7A, 3, "DEC",   EXT,	M6x09_GENERAL },
	{ 0x7B, 4, "TIM",   EXT,	HD6309_EXCLUSIVE },
	{ 0x7C, 3, "INC",   EXT,	M6x09_GENERAL },
	{ 0x7D, 3, "TST",   EXT,	M6x09_GENERAL },
	{ 0x7E, 3, "JMP",   EXT,	M6x09_GENERAL },
	{ 0x7F, 3, "CLR",   EXT,	M6x09_GENERAL },

	{ 0x80, 2, "SUBA",  IMM,	M6x09_GENERAL },
	{ 0x81, 2, "CMPA",  IMM,	M6x09_GENERAL },
	{ 0x82, 2, "SBCA",  IMM,	M6x09_GENERAL },
	{ 0x83, 3, "SUBD",  IMM,	M6x09_GENERAL },
	{ 0x84, 2, "ANDA",  IMM,	M6x09_GENERAL },
	{ 0x85, 2, "BITA",  IMM,	M6x09_GENERAL },
	{ 0x86, 2, "LDA",   IMM,	M6x09_GENERAL },
	{ 0x88, 2, "EORA",  IMM,	M6x09_GENERAL },
	{ 0x89, 2, "ADCA",  IMM,	M6x09_GENERAL },
	{ 0x8A, 2, "ORA",   IMM,	M6x09_GENERAL },
	{ 0x8B, 2, "ADDA",  IMM,	M6x09_GENERAL },
	{ 0x8C, 3, "CMPX",  IMM,	M6x09_GENERAL },
	{ 0x8D, 2, "BSR",   REL,	M6x09_GENERAL     , DASMFLAG_STEP_OVER },
	{ 0x8E, 3, "LDX",   IMM,	M6x09_GENERAL },

	{ 0x90, 2, "SUBA",  DIR,	M6x09_GENERAL },
	{ 0x91, 2, "CMPA",  DIR,	M6x09_GENERAL },
	{ 0x92, 2, "SBCA",  DIR,	M6x09_GENERAL },
	{ 0x93, 2, "SUBD",  DIR,	M6x09_GENERAL },
	{ 0x94, 2, "ANDA",  DIR,	M6x09_GENERAL },
	{ 0x95, 2, "BITA",  DIR,	M6x09_GENERAL },
	{ 0x96, 2, "LDA",   DIR,	M6x09_GENERAL },
	{ 0x97, 2, "STA",   DIR,	M6x09_GENERAL },
	{ 0x98, 2, "EORA",  DIR,	M6x09_GENERAL },
	{ 0x99, 2, "ADCA",  DIR,	M6x09_GENERAL },
	{ 0x9A, 2, "ORA",   DIR,	M6x09_GENERAL },
	{ 0x9B, 2, "ADDA",  DIR,	M6x09_GENERAL },
	{ 0x9C, 2, "CMPX",  DIR,	M6x09_GENERAL },
	{ 0x9D, 2, "JSR",   DIR,	M6x09_GENERAL , DASMFLAG_STEP_OVER },
	{ 0x9E, 2, "LDX",   DIR,	M6x09_GENERAL },
	{ 0x9F, 2, "STX",   DIR,	M6x09_GENERAL },

	{ 0xA0, 2, "SUBA",  IND,	M6x09_GENERAL },
	{ 0xA1, 2, "CMPA",  IND,	M6x09_GENERAL },
	{ 0xA2, 2, "SBCA",  IND,	M6x09_GENERAL },
	{ 0xA3, 2, "SUBD",  IND,	M6x09_GENERAL },
	{ 0xA4, 2, "ANDA",  IND,	M6x09_GENERAL },
	{ 0xA5, 2, "BITA",  IND,	M6x09_GENERAL },
	{ 0xA6, 2, "LDA",   IND,	M6x09_GENERAL },
	{ 0xA7, 2, "STA",   IND,	M6x09_GENERAL },
	{ 0xA8, 2, "EORA",  IND,	M6x09_GENERAL },
	{ 0xA9, 2, "ADCA",  IND,	M6x09_GENERAL },
	{ 0xAA, 2, "ORA",   IND,	M6x09_GENERAL },
	{ 0xAB, 2, "ADDA",  IND,	M6x09_GENERAL },
	{ 0xAC, 2, "CMPX",  IND,	M6x09_GENERAL },
	{ 0xAD, 2, "JSR",   IND,	M6x09_GENERAL, DASMFLAG_STEP_OVER },
	{ 0xAE, 2, "LDX",   IND,	M6x09_GENERAL },
	{ 0xAF, 2, "STX",   IND,	M6x09_GENERAL },

	{ 0xB0, 3, "SUBA",  EXT,	M6x09_GENERAL },
	{ 0xB1, 3, "CMPA",  EXT,	M6x09_GENERAL },
	{ 0xB2, 3, "SBCA",  EXT,	M6x09_GENERAL },
	{ 0xB3, 3, "SUBD",  EXT,	M6x09_GENERAL },
	{ 0xB4, 3, "ANDA",  EXT,	M6x09_GENERAL },
	{ 0xB5, 3, "BITA",  EXT,	M6x09_GENERAL },
	{ 0xB6, 3, "LDA",   EXT,	M6x09_GENERAL },
	{ 0xB7, 3, "STA",   EXT,	M6x09_GENERAL },
	{ 0xB8, 3, "EORA",  EXT,	M6x09_GENERAL },
	{ 0xB9, 3, "ADCA",  EXT,	M6x09_GENERAL },
	{ 0xBA, 3, "ORA",   EXT,	M6x09_GENERAL },
	{ 0xBB, 3, "ADDA",  EXT,	M6x09_GENERAL },
	{ 0xBC, 3, "CMPX",  EXT,	M6x09_GENERAL },
	{ 0xBD, 3, "JSR",   EXT,	M6x09_GENERAL, DASMFLAG_STEP_OVER },
	{ 0xBE, 3, "LDX",   EXT,	M6x09_GENERAL },
	{ 0xBF, 3, "STX",   EXT,	M6x09_GENERAL },

	{ 0xC0, 2, "SUBB",  IMM,	M6x09_GENERAL },
	{ 0xC1, 2, "CMPB",  IMM,	M6x09_GENERAL },
	{ 0xC2, 2, "SBCB",  IMM,	M6x09_GENERAL },
	{ 0xC3, 3, "ADDD",  IMM,	M6x09_GENERAL },
	{ 0xC4, 2, "ANDB",  IMM,	M6x09_GENERAL },
	{ 0xC5, 2, "BITB",  IMM,	M6x09_GENERAL },
	{ 0xC6, 2, "LDB",   IMM,	M6x09_GENERAL },
	{ 0xC8, 2, "EORB",  IMM,	M6x09_GENERAL },
	{ 0xC9, 2, "ADCB",  IMM,	M6x09_GENERAL },
	{ 0xCA, 2, "ORB",   IMM,	M6x09_GENERAL },
	{ 0xCB, 2, "ADDB",  IMM,	M6x09_GENERAL },
	{ 0xCC, 3, "LDD",   IMM,	M6x09_GENERAL },
	{ 0xCD, 5, "LDQ",   IMM,	HD6309_EXCLUSIVE },
	{ 0xCE, 3, "LDU",   IMM,	M6x09_GENERAL },

	{ 0xD0, 2, "SUBB",  DIR,	M6x09_GENERAL },
	{ 0xD1, 2, "CMPB",  DIR,	M6x09_GENERAL },
	{ 0xD2, 2, "SBCB",  DIR,	M6x09_GENERAL },
	{ 0xD3, 2, "ADDD",  DIR,	M6x09_GENERAL },
	{ 0xD4, 2, "ANDB",  DIR,	M6x09_GENERAL },
	{ 0xD5, 2, "BITB",  DIR,	M6x09_GENERAL },
	{ 0xD6, 2, "LDB",   DIR,	M6x09_GENERAL },
	{ 0xD7, 2, "STB",   DIR,	M6x09_GENERAL },
	{ 0xD8, 2, "EORB",  DIR,	M6x09_GENERAL },
	{ 0xD9, 2, "ADCB",  DIR,	M6x09_GENERAL },
	{ 0xDA, 2, "ORB",   DIR,	M6x09_GENERAL },
	{ 0xDB, 2, "ADDB",  DIR,	M6x09_GENERAL },
	{ 0xDC, 2, "LDD",   DIR,	M6x09_GENERAL },
	{ 0xDD, 2, "STD",   DIR,	M6x09_GENERAL },
	{ 0xDE, 2, "LDU",   DIR,	M6x09_GENERAL },
	{ 0xDF, 2, "STU",   DIR,	M6x09_GENERAL },

	{ 0xE0, 2, "SUBB",  IND,	M6x09_GENERAL },
	{ 0xE1, 2, "CMPB",  IND,	M6x09_GENERAL },
	{ 0xE2, 2, "SBCB",  IND,	M6x09_GENERAL },
	{ 0xE3, 2, "ADDD",  IND,	M6x09_GENERAL },
	{ 0xE4, 2, "ANDB",  IND,	M6x09_GENERAL },
	{ 0xE5, 2, "BITB",  IND,	M6x09_GENERAL },
	{ 0xE6, 2, "LDB",   IND,	M6x09_GENERAL },
	{ 0xE7, 2, "STB",   IND,	M6x09_GENERAL },
	{ 0xE8, 2, "EORB",  IND,	M6x09_GENERAL },
	{ 0xE9, 2, "ADCB",  IND,	M6x09_GENERAL },
	{ 0xEA, 2, "ORB",   IND,	M6x09_GENERAL },
	{ 0xEB, 2, "ADDB",  IND,	M6x09_GENERAL },
	{ 0xEC, 2, "LDD",   IND,	M6x09_GENERAL },
	{ 0xED, 2, "STD",   IND,	M6x09_GENERAL },
	{ 0xEE, 2, "LDU",   IND,	M6x09_GENERAL },
	{ 0xEF, 2, "STU",   IND,	M6x09_GENERAL },

	{ 0xF0, 3, "SUBB",  EXT,	M6x09_GENERAL },
	{ 0xF1, 3, "CMPB",  EXT,	M6x09_GENERAL },
	{ 0xF2, 3, "SBCB",  EXT,	M6x09_GENERAL },
	{ 0xF3, 3, "ADDD",  EXT,	M6x09_GENERAL },
	{ 0xF4, 3, "ANDB",  EXT,	M6x09_GENERAL },
	{ 0xF5, 3, "BITB",  EXT,	M6x09_GENERAL },
	{ 0xF6, 3, "LDB",   EXT,	M6x09_GENERAL },
	{ 0xF7, 3, "STB",   EXT,	M6x09_GENERAL },
	{ 0xF8, 3, "EORB",  EXT,	M6x09_GENERAL },
	{ 0xF9, 3, "ADCB",  EXT,	M6x09_GENERAL },
	{ 0xFA, 3, "ORB",   EXT,	M6x09_GENERAL },
	{ 0xFB, 3, "ADDB",  EXT,	M6x09_GENERAL },
	{ 0xFC, 3, "LDD",   EXT,	M6x09_GENERAL },
	{ 0xFD, 3, "STD",   EXT,	M6x09_GENERAL },
	{ 0xFE, 3, "LDU",   EXT,	M6x09_GENERAL },
	{ 0xFF, 3, "STU",   EXT,	M6x09_GENERAL }
};

// Page 1 opcodes (0x10 0x..)
static const opcodeinfo m6x09_pg1opcodes[] =
{
	{ 0x21, 4, "LBRN",  LREL,	M6x09_GENERAL },
	{ 0x22, 4, "LBHI",  LREL,	M6x09_GENERAL },
	{ 0x23, 4, "LBLS",  LREL,	M6x09_GENERAL },
	{ 0x24, 4, "LBCC",  LREL,	M6x09_GENERAL },
	{ 0x25, 4, "LBCS",  LREL,	M6x09_GENERAL },
	{ 0x26, 4, "LBNE",  LREL,	M6x09_GENERAL },
	{ 0x27, 4, "LBEQ",  LREL,	M6x09_GENERAL },
	{ 0x28, 4, "LBVC",  LREL,	M6x09_GENERAL },
	{ 0x29, 4, "LBVS",  LREL,	M6x09_GENERAL },
	{ 0x2A, 4, "LBPL",  LREL,	M6x09_GENERAL },
	{ 0x2B, 4, "LBMI",  LREL,	M6x09_GENERAL },
	{ 0x2C, 4, "LBGE",  LREL,	M6x09_GENERAL },
	{ 0x2D, 4, "LBLT",  LREL,	M6x09_GENERAL },
	{ 0x2E, 4, "LBGT",  LREL,	M6x09_GENERAL },
	{ 0x2F, 4, "LBLE",  LREL,	M6x09_GENERAL },

	{ 0x30, 3, "ADDR",  IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x31, 3, "ADCR",  IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x32, 3, "SUBR",  IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x33, 3, "SBCR",  IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x34, 3, "ANDR",  IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x35, 3, "ORR",   IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x36, 3, "EORR",  IMM_RR,	HD6309_EXCLUSIVE },
	{ 0x37, 3, "CMPR",  IMM_RR,	HD6309_EXCLUSIVE },

	{ 0x38, 2, "PSHSW", INH,	HD6309_EXCLUSIVE },
	{ 0x39, 2, "PULSW", INH,	HD6309_EXCLUSIVE },
	{ 0x3A, 2, "PSHUW", INH,	HD6309_EXCLUSIVE },
	{ 0x3B, 2, "PULUW", INH,	HD6309_EXCLUSIVE },

	{ 0x3F, 2, "SWI2",  INH,	HD6309_EXCLUSIVE },

	{ 0x40, 2, "NEGD",  INH,	HD6309_EXCLUSIVE },
	{ 0x43, 2, "COMD",  INH,	HD6309_EXCLUSIVE },
	{ 0x44, 2, "LSRD",  INH,	HD6309_EXCLUSIVE },
	{ 0x46, 2, "RORD",  INH,	HD6309_EXCLUSIVE },
	{ 0x47, 2, "ASRD",  INH,	HD6309_EXCLUSIVE },
	{ 0x48, 2, "ASLD",  INH,	HD6309_EXCLUSIVE },
	{ 0x49, 2, "ROLD",  INH,	HD6309_EXCLUSIVE },

	{ 0x4A, 2, "DECD",  INH,	HD6309_EXCLUSIVE },
	{ 0x4C, 2, "INCD",  INH,	HD6309_EXCLUSIVE },
	{ 0x4D, 2, "TSTD",  INH,	HD6309_EXCLUSIVE },
	{ 0x4f, 2, "CLRD",  INH,	HD6309_EXCLUSIVE },

	{ 0x53, 2, "COMW",  INH,	HD6309_EXCLUSIVE },
	{ 0x54, 2, "LSRW",  INH,	HD6309_EXCLUSIVE },
	{ 0x56, 2, "RORW",  INH,	HD6309_EXCLUSIVE },
	{ 0x59, 2, "ROLW",  INH,	HD6309_EXCLUSIVE },
	{ 0x5A, 2, "DECW",  INH,	HD6309_EXCLUSIVE },
	{ 0x5C, 2, "INCW",  INH,	HD6309_EXCLUSIVE },
	{ 0x5D, 2, "TSTW",  INH,	HD6309_EXCLUSIVE },
	{ 0x5F, 2, "CLRW",  INH,	HD6309_EXCLUSIVE },
	{ 0x80, 4, "SUBW",  IMM,	HD6309_EXCLUSIVE },
	{ 0x81, 4, "CMPW",  IMM,	HD6309_EXCLUSIVE },
	{ 0x82, 4, "SBCD",  IMM,	HD6309_EXCLUSIVE },

	{ 0x83, 4, "CMPD",  IMM,	M6x09_GENERAL },

	{ 0x84, 4, "ANDD",  IMM,	HD6309_EXCLUSIVE },
	{ 0x85, 4, "BITD",  IMM,	HD6309_EXCLUSIVE },
	{ 0x86, 4, "LDW",   IMM,	HD6309_EXCLUSIVE },
	{ 0x88, 4, "EORD",  IMM,	HD6309_EXCLUSIVE },
	{ 0x89, 4, "ADCD",  IMM,	HD6309_EXCLUSIVE },
	{ 0x8A, 4, "ORD",   IMM,	HD6309_EXCLUSIVE },
	{ 0x8B, 4, "ADDW",  IMM,	HD6309_EXCLUSIVE },

	{ 0x8C, 4, "CMPY",  IMM,	M6x09_GENERAL },
	{ 0x8E, 4, "LDY",   IMM,	M6x09_GENERAL },

	{ 0x90, 3, "SUBW",  DIR,	HD6309_EXCLUSIVE },
	{ 0x91, 3, "CMPW",  DIR,	HD6309_EXCLUSIVE },
	{ 0x92, 3, "SBCD",  DIR,	HD6309_EXCLUSIVE },

	{ 0x93, 3, "CMPD",  DIR,	M6x09_GENERAL },

	{ 0x94, 3, "ANDD",  DIR,	HD6309_EXCLUSIVE },
	{ 0x95, 3, "BITD",  DIR,	HD6309_EXCLUSIVE },
	{ 0x96, 3, "LDW",   DIR,	HD6309_EXCLUSIVE },
	{ 0x97, 3, "STW",   DIR,	HD6309_EXCLUSIVE },
	{ 0x98, 3, "EORD",  DIR,	HD6309_EXCLUSIVE },
	{ 0x99, 3, "ADCD",  DIR,	HD6309_EXCLUSIVE },
	{ 0x9A, 3, "ORD",   DIR,	HD6309_EXCLUSIVE },
	{ 0x9B, 3, "ADDW",  DIR,	HD6309_EXCLUSIVE },

	{ 0x9C, 3, "CMPY",  DIR,	M6x09_GENERAL },
	{ 0x9E, 3, "LDY",   DIR,	M6x09_GENERAL },
	{ 0x9F, 3, "STY",   DIR,	M6x09_GENERAL },

	{ 0xA0, 3, "SUBW",  IND,	HD6309_EXCLUSIVE },
	{ 0xA1, 3, "CMPW",  IND,	HD6309_EXCLUSIVE },
	{ 0xA2, 3, "SBCD",  IND,	HD6309_EXCLUSIVE },

	{ 0xA3, 3, "CMPD",  IND,	M6x09_GENERAL },

	{ 0xA4, 3, "ANDD",  IND,	HD6309_EXCLUSIVE },
	{ 0xA5, 3, "BITD",  IND,	HD6309_EXCLUSIVE },

	{ 0xA6, 3, "LDW",   IND,	HD6309_EXCLUSIVE },
	{ 0xA7, 3, "STW",   IND,	HD6309_EXCLUSIVE },
	{ 0xA8, 3, "EORD",  IND,	HD6309_EXCLUSIVE },
	{ 0xA9, 3, "ADCD",  IND,	HD6309_EXCLUSIVE },
	{ 0xAA, 3, "ORD",   IND,	HD6309_EXCLUSIVE },
	{ 0xAB, 3, "ADDW",  IND,	HD6309_EXCLUSIVE },

	{ 0xAC, 3, "CMPY",  IND,	M6x09_GENERAL },
	{ 0xAE, 3, "LDY",   IND,	M6x09_GENERAL },
	{ 0xAF, 3, "STY",   IND,	M6x09_GENERAL },

	{ 0xB0, 4, "SUBW",  EXT,	HD6309_EXCLUSIVE },
	{ 0xB1, 4, "CMPW",  EXT,	HD6309_EXCLUSIVE },
	{ 0xB2, 4, "SBCD",  EXT,	HD6309_EXCLUSIVE },

	{ 0xB3, 4, "CMPD",  EXT,	M6x09_GENERAL },

	{ 0xB4, 4, "ANDD",  EXT,	HD6309_EXCLUSIVE },
	{ 0xB5, 4, "BITD",  EXT,	HD6309_EXCLUSIVE },
	{ 0xB6, 4, "LDW",   EXT,	HD6309_EXCLUSIVE },
	{ 0xB7, 4, "STW",   EXT,	HD6309_EXCLUSIVE },
	{ 0xB8, 4, "EORD",  EXT,	HD6309_EXCLUSIVE },
	{ 0xB9, 4, "ADCD",  EXT,	HD6309_EXCLUSIVE },
	{ 0xBA, 4, "ORD",   EXT,	HD6309_EXCLUSIVE },
	{ 0xBB, 4, "ADDW",  EXT,	HD6309_EXCLUSIVE },

	{ 0xBC, 4, "CMPY",  EXT,	M6x09_GENERAL },
	{ 0xBE, 4, "LDY",   EXT,	M6x09_GENERAL },
	{ 0xBF, 4, "STY",   EXT,	M6x09_GENERAL },
	{ 0xCE, 4, "LDS",   IMM,	M6x09_GENERAL },

	{ 0xDC, 3, "LDQ",   DIR,	HD6309_EXCLUSIVE },
	{ 0xDD, 3, "STQ",   DIR,	HD6309_EXCLUSIVE },

	{ 0xDE, 3, "LDS",   DIR,	M6x09_GENERAL },
	{ 0xDF, 3, "STS",   DIR,	M6x09_GENERAL },

	{ 0xEC, 3, "LDQ",   IND,	HD6309_EXCLUSIVE },
	{ 0xED, 3, "STQ",   IND,	HD6309_EXCLUSIVE },

	{ 0xEE, 3, "LDS",   IND,	M6x09_GENERAL },
	{ 0xEF, 3, "STS",   IND,	M6x09_GENERAL },

	{ 0xFC, 4, "LDQ",   EXT,	HD6309_EXCLUSIVE },
	{ 0xFD, 4, "STQ",   EXT,	HD6309_EXCLUSIVE },

	{ 0xFE, 4, "LDS",   EXT,	M6x09_GENERAL },
	{ 0xFF, 4, "STS",   EXT,	M6x09_GENERAL }
};

// Page 2 opcodes (0x11 0x..)
static const opcodeinfo m6x09_pg2opcodes[] =
{
	{ 0x30, 4, "BAND",  IMM_BW,	HD6309_EXCLUSIVE },
	{ 0x31, 4, "BIAND", IMM_BW,	HD6309_EXCLUSIVE },
	{ 0x32, 4, "BOR",   IMM_BW,	HD6309_EXCLUSIVE },
	{ 0x33, 4, "BIOR",  IMM_BW,	HD6309_EXCLUSIVE },
	{ 0x34, 4, "BEOR",  IMM_BW,	HD6309_EXCLUSIVE },
	{ 0x35, 4, "BIEOR", IMM_BW,	HD6309_EXCLUSIVE },

	{ 0x36, 4, "LDBT",  IMM_BW,	HD6309_EXCLUSIVE },
	{ 0x37, 4, "STBT",  IMM_BW,	HD6309_EXCLUSIVE },

	{ 0x38, 3, "TFM",   IMM_TFM,	HD6309_EXCLUSIVE },
	{ 0x39, 3, "TFM",   IMM_TFM,	HD6309_EXCLUSIVE },
	{ 0x3A, 3, "TFM",   IMM_TFM,	HD6309_EXCLUSIVE },
	{ 0x3B, 3, "TFM",   IMM_TFM,	HD6309_EXCLUSIVE },

	{ 0x3C, 3, "BITMD", IMM,	HD6309_EXCLUSIVE },
	{ 0x3D, 3, "LDMD",  IMM,	HD6309_EXCLUSIVE },

	{ 0x3F, 2, "SWI3",  INH,	M6x09_GENERAL },

	{ 0x43, 2, "COME",  INH,	HD6309_EXCLUSIVE },
	{ 0x4A, 2, "DECE",  INH,	HD6309_EXCLUSIVE },
	{ 0x4C, 2, "INCE",  INH,	HD6309_EXCLUSIVE },
	{ 0x4D, 2, "TSTE",  INH,	HD6309_EXCLUSIVE },
	{ 0x4F, 2, "CLRE",  INH,	HD6309_EXCLUSIVE },
	{ 0x53, 2, "COMF",  INH,	HD6309_EXCLUSIVE },
	{ 0x5A, 2, "DECF",  INH,	HD6309_EXCLUSIVE },
	{ 0x5C, 2, "INCF",  INH,	HD6309_EXCLUSIVE },
	{ 0x5D, 2, "TSTF",  INH,	HD6309_EXCLUSIVE },
	{ 0x5F, 2, "CLRF",  INH,	HD6309_EXCLUSIVE },

	{ 0x80, 3, "SUBE",  IMM,	HD6309_EXCLUSIVE },
	{ 0x81, 3, "CMPE",  IMM,	HD6309_EXCLUSIVE },

	{ 0x83, 4, "CMPU",  IMM,	M6x09_GENERAL },

	{ 0x86, 3, "LDE",   IMM,	HD6309_EXCLUSIVE },
	{ 0x8b, 3, "ADDE",  IMM,	HD6309_EXCLUSIVE },

	{ 0x8C, 4, "CMPS",  IMM,	M6x09_GENERAL },

	{ 0x8D, 3, "DIVD",  IMM,	HD6309_EXCLUSIVE },
	{ 0x8E, 4, "DIVQ",  IMM,	HD6309_EXCLUSIVE },
	{ 0x8F, 4, "MULD",  IMM,	HD6309_EXCLUSIVE },
	{ 0x90, 3, "SUBE",  DIR,	HD6309_EXCLUSIVE },
	{ 0x91, 3, "CMPE",  DIR,	HD6309_EXCLUSIVE },

	{ 0x93, 3, "CMPU",  DIR,	M6x09_GENERAL },

	{ 0x96, 3, "LDE",   DIR,	HD6309_EXCLUSIVE },
	{ 0x97, 3, "STE",   DIR,	HD6309_EXCLUSIVE },
	{ 0x9B, 3, "ADDE",  DIR,	HD6309_EXCLUSIVE },

	{ 0x9C, 3, "CMPS",  DIR,	M6x09_GENERAL },

	{ 0x9D, 3, "DIVD",  DIR,	HD6309_EXCLUSIVE },
	{ 0x9E, 3, "DIVQ",  DIR,	HD6309_EXCLUSIVE },
	{ 0x9F, 3, "MULD",  DIR,	HD6309_EXCLUSIVE },

	{ 0xA0, 3, "SUBE",  IND,	HD6309_EXCLUSIVE },
	{ 0xA1, 3, "CMPE",  IND,	HD6309_EXCLUSIVE },

	{ 0xA3, 3, "CMPU",  IND,	M6x09_GENERAL },

	{ 0xA6, 3, "LDE",   IND,	HD6309_EXCLUSIVE },
	{ 0xA7, 3, "STE",   IND,	HD6309_EXCLUSIVE },
	{ 0xAB, 3, "ADDE",  IND,	HD6309_EXCLUSIVE },

	{ 0xAC, 3, "CMPS",  IND,	M6x09_GENERAL },

	{ 0xAD, 3, "DIVD",  IND,	HD6309_EXCLUSIVE },
	{ 0xAE, 3, "DIVQ",  IND,	HD6309_EXCLUSIVE },
	{ 0xAF, 3, "MULD",  IND,	HD6309_EXCLUSIVE },
	{ 0xB0, 4, "SUBE",  EXT,	HD6309_EXCLUSIVE },
	{ 0xB1, 4, "CMPE",  EXT,	HD6309_EXCLUSIVE },

	{ 0xB3, 4, "CMPU",  EXT,	M6x09_GENERAL },

	{ 0xB6, 4, "LDE",   EXT,	HD6309_EXCLUSIVE },
	{ 0xB7, 4, "STE",   EXT,	HD6309_EXCLUSIVE },

	{ 0xBB, 4, "ADDE",  EXT,	HD6309_EXCLUSIVE },
	{ 0xBC, 4, "CMPS",  EXT,	M6x09_GENERAL },

	{ 0xBD, 4, "DIVD",  EXT,	HD6309_EXCLUSIVE },
	{ 0xBE, 4, "DIVQ",  EXT,	HD6309_EXCLUSIVE },
	{ 0xBF, 4, "MULD",  EXT,	HD6309_EXCLUSIVE },

	{ 0xC0, 3, "SUBF",  IMM,	HD6309_EXCLUSIVE },
	{ 0xC1, 3, "CMPF",  IMM,	HD6309_EXCLUSIVE },
	{ 0xC6, 3, "LDF",   IMM,	HD6309_EXCLUSIVE },
	{ 0xCB, 3, "ADDF",  IMM,	HD6309_EXCLUSIVE },

	{ 0xD0, 3, "SUBF",  DIR,	HD6309_EXCLUSIVE },
	{ 0xD1, 3, "CMPF",  DIR,	HD6309_EXCLUSIVE },
	{ 0xD6, 3, "LDF",   DIR,	HD6309_EXCLUSIVE },
	{ 0xD7, 3, "STF",   DIR,	HD6309_EXCLUSIVE },
	{ 0xDB, 3, "ADDF",  DIR,	HD6309_EXCLUSIVE },

	{ 0xE0, 3, "SUBF",  IND,	HD6309_EXCLUSIVE },
	{ 0xE1, 3, "CMPF",  IND,	HD6309_EXCLUSIVE },
	{ 0xE6, 3, "LDF",   IND,	HD6309_EXCLUSIVE },
	{ 0xE7, 3, "STF",   IND,	HD6309_EXCLUSIVE },
	{ 0xEB, 3, "ADDF",  IND,	HD6309_EXCLUSIVE },

	{ 0xF0, 4, "SUBF",  EXT,	HD6309_EXCLUSIVE },
	{ 0xF1, 4, "CMPF",  EXT,	HD6309_EXCLUSIVE },
	{ 0xF6, 4, "LDF",   EXT,	HD6309_EXCLUSIVE },
	{ 0xF7, 4, "STF",   EXT,	HD6309_EXCLUSIVE },
	{ 0xFB, 4, "ADDF",  EXT,	HD6309_EXCLUSIVE }
};

static const opcodeinfo *const m6x09_pgpointers[3] =
{
	m6x09_pg0opcodes, m6x09_pg1opcodes, m6x09_pg2opcodes
};

static const int m6x09_numops[3] =
{
	ARRAY_LENGTH(m6x09_pg0opcodes),
	ARRAY_LENGTH(m6x09_pg1opcodes),
	ARRAY_LENGTH(m6x09_pg2opcodes)
};

static const char *const m6x09_regs[5] = { "X", "Y", "U", "S", "PC" };

static const char *const m6x09_btwregs[5] = { "CC", "A", "B", "inv" };

static const char *const hd6309_tfmregs[16] = {
		"D",   "X",   "Y",   "U",   "S", "inv", "inv", "inv",
	"inv", "inv", "inv", "inv", "inv", "inv", "inv", "inv"
};

static const char *const tfm_s[] = { "%s+,%s+", "%s-,%s-", "%s+,%s", "%s,%s+" };


//-------------------------------------------------
//  internal_m6x09_disassemble - core of the
//	disassembler
//-------------------------------------------------

static offs_t internal_m6x09_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, m6x09_instruction_level level, const char teregs[16][4])
{
	uint8_t pb, pbm, reg;
	unsigned int ea;
	int offset, indirect;

	int i, p = 0, page = 0;

	// look up the opcode
	uint8_t opcode;
	bool opcode_found;
	do
	{
		opcode = oprom[p++];
		for (i = 0; i < m6x09_numops[page]; i++)
			if (m6x09_pgpointers[page][i].opcode == opcode && m6x09_pgpointers[page][i].level <= level)
				break;

		// did we find something?
		if (i < m6x09_numops[page])
			opcode_found = true;
		else
		{
			stream << "Illegal Opcode";
			return p | DASMFLAG_SUPPORTED;
		}

		// was this a $10 or $11 page?
		if (m6x09_pgpointers[page][i].mode >= PG1)
		{
			page = m6x09_pgpointers[page][i].mode - PG1 + 1;
			opcode_found = false;
		}
	} while (!opcode_found);

	// how many operands do we have?
	int numoperands = (page == 0)
		? m6x09_pgpointers[page][i].length - 1
		: m6x09_pgpointers[page][i].length - 2;

	const uint8_t *operandarray = &opram[p];
	p += numoperands;
	pc += p;
	uint8_t mode = m6x09_pgpointers[page][i].mode;
	unsigned flags = m6x09_pgpointers[page][i].flags;

	// output the base instruction name
	util::stream_format(stream, "%-6s", m6x09_pgpointers[page][i].name);

	switch (mode)
	{
	case INH:
		switch (opcode)
		{
		case 0x34:  // PSHS
		case 0x36:  // PSHU
			pb = operandarray[0];
			if (pb & 0x80)
				util::stream_format(stream, "PC");
			if (pb & 0x40)
				util::stream_format(stream, "%s%s", (pb&0x80)?",":"", (opcode==0x34)?"U":"S");
			if (pb & 0x20)
				util::stream_format(stream, "%sY",  (pb&0xc0)?",":"");
			if (pb & 0x10)
				util::stream_format(stream, "%sX",  (pb&0xe0)?",":"");
			if (pb & 0x08)
				util::stream_format(stream, "%sDP", (pb&0xf0)?",":"");
			if (pb & 0x04)
				util::stream_format(stream, "%sB",  (pb&0xf8)?",":"");
			if (pb & 0x02)
				util::stream_format(stream, "%sA",  (pb&0xfc)?",":"");
			if (pb & 0x01)
				util::stream_format(stream, "%sCC", (pb&0xfe)?",":"");
			break;
		case 0x35:  // PULS
		case 0x37:  // PULU
			pb = operandarray[0];
			if (pb & 0x01)
				util::stream_format(stream, "CC");
			if (pb & 0x02)
				util::stream_format(stream, "%sA",  (pb&0x01)?",":"");
			if (pb & 0x04)
				util::stream_format(stream, "%sB",  (pb&0x03)?",":"");
			if (pb & 0x08)
				util::stream_format(stream, "%sDP", (pb&0x07)?",":"");
			if (pb & 0x10)
				util::stream_format(stream, "%sX",  (pb&0x0f)?",":"");
			if (pb & 0x20)
				util::stream_format(stream, "%sY",  (pb&0x1f)?",":"");
			if (pb & 0x40)
				util::stream_format(stream, "%s%s", (pb&0x3f)?",":"", (opcode==0x35)?"U":"S");
			if (pb & 0x80)
				util::stream_format(stream, "%sPC", (pb&0x7f)?",":"");
			break;
		default:
			// No operands
			break;
		}
		break;

	case DIR:
		ea = operandarray[0];
		util::stream_format(stream, "$%02X", ea);
		break;

	case DIR_IM:
		assert(level >= HD6309_EXCLUSIVE);
		util::stream_format(stream, "#$%02X;", operandarray[0]);
		util::stream_format(stream, "$%02X", operandarray[1]);
		break;

	case REL:
		offset = (int8_t)operandarray[0];
		util::stream_format(stream, "$%04X", (pc + offset) & 0xffff);
		break;

	case LREL:
		offset = (int16_t)((operandarray[0] << 8) + operandarray[1]);
		util::stream_format(stream, "$%04X", (pc + offset) & 0xffff);
		break;

	case EXT:
		if (numoperands == 3)
		{
			assert(level >= HD6309_EXCLUSIVE);
			pb = operandarray[0];
			ea = (operandarray[1] << 8) + operandarray[2];
			util::stream_format(stream, "#$%02X,$%04X", pb, ea);
		}
		else if (numoperands == 2)
		{
			ea = (operandarray[0] << 8) + operandarray[1];
			util::stream_format(stream, "$%04X", ea);
		}
		break;

	case IND:
		if (numoperands == 2)
		{
			assert(level >= HD6309_EXCLUSIVE);
			util::stream_format(stream, "#$%02X;", operandarray[0]);
			pb = operandarray[1];
		}
		else
		{
			pb = operandarray[0];
		}

		reg = (pb >> 5) & 3;
		pbm = pb & 0x8f;
		indirect = ((pb & 0x90) == 0x90 ) ? true : false;

		// open brackets if indirect
		if (indirect && pbm != 0x82)
			util::stream_format(stream, "[");

		switch (pbm)
		{
		case 0x80:  // ,R+ or operations relative to W
			if (indirect)
			{
				switch (reg)
				{
				case 0x00:
					util::stream_format(stream, ",W");
					break;
				case 0x01:
					offset = (int16_t)((opram[p+0] << 8) + opram[p+1]);
					p += 2;
					util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
					util::stream_format(stream, "$%04X,W", (offset < 0) ? -offset : offset);
					break;
				case 0x02:
					util::stream_format(stream, ",W++");
					break;
				case 0x03:
					util::stream_format(stream, ",--W");
					break;
				}
			}
			else
				util::stream_format(stream, ",%s+", m6x09_regs[reg]);
			break;

		case 0x81:  // ,R++
			util::stream_format(stream, ",%s++", m6x09_regs[reg]);
			break;

		case 0x82:  // ,-R
			if (indirect)
				stream << "Illegal Postbyte";
			else
				util::stream_format(stream, ",-%s", m6x09_regs[reg]);
			break;

		case 0x83:  // ,--R
			util::stream_format(stream, ",--%s", m6x09_regs[reg]);
			break;

		case 0x84:  // ,R
			util::stream_format(stream, ",%s", m6x09_regs[reg]);
			break;

		case 0x85:  // (+/- B),R
			util::stream_format(stream, "B,%s", m6x09_regs[reg]);
			break;

		case 0x86:  // (+/- A),R
			util::stream_format(stream, "A,%s", m6x09_regs[reg]);
			break;

		case 0x87:  // (+/- E),R
			util::stream_format(stream, "E,%s", m6x09_regs[reg]);
			break;

		case 0x88:  // (+/- 7 bit offset),R
			offset = (int8_t)opram[p++];
			util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
			util::stream_format(stream, "$%02X,", (offset < 0) ? -offset : offset);
			util::stream_format(stream, "%s", m6x09_regs[reg]);
			break;

		case 0x89:  // (+/- 15 bit offset),R
			offset = (int16_t)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
			util::stream_format(stream, "$%04X,", (offset < 0) ? -offset : offset);
			util::stream_format(stream, "%s", m6x09_regs[reg]);
			break;

		case 0x8a:  // (+/- F),R
			util::stream_format(stream, "F,%s", m6x09_regs[reg]);
			break;

		case 0x8b:  // (+/- D),R
			util::stream_format(stream, "D,%s", m6x09_regs[reg]);
			break;

		case 0x8c:  // (+/- 7 bit offset),PC
			offset = (int8_t)opram[p++];
			util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
			util::stream_format(stream, "$%02X,PC", (offset < 0) ? -offset : offset);
			break;

		case 0x8d:  // (+/- 15 bit offset),PC
			offset = (int16_t)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
			util::stream_format(stream, "$%04X,PC", (offset < 0) ? -offset : offset);
			break;

		case 0x8e:  // (+/- W),R
			util::stream_format(stream, "W,%s", m6x09_regs[reg]);
			break;

		case 0x8f:  // address or operations relative to W
			if (indirect)
			{
				ea = (uint16_t)((opram[p+0] << 8) + opram[p+1]);
				p += 2;
				util::stream_format(stream, "$%04X", ea);
				break;
			}
			else
			{
				switch (reg)
				{
				case 0x00:
					util::stream_format(stream, ",W");
					break;
				case 0x01:
					offset = (int16_t)((opram[p+0] << 8) + opram[p+1]);
					p += 2;
					util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
					util::stream_format(stream, "$%04X,W", (offset < 0) ? -offset : offset);
					break;
				case 0x02:
					util::stream_format(stream, ",W++");
					break;
				case 0x03:
					util::stream_format(stream, ",--W");
					break;
				}
			}
			break;

		default:    // (+/- 4 bit offset),R
			offset = pb & 0x1f;
			if (offset > 15)
				offset = offset - 32;
			util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
			util::stream_format(stream, "$%X,", (offset < 0) ? -offset : offset);
			util::stream_format(stream, "%s", m6x09_regs[reg]);
			break;
		}

		// close brackets if indirect
		if (indirect && pbm != 0x82)
			util::stream_format(stream, "]");
		break;

	case IMM:
		if (numoperands == 4)
		{
			ea = (operandarray[0] << 24) + (operandarray[1] << 16) + (operandarray[2] << 8) + operandarray[3];
			util::stream_format(stream, "#$%08X", ea);
		}
		else
		if (numoperands == 2)
		{
			ea = (operandarray[0] << 8) + operandarray[1];
			util::stream_format(stream, "#$%04X", ea);
		}
		else
		if (numoperands == 1)
		{
			ea = operandarray[0];
			util::stream_format(stream, "#$%02X", ea);
		}
		break;

	case IMM_RR:
		pb = operandarray[0];
		util::stream_format(stream, "%s,%s", teregs[(pb >> 4) & 0xf], teregs[pb & 0xf]);
		break;

	case IMM_BW:
		pb = operandarray[0];
		util::stream_format(stream, "%s,", m6x09_btwregs[((pb & 0xc0) >> 6)]);
		util::stream_format(stream, "%d,", (pb & 0x38) >> 3);
		util::stream_format(stream, "%d,", (pb & 0x07));
		util::stream_format(stream, "$%02X", operandarray[1]);
		break;

	case IMM_TFM:
		pb = operandarray[0];
		util::stream_format(stream, tfm_s[opcode & 0x07], hd6309_tfmregs[(pb >> 4) & 0xf], hd6309_tfmregs[pb & 0xf]);
		break;
	}

	return p | flags | DASMFLAG_SUPPORTED;
}


//-------------------------------------------------
//  M6809 disassembler
//-------------------------------------------------

CPU_DISASSEMBLE(m6809)
{
	static const char m6809_teregs[16][4] =
	{
		"D", "X",  "Y",  "U",   "S",  "PC", "inv", "inv",
		"A", "B", "CC", "DP", "inv", "inv", "inv", "inv"
	};
	std::ostringstream stream;
	offs_t result = internal_m6x09_disassemble(stream, pc, oprom, opram, M6x09_GENERAL, m6809_teregs);
	std::string stream_str = stream.str();
	strcpy(buffer, stream_str.c_str());
	return result;
}


//-------------------------------------------------
//  HD6309 disassembler
//-------------------------------------------------

CPU_DISASSEMBLE(hd6309)
{
	static const char hd6309_teregs[16][4] =
	{
		"D", "X",  "Y",  "U", "S", "PC", "W", "V",
		"A", "B", "CC", "DP", "0",  "0", "E", "F"
	};
	std::ostringstream stream;
	offs_t result = internal_m6x09_disassemble(stream, pc, oprom, opram, HD6309_EXCLUSIVE, hd6309_teregs);
	std::string stream_str = stream.str();
	strcpy(buffer, stream_str.c_str());
	return result;
}
