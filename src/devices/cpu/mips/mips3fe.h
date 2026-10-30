// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3fe.h

    Front-end for MIPS3 recompiler

***************************************************************************/
#ifndef MAME_CPU_MIPS_MIPS3FE_H
#define MAME_CPU_MIPS_MIPS3FE_H

#pragma once

#include "mips3.h"

#include "cpu/drcfe.h"

#include <cassert>
#include <bitset>


//**************************************************************************
//  STRUCTURES
//**************************************************************************

class mips3_device::opcode_desc : public opcode_desc_base<opcode_desc, 68>
{
public:
	enum
	{
		REG_FLAG_R0 = 0,
		REG_FLAG_CPR10 = REG_FLAG_R0 + 32,
		REG_FLAG_LO = REG_FLAG_CPR10 + 32,
		REG_FLAG_HI,
		REG_FLAG_FCC,

		REG_FLAG_COUNT
	};

	offs_t          physpc;                 // physical PC of this opcode

	uint32_t        opptr;                  // copy of opcode

	uint32_t        cycles;                 // number of cycles needed to execute

	void set_r_used(unsigned n) { if (n) regin.set(REG_FLAG_R0 + n); }
	void set_r_modified(unsigned n) { if (n) regout.set(REG_FLAG_R0 + n); }
	void set_cpr1_used(unsigned n) { regin.set(REG_FLAG_CPR10 + n); }
	void set_cpr1_modified(unsigned n) { regout.set(REG_FLAG_CPR10 + n); }
	void set_lo_used() { regin.set(REG_FLAG_LO); }
	void set_lo_modified() { regout.set(REG_FLAG_LO); }
	void set_hi_used() { regin.set(REG_FLAG_HI); }
	void set_hi_modified() { regout.set(REG_FLAG_HI); }
	void set_fcc_used() { regin.set(REG_FLAG_FCC); }
	void set_fcc_modified() { regout.set(REG_FLAG_FCC); }

	void set_can_trigger_sw_int() { m_extra_flags.set(CAN_TRIGGER_SW_INT); }
	void set_can_expose_external_int() { m_extra_flags.set(CAN_EXPOSE_EXTERNAL_INT); }
	void set_modifies_translation() { m_extra_flags.set(MODIFIES_TRANSLATION); }
	void set_compiler_unmapped() { m_extra_flags.set(COMPILER_UNMAPPED); }
	void set_can_change_modes() { m_extra_flags.set(CAN_CHANGE_MODES); }
	void set_reads_memory() { m_extra_flags.set(READS_MEMORY); }
	void set_writes_memory() { m_extra_flags.set(WRITES_MEMORY); }

	bool can_trigger_sw_int() { return m_extra_flags[CAN_TRIGGER_SW_INT]; }
	bool can_expose_external_int() { return m_extra_flags[CAN_EXPOSE_EXTERNAL_INT]; }
	bool modifies_translation() const { return m_extra_flags[MODIFIES_TRANSLATION]; }
	bool compiler_unmapped() const { return m_extra_flags[COMPILER_UNMAPPED]; }
	bool can_change_modes() const { return m_extra_flags[CAN_CHANGE_MODES]; }
	bool reads_memory() const { return m_extra_flags[READS_MEMORY]; }
	bool writes_memory() const { return m_extra_flags[WRITES_MEMORY]; }

	// compute the exception PC
	uint32_t epc() const { return in_delay_slot() ? (pc - 3) : pc; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_FLAG_COUNT <= 68);

		opcode_desc_base::reset(curpc, in_delay_slot);

		physpc = curpc;
		opptr = 0;
		cycles = 0;
		m_extra_flags.reset();
	}

private:
	enum
	{
		CAN_TRIGGER_SW_INT = 0,
		CAN_EXPOSE_EXTERNAL_INT,
		MODIFIES_TRANSLATION,
		COMPILER_UNMAPPED,
		CAN_CHANGE_MODES,
		READS_MEMORY,
		WRITES_MEMORY,

		EXTRA_FLAG_COUNT
	};

	std::bitset<EXTRA_FLAG_COUNT> m_extra_flags;
};


class mips3_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	// construction/destruction
	frontend(mips3_device *mips3, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

private:
	bool describe(opcode_desc &desc, opcode_desc const *prev);

	// internal helpers
	bool describe_special(uint32_t op, opcode_desc &desc);
	bool describe_regimm(uint32_t op, opcode_desc &desc);
	bool describe_idt(uint32_t op, opcode_desc &desc);
	bool describe_cop0(uint32_t op, opcode_desc &desc);
	bool describe_cop1(uint32_t op, opcode_desc &desc);
	bool describe_cop1x(uint32_t op, opcode_desc &desc);
	bool describe_cop2(uint32_t op, opcode_desc &desc);

	// internal state
	mips3_device *m_mips3;
};

#endif // MAME_CPU_MIPS_MIPS3FE_H
