// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor DP8344 BCP disassembler

***************************************************************************/

#include "emu.h"
#include "bcpdasm.h"


//**************************************************************************
//  BCP DISASSEMBLER
//**************************************************************************

dp8344_disassembler::dp8344_disassembler()
	: util::disasm_interface()
{
}

u32 dp8344_disassembler::opcode_alignment() const
{
	return 1;
}

const char *dp8344_disassembler::cc_to_string(u8 f, bool s) const
{
	// f = 0, s = 1: Zero or EQual
	// f = 0, s = 0: Not Zero or Not EQual
	// f = 1, s = 1: Carry
	// f = 1, s = 0: No Carry
	// f = 2, s = 1: Overflow
	// f = 2, s = 0: No Overflow
	// f = 3, s = 1: Negative
	// f = 3, s = 0: Positive
	// f = 4, s = 1: Receiver Active
	// f = 4, s = 0: Not Receiver Active
	// f = 5, s = 1: Receiver Error
	// f = 5, s = 0: No Receiver Error
	// f = 6, s = 1: Data Available
	// f = 6, s = 0: No Data Available
	// f = 7, s = 1: Transmitter FIFO Full
	// f = 7, s = 0: Transmitter FIFO Not Full

	return std::array<const char *, 16> {
		"nz", "nc", "nv", "p", "nra", "nre", "ndav", "ntss",
		"z", "c", "v", "n", "ra", "re", "dav", "tss"
	}[f | (s ? 8 : 0)];
}

const char *dp8344_disassembler::aop_to_string(u8 o) const
{
	return std::array<const char *, 8> {
		"adda", "adca", "suba", "sbca", "anda", "ora", "xora", "move"
	}[o];
}

void dp8344_disassembler::format_number(std::ostream &stream, u8 n) const
{
	if (n >= 0xa0)
		stream << "0";
	util::stream_format(stream, "%02Xh", n);
}

void dp8344_disassembler::format_address(std::ostream &stream, u16 nn) const
{
	if (nn >= 0xa000)
		stream << "0";
	util::stream_format(stream, "%04Xh", nn);
}

void dp8344_disassembler::format_register(std::ostream &stream, u8 r) const
{
	stream << std::array<const char *, 32> {
		"CCR/DCR", "NCF/IBR", "ICR/ATR", "ACR/FBR",
		"GP0/RTR", "GP1/TSR", "GP2/TCR", "GP3/TMR",
		"GP4/GP4'", "GP5/GP5'", "GP6/GP6'", "GP7/GP7'",
		"IWLO", "IWHI", "IXLO", "IXHI",
		"IYLO", "IYHI", "IZLO", "IZHI",
		"GP8", "GP9", "GP10", "GP11",
		"GP12", "GP13", "GP14", "GP15",
		"TRL", "TRH", "ISP", "DS"
	}[r];
}

void dp8344_disassembler::format_modified_index_register(std::ostream &stream, u8 ir, u8 m) const
{
	if (m == 3)
		stream << "+";
	util::stream_format(stream, "I%c", 'W' + ir);
	if (m == 0)
		stream << "-";
	else if (m == 2)
		stream << "+";
}

void dp8344_disassembler::format_interrupt_vector(std::ostream &stream, u8 v) const
{
	switch (v)
	{
	case 0x1c:
		stream << "NMI";
		break;

	case 0x04:
		stream << "RFF/DA/RA";
		break;

	case 0x08:
		stream << "TFE";
		break;

	case 0x0c:
		stream << "LTE";
		break;

	case 0x10:
		stream << "BIRQ";
		break;

	case 0x14:
		stream << "TO";
		break;

	default:
		format_number(stream, v);
		break;
	}
}

