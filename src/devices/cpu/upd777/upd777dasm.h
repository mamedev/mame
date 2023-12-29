// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_UPD777_UPD777DASM_H
#define MAME_CPU_UPD777_UPD777DASM_H

#pragma once


class upd777_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	upd777_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override { return NONLINEAR_PC; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x7f) | m_l2r[pc & 0x7f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x7f) | m_r2l[pc & 0x7f]; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	u8 m_l2r[0x80];
	u8 m_r2l[0x80];
};

#endif // MAME_CPU_UPD777_UPD777DASM_H
