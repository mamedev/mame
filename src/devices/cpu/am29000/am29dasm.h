// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    am29dasm.c
    Disassembler for the portable Am29000 emulator.
    Written by Phil Bennett

***************************************************************************/

#ifndef MAME_CPU_AM29000_AM29DASM_H
#define MAME_CPU_AM29000_AM29DASM_H

#pragma once

class am29000_disassembler : public util::disasm_interface
{
public:
	am29000_disassembler() = default;
	virtual ~am29000_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	std::string dasm_type1(uint32_t op);
	std::string dasm_type2(uint32_t op);
	std::string dasm_type3(uint32_t op);
	std::string dasm_type4(uint32_t op, uint32_t pc);
	std::string dasm_type5(uint32_t op);
	std::string dasm_type6(uint32_t op);
	const char* get_spr(int spid);

};

#endif
