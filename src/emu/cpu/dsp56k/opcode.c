// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include <stdio.h>

#include "opcode.h"

namespace DSP56K
{
Opcode::Opcode(UINT16 w0, UINT16 w1) : m_word0(w0)/*, m_word1(w1)*/
{
	m_instruction.reset(Instruction::decodeInstruction(this, w0, w1));
	m_parallelMove.reset(ParallelMove::decodeParallelMove(this, w0, w1));
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
	std::string opString = "";
	std::string pmString = "";
	if (m_instruction) m_instruction->disassemble(opString);
	if (m_parallelMove) m_parallelMove->disassemble(pmString);

	return opString + " " + pmString;
}


void Opcode::evaluate(dsp56k_core* cpustate) const
{
	if (m_instruction) m_instruction->evaluate(cpustate);
	if (m_parallelMove) m_parallelMove->evaluate();
}


size_t Opcode::size() const
{
	if (m_instruction && m_instruction->valid())
		return m_instruction->size() + m_instruction->sizeIncrement();

	// Opcode failed to decode, so push it past dc
	return 1;
}

size_t Opcode::evalSize() const
{
	if (m_instruction && m_instruction->valid())
		return m_instruction->evalSize(); // Probably doesn't matter : + m_instruction->sizeIncrement();

	// Opcode failed to decode, so push it past dc
	return 1;
}


const reg_id& Opcode::instSource() const { return m_instruction->source(); }
const reg_id& Opcode::instDestination() const { return m_instruction->destination(); }
size_t Opcode::instAccumulatorBitsModified() const { return m_instruction->accumulatorBitsModified(); }

std::string Opcode::dcString() const
{
	char tempStr[1024];
	sprintf(tempStr, "dc $%x", m_word0);
	return std::string(tempStr);
}

}
