// license:GPL-2.0
// copyright-holders:Segher Boessenkool, David Haywood
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#ifndef MAME_CPU_UNSP_UNSPDASM_H
#define MAME_CPU_UNSP_UNSPDASM_H

#pragma once

#define UNSP_DASM_OK (len | SUPPORTED)

class unsp_disassembler : public util::disasm_interface
{
public:
	unsp_disassembler()
	{
		m_iso = 10;
	}

	virtual ~unsp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	offs_t disassemble_code(std::ostream &stream, offs_t pc, uint16_t op, uint16_t imm16, const data_buffer& opcodes);

protected:
	offs_t disassemble_fxxx_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes);
	offs_t disassemble_jumps(std::ostream& stream, offs_t pc, const uint16_t op);
	offs_t disassemble_remaining(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes);

	offs_t disassemble_fxxx_000_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	offs_t disassemble_fxxx_001_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	offs_t disassemble_fxxx_010_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	offs_t disassemble_fxxx_011_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	offs_t disassemble_fxxx_100_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	virtual offs_t disassemble_fxxx_101_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	offs_t disassemble_fxxx_110_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	offs_t disassemble_fxxx_111_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes);

	virtual offs_t disassemble_exxx_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm);
	virtual offs_t disassemble_extended_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes);

	static char const *const regs[];
	static char const *const extregs[];
	static char const *const jumps[];
	static char const *const bitops[];
	static char const *const signmodes[];
	static char const *const lsft[];
	static char const *const aluops[];
	static char const *const forms[];

	int m_iso;

protected:
	void print_alu_op_start(std::ostream &stream, uint8_t op0, uint8_t opA);
	void print_alu_op3(std::ostream &stream, uint8_t op0, uint8_t opB);
	void print_alu_op_end(std::ostream &stream, uint8_t op0);
	void print_indirect_op(std::ostream &stream, uint8_t opN, uint8_t opB);
	void print_muls(std::ostream& stream, uint16_t op);
	void print_mul(std::ostream& stream, uint16_t op);

};

class unsp_12_disassembler : public unsp_disassembler
{
public:
	unsp_12_disassembler()
	{
		m_iso = 12;
	}

	virtual ~unsp_12_disassembler() = default;

private:
	virtual offs_t disassemble_fxxx_101_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm) override;
	virtual offs_t disassemble_exxx_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm) override;
};

class unsp_20_disassembler : public unsp_12_disassembler
{
public:
	unsp_20_disassembler()
	{
		m_iso = 20;
	}

	virtual ~unsp_20_disassembler() = default;

private:
	virtual offs_t disassemble_extended_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes) override;

};


#endif
