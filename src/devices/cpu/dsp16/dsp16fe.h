// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series instruction analyser

***************************************************************************/
#ifndef MAME_CPU_DSP16_DSP16FE_H
#define MAME_CPU_DSP16_DSP16FE_H

#pragma once

#include "dsp16.h"

#include "cpu/drcfe.h"


class dsp16_device_base::frontend : public drc_frontend
{
public:
	enum : u8
	{
		// in the same order as the R field for convenience
		// only include registers useful for dependency analysis here

		REG_BIT_YAAU_R0 = 0x00U,
		REG_BIT_YAAU_R1,
		REG_BIT_YAAU_R2,
		REG_BIT_YAAU_R3,
		REG_BIT_YAAU_J,
		REG_BIT_YAAU_K,
		REG_BIT_YAAU_RB,
		REG_BIT_YAAU_RE,

		REG_BIT_XAAU_PT = 0x08U,
		REG_BIT_XAAU_PR,
		REG_BIT_XAAU_PI, // handled specially
		REG_BIT_XAAU_I,

		REG_BIT_DAU_X = 0x10U,
		REG_BIT_DAU_Y,
		// yl isn't an independent register
		REG_BIT_DAU_AUC = 0x13U,
		REG_BIT_DAU_PSW,
		REG_BIT_DAU_C0,
		REG_BIT_DAU_C1,
		REG_BIT_DAU_C2,

		REG_BIT_SIO_SIOC = 0x18U,
		REG_BIT_SIO_SRTA,
		REG_BIT_SIO_SDX,
		REG_BIT_SIO_TDMS,

		// registers not accessible via the R field in gaps
		REG_BIT_DAU_P = 0x0dU,
		REG_BIT_DAU_A0,
		REG_BIT_DAU_A1
	};

	// construction/destruction
	frontend(dsp16_device_base &host, u32 window_start, u32 window_end, u32 max_sequence);

protected:
	// drc_frontend implementation
	virtual bool describe(opcode_desc &desc, opcode_desc const *prev) override;

private:
	// program fetch helpers
	u16 read_op(opcode_desc const &desc, u16 offset) const;
	u16 read_op(opcode_desc const &desc) const { return read_op(desc, 0U); }
	u16 read_imm(opcode_desc const &desc) const { return read_op(desc, 1U); }

	// non-trivial instruction helpers
	bool describe_goto_b(opcode_desc &desc, u16 op);
	bool describe_if_con(opcode_desc &desc, u16 op);
	bool describe_icall(opcode_desc &desc, u16 op);
	bool describe_do(opcode_desc &desc, u16 op);
	bool describe_redo(opcode_desc &desc, u16 op);

	// sub-operation helpers
	static void describe_r(opcode_desc &desc, u16 op, bool read, bool write);
	static void describe_con(opcode_desc &desc, u16 op, bool inc);
	static void describe_f1(opcode_desc &desc, u16 op);
	static void describe_f2(opcode_desc &desc, u16 op);
	static void describe_x(opcode_desc &desc, u16 op);
	static void describe_y(opcode_desc &desc, u16 op, bool read, bool write);
	static void describe_z(opcode_desc &desc, u16 op);

	// field access helpers
	static void flag_reg(u32 (&flags)[4]) { }
	template <typename... T> static void flag_reg(u32 (&flags)[4], u8 reg0, T... regn)
	{
		flags[reg0 >> 5] |= (u32(1) << (reg0 & (u32(32) - 1)));
		flag_reg(flags, regn...);
	}
	template <typename... T> static void flag_input_reg(opcode_desc &desc, T... reg) { flag_reg(desc.regin, reg...); }
	template <typename... T> static void flag_output_reg(opcode_desc &desc, T... reg) { flag_reg(desc.regout, reg...); }
	template <typename... T> static void flag_required_output_reg(opcode_desc &desc, T... reg) { flag_reg(desc.regreq, reg...); }

	// need access to host device for program fetch
	dsp16_device_base const &m_host;

	// for making sweeping assumptions
	u32 m_cache_cycles = 0U, m_cache_last_cycles = 0U, m_cache_flags = 0U;
	decltype(opcode_desc::regin) m_cache_regin, m_cache_regout, m_cache_regreq;
	bool m_cache_valid = false;
};

#endif // MAME_CPU_DSP16_DSP16FE_H
