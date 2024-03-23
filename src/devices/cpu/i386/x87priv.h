// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#pragma once

#ifndef __X87PRIV_H__
#define __X87PRIV_H__

#include <cmath>


/*************************************
 *
 * Defines
 *
 *************************************/

#define X87_SW_IE               0x0001
#define X87_SW_DE               0x0002
#define X87_SW_ZE               0x0004
#define X87_SW_OE               0x0008
#define X87_SW_UE               0x0010
#define X87_SW_PE               0x0020
#define X87_SW_SF               0x0040
#define X87_SW_ES               0x0080
#define X87_SW_C0               0x0100
#define X87_SW_C1               0x0200
#define X87_SW_C2               0x0400
#define X87_SW_TOP_SHIFT        11
#define X87_SW_TOP_MASK         7
#define X87_SW_C3               0x4000
#define X87_SW_BUSY             0x8000

#define X87_CW_IM               0x0001
#define X87_CW_DM               0x0002
#define X87_CW_ZM               0x0004
#define X87_CW_OM               0x0008
#define X87_CW_UM               0x0010
#define X87_CW_PM               0x0020
#define X87_CW_PC_SHIFT         8
#define X87_CW_PC_MASK          3
#define X87_CW_PC_SINGLE        0
#define X87_CW_PC_DOUBLE        2
#define X87_CW_PC_EXTEND        3
#define X87_CW_RC_SHIFT         10
#define X87_CW_RC_MASK          3
#define X87_CW_RC_NEAREST       0
#define X87_CW_RC_DOWN          1
#define X87_CW_RC_UP            2
#define X87_CW_RC_ZERO          3

#define X87_TW_MASK             3
#define X87_TW_VALID            0
#define X87_TW_ZERO             1
#define X87_TW_SPECIAL          2
#define X87_TW_EMPTY            3


 /*************************************
  *
  * Macros
  *
  *************************************/

#define ST_TO_PHYS(x)           (((m_x87_sw >> X87_SW_TOP_SHIFT) + (x)) & X87_SW_TOP_MASK)
#define ST(x)                   (m_x87_reg[ST_TO_PHYS(x)])
#define X87_TW_FIELD_SHIFT(x)   ((x) << 1)
#define X87_TAG(x)              ((m_x87_tw >> X87_TW_FIELD_SHIFT(x)) & X87_TW_MASK)
#define X87_RC                  ((m_x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK)
#define X87_IS_ST_EMPTY(x)      (X87_TAG(ST_TO_PHYS(x)) == X87_TW_EMPTY)
#define X87_SW_C3_0             X87_SW_C0

// FIXME: no reference about this across i386 package
#define UNIMPLEMENTED           throw emu_fatalerror("Unimplemented x87 op: %s (PC:%x)\n", __FUNCTION__, m_pc)


  /*************************************
   *
   * Constants
   *
   *************************************/

static const floatx80 fx80_zero = { 0x0000, 0x0000000000000000U };
static const floatx80 fx80_one = { 0x3fff, 0x8000000000000000U };

static const floatx80 fx80_ninf = { 0xffff, 0x8000000000000000U };
static const floatx80 fx80_inan = { 0xffff, 0xc000000000000000U };

/* Maps x87 round modes to SoftFloat round modes */
static const int x87_to_sf_rc[4] =
{
	float_round_nearest_even,
	float_round_down,
	float_round_up,
	float_round_to_zero,
};


/*************************************
 *
 * SoftFloat helpers
 *
 *************************************/

extern flag floatx80_is_nan(floatx80 a);

extern flag floatx80_is_signaling_nan(floatx80 a);

static inline flag floatx80_is_quiet_nan(floatx80 a)
{
	bits64 aLow;

	aLow = a.low & ~LIT64(0x4000000000000000);
	return
		((a.high & 0x7FFF) == 0x7FFF)
		&& (bits64)(aLow << 1)
		&& (a.low != aLow);
}

static inline int floatx80_is_zero(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0) && ((fx.low << 1) == 0));
}

static inline int floatx80_is_inf(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0x7fff) && ((fx.low << 1) == 0));
}

static inline int floatx80_is_denormal(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0) &&
		((fx.low & 0x8000000000000000U) == 0) &&
		((fx.low << 1) != 0));
}

static inline floatx80 floatx80_abs(floatx80 fx)
{
	fx.high &= 0x7fff;
	return fx;
}

static inline double fx80_to_double(floatx80 fx)
{
	uint64_t d = floatx80_to_float64(fx);
	return *(double*)&d;
}

static inline floatx80 double_to_fx80(double in)
{
	return float64_to_floatx80(*(uint64_t*)&in);
}

floatx80 i386_device::READ80(uint32_t ea)
{
	floatx80 t;

	t.low = READ64(ea);
	t.high = READ16(ea + 8);

	return t;
}

void i386_device::WRITE80(uint32_t ea, floatx80 t)
{
	WRITE64(ea, t.low);
	WRITE16(ea + 8, t.high);
}

#endif /* __X87PRIV_H__ */
