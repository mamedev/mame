// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppcfe.h

    Front-end for PowerPC recompiler

***************************************************************************/
#ifndef MAME_CPU_POWERPC_PPCFE_H
#define MAME_CPU_POWERPC_PPCFE_H

#pragma once

#include "ppc.h"

#include "cpu/drcfe.h"

#include <bitset>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ppc_device::opcode_desc : public opcode_desc_base<opcode_desc, 128>
{
public:
	offs_t          physpc;                 // physical PC of this opcode

	u32             opptr;                  // copy of opcode

	u32             cycles;                 // number of cycles needed to execute

	void set_can_expose_external_int() { m_extra_flags.set(CAN_EXPOSE_EXTERNAL_INT); }
	void set_privileged() { m_extra_flags.set(PRIVILEGED); }
	void set_compiler_unmapped() { m_extra_flags.set(COMPILER_UNMAPPED); }
	void set_can_change_modes() { m_extra_flags.set(CAN_CHANGE_MODES); }
	void set_reads_memory() { m_extra_flags.set(READS_MEMORY); }
	void set_writes_memory() { m_extra_flags.set(WRITES_MEMORY); }

	bool can_expose_external_int() const { return m_extra_flags[CAN_EXPOSE_EXTERNAL_INT]; }
	bool privileged() const { return m_extra_flags[PRIVILEGED]; }
	bool compiler_unmapped() const { return m_extra_flags[COMPILER_UNMAPPED]; }
	bool can_change_modes() const { return m_extra_flags[CAN_CHANGE_MODES]; }
	bool reads_memory() const { return m_extra_flags[READS_MEMORY]; }
	bool writes_memory() const { return m_extra_flags[WRITES_MEMORY]; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		opcode_desc_base::reset(curpc, in_delay_slot);

		physpc = curpc;
		opptr = 0;
		cycles = 0;
		m_extra_flags.reset();
	}

protected:
	enum
	{
		CAN_EXPOSE_EXTERNAL_INT = 0,
		PRIVILEGED,
		COMPILER_UNMAPPED,
		CAN_CHANGE_MODES,
		READS_MEMORY,
		WRITES_MEMORY,

		EXTRA_FLAG_COUNT
	};

	std::bitset<EXTRA_FLAG_COUNT> m_extra_flags;
};

class ppc_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	// TODO: uncomment constexpr when GCC/GNU libstdc++ catch up
	static constexpr bool     REGFLAG_R(const opcode_desc::regmask &r, unsigned n)      { return r[REG_BIT_R0 + n]; }
	static constexpr bool     REGFLAG_RZ(const opcode_desc::regmask &r, unsigned n)     { return n && REGFLAG_R(r, n); }
	static constexpr bool     REGFLAG_FR(const opcode_desc::regmask &r, unsigned n)     { return r[REG_BIT_FR0 + n]; }
	static /*constexpr*/ uint32_t REGFLAG_CR(const opcode_desc::regmask &r)                 { return uint32_t(((r << 32) >> (128 - 32)).to_ulong()); }
	static /*constexpr*/ uint8_t  REGFLAG_CR(const opcode_desc::regmask &r, unsigned n)     { return uint8_t(((r << ((4 * n) + 32)) >> (128 - 4)).to_ulong()); }
	static constexpr bool     REGFLAG_CR_BIT(const opcode_desc::regmask &r, unsigned n) { return r[64 + 31 - n]; }
	static constexpr bool     REGFLAG_XER_CA(const opcode_desc::regmask &r)             { return r[REG_BIT_XER_CA]; }
	static constexpr bool     REGFLAG_XER_OV(const opcode_desc::regmask &r)             { return r[REG_BIT_XER_OV]; }
	static constexpr bool     REGFLAG_XER_SO(const opcode_desc::regmask &r)             { return r[REG_BIT_XER_SO]; }
	static constexpr bool     REGFLAG_XER_COUNT(const opcode_desc::regmask &r)          { return r[REG_BIT_XER_COUNT]; }
	static constexpr bool     REGFLAG_CTR(const opcode_desc::regmask &r)                { return r[REG_BIT_CTR]; }
	static constexpr bool     REGFLAG_LR(const opcode_desc::regmask &r)                 { return r[REG_BIT_LR]; }
	static constexpr bool     REGFLAG_FPSCR(const opcode_desc::regmask &r, unsigned n)  { return r[REG_BIT_FPSCR0 + n]; }

	// construction/destruction
	frontend(ppc_device &ppc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

protected:
	enum
	{
		REG_BIT_R0          = 0,
		REG_BIT_FR0         = 32,
		REG_BIT_XER_CA      = 96,
		REG_BIT_XER_OV,
		REG_BIT_XER_SO,
		REG_BIT_XER_COUNT,
		REG_BIT_CTR,
		REG_BIT_LR,
		REG_BIT_FPSCR0
	};

	bool describe(opcode_desc &desc, const opcode_desc *prev);

	// inlines
	uint32_t compute_spr(uint32_t spr) const { return ((spr >> 5) | (spr << 5)) & 0x3ff; }
	bool is_403_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_403GA || m_ppc.m_flavor == ppc_device::PPC_MODEL_403GB || m_ppc.m_flavor == ppc_device::PPC_MODEL_403GC || m_ppc.m_flavor == ppc_device::PPC_MODEL_403GCX || m_ppc.m_flavor == ppc_device::PPC_MODEL_405GP); }
	bool is_601_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_601); }
	bool is_602_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_602); }
	bool is_603_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_603 || m_ppc.m_flavor == ppc_device::PPC_MODEL_603E || m_ppc.m_flavor == ppc_device::PPC_MODEL_603EV || m_ppc.m_flavor == ppc_device::PPC_MODEL_603R); }

	// internal helpers
	bool describe_13(uint32_t op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_1f(uint32_t op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_3b(uint32_t op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_3f(uint32_t op, opcode_desc &desc, const opcode_desc *prev);

	// internal state
	ppc_device &m_ppc;
};

#endif // MAME_CPU_POWERPC_PPCFE_H
