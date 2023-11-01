// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30 - Multiple effects generator subpart
//
// Audio dsp dedicated to effects generation
//
// Disassembler

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
// XLB----- -rrrrrrr rxlmmmmm m-MM---- -P-----* -----Arr rrrrrrmm mmmm----

// m = low is read port, high is write port, memory register
// r = low is read port, high is high port, rotating register

// X = used for lo-fi variation only
// L = lfo read for memory offset
// * = compute mul
// A = mul input = m or r
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
		append(r, util::string_format("lfo.%02x", pc >> 4));

	if(b(opc, 23, 1))
		switch(b(opc, 24, 2)) {
		case 0:
			if(b(opc, 18, 1))
				append(r, util::string_format("p += %s*m%02x", gconst(pc), b(opc,  4, 6)));
			else
				append(r, util::string_format("p += %s*r%02x", gconst(pc), b(opc, 10, 8)));
			break;
		case 1:
			append(r, util::string_format("p = %s*(r%02x+m%02x)", gconst(pc), b(opc, 10, 8), b(opc,  4, 6)));
			break;
		case 2:
			append(r, util::string_format("p ?= %s*(r%02x+m%02x)", gconst(pc), b(opc, 10, 8), b(opc,  4, 6)));
			break;
		case 3:
			if(b(opc, 18, 1))
				append(r, util::string_format("p = %s*m%02x", gconst(pc), b(opc,  4, 6)));
			else
				append(r, util::string_format("p = %s*r%02x", gconst(pc), b(opc, 10, 8)));
			break;
		}

	std::string source;
	if(b(opc, 30, 1))
		source = "p";
	else if(!b(opc, 45, 1))
		source = util::string_format("lfo.%02x", pc >> 4);

	if(!source.empty()) {
		if(b(opc, 61, 1))
			append(r, util::string_format("mb = %s", source));
		else if(b(opc, 46, 1) == 1)
			append(r, util::string_format("m%02x = %s", b(opc, 39, 6), source));
		else
			append(r, util::string_format("r%02x = %s", b(opc, 47, 8), source));
	}

	u32 memmode = b(opc, 36, 2);
	if(memmode) {
		static const char *modes[4] = { nullptr, "w", "r", "rr" };

		append(r, util::string_format("mem_%s %x +%s", modes[memmode], b(opc, 33, 3), goffset(pc/3)));
		r += util::string_format("-> m%02x", b(opcodes.r64(pc+2), 39, 6));
	}

	stream << r;

	return 1 | SUPPORTED;
}
