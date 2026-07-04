// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#ifndef MAME_CPU_I386_X87PRIV_H
#define MAME_CPU_I386_X87PRIV_H

#pragma once

#include "softfloat3/bochs_ext/softfloat-extra.h"
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

#define UNIMPLEMENTED           fatalerror("Unimplemented x87 op: %s (PC:%x)\n", __FUNCTION__, m_pc)


  /*************************************
   *
   * Constants
   *
   *************************************/

static const extFloat80_t fx80_zero = packToExtF80(0, 0x0000, 0x0000000000000000ULL);
static const extFloat80_t fx80_one  = packToExtF80(0, 0x3fff, 0x8000000000000000ULL);
static const extFloat80_t fx80_ninf = packToExtF80(1, 0x7fff, 0x8000000000000000ULL);
static const extFloat80_t fx80_inan = packToExtF80(1, 0x7fff, 0xC000000000000000ULL);

/* Maps x87 round modes to SoftFloat round modes */
static const int x87_to_sf_rc[4] =
{
	softfloat_round_near_even,
	softfloat_round_min,
	softfloat_round_max,
	softfloat_round_minMag,
};


/*************************************
 *
 * SoftFloat helpers
 *
 *************************************/

static inline bool floatx80_is_quiet_nan(extFloat80_t a)
{
	uint64_t aLow;

	aLow = a.signif & ~(0x4000000000000000ULL);
	return
		((a.signExp & 0x7FFF) == 0x7FFF)
		&& (uint64_t)(aLow << 1)
		&& (a.signif != aLow);
}

static inline int floatx80_is_zero(extFloat80_t fx)
{
	return (((fx.signExp & 0x7fff) == 0) && ((fx.signif << 1) == 0));
}

static inline int floatx80_is_inf(extFloat80_t fx)
{
	return (((fx.signExp & 0x7fff) == 0x7fff) && ((fx.signif << 1) == 0));
}

static inline int floatx80_is_denormal(extFloat80_t fx)
{
	return (((fx.signExp & 0x7fff) == 0) &&
		((fx.signif & 0x8000000000000000U) == 0) &&
		((fx.signif << 1) != 0));
}

static inline extFloat80_t floatx80_abs(extFloat80_t fx)
{
	fx.signExp &= 0x7fff;
	return fx;
}

static inline double fx80_to_double(extFloat80_t fx)
{
	float64_t d = extF80_to_f64(fx);
	return *(double*)&d;
}

extFloat80_t i386_device::READ80(uint32_t ea)
{
	extFloat80_t t;

	t.signif = READ64(ea);
	t.signExp = READ16(ea + 8);

	return t;
}

void i386_device::WRITE80(uint32_t ea, extFloat80_t t)
{
	WRITE64(ea, t.signif);
	WRITE16(ea + 8, t.signExp);
}

#endif // MAME_CPU_I386_X87PRIV_H
