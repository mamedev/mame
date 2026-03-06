// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_CPU_SHARC_SHARC_DASM_H
#define MAME_CPU_SHARC_SHARC_DASM_H

#pragma once

class sharc_disassembler : public util::disasm_interface
{
public:
	sharc_disassembler();
	virtual ~sharc_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	offs_t disassemble_one(std::ostream &stream, offs_t pc, uint64_t opcode) const;

private:
	using dasm_func = uint32_t (sharc_disassembler::*)(std::ostream &stream, uint32_t pc, uint64_t opcode) const;

	struct SHARC_DASM_OP
	{
		uint32_t op_mask;
		uint32_t op_bits;
		dasm_func handler;
	};

	static const SHARC_DASM_OP sharc_dasm_ops[];

	dasm_func sharcdasm_table[256];

	void compute(std::ostream &stream, uint32_t opcode) const;
	void get_if_condition(std::ostream &stream, int cond) const;
	void pm_dm_ureg(std::ostream &stream, int g, int d, int i, int m, int ureg, int update) const;
	void pm_dm_imm_dreg(std::ostream &stream, int g, int d, int i, int data, int dreg, int update) const;
	void pm_dm_dreg(std::ostream &stream, int g, int d, int i, int m, int dreg) const;
	void shiftop(std::ostream &stream, int shift, int data, int rn, int rx) const;

	uint32_t dasm_compute_dreg_dmpm(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_compute(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_compute_uregdmpm_regmod(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_compute_dregdmpm_immmod(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_compute_ureg_ureg(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_immshift_dregdmpm(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_immshift_dregdmpm_nodata(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_compute_modify(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_direct_jump(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_indirect_jump_compute(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_indirect_jump_compute_dregdm(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_rts_compute(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_do_until_counter(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_do_until(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_immmove_uregdmpm(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_immmove_uregdmpm_indirect(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_immmove_immdata_dmpm(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_immmove_immdata_ureg(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_sysreg_bitop(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_ireg_modify(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_misc(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_idlenop(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_cjump_rframe(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
	uint32_t dasm_invalid(std::ostream &stream, uint32_t pc, uint64_t opcode) const;
};

#endif // MAME_CPU_SHARC_SHARC_DASM_H
