// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************
    KL1839VM1 disassembler
***************************************************************************/

#include "emu.h"
#include "kl1839vm1dasm.h"

static const std::string r_name(u8 id)
{
	std::ostringstream ss;
	switch (id)
	{
	case 0x14: ss << "AK0"; break;
	case 0x15: ss << "AK1"; break;
	case 0x16: ss << "AK2"; break;
	case 0x12: ss << "AK3"; break;
	case 0x11: ss << "AK4"; break;
	case 0x13: ss << "AK5"; break;
	case 0x17: ss << "AK6"; break;
	case 0x18: ss << "AK7"; break;
	case 0x10: ss << "AK8"; break;
	case 0x1f: ss << "BO"; break;
	default: util::stream_format(ss, "R(%02X)", id); break;
	}
	return ss.str();
}

static const std::string k_name(u8 id)
{
	std::ostringstream ss;
	switch (id)
	{
	case 0x05: ss << "PCM"; break;
	case 0x06: ss << "RC"; break;
	default: util::stream_format(ss, "K(%X)", id); break;
	}
	return ss.str();
}

static const void m_base(std::ostream &stream, u32 op, char m, u8 kop, std::string x, std::string y, std::string z)
{
	const u8 fd = BIT(op, 28, 2);
	const u8 ps = BIT(op, 13, 2);
	const bool pd = BIT(op, 4);

	util::stream_format(stream, "M%c%-2s  :", m, "");
	std::ostringstream ss;
	switch(fd)
	{
		case 0b00: ss << "DS/"; break;
		case 0b01: ss << "SL/"; break;
		case 0b11: ss << "BT/"; break;
		default: break; // 0b10
	}
	switch(kop)
	{
		case 0b0000: ss << y << "=>" << z; break;
		case 0b0010: ss << x << "+" << y << "=>" << z; break;
		case 0b0011: ss << x << "=>" << z; break;
		case 0b0100: ss << x << "-" << y << "=>" << z; break;
		case 0b0110: ss << y << "-" << x << "=>" << z; break;
		case 0b0111: ss << x << "=>!" << x; break;
		case 0b1000: ss << x << "(+)" << y << "=>" << z; break;
		case 0b1001: ss << "AL" << x; break;
		case 0b1010: ss << x << "'" << y << "=>" << z; break;
		case 0b1011: ss << "CL" << x; break;
		case 0b1100: ss << x << "&!" << y << "=>" << z; break;
		case 0b1101: ss << "AP" << x; break;
		case 0b1110: ss << x << "&" << y << "=>" << z; break;
		case 0b1111: ss << "CP" << x; break;
		default: break; // 0b0001, 0b0101 - reserved
	}
	util::stream_format(stream, "%-24s;", ss.str());

	stream << (pd ? "PD " : "");

	switch(ps)
	{
		case 0b00: stream << "ZS "; break;
		case 0b01: stream << "MS "; break;
		case 0b10: stream << "NV "; break;
		default: break; // 0b11 - no state store
	}
}

static const void ma(std::ostream &stream, u32 op)
{
	const u8 kop1 = BIT(op, 24, 4);
	const u8 am = BIT(op, 20, 4);
	const u8 x = BIT(op, 15, 5);
	const bool va = BIT(op, 12);
	const u8 no = BIT(op, 10, 2);
	const u8 fo = BIT(op, 8, 2);
	const u8 kob = BIT(op, 5, 3);
	const bool po = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	m_base(stream, op, 'A', kop1, r_name(x), k_name(am), r_name(x));

	if (kob)
	{
		stream << (va ? "AZ " : "A4 ");

		switch(no)
		{
			case 0b00: break;
			case 0b01: stream << "PP "; break;
			case 0b10: stream << "SR "; break;
			case 0b11: stream << "SP "; break;
			default: break;
		}

		switch(fo)
		{
			case 0b00: stream << "DSO "; break;
			case 0b01: stream << "SLO "; break;
			case 0b10: stream << "4SO "; break;
			case 0b11: stream << "BO "; break;
			default: break;
		}

		switch(kob)
		{
			case 0b000: break;
			case 0b001: stream << "4D "; break;
			case 0b010: stream << "ZD "; break;
			case 0b011: stream << "4Z "; break;
			case 0b100: stream << "?R(17)==ACC "; break;
			case 0b101: stream << "4K "; break;
			case 0b110: break; // reserved
			case 0b111: stream << "BRD "; break;
			default: break;
		}
	}

	if (px | po | py)
		stream << "?:" << (px ? "PX " : "") << (po ? "PO " : "") << (py ? "PY " : "");
}

