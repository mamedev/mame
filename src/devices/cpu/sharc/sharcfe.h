// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for SHARC recompiler

******************************************************************************/
#ifndef MAME_CPU_SHARC_SHARCFE_H
#define MAME_CPU_SHARC_SHARCFE_H

#pragma once

#include "sharc.h"

#include "cpu/drcfe.h"

#include <cassert>
#include <bitset>
#include <iosfwd>


class adsp21062_device::opcode_desc : public opcode_desc_base<opcode_desc, 96>
{
public:
	enum
	{
		LOOP = 0,
		ASTAT_DELAY_COPY_AZ,
		ASTAT_DELAY_COPY_AV,
		ASTAT_DELAY_COPY_AN,
		ASTAT_DELAY_COPY_AC,
		ASTAT_DELAY_COPY_MV,
		ASTAT_DELAY_COPY_MN,
		ASTAT_DELAY_COPY_AF,
		ASTAT_DELAY_COPY_SV,
		ASTAT_DELAY_COPY_SZ,
		ASTAT_DELAY_COPY_BTF,
		CALL,

		READS_MEMORY,
		WRITES_MEMORY,

		EXTRA_FLAG_COUNT
	};

	using extra_flags = std::bitset<EXTRA_FLAG_COUNT>;

	uint64_t        opptr;                  // copy of opcode

	void set_reg_used(unsigned x)       { regin.set(REG_R0 + x); }
	void set_reg_modified(unsigned x)   { regout.set(REG_R0 + x); }

	void set_az_used()                  { regin.set(REG_AZ); }
	void set_az_modified()              { regout.set(REG_AZ); }
	void set_av_used()                  { regin.set(REG_AV); }
	void set_av_modified()              { regout.set(REG_AV); }
	void set_an_used()                  { regin.set(REG_AN); }
	void set_an_modified()              { regout.set(REG_AN); }
	void set_ac_used()                  { regin.set(REG_AC); }
	void set_ac_modified()              { regout.set(REG_AC); }
	void set_as_used()                  { regin.set(REG_AS); }
	void set_as_modified()              { regout.set(REG_AS); }
	void set_ai_used()                  { regin.set(REG_AI); }
	void set_ai_modified()              { regout.set(REG_AI); }
	void set_mn_used()                  { regin.set(REG_MN); }
	void set_mn_modified()              { regout.set(REG_MN); }
	void set_mv_used()                  { regin.set(REG_MV); }
	void set_mv_modified()              { regout.set(REG_MV); }
	void set_mu_used()                  { regin.set(REG_MU); }
	void set_mu_modified()              { regout.set(REG_MU); }
	void set_mi_used()                  { regin.set(REG_MI); }
	void set_mi_modified()              { regout.set(REG_MI); }
	void set_af_used()                  { regin.set(REG_AF); }
	void set_af_modified()              { regout.set(REG_AF); }
	void set_sv_used()                  { regin.set(REG_SV); }
	void set_sv_modified()              { regout.set(REG_SV); }
	void set_sz_used()                  { regin.set(REG_SZ); }
	void set_sz_modified()              { regout.set(REG_SZ); }
	void set_ss_used()                  { regin.set(REG_SS); }
	void set_ss_modified()              { regout.set(REG_SS); }
	void set_btf_used()                 { regin.set(REG_BTF); }
	void set_btf_modified()             { regout.set(REG_BTF); }

	void set_dm_i_used(unsigned x)      { regin.set(REG_DM_I0 + x); }
	void set_dm_i_modified(unsigned x)  { regout.set(REG_DM_I0 + x); }
	void set_dm_m_used(unsigned x)      { regin.set(REG_DM_M0 + x); }
	void set_dm_m_modified(unsigned x)  { regout.set(REG_DM_M0 + x); }
	void set_dm_l_used(unsigned x)      { regin.set(REG_DM_L0 + x); }
	void set_dm_l_modified(unsigned x)  { regout.set(REG_DM_L0 + x); }
	void set_dm_b_used(unsigned x)      { regin.set(REG_DM_B0 + x); }
	void set_dm_b_modified(unsigned x)  { regout.set(REG_DM_B0 + x); }

	void set_pm_i_used(unsigned x)      { regin.set(REG_PM_I0 + x); }
	void set_pm_i_modified(unsigned x)  { regout.set(REG_PM_I0 + x); }
	void set_pm_m_used(unsigned x)      { regin.set(REG_PM_M0 + x); }
	void set_pm_m_modified(unsigned x)  { regout.set(REG_PM_M0 + x); }
	void set_pm_l_used(unsigned x)      { regin.set(REG_PM_L0 + x); }
	void set_pm_l_modified(unsigned x)  { regout.set(REG_PM_L0 + x); }
	void set_pm_b_used(unsigned x)      { regin.set(REG_PM_B0 + x); }
	void set_pm_b_modified(unsigned x)  { regout.set(REG_PM_B0 + x); }

