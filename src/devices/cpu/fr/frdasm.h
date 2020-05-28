// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_FR_FRDASM_H
#define MAME_CPU_FR_FRDASM_H 1

#pragma once

class fr_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	fr_disassembler();

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	// internal helpers
	void format_u8(std::ostream &stream, u8 value);
	void format_i8(std::ostream &stream, u8 value);
	void format_disp(std::ostream &stream, u8 offs, unsigned shift);
	void format_u10(std::ostream &stream, u16 value);
	void format_i20(std::ostream &stream, u32 value);
	void format_i32(std::ostream &stream, u32 value);
	void format_label(std::ostream &stream, offs_t addr);
	void format_dir(std::ostream &stream, u16 addr);
	void format_ac_rdisp(std::ostream &stream, u8 rj);
	void format_sp_udisp(std::ostream &stream, u8 disp);
	void format_rs(std::ostream &stream, u8 reg);
	offs_t dasm_invalid(std::ostream &stream, u16 opcode);
	offs_t dasm_i4op(std::ostream &stream, u16 opcode, const char *inst);
	offs_t dasm_shift(std::ostream &stream, u16 opcode, const char *inst);
	offs_t dasm_rrop(std::ostream &stream, u16 opcode, const char *inst);
	offs_t dasm_ld_fp_disp(std::ostream &stream, u16 opcode, const char *inst, unsigned shift);
	offs_t dasm_st_fp_disp(std::ostream &stream, u16 opcode, const char *inst, unsigned shift);
	offs_t dasm_ldstm(std::ostream &stream, u16 opcode, const char *inst);
	offs_t dasm_bop(std::ostream &stream, u16 opcode, const char *inst);
	offs_t dasm_cop(std::ostream &stream, u16 op1, u16 op2, const char *inst, bool crj, bool cri);
	offs_t dasm_call(std::ostream &stream, offs_t pc, const char *inst, u16 disp);
	offs_t dasm_branch(std::ostream &stream, offs_t pc, const char *inst, u16 disp);
	offs_t dasm_07(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 opcode);
	offs_t dasm_17(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 opcode);
	offs_t dasm_97(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 opcode);
	offs_t dasm_9f(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 opcode);
};

#endif // MAME_CPU_FR_FRDASM_H
