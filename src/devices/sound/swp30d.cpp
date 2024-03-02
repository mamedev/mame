// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30 - Multiple effects generator subpart
//
// Audio dsp dedicated to effects generation
//
// Disassembler

// Known problems with the mu100 bios0 uploaded programs:
//
// At address 94 & 97, there's a missing r78 = p / r79 = p
// respectively.  Those are places where is both a rnn = p and mnn = p
//
// insertion1 rooms, at addresses cd, cf, d0, d2, d4, d5, dc, e3, e7
// the r slots have data but are not used.  No idea how they should be
// used.
//
// Same for insertion2 rooms, addresses fd+
//
// Insertion 1/2 aural have a bunch of instructions (c7-ce, f7-fe)
// using r02-r09 and r28/r3a which are not understood.
//

// Amplitude lfo is missing something.  For instance variation delay
// m34 is loaded at 120 and used at 13c, but the way it is used is not
// decoded.  In practice, we do not yet have a way to multiply two
// registers together, only with a constant.

// insertion phaser

// y0 = x1 + (y1 - x0) * lfo

// x = r4a
// y = r4b

//   14f:
//                  66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000
//                  32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210

//                  XLB--ttt -rrrrrrr -xlmmmmm m-MM---- -P----** *--T-Arr rrrrrsmm mmmm----
// 0000000003c25000 ........ ........ ........ ........ ......11 11....1. .1.1.... ........  p  = 0 * x1
// 0100000004925800 .......1 ........ ........ ........ .....1.. 1..1..1. .1.11... ........  p += 0 * y1
// 014b000044925400 .......1 .1..1.11 ........ ........ .1...1.. 1..1..1. .1.1.1.. ........  p += 0 * x0 ; y0 = p

// 0000000000805800 ........ ........ ........ ........ ........ 1....... .1.11... ........  p += fp48a * r0b;
// 0000000003822800 ........ ........ ........ ........ ......11 1.....1. ..1.1... ........  p = f1_2_c1 * r45;

// Important detail: the writes to register (rnn and mnn) seem to be
// delayed by 2 cycles.  That makes the filter computation work out.


#include "emu.h"
#include "swp30d.h"

swp30_disassembler::swp30_disassembler(info *inf) : m_info(inf)
{
}

u32 swp30_disassembler::opcode_alignment() const
{
	return 1;
}

std::string swp30_disassembler::gconst(offs_t address) const
{
	if(!m_info)
		return util::string_format("c%03x", address);
	s16 value = m_info->swp30d_const_r(address);
	return util::string_format("%g", value / 16384.0);
}

std::string swp30_disassembler::goffset(offs_t address) const
{
	return m_info ? util::string_format("%x", m_info->swp30d_offset_r(address)) : util::string_format("of%02x", address);
}

u32 swp30_disassembler::b(u64 opc, u32 start, u32 count)
{
  return (opc >> start) & ((1 << count) - 1);
}

void swp30_disassembler::append(std::string &r, const std::string &e)
{
	if(r != "")
		r += " ; ";
	r += e;
}

// 33333333 33333333 22222222 22222222 11111111 11111111 00000000 00000000
// fedcba98 76543210 fedcba98 76543210 fedcba98 76543210 fedcba98 76543210

// 66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000
// 32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210
// XLB----- -rrrrrrr -xlmmmmm m-MM---- -P----** *c---Arr rrrrrsmm mmmm----

// m = low is read port, high is write port, memory register
// r = low is read port, high is write port, rotating register

// X = used for lo-fi variation only
// L = lfo read for memory offset
// * = compute mul + mode
// A = mul input = m or r
// s = substract to p instead of adding
// P = P sent for register write
// B = register write to mbuf
// M = memory mode, none/read/write/read+1
// x = 0 register write to memory, 1 to rotating
// l = 0 = lfo sent for register write

offs_t swp30_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u64 opc = opcodes.r64(pc);

	std::string r;

	if(b(opc, 62, 1))
		append(r, util::string_format("m%02x = lfo.%02x", b(opc, 4, 6), pc >> 4));

	if(b(opc, 23, 1))
		switch(b(opc, 24, 2)) {
		case 0:
			if(b(opc, 18, 1))
				append(r, util::string_format("p %c= %s*m%02x", b(opc, 10, 1) ? '-' : '+', gconst(pc), b(opc,  4, 6)));
			else
				append(r, util::string_format("p %c= %s*r%02x", b(opc, 10, 1) ? '-' : '+', gconst(pc), b(opc, 11, 7)));
			break;
		case 1:
			append(r, util::string_format("p %c= %s*(r%02x+m%02x)", b(opc, 10, 1) ? '-' : '+', gconst(pc), b(opc, 11, 7), b(opc,  4, 6)));
			break;
		case 2:
			append(r, util::string_format("p = %s*(r%02x+m%02x)", gconst(pc), b(opc, 11, 7), b(opc,  4, 6)));
			break;
		case 3:
			if(b(opc, 18, 1))
				append(r, util::string_format("p = %s*m%02x", gconst(pc), b(opc,  4, 6)));
			else
				append(r, util::string_format("p = %s*r%02x", gconst(pc), b(opc, 11, 7)));
			break;
		}

	if(b(opc, 62, 1))
		append(r, util::string_format("idx = p*4000"));

	if(b(opc, 30, 1) == 1 && b(opc, 61, 1) == 1)
		append(r, util::string_format("mw = p"));

	if(b(opc, 30, 1) == 1 && b(opc, 61, 1) == 0 && b(opc, 46, 1) == 1 && b(opc, 62, 1) == 0)
		append(r, util::string_format("m%02x = p", b(opc, 39, 6)));

	if(b(opc, 30, 1) == 1 && b(opc, 61, 1) == 0 && b(opc, 46, 1) == 0)
		append(r, util::string_format("r%02x = p", b(opc, 48, 7)));

	if(b(opc, 30, 1) == 0 && b(opc, 45, 2) == 2)
		append(r, util::string_format("m%02x = lfo.%02x", b(opc, 39, 6), pc >> 4));

	if(b(opc, 30, 1) == 0 && b(opc, 45, 2) == 3)
		append(r, util::string_format("m%02x = m%02x", b(opc, 39, 6), b(opc,  4, 6)));

	if(b(opc, 46, 2) == 2)
		append(r, util::string_format("m%02x = mr", b(opc, 39, 6)));

	u32 memmode = b(opc, 36, 2);
	if(memmode) {
		static const char *modes[4] = { nullptr, "w", "r", "1r" };
		append(r, util::string_format("mem_%s %x +%s", modes[memmode], b(opc, 33, 3), goffset(pc/3)));
	}

	stream << r;

	return 1 | SUPPORTED;
}
