// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_RII_RIIDASM_H
#define MAME_CPU_RII_RIIDASM_H

#pragma once

class riscii_disassembler : public util::disasm_interface
{
public:
	riscii_disassembler() : riscii_disassembler(s_regs) { }

protected:
	// construction/destruction
	riscii_disassembler(const char *const regs[]);

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const s_regs[0x60];

	// internal helpers
	void format_register(std::ostream &stream, u8 reg) const;
	void format_immediate(std::ostream &stream, u8 data) const;

	// register names
	const char *const *m_regs;
};

#endif // MAME_CPU_RII_RIIDASM_H
