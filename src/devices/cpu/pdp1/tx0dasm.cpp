// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#include "emu.h"
#include "tx0dasm.h"

u32 tx0_64kw_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t tx0_64kw_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int md;
	int x;

	md = opcodes.r32(pc);

	x = md & 0177777;
	switch (md >> 16)
	{
	case 0:
		util::stream_format(stream, "sto 0%06o", x);
		break;
	case 1:
		util::stream_format(stream, "add 0%06o", x);
		break;
	case 2:
		util::stream_format(stream, "trn 0%06o", x);
		break;
	case 3:
		util::stream_format(stream, "opr 0%06o", x);
		break;
	}
	return 1;
}

u32 tx0_8kw_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t tx0_8kw_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int md;
	int x;

	md = opcodes.r32(pc);

	x = md & 0017777;
	switch (md >> 13)
	{
	case 0:
		util::stream_format(stream, "sto 0%05o", x);
		break;
	case 1:
		util::stream_format(stream, "stx 0%05o", x);
		break;
	case 2:
		util::stream_format(stream, "sxa 0%05o", x);
		break;
	case 3:
		util::stream_format(stream, "ado 0%05o", x);
		break;
	case 4:
		util::stream_format(stream, "slr 0%05o", x);
		break;
	case 5:
		util::stream_format(stream, "slx 0%05o", x);
		break;
	case 6:
		util::stream_format(stream, "stz 0%05o", x);
		break;
	case 8:
		util::stream_format(stream, "add 0%05o", x);
		break;
	case 9:
		util::stream_format(stream, "adx 0%05o", x);
		break;
	case 10:
		util::stream_format(stream, "ldx 0%05o", x);
		break;
	case 11:
		util::stream_format(stream, "aux 0%05o", x);
		break;
	case 12:
		util::stream_format(stream, "llr 0%05o", x);
		break;
	case 13:
		util::stream_format(stream, "llx 0%05o", x);
		break;
	case 14:
		util::stream_format(stream, "lda 0%05o", x);
		break;
	case 15:
		util::stream_format(stream, "lax 0%05o", x);
		break;
	case 16:
		util::stream_format(stream, "trn 0%05o", x);
		break;
	case 17:
		util::stream_format(stream, "tze 0%05o", x);
		break;
	case 18:
		util::stream_format(stream, "tsx 0%05o", x);
		break;
	case 19:
		util::stream_format(stream, "tix 0%05o", x);
		break;
	case 20:
		util::stream_format(stream, "tra 0%05o", x);
		break;
	case 21:
		util::stream_format(stream, "trx 0%05o", x);
		break;
	case 22:
		util::stream_format(stream, "tlv 0%05o", x);
		break;
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
		util::stream_format(stream, "opr 0%06o", md & 0177777);
		break;
	default:
		util::stream_format(stream, "illegal");
		break;
	}
	return 1;
}
