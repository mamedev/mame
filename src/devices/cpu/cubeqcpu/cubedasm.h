// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    cubedasm.c

    Implementation of the Cube Quest AM2901-based CPUs

***************************************************************************/

#ifndef MAME_CPU_CUBEQCPU_CUBEDASM_H
#define MAME_CPU_CUBEQCPU_CUBEDASM_H

#pragma once

class cubeq_disassembler : public util::disasm_interface
{
public:
	cubeq_disassembler() = default;
	virtual ~cubeq_disassembler() = default;

	virtual u32 opcode_alignment() const override;

protected:
	static const char *const ins[];
	static const char *const src[];
	static const char *const dst[];
};

class cquestsnd_disassembler : public cubeq_disassembler
{
public:
	cquestsnd_disassembler() = default;
	virtual ~cquestsnd_disassembler() = default;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

class cquestrot_disassembler : public cubeq_disassembler
{
public:
	cquestrot_disassembler() = default;
	virtual ~cquestrot_disassembler() = default;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

class cquestlin_disassembler : public cubeq_disassembler
{
public:
	cquestlin_disassembler() = default;
	virtual ~cquestlin_disassembler() = default;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