offs_t dp8344_disassembler::disassemble(std::ostream &stream, offs_t pc, const dp8344_disassembler::data_buffer &opcodes, const dp8344_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	offs_t words = 1;

	switch (inst & 0xf000)
	{
	case 0x0000:
		// 0000-0FFF: ADD n,rsd (2 T-states)
		util::stream_format(stream, "%-8s", "add");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << ",";
		format_register(stream, inst & 0x000f);
		break;

	case 0x1000:
		// 1000-1FFF: MOVE rs,[IZ+n] (3 T-states)
		util::stream_format(stream, "%-8s", "move");
		format_register(stream, inst & 0x000f);
		stream << ",[IZ+";
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << "]";
		break;

	case 0x2000:
		// 2000-2FFF: SUB n,rsd (2 T-states)
		util::stream_format(stream, "%-8s", "sub");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << ",";
		format_register(stream, inst & 0x000f);
		break;

	case 0x3000:
		// 3000-3FFF: CMP rs,n (2 T-states)
		util::stream_format(stream, "%-8s", "cmp");
		format_register(stream, inst & 0x000f);
		stream << ",";
		format_number(stream, (inst & 0x0ff0) >> 4);
		break;

	case 0x4000:
		// 4000-4FFF: AND n,rsd (2 T-states)
		util::stream_format(stream, "%-8s", "and");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << ",";
		format_register(stream, inst & 0x000f);
		break;

	case 0x5000:
		// 5000-5FFF: OR n,rsd (2 T-states)
		util::stream_format(stream, "%-8s", "or");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << ",";
		format_register(stream, inst & 0x000f);
		break;

	case 0x6000:
		// 6000-6FFF: XOR n,rsd (2 T-states)
		util::stream_format(stream, "%-8s", "xor");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << ",";
		format_register(stream, inst & 0x000f);
		break;

	case 0x7000:
		// 7000-7FFF: BIT rs,n (2 T-states)
		util::stream_format(stream, "%-8s", "bit");
		format_register(stream, inst & 0x000f);
		stream << ",";
		format_number(stream, (inst & 0x0ff0) >> 4);
		break;

	case 0x8000:
		if (!BIT(inst, 11))
		{
			// 8000-87FF: JRMK Rs,b,m (4 T-states)
			util::stream_format(stream, "%-8s", "jrmk");
			format_register(stream, inst & 0x001f);
			util::stream_format(stream, ",%d,%d", (inst & 0x00e0) >> 5, (inst & 0x0700) >> 8);
		}
		else if (!BIT(inst, 10))
		{
			// 8800-8BFF: MOVE n,[Ir] (3 T-states)
			util::stream_format(stream, "%-8s", "move");
			format_number(stream, (inst & 0x0380) >> 2 | (inst & 0x001f));
			util::stream_format(stream, ",[I%c]", 'W' + ((inst & 0x0060) >> 5));
		}
		else
		{
			words = 2;
			if (!BIT(inst, 9))
			{
				// 8C00-8DFF, 0000-FFFF: LJMP Rs,p,s,nn (2 + 2 T-states)
				util::stream_format(stream, "%-8s", "ljmp");
				words |= STEP_COND;
			}
			else
			{
				// 8E00-8FFF, 0000-FFFF: LCALL Rs,p,s,nn (2 + 2 T-states)
				util::stream_format(stream, "%-8s", "lcall");
				words |= STEP_OVER | STEP_COND;
			}
			format_register(stream, inst & 0x001f);
			util::stream_format(stream, ",%d,%d,", (inst & 0x00e0) >> 5, BIT(inst, 8));
			format_address(stream, opcodes.r16(pc + 1));
		}
		break;

	case 0x9000:
		// 9000-9FFF: MOVE [IZ+n],rd (3 T-states)
		util::stream_format(stream, "%-8s[IZ+", "move");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << "],";
		format_register(stream, inst & 0x000f);
		break;

	case 0xa000:
	{
		if (inst < 0xae00)
		{
			// A000-A1FF: ADDA Rs,[mIr] (3 T-states)
			// A200-A3FF: ADCA Rs,[mIr] (3 T-states)
			// A400-A5FF: SUBA Rs,[mIr] (3 T-states)
			// A600-A7FF: SBCA Rs,[mIr] (3 T-states)
			// A800-A9FF: ANDA Rs,[mIr] (3 T-states)
			// AA00-ABFF: ORA  Rs,[mIr] (3 T-states)
			// AC00-ADFF: XORA Rs,[mIr] (3 T-states)
			util::stream_format(stream, "%-8s", aop_to_string((inst & 0x0e00) >> 9));
			format_register(stream, inst & 0x001f);
			stream << ",[";
			format_modified_index_register(stream, (inst & 0x0060) >> 5, (inst & 0x0180) >> 7);
			stream << "]";
		}
		else if (!BIT(inst, 8))
		{
			if (!BIT(inst, 7))
			{
				// AE00-AE1F: CPL Rsd (2 T-states)
				util::stream_format(stream, "%-8s", "cpl");
				format_register(stream, inst & 0x001f);
			}
			else
			{
				// AE80-AEF8: EXX ba,bb{,g} (2 T-states)
				util::stream_format(stream, "%-8s%cA,%cB", "exx", BIT(inst, 4) ? 'A' : 'M', BIT(inst, 3) ? 'A' : 'M');
				switch (inst & 0x0060)
				{
				case 0x0000: // NCHG
					break;

				case 0x0020:
					stream << ",RI";
					break;

				case 0x0040:
					stream << ",EI";
					break;

				case 0x0060:
					stream << ",DI";
					break;
				}
			}
		}
		else
		{
			// AF00-A7FF: RETF f,s{,{g}{,rf}} (2 or 3 T-states)
			// AF80-AFF0: RET  {g{,rf}} (2 T-states)
			if (!BIT(inst, 7))
			{
				util::stream_format(stream, (inst & 0x0070) == 0 ? "r%s" : "r%-7s", cc_to_string(inst & 0x0007, BIT(inst, 3)));
				words |= STEP_COND;
			}
			else
				util::stream_format(stream, (inst & 0x0070) == 0 ? "%s" : "%-8s", "ret");

			switch (inst & 0x0060)
			{
			case 0x0000:
				if (BIT(inst, 4))
					stream << "NCHG";
				break;

			case 0x0020:
				stream << "RI";
				break;

			case 0x0040:
				stream << "EI";
				break;

			case 0x0060:
				stream << "DI";
				break;
			}
			if (BIT(inst, 4))
				stream << ",RF";

			words |= STEP_OUT;
		}
		break;
	}

	case 0xb000:
		// B000-BFFF: MOVE n,rd (2 T-states)
		util::stream_format(stream, "%-8s", "move");
		format_number(stream, (inst & 0x0ff0) >> 4);
		stream << ",";
		format_register(stream, inst & 0x000f);
		break;

	case 0xc000:
		switch (inst & 0xf80)
		{
		case 0x000: case 0x080: case 0x100: case 0x180:
			// C000-C1FF: MOVE [mIr],Rd (3 T-states)
			util::stream_format(stream, "%-8s[", "move");
			format_modified_index_register(stream, (inst & 0x0060) >> 5, (inst & 0x0180) >> 7);
			stream << "],";
			format_register(stream, inst & 0x001f);
			break;

		case 0x200: case 0x280: case 0x300: case 0x380:
			// C200-C3FF: MOVE Rs,[mIr] (3 T-states)
			util::stream_format(stream, "%-8s", "move");
			format_register(stream, inst & 0x001f);
			stream << ",[";
			format_modified_index_register(stream, (inst & 0x0060) >> 5, (inst & 0x0180) >> 7);
			stream << "]";
			break;

		case 0x400:
			// C400-C47F: MOVE [Ir+A],Rd (3 T-states)
			util::stream_format(stream, "%-8s[I%c+A],", "move", 'W' + ((inst & 0x0060) >> 5));
			format_register(stream, inst & 0x001f);
			break;

		case 0x480:
			// C480-C4FF: MOVE Rs,[Ir+A] (3 T-states)
			util::stream_format(stream, "%-8s", "move");
			format_register(stream, inst & 0x001f);
			util::stream_format(stream, ",[I%c+A]", 'W' + ((inst & 0x0060) >> 5));
			break;

		case 0x800: case 0x880:
			// C800-C8FF: SHR Rsd,b (2 T-states)
			util::stream_format(stream, "%-8s", "shr");
			format_register(stream, inst & 0x001f);
			util::stream_format(stream, ",%d", (inst & 0x00e0) >> 5);
			break;

		case 0x900: case 0x980:
			// C900-C9FF: SHL Rsd,b (2 T-states)
			util::stream_format(stream, "%-8s", "shl");
			format_register(stream, inst & 0x001f);
			util::stream_format(stream, ",%d", (inst & 0x00e0) == 0 ? 0 : 8 - ((inst & 0x00e0) >> 5));
			break;

		case 0xa00: case 0xa80:
			// CA00-CAFF: ROT Rsd,b (2 T-states)
			util::stream_format(stream, "%-8s", "rot");
			format_register(stream, inst & 0x001f);
			util::stream_format(stream, ",%d", (inst & 0x00e0) >> 5);
			break;

		case 0xb00: case 0xb80:
			// CB00-CBFF: JMP n (3 T-states)
			util::stream_format(stream, "%-8s", "jmp");
			format_address(stream, pc + 1 + s8(inst & 0x00ff));
			break;

		case 0xc00: case 0xc80:
			// CC00-CCFF: CALL n (3 T-states)
			util::stream_format(stream, "%-8s", "call");
			format_address(stream, pc + 1 + s8(inst & 0x00ff));
			words |= STEP_OVER;
			break;

		case 0xd00:
			// CD00-DC60: LJMP [Ir] (2 T-states)
			util::stream_format(stream, "%-8s[I%c]", "ljmp", 'W' + ((inst & 0x0060) >> 5));
			break;

		case 0xd80:
			// CD80-CD9F: JMP Rs (4 T-states)
			util::stream_format(stream, "%-8s", "jmp");
			format_register(stream, inst & 0x001f);
			break;

		case 0xe00:
			// CE00: LJMP nn (2 + 2 T-states)
			util::stream_format(stream, "%-8s", "ljmp");
			format_address(stream, opcodes.r16(pc + 1));
			words = 2;
			break;

		case 0xe80:
			// CE80: LCALL nn (2 + 2 T-states)
			util::stream_format(stream, "%-8s", "lcall");
			format_address(stream, opcodes.r16(pc + 1));
			words = 2 | STEP_OVER;
			break;

		case 0xf80:
			// CF80-CFFF: TRAP v{,g'} (2 T-states)
			util::stream_format(stream, "%-8s", "trap");
			format_interrupt_vector(stream, inst & 0x003f);
			if (BIT(inst, 6))
				stream << ",DI";
			words |= STEP_OVER;
			break;

		default:
			// invalid
			stream << "???";
			break;
		}
		break;

	case 0xd000:
		// D000-DFFF: JMP f,s,n (2 or 3 T-states)
		util::stream_format(stream, "j%-7s", cc_to_string((inst & 0x0700) >> 8, BIT(inst, 11)));
		format_address(stream, pc + 1 + s8(inst & 0x00ff));
		words |= STEP_COND;
		break;

	case 0xe000: case 0xf000:
		// E000-E3FF: ADDA Rs,Rd (2 T-states)
		// E400-E7FF: ADCA Rs,Rd (2 T-states)
		// E800-EBFF: SUBA Rs,Rd (2 T-states)
		// EC00-EFFF: SBCA Rs,Rd (2 T-states)
		// F000-F3FF: ANDA Rs,Rd (2 T-states)
		// F400-F7FF: ORA  Rs,Rd (2 T-states)
		// F800-FBFF: XORA Rs,Rd (2 T-states)
		// FC00-FFFF: MOVE Rs,Rd (2 T-states)
		util::stream_format(stream, "%-8s", aop_to_string((inst & 0x1c00) >> 10));
		format_register(stream, inst & 0x001f);
		stream << ",";
		format_register(stream, (inst & 0x03e0) >> 5);
		break;
	}

	return words | SUPPORTED;
}
