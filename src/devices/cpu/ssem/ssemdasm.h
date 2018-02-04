// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Manchester Small-Scale Experimental Machine (SSEM) disassembler

    Written by Ryan Holtz
*/

#ifndef MAME_CPU_SSEM_SSEMDASM_H
#define MAME_CPU_SSEM_SSEMDASM_H

#pragma once

class ssem_disassembler : public util::disasm_interface
{
public:
	ssem_disassembler() = default;
	virtual ~ssem_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static inline uint32_t reverse(uint32_t v);

};

#endif
