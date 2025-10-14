// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Sean Riddle,Tim Lindner
/*****************************************************************************

    a 6809/6309/Konami opcode disassembler

    Based on:
        6309dasm.c - a 6309 opcode disassembler
        Version 1.0 5-AUG-2000
        Copyright Tim Lindner

    and
        6809dasm.c - a 6809 opcode disassembler
        Version 1.4 1-MAR-95
        Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    TODO:
    - KONAMI EXG/TFR isn't disassembled accurately:
      0x3E/0x3F + param bit 7 clear = EXG
      0x3E/0x3F + param bit 7 set = TFR

*****************************************************************************/

#include "emu.h"
#include "6x09dasm.h"


const char *const m6x09_base_disassembler::m6x09_regs[5] = { "X", "Y", "U", "S", "PC" };

const char *const m6x09_base_disassembler::m6x09_btwregs[5] = { "CC", "A", "B", "inv" };

const char *const m6x09_base_disassembler::hd6309_tfmregs[16] = {
	"D",   "X",   "Y",   "U",   "S",   "inv", "inv", "inv",
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
//  constructor
//-------------------------------------------------

m6x09_base_disassembler::m6x09_base_disassembler(const opcodeinfo *opcodes, size_t opcode_count, uint32_t level)
	: m_level(level), m_page(0)
{
	// create filtered opcode table
	for (int i = 0; i < opcode_count; i++)
	{
		if (opcodes[i].level() & level)
		{
			m_opcodes.insert(opcodes[i]);
		}
	}
}

//-------------------------------------------------
//  transparent comparator support
//-------------------------------------------------

bool m6x09_base_disassembler::opcodeinfo::compare::operator()(opcodeinfo const& lhs, opcodeinfo const& rhs) const
{
	return lhs.opcode() < rhs.opcode();
}

bool m6x09_base_disassembler::opcodeinfo::compare::operator()(uint16_t opcode, opcodeinfo const& rhs) const
{
	return opcode < rhs.opcode();
}

bool m6x09_base_disassembler::opcodeinfo::compare::operator()(opcodeinfo const& lhs, uint16_t opcode) const
{
	return lhs.opcode() < opcode;
}

//-------------------------------------------------
//  fetch_opcode
//-------------------------------------------------

const m6x09_base_disassembler::opcodeinfo *m6x09_base_disassembler::fetch_opcode(const data_buffer &opcodes, offs_t &p)
{
	uint8_t page_count = 0;
	const opcodeinfo *op = nullptr;

	while(!op)
	{
		// retrieve the opcode
		uint16_t opcode = m_page | opcodes.r8(p++);

		// perform the lookup
		auto iter = m_opcodes.find(opcode);

		// did we find something?
		if (iter == m_opcodes.end())
		{
			// on the 6809 an unimplemented page 2 or 3 opcodes fall through to the first page.
			if (!(m_level & HD6309_EXCLUSIVE) && (m_page != 0))
			{
				// backup the opcode pointer and disassemble the bare opcode.
				--p;
				m_page = 0;
			}
			else
				// nothing to disassemble.
				return nullptr;
		}
		else
		{
			// was this a $10 or $11 page?
			switch (iter->mode())
			{
			case PG2:
			case PG3:
				if (m_page != 0 && m_level & HD6309_EXCLUSIVE)
				{
					// multiple pages are illegal on the 6309
					m_page = 0;
					return nullptr;
				}
				else if (m_page == 0)
				{
					// remember the page that comes first
					page_count++;
					m_page = iter->opcode() << 8;
				}
				else
				{
					// output single page opcode after a two pages
					if (page_count++)
					{
						--p;
						return nullptr;
					}
				}
				break;

			default:
				op = &*iter;
				m_page = 0;
				break;
			}
		}
	};

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
		util::stream_format(stream, "%-7s$%02X", "FCB", opcodes.r8(pc));
		for (offs_t q = pc + 1; q < p; q++)
			util::stream_format(stream, ",$%02X", opcodes.r8(q));
		return (p - pc) | SUPPORTED;
	}

	offs_t flags = op->flags();

	offs_t ppc = p;
	p += op->operand_length();

	// output the base instruction name
	if (op->mode() == INH)
		stream << op->name();
	else
		util::stream_format(stream, "%-7s", op->name());

	switch (op->mode())
	{
	case PG2:
	case PG3:
	case INH:
		// No operands
		break;

	case PSHS:
	case PSHU:
		pb = params.r8(ppc);
		if (pb & 0x80)
			util::stream_format(stream, "PC");
		if (pb & 0x40)
			util::stream_format(stream, "%s%s", (pb & 0x80) ? "," : "", (op->mode() == PSHS) ? "U" : "S");
		if (pb & 0x20)
			util::stream_format(stream, "%sY",  (pb & 0xc0) ? "," : "");
		if (pb & 0x10)
			util::stream_format(stream, "%sX",  (pb & 0xe0) ? "," : "");
		if (pb & 0x08)
			util::stream_format(stream, "%sDP", (pb & 0xf0) ? "," : "");
		if (pb & 0x04)
			util::stream_format(stream, "%s%s", (pb & 0xf8) ? "," : "", (pb & 0x02) ? "D" : "B");
		else if (pb & 0x02)
			util::stream_format(stream, "%sA",  (pb & 0xfc) ? "," : "");
		if (pb & 0x01)
			util::stream_format(stream, "%sCC", (pb & 0xfe) ? "," : "");
		break;

	case PULS:
	case PULU:
		pb = params.r8(ppc);
		if (pb & 0x01)
			util::stream_format(stream, "CC");
		if (pb & 0x02)
			util::stream_format(stream, "%s%s", (pb & 0x01) ? "," : "", (pb & 0x04) ? "D" : "A");
		else if (pb & 0x04)
			util::stream_format(stream, "%sB",  (pb & 0x03) ? "," : "");
		if (pb & 0x08)
			util::stream_format(stream, "%sDP", (pb & 0x07) ? "," : "");
		if (pb & 0x10)
			util::stream_format(stream, "%sX",  (pb & 0x0f) ? "," : "");
		if (pb & 0x20)
			util::stream_format(stream, "%sY",  (pb & 0x1f) ? "," : "");
		if (pb & 0x40)
			util::stream_format(stream, "%s%s", (pb & 0x3f) ? "," : "", (op->mode() == PULS) ? "U" : "S");
		if (pb & 0x80)
		{
			util::stream_format(stream, "%sPC", (pb & 0x7f) ? "," : "");
			flags |= STEP_OUT;
		}
		break;

	case DIR:
		ea = params.r8(ppc);
		util::stream_format(stream, "<$%02X", ea);
		break;

	case DIR_IM:
		assert_hd6309_exclusive();
		util::stream_format(stream, "#$%02X;", params.r8(ppc));
		util::stream_format(stream, "<$%02X", params.r8(ppc + 1));
		break;

	case REL:
		offset = (int8_t)params.r8(ppc);
		util::stream_format(stream, "$%04X", (p + offset) & 0xffff);
		break;

	case LREL:
		offset = (int16_t)params.r16(ppc);
		util::stream_format(stream, "$%04X", (p + offset) & 0xffff);
		break;

	case EXT:
		if (op->operand_length() == 3)
		{
			assert_hd6309_exclusive();
			pb = params.r8(ppc);
			ea = params.r16(ppc+1);
			util::stream_format(stream, "#$%02X,$%04X", pb, ea);
		}
		else if (op->operand_length() == 2)
		{
			ea = params.r16(ppc);
			if (!(ea & 0xff00))
			{
				stream << '>'; // need the '>' to force an assembler to use EXT addressing
			}
			util::stream_format(stream, "$%04X", ea);
		}
		break;

	case IND:
		if (op->operand_length() == 2)
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
		if (op->operand_length() == 4)
		{
			ea = params.r32(ppc);
			util::stream_format(stream, "#$%08X", ea);
		}
		else
		if (op->operand_length() == 2)
		{
			ea = params.r16(ppc);
			util::stream_format(stream, "#$%04X", ea);
		}
		else
		if (op->operand_length() == 1)
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

	return (p - pc) | flags | SUPPORTED;
}


//**************************************************************************
//  M6809/HD6309 disassembler
//**************************************************************************

const m6x09_base_disassembler::opcodeinfo m6x09_disassembler::m6x09_opcodes[] =
{
	// Page 1 opcodes (single byte)
	{ 0x00, 1, "NEG",   DIR,    M6x09_GENERAL },
	{ 0x01, 2, "OIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x01, 1, "NEG",   DIR,    M6809_UNDOCUMENTED },
	{ 0x02, 2, "AIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x02, 1, "XNC",   DIR,    M6809_UNDOCUMENTED },
	{ 0x03, 1, "COM",   DIR,    M6x09_GENERAL },
	{ 0x04, 1, "LSR",   DIR,    M6x09_GENERAL },
	{ 0x05, 2, "EIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x05, 1, "LSR",   DIR,    M6809_UNDOCUMENTED },
	{ 0x06, 1, "ROR",   DIR,    M6x09_GENERAL },
	{ 0x07, 1, "ASR",   DIR,    M6x09_GENERAL },
	{ 0x08, 1, "ASL",   DIR,    M6x09_GENERAL },
	{ 0x09, 1, "ROL",   DIR,    M6x09_GENERAL },
	{ 0x0A, 1, "DEC",   DIR,    M6x09_GENERAL },
	{ 0x0B, 2, "TIM",   DIR_IM, HD6309_EXCLUSIVE },
	{ 0x0B, 1, "XDEC",  DIR,    M6809_UNDOCUMENTED },
	{ 0x0C, 1, "INC",   DIR,    M6x09_GENERAL },
	{ 0x0D, 1, "TST",   DIR,    M6x09_GENERAL },
	{ 0x0E, 1, "JMP",   DIR,    M6x09_GENERAL },
	{ 0x0F, 1, "CLR",   DIR,    M6x09_GENERAL },

	{ 0x10, 0, "PAGE2", PG2,    M6x09_GENERAL },
	{ 0x11, 0, "PAGE3", PG3,    M6x09_GENERAL },
	{ 0x12, 0, "NOP",   INH,    M6x09_GENERAL },
	{ 0x13, 0, "SYNC",  INH,    M6x09_GENERAL },
	{ 0x14, 0, "SEXW",  INH,    HD6309_EXCLUSIVE },
	{ 0x14, 0, "XHCF",  INH,    M6809_UNDOCUMENTED },
	{ 0x15, 0, "XHCF",  INH,    M6809_UNDOCUMENTED },
	{ 0x16, 2, "LBRA",  LREL,   M6x09_GENERAL },
	{ 0x17, 2, "LBSR",  LREL,   M6x09_GENERAL, STEP_OVER },
	{ 0x18, 0, "X18",   INH,    M6809_UNDOCUMENTED },
	{ 0x19, 0, "DAA",   INH,    M6x09_GENERAL },
	{ 0x1A, 1, "ORCC",  IMM,    M6x09_GENERAL },
	{ 0x1B, 0, "NOP",   INH,    M6809_UNDOCUMENTED },
	{ 0x1C, 1, "ANDCC", IMM,    M6x09_GENERAL },
	{ 0x1D, 0, "SEX",   INH,    M6x09_GENERAL },
	{ 0x1E, 1, "EXG",   IMM_RR, M6x09_GENERAL },
	{ 0x1F, 1, "TFR",   IMM_RR, M6x09_GENERAL },

	{ 0x20, 1, "BRA",   REL,    M6x09_GENERAL },
	{ 0x21, 1, "BRN",   REL,    M6x09_GENERAL },
	{ 0x22, 1, "BHI",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x23, 1, "BLS",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x24, 1, "BCC",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x25, 1, "BCS",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x26, 1, "BNE",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x27, 1, "BEQ",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x28, 1, "BVC",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x29, 1, "BVS",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x2A, 1, "BPL",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x2B, 1, "BMI",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x2C, 1, "BGE",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x2D, 1, "BLT",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x2E, 1, "BGT",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x2F, 1, "BLE",   REL,    M6x09_GENERAL, STEP_COND },

	{ 0x30, 1, "LEAX",   IND,   M6x09_GENERAL },
	{ 0x31, 1, "LEAY",   IND,   M6x09_GENERAL },
	{ 0x32, 1, "LEAS",   IND,   M6x09_GENERAL },
	{ 0x33, 1, "LEAU",   IND,   M6x09_GENERAL },
	{ 0x34, 1, "PSHS",   PSHS,  M6x09_GENERAL },
	{ 0x35, 1, "PULS",   PULS,  M6x09_GENERAL },
	{ 0x36, 1, "PSHU",   PSHU,  M6x09_GENERAL },
	{ 0x37, 1, "PULU",   PULU,  M6x09_GENERAL },
	{ 0x38, 1, "XANDCC", IMM,   M6809_UNDOCUMENTED },
	{ 0x39, 0, "RTS",    INH,   M6x09_GENERAL, STEP_OUT },
	{ 0x3A, 0, "ABX",    INH,   M6x09_GENERAL },
	{ 0x3B, 0, "RTI",    INH,   M6x09_GENERAL, STEP_OUT },
	{ 0x3C, 1, "CWAI",   IMM,   M6x09_GENERAL },
	{ 0x3D, 0, "MUL",    INH,   M6x09_GENERAL },
	{ 0x3E, 0, "XRES",   INH,   M6809_UNDOCUMENTED },
	{ 0x3F, 0, "SWI",    INH,   M6x09_GENERAL },

	{ 0x40, 0, "NEGA",  INH,    M6x09_GENERAL },
	{ 0x41, 0, "NEGA",  INH,    M6809_UNDOCUMENTED },
	{ 0x42, 0, "XNCA",  INH,    M6809_UNDOCUMENTED },
	{ 0x43, 0, "COMA",  INH,    M6x09_GENERAL },
	{ 0x44, 0, "LSRA",  INH,    M6x09_GENERAL },
	{ 0x45, 0, "LSRA",  INH,    M6809_UNDOCUMENTED },
	{ 0x46, 0, "RORA",  INH,    M6x09_GENERAL },
	{ 0x47, 0, "ASRA",  INH,    M6x09_GENERAL },
	{ 0x48, 0, "ASLA",  INH,    M6x09_GENERAL },
	{ 0x49, 0, "ROLA",  INH,    M6x09_GENERAL },
	{ 0x4A, 0, "DECA",  INH,    M6x09_GENERAL },
	{ 0x4B, 0, "XDECA", INH,    M6809_UNDOCUMENTED },
	{ 0x4C, 0, "INCA",  INH,    M6x09_GENERAL },
	{ 0x4D, 0, "TSTA",  INH,    M6x09_GENERAL },
	{ 0x4E, 0, "XCLRA", INH,    M6809_UNDOCUMENTED },
	{ 0x4F, 0, "CLRA",  INH,    M6x09_GENERAL },

	{ 0x50, 0, "NEGB",  INH,    M6x09_GENERAL },
	{ 0x51, 0, "NEGB",  INH,    M6809_UNDOCUMENTED },
	{ 0x52, 0, "XNCB",  INH,    M6809_UNDOCUMENTED },
	{ 0x53, 0, "COMB",  INH,    M6x09_GENERAL },
	{ 0x54, 0, "LSRB",  INH,    M6x09_GENERAL },
	{ 0x55, 0, "LSRB",  INH,    M6809_UNDOCUMENTED },
	{ 0x56, 0, "RORB",  INH,    M6x09_GENERAL },
	{ 0x57, 0, "ASRB",  INH,    M6x09_GENERAL },
	{ 0x58, 0, "ASLB",  INH,    M6x09_GENERAL },
	{ 0x59, 0, "ROLB",  INH,    M6x09_GENERAL },
	{ 0x5A, 0, "DECB",  INH,    M6x09_GENERAL },
	{ 0x5B, 0, "XDECB", INH,    M6809_UNDOCUMENTED },
	{ 0x5C, 0, "INCB",  INH,    M6x09_GENERAL },
	{ 0x5D, 0, "TSTB",  INH,    M6x09_GENERAL },
	{ 0x5E, 0, "XCLRB", INH,    M6809_UNDOCUMENTED },
	{ 0x5F, 0, "CLRB",  INH,    M6x09_GENERAL },

	{ 0x60, 1, "NEG",   IND,    M6x09_GENERAL },
	{ 0x61, 2, "OIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x61, 1, "NEG",   IND,    M6809_UNDOCUMENTED },
	{ 0x62, 2, "AIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x62, 1, "XNC",   IND,    M6809_UNDOCUMENTED },
	{ 0x63, 1, "COM",   IND,    M6x09_GENERAL },
	{ 0x64, 1, "LSR",   IND,    M6x09_GENERAL },
	{ 0x65, 2, "EIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x65, 1, "LSR",   IND,    M6809_UNDOCUMENTED },
	{ 0x66, 1, "ROR",   IND,    M6x09_GENERAL },
	{ 0x67, 1, "ASR",   IND,    M6x09_GENERAL },
	{ 0x68, 1, "ASL",   IND,    M6x09_GENERAL },
	{ 0x69, 1, "ROL",   IND,    M6x09_GENERAL },
	{ 0x6A, 1, "DEC",   IND,    M6x09_GENERAL },
	{ 0x6B, 2, "TIM",   IND,    HD6309_EXCLUSIVE },
	{ 0x6B, 1, "XDEC",  IND,    M6809_UNDOCUMENTED },
	{ 0x6C, 1, "INC",   IND,    M6x09_GENERAL },
	{ 0x6D, 1, "TST",   IND,    M6x09_GENERAL },
	{ 0x6E, 1, "JMP",   IND,    M6x09_GENERAL },
	{ 0x6F, 1, "CLR",   IND,    M6x09_GENERAL },

	{ 0x70, 2, "NEG",   EXT,    M6x09_GENERAL },
	{ 0x71, 3, "OIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x71, 2, "NEG",   EXT,    M6809_UNDOCUMENTED },
	{ 0x72, 3, "AIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x72, 2, "XNC",   EXT,    M6809_UNDOCUMENTED },
	{ 0x73, 2, "COM",   EXT,    M6x09_GENERAL },
	{ 0x74, 2, "LSR",   EXT,    M6x09_GENERAL },
	{ 0x75, 3, "EIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x75, 2, "LSR",   EXT,    M6809_UNDOCUMENTED },
	{ 0x76, 2, "ROR",   EXT,    M6x09_GENERAL },
	{ 0x77, 2, "ASR",   EXT,    M6x09_GENERAL },
	{ 0x78, 2, "ASL",   EXT,    M6x09_GENERAL },
	{ 0x79, 2, "ROL",   EXT,    M6x09_GENERAL },
	{ 0x7A, 2, "DEC",   EXT,    M6x09_GENERAL },
	{ 0x7B, 3, "TIM",   EXT,    HD6309_EXCLUSIVE },
	{ 0x7B, 2, "XDEC",  EXT,    M6809_UNDOCUMENTED },
	{ 0x7C, 2, "INC",   EXT,    M6x09_GENERAL },
	{ 0x7D, 2, "TST",   EXT,    M6x09_GENERAL },
	{ 0x7E, 2, "JMP",   EXT,    M6x09_GENERAL },
	{ 0x7F, 2, "CLR",   EXT,    M6x09_GENERAL },

	{ 0x80, 1, "SUBA",  IMM,    M6x09_GENERAL },
	{ 0x81, 1, "CMPA",  IMM,    M6x09_GENERAL },
	{ 0x82, 1, "SBCA",  IMM,    M6x09_GENERAL },
	{ 0x83, 2, "SUBD",  IMM,    M6x09_GENERAL },
	{ 0x84, 1, "ANDA",  IMM,    M6x09_GENERAL },
	{ 0x85, 1, "BITA",  IMM,    M6x09_GENERAL },
	{ 0x86, 1, "LDA",   IMM,    M6x09_GENERAL },
	{ 0x87, 1, "XSTA",  IMM,    M6809_UNDOCUMENTED },
	{ 0x88, 1, "EORA",  IMM,    M6x09_GENERAL },
	{ 0x89, 1, "ADCA",  IMM,    M6x09_GENERAL },
	{ 0x8A, 1, "ORA",   IMM,    M6x09_GENERAL },
	{ 0x8B, 1, "ADDA",  IMM,    M6x09_GENERAL },
	{ 0x8C, 2, "CMPX",  IMM,    M6x09_GENERAL },
	{ 0x8D, 1, "BSR",   REL,    M6x09_GENERAL, STEP_OVER },
	{ 0x8E, 2, "LDX",   IMM,    M6x09_GENERAL },
	{ 0x8F, 2, "XSTX",  IMM,    M6809_UNDOCUMENTED },

	{ 0x90, 1, "SUBA",  DIR,    M6x09_GENERAL },
	{ 0x91, 1, "CMPA",  DIR,    M6x09_GENERAL },
	{ 0x92, 1, "SBCA",  DIR,    M6x09_GENERAL },
	{ 0x93, 1, "SUBD",  DIR,    M6x09_GENERAL },
	{ 0x94, 1, "ANDA",  DIR,    M6x09_GENERAL },
	{ 0x95, 1, "BITA",  DIR,    M6x09_GENERAL },
	{ 0x96, 1, "LDA",   DIR,    M6x09_GENERAL },
	{ 0x97, 1, "STA",   DIR,    M6x09_GENERAL },
	{ 0x98, 1, "EORA",  DIR,    M6x09_GENERAL },
	{ 0x99, 1, "ADCA",  DIR,    M6x09_GENERAL },
	{ 0x9A, 1, "ORA",   DIR,    M6x09_GENERAL },
	{ 0x9B, 1, "ADDA",  DIR,    M6x09_GENERAL },
	{ 0x9C, 1, "CMPX",  DIR,    M6x09_GENERAL },
	{ 0x9D, 1, "JSR",   DIR,    M6x09_GENERAL, STEP_OVER },
	{ 0x9E, 1, "LDX",   DIR,    M6x09_GENERAL },
	{ 0x9F, 1, "STX",   DIR,    M6x09_GENERAL },

	{ 0xA0, 1, "SUBA",  IND,    M6x09_GENERAL },
	{ 0xA1, 1, "CMPA",  IND,    M6x09_GENERAL },
	{ 0xA2, 1, "SBCA",  IND,    M6x09_GENERAL },
	{ 0xA3, 1, "SUBD",  IND,    M6x09_GENERAL },
	{ 0xA4, 1, "ANDA",  IND,    M6x09_GENERAL },
	{ 0xA5, 1, "BITA",  IND,    M6x09_GENERAL },
	{ 0xA6, 1, "LDA",   IND,    M6x09_GENERAL },
	{ 0xA7, 1, "STA",   IND,    M6x09_GENERAL },
	{ 0xA8, 1, "EORA",  IND,    M6x09_GENERAL },
	{ 0xA9, 1, "ADCA",  IND,    M6x09_GENERAL },
	{ 0xAA, 1, "ORA",   IND,    M6x09_GENERAL },
	{ 0xAB, 1, "ADDA",  IND,    M6x09_GENERAL },
	{ 0xAC, 1, "CMPX",  IND,    M6x09_GENERAL },
	{ 0xAD, 1, "JSR",   IND,    M6x09_GENERAL, STEP_OVER },
	{ 0xAE, 1, "LDX",   IND,    M6x09_GENERAL },
	{ 0xAF, 1, "STX",   IND,    M6x09_GENERAL },

	{ 0xB0, 2, "SUBA",  EXT,    M6x09_GENERAL },
	{ 0xB1, 2, "CMPA",  EXT,    M6x09_GENERAL },
	{ 0xB2, 2, "SBCA",  EXT,    M6x09_GENERAL },
	{ 0xB3, 2, "SUBD",  EXT,    M6x09_GENERAL },
	{ 0xB4, 2, "ANDA",  EXT,    M6x09_GENERAL },
	{ 0xB5, 2, "BITA",  EXT,    M6x09_GENERAL },
	{ 0xB6, 2, "LDA",   EXT,    M6x09_GENERAL },
	{ 0xB7, 2, "STA",   EXT,    M6x09_GENERAL },
	{ 0xB8, 2, "EORA",  EXT,    M6x09_GENERAL },
	{ 0xB9, 2, "ADCA",  EXT,    M6x09_GENERAL },
	{ 0xBA, 2, "ORA",   EXT,    M6x09_GENERAL },
	{ 0xBB, 2, "ADDA",  EXT,    M6x09_GENERAL },
	{ 0xBC, 2, "CMPX",  EXT,    M6x09_GENERAL },
	{ 0xBD, 2, "JSR",   EXT,    M6x09_GENERAL, STEP_OVER },
	{ 0xBE, 2, "LDX",   EXT,    M6x09_GENERAL },
	{ 0xBF, 2, "STX",   EXT,    M6x09_GENERAL },

	{ 0xC0, 1, "SUBB",  IMM,    M6x09_GENERAL },
	{ 0xC1, 1, "CMPB",  IMM,    M6x09_GENERAL },
	{ 0xC2, 1, "SBCB",  IMM,    M6x09_GENERAL },
	{ 0xC3, 2, "ADDD",  IMM,    M6x09_GENERAL },
	{ 0xC4, 1, "ANDB",  IMM,    M6x09_GENERAL },
	{ 0xC5, 1, "BITB",  IMM,    M6x09_GENERAL },
	{ 0xC6, 1, "LDB",   IMM,    M6x09_GENERAL },
	{ 0xC7, 1, "XSTB",  IMM,    M6809_UNDOCUMENTED },
	{ 0xC8, 1, "EORB",  IMM,    M6x09_GENERAL },
	{ 0xC9, 1, "ADCB",  IMM,    M6x09_GENERAL },
	{ 0xCA, 1, "ORB",   IMM,    M6x09_GENERAL },
	{ 0xCB, 1, "ADDB",  IMM,    M6x09_GENERAL },
	{ 0xCC, 2, "LDD",   IMM,    M6x09_GENERAL },
	{ 0xCD, 4, "LDQ",   IMM,    HD6309_EXCLUSIVE },
	{ 0xCD, 0, "XHCF",  INH,    M6809_UNDOCUMENTED },
	{ 0xCE, 2, "LDU",   IMM,    M6x09_GENERAL },
	{ 0xCF, 2, "XSTU",  IMM,    M6809_UNDOCUMENTED },

	{ 0xD0, 1, "SUBB",  DIR,    M6x09_GENERAL },
	{ 0xD1, 1, "CMPB",  DIR,    M6x09_GENERAL },
	{ 0xD2, 1, "SBCB",  DIR,    M6x09_GENERAL },
	{ 0xD3, 1, "ADDD",  DIR,    M6x09_GENERAL },
	{ 0xD4, 1, "ANDB",  DIR,    M6x09_GENERAL },
	{ 0xD5, 1, "BITB",  DIR,    M6x09_GENERAL },
	{ 0xD6, 1, "LDB",   DIR,    M6x09_GENERAL },
	{ 0xD7, 1, "STB",   DIR,    M6x09_GENERAL },
	{ 0xD8, 1, "EORB",  DIR,    M6x09_GENERAL },
	{ 0xD9, 1, "ADCB",  DIR,    M6x09_GENERAL },
	{ 0xDA, 1, "ORB",   DIR,    M6x09_GENERAL },
	{ 0xDB, 1, "ADDB",  DIR,    M6x09_GENERAL },
	{ 0xDC, 1, "LDD",   DIR,    M6x09_GENERAL },
	{ 0xDD, 1, "STD",   DIR,    M6x09_GENERAL },
	{ 0xDE, 1, "LDU",   DIR,    M6x09_GENERAL },
	{ 0xDF, 1, "STU",   DIR,    M6x09_GENERAL },

	{ 0xE0, 1, "SUBB",  IND,    M6x09_GENERAL },
	{ 0xE1, 1, "CMPB",  IND,    M6x09_GENERAL },
	{ 0xE2, 1, "SBCB",  IND,    M6x09_GENERAL },
	{ 0xE3, 1, "ADDD",  IND,    M6x09_GENERAL },
	{ 0xE4, 1, "ANDB",  IND,    M6x09_GENERAL },
	{ 0xE5, 1, "BITB",  IND,    M6x09_GENERAL },
	{ 0xE6, 1, "LDB",   IND,    M6x09_GENERAL },
	{ 0xE7, 1, "STB",   IND,    M6x09_GENERAL },
	{ 0xE8, 1, "EORB",  IND,    M6x09_GENERAL },
	{ 0xE9, 1, "ADCB",  IND,    M6x09_GENERAL },
	{ 0xEA, 1, "ORB",   IND,    M6x09_GENERAL },
	{ 0xEB, 1, "ADDB",  IND,    M6x09_GENERAL },
	{ 0xEC, 1, "LDD",   IND,    M6x09_GENERAL },
	{ 0xED, 1, "STD",   IND,    M6x09_GENERAL },
	{ 0xEE, 1, "LDU",   IND,    M6x09_GENERAL },
	{ 0xEF, 1, "STU",   IND,    M6x09_GENERAL },

	{ 0xF0, 2, "SUBB",  EXT,    M6x09_GENERAL },
	{ 0xF1, 2, "CMPB",  EXT,    M6x09_GENERAL },
	{ 0xF2, 2, "SBCB",  EXT,    M6x09_GENERAL },
	{ 0xF3, 2, "ADDD",  EXT,    M6x09_GENERAL },
	{ 0xF4, 2, "ANDB",  EXT,    M6x09_GENERAL },
	{ 0xF5, 2, "BITB",  EXT,    M6x09_GENERAL },
	{ 0xF6, 2, "LDB",   EXT,    M6x09_GENERAL },
	{ 0xF7, 2, "STB",   EXT,    M6x09_GENERAL },
	{ 0xF8, 2, "EORB",  EXT,    M6x09_GENERAL },
	{ 0xF9, 2, "ADCB",  EXT,    M6x09_GENERAL },
	{ 0xFA, 2, "ORB",   EXT,    M6x09_GENERAL },
	{ 0xFB, 2, "ADDB",  EXT,    M6x09_GENERAL },
	{ 0xFC, 2, "LDD",   EXT,    M6x09_GENERAL },
	{ 0xFD, 2, "STD",   EXT,    M6x09_GENERAL },
	{ 0xFE, 2, "LDU",   EXT,    M6x09_GENERAL },
	{ 0xFF, 2, "STU",   EXT,    M6x09_GENERAL },

	// Page 2 opcodes (0x10 0x..)
	{ 0x1010, 0, "PAGE2", PG2,  M6x09_GENERAL },
	{ 0x1011, 0, "PAGE3", PG3,  M6x09_GENERAL },

	{ 0x1020, 2, "XLBRA", LREL, M6809_UNDOCUMENTED },
	{ 0x1021, 2, "LBRN",  LREL, M6x09_GENERAL },
	{ 0x1022, 2, "LBHI",  LREL, M6x09_GENERAL },
	{ 0x1023, 2, "LBLS",  LREL, M6x09_GENERAL },
	{ 0x1024, 2, "LBCC",  LREL, M6x09_GENERAL },
	{ 0x1025, 2, "LBCS",  LREL, M6x09_GENERAL },
	{ 0x1026, 2, "LBNE",  LREL, M6x09_GENERAL },
	{ 0x1027, 2, "LBEQ",  LREL, M6x09_GENERAL },
	{ 0x1028, 2, "LBVC",  LREL, M6x09_GENERAL },
	{ 0x1029, 2, "LBVS",  LREL, M6x09_GENERAL },
	{ 0x102A, 2, "LBPL",  LREL, M6x09_GENERAL },
	{ 0x102B, 2, "LBMI",  LREL, M6x09_GENERAL },
	{ 0x102C, 2, "LBGE",  LREL, M6x09_GENERAL },
	{ 0x102D, 2, "LBLT",  LREL, M6x09_GENERAL },
	{ 0x102E, 2, "LBGT",  LREL, M6x09_GENERAL },
	{ 0x102F, 2, "LBLE",  LREL, M6x09_GENERAL },

	{ 0x1030, 1, "ADDR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1031, 1, "ADCR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1032, 1, "SUBR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1033, 1, "SBCR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1034, 1, "ANDR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1035, 1, "ORR",   IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1036, 1, "EORR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1037, 1, "CMPR",  IMM_RR, HD6309_EXCLUSIVE },
	{ 0x1038, 0, "PSHSW", INH,    HD6309_EXCLUSIVE },
	{ 0x1039, 0, "PULSW", INH,    HD6309_EXCLUSIVE },
	{ 0x103A, 0, "PSHUW", INH,    HD6309_EXCLUSIVE },
	{ 0x103B, 0, "PULUW", INH,    HD6309_EXCLUSIVE },
	{ 0x103E, 0, "XSWI2", INH,    M6809_UNDOCUMENTED },
	{ 0x103F, 0, "SWI2",  INH,    M6x09_GENERAL },

	{ 0x1040, 0, "NEGD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1043, 0, "COMD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1044, 0, "LSRD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1046, 0, "RORD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1047, 0, "ASRD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1048, 0, "ASLD",  INH,  HD6309_EXCLUSIVE },
	{ 0x1049, 0, "ROLD",  INH,  HD6309_EXCLUSIVE },

	{ 0x104A, 0, "DECD",  INH,  HD6309_EXCLUSIVE },
	{ 0x104C, 0, "INCD",  INH,  HD6309_EXCLUSIVE },
	{ 0x104D, 0, "TSTD",  INH,  HD6309_EXCLUSIVE },
	{ 0x104F, 0, "CLRD",  INH,  HD6309_EXCLUSIVE },

	{ 0x1053, 0, "COMW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1054, 0, "LSRW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1056, 0, "RORW",  INH,  HD6309_EXCLUSIVE },
	{ 0x1059, 0, "ROLW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105A, 0, "DECW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105C, 0, "INCW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105D, 0, "TSTW",  INH,  HD6309_EXCLUSIVE },
	{ 0x105F, 0, "CLRW",  INH,  HD6309_EXCLUSIVE },

	{ 0x1080, 2, "SUBW",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1081, 2, "CMPW",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1082, 2, "SBCD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1083, 2, "CMPD",  IMM,  M6x09_GENERAL },
	{ 0x1084, 2, "ANDD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1085, 2, "BITD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1086, 2, "LDW",   IMM,  HD6309_EXCLUSIVE },
	{ 0x1087, 1, "XSTA",  IMM,  M6809_UNDOCUMENTED },
	{ 0x1088, 2, "EORD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1089, 2, "ADCD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x108A, 2, "ORD",   IMM,  HD6309_EXCLUSIVE },
	{ 0x108B, 2, "ADDW",  IMM,  HD6309_EXCLUSIVE },
	{ 0x108C, 2, "CMPY",  IMM,  M6x09_GENERAL },
	{ 0x108E, 2, "LDY",   IMM,  M6x09_GENERAL },
	{ 0x108F, 2, "XSTY",  IMM,  M6809_UNDOCUMENTED },

	{ 0x1090, 1, "SUBW",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1091, 1, "CMPW",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1092, 1, "SBCD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1093, 1, "CMPD",  DIR,  M6x09_GENERAL },
	{ 0x1094, 1, "ANDD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1095, 1, "BITD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1096, 1, "LDW",   DIR,  HD6309_EXCLUSIVE },
	{ 0x1097, 1, "STW",   DIR,  HD6309_EXCLUSIVE },
	{ 0x1098, 1, "EORD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1099, 1, "ADCD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x109A, 1, "ORD",   DIR,  HD6309_EXCLUSIVE },
	{ 0x109B, 1, "ADDW",  DIR,  HD6309_EXCLUSIVE },
	{ 0x109C, 1, "CMPY",  DIR,  M6x09_GENERAL },
	{ 0x109E, 1, "LDY",   DIR,  M6x09_GENERAL },
	{ 0x109F, 1, "STY",   DIR,  M6x09_GENERAL },

	{ 0x10A0, 1, "SUBW",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A1, 1, "CMPW",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A2, 1, "SBCD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A3, 1, "CMPD",  IND,  M6x09_GENERAL },
	{ 0x10A4, 1, "ANDD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A5, 1, "BITD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A6, 1, "LDW",   IND,  HD6309_EXCLUSIVE },
	{ 0x10A7, 1, "STW",   IND,  HD6309_EXCLUSIVE },
	{ 0x10A8, 1, "EORD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10A9, 1, "ADCD",  IND,  HD6309_EXCLUSIVE },
	{ 0x10AA, 1, "ORD",   IND,  HD6309_EXCLUSIVE },
	{ 0x10AB, 1, "ADDW",  IND,  HD6309_EXCLUSIVE },
	{ 0x10AC, 1, "CMPY",  IND,  M6x09_GENERAL },
	{ 0x10AE, 1, "LDY",   IND,  M6x09_GENERAL },
	{ 0x10AF, 1, "STY",   IND,  M6x09_GENERAL },

	{ 0x10B0, 2, "SUBW",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B1, 2, "CMPW",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B2, 2, "SBCD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B3, 2, "CMPD",  EXT,  M6x09_GENERAL },
	{ 0x10B4, 2, "ANDD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B5, 2, "BITD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B6, 2, "LDW",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10B7, 2, "STW",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10B8, 2, "EORD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10B9, 2, "ADCD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10BA, 2, "ORD",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10BB, 2, "ADDW",  EXT,  HD6309_EXCLUSIVE },
	{ 0x10BC, 2, "CMPY",  EXT,  M6x09_GENERAL },
	{ 0x10BE, 2, "LDY",   EXT,  M6x09_GENERAL },
	{ 0x10BF, 2, "STY",   EXT,  M6x09_GENERAL },

	{ 0x10C3, 2, "XADDD", IMM,  M6809_UNDOCUMENTED },
	{ 0x10C7, 1, "XSTB",  IMM,  M6809_UNDOCUMENTED },
	{ 0x10CE, 2, "LDS",   IMM,  M6x09_GENERAL },
	{ 0x10CF, 2, "XSTS",  IMM,  M6809_UNDOCUMENTED },

	{ 0x10D3, 1, "XADDD", DIR,  M6809_UNDOCUMENTED },
	{ 0x10DC, 1, "LDQ",   DIR,  HD6309_EXCLUSIVE },
	{ 0x10DD, 1, "STQ",   DIR,  HD6309_EXCLUSIVE },
	{ 0x10DE, 1, "LDS",   DIR,  M6x09_GENERAL },
	{ 0x10DF, 1, "STS",   DIR,  M6x09_GENERAL },

	{ 0x10E3, 1, "XADDD", IND,  M6809_UNDOCUMENTED },
	{ 0x10EC, 1, "LDQ",   IND,  HD6309_EXCLUSIVE },
	{ 0x10ED, 1, "STQ",   IND,  HD6309_EXCLUSIVE },
	{ 0x10EE, 1, "LDS",   IND,  M6x09_GENERAL },
	{ 0x10EF, 1, "STS",   IND,  M6x09_GENERAL },

	{ 0x10F3, 2, "XADDD", EXT,  M6809_UNDOCUMENTED },
	{ 0x10FC, 2, "LDQ",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10FD, 2, "STQ",   EXT,  HD6309_EXCLUSIVE },
	{ 0x10FE, 2, "LDS",   EXT,  M6x09_GENERAL },
	{ 0x10FF, 2, "STS",   EXT,  M6x09_GENERAL },

	// Page 3 opcodes (0x11 0x..)
	{ 0x1110, 0, "PAGE2", PG2,    M6x09_GENERAL },
	{ 0x1111, 0, "PAGE3", PG3,    M6x09_GENERAL },

	{ 0x1130, 2, "BAND",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1131, 2, "BIAND", IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1132, 2, "BOR",   IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1133, 2, "BIOR",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1134, 2, "BEOR",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1135, 2, "BIEOR", IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1136, 2, "LDBT",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1137, 2, "STBT",  IMM_BW,   HD6309_EXCLUSIVE },
	{ 0x1138, 1, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x1139, 1, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x113A, 1, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x113B, 1, "TFM",   IMM_TFM,  HD6309_EXCLUSIVE },
	{ 0x113C, 1, "BITMD", IMM,      HD6309_EXCLUSIVE },
	{ 0x113D, 1, "LDMD",  IMM,      HD6309_EXCLUSIVE },
	{ 0x113E, 0, "XFIRQ", INH,      M6809_UNDOCUMENTED },
	{ 0x113F, 0, "SWI3",  INH,      M6x09_GENERAL },

	{ 0x1143, 0, "COME",  INH,  HD6309_EXCLUSIVE },
	{ 0x114A, 0, "DECE",  INH,  HD6309_EXCLUSIVE },
	{ 0x114C, 0, "INCE",  INH,  HD6309_EXCLUSIVE },
	{ 0x114D, 0, "TSTE",  INH,  HD6309_EXCLUSIVE },
	{ 0x114F, 0, "CLRE",  INH,  HD6309_EXCLUSIVE },

	{ 0x1153, 0, "COMF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115A, 0, "DECF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115C, 0, "INCF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115D, 0, "TSTF",  INH,  HD6309_EXCLUSIVE },
	{ 0x115F, 0, "CLRF",  INH,  HD6309_EXCLUSIVE },

	{ 0x1180, 1, "SUBE",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1181, 1, "CMPE",  IMM,  HD6309_EXCLUSIVE },
	{ 0x1183, 2, "CMPU",  IMM,  M6x09_GENERAL },
	{ 0x1186, 1, "LDE",   IMM,  HD6309_EXCLUSIVE },
	{ 0x1187, 1, "XSTA",  IMM,  M6809_UNDOCUMENTED },
	{ 0x118B, 1, "ADDE",  IMM,  HD6309_EXCLUSIVE },
	{ 0x118C, 2, "CMPS",  IMM,  M6x09_GENERAL },
	{ 0x118D, 1, "DIVD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x118E, 2, "DIVQ",  IMM,  HD6309_EXCLUSIVE },
	{ 0x118F, 2, "MULD",  IMM,  HD6309_EXCLUSIVE },
	{ 0x118F, 2, "XSTX",  IMM,  M6809_UNDOCUMENTED },

	{ 0x1190, 1, "SUBE",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1191, 1, "CMPE",  DIR,  HD6309_EXCLUSIVE },
	{ 0x1193, 1, "CMPU",  DIR,  M6x09_GENERAL },
	{ 0x1196, 1, "LDE",   DIR,  HD6309_EXCLUSIVE },
	{ 0x1197, 1, "STE",   DIR,  HD6309_EXCLUSIVE },
	{ 0x119B, 1, "ADDE",  DIR,  HD6309_EXCLUSIVE },
	{ 0x119C, 1, "CMPS",  DIR,  M6x09_GENERAL },
	{ 0x119D, 1, "DIVD",  DIR,  HD6309_EXCLUSIVE },
	{ 0x119E, 1, "DIVQ",  DIR,  HD6309_EXCLUSIVE },
	{ 0x119F, 1, "MULD",  DIR,  HD6309_EXCLUSIVE },

	{ 0x11A0, 1, "SUBE",  IND,  HD6309_EXCLUSIVE },
	{ 0x11A1, 1, "CMPE",  IND,  HD6309_EXCLUSIVE },
	{ 0x11A3, 1, "CMPU",  IND,  M6x09_GENERAL },
	{ 0x11A6, 1, "LDE",   IND,  HD6309_EXCLUSIVE },
	{ 0x11A7, 1, "STE",   IND,  HD6309_EXCLUSIVE },
	{ 0x11AB, 1, "ADDE",  IND,  HD6309_EXCLUSIVE },
	{ 0x11AC, 1, "CMPS",  IND,  M6x09_GENERAL },
	{ 0x11AD, 1, "DIVD",  IND,  HD6309_EXCLUSIVE },
	{ 0x11AE, 1, "DIVQ",  IND,  HD6309_EXCLUSIVE },
	{ 0x11AF, 1, "MULD",  IND,  HD6309_EXCLUSIVE },

	{ 0x11B0, 2, "SUBE",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11B1, 2, "CMPE",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11B3, 2, "CMPU",  EXT,  M6x09_GENERAL },
	{ 0x11B6, 2, "LDE",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11B7, 2, "STE",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11BB, 2, "ADDE",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11BC, 2, "CMPS",  EXT,  M6x09_GENERAL },
	{ 0x11BD, 2, "DIVD",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11BE, 2, "DIVQ",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11BF, 2, "MULD",  EXT,  HD6309_EXCLUSIVE },

	{ 0x11C0, 1, "SUBF",  IMM,  HD6309_EXCLUSIVE },
	{ 0x11C1, 1, "CMPF",  IMM,  HD6309_EXCLUSIVE },
	{ 0x11C3, 2, "XADDU", IMM,  M6809_UNDOCUMENTED },
	{ 0x11C6, 1, "LDF",   IMM,  HD6309_EXCLUSIVE },
	{ 0x11C7, 1, "XSTB",  IMM,  M6809_UNDOCUMENTED },
	{ 0x11CB, 1, "ADDF",  IMM,  HD6309_EXCLUSIVE },
	{ 0x11CF, 2, "XSTU",  IMM,  M6809_UNDOCUMENTED },

	{ 0x11D0, 1, "SUBF",  DIR,  HD6309_EXCLUSIVE },
	{ 0x11D1, 1, "CMPF",  DIR,  HD6309_EXCLUSIVE },
	{ 0x11D3, 1, "XADDU", DIR,  M6809_UNDOCUMENTED },
	{ 0x11D6, 1, "LDF",   DIR,  HD6309_EXCLUSIVE },
	{ 0x11D7, 1, "STF",   DIR,  HD6309_EXCLUSIVE },
	{ 0x11DB, 1, "ADDF",  DIR,  HD6309_EXCLUSIVE },

	{ 0x11E0, 1, "SUBF",  IND,  HD6309_EXCLUSIVE },
	{ 0x11E1, 1, "CMPF",  IND,  HD6309_EXCLUSIVE },
	{ 0x11E3, 1, "XADDU", IND,  M6809_UNDOCUMENTED },
	{ 0x11E6, 1, "LDF",   IND,  HD6309_EXCLUSIVE },
	{ 0x11E7, 1, "STF",   IND,  HD6309_EXCLUSIVE },
	{ 0x11EB, 1, "ADDF",  IND,  HD6309_EXCLUSIVE },

	{ 0x11F0, 2, "SUBF",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11F1, 2, "CMPF",  EXT,  HD6309_EXCLUSIVE },
	{ 0x11F3, 2, "XADDU", EXT,  M6809_UNDOCUMENTED },
	{ 0x11F6, 2, "LDF",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11F7, 2, "STF",   EXT,  HD6309_EXCLUSIVE },
	{ 0x11FB, 2, "ADDF",  EXT,  HD6309_EXCLUSIVE }
};


// ======================> m6x09_disassembler

m6x09_disassembler::m6x09_disassembler(uint32_t level, const char teregs[16][4])
	: m6x09_base_disassembler(m6x09_opcodes, std::size(m6x09_opcodes), level)
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
		util::stream_format(stream, "$%04X,PCR", (p+offset) & 0xffff); // PC Relative addressing (assembler computes offset from specified absolute address)
		break;

	case 0x8d:  // (+/- 15 bit offset),PC
		offset = (int16_t)params.r16(p);
		p += 2;
		util::stream_format(stream, "$%04X,PCR", (p+offset) & 0xffff); // PC Relative addressing (assembler computes offset from specified absolute address)
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


m6809_disassembler::m6809_disassembler() : m6x09_disassembler(M6x09_GENERAL|M6809_UNDOCUMENTED, m6809_teregs)
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

hd6309_disassembler::hd6309_disassembler() : m6x09_disassembler(M6x09_GENERAL|HD6309_EXCLUSIVE, hd6309_teregs)
{
}


//**************************************************************************
//  Konami disassembler
//**************************************************************************

const m6x09_base_disassembler::opcodeinfo konami_disassembler::konami_opcodes[] =
{
	{ 0x08, 1, "LEAX",  IND,    M6x09_GENERAL },
	{ 0x09, 1, "LEAY",  IND,    M6x09_GENERAL },
	{ 0x0A, 1, "LEAU",  IND,    M6x09_GENERAL },
	{ 0x0B, 1, "LEAS",  IND,    M6x09_GENERAL },
	{ 0x0C, 1, "PUSHS", PSHS,   M6x09_GENERAL },
	{ 0x0D, 1, "PUSHU", PSHU,   M6x09_GENERAL },
	{ 0x0E, 1, "PULLS", PULS,   M6x09_GENERAL },
	{ 0x0F, 1, "PULLU", PULU,   M6x09_GENERAL },

	{ 0x10, 1, "LDA",   IMM,    M6x09_GENERAL },
	{ 0x11, 1, "LDB",   IMM,    M6x09_GENERAL },
	{ 0x12, 1, "LDA",   IND,    M6x09_GENERAL },
	{ 0x13, 1, "LDB",   IND,    M6x09_GENERAL },
	{ 0x14, 1, "ADDA",  IMM,    M6x09_GENERAL },
	{ 0x15, 1, "ADDB",  IMM,    M6x09_GENERAL },
	{ 0x16, 1, "ADDA",  IND,    M6x09_GENERAL },
	{ 0x17, 1, "ADDB",  IND,    M6x09_GENERAL },
	{ 0x18, 1, "ADCA",  IMM,    M6x09_GENERAL },
	{ 0x19, 1, "ADCB",  IMM,    M6x09_GENERAL },
	{ 0x1A, 1, "ADCA",  IND,    M6x09_GENERAL },
	{ 0x1B, 1, "ADCB",  IND,    M6x09_GENERAL },
	{ 0x1C, 1, "SUBA",  IMM,    M6x09_GENERAL },
	{ 0x1D, 1, "SUBB",  IMM,    M6x09_GENERAL },
	{ 0x1E, 1, "SUBA",  IND,    M6x09_GENERAL },
	{ 0x1F, 1, "SUBB",  IND,    M6x09_GENERAL },

	{ 0x20, 1, "SBCA",  IMM,    M6x09_GENERAL },
	{ 0x21, 1, "SBCB",  IMM,    M6x09_GENERAL },
	{ 0x22, 1, "SBCA",  IND,    M6x09_GENERAL },
	{ 0x23, 1, "SBCB",  IND,    M6x09_GENERAL },
	{ 0x24, 1, "ANDA",  IMM,    M6x09_GENERAL },
	{ 0x25, 1, "ANDB",  IMM,    M6x09_GENERAL },
	{ 0x26, 1, "ANDA",  IND,    M6x09_GENERAL },
	{ 0x27, 1, "ANDB",  IND,    M6x09_GENERAL },
	{ 0x28, 1, "BITA",  IMM,    M6x09_GENERAL },
	{ 0x29, 1, "BITB",  IMM,    M6x09_GENERAL },
	{ 0x2A, 1, "BITA",  IND,    M6x09_GENERAL },
	{ 0x2B, 1, "BITB",  IND,    M6x09_GENERAL },
	{ 0x2C, 1, "EORA",  IMM,    M6x09_GENERAL },
	{ 0x2D, 1, "EORB",  IMM,    M6x09_GENERAL },
	{ 0x2E, 1, "EORA",  IND,    M6x09_GENERAL },
	{ 0x2F, 1, "EORB",  IND,    M6x09_GENERAL },

	{ 0x30, 1, "ORA",   IMM,    M6x09_GENERAL },
	{ 0x31, 1, "ORB",   IMM,    M6x09_GENERAL },
	{ 0x32, 1, "ORA",   IND,    M6x09_GENERAL },
	{ 0x33, 1, "ORB",   IND,    M6x09_GENERAL },
	{ 0x34, 1, "CMPA",  IMM,    M6x09_GENERAL },
	{ 0x35, 1, "CMPB",  IMM,    M6x09_GENERAL },
	{ 0x36, 1, "CMPA",  IND,    M6x09_GENERAL },
	{ 0x37, 1, "CMPB",  IND,    M6x09_GENERAL },
	{ 0x38, 1, "SETLN", IMM,    M6x09_GENERAL },
	{ 0x39, 1, "SETLN", IND,    M6x09_GENERAL },
	{ 0x3A, 1, "STA",   IND,    M6x09_GENERAL },
	{ 0x3B, 1, "STB",   IND,    M6x09_GENERAL },
	{ 0x3C, 1, "ANDCC", IMM,    M6x09_GENERAL },
	{ 0x3D, 1, "ORCC",  IMM,    M6x09_GENERAL },
	{ 0x3E, 1, "EXG",   IMM_RR, M6x09_GENERAL },
	{ 0x3F, 1, "TFR",   IMM_RR, M6x09_GENERAL },

	{ 0x40, 2, "LDD",   IMM,    M6x09_GENERAL },
	{ 0x41, 1, "LDD",   IND,    M6x09_GENERAL },
	{ 0x42, 2, "LDX",   IMM,    M6x09_GENERAL },
	{ 0x43, 1, "LDX",   IND,    M6x09_GENERAL },
	{ 0x44, 2, "LDY",   IMM,    M6x09_GENERAL },
	{ 0x45, 1, "LDY",   IND,    M6x09_GENERAL },
	{ 0x46, 2, "LDU",   IMM,    M6x09_GENERAL },
	{ 0x47, 1, "LDU",   IND,    M6x09_GENERAL },
	{ 0x48, 2, "LDS",   IMM,    M6x09_GENERAL },
	{ 0x49, 1, "LDS",   IND,    M6x09_GENERAL },
	{ 0x4A, 2, "CMPD",  IMM,    M6x09_GENERAL },
	{ 0x4B, 1, "CMPD",  IND,    M6x09_GENERAL },
	{ 0x4C, 2, "CMPX",  IMM,    M6x09_GENERAL },
	{ 0x4D, 1, "CMPX",  IND,    M6x09_GENERAL },
	{ 0x4E, 2, "CMPY",  IMM,    M6x09_GENERAL },
	{ 0x4F, 1, "CMPY",  IND,    M6x09_GENERAL },

	{ 0x50, 2, "CMPU",  IMM,    M6x09_GENERAL },
	{ 0x51, 1, "CMPU",  IND,    M6x09_GENERAL },
	{ 0x52, 2, "CMPS",  IMM,    M6x09_GENERAL },
	{ 0x53, 1, "CMPS",  IND,    M6x09_GENERAL },
	{ 0x54, 2, "ADDD",  IMM,    M6x09_GENERAL },
	{ 0x55, 1, "ADDD",  IND,    M6x09_GENERAL },
	{ 0x56, 2, "SUBD",  IMM,    M6x09_GENERAL },
	{ 0x57, 1, "SUBD",  IND,    M6x09_GENERAL },
	{ 0x58, 1, "STD",   IND,    M6x09_GENERAL },
	{ 0x59, 1, "STX",   IND,    M6x09_GENERAL },
	{ 0x5A, 1, "STY",   IND,    M6x09_GENERAL },
	{ 0x5B, 1, "STU",   IND,    M6x09_GENERAL },
	{ 0x5C, 1, "STS",   IND,    M6x09_GENERAL },

	{ 0x60, 1, "BRA",   REL,    M6x09_GENERAL },
	{ 0x61, 1, "BHI",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x62, 1, "BCC",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x63, 1, "BNE",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x64, 1, "BVC",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x65, 1, "BPL",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x66, 1, "BGE",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x67, 1, "BGT",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x68, 2, "LBRA",  LREL,   M6x09_GENERAL },
	{ 0x69, 2, "LBHI",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x6A, 2, "LBCC",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x6B, 2, "LBNE",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x6C, 2, "LBVC",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x6D, 2, "LBPL",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x6E, 2, "LBGE",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x6F, 2, "LBGT",  LREL,   M6x09_GENERAL, STEP_COND },

	{ 0x70, 1, "BRN",   REL,    M6x09_GENERAL },
	{ 0x71, 1, "BLS",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x72, 1, "BCS",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x73, 1, "BEQ",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x74, 1, "BVS",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x75, 1, "BMI",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x76, 1, "BLT",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x77, 1, "BLE",   REL,    M6x09_GENERAL, STEP_COND },
	{ 0x78, 2, "LBRN",  LREL,   M6x09_GENERAL },
	{ 0x79, 2, "LBLS",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x7A, 2, "LBCS",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x7B, 2, "LBEQ",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x7C, 2, "LBVS",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x7D, 2, "LBMI",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x7E, 2, "LBLT",  LREL,   M6x09_GENERAL, STEP_COND },
	{ 0x7F, 2, "LBLE",  LREL,   M6x09_GENERAL, STEP_COND },

	{ 0x80, 0, "CLRA",  INH,    M6x09_GENERAL },
	{ 0x81, 0, "CLRB",  INH,    M6x09_GENERAL },
	{ 0x82, 1, "CLR",   IND,    M6x09_GENERAL },
	{ 0x83, 0, "COMA",  INH,    M6x09_GENERAL },
	{ 0x84, 0, "COMB",  INH,    M6x09_GENERAL },
	{ 0x85, 1, "COM",   IND,    M6x09_GENERAL },
	{ 0x86, 0, "NEGA",  INH,    M6x09_GENERAL },
	{ 0x87, 0, "NEGB",  INH,    M6x09_GENERAL },
	{ 0x88, 1, "NEG",   IND,    M6x09_GENERAL },
	{ 0x89, 0, "INCA",  INH,    M6x09_GENERAL },
	{ 0x8A, 0, "INCB",  INH,    M6x09_GENERAL },
	{ 0x8B, 1, "INC",   IND,    M6x09_GENERAL },
	{ 0x8C, 0, "DECA",  INH,    M6x09_GENERAL },
	{ 0x8D, 0, "DECB",  INH,    M6x09_GENERAL },
	{ 0x8E, 1, "DEC",   IND,    M6x09_GENERAL },
	{ 0x8F, 0, "RTS",   INH,    M6x09_GENERAL, STEP_OUT },

	{ 0x90, 0, "TSTA",  INH,    M6x09_GENERAL },
	{ 0x91, 0, "TSTB",  INH,    M6x09_GENERAL },
	{ 0x92, 1, "TST",   IND,    M6x09_GENERAL },
	{ 0x93, 0, "LSRA",  INH,    M6x09_GENERAL },
	{ 0x94, 0, "LSRB",  INH,    M6x09_GENERAL },
	{ 0x95, 1, "LSR",   IND,    M6x09_GENERAL },
	{ 0x96, 0, "RORA",  INH,    M6x09_GENERAL },
	{ 0x97, 0, "RORB",  INH,    M6x09_GENERAL },
	{ 0x98, 1, "ROR",   IND,    M6x09_GENERAL },
	{ 0x99, 0, "ASRA",  INH,    M6x09_GENERAL },
	{ 0x9A, 0, "ASRB",  INH,    M6x09_GENERAL },
	{ 0x9B, 1, "ASR",   IND,    M6x09_GENERAL },
	{ 0x9C, 0, "ASLA",  INH,    M6x09_GENERAL },
	{ 0x9D, 0, "ASLB",  INH,    M6x09_GENERAL },
	{ 0x9E, 1, "ASL",   IND,    M6x09_GENERAL },
	{ 0x9F, 0, "RTI",   INH,    M6x09_GENERAL, STEP_OUT },

	{ 0xA0, 0, "ROLA",  INH,    M6x09_GENERAL },
	{ 0xA1, 0, "ROLB",  INH,    M6x09_GENERAL },
	{ 0xA2, 1, "ROL",   IND,    M6x09_GENERAL },
	{ 0xA3, 1, "LSRW",  IND,    M6x09_GENERAL },
	{ 0xA4, 1, "RORW",  IND,    M6x09_GENERAL },
	{ 0xA5, 1, "ASRW",  IND,    M6x09_GENERAL },
	{ 0xA6, 1, "ASLW",  IND,    M6x09_GENERAL },
	{ 0xA7, 1, "ROLW",  IND,    M6x09_GENERAL },
	{ 0xA8, 1, "JMP",   IND,    M6x09_GENERAL },
	{ 0xA9, 1, "JSR",   IND,    M6x09_GENERAL, STEP_OVER },
	{ 0xAA, 1, "BSR",   REL,    M6x09_GENERAL, STEP_OVER },
	{ 0xAB, 2, "LBSR",  LREL,   M6x09_GENERAL, STEP_OVER },
	{ 0xAC, 1, "DBJNZ", REL,    M6x09_GENERAL },
	{ 0xAD, 1, "DXJNZ", REL,    M6x09_GENERAL },
	{ 0xAE, 0, "NOP",   INH,    M6x09_GENERAL },

	{ 0xB0, 0, "ABX",   INH,    M6x09_GENERAL },
	{ 0xB1, 0, "DAA",   INH,    M6x09_GENERAL },
	{ 0xB2, 0, "SEX",   INH,    M6x09_GENERAL },
	{ 0xB3, 0, "MUL",   INH,    M6x09_GENERAL },
	{ 0xB4, 0, "LMUL",  INH,    M6x09_GENERAL },
	{ 0xB5, 0, "DIV    X,B",   INH, M6x09_GENERAL },
	{ 0xB6, 0, "BMOVE  Y,X,U", INH, M6x09_GENERAL },
	{ 0xB7, 0, "MOVE   Y,X,U", INH, M6x09_GENERAL },
	{ 0xB8, 1, "LSRDI", IMM,    M6x09_GENERAL },
	{ 0xB9, 1, "LSRWA", IND,    M6x09_GENERAL },
	{ 0xBA, 1, "RORDI", IMM,    M6x09_GENERAL },
	{ 0xBB, 1, "RORWA", IND,    M6x09_GENERAL },
	{ 0xBC, 1, "ASRDI", IMM,    M6x09_GENERAL },
	{ 0xBD, 1, "ASRWA", IND,    M6x09_GENERAL },
	{ 0xBE, 1, "ASLDI", IMM,    M6x09_GENERAL },
	{ 0xBF, 1, "ASLWA", IND,    M6x09_GENERAL },

	{ 0xC0, 1, "ROLDI", IMM,    M6x09_GENERAL },
	{ 0xC1, 1, "ROLWA", IND,    M6x09_GENERAL },
	{ 0xC2, 0, "CLRD",  INH,    M6x09_GENERAL },
	{ 0xC3, 1, "CLRW",  IND,    M6x09_GENERAL },
	{ 0xC4, 0, "NEGD",  INH,    M6x09_GENERAL },
	{ 0xC5, 1, "NEGW",  IND,    M6x09_GENERAL },
	{ 0xC6, 0, "INCD",  INH,    M6x09_GENERAL },
	{ 0xC7, 1, "INCW",  IND,    M6x09_GENERAL },
	{ 0xC8, 0, "DECD",  INH,    M6x09_GENERAL },
	{ 0xC9, 1, "DECW",  IND,    M6x09_GENERAL },
	{ 0xCA, 0, "TSTD",  INH,    M6x09_GENERAL },
	{ 0xCB, 1, "TSTW",  IND,    M6x09_GENERAL },
	{ 0xCC, 0, "ABSA",  INH,    M6x09_GENERAL },
	{ 0xCD, 0, "ABSB",  INH,    M6x09_GENERAL },
	{ 0xCE, 0, "ABSD",  INH,    M6x09_GENERAL },
	{ 0xCF, 0, "BSET   A,X,U", INH, M6x09_GENERAL },

	{ 0xD0, 0, "BSETW  D,X,U", INH, M6x09_GENERAL }
};


// ======================> konami_disassembler

konami_disassembler::konami_disassembler() : m6x09_base_disassembler(konami_opcodes, std::size(konami_opcodes), M6x09_GENERAL)
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
		"X", /* 2 */
		"Y", /* 3 */
		"?", /* 4 - direct page */
		"U", /* 5 */
		"S", /* 6 */
		"PC" /* 7 - pc */
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
				util::stream_format(stream, "[A,%s]", index_reg[idx]);
				break;

			case 0x01: /* register b */
				util::stream_format(stream, "[B,%s]", index_reg[idx]);
				break;

			case 0x04: /* direct - mode */
				util::stream_format(stream, "[<$%02X]", params.r8(p++));
				break;

			case 0x07: /* register d */
				util::stream_format(stream, "[D,%s]", index_reg[idx]);
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
				util::stream_format(stream, "A,%s", index_reg[idx]);
				break;

			case 0x01: /* register b */
				util::stream_format(stream, "B,%s", index_reg[idx]);
				break;

			case 0x04: /* direct - mode */
				util::stream_format(stream, "<$%02X", params.r8(p++));
				break;

			case 0x07: /* register d */
				util::stream_format(stream, "D,%s", index_reg[idx]);
				break;

			default:
				util::stream_format(stream, "?,%s", index_reg[idx]);
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
					util::stream_format(stream, "[#$-%02X,%s]", 0x100 - val, index_reg[idx]);
				else
					util::stream_format(stream, "[#$%02X,%s]", val, index_reg[idx]);
				break;

			case 5: // post word offset
				val = params.r16(p);
				p += 2;

				if (val & 0x8000)
					util::stream_format(stream, "[#$-%04X,%s]", 0x10000 - val, index_reg[idx]);
				else
					util::stream_format(stream, "[#$%04X,%s]", val, index_reg[idx]);
				break;

			case 6: // simple
				util::stream_format(stream, "[,%s]", index_reg[idx]);
				break;

			case 7: // extended
				val = params.r16(p);
				p += 2;

				util::stream_format(stream, "[$%04X]", val);
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
					util::stream_format(stream, "#$-%02X,%s", 0x100 - val, index_reg[idx]);
				else
					util::stream_format(stream, "#$%02X,%s", val, index_reg[idx]);
				break;

			case 5: // post word offset
				val = params.r16(p);
				p += 2;

				if (val & 0x8000)
					util::stream_format(stream, "#$-%04X,%s", 0x10000 - val, index_reg[idx]);
				else
					util::stream_format(stream, "#$%04X,%s", val, index_reg[idx]);
				break;

			case 6: // simple
				util::stream_format(stream, ",%s", index_reg[idx]);
				break;

			case 7: // extended
				val = params.r16(p);
				p += 2;

				util::stream_format(stream, "$%04X", val);
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
	const char *const konami_teregs[8] =
	{
		// B: D when reading, B when writing
		"A", "B", "X", "Y", "DP", "U", "S", "PC"
	};
	util::stream_format(stream, "%s,%s",
		konami_teregs[(pb >> 0) & 0x7],
		konami_teregs[(pb >> 4) & 0x7]);
}
