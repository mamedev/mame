// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_DSP56000_DSP56000D_H
#define MAME_CPU_DSP56000_DSP56000D_H

#pragma once

class dsp56000_disassembler : public util::disasm_interface
{
public:
	dsp56000_disassembler() = default;
	virtual ~dsp56000_disassembler() = default;

	// disasm_interface overrides
	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;

private:
	// helpers
	std::string reg(unsigned const dddddd) const;
	std::string ea(unsigned const MMMRRR, u16 const absolute = 0) const;
};

#endif // MAME_CPU_DSP56000_DSP56000D_H
