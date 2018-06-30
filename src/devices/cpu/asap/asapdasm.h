// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    asapdasm.c
    Disassembler for the portable ASAP emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_ASAP_ASAPDASM_H
#define MAME_CPU_ASAP_ASAPDASM_H

#pragma once

class asap_disassembler : public util::disasm_interface
{
public:
	asap_disassembler() = default;
	virtual ~asap_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const reg[32];
	static const char *const setcond[2];
	static const char *const condition[16];
	std::string src2(uint32_t op, int scale);

};

#endif
