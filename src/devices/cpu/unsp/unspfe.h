// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_CPU_UNSP_UNSPFE_H
#define MAME_CPU_UNSP_UNSPFE_H

#pragma once

#include "unsp.h"

#include "cpu/drcfe.h"

#include <algorithm>


class unsp_device::opcode_desc : public opcode_desc_base<opcode_desc, 16>
{
public:
	uint16_t        opptr[2];               // copy of opcode

	uint32_t        cycles;                 // number of cycles needed to execute

	void set_reads_memory() { m_reads_memory = true; }
	void set_writes_memory() { m_writes_memory = true; }

	bool reads_memory() const { return m_reads_memory; }
	bool writes_memory() const { return m_writes_memory; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		opcode_desc_base::reset(curpc, in_delay_slot);

		std::fill(std::begin(opptr), std::end(opptr), 0);
		cycles = 0;
		m_reads_memory = false;
		m_writes_memory = false;
	}

private:
	bool m_reads_memory;
	bool m_writes_memory;
};


class unsp_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	frontend(unsp_device *unsp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

private:
	bool describe(opcode_desc &desc, const opcode_desc *prev);

	uint16_t read_op_word(opcode_desc &desc, int offset);

	unsp_device *m_cpu;
};

#endif // MAME_CPU_UNSP_UNSPFE_H
