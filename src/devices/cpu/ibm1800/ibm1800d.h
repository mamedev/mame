// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_IBM1800_IBM1800D_H
#define MAME_CPU_IBM1800_IBM1800D_H

#pragma once

class ibm1800_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	ibm1800_disassembler(bool type_1130 = false);

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// formatting helpers
	static void format_cond(std::ostream &stream, u8 cond);
	static void format_disp(std::ostream &stream, int disp);

	const bool m_1130;
};

class ibm1130_disassembler : public ibm1800_disassembler
{
public:
	// construction/destrution
	ibm1130_disassembler();
};

#endif // MAME_CPU_IBM1800_IBM1800D_H
