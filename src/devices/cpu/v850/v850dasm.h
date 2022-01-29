// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_V850_V850DASM_H
#define MAME_CPU_V850_V850DASM_H

#pragma once

#include <string_view>

class v850_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	v850_disassembler();

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	// internal helpers
	virtual void format_system_reg(std::ostream &stream, u8 sreg);
	void format_imm5_signed(std::ostream &stream, u8 imm5);
	void format_imm16_signed(std::ostream &stream, u16 imm16);
	void format_disp16_reg(std::ostream &stream, u16 disp16, u8 reg);
	void format_disp8_ep(std::ostream &stream, u8 disp8);
	virtual offs_t dasm_000000(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_000010(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_000011(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_0001(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_010x(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_1100(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_1101(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_11110(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_extended(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const data_buffer &opcodes);

	// static tables
	static const std::string_view s_regs[32];
	static const std::string_view s_dsp_ops[4];
	static const std::string_view s_rr_ops[8];
	static const std::string_view s_imm5_ops[8];
	static const std::string_view s_bit_ops[4];
	static const std::string_view s_conds[16];
	static const std::string_view s_bconds[16];
};

class v850es_disassembler : public v850_disassembler
{
public:
	// construction/destruction
	v850es_disassembler();

protected:
	void format_list12(std::ostream &stream, u16 list12);
	virtual void format_system_reg(std::ostream &stream, u8 sreg) override;
	virtual offs_t dasm_000010(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_000011(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_0001(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_010x(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_1100(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_11110(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_extended(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const data_buffer &opcodes) override;

	static const std::string_view s_szx_ops[4];
};

class v850e2_disassembler : public v850es_disassembler
{
public:
	// construction/destruction
	v850e2_disassembler();

protected:
	void format_disp23_reg(std::ostream &stream, u32 disp23, u8 reg);
	void format_disp32_reg(std::ostream &stream, u32 disp32, u8 reg);
	virtual void format_system_reg(std::ostream &stream, u8 sreg) override;
	virtual offs_t dasm_000000(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_000010(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_010x(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_1101(std::ostream &stream, u16 opcode, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_11110(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_extended(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const data_buffer &opcodes) override;
};

#endif // MAME_CPU_V850_V850DASM_H