static const void mb(std::ostream &stream, u32 op)
{
	const u8 kop2 = BIT(op, 25, 3);
	const u8 y = BIT(op, 20, 5);
	const u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);
	const bool va = BIT(op, 12);
	const u8 no = BIT(op, 10, 2);
	const u8 fo = BIT(op, 8, 2);
	const u8 kob = BIT(op, 5, 3);
	const bool pz = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	m_base(stream, op, 'B', kop2 << 1, r_name(x), r_name(y), r_name(x));

	if (kob)
	{
		stream << (va ? "AZ " : "A4 ");

		switch(ps)
		{
			case 0b00: stream << "ZS "; break;
			case 0b01: stream << "MS "; break;
			case 0b10: stream << "NV "; break;
			default: break; // 0b11 - no state store
		}

		switch(no)
		{
			case 0b00: break;
			case 0b01: stream << "PP "; break;
			case 0b10: stream << "SR "; break;
			case 0b11: stream << "SP "; break;
			default: break;
		}

		switch(fo)
		{
			case 0b00: stream << "DSO "; break;
			case 0b01: stream << "SLO "; break;
			case 0b10: stream << "4SO "; break;
			case 0b11: stream << "BO "; break;
			default: break;
		}

		switch(kob)
		{
			case 0b000: break;
			case 0b001: stream << "4D "; break;
			case 0b010: stream << "ZD "; break;
			case 0b011: stream << "4Z "; break;
			case 0b100: stream << "?R(17)==ACC "; break;
			case 0b101: stream << "4K "; break;
			case 0b110: break; // reserved
			case 0b111: stream << "BRD "; break;
			default: break;
		}
	}

	if (px | pz | py)
		stream << "?:" << (px ? "PX " : "") << (pz ? "PZ " : "") << (py ? "PY " : "");
}

static const void mc(std::ostream &stream, u32 op)
{
	const u8 kop2 = BIT(op, 25, 3);
	const u8 y = BIT(op, 20, 5);
	const u8 x = BIT(op, 15, 5);
	const u8 z = BIT(op, 7, 5);
	const bool pz = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	m_base(stream, op, 'C', kop2 << 1, r_name(x), kop2 ? r_name(y) : r_name(x), r_name(z));

	if (px | pz | py)
		stream << "?:" << (px ? "PX " : "") << (pz ? "PZ " : "") << (py ? "PY " : "");
}

static const u32 mk(std::ostream &stream, u32 op)
{
	const bool ret = BIT(op, 17);
	if ((op & 0xfe000000) == 0xe0000000)
	{
		if (BIT(~op, 16)) // madr == 0
		{
			const u16 addr = BIT(op, 2, 14);
			util::stream_format(stream, "BP%s   :                        ;A=%04X", ret ? " " : "B" ,addr);
		}
		else
		{
			stream << "??MK-2";
		}
	}
	else if ((op & 0xfe000000) == 0xe2000000)
	{
		stream << "??MK-3";
	}

	return ret ? 0 : kl1839vm1_disassembler::STEP_OVER;
}

static const u32 yp(std::ostream &stream, u32 op)
{
	const bool uv = BIT(op, 28);
	const bool n = BIT(op, 27);
	const bool z = BIT(op, 26);
	const bool v = BIT(op, 25);
	const bool c = BIT(op, 24);
	const bool fp = BIT(op, 23);
	const bool fpd = BIT(op, 22);
	const bool prb = BIT(op, 21);
	const bool rts = BIT(op, 20);
	const bool rd = BIT(op, 19);
	const bool ret = BIT(op, 17);
	const bool js = BIT(op, 16);
	const u16 addr = BIT(op, 2, 14);
	if (fp)
	{
		util::stream_format(stream, "YPF%s  : ", uv);
	}
	else if (fpd)
		util::stream_format(stream, "YPFPD :", uv);
	else
	{
		util::stream_format(stream, "YP%s%s :", uv , (js ?  "JS" : "  "));
		stream << (n ? "N" : "");
		stream << (z ? "Z" : "");
		stream << (v ? "V" : "");
		stream << (c ? "C" : "");
	}

	util::stream_format(stream, "                       ;A=%04X", addr);

	if (prb | rts | rd)
		stream << " ?:" << (prb ? "PRB " : "") << (rts ? "RTS " : "") << (rd ? "RD " : "");

	return ret ? 0 : kl1839vm1_disassembler::STEP_OVER;
}

static const void zsch(std::ostream &stream, u32 op)
{
	stream << "ZS4   :";
	if (BIT(~op, 16)) // mcc == 0
	{
		const u8 cnst = BIT(op, 2, 8);
		util::stream_format(stream, "S4=K(%02X)                ;", cnst);
	}
	else
	{
		//const u8 cnst = BIT(op, 8, 2);
		const u8 obb = BIT(op, 2, 6);
		util::stream_format(stream, "S4=K(%04X)              ;", obb);
	}
}

