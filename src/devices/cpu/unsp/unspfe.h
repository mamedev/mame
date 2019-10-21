// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_CPU_UNSP_UNSPFE_H
#define MAME_CPU_UNSP_UNSPFE_H

#pragma once

#include "unsp.h"
#include "cpu/drcfe.h"

class unsp_frontend : public drc_frontend
{
public:
	unsp_frontend(unsp_device *unsp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	void flush();

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	inline uint16_t read_op_word(opcode_desc &desc, int offset);

	unsp_device *m_cpu;
};

#endif /* MAME_CPU_UNSP_UNSPFE_H */
