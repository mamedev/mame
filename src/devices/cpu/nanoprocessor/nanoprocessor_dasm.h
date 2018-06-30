// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// *******************************
// * HP nanoprocessor disassembler
// *******************************

#ifndef MAME_CPU_NANOPROCESSOR_NANOPROCESSOR_DASM_H
#define MAME_CPU_NANOPROCESSOR_NANOPROCESSOR_DASM_H

#pragma once

class hp_nanoprocessor_disassembler : public util::disasm_interface
{
public:
	hp_nanoprocessor_disassembler() = default;
	virtual ~hp_nanoprocessor_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	typedef void (hp_nanoprocessor_disassembler::*fn_dis_param)(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);

	typedef struct {
		uint8_t m_op_mask;
		uint8_t m_opcode;
		const char *m_mnemonic;
		fn_dis_param m_param_fn;
		uint32_t m_dasm_flags;
	} dis_entry_t;

	void param_bitno(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_ds(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_reg(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_11bit(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_page_no(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_byte(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_ds_byte(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);
	void param_reg_byte(std::ostream& stream, uint8_t opcode, offs_t pc, const data_buffer &params);

	static const dis_entry_t dis_table[];
};

#endif
