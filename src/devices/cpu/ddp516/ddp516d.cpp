// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Honeywell (Computer Control Division) DDP-516 disassembler

    Alternate disassemblers are provided for Prime Computer's 16S, 32S,
    32R, 64R and 64V instruction modes, which are increasingly elaborate
    extensions of DDP-516. (The 32I and IX modes are general register
    architectures with mostly different opcodes and are not supported
    here.)

    Bits are numbered from 1 (sign) to 16 (LSB). All arithmetic is two's
    complement, though DAP-16 prints memory reference instructions in sign-
    magnitude format.

    In DBL mode, LDA, STA, ADD and SUB are executed as DLD, DST, DAD and
    DSB. The alternate mnemonics are not used in the disassembly output.
    (These operations also perform 31-bit rather than 32-bit arithmetic,
    omitting the sign bit of B. MPY and DIV likewise have 31-bit products
    and dividends except in Prime 64V mode.)

    Address 0 is equivalent to index register X, though its indexing tag
    is conventionally notated as 1. Several other primary registers,
    including A (1), B (2), S/Y (3) and P (7), are mapped to low memory
    addresses on Prime processors.

***************************************************************************/

#include "emu.h"
#include "ddp516d.h"

ddp516_disassembler::ddp516_disassembler()
	: util::disasm_interface()
{
}

prime_disassembler::prime_disassembler(u8 mode)
	: ddp516_disassembler()
	, c_mode_key(mode)
{
}

prime16s_disassembler::prime16s_disassembler()
	: prime_disassembler(0)
{
}

prime32s_disassembler::prime32s_disassembler()
	: prime_disassembler(1)
{
}

prime32r_disassembler::prime32r_disassembler()
	: prime_disassembler(3)
{
}

prime64r_disassembler::prime64r_disassembler()
	: prime_disassembler(2)
{
}

prime64v_disassembler::prime64v_disassembler()
	: prime_disassembler(6)
{
}

u32 ddp516_disassembler::opcode_alignment() const
{
	return 1;
}

u32 prime64v_disassembler::interface_flags() const
{
	return PAGED;
}

u32 prime64v_disassembler::page_address_bits() const
{
	// Virtual space consists of 65536-word segments
	return 16;
}

