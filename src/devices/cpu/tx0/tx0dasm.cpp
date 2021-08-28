// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    MIT TX-0 disassembler

    Mnemonics and relative timing of operate class micro-instructions
    until mid-1960 (as given in M-5001-27):

    0.8 | 700000 (CLL) Clear left 9 bits of AC
    0.8 | 640000 (CLR) Clear right 9 bits of AC
    ----+--------------------------------------------------------
    IOS | 610000 (EX0) Operate external equipment 0
    IOS | 611000 (EX1) Operate external equipment 1
    IOS | 612000 (EX2) Operate external equipment 2
    IOS | 613000 (EX3) Operate external equipment 3
    IOS | 614000 (EX4) Operate external equipment 4
    IOS | 615000 (EX5) Operate external equipment 5
    IOS | 616000 (EX6) Operate external equipment 6
    IOS | 617000 (EX7) Operate external equipment 7
    IOS | 621000 (R1L) Read one line of tape from PETR into AC
    IOS | 622000 (DIS) Display point on scope (coordinates in AC)
    IOS | 623000 (R3L) Read three lines of tape from PETR into AC
    IOS | 624000 (PRT) Print character on Flexowriter
    IOS | 626000 (P6H) Punch six holes (as specified by AC)
    IOS | 627000 (P7H) Punch seven holes (as specified by AC)
    ----+--------------------------------------------------------
    1.1 |              Clear MBR
    1.1 | 600100 (PEN) Set AC bits 0 and 1 from LP0 and LP1
    1.1 | 600004 (TAC) Transfer (inclusive or) TAC to AC
    1.2 | 600001 (AMB) Transfer AC to MBR (executed before COM)
    1.2 | 600040 (COM) Complement AC
    1.2 | 600003 (TBR) Transfer (inclusive or) TBR to MBR
    1.3 | 600200 (MLR) Transfer MBR to LR
    1.3 | 600002 (LMB) Transfer LR to MBR
    1.3 | 600104 (ORL) Inclusive or MBR into LR
    1.3 | 600304 (ANL) And MBR into LR
    1.4 | 600020 (PAD) Partial add (exclusive or) MBR to AC
    1.4 | 600400 (SHR) Shift AC right once
    1.4 | 600600 (CYR) Cycle AC right once
    1.7 | 600010 (CRY) Add carry digits to AC (according to LR)
    1.8 | 603000 (HLT) Halt computer

    ANL and ORL were added in the first half of 1959 (M-5001-6).

    Mnemonics and relative timing of operate class micro-instructions
    after 1960 (as given in M-5001-27-3 and M-5001-27-4):

    0.6 | 631000 (CLL) Clear left 9 bits of AC
    0.6 | 632000 (CLR) Clear right 9 bits of AC
    0.7 | 640000 (AMB) Transfer AC to MBR
    0.8 | 700000 (CLA) Clear AC
    ----+------------------------------------------------------------
    **  | 604000 (SEL) Select magnetic tape device
        | 6x4x00       Backspace tape
        | 6x4x04       Read select tape
        | 6x4x10       Rewind tape
        | 6x4x14       Write select tape
    **  | 620000 (CPY) Synchronize with copy (transfer to or from LR)
    ----+------------------------------------------------------------
    1.1 | 601000 (TAC) Transfer (inclusive or) TAC to AC
    1.1 | 603000 (PEN) Set AC bits 0 and 1 from LP0 and LP1
    1.2 | 600100 (XMB) Transfer XR to MBR (with sign extension)
    1.2 | 600040 (COM) Complement AC
    1.2 | 602000 (TBR) Transfer (inclusive or) TBR to MBR
    1.2 | 606000 (RPF) Read (inclusive or) PFR into MBR
    1.3 | 600005 (ORB) Inclusive or LR into MBR
    1.3 | 600007 (ANB) And LR into MBR
    1.4 | 600200 (MBL) Transfer MBR to LR
    1.4 | 600002 (LMB) Transfer LR to MBR
    1.5 | 600020 (PAD) Partial add (exclusive or) MBR to AC
    1.6 | 600400 (SHR) Shift AC right once
    1.6 | 600600 (CYR) Cycle AC right once
    1.6 | 607000 (SPF) Set PFR from MBR
    1.7 | 600010 (CRY) Add carry digits to AC (according to LR)
    1.8 | 600001 (MBX) Transfer MBR to XR
    1.8 | 603000 (HLT) Halt computer

    Previously supported Input-Output Stop group codes were unchanged.

