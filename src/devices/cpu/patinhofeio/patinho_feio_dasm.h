// license:GPL-2.0+
// copyright-holders:Felipe Sanches

#ifndef MAME_CPU_PATINHOFEIO_PATINHO_FEIO_DASM_H
#define MAME_CPU_PATINHOFEIO_PATINHO_FEIO_DASM_H

#pragma once

class patinho_feio_disassembler : public util::disasm_interface
{
public:
	patinho_feio_disassembler() = default;
	virtual ~patinho_feio_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
