// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// *******************************
// * HP nanoprocessor disassembler
// *******************************

#include "emu.h"
#include "nanoprocessor_dasm.h"

void hp_nanoprocessor_disassembler::param_bitno(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	stream << (char)('0' + (opcode & 7));
}

void hp_nanoprocessor_disassembler::param_ds(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	util::stream_format(stream, "DS%u", opcode & 0xf);
}

void hp_nanoprocessor_disassembler::param_reg(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	util::stream_format(stream, "R%u", opcode & 0xf);
}

void hp_nanoprocessor_disassembler::param_11bit(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	unsigned tmp = ((unsigned)(opcode & 7) << 8) | params.r8(pc);
	util::stream_format(stream, "$%03x", tmp);
}

void hp_nanoprocessor_disassembler::param_page_no(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	stream << (char)('0' + (opcode & 7));
}

void hp_nanoprocessor_disassembler::param_byte(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	util::stream_format(stream, "$%02x", params.r8(pc));
}

void hp_nanoprocessor_disassembler::param_ds_byte(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	util::stream_format(stream, "DS%u,$%02x", opcode & 0xf, params.r8(pc));
}

void hp_nanoprocessor_disassembler::param_reg_byte(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params)
{
	util::stream_format(stream, "R%u,$%02x", opcode & 0xf, params.r8(pc));
}

const hp_nanoprocessor_disassembler::dis_entry_t hp_nanoprocessor_disassembler::dis_table[] = {
	{0xff, 0x00, "INB", nullptr, 1},
	{0xff, 0x01, "DEB", nullptr, 1},
	{0xff, 0x02, "IND", nullptr, 1},
	{0xff, 0x03, "DED", nullptr, 1},
	{0xff, 0x04, "CLA", nullptr, 1},
	{0xff, 0x05, "CMA", nullptr, 1},
	{0xff, 0x06, "RSA", nullptr, 1},
	{0xff, 0x07, "LSA", nullptr, 1},
	{0xff, 0x08, "SGT", nullptr, 1},
	{0xff, 0x09, "SLT", nullptr, 1},
	{0xff, 0x0a, "SEQ", nullptr, 1},
	{0xff, 0x0b, "SAZ", nullptr, 1},
	{0xff, 0x0c, "SLE", nullptr, 1},
	{0xff, 0x0d, "SGE", nullptr, 1},
	{0xff, 0x0e, "SNE", nullptr, 1},
	{0xff, 0x0f, "SAN", nullptr, 1},
	{0xf8, 0x10, "SBS", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xff, 0x1f, "SES", nullptr, 1},
	{0xf8, 0x18, "SFS", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xf8, 0x20, "SBN", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xff, 0x2f, "ENI", nullptr, 1},
	{0xf8, 0x28, "STC", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xf8, 0x30, "SBZ", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xff, 0x3f, "SEZ", nullptr, 1},
	{0xf8, 0x38, "SFZ", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xf0, 0x40, "INA", &hp_nanoprocessor_disassembler::param_ds, 1},
	{0xff, 0x5f, "NOP", nullptr, 1},
	{0xf0, 0x50, "OTA", &hp_nanoprocessor_disassembler::param_ds, 1},
	{0xf0, 0x60, "LDA", &hp_nanoprocessor_disassembler::param_reg, 1},
	{0xf0, 0x70, "STA", &hp_nanoprocessor_disassembler::param_reg, 1},
	{0xf8, 0x80, "JMP", &hp_nanoprocessor_disassembler::param_11bit, 2},
	{0xf8, 0x88, "JSB", &hp_nanoprocessor_disassembler::param_11bit, 2 | STEP_OVER},
	{0xf8, 0x90, "JAI", &hp_nanoprocessor_disassembler::param_page_no, 1},
	{0xf8, 0x98, "JAS", &hp_nanoprocessor_disassembler::param_page_no, 1 | STEP_OVER},
	{0xf8, 0xa0, "CBN", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xff, 0xaf, "DSI", nullptr, 1},
	{0xf8, 0xa8, "CLC", &hp_nanoprocessor_disassembler::param_bitno, 1},
	{0xff, 0xb0, "RTI", nullptr, 1 | STEP_OUT},
	{0xff, 0xb1, "RTE", nullptr, 1 | STEP_OUT},
	{0xff, 0xb4, "STE", nullptr, 1},
	{0xff, 0xb5, "CLE", nullptr, 1},
	{0xff, 0xb8, "RTS", nullptr, 1 | STEP_OUT},
	{0xff, 0xb9, "RSE", nullptr, 1 | STEP_OUT},
	{0xff, 0xcf, "LDR", &hp_nanoprocessor_disassembler::param_byte, 2},
	{0xf0, 0xc0, "OTR", &hp_nanoprocessor_disassembler::param_ds_byte, 2},
	{0xf0, 0xd0, "STR", &hp_nanoprocessor_disassembler::param_reg_byte, 2},
	{0xf0, 0xe0, "LDI", &hp_nanoprocessor_disassembler::param_reg, 1},
	{0xf0, 0xf0, "STI", &hp_nanoprocessor_disassembler::param_reg, 1},
	// Catchall for undefined opcodes
	{0x00, 0x00, "???", nullptr, 1}
};

u32 hp_nanoprocessor_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t hp_nanoprocessor_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const uint8_t opcode = opcodes.r8(pc);

	for (const dis_entry_t& ent : dis_table) {
		if ((opcode & ent.m_op_mask) == ent.m_opcode) {
			stream << ent.m_mnemonic << ' ';
			if (ent.m_param_fn != nullptr) {
				(this->*ent.m_param_fn)(stream, opcode, pc+1, params);
			}
			return ent.m_dasm_flags | SUPPORTED;
		}
	}
	// Should never ever happen
	return 0;
}
