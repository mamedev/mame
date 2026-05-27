// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series core state

***************************************************************************/
#ifndef MAME_CPU_DSP16_DSP16CORE_H
#define MAME_CPU_DSP16_DSP16CORE_H

#pragma once

#include "dsp16.h"


class dsp16_device_base::core_state
{
public:
	// construction/destruction
	constexpr core_state(u8 yaau_bits) : yaau_mask(u16((u32(1) << yaau_bits) - 1)), yaau_sign(u16(1) << (yaau_bits - 1)) { }
	~core_state() = default;

	// setup
	void register_save_items(device_t &host);

	// execution state helpers
	constexpr bool icount_remaining() const { return 0 < icount; }
	void decrement_icount() { --icount; }

	// XAAU helpers
	constexpr u16 xaau_next_pc() const { return (xaau_pc & XAAU_I_EXT) | ((xaau_pc + 1) & XAAU_I_MASK); }
	void xaau_increment_pt(u16 op) { xaau_pt = (xaau_pt & XAAU_I_EXT) | ((xaau_pt + op_xaau_increment(op)) & XAAU_I_MASK); }
	void xaau_extend_i() { xaau_i = (xaau_i & XAAU_I_MASK) | ((xaau_i & XAAU_I_SIGN) ? XAAU_I_EXT : 0); }

	// YAAU helpers
	u16 &yaau_postmodify_r(u16 op);
	s16 &yaau_set_j(s16 value) { return yaau_j = yaau_extend(value); }
	s16 &yaau_set_k(s16 value) { return yaau_k = yaau_extend(value); }
	u16 &yaau_set_rb(s16 value) { return yaau_rb = value & yaau_mask; }
	u16 &yaau_set_re(s16 value) { return yaau_re = value & yaau_mask; }
	constexpr s16 yaau_extend(s16 value) const { return (value & yaau_mask) | ((value & yaau_sign) ? ~yaau_mask : 0); }
	s16 &yaau_extend_j() { return yaau_j = yaau_extend(yaau_j); }
	s16 &yaau_extend_k() { return yaau_k = yaau_extend(yaau_k); }

	// DAU helpers
	u64 dau_f1(u16 op);
	void dau_f2(u16 op);
	constexpr s16 dau_get_y(u16 op) const { return u16(u32(dau_y) >> (op_x(op) ? 16 : 0)); }
	void dau_set_y(u16 op, s16 value) { op_x(op) ? dau_set_y(value) : dau_set_yl(value); }
	void dau_set_y(s16 value) { dau_y = (u32(u16(value)) << 16) | u32(BIT(dau_auc, 6) ? u16(u32(dau_y)) : u16(0)); }
	void dau_set_yl(s16 value) { dau_y = (dau_y & ~((u32(1) << 16) - 1)) | u16(value); }
	s64 &dau_set_atx(u16 op, s16 value);
	s64 &dau_set_at(u16 op, s16 value);
	template <unsigned T> void dau_extend_a() { dau_a[T] = (dau_a[T] & DAU_A_MASK) | ((dau_a[T] & DAU_A_SIGN) ? DAU_A_EXT : 0); }
	u16 dau_export_psw()
	{
		return dau_psw = (dau_psw & 0xfe10U) | (u16(u64(dau_a[0]) >> 32) & 0x000fU) | (u16(u64(dau_a[1]) >> 27) & 0x01e0U);
	}
	void dau_import_psw()
	{
		dau_a[0] = u64(u32(dau_a[0])) | (u64(dau_psw & 0x000fU) << 32) | (BIT(dau_psw, 3) ? DAU_A_EXT : 0U);
		dau_a[1] = u64(u32(dau_a[1])) | (u64(dau_psw & 0x01e0U) << 27) | (BIT(dau_psw, 8) ? DAU_A_EXT : 0U);
	}

	// DAU flag accessors
	constexpr bool dau_auc_sat(u16 a) const { return bool(BIT(dau_auc, 2 + a)); }
	constexpr u16 dau_auc_align() const { return dau_auc & 0x0003U; }
	constexpr bool dau_psw_lmi() const { return bool(BIT(dau_psw, 15)); }
	constexpr bool dau_psw_leq() const { return bool(BIT(dau_psw, 14)); }
	constexpr bool dau_psw_llv() const { return bool(BIT(dau_psw, 13)); }
	constexpr bool dau_psw_lmv() const { return bool(BIT(dau_psw, 12)); }

	// opcode field handling
	constexpr s16 op_xaau_increment(u16 op) const { return op_x(op) ? xaau_i : 1; }
	u16 &op_yaau_r(u16 op) { return yaau_r[(op >> 2) & 0x0003U]; }
	s64 &op_dau_as(u16 op) { return dau_a[op_s(op)]; }
	s64 &op_dau_ad(u16 op) { return dau_a[op_d(op)]; }
	s64 &op_dau_at(u16 op) { return dau_a[op_d(~op)]; }

	// configuration
	u16 const   yaau_mask;
	u16 const   yaau_sign;

	// execution state
	int     icount = 0;

	// XAAU - ROM Address Arithmetic Unit
	u16         xaau_pc = 0U;                   // 16 bits unsigned
	u16         xaau_pt = 0U;                   // 16 bits unsigned
	u16         xaau_pr = 0U;                   // 16 bits unsigned
	u16         xaau_pi = 0U;                   // 16 bits unsigned
	s16         xaau_i = 0;                     // 12 bits signed

	// YAAU - RAM Address Arithmetic Unit
	u16         yaau_r[4] = { 0U, 0U, 0U, 0U }; // 9/16 bits unsigned
	u16         yaau_rb = 0U;                   // 9/16 bits unsigned
	u16         yaau_re = 0U;                   // 9/16 bits unsigned
	s16         yaau_j = 0;                     // 9/16 bits signed
	s16         yaau_k = 0;                     // 9/16 bits signed

	// DAU - Data Arithmetic Unit
	s16         dau_x = 0;                      // 16 bits signed
	s32         dau_y = 0;                      // 32 bits signed
	s32         dau_p = 0;                      // 32 bits signed
	s64         dau_a[2] = { 0, 0 };            // 36 bits signed
	s8          dau_c[3] = { 0, 0, 0 };         // 8 bits signed
	u8          dau_auc = 0U;                   // 7 bits unsigned
	u16         dau_psw = 0U;                   // 16 bits
	s16         dau_temp = 0;                   // 16 bits

private:
	// internal helpers
	u64 dau_get_p_aligned() const;
	u64 dau_set_psw_flags(s64 d);
};


inline void dsp16_device::core_destructer::operator()(core_state *obj) const noexcept
{
	if (obj)
		obj->~core_state();
}

#endif // MAME_CPU_DSP16_DSP16CORE_H
