// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// ********************************************************************************
// * HP "hybrid" processor disassembler
// ********************************************************************************

#ifndef MAME_CPU_HPHYBRID_HPHYBRID_DASM_H
#define MAME_CPU_HPHYBRID_HPHYBRID_DASM_H

#pragma once

enum : unsigned {
	HP_HYBRID_DASM_HAS_15BITS = 1,
	HP_HYBRID_DASM_HAS_EMC = 2,
	HP_HYBRID_DASM_HAS_AEC = 4,
	HP_HYBRID_DASM_ABS_MODE = 8
};

class hp_hybrid_disassembler : public util::disasm_interface
{
public:
	hp_hybrid_disassembler(unsigned flags);
	virtual ~hp_hybrid_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	typedef void (hp_hybrid_disassembler::*fn_dis_param)(std::ostream &stream, offs_t pc, uint16_t opcode);

	typedef struct {
		uint16_t m_op_mask;
		uint16_t m_opcode;
		const char *m_mnemonic;
		fn_dis_param m_param_fn;
		uint32_t m_dasm_flags;
	} dis_entry_t;

	unsigned m_flags;
	static const dis_entry_t dis_table_common[];
	static const dis_entry_t dis_table_ioc16[];
	static const dis_entry_t dis_table_emc[];
	static const dis_entry_t dis_table_aec[];

	void addr_2_str(std::ostream &stream, uint16_t addr, bool indirect);
	void param_none(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_loc(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_addr32(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_skip(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_skip_sc(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_ret(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_n16(std::ostream &stream, offs_t pc, uint16_t opcode);
	void param_reg_id(std::ostream &stream, offs_t pc, uint16_t opcode);

	offs_t disassemble_table(uint16_t opcode, offs_t pc, const dis_entry_t *table, std::ostream &stream);
};

class hp_5061_3001_disassembler : public hp_hybrid_disassembler
{
public:
	hp_5061_3001_disassembler(bool relative_mode = true);
	virtual ~hp_5061_3001_disassembler() = default;
};

class hp_5061_3011_disassembler : public hp_hybrid_disassembler
{
public:
	hp_5061_3011_disassembler(bool relative_mode = true);
	virtual ~hp_5061_3011_disassembler() = default;
};

class hp_09825_67907_disassembler : public hp_hybrid_disassembler
{
public:
	hp_09825_67907_disassembler(bool relative_mode = false);
	virtual ~hp_09825_67907_disassembler() = default;
};

#endif
