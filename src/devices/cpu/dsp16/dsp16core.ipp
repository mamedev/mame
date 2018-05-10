// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series core state

***************************************************************************/
#ifndef MAME_CPU_DSP16_DSP16CORE_IPP
#define MAME_CPU_DSP16_DSP16CORE_IPP

#pragma once

#include "dsp16core.h"


/***********************************************************************
    YAAU helpers
***********************************************************************/

inline u16 &dsp16_device_base::core_state::yaau_postmodify_r(u16 op)
{
	u16 &r(op_yaau_r(op));
	switch (op & 0x0003U)
	{
	case 0x0: // *rN
		break;
	case 0x1: // *rN++
		r = (yaau_re && (yaau_re == r)) ? yaau_rb : (r + 1);
		break;
	case 0x2: // *rN--
		--r;
		break;
	case 0x3: // *rN++j
		r += yaau_j;
		break;
	}
	return r &= yaau_mask;
}

/***********************************************************************
    DAU helpers
***********************************************************************/

inline u64 dsp16_device_base::core_state::dau_f1(u16 op)
{
	s64 const &s(op_dau_as(op));
	s64 d(0);
	switch (op_f1(op))
	{
	case 0x0: // aD = p ; p = x*y
		d = dau_get_p_aligned();
		dau_p = dau_x * (dau_y >> 16);
		break;
	case 0x1: // aD = aS + p ; p = x*y
		d = s + dau_get_p_aligned();
		dau_p = dau_x * (dau_y >> 16);
		break;
	case 0x2: // p = x*y
		dau_p = dau_x * (dau_y >> 16);
		return op_dau_ad(op);
	case 0x3: // aD = aS - p ; p = x*y
		d = s - dau_get_p_aligned();
		dau_p = dau_x * (dau_y >> 16);
		break;
	case 0x4: // aD = p
		d = dau_get_p_aligned();
		break;
	case 0x5: // aD = aS + p
		d = s + dau_get_p_aligned();
		break;
	case 0x6: // NOP
		return op_dau_ad(op);
	case 0x7: // aD = aS - p
		d = s - dau_get_p_aligned();
		break;
	case 0x8: // aD = aS | y
		d = s | dau_y;
		break;
	case 0x9: // aD = aS ^ y
		d = s ^ dau_y;
		break;
	case 0xa: // aS & y
		dau_set_psw_flags(s & dau_y);
		return op_dau_ad(op);
	case 0xb: // aS - y
		dau_set_psw_flags(s - dau_y);
		return op_dau_ad(op);
	case 0xc: // aD = y
		d = dau_y;
		break;
	case 0xd: // aD = aS + y
		d = s + dau_y;
		break;
	case 0xe: // aD = aS & y
		d = s & dau_y;
		break;
	case 0xf: // aD = aS - y
		d = s - dau_y;
		break;
	}
	return dau_set_psw_flags(d);
}