namespace {

constexpr auto STEP_COND = util::disasm_interface::STEP_COND;
constexpr auto STEP_OVER = util::disasm_interface::STEP_OVER;
constexpr auto STEP_OUT = util::disasm_interface::STEP_OUT;

const std::pair<const char *, offs_t> s_mr_insts[16] =
{
	{ "LDX", 0 },
	{ "JMP", 0 },
	{ "LDA", 0 },
	{ "ANA", 0 },
	{ "STA", 0 },
	{ "ERA", 0 },
	{ "ADD", 0 },
	{ "SUB", 0 },
	{ "JST", STEP_OVER },
	{ "CAS", STEP_COND }, // three-way skip, IBM 704 style
	{ "IRS", STEP_COND },
	{ "IMA", 0 },
	{ "IOG", 0 }, // not a real instruction
	{ "STX", 0 },
	{ "MPY", 0 },
	{ "DIV", 0 }
};

const std::pair<const char *, offs_t> s_mr_insts_r_mode[16][3] =
{
	{ { nullptr, 0 }, { nullptr, 0 }, { "JSX", STEP_OVER } },
	{ { "EAA", 0 }, { "XEC", 0 }, { "ENTR", 0 } },
	{ { "FLD", 0 }, { "DFLD", 0 }, { "JEQ", STEP_COND } },
	{ { nullptr, 0 }, { nullptr, 0 }, { "JNE", STEP_COND } },
	{ { "FST", 0 }, { "DFST", 0 }, { "JLE", STEP_COND } },
	{ { nullptr, 0 }, { nullptr, 0 }, { "JGT", STEP_COND } },
	{ { "FAD", 0 }, { "DFAD", 0 }, { "JLT", STEP_COND } },
	{ { "FSB", 0 }, { "DFSB", 0 }, { "JGE", STEP_COND } },
	{ { nullptr, 0 }, { "CREP", STEP_OVER }, { nullptr, 0 } },
	{ { "FCS", STEP_COND }, { "DFCS", STEP_COND }, { nullptr, 0 } },
	{ { nullptr, 0 }, { nullptr, 0 }, { nullptr, 0 } },
	{ { nullptr, 0 }, { nullptr, 0 }, { nullptr, 0 } },
	{ { nullptr, 0 }, { nullptr, 0 }, { nullptr, 0 } },
	{ { "FLX", 0 }, { "JDX", STEP_COND }, { "JIX", STEP_COND } },
	{ { "FMP", 0 }, { "DFMP", 0 }, { nullptr, 0 } },
	{ { "FDV", 0 }, { "DFDV", 0 }, { nullptr, 0 } }
};

const std::pair<const char *, offs_t> s_mr_insts_v_mode[16][4] =
{
	{ { "LDX", 0 }, { "LDY", 0 }, { "STY", 0 }, { "JSX", STEP_OVER } },
	{ { "JMP", 0 }, { "EAL", 0 }, { "XEC", 0 }, { nullptr, 0 } },
	{ { "LDA", 0 }, { "FLD", 0 }, { "DFLD", 0 }, { "LDL", 0 } },
	{ { "ANA", 0 }, { "STLR", 0 }, { "ORA", 0 }, { "ANL", 0 } },
	{ { "STA", 0 }, { "FST", 0 }, { "DFST", 0 }, { "STL", 0 } },
	{ { "ERA", 0 }, { "LDLR", 0 }, { nullptr, 0 }, { "ERL", 0 } },
	{ { "ADD", 0 }, { "FAD", 0 }, { "DFAD", 0 }, { "ADL", 0 } },
	{ { "SUB", 0 }, { "FSB", 0 }, { "DFSB", 0 }, { "SBL", 0 } },
	{ { "JST", STEP_OVER }, { nullptr, 0 }, { "PCL", STEP_OVER }, { nullptr, 0 } },
	{ { "CAS", STEP_COND }, { "FCS", STEP_COND }, { "DFCS", STEP_COND }, { "CLS", STEP_COND } },
	{ { "IRS", STEP_COND }, { "MIA", 0 }, { "EAXB", 0 }, { nullptr, 0 } },
	{ { "IMA", 0 }, { "MIB", 0 }, { "EALB", 0 }, { nullptr, 0 } },
	{ { "JSY", STEP_OVER }, { "EIO", 0 }, { "JSXB", STEP_OVER }, { nullptr, 0 } },
	{ { "STX", 0 }, { "FLX", 0 }, { "DFLX", 0 }, { "QFLX", 0 } },
	{ { "MPY", 0 }, { "FMP", 0 }, { "DFMP", 0 }, { "MPL", 0 } },
	{ { "DIV", 0 }, { "FDV", 0 }, { "DFDV", 0 }, { "DVL", 0 } }
};

const char *const s_io_insts[4] =
{
	"OCP", "SKS", "INA", "OTA"
};

const char *const s_skip_insts[10][2] =
{
	{ "SPL", "SMI" }, // Sign of A
	{ "SPN", "SPS" }, // Memory parity error
	{ "SLZ", "SLN" }, // LSB of A
	{ "SZE", "SNE" }, // A ≠ 0
	{ "SR1", "SS1" }, // Sense switch 1
	{ "SR2", "SS2" }, // Sense switch 2
	{ "SR3", "SS3" }, // Sense switch 3
	{ "SR4", "SS4" }, // Sense switch 4
	{ "SRC", "SSC" }, // C bit
	{ "SKP", "NOP" }  // No condition
};

const char *const s_sh_insts[16] =
{
	"LRL", "LRS", "LRR", nullptr, // Long right shifts
	"LGR", "ARS", "ARR", nullptr, // A right shifts
	"LLL", "LLS", "LLR", nullptr, // Long left shifts
	"LGL", "ALS", "ALR", nullptr  // A left shifts
};

const char *const s_sh_insts_prime[16] =
{
	"LRL", "LRS", "LRR", nullptr, // Long right shifts
	"ARR", "ARS", "ARR", nullptr, // A right shifts
	"LLL", "LLS", "LLR", nullptr, // Long left shifts
	"ALL", "ALS", "ALR", nullptr  // A left shifts
};

const char *const s_base_regs_v_mode[4] =
{
	"PB", "SB", "LB", "XB"
};

const std::pair<const char *, offs_t> s_quad_insts[8] =
{
	{ "QFLD", 0 },
	{ "QFST", 0 },
	{ "QFAD", 0 },
	{ "QFSB", 0 },
	{ "QFMP", 0 },
	{ "QFDV", 0 },
	{ "QFCS", STEP_COND },
	{ nullptr, 0 }
};

} // anonymous namespace


void ddp516_disassembler::format_disp(std::ostream &stream, u16 disp) const
{
	if (s16(disp) < 0)
	{
		stream << '-';
		disp = -disp;
	}
	if (disp > 7)
		stream << '\'';
	util::stream_format(stream, "%o", disp);
}

offs_t ddp516_disassembler::dasm_skip(std::ostream &stream, u16 inst) const
{
	const u16 cond = inst & 000777;
	if (cond == 036)
		util::stream_format(stream, "SS%c", BIT(inst, 9) ? 'S' : 'R');
	else if ((cond & (cond - 1)) == 0)
		stream << s_skip_insts[count_leading_zeros_32(cond) - 23][BIT(inst, 9)];
	else
		util::stream_format(stream, "%-6s'%04o", "SKP", inst & 001777);
	return 1 | (cond != 0 ? STEP_COND : 0) | SUPPORTED;
}

