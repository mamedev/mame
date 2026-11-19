// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_I750_I750D_H
#define MAME_CPU_I750_I750D_H

#pragma once

class i750_disassembler : public util::disasm_interface
{
public:
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_I750_I750D_H
