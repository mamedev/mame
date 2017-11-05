// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DSP56K_OPCODE_H__
#define __DSP56K_OPCODE_H__

#include "inst.h"
#include "pmove.h"

#include "dsp56k.h"

//
// An Opcode contains an instruction and a parallel move operation.
//
namespace DSP56K
{
class Instruction;
class ParallelMove;

class Opcode
{
public:
	Opcode(uint16_t w0, uint16_t w1);
	virtual ~Opcode();

	std::string disassemble() const;
	void evaluate(dsp56k_core* cpustate) const;
	size_t size() const;
	size_t evalSize() const;

	// Peek through to the instruction
	const reg_id& instSource() const;
	const reg_id& instDestination() const;
	size_t instAccumulatorBitsModified() const;

private:
	std::unique_ptr<Instruction> m_instruction;
	std::unique_ptr<ParallelMove> m_parallelMove;

	uint16_t m_word0;
	//uint16_t m_word1;

	std::string dcString() const;
};

}
#endif
