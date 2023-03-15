// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VT61 microcode disassembler

    No microprogram source listing appears to have been published by DEC,
    nor any instruction reference other than the decode signal references
    in the schematics. The instruction format should not be regarded as
    official, though it draws on some of DEC's PDP assembly languages.

***************************************************************************/

#include "emu.h"
#include "vt61dasm.h"

vt61_disassembler::vt61_disassembler()
	: util::disasm_interface()
{
}

const char *const vt61_disassembler::s_opr_a[8] = {
	"NOP",      // NO OP A
	"IAC",      // INC AC
	"LIR",      // LD IR
	"IMA",      // INC MA
	"SMD",      // SHIFT MDR
	"JMP0",     // CLR PC
	"CMPC8",    // COMP PC 8
	"LDR"       // LD RAM
};

const char *const vt61_disassembler::s_opr_b[16] = {
	"OPR",      // NO OP B
	"SPARE",
	"RC",       // RESET C FLAG
	"SPARE",
	"SPARE",
	"LMISC",    // LD MISC FLAG
	"LLED",     // LD LED FLAG
	"LMOD",     // LD MODEM FLAG
	"SKCLK",    // SET KEYCLICK
	"LINTRC",   // LD INTRPT CONTROL
	"SPARE",
	"CNBR",     // CLR NBR
	"LSYNC",    // LD SYNC + CLR NBX
	"PLD",      // PLD
	"CVSR",     // CLR VID SERV REQ
	"ENVID"     // ENABLE VID LOAD
};

const char *const vt61_disassembler::s_sources[12] = {
	"AC",       // SEL AC
	"STAT1",    // SEL STATUS 1
	"SW",       // SEL SWITCHES
	"CAS1",     // SEL CAS 1
	"CAS2",     // SEL CAS 2
	"RAM",      // SEL RAM
	"IDR",      // SEL IDR
	"UART",     // SEL UART
	"SPARE",
	"TBSW",     // TEST BOX SWITCH
	"invalid",  // SEL SPM LO
	"invalid"   // SEL SPM HI
};

const char *const vt61_disassembler::s_conditions[32][2] = {
	{ "INTR1",  "NINTR1" }, // INTR 1 L (actually active high)
	{ "NBXF",   "NBXT"   }, // NBX L
	{ "NBRF",   "NBRT"   }, // NBR L
	{ "NF1",    "F1"     }, // F1 L
	{ "NF2",    "F2"     }, // F2 L
	{ "NCF",    "CF"     }, // C L
	{ "NURF",   "URF"    }, // UART R FLAG L
	{ "NUTB",   "UTB"    }, // UART T BUFF L
	{ "NXOFF",  "XOFF"   }, // DO X OFF L
	{ "NGOSR",  "GOSR"   }, // GOUT SER RQ L
	{ "NF3",    "F3"     }, // F3 L
	{ "NINTR2", "INTR2"  }, // INTR 2 L
	{ "KEYUP",  "KEYDN"  }, // KEY DOWN L
	{ "NPWRUP", "PWRUP"  }, // PWR UP L
	{ "NSYNC",  "SYNC"   }, // SYNC ENA L
	{ "PE",     "NPE"    }, // PARITY ERROR H
	{ "BZY",    "NBZY"   }, // MEM BZY H
	{ "EQ",     "NE"     }, // EQUAL H
	{ "CC",     "CS"     }, // CARRY OUT L
	{ "MDOFO",  "MDOFZ"  }, // MDR O.F. OUT H
	{ "NCSR",   "CSR"    }, // COPIER SER REQ L
	{ "NCPCF",  "CPCF"   }, // C-PCF L
	{ "T",      "F"      }, // TRUE H
	{ "NVSR",   "VSR"    }, // VID SERV REQ L
	{ "?H",     "?L"     },
	{ "?H",     "?L"     },
	{ "?H",     "?L"     },
	{ "?H",     "?L"     },
	{ "?H",     "?L"     },
	{ "?H",     "?L"     },
	{ "?H",     "?L"     },
	{ "?H",     "?L"     }
};

u32 vt61_disassembler::opcode_alignment() const
{
	return 1;
}

u32 vt61_disassembler::interface_flags() const
{
	return PAGED;
}

