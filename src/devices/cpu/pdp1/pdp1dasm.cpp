// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#include "emu.h"
#include "cpu/pdp1/pdp1.h"

/* PDP1 registers */
static int ib;
static int y;

static inline void ea (void)
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

CPU_DISASSEMBLE(pdp1)
{
	int md;
	//int etime = 0;

	md = oprom[0] << 24 | oprom[1] << 16 | oprom[2] << 8 | oprom[3];

	y = md & 07777;
	ib = (md >> 12) & 1;               /* */
	switch (md >> 13)
	{
	case AND:
		ea ();
		util::stream_format(stream, "AND (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case IOR:
		ea ();
		util::stream_format(stream, "IOR (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case XOR:
		ea ();
		util::stream_format(stream, "XOR (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case XCT:
		ea ();
		util::stream_format(stream, "XCT (0%06o)", y);
		IN;
		//etime = 5;
		break;
	case CALJDA:
		{
			if (ib == 1)
				util::stream_format(stream, "JDA 0%06o  ", y);
			if (ib == 0)
				util::stream_format(stream, "CAL         ");
			//etime = 10;
			break;
		}
	case LAC:
		ea ();
		util::stream_format(stream, "LAC (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case LIO:
		ea ();
		util::stream_format(stream, "LIO (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case DAC:
		ea ();
		util::stream_format(stream, "DAC 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case DAP:
		ea ();
		util::stream_format(stream, "DAP 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case DIP:
		ea ();
		util::stream_format(stream, "DIP 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case DIO:
		ea ();
		util::stream_format(stream, "DIO 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case DZM:
		ea ();
		util::stream_format(stream, "DZM 0%06o  ", y);
		IN;
		//etime = 10;
		break;
	case ADD:
		ea ();
		util::stream_format(stream, "ADD (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case SUB:
		ea ();
		util::stream_format(stream, "SUB (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case IDX:
		ea ();
		util::stream_format(stream, "IDX (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case ISP:
		ea ();
		util::stream_format(stream, "ISP (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case SAD:
		ea ();
		util::stream_format(stream, "SAD (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case SAS:
		ea ();
		util::stream_format(stream, "SAS (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case MUS_MUL:
		ea ();
		util::stream_format(stream, "MUS (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case DIS_DIV:
		ea ();
		util::stream_format(stream, "DIS (0%06o)", y);
		IN;
		//etime = 10;
		break;
	case JMP:
		ea ();
		util::stream_format(stream, "JMP 0%06o  ", y);
		IN;
		//etime = 5;
		break;
	case JSP:
		ea ();
		util::stream_format(stream, "JSP 0%06o  ", y);
		IN;
		//etime = 5;
		break;
	case SKP:
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
	case SFT:
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
	case LAW:
		util::stream_format(stream, "LAW 0%06o", y);
		IN;
		//etime = 5;
		break;
	case IOT:
		util::stream_format(stream, "IOT 0%06o", md);
		//etime = 10;
		break;
	case OPR:
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
