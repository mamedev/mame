// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_PDP8_PDP8DASM_H
#define MAME_CPU_PDP8_PDP8DASM_H

#pragma once

class pdp8_disassembler : public util::disasm_interface
{
public:
	pdp8_disassembler(bool has_r3l = false);

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	void dasm_memory_reference(std::ostream &stream, u16 inst, offs_t pc);
	virtual offs_t dasm_iot(std::ostream &stream, u16 dev, offs_t pc);
	void dasm_opr_group1(std::ostream &stream, u16 inst);
	offs_t dasm_opr_group2(std::ostream &stream, u16 inst);
	virtual offs_t dasm_opr_group3(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes);

private:
	const bool m_has_r3l;
};

class hd6120_disassembler : public pdp8_disassembler
{
public:
	hd6120_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual u32 page2_address_bits() const override;

	virtual offs_t dasm_iot(std::ostream &stream, u16 dev, offs_t pc) override;
	virtual offs_t dasm_opr_group3(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) override;
};

#endif // MAME_CPU_PDP8_PDP8DASM_H
