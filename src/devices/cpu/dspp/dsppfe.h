// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    dsppfe.h

    Front-end for DSPP recompiler

***************************************************************************/

#ifndef MAME_CPU_DSPP_DSPPFE_H
#define MAME_CPU_DSPP_DSPPFE_H

#pragma once

#include "dspp.h"

#include "cpu/drcfe.h"

#include <cassert>
#include <algorithm>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dspp_device::opcode_desc : public opcode_desc_base<opcode_desc, 16>
{
public:
	uint16_t        opptr[6];               // copy of opcode and operands

	uint32_t        cycles;                 // number of cycles needed to execute

	void set_cc_c_used() { regin.set(REG_CC_C); }
	void set_cc_c_modified() { regout.set(REG_CC_C); }
	void set_cc_z_used() { regin.set(REG_CC_Z); }
	void set_cc_z_modified() { regout.set(REG_CC_Z); }
	void set_cc_n_used() { regin.set(REG_CC_N); }
	void set_cc_n_modified() { regout.set(REG_CC_N); }
	void set_cc_v_used() { regin.set(REG_CC_V); }
	void set_cc_v_modified() { regout.set(REG_CC_V); }
	void set_cc_x_used() { regin.set(REG_CC_X); }
	void set_cc_x_modified() { regout.set(REG_CC_X); }

	void set_reads_memory() { m_reads_memory = true; }
	void set_writes_memory() { m_writes_memory = true; }

	bool reads_memory() const { return m_reads_memory; }
	bool writes_memory() const { return m_writes_memory; }

	uint32_t epc() const { return pc; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_COUNT <= 16);

		opcode_desc_base::reset(curpc, in_delay_slot);

		std::fill(std::begin(opptr), std::end(opptr), 0);
		cycles = 0;
	}

private:
	enum
	{
		REG_CC_C = 0,
		REG_CC_Z,
		REG_CC_N,
		REG_CC_V,
		REG_CC_X,

		REG_COUNT
	};

	bool m_reads_memory;
	bool m_writes_memory;
};


class dspp_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	// construction/destruction
	frontend(dspp_device *dspp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

private:
	bool describe(opcode_desc &desc, opcode_desc const *prev);

	// internal helpers
	void describe_special(uint16_t op, opcode_desc &desc);
	void describe_branch(uint16_t op, opcode_desc &desc);
	void describe_complex_branch(uint16_t op, opcode_desc &desc);
	void describe_arithmetic(uint16_t op, opcode_desc &desc);
	void parse_operands(uint16_t op, opcode_desc &desc, uint32_t numops);

	// internal state
	dspp_device *m_dspp;
};

#endif // MAME_CPU_DSPP_DSPPFE_H
