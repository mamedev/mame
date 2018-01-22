// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#include "emu.h"
#include "pdp1dasm.h"
#include "pdp1.h"

inline void pdp1_disassembler::ea()
{
/*  while (1)
    {
        if (ib == 0)
            return;
        ib = (READ_PDP_18BIT (y) >> 12) & 1;
        y = READ_PDP_18BIT (y) & 07777;
    }*/
}

#define IN if (ib) util::stream_format(stream, " i")

u32 pdp1_disassembler::opcode_alignment() const
{
	return 4;
}

offs_t pdp1_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int md;
	//int etime = 0;

	md = opcodes.r32(pc);

	int y = md & 07777;
	int ib = (md >> 12) & 1;               /* */
	switch (md >> 13)
	{
	case pdp1_device::AND:
		ea ();
		util::stream_format(stream, "AND (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::IOR:
		ea ();
		util::stream_format(stream, "IOR (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::XOR:
		ea ();
		util::stream_format(stream, "XOR (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::XCT:
		ea ();
		util::stream_format(stream, "XCT (0%06o)", y);
		IN;
		//etime = 5;
		break;
	case pdp1_device::CALJDA:
		{
			if (ib == 1)
				util::stream_format(stream, "JDA 0%06o  ", y);
			if (ib == 0)
				util::stream_format(stream, "CAL         ");
			//etime = 10;
			break;
		}
	case pdp1_device::LAC:
		ea ();
		util::stream_format(stream, "LAC (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::LIO:
		ea ();
		util::stream_format(stream, "LIO (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::DAC:
		ea ();
		util::stream_format(stream, "DAC 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::DAP:
		ea ();
		util::stream_format(stream, "DAP 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::DIP:
		ea ();
		util::stream_format(stream, "DIP 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::DIO:
		ea ();
		util::stream_format(stream, "DIO 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::DZM:
		ea ();
		util::stream_format(stream, "DZM 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::ADD:
		ea ();
		util::stream_format(stream, "ADD (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::SUB:
		ea ();
		util::stream_format(stream, "SUB (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::IDX:
		ea ();
		util::stream_format(stream, "IDX (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::ISP:
		ea ();
		util::stream_format(stream, "ISP (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::SAD:
		ea ();
		util::stream_format(stream, "SAD (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::SAS:
		ea ();
		util::stream_format(stream, "SAS (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::MUS_MUL:
		ea ();
		util::stream_format(stream, "MUS (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::DIS_DIV:
		ea ();
		util::stream_format(stream, "DIS (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case pdp1_device::JMP:
		ea ();
		util::stream_format(stream, "JMP 0%06o  ", y);
		IN;
		//etime = 5;
		break;
	case pdp1_device::JSP:
		ea ();
		util::stream_format(stream, "JSP 0%06o  ", y);
		IN;
		//etime = 5;
		break;
	case pdp1_device::SKP:
		{
			if ((y & 0100) == 0100)
				util::stream_format(stream, "SZA ");
			if ((y & 0200) == 0200)
				util::stream_format(stream, "SPA ");
			if ((y & 0400) == 0400)
				util::stream_format(stream, "SMA ");
			if ((y & 01000) == 01000)
				util::stream_format(stream, "SZO ");
			if ((y & 02000) == 02000)
				util::stream_format(stream, "SPI ");
			if (y & 070)
				util::stream_format(stream, "SZS 0%01o ", (y & 070));
			if (y & 7)
				util::stream_format(stream, "SZF 0%01o ", (y & 7));
			IN;
			//etime = 5;
			break;
		}
	case pdp1_device::SFT:
		{
			int nshift = 0;
			int mask = md & 0777;

			while (mask != 0)
			{
				nshift += mask & 1;
				mask = mask >> 1;
			}
			switch ((md >> 9) & 017)
			{
			case 1:
				util::stream_format(stream, "RAL 0%02o", nshift);
				//etime = 5;
				break;
			case 2:
				util::stream_format(stream, "RIL 0%02o", nshift);
				//etime = 5;
				break;
			case 3:
				util::stream_format(stream, "RCL 0%02o", nshift);
				//etime = 5;
				break;
			case 5:
				util::stream_format(stream, "SAL 0%02o", nshift);
				//etime = 5;
				break;
			case 6:
				util::stream_format(stream, "SIL 0%02o", nshift);
				//etime = 5;
				break;
			case 7:
				util::stream_format(stream, "SCL 0%02o", nshift);
				//etime = 5;
				break;
			case 9:
				util::stream_format(stream, "RAR 0%02o", nshift);
				//etime = 5;
				break;
			case 10:
				util::stream_format(stream, "RIR 0%02o", nshift);
				//etime = 5;
				break;
			case 11:
				util::stream_format(stream, "RCR 0%02o", nshift);
				//etime = 5;
				break;
			case 13:
				util::stream_format(stream, "SAR 0%02o", nshift);
				//etime = 5;
				break;
			case 14:
				util::stream_format(stream, "SIR 0%02o", nshift);
				//etime = 5;
				break;
			case 15:
				util::stream_format(stream, "SCR 0%02o", nshift);
				//etime = 5;
				break;
			default:
				util::stream_format(stream, "SKP ???");
				//etime = 5;
				break;
			}
			break;
		}
	case pdp1_device::LAW:
		util::stream_format(stream, "LAW 0%06o", y);
		IN;
		//etime = 5;
		break;
	case pdp1_device::IOT:
		util::stream_format(stream, "IOT 0%06o", md);
		//etime = 10;
		break;
	case pdp1_device::OPR:
		{
			if ((y & 04000) == 04000)
				util::stream_format(stream, "CLI ");
			if ((y & 02000) == 02000)
				util::stream_format(stream, "LAT ");
			if ((y & 01000) == 01000)
				util::stream_format(stream, "CMA ");
			if ((y & 0400) == 0400)
				util::stream_format(stream, "HLT ");
			if ((y & 0100) == 0100)
				util::stream_format(stream, "LAP ");
			if ((y & 010) && (y & 7))
				util::stream_format(stream, "STF 0%01o ", (y & 7));
			if ((!(y & 010)) && (y & 7))
				util::stream_format(stream, "CLF 0%01o ", (y & 7));
			if (!(y))
				util::stream_format(stream, "NOP ");
			//etime = 5;
			break;
		}
	default:
		util::stream_format(stream, "ILLEGAL");
		//etime = 5;
		break;
	}
	return 4;
}
