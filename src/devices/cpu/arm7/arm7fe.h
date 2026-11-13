// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_ARM7_ARM7FE_H
#define MAME_CPU_ARM7_ARM7FE_H

#pragma once

#include "arm7.h"

#include "cpu/drcfe.h"

#include <bitset>

// the cycles accumulated in a sequence and not yet taken from icount (the recompiler's map variable)
#define MAPVAR_CYCLES                   uml::M1

class arm7_cpu_device::opcode_desc : public opcode_desc_base<opcode_desc, 24>
{
public:
	// register usage: R0-R15 of the block's mode, the condition flags, and the rest of the CPSR
	enum
	{
		REG_FLAG_R0 = 0,
		REG_FLAG_NZCV = 16,
		REG_FLAG_CPSR,

		REG_FLAG_COUNT
	};

	// the condition flags, as the CPSR orders them
	enum : uint8_t
	{
		NZCV_N = 8,
		NZCV_Z = 4,
		NZCV_C = 2,
		NZCV_V = 1,
		NZCV_ALL = 0xf
	};

	offs_t physpc;  // physical address of the opcode
	uint32_t opptr;  // the opcode: a word in ARM state, a halfword in Thumb state
	uint32_t opword;  // the word the opcode was fetched from
	uint32_t cycles;  // the interpreter's cost, less the operand-dependent part of the multiplies
	uint8_t flags_used;  // the condition flags read (NZCV_*)
	uint8_t flags_written;  // the condition flags written; a shifter carry that may stay is both read and written
	uint8_t flags_live;  // of those written, the ones a later instruction reads

	void set_r_used(uint32_t n) { regin.set(REG_FLAG_R0 + n); }
	void set_r_modified(uint32_t n) { regout.set(REG_FLAG_R0 + n); }
	void set_flags_used(uint8_t flags = NZCV_ALL) { regin.set(REG_FLAG_NZCV); flags_used |= flags; }
	void set_flags_modified(uint8_t flags = NZCV_ALL) { regout.set(REG_FLAG_NZCV); flags_written |= flags; }
	void set_cpsr_used() { regin.set(REG_FLAG_CPSR); }
	void set_cpsr_modified() { regout.set(REG_FLAG_CPSR); }

	void set_can_change_modes() { m_extra_flags.set(CAN_CHANGE_MODES); }
	void set_reads_memory() { m_extra_flags.set(READS_MEMORY); }
	void set_writes_memory() { m_extra_flags.set(WRITES_MEMORY); }

	bool can_change_modes() const { return m_extra_flags[CAN_CHANGE_MODES]; }
	bool reads_memory() const { return m_extra_flags[READS_MEMORY]; }
	bool writes_memory() const { return m_extra_flags[WRITES_MEMORY]; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_FLAG_COUNT <= 24);

		opcode_desc_base::reset(curpc, in_delay_slot);

		physpc = curpc;
		opptr = 0;
		opword = 0;
		cycles = 0;
		flags_used = 0;
		flags_written = 0;
		flags_live = NZCV_ALL;
		m_extra_flags.reset();
	}

private:
	enum
	{
		CAN_CHANGE_MODES = 0,   // mode, T bit or 26/32-bit state may change: the next block's hash mode is dynamic
		READS_MEMORY,
		WRITES_MEMORY,

		EXTRA_FLAG_COUNT
	};

	std::bitset<EXTRA_FLAG_COUNT> m_extra_flags;
};


class arm7_cpu_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	// construction/destruction
	frontend(arm7_cpu_device &arm, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	const opcode_desc *describe_code(offs_t startpc);

private:
	bool describe(opcode_desc &desc, const opcode_desc *prev);
	void compute_flag_liveness(const opcode_desc *desclist);

	// internal helpers
	void describe_arm(opcode_desc &desc, uint32_t op);
	void describe_arm_unconditional(opcode_desc &desc, uint32_t op);
	void describe_arm_misc(opcode_desc &desc, uint32_t op);
	void describe_arm_alu(opcode_desc &desc, uint32_t op);
	void describe_arm_mem_single(opcode_desc &desc, uint32_t op);
	void describe_arm_mem_half(opcode_desc &desc, uint32_t op);
	void describe_arm_mem_block(opcode_desc &desc, uint32_t op);
	static void describe_coprocessor(opcode_desc &desc, uint32_t op);
	void describe_thumb(opcode_desc &desc, uint16_t op, const opcode_desc *prev);

	static void describe_branch(opcode_desc &desc, bool conditional, offs_t target);
	static void describe_pc_write(opcode_desc &desc, bool conditional, bool changes_modes);
	static void describe_undefined(opcode_desc &desc);
	uint32_t pc_mask() const { return (m_arm.m_core->mode & 1) ? 0x03fffffc : ~uint32_t(0); }   // branch targets wrap in the 26-bit modes

	// internal state
	arm7_cpu_device &m_arm;
};

#endif // MAME_CPU_ARM7_ARM7FE_H
