// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_ST9_ST9DASM_H
#define MAME_CPU_ST9_ST9DASM_H

#pragma once

class st9_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	st9_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// formatting helpers
	void format_dir(std::ostream &stream, u8 r) const;
	void format_dirw(std::ostream &stream, u8 rr) const;
	void format_imm(std::ostream &stream, u8 n) const;
	void format_immw(std::ostream &stream, u16 n) const;
	void format_disp(std::ostream &stream, s8 n) const;
	void format_dispw(std::ostream &stream, u16 nn) const;
	void format_label(std::ostream &stream, u16 nn) const;

	// disassembly helpers
	offs_t dasm_unknown(std::ostream &stream, u8 opc) const;
	offs_t dasm_06(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_0f(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_1f(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_26(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_2f(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_36(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	virtual offs_t dasm_3f(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_60(std::ostream &stream, u8 byte1, u8 byte2) const;
	offs_t dasm_6f(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_72(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	virtual offs_t dasm_73(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_74(std::ostream &stream, u8 byte1) const;
	virtual offs_t dasm_75(std::ostream &stream, u8 byte1) const;
	offs_t dasm_7e(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_7f(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_86(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_8f(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_96(std::ostream &stream, u8 byte1, u8 byte2) const;
	offs_t dasm_a6(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_b4(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	virtual offs_t dasm_b6(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_be(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_bf(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_c2(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_c4(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_c6(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_c7(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_cf(std::ostream &stream, u8 opc, u8 byte1) const;
	virtual offs_t dasm_d4(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_d5(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_e2(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_e6(std::ostream &stream, u8 byte1, u8 byte2) const;
	offs_t dasm_e7(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_ef(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_f2(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	offs_t dasm_f3(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const;
	virtual offs_t dasm_f6(std::ostream &stream, u8 byte1) const;
};

class st9p_disassembler : public st9_disassembler
{
public:
	// construction/destruction
	using st9_disassembler::st9_disassembler;

protected:
	// util::disasm_interface overrides
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;

	// st9_disassembler overrides
	virtual offs_t dasm_3f(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const override;
	virtual offs_t dasm_73(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const override;
	virtual offs_t dasm_75(std::ostream &stream, u8 byte1) const override;
	virtual offs_t dasm_b6(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const data_buffer &opcodes) const override;
	virtual offs_t dasm_d4(std::ostream &stream, u8 byte1, offs_t pc, const data_buffer &opcodes) const override;
	virtual offs_t dasm_f6(std::ostream &stream, u8 byte1) const override;
};

#endif // MAME_CPU_ST9_ST9DASM_H
