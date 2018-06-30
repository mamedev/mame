// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller

#ifndef MAME_CPU_F8_F8DASM_H
#define MAME_CPU_F8_F8DASM_H

#pragma once

class f8_disassembler : public util::disasm_interface
{
public:
	f8_disassembler() = default;
	virtual ~f8_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
private:
	static const char *const rname[16];
};

#endif
