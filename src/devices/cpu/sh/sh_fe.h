// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
/***************************************************************************

    shfe.h

    Front end for SuperH recompiler

***************************************************************************/
#ifndef MAME_CPU_SH_SH_FE_H
#define MAME_CPU_SH_SH_FE_H

#pragma once

#include "sh.h"

#include "cpu/drcfe.h"

#include <cassert>
#include <iosfwd>


class sh_common_execution::opcode_desc : public opcode_desc_base<opcode_desc, 32>
{
public:
	offs_t          physpc;                 // physical PC of this opcode

	uint16_t        opptr;                  // copy of opcode

	uint32_t        cycles;                 // number of cycles needed to execute

	void set_r_used(unsigned n)     { regin.set(REG_R0 + n); }
	void set_pr_used()              { regin.set(REG_PR); }
	void set_sr_used()              { regin.set(REG_SR); }
	void set_macl_used()            { regin.set(REG_MACL); }
	void set_mach_used()            { regin.set(REG_MACH); }
	void set_mac_used()             { set_macl_used(); set_mach_used(); }
	void set_gbr_used()             { regin.set(REG_GBR); }
	void set_vbr_used()             { regin.set(REG_VBR); }

	void set_r_modified(unsigned n) { regout.set(REG_R0 + n); }
	void set_pr_modified()          { regout.set(REG_PR); }
	void set_sr_modified()          { regout.set(REG_SR); }
	void set_macl_modified()        { regout.set(REG_MACL); }
	void set_mach_modified()        { regout.set(REG_MACH); }
	void set_mac_modified()         { set_macl_modified(); set_mach_modified(); }
	void set_gbr_modified()         { regout.set(REG_GBR); }
	void set_vbr_modified()         { regout.set(REG_VBR); }

	void set_reads_memory() { m_reads_memory = true; }
	void set_writes_memory() { m_writes_memory = true; }
	void set_can_expose_external_int() { m_can_expose_external_int = true; }

	bool reads_memory() const { return m_reads_memory; }
	bool writes_memory() const { return m_writes_memory; }
	bool can_expose_external_int() const { return m_can_expose_external_int; }

	// compute the exception PC
	uint32_t epc() const { return in_delay_slot() ? (pc - 1) : pc; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_COUNT <= 32);

		opcode_desc_base::reset(curpc, in_delay_slot);

		physpc = curpc;
		opptr = 0;
		cycles = 0;
		m_reads_memory = false;
		m_writes_memory = false;
		m_can_expose_external_int = false;
	}

	void log_flags(std::ostream &stream) const;
	void log_registers_used(std::ostream &stream) const;
	void log_registers_modified(std::ostream &stream) const;

private:
	enum
	{
		REG_R0 = 0,

		REG_PR = REG_R0 + 16,
		REG_SR,
		REG_MACL,
		REG_MACH,
		REG_GBR,
		REG_VBR,

		REG_COUNT
	};

	static void log_register_list(std::ostream &stream, const regmask &reglist, const regmask *regnostarlist);

	bool m_reads_memory;
	bool m_writes_memory;
	bool m_can_expose_external_int;
};


class sh_common_execution::frontend : public drc_frontend_base<opcode_desc>
{
public:
	frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	virtual ~frontend();

	opcode_desc const *describe_code(offs_t startpc);

protected:
	virtual uint16_t read_word(opcode_desc &desc);

	virtual bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	virtual bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) = 0;

	sh_common_execution *m_sh;

private:
	bool describe(opcode_desc &desc, const opcode_desc *prev);

	bool describe_group_2(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_3(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_6(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_8(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_12(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
};

#endif // MAME_CPU_SH_SH_FE_H
