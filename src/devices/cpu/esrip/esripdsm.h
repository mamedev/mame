// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    esripdsm.c

    Implementation of the Entertainment Sciences
    AM29116-based Real Time Image Processor

***************************************************************************/

#ifndef MAME_CPU_ESRIP_ESRIPDSM_H
#define MAME_CPU_ESRIP_ESRIPDSM_H

#pragma once

class esrip_disassembler : public util::disasm_interface
{
public:
	esrip_disassembler() = default;
	virtual ~esrip_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