offs_t ddp516_disassembler::dasm_generic(std::ostream &stream, u16 inst) const
{
	switch (inst)
	{
	case 000000:
		stream << "HLT";
		break;

	case 000005:
		stream << "SGL";
		break;

	case 000007:
		stream << "DBL";
		break;

	case 000011:
		stream << "DXA";
		break;

	case 000013:
		stream << "EXA";
		break;

	case 000021:
		stream << "RMP";
		break;

	case 000041:
		stream << "SCA";
		break;

	case 000043:
		stream << "INK";
		break;

	case 000101:
		stream << "NRM";
		break;

	case 000201:
		stream << "IAB";
		break;

	case 000401:
		stream << "ENB";
		break;

	case 001001:
		stream << "INH";
		break;

	case 001401:
		stream << "ERM";
		break;

	case 0140024:
		stream << "CHS";
		break;

	case 0140040:
		stream << "CRA";
		break;

	case 0140100:
		stream << "SSP";
		break;

	case 0140200:
		stream << "RCB";
		break;

	case 0140240:
		stream << "CCA";
		break;

	case 0140320:
		stream << "CSA";
		break;

	case 0140401:
		stream << "CMA";
		break;

	case 0140407:
		stream << "TCA";
		break;

	case 0140500:
		stream << "SSM";
		break;

	case 0140600:
		stream << "SCB";
		break;

	case 0141044:
		stream << "CAR";
		break;

	case 0141050:
		stream << "CAL";
		break;

	case 0141140:
		stream << "ICL";
		break;

	case 0141206:
		stream << "AOA";
		break;

	case 0141216:
		stream << "ACA";
		break;

	case 0141240:
		stream << "ICR";
		break;

	case 0141340:
		stream << "ICA";
		break;

	default:
		// Some combined operations are possible, though not documented
		util::stream_format(stream, "%-6s%o", "OCT", inst);
		break;
	}
	return 1 | SUPPORTED;
}

offs_t ddp516_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u16 inst = opcodes.r16(pc);
	if (BIT(inst, 10, 4) != 0)
	{
		if (BIT(inst, 10, 4) == 014)
		{
			// Input-Output group
			if ((inst & 0140073) == 0140020)
			{
				// Output to device codes '20 and '24 is special
				if ((inst & 001777) == 01020)
					stream << "OTK";
				else
					util::stream_format(stream, "%-6s'%04o", "SMK", inst & 001777);
				return 1 | SUPPORTED;
			}
			else
			{
				util::stream_format(stream, "%-6s'%04o", s_io_insts[BIT(inst, 14, 2)], inst & 001777);
				return 1 | (BIT(inst, 14, 2) != 0 ? STEP_COND : 0) | SUPPORTED;
			}
		}
		else
		{
			// Memory reference instructions
			const u8 op = BIT(inst, 10, 4) == 015 && BIT(inst, 14) ? 0 : BIT(inst, 10, 4);
			const auto &mr = s_mr_insts[op];
			util::stream_format(stream, "%s%-3c", mr.first, BIT(inst, 15) ? '*' : ' ');
			const u16 ea = (BIT(inst, 9) ? (pc & 077000) : 0) | (inst & 000777); // form sectored address
			if (!BIT(inst, 9) && ((BIT(inst, 14) && op != 0) || ea == 0))
				format_disp(stream, ea);
			else
				util::stream_format(stream, "'%05o", ea);
			if (BIT(inst, 14) && op != 0)
				stream << ",1";
			return 1 | (BIT(inst, 10, 6) == 041 ? STEP_OUT : mr.second) | SUPPORTED;
		}
	}
	else if (BIT(inst, 14, 2) == 1 && s_sh_insts[BIT(inst, 6, 4)] != nullptr)
	{
		// Shift instructions
		util::stream_format(stream, "%-6s%d", s_sh_insts[BIT(inst, 6, 4)], -inst & 000077);
		return 1 | SUPPORTED;
	}
	else if (BIT(inst, 14, 2) == 2)
	{
		// Skip instructions
		return dasm_skip(stream, inst);
	}
	else
	{
		// Generic instructions
		return dasm_generic(stream, inst);
	}
}