	void set_alu_flags_modified()
	{
		set_az_modified();
		set_av_modified();
		set_an_modified();
		set_ac_modified();
		set_as_modified();
		set_ai_modified();
		set_af_modified();
	}

	void set_mult_flags_modified()
	{
		set_mn_modified();
		set_mv_modified();
		set_mu_modified();
		set_mi_modified();
	}

	void set_shift_flags_modified()
	{
		set_sv_modified();
		set_sz_modified();
		set_ss_modified();
	}

	bool az_used() const                { return regin[REG_AZ]; }
	bool av_used() const                { return regin[REG_AV]; }
	bool an_used() const                { return regin[REG_AN]; }
	bool ac_used() const                { return regin[REG_AC]; }
	bool as_used() const                { return regin[REG_AS]; }
	bool ai_used() const                { return regin[REG_AI]; }
	bool mn_used() const                { return regin[REG_MN]; }
	bool mv_used() const                { return regin[REG_MV]; }
	bool mu_used() const                { return regin[REG_MU]; }
	bool mi_used() const                { return regin[REG_MI]; }
	bool af_used() const                { return regin[REG_AF]; }
	bool sv_used() const                { return regin[REG_SV]; }
	bool sz_used() const                { return regin[REG_SZ]; }
	bool ss_used() const                { return regin[REG_SS]; }
	bool btf_used() const               { return regin[REG_BTF]; }

	bool az_calc_required() const       { return regreq[REG_AZ] || in_delay_slot(); }
	bool av_calc_required() const       { return regreq[REG_AV] || in_delay_slot(); }
	bool an_calc_required() const       { return regreq[REG_AN] || in_delay_slot(); }
	bool ac_calc_required() const       { return regreq[REG_AC] || in_delay_slot(); }
	bool as_calc_required() const       { return regreq[REG_AS] || in_delay_slot(); }
	bool ai_calc_required() const       { return regreq[REG_AI] || in_delay_slot(); }
	bool mn_calc_required() const       { return regreq[REG_MN] || in_delay_slot(); }
	bool mv_calc_required() const       { return regreq[REG_MV] || in_delay_slot(); }
	bool mu_calc_required() const       { return regreq[REG_MU] || in_delay_slot(); }
	bool mi_calc_required() const       { return regreq[REG_MI] || in_delay_slot(); }
	bool af_calc_required() const       { return regreq[REG_AF] || in_delay_slot(); }
	bool sv_calc_required() const       { return regreq[REG_SV] || in_delay_slot(); }
	bool sz_calc_required() const       { return regreq[REG_SZ] || in_delay_slot(); }
	bool ss_calc_required() const       { return regreq[REG_SS] || in_delay_slot(); }
	bool btf_calc_required() const      { return regreq[REG_BTF] || in_delay_slot(); }

	void set_loop()                     { m_extra_flags.set(LOOP); }
	void set_astat_delay_copy_az()      { m_extra_flags.set(ASTAT_DELAY_COPY_AZ); }
	void set_astat_delay_copy_av()      { m_extra_flags.set(ASTAT_DELAY_COPY_AV); }
	void set_astat_delay_copy_an()      { m_extra_flags.set(ASTAT_DELAY_COPY_AN); }
	void set_astat_delay_copy_ac()      { m_extra_flags.set(ASTAT_DELAY_COPY_AC); }
	void set_astat_delay_copy_mv()      { m_extra_flags.set(ASTAT_DELAY_COPY_MV); }
	void set_astat_delay_copy_mn()      { m_extra_flags.set(ASTAT_DELAY_COPY_MN); }
	void set_astat_delay_copy_af()      { m_extra_flags.set(ASTAT_DELAY_COPY_AF); }
	void set_astat_delay_copy_sv()      { m_extra_flags.set(ASTAT_DELAY_COPY_SV); }
	void set_astat_delay_copy_sz()      { m_extra_flags.set(ASTAT_DELAY_COPY_SZ); }
	void set_astat_delay_copy_btf()     { m_extra_flags.set(ASTAT_DELAY_COPY_BTF); }
	void set_call()                     { m_extra_flags.set(CALL); }
	void set_reads_memory()             { m_extra_flags.set(READS_MEMORY); }
	void set_writes_memory()            { m_extra_flags.set(WRITES_MEMORY); }

	void set_extra_flags(extra_flags const &flags)
	{
		m_extra_flags |= flags;
	}

