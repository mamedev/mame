// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_ROLAND_LSPD_H
#define MAME_SOUND_ROLAND_LSPD_H

#pragma once

class roland_lsp_disassembler : public util::disasm_interface
{
public:
	roland_lsp_disassembler() = default;
	virtual ~roland_lsp_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	static void describe(std::ostream &stream, offs_t pc, u32 word);
};

#endif // MAME_SOUND_ROLAND_LSPD_H
