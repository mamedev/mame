// license:GPL-2.0
// copyright-holders:Segher Boessenkool
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#ifndef MAME_CPU_UNSP_UNSPDASM_H
#define MAME_CPU_UNSP_UNSPDASM_H

#pragma once

class unsp_disassembler : public util::disasm_interface
{
public:
	unsp_disassembler() = default;
	virtual ~unsp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	offs_t disassemble(std::ostream &stream, offs_t pc, uint16_t op, uint16_t imm16);

protected:
	virtual offs_t disassemble_f_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, uint8_t opN, uint8_t opA, uint8_t opB, uint32_t len);

	static char const *const regs[];
	static char const *const jumps[];

private:
	void print_alu_op_start(std::ostream &stream, uint8_t op0, uint8_t opA);
	void print_alu_op3(std::ostream &stream, uint8_t op0, uint8_t opB);
	void print_alu_op_end(std::ostream &stream, uint8_t op0);
	void print_indirect_op(std::ostream &stream, uint8_t opN, uint8_t opB);

};

class unsp_newer_disassembler : public unsp_disassembler
{
public:
	unsp_newer_disassembler() = default;
	virtual ~unsp_newer_disassembler() = default;

private:
	virtual offs_t disassemble_f_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, uint8_t opN, uint8_t opA, uint8_t opB, uint32_t len) override;

};


#endif