	bool loop() const                   { return m_extra_flags[LOOP]; }
	bool astat_delay_copy_az() const    { return m_extra_flags[ASTAT_DELAY_COPY_AZ]; }
	bool astat_delay_copy_av() const    { return m_extra_flags[ASTAT_DELAY_COPY_AV]; }
	bool astat_delay_copy_an() const    { return m_extra_flags[ASTAT_DELAY_COPY_AN]; }
	bool astat_delay_copy_ac() const    { return m_extra_flags[ASTAT_DELAY_COPY_AC]; }
	bool astat_delay_copy_mv() const    { return m_extra_flags[ASTAT_DELAY_COPY_MV]; }
	bool astat_delay_copy_mn() const    { return m_extra_flags[ASTAT_DELAY_COPY_MN]; }
	bool astat_delay_copy_af() const    { return m_extra_flags[ASTAT_DELAY_COPY_AF]; }
	bool astat_delay_copy_sv() const    { return m_extra_flags[ASTAT_DELAY_COPY_SV]; }
	bool astat_delay_copy_sz() const    { return m_extra_flags[ASTAT_DELAY_COPY_SZ]; }
	bool astat_delay_copy_btf() const   { return m_extra_flags[ASTAT_DELAY_COPY_BTF]; }
	bool call() const                   { return m_extra_flags[CALL]; }
	bool reads_memory() const           { return m_extra_flags[READS_MEMORY]; }
	bool writes_memory() const          { return m_extra_flags[WRITES_MEMORY]; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_COUNT <= 96);

		opcode_desc_base::reset(curpc, in_delay_slot);

		opptr = 0;
		m_extra_flags.reset();
	}

	void log_flags(std::ostream &stream) const;
	void log_registers_used(std::ostream &stream) const;
	void log_registers_modified(std::ostream &stream) const;

private:
	enum
	{
		REG_R0 = 0,

		REG_AZ = REG_R0 + 16,
		REG_AV,
		REG_AN,
		REG_AC,
		REG_AS,
		REG_AI,
		REG_MN,
		REG_MV,
		REG_MU,
		REG_MI,
		REG_AF,
		REG_SV,
		REG_SZ,
		REG_SS,
		REG_BTF,

		REG_DM_I0,
		REG_DM_M0 = REG_DM_I0 + 8,
		REG_DM_L0 = REG_DM_M0 + 8,
		REG_DM_B0 = REG_DM_L0 + 8,

		REG_PM_I0 = REG_DM_B0 + 8,
		REG_PM_M0 = REG_PM_I0 + 8,
		REG_PM_L0 = REG_PM_M0 + 8,
		REG_PM_B0 = REG_PM_L0 + 8,

		REG_COUNT = REG_PM_B0 + 8
	};

	static void log_register_list(std::ostream &stream, const regmask &reglist, const regmask *regnostarlist);

	extra_flags m_extra_flags;
};


class adsp21062_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	frontend(adsp21062_device *sharc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

	void flush();

private:
	enum UREG_ACCESS
	{
		UREG_READ,
		UREG_WRITE
	};

	enum LOOP_TYPE
	{
		LOOP_TYPE_COUNTER,
		LOOP_TYPE_CONDITIONAL
	};

	enum LOOP_ENTRY_TYPE
	{
		LOOP_ENTRY_START = 0x1,
		LOOP_ENTRY_EVALUATION = 0x2,
		LOOP_ENTRY_ASTAT_CHECK = 0x4,
	};

	struct LOOP_ENTRY
	{
		uint16_t entrytype;
		opcode_desc::extra_flags userflags;

		void clear() { entrytype = 0; userflags.reset(); }
	};

	struct LOOP_DESCRIPTOR
	{
		uint32_t start_pc;
		uint32_t end_pc;
		uint32_t astat_check_pc;
		LOOP_TYPE type;
		int condition;
	};

	bool describe(opcode_desc &desc, const opcode_desc *prev);

	bool describe_compute(opcode_desc &desc, uint64_t opcode);
	bool describe_ureg_access(opcode_desc &desc, int reg, UREG_ACCESS access);
	bool describe_shiftop_imm(opcode_desc &desc, int shiftop, int rn, int rx);
	void describe_if_condition(opcode_desc &desc, int condition);

	void insert_loop(const LOOP_DESCRIPTOR &loopdesc);
	void add_loop_entry(uint32_t pc, uint8_t type, opcode_desc::extra_flags const &userflags);
	bool is_loop_evaluation(uint32_t pc);
	bool is_loop_start(uint32_t pc);
	bool is_astat_delay_check(uint32_t pc);

	adsp21062_device *m_sharc;

	std::unique_ptr<LOOP_ENTRY[]> m_loopmap;
};

#endif // MAME_CPU_SHARC_SHARCFE_H