offs_t prime_disassembler::dasm_generic_prime(std::ostream &stream, u16 inst) const
{
	if ((inst >> 6) == 0016)
		util::stream_format(stream, "%-6s'%02o", "WCS", inst & 000077);
	else switch (inst)
	{
	case 000001:
		stream << "NOP";
		break;

	case 000010:
		stream << "E64V";
		break;

	case 000011:
		stream << "E16S";
		break;

	case 000013:
		stream << "E32S";
		break;

	case 000021:
		stream << "RMC";
		break;

	case 000311:
		stream << "VIRY";
		return 1 | STEP_COND | SUPPORTED;

	case 000400:
		stream << "ENBM"; // P850
		break;

	case 000402:
		stream << "ENBP"; // P850
		break;

	case 000405:
		stream << "OTK";
		break;

	case 000411:
		stream << "CAI";
		break;

	case 000415:
		stream << "ESIM";
		break;

	case 000417:
		stream << "EVIM";
		break;

	case 000501:
		stream << "EMCM";
		break;

	case 000503:
		stream << "LMCM";
		break;

	case 000505:
		stream << "SVC";
		return 1 | STEP_OVER | SUPPORTED;

	case 001000:
		stream << "INHM"; // P850
		break;

	case 001002:
		stream << "INHP"; // P850
		break;

	case 001010:
		stream << "E32I";
		break;

	case 001011:
		stream << "E64R";
		break;

	case 001013:
		stream << "E32R";
		break;

	case 0140010:
		stream << "CRL";
		break;

	case 0140014:
		stream << "CRB";
		break;

	case 0140104:
		stream << "XCA";
		break;

	case 0140110:
		stream << "S1A";
		break;

	case 0140114:
		stream << "IRX";
		return 1 | STEP_COND | SUPPORTED;

	case 0140204:
		stream << "XCB";
		break;

	case 0141206:
		stream << "A1A";
		break;

	case 0140210:
		stream << "DRX";
		return 1 | STEP_COND | SUPPORTED;

	case 0140214:
		stream << "CAZ";
		return 1 | STEP_COND | SUPPORTED;

	case 0140304:
		stream << "A2A";
		break;

	case 0140310:
		stream << "S2A";
		break;

	case 0140410:
		stream << "LLT";
		break;

	case 0140411:
		stream << "LLE";
		break;

	case 0140412:
		stream << "LNE";
		break;

	case 0140413:
		stream << "LEQ";
		break;

	case 0140414:
		stream << "LGE";
		break;

	case 0140415:
		stream << "LGT";
		break;

	case 0140416:
		stream << "LF";
		break;

	case 0140417:
		stream << "LT";
		break;

	case 0140510:
		stream << "FSZE";
		return 1 | STEP_COND | SUPPORTED;

	case 0140511:
		stream << "FSNZ";
		return 1 | STEP_COND | SUPPORTED;

	case 0140512:
		stream << "FSMI";
		return 1 | STEP_COND | SUPPORTED;

	case 0140513:
		stream << "FSPL";
		return 1 | STEP_COND | SUPPORTED;

	case 0140514:
		stream << "FSLE";
		return 1 | STEP_COND | SUPPORTED;

	case 0140515:
		stream << "FSGT";
		return 1 | STEP_COND | SUPPORTED;

	case 0140530:
		stream << "FCM";
		break;

	case 0140534:
		stream << "FRN";
		break;

	case 0140570:
		stream << "FRAC";
		break;

	case 0140574:
		stream << "DFCM";
		break;

	default:
		return dasm_generic(stream, inst);
	}
	return 1 | SUPPORTED;
}

