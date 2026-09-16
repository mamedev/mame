// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    e1fe.h

    Hyperstone E1 instruction decoder

***************************************************************************/
#ifndef MAME_CPU_E132XS_E1FE_H
#define MAME_CPU_E132XS_E1FE_H

#pragma once

#include "e132xs.h"

#include "cpu/drcfe.h"

#include <algorithm>
#include <cassert>
#include <bitset>
#include <iosfwd>


class hyperstone_device::opcode_desc : public opcode_desc_base<opcode_desc, 40>
{
public:
	uint16_t        opptr[3];
	uint8_t         dst_code;
	uint8_t         src_code;
	bool            dst_local;
	bool            src_local;
	uint32_t        imm;

	void set_g_used(unsigned n)
	{
		regin.set(REG_G0 + n);
		if (n == 1) // SR
		{
			regin.set(REG_C);
			regin.set(REG_Z);
			regin.set(REG_N);
			regin.set(REG_V);
			regin.set(REG_FP);
		}
	}

	void set_c_used() { regin.set(REG_C); }
	void set_z_used() { regin.set(REG_Z); }
	void set_n_used() { regin.set(REG_N); }
	void set_v_used() { regin.set(REG_V); }
	void set_fp_used() { regin.set(REG_FP); }

	void set_cz_used()
	{
		set_c_used();
		set_z_used();
	}

	void set_g_modified(unsigned n)
	{
		regout.set(REG_G0 + n);
		if (n == 1) // SR
		{
			regout.set(REG_C);
			regout.set(REG_Z);
			regout.set(REG_N);
			regout.set(REG_V);
		}
	}

	void set_z_modified()
	{
		regout.set(REG_Z);
	}

	void set_znv_modified()
	{
		regout.set(REG_Z);
		regout.set(REG_N);
		regout.set(REG_V);
	}

	void set_czn_modified()
	{
		regout.set(REG_C);
		regout.set(REG_Z);
		regout.set(REG_N);
	}

	void set_cznv_modified()
	{
		regout.set(REG_C);
		regout.set(REG_Z);
		regout.set(REG_N);
		regout.set(REG_V);
	}

	void set_fp_modified()
	{
		regout.set(REG_FP);
	}

	bool c_calc_required() const { return regreq[REG_C] || in_delay_slot(); }
	bool z_calc_required() const { return regreq[REG_Z] || in_delay_slot(); }
	bool n_calc_required() const { return regreq[REG_N] || in_delay_slot(); }
	bool v_calc_required() const { return regreq[REG_V] || in_delay_slot(); }
	bool condition_calc_required() const { return regreq[REG_C] || regreq[REG_Z] || regreq[REG_N] || regreq[REG_V] || in_delay_slot(); }

	void set_check_h() { m_extra_flags.set(CHECK_H); }
	void set_can_change_modes() { m_extra_flags.set(CAN_CHANGE_MODES); }
	void set_reads_memory() { m_extra_flags.set(READS_MEMORY); }
	void set_writes_memory() { m_extra_flags.set(WRITES_MEMORY); }

	bool check_h() const { return m_extra_flags[CHECK_H]; }
	bool can_change_modes() const { return m_extra_flags[CAN_CHANGE_MODES]; }
	bool reads_memory() const { return m_extra_flags[READS_MEMORY]; }
	bool writes_memory() const { return m_extra_flags[WRITES_MEMORY]; }

	bool dst_is_src() const { return (dst_local == src_local) && (dst_code == src_code); }
	bool dst_is_pc() const { return !dst_local && (dst_code == 0); }
	bool dst_is_sr() const { return !dst_local && (dst_code == 1); }
	bool src_is_pc() const { return !src_local && (src_code == 0); }
	bool src_is_sr() const { return !src_local && (src_code == 1); }

	// epc - compute the exception PC
	uint32_t epc() const
	{
		return in_delay_slot() ? branch->pc : pc;
	}

	// pc_value - value of PC when used for arithmetic
	uint32_t pc_value() const
	{
		return in_delay_slot() ? branch->targetpc : (pc + length);
	}

	bool pc_value_unknown() const
	{
		return in_delay_slot() && (branch->targetpc == BRANCH_TARGET_DYNAMIC);
	}

	void reset(offs_t curpc, bool in_delay_slot)
	{
		static_assert(REG_COUNT <= 40);

		opcode_desc_base::reset(curpc, in_delay_slot);

		std::fill(std::begin(opptr), std::end(opptr), 0);
		dst_code = ~uint8_t(0);
		src_code = ~uint8_t(0);
		dst_local = false;
		src_local = false;
		imm = 0x80000000;
		m_extra_flags.reset();
	}

	void log_flags(std::ostream &stream) const;
	void log_registers_used(std::ostream &stream) const;
	void log_registers_modified(std::ostream &stream) const;

protected:
	enum
	{
		REG_G0 = 0,
		REG_PC = REG_G0,
		REG_SR = REG_G0 + 1,

		REG_C = REG_G0 + 32,
		REG_Z,
		REG_N,
		REG_V,
		REG_FP,

		REG_COUNT
	};

	enum
	{
		CHECK_H = 0,
		CAN_CHANGE_MODES,
		READS_MEMORY,
		WRITES_MEMORY,

		EXTRA_FLAG_COUNT
	};

	static void log_register_list(std::ostream &stream, const regmask &reglist, const regmask *regnostarlist);

	std::bitset<EXTRA_FLAG_COUNT> m_extra_flags;
};


class hyperstone_device::frontend : public drc_frontend_base<opcode_desc>
{
public:
	frontend(hyperstone_device &cpu, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
	~frontend();

	opcode_desc const *describe_code(offs_t startpc);

private:
	bool describe(opcode_desc &desc, opcode_desc const *prev);

	void read_op(opcode_desc &desc);
	void read_imm1(opcode_desc &desc);
	void read_imm2(opcode_desc &desc);
	void decode_const(opcode_desc &desc);

	void decode_ll(opcode_desc &desc);
	void decode_llext(opcode_desc &desc);
	void decode_lr(opcode_desc &desc);
	void decode_rr(opcode_desc &desc);
	void decode_ln(opcode_desc &desc);
	void decode_rn(opcode_desc &desc);
	void decode_pcrel(opcode_desc &desc);
	void decode_lrconst(opcode_desc &desc);
	void decode_rrconst(opcode_desc &desc);
	void decode_rrdis(opcode_desc &desc);
	void decode_rimm(opcode_desc &desc, bool cmpbi_andni);
	void decode_rrlim(opcode_desc &desc);

	hyperstone_device &m_cpu;
};

#endif // MAME_CPU_E132XS_E1FE_H
