// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_NEXT_SPECNEXT_COPPER_DASM_H
#define MAME_SINCLAIR_NEXT_SPECNEXT_COPPER_DASM_H

#pragma once

class specnext_copper_disassembler : public util::disasm_interface
{
public:
	specnext_copper_disassembler() = default;
	virtual ~specnext_copper_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		u32 (*cb)(std::ostream &, const data_buffer &, u16, u16);
	};

	static const instruction instructions[];
};

#endif // MAME_SINCLAIR_NEXT_SPECNEXT_COPPER_DASM_H
