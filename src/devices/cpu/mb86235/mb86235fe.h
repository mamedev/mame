// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for MB86235 recompiler

******************************************************************************/

#ifndef MAME_CPU_MB86235_MB86235FE_H
#define MAME_CPU_MB86235_MB86235FE_H

#pragma once

#include "mb86235.h"
#include "cpu/drcfe.h"

class mb86235_frontend : public drc_frontend
{
public:
	mb86235_frontend(mb86235_device *core, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:

	mb86235_device *m_core;

	void describe_alu(opcode_desc &desc, uint32_t aluop);
	void describe_mul(opcode_desc &desc, uint32_t mulop);
	void describe_xfer1(opcode_desc &desc);
	void describe_double_xfer1(opcode_desc &desc);
	void describe_xfer2(opcode_desc &desc);
	void describe_double_xfer2(opcode_desc &desc);
	void describe_xfer3(opcode_desc &desc);
	void describe_control(opcode_desc &desc);
	void describe_alu_input(opcode_desc &desc, int reg);
	void describe_mul_input(opcode_desc &desc, int reg);
	void describe_alumul_output(opcode_desc &desc, int reg);
	void describe_reg_read(opcode_desc &desc, int reg);
	void describe_reg_write(opcode_desc &desc, int reg);
	void describe_ea(opcode_desc &desc, int md, int arx, int ary, int disp);
};

#endif // MAME_CPU_MB86235_MB86235FE_H
