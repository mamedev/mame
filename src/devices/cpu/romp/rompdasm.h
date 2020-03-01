// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ROMP_ROMPDASM_H
#define MAME_CPU_ROMP_ROMPDASM_H

#pragma once

class romp_disassembler : public util::disasm_interface
{
public:
	romp_disassembler() = default;
	virtual ~romp_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;
	virtual u32 opcode_alignment() const override { return 2; }
};

#endif // MAME_CPU_ROMP_ROMPDASM_H
