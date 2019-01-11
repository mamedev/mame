// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// ********************************************************************************
// * HP "hybrid" processor disassembler
// ********************************************************************************

#ifndef MAME_CPU_HPHYBRID_HPHYBRID_DASM_H
#define MAME_CPU_HPHYBRID_HPHYBRID_DASM_H

#pragma once

class hp_hybrid_disassembler : public util::disasm_interface
{
public:
	hp_hybrid_disassembler() = default;
	virtual ~hp_hybrid_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	typedef void (hp_hybrid_disassembler::*fn_dis_param)(std::ostream &stream , offs_t pc , uint16_t opcode , bool is_3001);

	typedef struct {
		uint16_t m_op_mask;
		uint16_t m_opcode;
		const char *m_mnemonic;
		fn_dis_param m_param_fn;
		uint32_t m_dasm_flags;
	} dis_entry_t;

	static const dis_entry_t dis_table[];

	void addr_2_str(std::ostream &stream, uint16_t addr , bool indirect , bool is_3001);
	void param_none(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_loc(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_addr32(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_skip(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_skip_sc(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_ret(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_n16(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);
	void param_reg_id(std::ostream &stream, offs_t pc , uint16_t opcode , bool is_3001);

	offs_t disassemble_table(uint16_t opcode , offs_t pc , const dis_entry_t *table , bool is_3001 , std::ostream &stream);
};

class hp_5061_3001_disassembler : public hp_hybrid_disassembler
{
public:
	hp_5061_3001_disassembler() = default;
	virtual ~hp_5061_3001_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	static const dis_entry_t dis_table_emc[];
};

#endif
