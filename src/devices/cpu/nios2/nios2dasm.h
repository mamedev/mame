// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_NIOS2_NIOS2DASM_H
#define MAME_CPU_NIOS2_NIOS2DASM_H

#pragma once

class nios2_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	nios2_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// disassembly helpers
	offs_t dasm_rtype(std::ostream &stream, u32 inst) const;

	// formatting helpers
	void format_simm16(std::ostream &stream, u16 imm16) const;
	void format_ctlreg(std::ostream &stream, u8 n) const;
};

#endif // MAME_CPU_NIOS2_NIOS2DASM_H