u32 vt61_disassembler::page_address_bits() const
{
	return 8;
}

void vt61_disassembler::dasm_spr(std::ostream &stream, u8 r)
{
	if (BIT(r, 4))
		util::stream_format(stream, "IR%d", r & 9);
	else
		util::stream_format(stream, "R%d", r);
}

void vt61_disassembler::dasm_source(std::ostream &stream, u16 inst)
{
	if ((inst & 003000) == 003000)
		util::stream_format(stream, "#%03o", inst & 000377);
	else
	{
		if ((inst & 003400) == 002400 && (inst & 070000) != 070000)
			dasm_spr(stream, (inst & 000170) >> 2 | (inst & 000200) >> 7);
		else
			stream << s_sources[(inst & 003600) >> 7];
		if ((inst & 000007) != 0 && (inst & 0170000) != 0100000)
			stream << "," << s_opr_a[inst & 000007];
	}
}

offs_t vt61_disassembler::disassemble(std::ostream &stream, offs_t pc, const vt61_disassembler::data_buffer &opcodes, const vt61_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	offs_t flags = SUPPORTED;

	if (!BIT(inst, 15))
	{
		if ((inst & 074000) == 054000)
		{
			stream << "LDU";
			if ((inst & 000007) != 0 && (inst & 003000) != 003000)
			{
				stream << " " << s_opr_a[inst & 000007];
				if ((inst & 000007) == 5)
					flags |= STEP_OUT;
			}
		}
		else
		{
			switch (inst & 070000)
			{
			case 000000:
				stream << "LAC ";
				break;

			case 040000:
				stream << "LMA " << (BIT(inst, 11) ? "HI," : "LO,");
				break;

			case 050000:
				stream << "JUMP ";
				break;

			case 060000:
				if (BIT(inst, 11))
					stream << "LMD ";
				else
					stream << "CAS3 ";
				break;

			case 070000:
				stream << "LSP ";
				dasm_spr(stream, (inst & 000170) >> 2 | (inst & 004000) >> 11);
				stream << ",";
				break;

			default:
				stream << "LD? ";
				break;
			}

			dasm_source(stream, inst);
			if ((inst & 000007) == 5 && (inst & 003000) != 003000)
				flags |= STEP_OUT;
		}
	}
	else if (BIT(inst, 14))
	{
		util::stream_format(stream, "JMP %s,%03o", s_conditions[(inst & 017400) >> 8][BIT(inst, 13)], inst & 000377);
		if ((inst & 017400) != 013000)
			flags |= STEP_COND;
	}
	else if (BIT(inst, 13))
	{
		if ((inst & 007407) == 0)
			stream << "NOP";
		else
		{
			stream << s_opr_b[(inst & 007400) >> 8];
			if ((inst & 000007) != 0)
			{
				stream << " " << s_opr_a[inst & 000007];
				if ((inst & 000207) == 000204)
					stream << "A"; // ENABLE MDR A (CAS DATA)
				if ((inst & 000107) == 000104)
					stream << "B"; // ENABLE MDR B (SYNC DATA IN)
				if ((inst & 000007) == 5)
					flags |= STEP_OUT;
			}
		}
	}
	else if (BIT(inst, 12))
	{
		stream << "CMP ";
		dasm_source(stream, inst);
		if ((inst & 000007) == 5 && (inst & 003000) != 003000)
			flags |= STEP_OUT;
	}
	else switch (inst & 000007)
	{
	case 0:
		stream << "CMA";
		break;

	case 1:
		stream << "ORA ";
		dasm_source(stream, inst);
		break;

	case 2:
		stream << "XOR ";
		dasm_source(stream, inst);
		break;

	case 3:
		stream << "AND ";
		dasm_source(stream, inst);
		break;

	case 4:
		stream << "DCA";
		if (BIT(inst, 11))
			stream << " C";
		break;

	case 5:
		stream << "ADD ";
		if (BIT(inst, 11))
			stream << "C,";
		dasm_source(stream, inst);
		break;

	case 6:
		stream << "SUB ";
		if (BIT(inst, 11))
			stream << "C,";
		dasm_source(stream, inst);
		break;

	case 7:
		stream << "SLA";
		if (BIT(inst, 11))
			stream << " C";
		break;
	}

	return 1 | flags;
}
