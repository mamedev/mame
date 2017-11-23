// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_CPU_E132XS_E132XSFE_H
#define MAME_CPU_E132XS_E132XSFE_H

#pragma once

#include "e132xs.h"
#include "cpu/drcfe.h"

class e132xs_frontend : public drc_frontend
{
public:
	e132xs_frontend(hyperstone_device *e132xs, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	void flush();

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	inline uint16_t read_word(opcode_desc &desc);
	inline uint16_t read_imm1(opcode_desc &desc);
	inline uint16_t read_imm2(opcode_desc &desc);
	inline uint32_t read_ldstxx_imm(opcode_desc &desc);
	inline uint32_t read_limm(opcode_desc &desc, uint16_t op);
	inline int32_t decode_pcrel(opcode_desc &desc, uint16_t op);
	inline int32_t decode_call(opcode_desc &desc);

	hyperstone_device *m_cpu;
};

#endif /* MAME_CPU_E132XS_E132XSFE_H */
