// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "opcode.h"

#include <cstdio>


namespace DSP_56156 {

Opcode::Opcode(uint16_t w0, uint16_t w1) : m_word0(w0)/*, m_word1(w1)*/
{
	m_instruction = Instruction::decodeInstruction(this, w0, w1);
	m_parallelMove = ParallelMove::decodeParallelMove(this, w0, w1);
}


Opcode::~Opcode()
{
}


std::string Opcode::disassemble() const
{
	// Duck out early if there isn't a valid op
	if (!m_instruction)
		return dcString();

	// Duck out if either has had an explicit error.
	if (m_instruction && !m_instruction->valid())
		return dcString();
	if (m_parallelMove && !m_parallelMove->valid())
		return dcString();

	// Disassemble what you can.
	auto opString = m_instruction ? m_instruction->disassemble() : "";
	auto pmString = m_parallelMove ? m_parallelMove->disassemble() : "";

	return opString + " " + pmString;
}


size_t Opcode::size() const
{
	if (m_instruction && m_instruction->valid())
		return m_instruction->size() + m_instruction->sizeIncrement();

	// Opcode failed to decode, so push it past dc
	return 1;
}


const reg_id& Opcode::instSource() const { return m_instruction->source(); }
const reg_id& Opcode::instDestination() const { return m_instruction->destination(); }
size_t Opcode::instAccumulatorBitsModified() const { return m_instruction->accumulatorBitsModified(); }

std::string Opcode::dcString() const
{
	return util::string_format("dc $%x", m_word0);
}

} // namespace DSP_56156
