// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha MEG - Multiple effects generator
//
// Audio dsp dedicated to effects generation
//
// Disassembler

#include "emu.h"
#include "megd.h"

meg_disassembler::meg_disassembler(info *inf) : m_info(inf)
{
}

u32 meg_disassembler::opcode_alignment() const
{
	return 1;
}

std::string meg_disassembler::gfp(offs_t address) const
{
	if(!m_info)
		return util::string_format("fp%03x", address);
	s16 fp = m_info->fp_r(address);
	return util::string_format("%g", fp / 16384.0);
}

std::string meg_disassembler::goffset(offs_t address) const
{
	return m_info ? util::string_format("%x", m_info->offset_r(address)) : util::string_format("of%02x", address);
}

u32 meg_disassembler::b(u64 opc, u32 start, u32 count)
{
  return (opc >> start) & ((1 << count) - 1);
}

void meg_disassembler::append(std::string &r, std::string e)
{
	if(r != "")
		r += " ; ";
	r += e;
}

// 33333333 33333333 22222222 22222222 11111111 11111111 00000000 00000000
// fedcba98 76543210 fedcba98 76543210 fedcba98 76543210 fedcba98 76543210

// 66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000
// 32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210
// XLB----- -rrrrrrr r--mmmmm m-MM---- -P-----* -----Arr rrrrrrmm mmmm----

// m = low is read port, high is write port, memory register
// r = low is read port, high is high port, rotating register

// X = used for lo-fi variation only
// L = lfo read
// * = compute mul
// A = mul input = m or r
// P = P sent for register write
// B = register write to mbuf
// M = memory mode, none/read/write/read+1

offs_t meg_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u64 opc = opcodes.r64(pc);

	std::string r;

	r = util::string_format("[m%02x]", b(opc, 39, 6));

	if(b(opc, 62, 1))
		append(r, "lfo");

	if(b(opc, 23, 1))
		switch(b(opc, 24, 2)) {
		case 0:
			if(b(opc, 18, 1))
				append(r, util::string_format("p += %s*m%02x", gfp(pc), b(opc,  4, 6)));
			else
				append(r, util::string_format("p += %s*r%02x", gfp(pc), b(opc, 10, 8)));
			break;
		case 1:
			append(r, util::string_format("p = %s*(r%02x+m%02x)", gfp(pc), b(opc, 10, 8), b(opc,  4, 6)));
			break;
		case 2:
			append(r, util::string_format("p ?= %s*(r%02x+m%02x)", gfp(pc), b(opc, 10, 8), b(opc,  4, 6)));
			break;
		case 3:
			if(b(opc, 18, 1))
				append(r, util::string_format("p = %s*m%02x", gfp(pc), b(opc,  4, 6)));
			else
				append(r, util::string_format("p = %s*r%02x", gfp(pc), b(opc, 10, 8)));
			break;
		}

	if(b(opc, 30, 1)) {
		if(b(opc, 61, 1))
			append(r, "mb = p");
		else if(b(opc, 46, 1) == 1)
			append(r, util::string_format("m%02x = p", b(opc, 39, 6)));
		else
			append(r, util::string_format("r%02x = p", b(opc, 47, 8)));
	}

	u32 memmode = b(opc, 36, 2);
	if(memmode) {
		static const char *modes[4] = { nullptr, "w", "r", "rw" };

		append(r, util::string_format("mem_%s %x +%s", modes[memmode], b(opc, 33, 3), goffset(pc/3)));
			r += util::string_format("-> m%02x", b(opcodes.r64(pc+2), 39, 6));
	}

	stream << r;

	return 1 | SUPPORTED;
}