static const void psch(std::ostream &stream, u32 op)
{
	stream << "PS4   :";
	if (BIT(~op, 16)) // madr == 0
	{
		const u8 addr = BIT(op, 2, 14);
		util::stream_format(stream, "                        ;A=%04X", addr);
	}
	else
	{
		stream << "...";
	}
}

static const void rts(std::ostream &stream, u32 op)
{
	stream << "RTS   :                        ;";
}

static const void acc(std::ostream &stream, u32 op)
{
	stream << "ACC   :                        ;";
}

static const void chka(std::ostream &stream, u32 op)
{
	stream << "4KA   :";
	if (BIT(~op, 16)) // madr == 0
	{
		const u16 addr = BIT(op, 2, 14);
		util::stream_format(stream, "                        ;A=%04X", addr);
	}
	else
	{
		stream << "??";
	}
}

static const void chlk(std::ostream &stream, u32 op)
{
	const bool sb = BIT(op, 1);
	const u32 cnst = BIT(op, 2, 24);
	util::stream_format(stream, "4LK   :SB%X/K(%06X)           ;", sb, cnst);
}

static const void srf(std::ostream &stream, u32 op)
{
	if (!BIT(op, 1, 25))
	{
		stream << "NOP   :                        ;";
	}
	else if (BIT(op, 4))
	{
		stream << "OCT   :                        ;";
	}
	else
	{
		stream << "SRF   :                        ;";
		stream << (BIT(op, 25) ? "WIMM " : "");
		stream << (BIT(op, 21) ? "SFP1 " : "");
		stream << (BIT(op, 20) ? "RFP1 " : "");
		stream << (BIT(op, 5) ? "DEC " : "");
		stream << (BIT(op, 2) ? "JDZRA " : "");
		stream << (BIT(op, 1) ? "INC " : "");
	}
}

static const void flag(std::ostream &stream, u32 op)
{
	stream << "FLAG  :                        ;";
	stream << (BIT(op, 23) ? "INTMS " : "");
	stream << (BIT(op, 22) ? "RCPU " : "");
	stream << (BIT(op, 21) ? "SIVV " : "");
	stream << (BIT(op, 20) ? "CCFPU " : "");
	stream << (BIT(op, 19) ? "PROB " : "");
	stream << (BIT(op, 17) ? "SIU " : "");
	stream << (BIT(op, 16) ? "TOU " : "");
	stream << (BIT(op, 15) ? "COPWT " : "");
}

static const void rest(std::ostream &stream, u32 op)
{
	stream << "REST  :                        ;";
}

offs_t kl1839vm1_disassembler::disassemble(std::ostream &stream, offs_t pc, const kl1839vm1_disassembler::data_buffer &opcodes, const kl1839vm1_disassembler::data_buffer &params)
{
	u32 op = opcodes.r32(pc);
	u32 step = 0;
	if ((op & 0xc0000000) == 0x00000000) // MA
	{
		if ((op & 0xff000000) == 0x01000000)
			flag(stream, op);
		else if ((op & 0xff000000) == 0x05000000)
			rest(stream, op);
		else
			ma(stream, op);
	}
	else if ((op & 0xc0000000) == 0x40000000) // MB
	{
		mb(stream, op);
	}
	else if ((op & 0xc0000000) == 0x80000000) // MC
	{
		mc(stream, op);
	}
	else
	{
		if ((op & 0xfc000000) == 0xe0000000)
		{
			step |= mk(stream, op);
		}
		else if ((op & 0xe0000000) == 0xc0000000)
		{
			step |= yp(stream, op) | STEP_COND;
		}
		else if ((op & 0xfc000000) == 0xec000000)
			zsch(stream, op);
		else if ((op & 0xfc000000) == 0xe4000000)
		{
			psch(stream, op);
			step |= STEP_COND;
		}
		else if ((op & 0xfc000000) == 0xf0000000)
		{
			rts(stream, op);
			step |= STEP_OUT;
		}
		else if ((op & 0xfc000000) == 0xe8000000)
			acc(stream, op);
		else if ((op & 0xfc000000) == 0xf4000000)
			chka(stream, op);
		else if ((op & 0xfc000000) == 0xf8000000)
			chlk(stream, op);
		else if ((op & 0xfc000000) == 0xfc000000)
			srf(stream, op);
		else
			stream << "<invalid>";
	}

	return 1 | SUPPORTED | step;
}
