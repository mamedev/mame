// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_CLIPPER_CLIPPERD_H
#define MAME_CPU_CLIPPER_CLIPPERD_H

#pragma once

class clipper_disassembler : public util::disasm_interface
{
public:
	clipper_disassembler() = default;
	virtual ~clipper_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const cc[];
	std::string address (offs_t pc, const data_buffer &opcodes);
};

#endif // MAME_CPU_CLIPPER_CLIPPERD_H