***************************************************************************/

#include "emu.h"
#include "tx0dasm.h"

u32 tx0_64kw_disassembler::opcode_alignment() const
{
	return 1;
}

u32 tx0_8kw_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

const std::string_view s_addressable_insts[24] =
{
	"sto", "stx", "sxa", "ado", "slr", "slx", "stz", {},
	"add", "adx", "ldx", "aux", "llr", "llx", "lda", "lax",
	"trn", "tze", "tsx", "tix", "tra", "trx", "tlv", {}
};

const std::string_view s_tape_insts[8] =
{
	"bsr", "rtb", "rew", "wtb",
	"bsr", "rtd", "rew", "wtd"
};

} // anonymous namespace

offs_t tx0_64kw_disassembler::disassemble(std::ostream &stream, offs_t pc, const tx0_64kw_disassembler::data_buffer &opcodes, const tx0_64kw_disassembler::data_buffer &params)
{
	u32 inst = opcodes.r32(pc) & 0777777;

	if (inst < 0600000)
	{
		// Addressable instructions (only 3 in this version)
		util::stream_format(stream, "%s %06o", s_addressable_insts[(inst & 0600000) >> 13], inst & 0177777);
	}
	else switch (inst)
	{
	case 0600012:
		stream << "cry";
		break;

	case 0600022:
		stream << "lpd";
		break;

	case 0600031:
		stream << "cyl";
		break;

	case 0600032:
		stream << "lad";
		break;

	case 0600040:
		stream << "com";
		break;

	case 0600051:
		stream << "amz";
		break;

	case 0600100:
		stream << "pen";
		break;

	case 0600200:
		stream << "lro";
		break;

	case 0600201:
		stream << "alr";
		break;

	case 0600221:
		stream << "ala";
		break;

	case 0600261:
		stream << "alc";
		break;

	case 0600400:
		stream << "shr";
		break;

	case 0600600:
		stream << "cyr";
		break;

	case 0622000:
		stream << "dis";
		break;

	case 0622021:
		stream << "dsa";
		break;

	case 0622061:
		stream << "dsc";
		break;

	case 0624000:
		stream << "prt";
		break;

	case 0624021:
		stream << "pna";
		break;

	case 0624061:
		stream << "pnc";
		break;

	case 0624600:
		stream << "pnt";
		break;

	case 0626021: case 0627021:
		util::stream_format(stream, "p%da", BIT(inst, 9) ? 7 : 6);
		break;

	case 0626600: case 0627600:
		util::stream_format(stream, "p%dh", BIT(inst, 9) ? 7 : 6);
		break;

	case 0630000:
		stream << "hlt";
		break;

	case 0640000:
		stream << "clr";
		break;

	case 0666020: case 0667020:
		util::stream_format(stream, "p%do", BIT(inst, 9) ? 7 : 6);
		break;

	case 0700000:
		stream << "cll";
		break;

	case 0740000:
		stream << "cla";
		break;

	case 0740004:
		stream << "tac";
		break;

	case 0740022:
		stream << "lac";
		break;

	case 0740023:
		stream << "tbr";
		break;

	case 0740040:
		stream << "clc";
		break;

	case 0740200:
		stream << "cal";
		break;

	case 0761000:
		stream << "r1c";
		break;

	case 0761031:
		stream << "r1l";
		break;

	case 0761600:
		stream << "r1r";
		break;

	case 0763000:
		stream << "r3c";
		break;

	case 0766000:
		stream << "p6s";
		break;

	default:
		if (inst >= 0760000)
			util::stream_format(stream, "ios %o", inst & 017777);
		else
			util::stream_format(stream, "opr %o", inst & 0177777);
		break;
	}

	return 1 | SUPPORTED;
}