offs_t prime_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u16 inst = opcodes.r16(pc);
	if (BIT(inst, 10, 4) != 0)
	{
		if (BIT(inst, 10, 4) == 014)
		{
			// Input-Output group
			if ((inst & 0101777) == 0100020)
			{
				util::stream_format(stream, "%cMK", BIT(inst, 14) ? 'S' : 'I');
				return 1 | SUPPORTED;
			}
			else
			{
				util::stream_format(stream, "%-6s'%04o", s_io_insts[BIT(inst, 14, 2)], inst & 001777);
				return 1 | (BIT(inst, 14, 2) != 0 ? STEP_COND : 0) | SUPPORTED;
			}
		}
		else
		{
			// Memory reference instructions
			const u8 op = BIT(inst, 10, 4) == 015 && BIT(inst, 14) ? 0 : BIT(inst, 10, 4);
			if (BIT(inst, 4, 6) == 060 && c_mode_key >= 2)
			{
				const auto &mr = BIT(inst, 2, 2) == 0 ? s_mr_insts[op] : s_mr_insts_r_mode[op][BIT(inst, 2, 2) - 1];
				if (mr.first != nullptr)
				{
					if (!BIT(inst, 1) || (BIT(inst, 14, 2) == 3 && op != 0))
					{
						// Stack relative/long reach
						const bool preindex = BIT(inst, 14) && !BIT(inst, 1) && op != 0;
						const u16 a = opcodes.r16(pc + 1);
						if (BIT(inst, 2, 2) == 0 && !BIT(inst, 0))
							util::stream_format(stream, "%s%-3c", mr.first, '%');
						else
						{
							util::stream_format(stream, "%-6s", mr.first);
							if (BIT(inst, 0))
								stream << '@';
						}
						if (BIT(inst, 0) || preindex)
						{
							if (s16(a) >= 0 && BIT(inst, 0))
								stream << '+';
							format_disp(stream, a);
						}
						else
							util::stream_format(stream, "'%0*o", c_mode_key == 2 ? 6 : 5, a);
						if (preindex)
						{
							stream << ",X";
							if (BIT(inst, 15))
								stream << '*';
						}
						else if (BIT(inst, 15))
						{
							stream << ",*";
							if (BIT(inst, 1))
								stream << 'X';
						}
						return 2 | mr.second | SUPPORTED;
					}
					else
					{
						// Stack postincrement or predecrement
						util::stream_format(stream, "%-6s", mr.first);
						if (BIT(inst, 0))
							stream << "-@";
						else
							stream << "@+";
						if (BIT(inst, 14) && op != 0)
							stream << ",*1";
						else if (BIT(inst, 15))
							stream << ",*";
						return 1 | mr.second | SUPPORTED;
					}
				}
				else
				{
					util::stream_format(stream, "%-6s%o", "OCT", inst);
					return 1 | SUPPORTED;
				}
			}
			else
			{
				const bool preindex = BIT(inst, 14) && op != 0 && (!BIT(inst, 15) || c_mode_key == 0 || (!BIT(inst, 9) && BIT(inst, 6, 3) == 0));
				const auto &mr = s_mr_insts[op];
				util::stream_format(stream, "%-6s", mr.first);
				if (BIT(inst, 9))
				{
					// Sectored or relative addressing
					const u16 ea = c_mode_key < 2 ? (pc & 077000) | (inst & 000777) : pc + 1 + util::sext(inst, 9);
					if (c_mode_key == 2)
						util::stream_format(stream, "'%06o", ea);
					else
						util::stream_format(stream, "'%05o", ea & 077777);
				}
				else
				{
					// Direct addressing
					const u16 ea = inst & 000777;
					if (preindex || ea == 0)
						format_disp(stream, ea);
					else
						util::stream_format(stream, "'%0*o", c_mode_key == 2 ? 6 : 5, ea);
				}
				if (preindex)
				{
					stream << ",1";
					if (BIT(inst, 15))
						stream << '*';
				}
				else if (BIT(inst, 15))
				{
					stream << ",*";
					if (BIT(inst, 14) && op != 0)
						stream << '1';
				}
				return 1 | (BIT(inst, 10, 6) == 041 ? STEP_OUT : mr.second) | SUPPORTED;
			}
		}
	}
	else if (BIT(inst, 14, 2) == 1 && s_sh_insts_prime[BIT(inst, 6, 4)] != nullptr)
	{
		// Shift instructions
		util::stream_format(stream, "%-6s%d", s_sh_insts_prime[BIT(inst, 6, 4)], -inst & 000077);
		return 1 | SUPPORTED;
	}
	else if (BIT(inst, 14, 2) == 2)
	{
		// Skip instructions
		if (BIT(inst, 7))
		{
			if (BIT(inst, 5, 4) == 5)
				util::stream_format(stream, "S%c%-4c%d", BIT(inst, 4) ? 'A' : 'N', BIT(inst, 9) ? 'S' : 'R', (inst & 000017) + 1);
			else switch (inst & 001777)
			{
			case 0200:
				stream << "SMCR";
				break;

			case 0220:
				stream << "SGT";
				break;

			case 01200:
				stream << "SMCS";
				break;

			case 01220:
				stream << "SLE";
				break;

			default:
				util::stream_format(stream, "%-6s'%04o", "SKP", inst & 001777);
				break;
			}
			return 1 | STEP_COND | SUPPORTED;
		}
		else
			return dasm_skip(stream, inst);
	}
	else
	{
		// Generic instructions
		switch (inst)
		{
		case 000105:
			stream << "RTN";
			return 1 | STEP_OUT | SUPPORTED;

		case 000111:
			stream << "CEA";
			return 1 | SUPPORTED;

		case 000205:
			stream << "PIM";
			return 1 | SUPPORTED;

		case 000211:
			stream << "PID";
			return 1 | SUPPORTED;

		case 000213:
			util::stream_format(stream, "%-6s'%06o", "EPMJ", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000215:
			util::stream_format(stream, "%-6s'%06o", "LPMJ", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000235:
			util::stream_format(stream, "%-6s'%06o", "LPMX", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000237:
			util::stream_format(stream, "%-6s'%06o", "EPMX", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000511:
			stream << "ISI";
			return 1 | SUPPORTED;

		case 000515:
			stream << "OSI";
			return 1 | SUPPORTED;

		case 000701:
			util::stream_format(stream, "%-6s'%06o", "ERMJ", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000703:
			util::stream_format(stream, "%-6s'%06o", "EVMJ", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000721:
			util::stream_format(stream, "%-6s'%06o", "ERMX", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 000723:
			util::stream_format(stream, "%-6s'%06o", "EVMX", opcodes.r16(pc + 1)); // P300
			return 2 | SUPPORTED;

		case 0140550:
			stream << "FLOT";
			return 1 | SUPPORTED;

		case 0140554:
			stream << "INT";
			return 1 | SUPPORTED;

		default:
			return dasm_generic_prime(stream, inst);
		}
	}
}

void prime64v_disassembler::format_ap(std::ostream &stream, u32 ap) const
{
	if (BIT(ap, 24, 2) != 0)
	{
		util::stream_format(stream, "%s%%", s_base_regs_v_mode[BIT(ap, 24, 2)]);
		if (u16(ap) != 0)
		{
			if (!BIT(ap, 15))
				stream << '+';
			format_disp(stream, u16(ap));
		}
	}
	else
		util::stream_format(stream, "'%06o", u16(ap));
	if (BIT(ap, 27))
		stream << ",*";
	else if (BIT(ap, 28, 4) != 0)
		util::stream_format(stream, "+%dB", BIT(ap, 28, 4));
}

offs_t prime64v_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u16 inst = opcodes.r16(pc);
	if (BIT(inst, 10, 4) != 0)
	{
		// Memory reference instructions
		const u8 op = BIT(inst, 10, 4) == 015 && BIT(inst, 14) ? 0 : BIT(inst, 10, 4);
		if (BIT(inst, 5, 5) == 030)
		{
			const bool quad = op == 5 && BIT(inst, 2, 2) == 2;
			const auto &mr = quad ? s_quad_insts[std::min<u16>(opcodes.r16(pc + 2), 7)] : s_mr_insts_v_mode[op][BIT(inst, 2, 2)];
			if (mr.first != nullptr)
			{
				const bool preindex = (BIT(inst, 14, 2) != 0 ? !BIT(inst, 4) : BIT(inst, 4)) && op != 0;
				const u16 d = opcodes.r16(pc + 1);
				if (!BIT(inst, 15) && BIT(inst, 0, 5) < 3)
					util::stream_format(stream, "%s%-3c", mr.first, '%');
				else
					util::stream_format(stream, "%-6s", mr.first);
				if (BIT(inst, 0, 2) != 0)
				{
					util::stream_format(stream, "%s%%", s_base_regs_v_mode[BIT(inst, 0, 2)]);
					if (d != 0)
					{
						if (s16(d) > 0)
							stream << '+';
						format_disp(stream, d);
					}
				}
				else if (preindex && (op != 12 || BIT(inst, 2, 2) != 1))
					format_disp(stream, d);
				else
					util::stream_format(stream, "'%06o", d);
				if (preindex)
				{
					if (BIT(inst, 14))
						stream << ",X";
					else
						stream << ",Y";
					if (BIT(inst, 15))
						stream << '*';
				}
				else if (op == 0 ? BIT(inst, 15) : BIT(inst, 14, 2) != 0)
				{
					stream << ",*";
					if (BIT(inst, 15) && op != 0)
					{
						if (BIT(inst, 14))
							stream << 'X';
						else
							stream << 'Y';
					}
				}
				return (quad ? 3 : 2) | ((inst == 003403 || inst == 003420) && d == 0 ? STEP_OUT : mr.second) | SUPPORTED;
			}
			else
			{
				util::stream_format(stream, "%-6s%o", "OCT", inst);
				return 1 | SUPPORTED;
			}
		}
		else
		{
			// Short modes
			const auto &mr = s_mr_insts_v_mode[op][0];
			util::stream_format(stream, "%s%-3c", mr.first, BIT(inst, 15) ? '#' : ' ');
			if (BIT(inst, 9))
			{
				const u16 ea = pc + 1 + util::sext(inst, 9); // form relative address
				util::stream_format(stream, "'%06o", ea);
				if (BIT(inst, 14) && op != 0)
				{
					stream << ",1";
					if (BIT(inst, 15))
						stream << '*';
				}
				else if (BIT(inst, 15))
					stream << ",*";
			}
			else
			{
				if (!BIT(inst, 15) && ((BIT(inst, 14) && op != 0) || BIT(inst, 3, 6) != 0))
				{
					util::stream_format(stream, "%s%%+", s_base_regs_v_mode[BIT(inst, 8) ? 2 : 1]);
					format_disp(stream, inst & 000777);
					if (BIT(inst, 14) && op != 0)
						stream << ",X";
				}
				else if (BIT(inst, 14, 2) == 3 && op != 0 && BIT(inst, 6, 3) == 0)
				{
					format_disp(stream, inst & 000077);
					stream << ",X*";
				}
				else
				{
					if (BIT(inst, 3, 6) == 0)
						util::stream_format(stream, "%o", inst & 000777);
					else
						util::stream_format(stream, "'%06o", inst & 000777);
					if (BIT(inst, 15))
					{
						stream << ",*";
						if (BIT(inst, 14) && op != 0)
							stream << '1';
					}
				}
			}
			return 1 | (BIT(inst, 10, 6) == 041 ? STEP_OUT : mr.second) | SUPPORTED;
		}
	}
	else if (BIT(inst, 14, 2) == 1)
	{
		// Shift instructions + FOP
		if (s_sh_insts_prime[BIT(inst, 6, 4)] != nullptr)
			util::stream_format(stream, "%-6s%d", s_sh_insts_prime[BIT(inst, 6, 4)], -inst & 000077);
		else switch (inst & 01777)
		{
		case 0300:
			stream << "DRN";
			break;

		case 0301:
			stream << "DRNP";
			break;

		case 0302:
			stream << "DRNZ";
			break;

		case 0303:
			stream << "FRNP";
			break;

		case 0310:
			stream << "SSSN";
			break;

		case 0320:
			stream << "FRNM";
			break;

		case 0321:
			stream << "FRNZ";
			break;

		default:
			util::stream_format(stream, "%-6s%o", "OCT", inst);
			break;
		}
		return 1 | SUPPORTED;
	}
	else if (BIT(inst, 14, 2) == 2)
	{
		// Skip instructions
		if (BIT(inst, 7))
		{
			if (BIT(inst, 5, 4) == 5)
				util::stream_format(stream, "S%c%-4c%d", BIT(inst, 4) ? 'A' : 'N', BIT(inst, 9) ? 'S' : 'R', (inst & 000017) + 1);
			else switch (inst & 001777)
			{
			case 0200:
				stream << "SMCR";
				break;

			case 0220:
				stream << "SGT";
				break;

			case 01200:
				stream << "SMCS";
				break;

			case 01220:
				stream << "SLE";
				break;

			default:
				util::stream_format(stream, "%-6s'%04o", "SKP", inst & 001777);
				break;
			}
			return 1 | STEP_COND | SUPPORTED;
		}
		else
			return dasm_skip(stream, inst);
	}
	else
	{
		// Generic instructions
		switch (inst)
		{
		case 000015:
			stream << "PIMA";
			break;

		case 000024:
			stream << "STPM";
			break;

		case 000044:
			util::stream_format(stream, "%-6s", "LIOT");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 000064:
			stream << "PTLB";
			break;

		case 000115:
			stream << "PIDA";
			break;

		case 000301:
			stream << "PIML";
			break;

		case 000305:
			stream << "PIDL";
			break;

		case 000315:
			util::stream_format(stream, "%-6s", "WAIT");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 000510:
			stream << "STTM";
			break;

		case 000511:
			stream << "RTS";
			break;

		case 000601:
			stream << "IRTN";
			return 1 | STEP_OUT | SUPPORTED;

		case 000603:
			stream << "IRTC";
			return 1 | STEP_OUT | SUPPORTED;

		case 000605:
			stream << "ARGT";
			break;

		case 000611:
			stream << "PRTN";
			return 1 | STEP_OUT | SUPPORTED;

		case 000615:
			stream << "ITLB";
			break;

		case 000617:
			stream << "LPID";
			break;

		case 000705:
			util::stream_format(stream, "%-6s", "CALF");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | STEP_OVER | SUPPORTED;

		case 000711:
			util::stream_format(stream, "%-6s", "LPSW");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 000715:
			util::stream_format(stream, "%-6s", "RSAV");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 000717:
			util::stream_format(stream, "%-6s", "RRST");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001005:
			stream << "TKA";
			break;

		case 001015:
			stream << "TAK";
			break;

		case 001100:
			stream << "XAD";
			break;

		case 001101:
			stream << "XMV";
			break;

		case 001102:
			stream << "XCM";
			break;

		case 001104:
			stream << "XMP";
			break;

		case 001107:
			stream << "XDV";
			break;

		case 001110:
			stream << "ZTRN";
			break;

		case 001111:
			stream << "ZED";
			break;

		case 001112:
			stream << "XED";
			break;

		case 001113:
			stream << "XVRY"; // P500
			break;

		case 001114:
			stream << "ZMV";
			break;

		case 001115:
			stream << "ZMVD";
			break;

		case 001116:
			stream << "ZFIL";
			break;

		case 001117:
			stream << "ZCM";
			break;

		case 001145:
			stream << "XBTD";
			break;

		case 001146:
			stream << "XDTB";
			break;

		case 001200:
			util::stream_format(stream, "%-6s", "STAC");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001204:
			util::stream_format(stream, "%-6s", "STLC");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001210:
			util::stream_format(stream, "%-6s", "NFYE");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001211:
			util::stream_format(stream, "%-6s", "NFYB");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001214:
			util::stream_format(stream, "%-6s", "INEN");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001215:
			util::stream_format(stream, "%-6s", "INBN");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001216:
			util::stream_format(stream, "%-6s", "INEC");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001217:
			util::stream_format(stream, "%-6s", "INBC");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001300: case 001310:
			util::stream_format(stream, "%-6s%d,", "EAFA", BIT(inst, 3));
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001301: case 001311:
			util::stream_format(stream, "%-6s%d", "ALFA", BIT(inst, 3));
			break;

		case 001302: case 001312:
			util::stream_format(stream, "%-6s%d", "LDC", BIT(inst, 3));
			break;

		case 001303: case 001313:
		{
			util::stream_format(stream, "%-6s%d,", "LFLI", BIT(inst, 3));
			const u16 n = opcodes.r16(pc + 1);
			if (n > 7)
				stream << '\'';
			util::stream_format(stream, "%o", n);
			return 2 | SUPPORTED;
		}

		case 001304:
			stream << "MDEI";
			break;

		case 001305:
			stream << "MDII";
			break;

		case 001306:
			stream << "MDRS";
			break;

		case 001307:
			stream << "MDWC";
			break;

		case 001314:
			stream << "CGT";
			break;

		case 001315:
			stream << "STEX";
			break;

		case 001320: case 001330:
			util::stream_format(stream, "%-6s%d,", "STFA", BIT(inst, 3));
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 001321: case 001331:
			util::stream_format(stream, "%-6s%d", "TLFL", BIT(inst, 3));
			break;

		case 001322: case 001332:
			util::stream_format(stream, "%-6s%d", "STC", BIT(inst, 3));
			break;

		case 001323: case 001333:
			util::stream_format(stream, "%-6s%d", "TFLL", BIT(inst, 3));
			break;

		case 001324:
			stream << "MDIW";
			break;

		case 001710:
			stream << "LWCS";
			break;

		case 001714:
			stream << "CXCS";
			break;

		case 0140015:
			stream << "CRB"; // old instruction with new opcode
			break;

		case 0140016:
			stream << "FDBL";
			break;

		case 0140314:
			stream << "TAB";
			break;

		case 0140504:
			stream << "TAX";
			break;

		case 0140505:
			stream << "TAY";
			break;

		case 0140531:
			stream << "INTA";
			break;

		case 0140532:
			stream << "FLTA";
			break;

		case 0140533:
			stream << "INTL";
			break;

		case 0140535:
			stream << "FLTL";
			break;

		case 0140570:
			stream << "QFCM";
			break;

		case 0140571:
			stream << "FCDQ";
			break;

		case 0140572:
			stream << "QINQ";
			break;

		case 0140573:
			stream << "QIQR";
			break;

		case 0140604:
			stream << "TBA";
			break;

		case 0140610:
			util::stream_format(stream, "%-6s'%06o", "BLE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140611:
			util::stream_format(stream, "%-6s'%06o", "BGT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140612:
			util::stream_format(stream, "%-6s'%06o", "BEQ", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140613:
			util::stream_format(stream, "%-6s'%06o", "BNE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140614:
			util::stream_format(stream, "%-6s'%06o", "BLT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140615:
			util::stream_format(stream, "%-6s'%06o", "BGE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140700:
			util::stream_format(stream, "%-6s'%06o", "BLLE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140701:
			util::stream_format(stream, "%-6s'%06o", "BLGT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140702:
			util::stream_format(stream, "%-6s'%06o", "BLEQ", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140703:
			util::stream_format(stream, "%-6s'%06o", "BLNE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140724:
			util::stream_format(stream, "%-6s'%06o", "BDY", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0140734:
			util::stream_format(stream, "%-6s'%06o", "BDX", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141000:
			stream << "ADDL";
			break;

		case 0141034:
			stream << "TXA";
			break;

		case 0141110:
			stream << "LFLT";
			break;

		case 0141111:
			stream << "LFLE";
			break;

		case 0141112:
			stream << "LFNE";
			break;

		case 0141113:
			stream << "LFEQ";
			break;

		case 0141114:
			stream << "LFGE";
			break;

		case 0141115:
			stream << "LFGT";
			break;

		case 0141124:
			stream << "TYA";
			break;

		case 0141210:
			stream << "TCL";
			break;

		case 0141324:
			util::stream_format(stream, "%-6s'%06o", "BIY", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141334:
			util::stream_format(stream, "%-6s'%06o", "BIX", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141404:
			stream << "CRE";
			break;

		case 0141410:
			stream << "CRLE";
			break;

		case 0141414:
			stream << "ILE";
			break;

		case 0141500:
			stream << "LCLT";
			break;

		case 0141501:
			stream << "LCLE";
			break;

		case 0141502:
			stream << "LCNE";
			break;

		case 0141503:
			stream << "LCEQ";
			break;

		case 0141504:
			stream << "LCGE";
			break;

		case 0141505:
			stream << "LCGT";
			break;

		case 0141511:
			stream << "LLLE";
			break;

		case 0141512:
			stream << "LLNE";
			break;

		case 0141513:
			stream << "LLEQ";
			break;

		case 0141515:
			stream << "LLGT";
			break;

		case 0141600:
			util::stream_format(stream, "%-6s'%06o", "BCLE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141601:
			util::stream_format(stream, "%-6s'%06o", "BCGT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141602:
			util::stream_format(stream, "%-6s'%06o", "BCEQ", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141603:
			util::stream_format(stream, "%-6s'%06o", "BCNE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141604:
			util::stream_format(stream, "%-6s'%06o", "BCLT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141605:
			util::stream_format(stream, "%-6s'%06o", "BCGE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141610:
			util::stream_format(stream, "%-6s'%06o", "BFLE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141611:
			util::stream_format(stream, "%-6s'%06o", "BFGT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141612:
			util::stream_format(stream, "%-6s'%06o", "BFEQ", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141613:
			util::stream_format(stream, "%-6s'%06o", "BFNE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141614:
			util::stream_format(stream, "%-6s'%06o", "BFLT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141615:
			util::stream_format(stream, "%-6s'%06o", "BFGE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141704:
			util::stream_format(stream, "%-6s'%06o", "BCS", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141705:
			util::stream_format(stream, "%-6s'%06o", "BCR", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141706:
			util::stream_format(stream, "%-6s'%06o", "BLS", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141707:
			util::stream_format(stream, "%-6s'%06o", "BLR", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141710:
			util::stream_format(stream, "%-6s'%06o", "BMGT", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141711:
			util::stream_format(stream, "%-6s'%06o", "BMLE", opcodes.r16(pc + 1));
			return 2 | STEP_COND | SUPPORTED;

		case 0141714:
			util::stream_format(stream, "%-6s", "RTQ");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 0141715:
			util::stream_format(stream, "%-6s", "RBQ");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 0141716:
			util::stream_format(stream, "%-6s", "ABQ");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 0141717:
			util::stream_format(stream, "%-6s", "ATQ");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		case 0141757:
			util::stream_format(stream, "%-6s", "TSTQ");
			format_ap(stream, opcodes.r32(pc + 1));
			return 3 | SUPPORTED;

		default:
			return dasm_generic_prime(stream, inst);
		}
		return 1 | SUPPORTED;
	}
}
