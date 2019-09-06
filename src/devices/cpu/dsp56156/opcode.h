// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef DSP56156_OPCODE_H
#define DSP56156_OPCODE_H

#include "inst.h"
#include "pmove.h"

#include "dsp56156.h"

//
// An Opcode contains an instruction and a parallel move operation.
//
namespace DSP_56156
{
class Instruction;
class ParallelMove;

class Opcode
{
public:
	Opcode(uint16_t w0, uint16_t w1);
	virtual ~Opcode();

	std::string disassemble() const;
	void evaluate(dsp56156_core* cpustate) const;
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