inline void dsp16_device_base::core_state::dau_f2(u16 op)
{
	s64 const &s(op_dau_as(op));
	s64 &d(op_dau_ad(op));
	switch (op_f2(op))
	{
	case 0x0: // aD = aS >> 1
		d = s >> 1;
		break;
	case 0x1: // aD = aS << 1
		d = s32(u32(u64(s)) << 1);
		break;
	case 0x2: // aD = aS >> 4
		d = s >> 4;
		break;
	case 0x3: // aD = aS << 4
		d = s32(u32(u64(s)) << 4);
		break;
	case 0x4: // aD = aS >> 8
		d = s >> 8;
		break;
	case 0x5: // aD = aS << 8
		d = s32(u32(u64(s)) << 8);
		break;
	case 0x6: // aD = aS >> 16
		d = s >> 16;
		break;
	case 0x7: // aD = aS << 16
		d = s32(u32(u64(s)) << 16);
		break;
	case 0x8: // aD = p
		d = dau_get_p_aligned();
		break;
	case 0x9: // aDh = aSh + 1
		d = s64(s32(u32(u64(s)) & ~((u32(1) << 16) - 1))) + (s32(1) << 16);
		if (BIT(dau_auc, op_s(op) + 4))
			d |= op_dau_ad(op) & ((s64(1) << 16) - 1);
		break;
	case 0xa: // Reserved
		throw emu_fatalerror("DSP16: reserved F2 value %01X (PC = %04X)\n", op_f2(op), xaau_pc/*FIXME: st_pcbase*/);
	case 0xb: // aD = rnd(aS)
		// FIXME: behaviour is not clear
		// p 3-13: "Round upper 20 bits of accumulator."
		// p 3-14: "The contents of the source accumulator, aS, are rounded to 16 bits, and the sign-extended result is placed in aD[35 - 16] with zeroes in aD[15 - 0]."
		// It presumably rounds to nearest, but does it yield a 16-bit or 20-bit result, and what does it do about overflow?
		d = (s + ((s32(1) << 15) - ((0 > s) ? 1 : 0))) & ~((u64(1) << 16) - 1);
		break;
	case 0xc: // aD = y
		d = dau_y;
		break;
	case 0xd: // aD = aS + 1
		d = s + 1;
		break;
	case 0xe: // aD = aS
		d = s;
		break;
	case 0xf: // aD = -aS
		// FIXME: does this detect negation of largest negative number as overflow?
		d = -s;
		break;
	}
	d = dau_set_psw_flags(d);
}

inline s64 &dsp16_device_base::core_state::dau_set_atx(u16 op, s16 value)
{
	s64 &at(op_dau_at(op));
	if (op_x(op))
	{
		bool const clear(!BIT(dau_psw, 4 + op_d(~op)));
		return at = s32((u32(u16(value)) << 16) | u32(clear ? u16(0) : u16(u64(at))));
	}
	else
	{
		return at = (at & ~((s64(1) << 16) - 1)) | u64(u16(value));
	}
}

inline s64 &dsp16_device_base::core_state::dau_set_at(u16 op, s16 value)
{
	s64 &at(op_dau_at(op));
	bool const clear(!BIT(dau_psw, 4 + op_d(~op)));
	return at = s32((u32(u16(value)) << 16) | u32(clear ? u16(0) : u16(u64(at))));
}

/***********************************************************************
    internal helpers
***********************************************************************/

inline u64 dsp16_device_base::core_state::dau_get_p_aligned() const
{
	// TODO: manual is contradictory
	// p 2-10: "Bits 1 and 2 of the accumulator are not changed by the load of the accumulator with the data in p, since 00 is added to or copied into these accumulator bits as indicated in Figure 2-8."
	// I'm reading this as copying p to aD clears the low two bits, but adding it leaves them unchanged.
	switch (dau_auc_align())
	{
	case 0x0:
		return dau_p;
	case 0x1:
		return dau_p >> 2;
	case 0x2:
		return u64(dau_p) << 2;
	case 0x3:
	default:
		throw emu_fatalerror("DSP16: reserved ALIGN value %01X (PC = %04X)\n", dau_auc_align(), xaau_pc/*FIXME: st_pcbase*/);
	}
}

inline u64 dsp16_device_base::core_state::dau_set_psw_flags(s64 d)
{
	dau_psw &= 0x0fffU;
	bool const negative(d & DAU_A_SIGN);
	if (negative)
		dau_psw |= 0x8000U;
	if (!(d & DAU_A_MASK))
		dau_psw |= 0x4000U;
	if ((d >> 36) != (negative ? -1 : 0))
		dau_psw |= 0x2000U;
	if (((d >> 32) & ((1 << 4) - 1)) != (BIT(d, 31) ? 15 : 0))
		dau_psw |= 0x1000U;
	if (negative)
		return d | DAU_A_EXT;
	else
		return d & DAU_A_MASK;
}

#endif // MAME_CPU_DSP16_DSP16CORE_IPP
