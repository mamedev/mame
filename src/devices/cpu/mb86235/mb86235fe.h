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

#include <cassert>


class mb86235_device::opcode_desc : public opcode_desc_base<opcode_desc, 56>
{
public:
	enum : uint8_t { INC = 1, DEC, ZERO };

	uint64_t        opptr;                  // copy of opcode

	void set_aa_used(unsigned n)        { regin.set(REG_AA0 + n); }
	void set_aa_modified(unsigned n)    { regout.set(REG_AA0 + n); }
	void set_ab_used(unsigned n)        { regin.set(REG_AB0 + n); }
	void set_ab_modified(unsigned n)    { regout.set(REG_AB0 + n); }
	void set_ma_used(unsigned n)        { regin.set(REG_MA0 + n); }
	void set_ma_modified(unsigned n)    { regout.set(REG_MA0 + n); }
	void set_mb_used(unsigned n)        { regin.set(REG_MB0 + n); }
	void set_mb_modified(unsigned n)    { regout.set(REG_MB0 + n); }
	void set_ar_used(unsigned n)        { regin.set(REG_AR0 + n); }
	void set_ar_modified(unsigned n)    { regout.set(REG_AR0 + n); }

	void set_az_used()                  { regin.set(REG_AZ); }
	void set_az_modified()              { regout.set(REG_AZ); }
	void set_an_used()                  { regin.set(REG_AN); }
	void set_an_modified()              { regout.set(REG_AN); }
	void set_av_used()                  { regin.set(REG_AV); }
	void set_av_modified()              { regout.set(REG_AV); }
	void set_au_used()                  { regin.set(REG_AU); }
	void set_au_modified()              { regout.set(REG_AU); }
	void set_ad_used()                  { regin.set(REG_AD); }
	void set_ad_modified()              { regout.set(REG_AD); }
	void set_zc_used()                  { regin.set(REG_ZC); }
	void set_zc_modified()              { regout.set(REG_ZC); }
	void set_il_used()                  { regin.set(REG_IL); }
	void set_il_modified()              { regout.set(REG_IL); }
	void set_nr_used()                  { regin.set(REG_NR); }
	void set_nr_modified()              { regout.set(REG_NR); }
	void set_zd_used()                  { regin.set(REG_ZD); }
	void set_zd_modified()              { regout.set(REG_ZD); }
	void set_mn_used()                  { regin.set(REG_MN); }
	void set_mn_modified()              { regout.set(REG_MN); }
	void set_mz_used()                  { regin.set(REG_MZ); }
	void set_mz_modified()              { regout.set(REG_MZ); }
	void set_mv_used()                  { regin.set(REG_MV); }
	void set_mv_modified()              { regout.set(REG_MV); }
	void set_mu_used()                  { regin.set(REG_MU); }
	void set_mu_modified()              { regout.set(REG_MU); }
	void set_md_used()                  { regin.set(REG_MD); }
	void set_md_modified()              { regout.set(REG_MD); }

	bool aa_used(unsigned n) const      { return regin[REG_AA0 + n]; }
	bool aa_modified(unsigned n) const  { return regout[REG_AA0 + n]; }
	bool ab_used(unsigned n) const      { return regin[REG_AB0 + n]; }
	bool ab_modified(unsigned n) const  { return regout[REG_AB0 + n]; }
	bool ma_used(unsigned n) const      { return regin[REG_MA0 + n]; }
	bool ma_modified(unsigned n) const  { return regout[REG_MA0 + n]; }
	bool mb_used(unsigned n) const      { return regin[REG_MB0 + n]; }
	bool mb_modified(unsigned n) const  { return regout[REG_MB0 + n]; }
	bool ar_used(unsigned n) const      { return regin[REG_AR0 + n]; }
	bool ar_modified(unsigned n) const  { return regout[REG_AR0 + n]; }

	bool az_calc_required() const       { return regreq[REG_AZ] || in_delay_slot(); }
	bool an_calc_required() const       { return regreq[REG_AN] || in_delay_slot(); }
	bool av_calc_required() const       { return regreq[REG_AV] || in_delay_slot(); }
	bool au_calc_required() const       { return regreq[REG_AU] || in_delay_slot(); }
	bool ad_calc_required() const       { return regreq[REG_AD] || in_delay_slot(); }
	bool zc_calc_required() const       { return regreq[REG_ZC] || in_delay_slot(); }
	bool il_calc_required() const       { return regreq[REG_IL] || in_delay_slot(); }
	bool nr_calc_required() const       { return regreq[REG_NR] || in_delay_slot(); }
	bool zd_calc_required() const       { return regreq[REG_ZD] || in_delay_slot(); }
	bool mn_calc_required() const       { return regreq[REG_MN] || in_delay_slot(); }
	bool mz_calc_required() const       { return regreq[REG_MZ] || in_delay_slot(); }
	bool mv_calc_required() const       { return regreq[REG_MV] || in_delay_slot(); }
	bool mu_calc_required() const       { return regreq[REG_MU] || in_delay_slot(); }
	bool md_calc_required() const       { return regreq[REG_MD] || in_delay_slot(); }

	void set_reads_memory() { m_reads_memory = true; }
	void set_writes_memory() { m_writes_memory = true; }
	void set_fifoin() { m_fifoin = true; }
	void set_fifoout() { m_fifoout = true; }
	void set_repeat() { m_repeat = true; }
	void set_repeated_op() { m_repeated_op = true; }
	void set_pr_inc() { m_pr = INC; }
	void set_pr_dec() { m_pr = DEC; }
	void set_pr_zero() { m_pr = ZERO; }
	void set_pw_inc() { m_pw = INC; }
	void set_pw_dec() { m_pw = DEC; }
	void set_pw_zero() { m_pw = ZERO; }

	bool reads_memory() const { return m_reads_memory; }
	bool writes_memory() const { return m_writes_memory; }
	bool fifoin() const { return m_fifoin; }
	bool fifoout() const { return m_fifoout; }
	bool repeat() const { return m_repeat; }
	bool repeated_op() const { return m_repeated_op; }
	uint8_t pr() const { return m_pr; }
	uint8_t pw() const { return m_pw; }

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_COUNT <= 56);

		opcode_desc_base::reset(curpc, in_delay_slot);

		opptr = 0;
		m_reads_memory = false;
		m_writes_memory = false;
		m_fifoin = false;
		m_fifoout = false;
		m_repeat = false;
		m_repeated_op = false;
		m_pr = 0;
		m_pw = 0;
	}

private:
	enum
	{
		REG_AA0 = 0,
		REG_AB0 = REG_AA0 + 8,
		REG_MA0 = REG_AB0 + 8,
		REG_MB0 = REG_MA0 + 8,
		REG_AR0 = REG_MB0 + 8,

		REG_AZ = REG_AR0 + 8,
		REG_AN,
		REG_AV,
		REG_AU,
		REG_AD,
		REG_ZC,
		REG_IL,
		REG_NR,
		REG_ZD,
		REG_MN,
		REG_MZ,
		REG_MV,
		REG_MU,
		REG_MD,

		REG_COUNT
	};

	bool m_reads_memory;
	bool m_writes_memory;
	bool m_fifoin;
	bool m_fifoout;
	bool m_repeat;
	bool m_repeated_op;
	uint8_t m_pr;
	uint8_t m_pw;
};


class mb86235_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	frontend(mb86235_device *core, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

private:
	mb86235_device *m_core;

	bool describe(opcode_desc &desc, opcode_desc const *prev);

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