offs_t tx0_8kw_disassembler::disassemble(std::ostream &stream, offs_t pc, const tx0_8kw_disassembler::data_buffer &opcodes, const tx0_8kw_disassembler::data_buffer &params)
{
	u32 inst = opcodes.r32(pc) & 0777777;

	if (inst < 0600000)
	{
		// Addressable instructions
		std::string_view str = s_addressable_insts[(inst & 0760000) >> 13];
		if (str.empty())
			util::stream_format(stream, "%06o", inst);
		else
		{
			util::stream_format(stream, "%s %05o", str, inst & 017777);

			if ((inst & 0760000) == 0440000) // tsx
				return 1 | STEP_OVER | SUPPORTED;
			else if ((inst & 0760000) == 0520000) // trx
				return 1 | STEP_OUT | SUPPORTED;
		}
	}
	else if ((inst & 037000) == 004000)
	{
		// Select class
		if (BIT(inst, 15))
			stream << "claU";
		util::stream_format(stream, "%s %d", s_tape_insts[BIT(inst, 2, 3)], inst & 3);
	}
	else
	{
		if ((inst & 037000) == 02000)
		{
			stream << "tbr";
			if ((inst & 0100757) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if (inst == 0706020)
		{
			stream << "rpf";
			return 1 | SUPPORTED;
		}
		else if ((inst & 037000) == 07000)
		{
			if (BIT(inst, 14))
				stream << "spf";
			else
				stream << "cpf";
			if ((inst & 0100777) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if ((inst & 037000) == 020000)
		{
			stream << "cpy";
			if ((inst & 0100777) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if ((inst & 037000) == 025000)
		{
			stream << "typ";
			if ((inst & 0100777) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if ((inst & 036777) == 026600)
		{
			util::stream_format(stream, "p%dh", BIT(inst, 9) ? 7 : 6);
			return 1 | SUPPORTED;
		}
		else if ((inst & 076777) == 066020)
		{
			util::stream_format(stream, "p%do", BIT(inst, 9) ? 7 : 6);
			return 1 | SUPPORTED;
		}
		else if ((inst & 037000) == 030000)
		{
			// HLT is actually executed last, but normally written first
			stream << "hlt";
			if ((inst & 0100777) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if ((inst & 037000) == 031000)
		{
			stream << "cll";
			if ((inst & 0100777) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if ((inst & 037000) == 032000)
		{
			stream << "clr";
			if ((inst & 0100777) == 0)
				return 1 | SUPPORTED;
			stream << 'U';
		}
		else if ((inst & 037000) != 0)
		{
			if (BIT(inst, 15))
			{
				switch (inst & 037000)
				{
				case 01000:
					stream << "tac";
					break;

				case 03000:
					stream << "claUpen";
					break;

				case 010000: case 011000: case 012000: case 013000: case 014000: case 015000: case 016000: case 017000:
					util::stream_format(stream, "claUex%d", BIT(inst, 9, 3));
					break;

				case 021000:
					if ((inst & 0777) == 0600)
					{
						stream << "r1r";
						return 1 | SUPPORTED;
					}
					stream << "r1c";
					break;

				case 022000:
					stream << "claUdis";
					break;

				case 023000:
					stream << "r3c";
					break;

				case 024000:
					stream << "claUprt";
					break;

				case 026000:
					if ((inst & 040777) == 040020)
					{
						stream << "p6b";
						return 1 | SUPPORTED;
					}
					stream << "p6s";
					break;

				default:
					util::stream_format(stream, "opr %o", inst & 0177777);
					return 1 | SUPPORTED;
				}
				if ((inst & 0777) == 0)
					return 1 | SUPPORTED;
				inst &= 040777;
				stream << 'U';
			}
			else
			{
				switch (inst & 037000)
				{
				case 03000:
					stream << "pen";
					break;

				case 010000: case 011000: case 012000: case 013000: case 014000: case 015000: case 016000: case 017000:
					util::stream_format(stream, "ex%d", BIT(inst, 9, 3));
					break;

				case 021000:
					stream << "r1l";
					break;

				case 022000:
					if ((inst & 040777) == 040020)
					{
						stream << "dso";
						return 1 | SUPPORTED;
					}
					stream << "dis";
					break;

				case 023000:
					stream << "r3l";
					break;

				case 024000:
					if ((inst & 040777) == 040020)
					{
						stream << "pno";
						return 1 | SUPPORTED;
					}
					else if ((inst & 040777) == 040060)
					{
						stream << "pnc";
						return 1 | SUPPORTED;
					}
					else if ((inst & 0777) == 0600)
					{
						stream << "pnt";
						return 1 | SUPPORTED;
					}
					stream << "prt";
					break;

				default:
					util::stream_format(stream, "opr %o", inst & 0177777);
					return 1 | SUPPORTED;
				}
				if ((inst & 0777) == 0)
					return 1 | SUPPORTED;
				stream << 'U';
			}
		}
		else if (inst == 0600000)
		{
			stream << "nop";
			return 1 | SUPPORTED;
		}

		switch (inst & 0140777)
		{
		case 0:
			break;

		case 000001:
			stream << "xro";
			break;

		case 000003:
			stream << "lxr";
			break;

		case 000012:
			stream << "cry";
			break;

		case 000022:
			stream << "lpd";
			break;

		case 000031:
			stream << "alx";
			break;

		case 000032:
			stream << "lad";
			break;

		case 000033:
			stream << "ladUxro";
			break;

		case 000040: case 040040:
			stream << "com";
			break;

		case 000041:
			stream << "comUxro";
			break;

		case 000043:
			stream << "comUlxr";
			break;

		case 000062:
			stream << "lpdUcom";
			break;

		case 000072:
			stream << "lcd";
			break;

		case 000130:
			stream << "xad";
			break;

		case 000170:
			stream << "xcd";
			break;

		case 000200:
			stream << "lro";
			break;

		case 000240:
			stream << "comUlro";
			break;

		case 000272:
			stream << "lcdUlro";
			break;

		case 000300:
			stream << "xlr";
			break;

		case 000303:
			stream << "ixl";
			break;

		case 000400:
			stream << "shr";
			break;

		case 000600:
			stream << "cyr";
			break;

		case 000612:
			stream << "cyrUcry"; // M-5001-19 names this 'ran' (random number generator)
			break;

		case 000640:
			stream << "comUcyr";
			break;

		case 040001:
			stream << "axr";
			break;

		case 040021:
			stream << "axo";
			break;

		case 040030:
			stream << "cyl";
			break;

		case 040031:
			stream << "axrUcyl";
			break;

		case 040050:
			stream << "amz";
			break;

		case 040061:
			stream << "axc";
			break;

		case 040200:
			stream << "alr";
			break;

		case 040201:
			stream << "alrUaxr";
			break;

		case 040203:
			stream << "rax";
			break;

		case 040205:
			stream << "orl";
			break;

		case 040207:
			stream << "anl";
			break;

		case 040220:
			stream << "alo";
			break;

		case 040230:
			stream << "all";
			break;

		case 040231:
			stream << "alrUalx";
			break;

		case 040232:
			stream << "iad";
			break;

		case 040240:
			stream << "alrUcom";
			break;

		case 040247:
			stream << "anlUcom";
			break;

		case 040260:
			stream << "alc";
			break;

		case 040601:
			stream << "arx";
			break;

		case 0100000: case 0140000:
			stream << "cla";
			break;

		case 0100001:
			stream << "cax";
			break;

		case 0100012:
			stream << "lal";
			break;

		case 0100022: case 0140022:
			stream << "lac";
			break;

		case 0100023:
			stream << "lacUlxr";
			break;

		case 0100040: case 0140040:
			stream << "clc";
			break;

		case 0100062:
			stream << "lcc";
			break;

		case 0100072:
			stream << "laz";
			break;

		case 0100110:
			stream << "xal";
			break;

		case 0100120:
			stream << "xac";
			break;

		case 0100160:
			stream << "xcc";
			break;

		case 0100200:
			stream << "cal";
			break;

		case 0100240:
			stream << "calUcom";
			break;

		case 0100322:
			stream << "rxa";
			break;

		case 0100622:
			stream << "lar";
			break;

		case 0140025:
			stream << "ora";
			break;

		case 0140027:
			stream << "ana";
			break;

		case 0140065:
			stream << "oraUcom";  // M-5001-19-1 names this 'orc'
			break;

		case 0140067:
			stream << "anaUcom";  // M-5001-19-1 names this 'anc'
			break;

		case 0140205:
			stream << "oro";
			break;

		case 0140207:
			stream << "ano";
			break;

		case 0140212:
			stream << "lalUalr"; // M-5001-19-1 names this 'ill'
			break;

		case 0140222:
			stream << "ial";
			break;

		case 0140262:
			stream << "ialUcom";
			break;

		default:
			util::stream_format(stream, "opr %o", inst & 0140777);
			break;
		}
	}

	return 1 | SUPPORTED;
}
