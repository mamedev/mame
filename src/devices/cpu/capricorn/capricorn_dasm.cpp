// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// ********************************************************************************
// * HP Capricorn processor disassembler
// ********************************************************************************

#include "emu.h"
#include "capricorn_dasm.h"

void capricorn_disassembler::direct_addr(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	util::stream_format(stream, "$%04x", opcodes.r16(pc));
}

offs_t capricorn_disassembler::param_arp_drp(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	stream << "R";
	util::stream_format(stream, "%02o", opcodes.r8(pc) & 0x3f);
	return 0;
}

offs_t capricorn_disassembler::param_dr(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	stream << "DR";
	return 0;
}

offs_t capricorn_disassembler::param_dr_ar(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	stream << "DR,AR";
	return 0;
}

offs_t capricorn_disassembler::param_dr_lit(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	stream << "DR,=";
	// Here we assume that multi-byte instructions operate on 2 bytes because we
	// have no way of knowing how many they are (the actual number of bytes is
	// dynamically determined by the value of DRP register at run-time)
	unsigned bytes = BIT(opcodes.r8(pc), 0) ? 2 : 1;

	for (unsigned i = 1; i <= bytes; i++) {
		util::stream_format(stream, "$%02x ", opcodes.r8(pc+i));
	}

	return bytes;
}

offs_t capricorn_disassembler::param_dr_lit_dir(std::ostream &stream, offs_t pc,const data_buffer &opcodes)
{
	stream << "DR,=";
	direct_addr(stream, pc+1, opcodes);
	return 2;
}

offs_t capricorn_disassembler::param_dr_idx_dir(std::ostream &stream, offs_t pc,const data_buffer &opcodes)
{
	stream << "DR,XAR,";
	direct_addr(stream, pc+1, opcodes);
	return 2;
}

offs_t capricorn_disassembler::param_xr_lit(std::ostream &stream, offs_t pc,const data_buffer &opcodes)
{
	stream << "XR,";
	direct_addr(stream, pc+1, opcodes);
	return 2;
}

offs_t capricorn_disassembler::param_lit_dir(std::ostream &stream, offs_t pc,const data_buffer &opcodes)
{
	stream << "=";
	direct_addr(stream, pc+1, opcodes);
	return 2;
}

offs_t capricorn_disassembler::param_dr_id_ar(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	stream << "DR," << (BIT(opcodes.r8(pc), 1) ? '-' : '+') << "AR";
	return 0;
}

offs_t capricorn_disassembler::param_jmp_off(std::ostream &stream, offs_t pc, const data_buffer &opcodes)
{
	uint16_t off = opcodes.r8(pc+1);
	if (BIT(off, 7)) {
		off -= 0x100;
	}
	util::stream_format(stream, "$%04x", (pc + 2 + off) & 0xffff);
	return 1;
}

