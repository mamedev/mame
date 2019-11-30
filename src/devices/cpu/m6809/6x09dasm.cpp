// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Sean Riddle,Tim Lindner
/*****************************************************************************

    6x09dasm.cpp - a 6809/6309/Konami opcode disassembler

    Based on:
        6309dasm.c - a 6309 opcode disassembler
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
#include "6x09dasm.h"


const char *const m6x09_base_disassembler::m6x09_regs[5] = { "X", "Y", "U", "S", "PC" };

const char *const m6x09_base_disassembler::m6x09_btwregs[5] = { "CC", "A", "B", "inv" };

const char *const m6x09_base_disassembler::hd6309_tfmregs[16] = {
	"D",   "X",   "Y",   "U",   "S", "inv", "inv", "inv",
	"inv", "inv", "inv", "inv", "inv", "inv", "inv", "inv"
};

const char *const m6x09_base_disassembler::tfm_s[] = { "%s+,%s+", "%s-,%s-", "%s+,%s", "%s,%s+" };

//**************************************************************************
//  BASE CLASS
//**************************************************************************

u32 m6x09_base_disassembler::opcode_alignment() const
{
	return 1;
}

//-------------------------------------------------
//  fetch_opcode
//-------------------------------------------------

const m6x09_base_disassembler::opcodeinfo *m6x09_base_disassembler::fetch_opcode(const data_buffer &opcodes, offs_t &p)
{
	uint16_t page = 0;
	const opcodeinfo *op = nullptr;
	while(!op)
	{
		// retrieve the opcode
		uint16_t opcode = page | opcodes.r8(p++);

		// perform the lookup
		auto iter = std::find_if(
			m_opcodes.begin(),
			m_opcodes.end(),
			[opcode](const opcodeinfo &info) { return info.opcode() == opcode; });

		// did we find something?
		if (iter == m_opcodes.end())
			return nullptr;

		// was this a $10 or $11 page?
		switch (iter->mode())
		{
		case PG1:
			page = 0x1000;
			break;

		case PG2:
			page = 0x1100;
			break;

		default:
			op = &*iter;
			break;
		}
	};

	// is this an HD6309 exclusive instruction, and we're not HD6309?  if so, reject it
	if (op && (op->level() > m_level))
		op = nullptr;

	return op;
}


//-------------------------------------------------
//  disassemble - core of the disassembler
//-------------------------------------------------

offs_t m6x09_base_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint8_t pb;
	unsigned int ea;
	int offset;
	offs_t p = pc;

	// look up the opcode
	const opcodeinfo *op = fetch_opcode(opcodes, p);
	if (!op)
	{
		// illegal opcode
		util::stream_format(stream, "%-6s$%02X", "FCB", opcodes.r8(pc));
		for (offs_t q = pc + 1; q < p; q++)
			util::stream_format(stream, ",$%02X", opcodes.r8(q));
		return (p - pc) | SUPPORTED;
	}

	// how many operands do we have?
	int numoperands = (p - pc == 1)
		? op->length() - 1
		: op->length() - 2;

	offs_t ppc = p;
	p += numoperands;

	// output the base instruction name
	if (op->mode() == INH)
		stream << op->name();
	else
		util::stream_format(stream, "%-6s", op->name());

	switch (op->mode())
	{
	case INH:
		// No operands
		break;

	case PSHS:
	case PSHU:
		pb = params.r8(ppc);
		if (pb & 0x80)
			util::stream_format(stream, "PC");
		if (pb & 0x40)
			util::stream_format(stream, "%s%s", (pb&0x80)?",":"", (op->mode() == PSHS)?"U":"S");
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

	case PULS:
	case PULU:
		pb = params.r8(ppc);
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
			util::stream_format(stream, "%s%s", (pb&0x3f)?",":"", (op->mode() == PULS)?"U":"S");
		if (pb & 0x80)
			util::stream_format(stream, "%sPC", (pb&0x7f)?",":"");
		break;

	case DIR:
		ea = params.r8(ppc);
		util::stream_format(stream, "$%02X", ea);
		break;

	case DIR_IM:
		assert_hd6309_exclusive();
		util::stream_format(stream, "#$%02X;", params.r8(ppc));
		util::stream_format(stream, "$%02X", params.r8(ppc + 1));
		break;

	case REL:
		offset = (int8_t)params.r8(ppc);
		util::stream_format(stream, "$%04X", (pc + op->length() + offset) & 0xffff);
		break;

	case LREL:
		offset = (int16_t)params.r16(ppc);
		util::stream_format(stream, "$%04X", (pc + op->length() + offset) & 0xffff);
		break;

	case EXT:
		if (numoperands == 3)
		{
			assert_hd6309_exclusive();
			pb = params.r8(ppc);
			ea = params.r16(ppc+1);
			util::stream_format(stream, "#$%02X,$%04X", pb, ea);
		}
		else if (numoperands == 2)
		{
			ea = params.r16(ppc);
			if( !(ea & 0xff00) )
			{
				stream << '>'; // need the '>' to force an assembler to use EXT addressing
			}
			util::stream_format(stream, "$%04X", ea);
		}
		break;

	case IND:
		if (numoperands == 2)
		{
			assert_hd6309_exclusive();
			util::stream_format(stream, "#$%02X;", params.r8(ppc));
			pb = params.r8(ppc+1);
		}
		else
		{
			pb = params.r8(ppc);
		}

		indexed(stream, pb, params, p);
		break;

	case IMM:
		if (numoperands == 4)
		{
			ea = params.r32(ppc);
			util::stream_format(stream, "#$%08X", ea);
		}
		else
		if (numoperands == 2)
		{
			ea = params.r16(ppc);
			util::stream_format(stream, "#$%04X", ea);
		}
		else
		if (numoperands == 1)
		{
			ea = params.r8(ppc);
			util::stream_format(stream, "#$%02X", ea);
		}
		break;

	case IMM_RR:
		pb = params.r8(ppc);
		register_register(stream, pb);
		break;

	case IMM_BW:
		pb = params.r8(ppc);
		util::stream_format(stream, "%s,", m6x09_btwregs[((pb & 0xc0) >> 6)]);
		util::stream_format(stream, "%d,", (pb & 0x38) >> 3);
		util::stream_format(stream, "%d,", (pb & 0x07));
		util::stream_format(stream, "$%02X", params.r8(ppc+1));
		break;

	case IMM_TFM:
		pb = params.r8(ppc);
		util::stream_format(stream, tfm_s[op->opcode() & 0x07], hd6309_tfmregs[(pb >> 4) & 0xf], hd6309_tfmregs[pb & 0xf]);
		break;

	default:
		throw false;
	}

	return (p - pc) | op->flags() | SUPPORTED;
}


//**************************************************************************
//  M6809/HD6309 disassembler
//**************************************************************************

const m6x09_base_disassembler::opcodeinfo m6x09_disassembler::m6x09_opcodes[] =
{
	// Page 0 opcodes (single byte)
	{ 0x00, 2, "NEG",   DIR,    M6x09_GENERAL },
	{ 0x01, 3, "OIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x02, 3, "AIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x03, 2, "COM",   DIR,    M6x09_GENERAL },
	{ 0x04, 2, "LSR",   DIR,    M6x09_GENERAL },
	{ 0x05, 3, "EIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x06, 2, "ROR",   DIR,    M6x09_GENERAL },
	{ 0x07, 2, "ASR",   DIR,    M6x09_GENERAL },
	{ 0x08, 2, "ASL",   DIR,    M6x09_GENERAL },
	{ 0x09, 2, "ROL",   DIR,    M6x09_GENERAL },
	{ 0x0A, 2, "DEC",   DIR,    M6x09_GENERAL },
	{ 0x0B, 3, "TIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x0C, 2, "INC",   DIR,    M6x09_GENERAL },
	{ 0x0D, 2, "TST",   DIR,    M6x09_GENERAL },
	{ 0x0E, 2, "JMP",   DIR,    M6x09_GENERAL },
	{ 0x0F, 2, "CLR",   DIR,    M6x09_GENERAL },

	{ 0x10, 1, "page1", PG1,    M6x09_GENERAL },
	{ 0x11, 1, "page2", PG2,    M6x09_GENERAL },
	{ 0x12, 1, "NOP",   INH,    M6x09_GENERAL },
	{ 0x13, 1, "SYNC",  INH,    M6x09_GENERAL },
	{ 0x14, 1, "SEXW",  INH,    HD6309_EXCLUSIVE },
	{ 0x16, 3, "LBRA",  LREL,   M6x09_GENERAL },
	{ 0x17, 3, "LBSR",  LREL,   M6x09_GENERAL, STEP_OVER },
	{ 0x19, 1, "DAA",   INH,    M6x09_GENERAL },
	{ 0x1A, 2, "ORCC",  IMM,    M6x09_GENERAL },
	{ 0x1C, 2, "ANDCC", IMM,    M6x09_GENERAL },
	{ 0x1D, 1, "SEX",   INH,    M6x09_GENERAL },
	{ 0x1E, 2, "EXG",   IMM_RR, M6x09_GENERAL },
	{ 0x1F, 2, "TFR",   IMM_RR, M6x09_GENERAL },

	{ 0x20, 2, "BRA",   REL,    M6x09_GENERAL },
	{ 0x21, 2, "BRN",   REL,    M6x09_GENERAL },
	{ 0x22, 2, "BHI",   REL,    M6x09_GENERAL },
	{ 0x23, 2, "BLS",   REL,    M6x09_GENERAL },
	{ 0x24, 2, "BCC",   REL,    M6x09_GENERAL },
	{ 0x25, 2, "BCS",   REL,    M6x09_GENERAL },
	{ 0x26, 2, "BNE",   REL,    M6x09_GENERAL },
	{ 0x27, 2, "BEQ",   REL,    M6x09_GENERAL },
	{ 0x28, 2, "BVC",   REL,    M6x09_GENERAL },
	{ 0x29, 2, "BVS",   REL,    M6x09_GENERAL },
	{ 0x2A, 2, "BPL",   REL,    M6x09_GENERAL },
	{ 0x2B, 2, "BMI",   REL,    M6x09_GENERAL },
	{ 0x2C, 2, "BGE",   REL,    M6x09_GENERAL },
	{ 0x2D, 2, "BLT",   REL,    M6x09_GENERAL },
	{ 0x2E, 2, "BGT",   REL,    M6x09_GENERAL },
	{ 0x2F, 2, "BLE",   REL,    M6x09_GENERAL },

	{ 0x30, 2, "LEAX",  IND,    M6x09_GENERAL },
	{ 0x31, 2, "LEAY",  IND,    M6x09_GENERAL },
	{ 0x32, 2, "LEAS",  IND,    M6x09_GENERAL },
	{ 0x33, 2, "LEAU",  IND,    M6x09_GENERAL },
	{ 0x34, 2, "PSHS",  PSHS,   M6x09_GENERAL },
	{ 0x35, 2, "PULS",  PULS,   M6x09_GENERAL },
	{ 0x36, 2, "PSHU",  PSHU,   M6x09_GENERAL },
	{ 0x37, 2, "PULU",  PULU,   M6x09_GENERAL },
	{ 0x39, 1, "RTS",   INH ,   M6x09_GENERAL },
	{ 0x3A, 1, "ABX",   INH,    M6x09_GENERAL },
	{ 0x3B, 1, "RTI",   INH,    M6x09_GENERAL },
	{ 0x3C, 2, "CWAI",  IMM,    M6x09_GENERAL },
	{ 0x3D, 1, "MUL",   INH,    M6x09_GENERAL },
	{ 0x3F, 1, "SWI",   INH,    M6x09_GENERAL },

	{ 0x40, 1, "NEGA",  INH,    M6x09_GENERAL },
	{ 0x43, 1, "COMA",  INH,    M6x09_GENERAL },
	{ 0x44, 1, "LSRA",  INH,    M6x09_GENERAL },
	{ 0x46, 1, "RORA",  INH,    M6x09_GENERAL },
	{ 0x47, 1, "ASRA",  INH,    M6x09_GENERAL },
	{ 0x48, 1, "ASLA",  INH,    M6x09_GENERAL },
	{ 0x49, 1, "ROLA",  INH,    M6x09_GENERAL },
	{ 0x4A, 1, "DECA",  INH,    M6x09_GENERAL },
	{ 0x4C, 1, "INCA",  INH,    M6x09_GENERAL },
	{ 0x4D, 1, "TSTA",  INH,    M6x09_GENERAL },
	{ 0x4F, 1, "CLRA",  INH,    M6x09_GENERAL },

	{ 0x50, 1, "NEGB",  INH,    M6x09_GENERAL },
	{ 0x53, 1, "COMB",  INH,    M6x09_GENERAL },
	{ 0x54, 1, "LSRB",  INH,    M6x09_GENERAL },
	{ 0x56, 1, "RORB",  INH,    M6x09_GENERAL },
	{ 0x57, 1, "ASRB",  INH,    M6x09_GENERAL },
	{ 0x58, 1, "ASLB",  INH,    M6x09_GENERAL },
	{ 0x59, 1, "ROLB",  INH,    M6x09_GENERAL },
	{ 0x5A, 1, "DECB",  INH,    M6x09_GENERAL },
	{ 0x5C, 1, "INCB",  INH,    M6x09_GENERAL },
	{ 0x5D, 1, "TSTB",  INH,    M6x09_GENERAL },
	{ 0x5F, 1, "CLRB",  INH,    M6x09_GENERAL },

	{ 0x60, 2, "NEG",   IND,    M6x09_GENERAL },
	{ 0x61, 3, "OIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x62, 3, "AIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x63, 2, "COM",   IND,    M6x09_GENERAL },
	{ 0x64, 2, "LSR",   IND,    M6x09_GENERAL },
	{ 0x65, 3, "EIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x66, 2, "ROR",   IND,    M6x09_GENERAL },
	{ 0x67, 2, "ASR",   IND,    M6x09_GENERAL },
	{ 0x68, 2, "ASL",   IND,    M6x09_GENERAL },
	{ 0x69, 2, "ROL",   IND,    M6x09_GENERAL },
	{ 0x6A, 2, "DEC",   IND,    M6x09_GENERAL },
	{ 0x6B, 3, "TIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x6C, 2, "INC",   IND,    M6x09_GENERAL },
	{ 0x6D, 2, "TST",   IND,    M6x09_GENERAL },
	{ 0x6E, 2, "JMP",   IND,    M6x09_GENERAL },
	{ 0x6F, 2, "CLR",   IND,    M6x09_GENERAL },

	{ 0x70, 3, "NEG",   EXT,    M6x09_GENERAL },
	{ 0x71, 4, "OIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x72, 4, "AIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x73, 3, "COM",   EXT,    M6x09_GENERAL },
	{ 0x74, 3, "LSR",   EXT,    M6x09_GENERAL },
	{ 0x75, 4, "EIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x76, 3, "ROR",   EXT,    M6x09_GENERAL },
	{ 0x77, 3, "ASR",   EXT,    M6x09_GENERAL },
	{ 0x78, 3, "ASL",   EXT,    M6x09_GENERAL },
	{ 0x79, 3, "ROL",   EXT,    M6x09_GENERAL },
	{ 0x7A, 3, "DEC",   EXT,    M6x09_GENERAL },
	{ 0x7B, 4, "TIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x7C, 3, "INC",   EXT,    M6x09_GENERAL },
	{ 0x7D, 3, "TST",   EXT,    M6x09_GENERAL },
	{ 0x7E, 3, "JMP",   EXT,    M6x09_GENERAL },
	{ 0x7F, 3, "CLR",   EXT,    M6x09_GENERAL },

	{ 0x80, 2, "SUBA",  IMM,    M6x09_GENERAL },
	{ 0x81, 2, "CMPA",  IMM,    M6x09_GENERAL },
	{ 0x82, 2, "SBCA",  IMM,    M6x09_GENERAL },
	{ 0x83, 3, "SUBD",  IMM,    M6x09_GENERAL },
	{ 0x84, 2, "ANDA",  IMM,    M6x09_GENERAL },
	{ 0x85, 2, "BITA",  IMM,    M6x09_GENERAL },
	{ 0x86, 2, "LDA",   IMM,    M6x09_GENERAL },
	{ 0x88, 2, "EORA",  IMM,    M6x09_GENERAL },
	{ 0x89, 2, "ADCA",  IMM,    M6x09_GENERAL },
	{ 0x8A, 2, "ORA",   IMM,    M6x09_GENERAL },
	{ 0x8B, 2, "ADDA",  IMM,    M6x09_GENERAL },
	{ 0x8C, 3, "CMPX",  IMM,    M6x09_GENERAL },
	{ 0x8D, 2, "BSR",   REL,    M6x09_GENERAL, STEP_OVER },
	{ 0x8E, 3, "LDX",   IMM,    M6x09_GENERAL },

	{ 0x90, 2, "SUBA",  DIR,    M6x09_GENERAL },
	{ 0x91, 2, "CMPA",  DIR,    M6x09_GENERAL },
	{ 0x92, 2, "SBCA",  DIR,    M6x09_GENERAL },
	{ 0x93, 2, "SUBD",  DIR,    M6x09_GENERAL },
	{ 0x94, 2, "ANDA",  DIR,    M6x09_GENERAL },
	{ 0x95, 2, "BITA",  DIR,    M6x09_GENERAL },
	{ 0x96, 2, "LDA",   DIR,    M6x09_GENERAL },
	{ 0x97, 2, "STA",   DIR,    M6x09_GENERAL },
	{ 0x98, 2, "EORA",  DIR,    M6x09_GENERAL },
	{ 0x99, 2, "ADCA",  DIR,    M6x09_GENERAL },
	{ 0x9A, 2, "ORA",   DIR,    M6x09_GENERAL },
	{ 0x9B, 2, "ADDA",  DIR,    M6x09_GENERAL },
	{ 0x9C, 2, "CMPX",  DIR,    M6x09_GENERAL },
	{ 0x9D, 2, "JSR",   DIR,    M6x09_GENERAL, STEP_OVER },
	{ 0x9E, 2, "LDX",   DIR,    M6x09_GENERAL },
	{ 0x9F, 2, "STX",   DIR,    M6x09_GENERAL },

	{ 0xA0, 2, "SUBA",  IND,    M6x09_GENERAL },
	{ 0xA1, 2, "CMPA",  IND,    M6x09_GENERAL },
	{ 0xA2, 2, "SBCA",  IND,    M6x09_GENERAL },
	{ 0xA3, 2, "SUBD",  IND,    M6x09_GENERAL },
	{ 0xA4, 2, "ANDA",  IND,    M6x09_GENERAL },
	{ 0xA5, 2, "BITA",  IND,    M6x09_GENERAL },
	{ 0xA6, 2, "LDA",   IND,    M6x09_GENERAL },
	{ 0xA7, 2, "STA",   IND,    M6x09_GENERAL },
	{ 0xA8, 2, "EORA",  IND,    M6x09_GENERAL },
	{ 0xA9, 2, "ADCA",  IND,    M6x09_GENERAL },
	{ 0xAA, 2, "ORA",   IND,    M6x09_GENERAL },
	{ 0xAB, 2, "ADDA",  IND,    M6x09_GENERAL },
	{ 0xAC, 2, "CMPX",  IND,    M6x09_GENERAL },
	{ 0xAD, 2, "JSR",   IND,    M6x09_GENERAL, STEP_OVER },
	{ 0xAE, 2, "LDX",   IND,    M6x09_GENERAL },
	{ 0xAF, 2, "STX",   IND,    M6x09_GENERAL },

	{ 0xB0, 3, "SUBA",  EXT,    M6x09_GENERAL },
	{ 0xB1, 3, "CMPA",  EXT,    M6x09_GENERAL },
	{ 0xB2, 3, "SBCA",  EXT,    M6x09_GENERAL },
	{ 0xB3, 3, "SUBD",  EXT,    M6x09_GENERAL },
	{ 0xB4, 3, "ANDA",  EXT,    M6x09_GENERAL },
	{ 0xB5, 3, "BITA",  EXT,    M6x09_GENERAL },
	{ 0xB6, 3, "LDA",   EXT,    M6x09_GENERAL },
	{ 0xB7, 3, "STA",   EXT,    M6x09_GENERAL },
	{ 0xB8, 3, "EORA",  EXT,    M6x09_GENERAL },
	{ 0xB9, 3, "ADCA",  EXT,    M6x09_GENERAL },
	{ 0xBA, 3, "ORA",   EXT,    M6x09_GENERAL },
	{ 0xBB, 3, "ADDA",  EXT,    M6x09_GENERAL },
	{ 0xBC, 3, "CMPX",  EXT,    M6x09_GENERAL },
	{ 0xBD, 3, "JSR",   EXT,    M6x09_GENERAL, STEP_OVER },
	{ 0xBE, 3, "LDX",   EXT,    M6x09_GENERAL },
	{ 0xBF, 3, "STX",   EXT,    M6x09_GENERAL },

	{ 0xC0, 2, "SUBB",  IMM,    M6x09_GENERAL },
	{ 0xC1, 2, "CMPB",  IMM,    M6x09_GENERAL },
	{ 0xC2, 2, "SBCB",  IMM,    M6x09_GENERAL },
	{ 0xC3, 3, "ADDD",  IMM,    M6x09_GENERAL },
	{ 0xC4, 2, "ANDB",  IMM,    M6x09_GENERAL },
	{ 0xC5, 2, "BITB",  IMM,    M6x09_GENERAL },
	{ 0xC6, 2, "LDB",   IMM,    M6x09_GENERAL },
	{ 0xC8, 2, "EORB",  IMM,    M6x09_GENERAL },
	{ 0xC9, 2, "ADCB",  IMM,    M6x09_GENERAL },
	{ 0xCA, 2, "ORB",   IMM,    M6x09_GENERAL },
	{ 0xCB, 2, "ADDB",  IMM,    M6x09_GENERAL },
	{ 0xCC, 3, "LDD",   IMM,    M6x09_GENERAL },
	{ 0xCD, 5, "LDQ",   IMM,    HD6309_EXCLUSIVE },
	{ 0xCE, 3, "LDU",   IMM,    M6x09_GENERAL },

	{ 0xD0, 2, "SUBB",  DIR,    M6x09_GENERAL },
	{ 0xD1, 2, "CMPB",  DIR,    M6x09_GENERAL },
	{ 0xD2, 2, "SBCB",  DIR,    M6x09_GENERAL },
	{ 0xD3, 2, "ADDD",  DIR,    M6x09_GENERAL },
	{ 0xD4, 2, "ANDB",  DIR,    M6x09_GENERAL },
	{ 0xD5, 2, "BITB",  DIR,    M6x09_GENERAL },
	{ 0xD6, 2, "LDB",   DIR,    M6x09_GENERAL },
	{ 0xD7, 2, "STB",   DIR,    M6x09_GENERAL },
	{ 0xD8, 2, "EORB",  DIR,    M6x09_GENERAL },
	{ 0xD9, 2, "ADCB",  DIR,    M6x09_GENERAL },
	{ 0xDA, 2, "ORB",   DIR,    M6x09_GENERAL },
	{ 0xDB, 2, "ADDB",  DIR,    M6x09_GENERAL },
	{ 0xDC, 2, "LDD",   DIR,    M6x09_GENERAL },
	{ 0xDD, 2, "STD",   DIR,    M6x09_GENERAL },
	{ 0xDE, 2, "LDU",   DIR,    M6x09_GENERAL },
	{ 0xDF, 2, "STU",   DIR,    M6x09_GENERAL },

	{ 0xE0, 2, "SUBB",  IND,    M6x09_GENERAL },
	{ 0xE1, 2, "CMPB",  IND,    M6x09_GENERAL },
	{ 0xE2, 2, "SBCB",  IND,    M6x09_GENERAL },
	{ 0xE3, 2, "ADDD",  IND,    M6x09_GENERAL },
	{ 0xE4, 2, "ANDB",  IND,    M6x09_GENERAL },
	{ 0xE5, 2, "BITB",  IND,    M6x09_GENERAL },
	{ 0xE6, 2, "LDB",   IND,    M6x09_GENERAL },
	{ 0xE7, 2, "STB",   IND,    M6x09_GENERAL },
	{ 0xE8, 2, "EORB",  IND,    M6x09_GENERAL },
	{ 0xE9, 2, "ADCB",  IND,    M6x09_GENERAL },
	{ 0xEA, 2, "ORB",   IND,    M6x09_GENERAL },
	{ 0xEB, 2, "ADDB",  IND,    M6x09_GENERAL },
	{ 0xEC, 2, "LDD",   IND,    M6x09_GENERAL },
	{ 0xED, 2, "STD",   IND,    M6x09_GENERAL },
	{ 0xEE, 2, "LDU",   IND,    M6x09_GENERAL },
	{ 0xEF, 2, "STU",   IND,    M6x09_GENERAL },

	{ 0xF0, 3, "SUBB",  EXT,    M6x09_GENERAL },
	{ 0xF1, 3, "CMPB",  EXT,    M6x09_GENERAL },
	{ 0xF2, 3, "SBCB",  EXT,    M6x09_GENERAL },
	{ 0xF3, 3, "ADDD",  EXT,    M6x09_GENERAL },
	{ 0xF4, 3, "ANDB",  EXT,    M6x09_GENERAL },
	{ 0xF5, 3, "BITB",  EXT,    M6x09_GENERAL },
	{ 0xF6, 3, "LDB",   EXT,    M6x09_GENERAL },
	{ 0xF7, 3, "STB",   EXT,    M6x09_GENERAL },
	{ 0xF8, 3, "EORB",  EXT,    M6x09_GENERAL },
	{ 0xF9, 3, "ADCB",  EXT,    M6x09_GENERAL },
	{ 0xFA, 3, "ORB",   EXT,    M6x09_GENERAL },
	{ 0xFB, 3, "ADDB",  EXT,    M6x09_GENERAL },
	{ 0xFC, 3, "LDD",   EXT,    M6x09_GENERAL },
	{ 0xFD, 3, "STD",   EXT,    M6x09_GENERAL },
	{ 0xFE, 3, "LDU",   EXT,    M6x09_GENERAL },
	{ 0xFF, 3, "STU",   EXT,    M6x09_GENERAL },

	// Page 1 opcodes (0x10 0x..)
	{ 0x1021, 4, "LBRN",  LREL, M6x09_GENERAL },
	{ 0x1022, 4, "LBHI",  LREL, M6x09_GENERAL },
	{ 0x1023, 4, "LBLS",  LREL, M6x09_GENERAL },
	{ 0x1024, 4, "LBCC",  LREL, M6x09_GENERAL },
	{ 0x1025, 4, "LBCS",  LREL, M6x09_GENERAL },
	{ 0x1026, 4, "LBNE",  LREL, M6x09_GENERAL },
	{ 0x1027, 4, "LBEQ",  LREL, M6x09_GENERAL },
	{ 0x1028, 4, "LBVC",  LREL, M6x09_GENERAL },
	{ 0x1029, 4, "LBVS",  LREL, M6x09_GENERAL },
	{ 0x102A, 4, "LBPL",  LREL, M6x09_GENERAL },
	{ 0x102B, 4, "LBMI",  LREL, M6x09_GENERAL },
	{ 0x102C, 4, "LBGE",  LREL, M6x09_GENERAL },
	{ 0x102D, 4, "LBLT",  LREL, M6x09_GENERAL },
	{ 0x102E, 4, "LBGT",  LREL, M6x09_GENERAL },
	{ 0x102F, 4, "LBLE",  LREL, M6x09_GENERAL },

	{ 0x1030, 3, "ADDR",  IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1031, 3, "ADCR",  IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1032, 3, "SUBR",  IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1033, 3, "SBCR",  IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1034, 3, "ANDR",  IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1035, 3, "ORR",   IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1036, 3, "EORR",  IMM_RR,   HD6309_EXCLUSIVE },
	{ 0x1037, 3, "CMPR",  IMM_RR,   HD6309_EXCLUSIVE },

	{ 0x1038, 2, "PSHSW", INH,  HD6309_EXCLUSIVE },
	{ 0x1039, 2, "PULSW", INH,  HD6309_EXCLUSIVE },
	{ 0x103A, 2, "PSHUW", INH,  HD6309_EXCLUSIVE },
	{ 0x103B, 2, "PULUW", INH,  HD6309_EXCLUSIVE },

	{ 0x103F, 2, "SWI2",  INH,  M6x09_GENERAL },

	{ 0x1040, 2, "NEGD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1043, 2, "COMD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1044, 2, "LSRD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1046, 2, "RORD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1047, 2, "ASRD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1048, 2, "ASLD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1049, 2, "ROLD",  INH,  HD6309_EXCLUSIVE },

	{ 0x104A, 2, "DECD",  INH,  HD6309_EXCLUSIVE },
	{ 0x104C, 2, "INCD",  INH,  HD6309_EXCLUSIVE },
	{ 0x104D, 2, "TSTD",  INH,  HD6309_EXCLUSIVE },
	{ 0x104f, 2, "CLRD",  INH,  HD6309_EXCLUSIVE },

	{ 0x1053, 2, "COMW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1054, 2, "LSRW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1056, 2, "RORW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1059, 2, "ROLW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105A, 2, "DECW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105C, 2, "INCW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105D, 2, "TSTW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105F, 2, "CLRW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1080, 4, "SUBW",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1081, 4, "CMPW",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1082, 4, "SBCD",  IMM,  HD6309_EXCLUSIVE },

	{ 0x1083, 4, "CMPD",  IMM,  M6x09_GENERAL },

	{ 0x1084, 4, "ANDD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1085, 4, "BITD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1086, 4, "LDW",   IMM,  HD6309_EXCLUSIVE },
	{ 0x1088, 4, "EORD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1089, 4, "ADCD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x108A, 4, "ORD",   IMM,  HD6309_EXCLUSIVE },
	{ 0x108B, 4, "ADDW",  IMM,  HD6309_EXCLUSIVE },

	{ 0x108C, 4, "CMPY",  IMM,  M6x09_GENERAL },
	{ 0x108E, 4, "LDY",   IMM,  M6x09_GENERAL },

	{ 0x1090, 3, "SUBW",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1091, 3, "CMPW",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1092, 3, "SBCD",  DIR,  HD6309_EXCLUSIVE },

	{ 0x1093, 3, "CMPD",  DIR,  M6x09_GENERAL },

	{ 0x1094, 3, "ANDD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1095, 3, "BITD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1096, 3, "LDW",   DIR,  HD6309_EXCLUSIVE },
	{ 0x1097, 3, "STW",   DIR,  HD6309_EXCLUSIVE },
	{ 0x1098, 3, "EORD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1099, 3, "ADCD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x109A, 3, "ORD",   DIR,  HD6309_EXCLUSIVE },
	{ 0x109B, 3, "ADDW",  DIR,  HD6309_EXCLUSIVE },

	{ 0x109C, 3, "CMPY",  DIR,  M6x09_GENERAL },
	{ 0x109E, 3, "LDY",   DIR,  M6x09_GENERAL },
	{ 0x109F, 3, "STY",   DIR,  M6x09_GENERAL },

	{ 0x10A0, 3, "SUBW",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A1, 3, "CMPW",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A2, 3, "SBCD",  IND,  HD6309_EXCLUSIVE },

	{ 0x10A3, 3, "CMPD",  IND,  M6x09_GENERAL },

	{ 0x10A4, 3, "ANDD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A5, 3, "BITD",  IND,  HD6309_EXCLUSIVE },

	{ 0x10A6, 3, "LDW",   IND,  HD6309_EXCLUSIVE },
	{ 0x10A7, 3, "STW",   IND,  HD6309_EXCLUSIVE },
	{ 0x10A8, 3, "EORD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A9, 3, "ADCD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10AA, 3, "ORD",   IND,  HD6309_EXCLUSIVE },
	{ 0x10AB, 3, "ADDW",  IND,  HD6309_EXCLUSIVE },

	{ 0x10AC, 3, "CMPY",  IND,  M6x09_GENERAL },
	{ 0x10AE, 3, "LDY",   IND,  M6x09_GENERAL },
	{ 0x10AF, 3, "STY",   IND,  M6x09_GENERAL },

	{ 0x10B0, 4, "SUBW",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B1, 4, "CMPW",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B2, 4, "SBCD",  EXT,  HD6309_EXCLUSIVE },

	{ 0x10B3, 4, "CMPD",  EXT,  M6x09_GENERAL },

	{ 0x10B4, 4, "ANDD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B5, 4, "BITD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B6, 4, "LDW",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10B7, 4, "STW",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10B8, 4, "EORD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B9, 4, "ADCD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10BA, 4, "ORD",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10BB, 4, "ADDW",  EXT,  HD6309_EXCLUSIVE },

	{ 0x10BC, 4, "CMPY",  EXT,  M6x09_GENERAL },
	{ 0x10BE, 4, "LDY",   EXT,  M6x09_GENERAL },
	{ 0x10BF, 4, "STY",   EXT,  M6x09_GENERAL },
	{ 0x10CE, 4, "LDS",   IMM,  M6x09_GENERAL },

	{ 0x10DC, 3, "LDQ",   DIR,  HD6309_EXCLUSIVE },
	{ 0x10DD, 3, "STQ",   DIR,  HD6309_EXCLUSIVE },

	{ 0x10DE, 3, "LDS",   DIR,  M6x09_GENERAL },
	{ 0x10DF, 3, "STS",   DIR,  M6x09_GENERAL },

	{ 0x10EC, 3, "LDQ",   IND,  HD6309_EXCLUSIVE },
	{ 0x10ED, 3, "STQ",   IND,  HD6309_EXCLUSIVE },

	{ 0x10EE, 3, "LDS",   IND,  M6x09_GENERAL },
	{ 0x10EF, 3, "STS",   IND,  M6x09_GENERAL },

	{ 0x10FC, 4, "LDQ",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10FD, 4, "STQ",   EXT,  HD6309_EXCLUSIVE },

	{ 0x10FE, 4, "LDS",   EXT,  M6x09_GENERAL },
	{ 0x10FF, 4, "STS",   EXT,  M6x09_GENERAL },

	// Page 2 opcodes (0x11 0x..)
	{ 0x1130, 4, "BAND",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1131, 4, "BIAND", IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1132, 4, "BOR",   IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1133, 4, "BIOR",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1134, 4, "BEOR",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1135, 4, "BIEOR", IMM_BW,   HD6309_EXCLUSIVE },

	{ 0x1136, 4, "LDBT",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1137, 4, "STBT",  IMM_BW,   HD6309_EXCLUSIVE },

	{ 0x1138, 3, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x1139, 3, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x113A, 3, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x113B, 3, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },

	{ 0x113C, 3, "BITMD", IMM,  HD6309_EXCLUSIVE },
	{ 0x113D, 3, "LDMD",  IMM,  HD6309_EXCLUSIVE },

	{ 0x113F, 2, "SWI3",  INH,  M6x09_GENERAL },

	{ 0x1143, 2, "COME",  INH,  HD6309_EXCLUSIVE },
	{ 0x114A, 2, "DECE",  INH,  HD6309_EXCLUSIVE },
	{ 0x114C, 2, "INCE",  INH,  HD6309_EXCLUSIVE },
	{ 0x114D, 2, "TSTE",  INH,  HD6309_EXCLUSIVE },
	{ 0x114F, 2, "CLRE",  INH,  HD6309_EXCLUSIVE },
	{ 0x1153, 2, "COMF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115A, 2, "DECF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115C, 2, "INCF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115D, 2, "TSTF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115F, 2, "CLRF",  INH,  HD6309_EXCLUSIVE },

	{ 0x1180, 3, "SUBE",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1181, 3, "CMPE",  IMM,  HD6309_EXCLUSIVE },

	{ 0x1183, 4, "CMPU",  IMM,  M6x09_GENERAL },

	{ 0x1186, 3, "LDE",   IMM,  HD6309_EXCLUSIVE },
	{ 0x118B, 3, "ADDE",  IMM,  HD6309_EXCLUSIVE },

	{ 0x118C, 4, "CMPS",  IMM,  M6x09_GENERAL },

	{ 0x118D, 3, "DIVD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x118E, 4, "DIVQ",  IMM,  HD6309_EXCLUSIVE },
	{ 0x118F, 4, "MULD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1190, 3, "SUBE",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1191, 3, "CMPE",  DIR,  HD6309_EXCLUSIVE },

	{ 0x1193, 3, "CMPU",  DIR,  M6x09_GENERAL },

	{ 0x1196, 3, "LDE",   DIR,  HD6309_EXCLUSIVE },
	{ 0x1197, 3, "STE",   DIR,  HD6309_EXCLUSIVE },
	{ 0x119B, 3, "ADDE",  DIR,  HD6309_EXCLUSIVE },

	{ 0x119C, 3, "CMPS",  DIR,  M6x09_GENERAL },

	{ 0x119D, 3, "DIVD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x119E, 3, "DIVQ",  DIR,  HD6309_EXCLUSIVE },
	{ 0x119F, 3, "MULD",  DIR,  HD6309_EXCLUSIVE },

	{ 0x11A0, 3, "SUBE",  IND,  HD6309_EXCLUSIVE },
	{ 0x11A1, 3, "CMPE",  IND,  HD6309_EXCLUSIVE },

	{ 0x11A3, 3, "CMPU",  IND,  M6x09_GENERAL },

	{ 0x11A6, 3, "LDE",   IND,  HD6309_EXCLUSIVE },
	{ 0x11A7, 3, "STE",   IND,  HD6309_EXCLUSIVE },
	{ 0x11AB, 3, "ADDE",  IND,  HD6309_EXCLUSIVE },

	{ 0x11AC, 3, "CMPS",  IND,  M6x09_GENERAL },

	{ 0x11AD, 3, "DIVD",  IND,  HD6309_EXCLUSIVE },
	{ 0x11AE, 3, "DIVQ",  IND,  HD6309_EXCLUSIVE },
	{ 0x11AF, 3, "MULD",  IND,  HD6309_EXCLUSIVE },
	{ 0x11B0, 4, "SUBE",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11B1, 4, "CMPE",  EXT,  HD6309_EXCLUSIVE },

	{ 0x11B3, 4, "CMPU",  EXT,  M6x09_GENERAL },

	{ 0x11B6, 4, "LDE",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11B7, 4, "STE",   EXT,  HD6309_EXCLUSIVE },

	{ 0x11BB, 4, "ADDE",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11BC, 4, "CMPS",  EXT,  M6x09_GENERAL },

	{ 0x11BD, 4, "DIVD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11BE, 4, "DIVQ",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11BF, 4, "MULD",  EXT,  HD6309_EXCLUSIVE },

	{ 0x11C0, 3, "SUBF",  IMM,  HD6309_EXCLUSIVE },
	{ 0x11C1, 3, "CMPF",  IMM,  HD6309_EXCLUSIVE },
	{ 0x11C6, 3, "LDF",   IMM,  HD6309_EXCLUSIVE },
	{ 0x11CB, 3, "ADDF",  IMM,  HD6309_EXCLUSIVE },

	{ 0x11D0, 3, "SUBF",  DIR,  HD6309_EXCLUSIVE },
	{ 0x11D1, 3, "CMPF",  DIR,  HD6309_EXCLUSIVE },
	{ 0x11D6, 3, "LDF",   DIR,  HD6309_EXCLUSIVE },
	{ 0x11D7, 3, "STF",   DIR,  HD6309_EXCLUSIVE },
	{ 0x11DB, 3, "ADDF",  DIR,  HD6309_EXCLUSIVE },

	{ 0x11E0, 3, "SUBF",  IND,  HD6309_EXCLUSIVE },
	{ 0x11E1, 3, "CMPF",  IND,  HD6309_EXCLUSIVE },
	{ 0x11E6, 3, "LDF",   IND,  HD6309_EXCLUSIVE },
	{ 0x11E7, 3, "STF",   IND,  HD6309_EXCLUSIVE },
	{ 0x11EB, 3, "ADDF",  IND,  HD6309_EXCLUSIVE },

	{ 0x11F0, 4, "SUBF",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11F1, 4, "CMPF",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11F6, 4, "LDF",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11F7, 4, "STF",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11FB, 4, "ADDF",  EXT,  HD6309_EXCLUSIVE }
};


// ======================> m6x09_disassembler

m6x09_disassembler::m6x09_disassembler(m6x09_instruction_level level, const char teregs[16][4])
	: m6x09_base_disassembler(m6x09_opcodes, ARRAY_LENGTH(m6x09_opcodes), level)
	, m_teregs(*reinterpret_cast<const std::array<std::array<char, 4>, 16> *>(teregs))
{
}

//-------------------------------------------------
//  indexed addressing mode for M6809/HD6309
//-------------------------------------------------

void m6x09_disassembler::indexed(std::ostream &stream, uint8_t pb, const data_buffer &params, offs_t &p)
{
	uint8_t reg = (pb >> 5) & 3;
	uint8_t pbm = pb & 0x8f;
	bool indirect = (pb & 0x90) == 0x90;
	int offset;
	unsigned int ea;

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
				offset = (int16_t)params.r16(p);
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
		offset = (int8_t)params.r8(p++);
		util::stream_format(stream, "%s", (offset < 0) ? "-" : "");
		util::stream_format(stream, "$%02X,", (offset < 0) ? -offset : offset);
		util::stream_format(stream, "%s", m6x09_regs[reg]);
		break;

	case 0x89:  // (+/- 15 bit offset),R
		offset = (int16_t)params.r16(p);
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
		offset = (int8_t)params.r8(p++);
		util::stream_format(stream, "$%04X,PCR", (p+offset)); // PC Relative addressing (assembler computes offset from specified absolute address)
		break;

	case 0x8d:  // (+/- 15 bit offset),PC
		offset = (int16_t)params.r16(p);
		p += 2;
		util::stream_format(stream, "$%04X,PCR", (p+offset)); // PC Relative addressing (assembler computes offset from specified absolute address)
		break;

	case 0x8e:  // (+/- W),R
		util::stream_format(stream, "W,%s", m6x09_regs[reg]);
		break;

	case 0x8f:  // address or operations relative to W
		if (indirect)
		{
			ea = (uint16_t)params.r16(p);
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
				offset = (int16_t)params.r16(p);
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
}


//-------------------------------------------------
//  M6809/HD6309 register/register parameter
//-------------------------------------------------

void m6x09_disassembler::register_register(std::ostream &stream, uint8_t pb)
{
	util::stream_format(stream, "%s,%s",
		m_teregs[(pb >> 4) & 0xf].data(),
		m_teregs[(pb >> 0) & 0xf].data());
}


//-------------------------------------------------
//  M6809 disassembler entry point
//-------------------------------------------------

const char m6809_disassembler::m6809_teregs[16][4] =
{
	"D", "X",  "Y",  "U",   "S",  "PC", "inv", "inv",
	"A", "B", "CC", "DP", "inv", "inv", "inv", "inv"
};


m6809_disassembler::m6809_disassembler() : m6x09_disassembler(M6x09_GENERAL, m6809_teregs)
{
}


//-------------------------------------------------
//  HD6309 disassembler entry point
//-------------------------------------------------

const char hd6309_disassembler::hd6309_teregs[16][4] =
{
	"D", "X",  "Y",  "U", "S", "PC", "W", "V",
	"A", "B", "CC", "DP", "0",  "0", "E", "F"
};

hd6309_disassembler::hd6309_disassembler() : m6x09_disassembler(HD6309_EXCLUSIVE, hd6309_teregs)
{
}


//**************************************************************************
//  Konami disassembler
//**************************************************************************

const m6x09_base_disassembler::opcodeinfo konami_disassembler::konami_opcodes[] =
{
	{ 0x08, 2, "LEAX",  IND,    M6x09_GENERAL },
	{ 0x09, 2, "LEAY",  IND,    M6x09_GENERAL },
	{ 0x0A, 2, "LEAU",  IND,    M6x09_GENERAL },
	{ 0x0B, 2, "LEAS",  IND,    M6x09_GENERAL },
	{ 0x0C, 2, "PUSHS",  PSHS,  M6x09_GENERAL },
	{ 0x0D, 2, "PUSHU",  PSHU,  M6x09_GENERAL },
	{ 0x0E, 2, "PULLS",  PULS,  M6x09_GENERAL },
	{ 0x0F, 2, "PULLU",  PULU,  M6x09_GENERAL },

	{ 0x10, 2, "LDA",   IMM,    M6x09_GENERAL },
	{ 0x11, 2, "LDB",   IMM,    M6x09_GENERAL },
	{ 0x12, 2, "LDA",   IND,    M6x09_GENERAL },
	{ 0x13, 2, "LDB",   IND,    M6x09_GENERAL },
	{ 0x14, 2, "ADDA",  IMM,    M6x09_GENERAL },
	{ 0x15, 2, "ADDB",  IMM,    M6x09_GENERAL },
	{ 0x16, 2, "ADDA",  IND,    M6x09_GENERAL },
	{ 0x17, 2, "ADDB",  IND,    M6x09_GENERAL },
	{ 0x18, 2, "ADCA",  IMM,    M6x09_GENERAL },
	{ 0x19, 2, "ADCB",  IMM,    M6x09_GENERAL },
	{ 0x1A, 2, "ADCA",  IND,    M6x09_GENERAL },
	{ 0x1B, 2, "ADCB",  IND,    M6x09_GENERAL },
	{ 0x1C, 2, "SUBA",  IMM,    M6x09_GENERAL },
	{ 0x1D, 2, "SUBB",  IMM,    M6x09_GENERAL },
	{ 0x1E, 2, "SUBA",  IND,    M6x09_GENERAL },
	{ 0x1F, 2, "SUBB",  IND,    M6x09_GENERAL },

	{ 0x20, 2, "SBCA",  IMM,    M6x09_GENERAL },
	{ 0x21, 2, "SBCB",  IMM,    M6x09_GENERAL },
	{ 0x22, 2, "SBCA",  IND,    M6x09_GENERAL },
	{ 0x23, 2, "SBCB",  IND,    M6x09_GENERAL },
	{ 0x24, 2, "ANDA",  IMM,    M6x09_GENERAL },
	{ 0x25, 2, "ANDB",  IMM,    M6x09_GENERAL },
	{ 0x26, 2, "ANDA",  IND,    M6x09_GENERAL },
	{ 0x27, 2, "ANDB",  IND,    M6x09_GENERAL },
	{ 0x28, 2, "BITA",  IMM,    M6x09_GENERAL },
	{ 0x29, 2, "BITB",  IMM,    M6x09_GENERAL },
	{ 0x2A, 2, "BITA",  IND,    M6x09_GENERAL },
	{ 0x2B, 2, "BITB",  IND,    M6x09_GENERAL },
	{ 0x2C, 2, "EORA",  IMM,    M6x09_GENERAL },
	{ 0x2D, 2, "EORB",  IMM,    M6x09_GENERAL },
	{ 0x2E, 2, "EORA",  IND,    M6x09_GENERAL },
	{ 0x2F, 2, "EORB",  IND,    M6x09_GENERAL },

	{ 0x30, 2, "ORA",   IMM,    M6x09_GENERAL },
	{ 0x31, 2, "ORB",   IMM,    M6x09_GENERAL },
	{ 0x32, 2, "ORA",   IND,    M6x09_GENERAL },
	{ 0x33, 2, "ORB",   IND,    M6x09_GENERAL },
	{ 0x34, 2, "CMPA",  IMM,    M6x09_GENERAL },
	{ 0x35, 2, "CMPB",  IMM,    M6x09_GENERAL },
	{ 0x36, 2, "CMPA",  IND,    M6x09_GENERAL },
	{ 0x37, 2, "CMPB",  IND,    M6x09_GENERAL },
	{ 0x38, 2, "SETLINES",  IMM,    M6x09_GENERAL },
	{ 0x39, 2, "SETLINES",  IND,    M6x09_GENERAL },
	{ 0x3A, 2, "STA",   IND,    M6x09_GENERAL },
	{ 0x3B, 2, "STB",   IND,    M6x09_GENERAL },
	{ 0x3C, 2, "ANDCC", IMM,    M6x09_GENERAL },
	{ 0x3D, 2, "ORCC",  IMM,    M6x09_GENERAL },
	{ 0x3E, 2, "EXG",   IMM_RR, M6x09_GENERAL },
	{ 0x3F, 2, "TFR",   IMM_RR, M6x09_GENERAL },

	{ 0x40, 3, "LDD",   IMM,    M6x09_GENERAL },
	{ 0x41, 2, "LDD",   IND,    M6x09_GENERAL },
	{ 0x42, 3, "LDX",   IMM,    M6x09_GENERAL },
	{ 0x43, 2, "LDX",   IND,    M6x09_GENERAL },
	{ 0x44, 3, "LDY",   IMM,    M6x09_GENERAL },
	{ 0x45, 2, "LDY",   IND,    M6x09_GENERAL },
	{ 0x46, 3, "LDU",   IMM,    M6x09_GENERAL },
	{ 0x47, 2, "LDU",   IND,    M6x09_GENERAL },
	{ 0x48, 3, "LDS",   IMM,    M6x09_GENERAL },
	{ 0x49, 2, "LDS",   IND,    M6x09_GENERAL },
	{ 0x4A, 3, "CMPD",  IMM,    M6x09_GENERAL },
	{ 0x4B, 2, "CMPD",  IND,    M6x09_GENERAL },
	{ 0x4C, 3, "CMPX",  IMM,    M6x09_GENERAL },
	{ 0x4D, 2, "CMPX",  IND,    M6x09_GENERAL },
	{ 0x4E, 3, "CMPY",  IMM,    M6x09_GENERAL },
	{ 0x4F, 2, "CMPY",  IND,    M6x09_GENERAL },

	{ 0x50, 3, "CMPU",  IMM,    M6x09_GENERAL },
	{ 0x51, 2, "CMPU",  IND,    M6x09_GENERAL },
	{ 0x52, 3, "CMPS",  IMM,    M6x09_GENERAL },
	{ 0x53, 2, "CMPS",  IND,    M6x09_GENERAL },
	{ 0x54, 3, "ADDD",  IMM,    M6x09_GENERAL },
	{ 0x55, 2, "ADDD",  IND,    M6x09_GENERAL },
	{ 0x56, 3, "SUBD",  IMM,    M6x09_GENERAL },
	{ 0x57, 2, "SUBD",  IND,    M6x09_GENERAL },
	{ 0x58, 2, "STD",   IND,    M6x09_GENERAL },
	{ 0x59, 2, "STX",   IND,    M6x09_GENERAL },
	{ 0x5A, 2, "STY",   IND,    M6x09_GENERAL },
	{ 0x5B, 2, "STU",   IND,    M6x09_GENERAL },
	{ 0x5C, 2, "STS",   IND,    M6x09_GENERAL },

	{ 0x60, 2, "BRA",   REL,    M6x09_GENERAL },
	{ 0x61, 2, "BHI",   REL,    M6x09_GENERAL },
	{ 0x62, 2, "BCC",   REL,    M6x09_GENERAL },
	{ 0x63, 2, "BNE",   REL,    M6x09_GENERAL },
	{ 0x64, 2, "BVC",   REL,    M6x09_GENERAL },
	{ 0x65, 2, "BPL",   REL,    M6x09_GENERAL },
	{ 0x66, 2, "BGE",   REL,    M6x09_GENERAL },
	{ 0x67, 2, "BGT",   REL,    M6x09_GENERAL },
	{ 0x68, 3, "LBRA",  LREL,   M6x09_GENERAL },
	{ 0x69, 3, "LBHI",  LREL,   M6x09_GENERAL },
	{ 0x6A, 3, "LBCC",  LREL,   M6x09_GENERAL },
	{ 0x6B, 3, "LBNE",  LREL,   M6x09_GENERAL },
	{ 0x6C, 3, "LBVC",  LREL,   M6x09_GENERAL },
	{ 0x6D, 3, "LBPL",  LREL,   M6x09_GENERAL },
	{ 0x6E, 3, "LBGE",  LREL,   M6x09_GENERAL },
	{ 0x6F, 3, "LBGT",  LREL,   M6x09_GENERAL },

	{ 0x70, 2, "BRN",   REL,    M6x09_GENERAL },
	{ 0x71, 2, "BLS",   REL,    M6x09_GENERAL },
	{ 0x72, 2, "BCS",   REL,    M6x09_GENERAL },
	{ 0x73, 2, "BEQ",   REL,    M6x09_GENERAL },
	{ 0x74, 2, "BVS",   REL,    M6x09_GENERAL },
	{ 0x75, 2, "BMI",   REL,    M6x09_GENERAL },
	{ 0x76, 2, "BLT",   REL,    M6x09_GENERAL },
	{ 0x77, 2, "BLE",   REL,    M6x09_GENERAL },
	{ 0x78, 3, "LBRN",  LREL,   M6x09_GENERAL },
	{ 0x79, 3, "LBLS",  LREL,   M6x09_GENERAL },
	{ 0x7A, 3, "LBCS",  LREL,   M6x09_GENERAL },
	{ 0x7B, 3, "LBEQ",  LREL,   M6x09_GENERAL },
	{ 0x7C, 3, "LBVS",  LREL,   M6x09_GENERAL },
	{ 0x7D, 3, "LBMI",  LREL,   M6x09_GENERAL },
	{ 0x7E, 3, "LBLT",  LREL,   M6x09_GENERAL },
	{ 0x7F, 3, "LBLE",  LREL,   M6x09_GENERAL },

	{ 0x80, 1, "CLRA",  INH,    M6x09_GENERAL },
	{ 0x81, 1, "CLRB",  INH,    M6x09_GENERAL },
	{ 0x82, 2, "CLR",   IND,    M6x09_GENERAL },
	{ 0x83, 1, "COMA",  INH,    M6x09_GENERAL },
	{ 0x84, 1, "COMB",  INH,    M6x09_GENERAL },
	{ 0x85, 2, "COM",   IND,    M6x09_GENERAL },
	{ 0x86, 1, "NEGA",  INH,    M6x09_GENERAL },
	{ 0x87, 1, "NEGB",  INH,    M6x09_GENERAL },
	{ 0x88, 2, "NEG",   IND,    M6x09_GENERAL },
	{ 0x89, 1, "INCA",  INH,    M6x09_GENERAL },
	{ 0x8A, 1, "INCB",  INH,    M6x09_GENERAL },
	{ 0x8B, 2, "INC",   IND,    M6x09_GENERAL },
	{ 0x8C, 1, "DECA",  INH,    M6x09_GENERAL },
	{ 0x8D, 1, "DECB",  INH,    M6x09_GENERAL },
	{ 0x8E, 2, "DEC",   IND,    M6x09_GENERAL },
	{ 0x8F, 1, "RTS",   INH ,   M6x09_GENERAL },

	{ 0x90, 1, "TSTA",  INH,    M6x09_GENERAL },
	{ 0x91, 1, "TSTB",  INH,    M6x09_GENERAL },
	{ 0x92, 2, "TST",   IND,    M6x09_GENERAL },
	{ 0x93, 1, "LSRA",  INH,    M6x09_GENERAL },
	{ 0x94, 1, "LSRB",  INH,    M6x09_GENERAL },
	{ 0x95, 2, "LSR",   IND,    M6x09_GENERAL },
	{ 0x96, 1, "RORA",  INH,    M6x09_GENERAL },
	{ 0x97, 1, "RORB",  INH,    M6x09_GENERAL },
	{ 0x98, 2, "ROR",   IND,    M6x09_GENERAL },
	{ 0x99, 1, "ASRA",  INH,    M6x09_GENERAL },
	{ 0x9A, 1, "ASRB",  INH,    M6x09_GENERAL },
	{ 0x9B, 2, "ASR",   IND,    M6x09_GENERAL },
	{ 0x9C, 1, "ASLA",  INH,    M6x09_GENERAL },
	{ 0x9D, 1, "ASLB",  INH,    M6x09_GENERAL },
	{ 0x9E, 2, "ASL",   IND,    M6x09_GENERAL },
	{ 0x9F, 1, "RTI",   INH ,   M6x09_GENERAL },

	{ 0xA0, 1, "ROLA",  INH,    M6x09_GENERAL },
	{ 0xA1, 1, "ROLB",  INH,    M6x09_GENERAL },
	{ 0xA2, 2, "ROL",   IND,    M6x09_GENERAL },
	{ 0xA3, 2, "LSRW",  IND,    M6x09_GENERAL },
	{ 0xA4, 2, "RORW",  IND,    M6x09_GENERAL },
	{ 0xA5, 2, "ASRW",  IND,    M6x09_GENERAL },
	{ 0xA6, 2, "ASLW",  IND,    M6x09_GENERAL },
	{ 0xA7, 2, "ROLW",  IND,    M6x09_GENERAL },
	{ 0xA8, 2, "JMP",   IND,    M6x09_GENERAL },
	{ 0xA9, 2, "JSR",   IND,    M6x09_GENERAL, STEP_OVER },
	{ 0xAA, 2, "BSR",   REL,    M6x09_GENERAL, STEP_OVER },
	{ 0xAB, 3, "LBSR",  LREL,   M6x09_GENERAL, STEP_OVER },
	{ 0xAC, 2, "DECB,JNZ",   REL,   M6x09_GENERAL },
	{ 0xAD, 2, "DECX,JNZ",   REL,   M6x09_GENERAL },
	{ 0xAE, 1, "NOP",   INH,    M6x09_GENERAL },

	{ 0xB0, 1, "ABX",   INH,    M6x09_GENERAL },
	{ 0xB1, 1, "DAA",   INH,    M6x09_GENERAL },
	{ 0xB2, 1, "SEX",   INH,    M6x09_GENERAL },
	{ 0xB3, 1, "MUL",   INH,    M6x09_GENERAL },
	{ 0xB4, 1, "LMUL",   INH,   M6x09_GENERAL },
	{ 0xB5, 1, "DIV X,B",   INH,    M6x09_GENERAL },
	{ 0xB6, 1, "BMOVE Y,X,U", INH , M6x09_GENERAL },
	{ 0xB7, 1, "MOVE Y,X,U", INH ,  M6x09_GENERAL },
	{ 0xB8, 2, "LSRD",   IMM,   M6x09_GENERAL },
	{ 0xB9, 2, "LSRD",   IND,   M6x09_GENERAL },
	{ 0xBA, 2, "RORD",   IMM,   M6x09_GENERAL },
	{ 0xBB, 2, "RORD",   IND,   M6x09_GENERAL },
	{ 0xBC, 2, "ASRD",   IMM,   M6x09_GENERAL },
	{ 0xBD, 2, "ASRD",   IND,   M6x09_GENERAL },
	{ 0xBE, 2, "ASLD",   IMM,   M6x09_GENERAL },
	{ 0xBF, 2, "ASLD",   IND,   M6x09_GENERAL },

	{ 0xC0, 2, "ROLD",   IMM,   M6x09_GENERAL },
	{ 0xC1, 2, "ROLD",   IND,   M6x09_GENERAL },
	{ 0xC2, 1, "CLRD",   INH,   M6x09_GENERAL },
	{ 0xC3, 2, "CLRW",   IND,   M6x09_GENERAL },
	{ 0xC4, 1, "NEGD",   INH,   M6x09_GENERAL },
	{ 0xC5, 2, "NEGW",   IND,   M6x09_GENERAL },
	{ 0xC6, 1, "INCD",   INH,   M6x09_GENERAL },
	{ 0xC7, 2, "INCW",   IND,   M6x09_GENERAL },
	{ 0xC8, 1, "DECD",   INH,   M6x09_GENERAL },
	{ 0xC9, 2, "DECW",   IND,   M6x09_GENERAL },
	{ 0xCA, 1, "TSTD",   INH,   M6x09_GENERAL },
	{ 0xCB, 2, "TSTW",   IND,   M6x09_GENERAL },
	{ 0xCC, 1, "ABSA",   INH,   M6x09_GENERAL },
	{ 0xCD, 1, "ABSB",   INH,   M6x09_GENERAL },
	{ 0xCE, 1, "ABSD",   INH,   M6x09_GENERAL },
	{ 0xCF, 1, "BSET A,X,U", INH,   M6x09_GENERAL },

	{ 0xD0, 1, "BSET D,X,U", INH,   M6x09_GENERAL }
};


// ======================> konami_disassembler

konami_disassembler::konami_disassembler() : m6x09_base_disassembler(konami_opcodes, ARRAY_LENGTH(konami_opcodes), M6x09_GENERAL)
{
}

//-------------------------------------------------
//  indexed addressing mode for Konami
//-------------------------------------------------

void konami_disassembler::indexed(std::ostream &stream, uint8_t mode, const data_buffer &params, offs_t &p)
{
	static const char index_reg[8][3] =
	{
		"?", /* 0 - extended mode */
		"?", /* 1 */
		"x", /* 2 */
		"y", /* 3 */
		"?", /* 4 - direct page */
		"u", /* 5 */
		"s", /* 6 */
		"pc" /* 7 - pc */
	};

	int idx = (mode >> 4) & 7;
	int type = mode & 0x0f;
	int val;

	// special modes
	if (mode & 0x80)
	{
		if (type & 8)
		{
			// indirect
			switch (type & 7)
			{
			case 0x00: /* register a */
				util::stream_format(stream, "[a,%s]", index_reg[idx]);
				break;

			case 0x01: /* register b */
				util::stream_format(stream, "[b,%s]", index_reg[idx]);
				break;

			case 0x04: /* direct - mode */
				util::stream_format(stream, "[$%02x]", params.r8(p++));
				break;

			case 0x07: /* register d */
				util::stream_format(stream, "[d,%s]", index_reg[idx]);
				break;

			default:
				util::stream_format(stream, "[?,%s]", index_reg[idx]);
				break;
			}
		}
		else
		{
			switch (type & 7)
			{
			case 0x00: /* register a */
				util::stream_format(stream, "a,%s", index_reg[idx]);
				break;

			case 0x01: /* register b */
				util::stream_format(stream, "b,%s", index_reg[idx]);
				break;

			case 0x04: /* direct - mode */
				util::stream_format(stream, "$%02x", params.r8(p++));
				break;

			case 0x07: /* register d */
				util::stream_format(stream, "d,%s", index_reg[idx]);
				break;

			default:
				util::stream_format(stream, "????,%s", index_reg[idx]);
				break;
			}
		}
	}
	else
	{
		if (type & 8)
		{
			// indirect
			switch (type & 7)
			{
			case 0: // auto increment
				util::stream_format(stream, "[,%s+]", index_reg[idx]);
				break;

			case 1: // auto increment double
				util::stream_format(stream, "[,%s++]", index_reg[idx]);
				break;

			case 2: // auto decrement
				util::stream_format(stream, "[,-%s]", index_reg[idx]);
				break;

			case 3: // auto decrement double
				util::stream_format(stream, "[,--%s]", index_reg[idx]);
				break;

			case 4: // post byte offset
				val = params.r8(p++);

				if (val & 0x80)
					util::stream_format(stream, "[#$-%02x,%s]", 0x100 - val, index_reg[idx]);
				else
					util::stream_format(stream, "[#$%02x,%s]", val, index_reg[idx]);
				break;

			case 5: // post word offset
				val = params.r16(p);
				p += 2;

				if (val & 0x8000)
					util::stream_format(stream, "[#$-%04x,%s]", 0x10000 - val, index_reg[idx]);
				else
					util::stream_format(stream, "[#$%04x,%s]", val, index_reg[idx]);
				break;

			case 6: // simple
				util::stream_format(stream, "[,%s]", index_reg[idx]);
				break;

			case 7: // extended
				val = params.r16(p);
				p += 2;

				util::stream_format(stream, "[$%04x]", val);
				break;
			}
		}
		else
		{
			switch (type & 7)
			{
			case 0: // auto increment
				util::stream_format(stream, ",%s+", index_reg[idx]);
				break;

			case 1: // auto increment double
				util::stream_format(stream, ",%s++", index_reg[idx]);
				break;

			case 2: // auto decrement
				util::stream_format(stream, ",-%s", index_reg[idx]);
				break;

			case 3: // auto decrement double
				util::stream_format(stream, ",--%s", index_reg[idx]);
				break;

			case 4: // post byte offset
				val = params.r8(p++);

				if (val & 0x80)
					util::stream_format(stream, "#$-%02x,%s", 0x100 - val, index_reg[idx]);
				else
					util::stream_format(stream, "#$%02x,%s", val, index_reg[idx]);
				break;

			case 5: // post word offset
				val = params.r16(p);
				p += 2;

				if (val & 0x8000)
					util::stream_format(stream, "#$-%04x,%s", 0x10000 - val, index_reg[idx]);
				else
					util::stream_format(stream, "#$%04x,%s", val, index_reg[idx]);
				break;

			case 6: // simple
				util::stream_format(stream, ",%s", index_reg[idx]);
				break;

			case 7: // extended
				val = params.r16(p);
				p += 2;

				util::stream_format(stream, "$%04x", val);
				break;
			}
		}
	}
}


//-------------------------------------------------
//  Konami register/register parameter
//-------------------------------------------------

void konami_disassembler::register_register(std::ostream &stream, uint8_t pb)
{
	static const char konami_teregs[8] =
	{
		'A', 'B', 'X', 'Y', 'S', 'U', '?', '?'
	};
	util::stream_format(stream, "%c,%c",
		konami_teregs[(pb >> 0) & 0x7],
		konami_teregs[(pb >> 4) & 0x7]);
}
