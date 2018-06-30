// license:BSD-3-Clause
// copyright-holders:Ville Linde

#ifndef MAME_CPU_SHARC_SHARCDSM_H
#define MAME_CPU_SHARC_SHARCDSM_H

#pragma once

class sharc_disassembler : public util::disasm_interface
{
public:
	sharc_disassembler();
	virtual ~sharc_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct SHARC_DASM_OP
	{
		uint32_t op_mask;
		uint32_t op_bits;
		uint32_t (sharc_disassembler::*handler)(std::ostream &, uint32_t, uint64_t);
	};

	static const char ureg_names[256][16];
	static const char bopnames[8][8];
	static const char condition_codes_if[32][32];
	static const char condition_codes_do[32][32];
	static const char mr_regnames[16][8];
	static const SHARC_DASM_OP sharc_dasm_ops[];

	uint32_t (sharc_disassembler::*sharcdasm_table[256])(std::ostream &, uint32_t, uint64_t);

	void compute(std::ostream &stream, uint32_t opcode);
	void get_if_condition(std::ostream &stream, int cond);
	void pm_dm_ureg(std::ostream &stream, int g, int d, int i, int m, int ureg, int update);
	void pm_dm_imm_dreg(std::ostream &stream, int g, int d, int i, int data, int dreg, int update);
	void pm_dm_dreg(std::ostream &stream, int g, int d, int i, int m, int dreg);
	void shiftop(std::ostream &stream, int shift, int data, int rn, int rx);

	uint32_t dasm_compute_dreg_dmpm(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_compute(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_compute_uregdmpm_regmod(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_compute_dregdmpm_immmod(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_compute_ureg_ureg(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_immshift_dregdmpm(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_immshift_dregdmpm_nodata(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_compute_modify(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_direct_jump(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_indirect_jump_compute(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_indirect_jump_compute_dregdm(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_rts_compute(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_do_until_counter(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_do_until(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_immmove_uregdmpm(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_immmove_uregdmpm_indirect(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_immmove_immdata_dmpm(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_immmove_immdata_ureg(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_sysreg_bitop(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_ireg_modify(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_misc(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_idlenop(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_cjump_rframe(std::ostream &stream, uint32_t pc, uint64_t opcode);
	uint32_t dasm_invalid(std::ostream &stream, uint32_t pc, uint64_t opcode);
};

#endif
