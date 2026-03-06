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
#include <cassert>
#include <iosfwd>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ppc_device::opcode_desc : public opcode_desc_base<opcode_desc, 128>
{
public:
	offs_t          physpc;                 // physical PC of this opcode

	u32             opptr;                  // copy of opcode

	u32             cycles;                 // number of cycles needed to execute

	void set_gpr_used(unsigned n)           { regin.set(REG_BIT_R0 + n); }
	void set_gpr_used_or_zero(unsigned n)   { if (n) set_gpr_used(n); }
	void set_gpr_modified(unsigned n)       { regout.set(REG_BIT_R0 + n); }

	void set_fpr_used(unsigned n)           { regin.set(REG_BIT_FR0 + n); }
	void set_fpr_modified(unsigned n)       { regout.set(REG_BIT_FR0 + n); }

	void set_cr_used(unsigned n)            { set_cr_bit_used((n * 4) + 0); set_cr_bit_used((n * 4) + 1); set_cr_bit_used((n * 4) + 2); set_cr_bit_used((n * 4) + 3); }
	void set_cr_bit_used(unsigned n)        { regin.set(64 + 31 - n); }
	void set_cr_modified(unsigned n)        { set_cr_bit_modified((n * 4) + 0); set_cr_bit_modified((n * 4) + 1); set_cr_bit_modified((n * 4) + 2); set_cr_bit_modified((n * 4) + 3); }
	void set_cr_bit_modified(unsigned n)    { regout.set(64 + 31 - n); }

	void set_xer_ca_used()                  { regin.set(REG_BIT_XER_CA); }
	void set_xer_ov_used()                  { regin.set(REG_BIT_XER_OV); }
	void set_xer_so_used()                  { regin.set(REG_BIT_XER_SO); }
	void set_xer_count_used()               { regin.set(REG_BIT_XER_COUNT); }
	void set_xer_ca_modified()              { regout.set(REG_BIT_XER_CA); }
	void set_xer_ov_modified()              { regout.set(REG_BIT_XER_OV); }
	void set_xer_so_modified()              { regout.set(REG_BIT_XER_SO); }
	void set_xer_count_modified()           { regout.set(REG_BIT_XER_COUNT); }

	void set_ctr_used()                     { regin.set(REG_BIT_CTR); }
	void set_lr_used()                      { regin.set(REG_BIT_LR); }
	void set_ctr_modified()                 { regout.set(REG_BIT_CTR); }
	void set_lr_modified()                  { regout.set(REG_BIT_LR); }

	void set_fpscr_used(unsigned n)         { regin.set(REG_BIT_FPSCR0 + n); }
	void set_fpscr_modified(unsigned n)     { regout.set(REG_BIT_FPSCR0 + n); }

	uint32_t cr_modified() const            { return regmask_field<64, 32>(regout); }
	uint8_t cr_modified(unsigned n) const   { return reg_cr(regout, n); }
	uint32_t cr_required() const            { return regmask_field<64, 32>(regreq); }
	uint8_t cr_required(unsigned n) const   { return reg_cr(regreq, n); }

	bool xer_ca_required() const            { return regreq[REG_BIT_XER_CA]; }
	bool xer_ov_required() const            { return regreq[REG_BIT_XER_OV]; }
	bool xer_so_required() const            { return regreq[REG_BIT_XER_SO]; }
	bool xer_count_required() const         { return regreq[REG_BIT_XER_COUNT]; }

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
		static_assert(REG_BIT_COUNT <= 128);

		opcode_desc_base::reset(curpc, in_delay_slot);

		physpc = curpc;
		opptr = 0;
		cycles = 0;
		m_extra_flags.reset();
	}

	void log_flags(std::ostream &stream) const;
	void log_registers_used(std::ostream &stream) const;
	void log_registers_modified(std::ostream &stream) const;

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
		REG_BIT_FPSCR0,

		REG_BIT_COUNT = REG_BIT_FPSCR0 + 8
	};

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

	// TODO: uncomment constexpr when GCC/GNU libstdc++ catch up
	static /*constexpr*/ bool reg_cr_bit(const regmask &r, unsigned n)
	{
		return r[64 + 31 - n];
	}
	static /*constexpr*/ uint8_t reg_cr(const regmask &r, unsigned n)
	{
		return uint8_t(((r << ((4 * n) + 32)) >> (128 - 4)).to_ulong());
	}

	static void log_register_list(std::ostream &stream, const regmask &reglist, const regmask *regnostarlist);

	std::bitset<EXTRA_FLAG_COUNT> m_extra_flags;
};

class ppc_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	// construction/destruction
	frontend(ppc_device &ppc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

protected:
	bool describe(opcode_desc &desc, const opcode_desc *prev);

	// inlines
	static constexpr uint32_t compute_spr(uint32_t spr) { return ((spr >> 5) | (spr << 5)) & 0x3ff; }
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
