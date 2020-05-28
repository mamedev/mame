// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*
    DSPP disassembler shim
*/

#ifndef MAME_CPU_DSPP_DSPPDASM_H
#define MAME_CPU_DSPP_DSPPDASM_H

#pragma once

class dspp_disassembler : public util::disasm_interface
{
public:
	dspp_disassembler() = default;
	virtual ~dspp_disassembler() = default;

	virtual uint32_t opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_DSPP_DSPPDASM_H
