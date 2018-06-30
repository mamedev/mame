// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cosdasm.c

    Simple RCA COSMAC disassembler.
    Written by Curt Coder

***************************************************************************/

#ifndef MAME_CPU_COSMAC_COSDASM_H
#define MAME_CPU_COSMAC_COSDASM_H

#pragma once

class cosmac_disassembler : public util::disasm_interface
{
public:
	enum
	{
		TYPE_1801,
		TYPE_1802
	};

	cosmac_disassembler(int variant);
	virtual ~cosmac_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	int m_variant;

	offs_t implied(const uint8_t opcode);
	offs_t immediate(offs_t &pc, const data_buffer &params);
	offs_t short_branch(offs_t base_pc, offs_t &pc, const data_buffer &params);
	offs_t long_branch(offs_t &pc, const data_buffer &params);
	offs_t short_skip(offs_t pc);
	offs_t long_skip(offs_t pc);
};

#endif
