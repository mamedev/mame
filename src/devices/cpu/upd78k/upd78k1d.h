// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_UPD78K_UPD78K1D_H
#define MAME_CPU_UPD78K_UPD78K1D_H

#pragma once

#include "upd78kd.h"

class upd78k1_disassembler : public upd78k_8reg_disassembler
{
protected:
	upd78k1_disassembler(const char *const sfr_names[], const char *const sfrp_names[]);

	// mnemonic tables
	static const char *const s_alu_ops[8];
	static const char *const s_alu_ops16[3];
	static const char *const s_bool_ops[4];
	static const char *const s_shift_ops[2][4];
	static const char *const s_bcond[4];

	// disasm_interface overrides
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// disassembly helpers
	virtual offs_t dasm_01xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_02xx(std::ostream &stream, u8 op1, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_05xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_06(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_08xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_16xx(std::ostream &stream, u8 op2, const data_buffer &opcodes);
	virtual offs_t dasm_25(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_29(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_38(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_43(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_50(std::ostream &stream, u8 op);
	virtual offs_t dasm_78(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes);
};

class upd78138_disassembler : public upd78k1_disassembler
{
public:
	upd78138_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78148_disassembler : public upd78k1_disassembler
{
public:
	upd78148_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

#endif // MAME_CPU_UPD78K_UPD78K1D_H