const capricorn_disassembler::dis_entry_t capricorn_disassembler::dis_table[] = {
	{ 0xff, 0x01, "ARP R*", false, '\0', nullptr, 0 },
	{ 0xc0, 0x00, "ARP"   , false, '\0', &capricorn_disassembler::param_arp_drp, 0 },
	{ 0xff, 0x41, "DRP R*", false, '\0', nullptr, 0 },
	{ 0xc0, 0x40, "DRP"   , false, '\0', &capricorn_disassembler::param_arp_drp, 0 },
	{ 0xfe, 0x80, "EL"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x82, "ER"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x84, "LL"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x86, "LR"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x88, "IC"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x8a, "DC"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x8c, "TC"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x8e, "NC"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x90, "TS"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x92, "CL"    , true , '\0', &capricorn_disassembler::param_dr, 0 },
	{ 0xfe, 0x94, "OR"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0x96, "XR"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xff, 0x98, "BIN"   , false, '\0', nullptr, 0 },
	{ 0xff, 0x99, "BCD"   , false, '\0', nullptr, 0 },
	{ 0xff, 0x9a, "SAD"   , false, '\0', nullptr, 0 },
	{ 0xff, 0x9b, "DCE"   , false, '\0', nullptr, 0 },
	{ 0xff, 0x9c, "ICE"   , false, '\0', nullptr, 0 },
	{ 0xff, 0x9d, "CLE"   , false, '\0', nullptr, 0 },
	{ 0xff, 0x9e, "RTN"   , false, '\0', nullptr, STEP_OUT },
	{ 0xff, 0x9f, "PAD"   , false, '\0', nullptr, 0 },
	{ 0xfe, 0xa0, "LD"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xa2, "ST"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xa4, "LD"    , true , 'D' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xa6, "ST"    , true , 'D' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xa8, "LD"    , true , '\0', &capricorn_disassembler::param_dr_lit, 0 },
	{ 0xfe, 0xaa, "ST"    , true , '\0', &capricorn_disassembler::param_dr_lit, 0 },
	{ 0xfe, 0xac, "LD"    , true , 'I' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xae, "ST"    , true , 'I' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xb0, "LD"    , true , 'D' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xb2, "ST"    , true , 'D' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xb4, "LD"    , true , 'D' , &capricorn_disassembler::param_dr_idx_dir, 0 },
	{ 0xfe, 0xb6, "ST"    , true , 'D' , &capricorn_disassembler::param_dr_idx_dir, 0 },
	{ 0xfe, 0xb8, "LD"    , true , 'I' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xba, "ST"    , true , 'I' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xbc, "LD"    , true , 'I' , &capricorn_disassembler::param_dr_idx_dir, 0 },
	{ 0xfe, 0xbe, "ST"    , true , 'I' , &capricorn_disassembler::param_dr_idx_dir, 0 },
	{ 0xfe, 0xc0, "CM"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xc2, "AD"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xc4, "SB"    , true , '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xff, 0xc6, "JSB"   , false, '\0', &capricorn_disassembler::param_xr_lit, STEP_OVER },
	{ 0xff, 0xc7, "ANM"   , false, '\0', &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xc8, "CM"    , true , '\0', &capricorn_disassembler::param_dr_lit, 0 },
	{ 0xfe, 0xca, "AD"    , true , '\0', &capricorn_disassembler::param_dr_lit, 0 },
	{ 0xfe, 0xcc, "SB"    , true , '\0', &capricorn_disassembler::param_dr_lit, 0 },
	{ 0xff, 0xce, "JSB"   , false, '\0', &capricorn_disassembler::param_lit_dir, STEP_OVER },
	{ 0xff, 0xcf, "ANM"   , false, '\0', &capricorn_disassembler::param_dr_lit, 0 },
	{ 0xfe, 0xd0, "CM"    , true , 'D' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xd2, "AD"    , true , 'D' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xd4, "SB"    , true , 'D' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xff, 0xd7, "ANM"   , false, 'D' , &capricorn_disassembler::param_dr_lit_dir, 0 },
	{ 0xfe, 0xd8, "CM"    , true , 'D' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xda, "AD"    , true , 'D' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfe, 0xdc, "SB"    , true , 'D' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xff, 0xdf, "ANM"   , false, 'D' , &capricorn_disassembler::param_dr_ar, 0 },
	{ 0xfc, 0xe0, "PO"    , true , 'D' , &capricorn_disassembler::param_dr_id_ar, 0 },
	{ 0xfc, 0xe4, "PU"    , true , 'D' , &capricorn_disassembler::param_dr_id_ar, 0 },
	{ 0xfc, 0xe8, "PO"    , true , 'I' , &capricorn_disassembler::param_dr_id_ar, 0 },
	{ 0xfc, 0xec, "PU"    , true , 'I' , &capricorn_disassembler::param_dr_id_ar, 0 },
	{ 0xff, 0xf0, "JMP"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf1, "JNO"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf2, "JOD"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf3, "JEV"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf4, "JNG"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf5, "JPS"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf6, "JNZ"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf7, "JZR"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf8, "JEN"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xf9, "JEZ"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xfa, "JNC"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xfb, "JCY"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xfc, "JLZ"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xfd, "JLN"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xfe, "JRZ"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	{ 0xff, 0xff, "JRN"   , false, '\0', &capricorn_disassembler::param_jmp_off, 0 },
	// *** END ***
	{0, 0, nullptr, false, 0, nullptr, 0 }
};

u32 capricorn_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t capricorn_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const dis_entry_t *p;
	uint8_t opcode = opcodes.r8(pc);

	for (p = dis_table; p->m_op_mask; p++) {
		if ((opcode & p->m_op_mask) == p->m_opcode) {
			offs_t res = 1 | p->m_dasm_flags | SUPPORTED;
			stream << p->m_mnemonic;
			if (p->m_has_mb) {
				stream << (BIT(opcode, 0) ? 'M' : 'B');
			}
			if (p->m_addr_mode != '\0') {
				stream << p->m_addr_mode;
			}
			if (p->m_param_fn != nullptr) {
				stream << " ";
				res += (this->*(p->m_param_fn))(stream, pc, opcodes);
			}
			return res;
		}
	}

	// Unknown opcode
	stream << "???";
	return 1 | SUPPORTED;
}
