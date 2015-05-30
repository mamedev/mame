// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    x87 FPU emulation

    TODO:
     - 80-bit precision for F2XM1, FYL2X, FPATAN
     - Figure out why SoftFloat trig extensions produce bad values
     - Cycle counts for all processors (currently using 486 counts)
     - Precision-dependent cycle counts for divide instructions
     - Last instruction, operand pointers etc.
     - Fix FLDENV, FSTENV, FSAVE, FRSTOR and FPREM
     - Status word C2 updates to reflect round up/down
     - Handling of invalid and denormal numbers
     - Remove redundant operand checks
     - Exceptions

***************************************************************************/

#include <math.h>


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

static const floatx80 fx80_zero =   { 0x0000, U64(0x0000000000000000) };
static const floatx80 fx80_one =    { 0x3fff, U64(0x8000000000000000) };

static const floatx80 fx80_ninf =   { 0xffff, U64(0x8000000000000000) };
static const floatx80 fx80_inan =   { 0xffff, U64(0xc000000000000000) };

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

extern flag floatx80_is_nan( floatx80 a );

extern flag floatx80_is_signaling_nan(floatx80 a);

INLINE flag floatx80_is_quiet_nan(floatx80 a)
{
	bits64 aLow;

	aLow = a.low & ~LIT64(0x4000000000000000);
	return
		((a.high & 0x7FFF) == 0x7FFF)
		&& (bits64)(aLow << 1)
		&& (a.low != aLow);
}

INLINE int floatx80_is_zero(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0) && ((fx.low << 1) == 0));
}

INLINE int floatx80_is_inf(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0x7fff) && ((fx.low << 1) == 0));
}

INLINE int floatx80_is_denormal(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0) &&
			((fx.low & U64(0x8000000000000000)) == 0) &&
			((fx.low << 1) != 0));
}

INLINE floatx80 floatx80_abs(floatx80 fx)
{
	fx.high &= 0x7fff;
	return fx;
}

INLINE double fx80_to_double(floatx80 fx)
{
	UINT64 d = floatx80_to_float64(fx);
	return *(double*)&d;
}

INLINE floatx80 double_to_fx80(double in)
{
	return float64_to_floatx80(*(UINT64*)&in);
}

floatx80 i386_device::READ80(UINT32 ea)
{
	floatx80 t;

	t.low = READ64(ea);
	t.high = READ16(ea + 8);

	return t;
}

void i386_device::WRITE80(UINT32 ea, floatx80 t)
{
	WRITE64(ea, t.low);
	WRITE16(ea + 8, t.high);
}


/*************************************
 *
 * x87 stack handling
 *
 *************************************/

void i386_device::x87_set_stack_top(int top)
{
	m_x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT);
	m_x87_sw |= (top << X87_SW_TOP_SHIFT);
}

void i386_device::x87_set_tag(int reg, int tag)
{
	int shift = X87_TW_FIELD_SHIFT(reg);

	m_x87_tw &= ~(X87_TW_MASK << shift);
	m_x87_tw |= (tag << shift);
}

void i386_device::x87_write_stack(int i, floatx80 value, int update_tag)
{
	ST(i) = value;

	if (update_tag)
	{
		int tag;

		if (floatx80_is_zero(value))
		{
			tag = X87_TW_ZERO;
		}
		else if (floatx80_is_inf(value) || floatx80_is_nan(value))
		{
			tag = X87_TW_SPECIAL;
		}
		else
		{
			tag = X87_TW_VALID;
		}

		x87_set_tag(ST_TO_PHYS(i), tag);
	}
}

void i386_device::x87_set_stack_underflow()
{
	m_x87_sw &= ~X87_SW_C1;
	m_x87_sw |= X87_SW_IE | X87_SW_SF;
}

void i386_device::x87_set_stack_overflow()
{
	m_x87_sw |= X87_SW_C1 | X87_SW_IE | X87_SW_SF;
}

int i386_device::x87_inc_stack()
{
	int ret = 1;

	// Check for stack underflow
	if (X87_IS_ST_EMPTY(0))
	{
		ret = 0;
		x87_set_stack_underflow();

		// Don't update the stack if the exception is unmasked
		if (~m_x87_cw & X87_CW_IM)
			return ret;
	}

	x87_set_tag(ST_TO_PHYS(0), X87_TW_EMPTY);
	x87_set_stack_top(ST_TO_PHYS(1));
	return ret;
}

int i386_device::x87_dec_stack()
{
	int ret = 1;

	// Check for stack overflow
	if (!X87_IS_ST_EMPTY(7))
	{
		ret = 0;
		x87_set_stack_overflow();

		// Don't update the stack if the exception is unmasked
		if (~m_x87_cw & X87_CW_IM)
			return ret;
	}

	x87_set_stack_top(ST_TO_PHYS(7));
	return ret;
}


/*************************************
 *
 * Exception handling
 *
 *************************************/

int i386_device::x87_check_exceptions()
{
	/* Update the exceptions from SoftFloat */
	if (float_exception_flags & float_flag_invalid)
	{
		m_x87_sw |= X87_SW_IE;
		float_exception_flags &= ~float_flag_invalid;
	}
	if (float_exception_flags & float_flag_overflow)
	{
		m_x87_sw |= X87_SW_OE;
		float_exception_flags &= ~float_flag_overflow;
	}
	if (float_exception_flags & float_flag_underflow)
	{
		m_x87_sw |= X87_SW_UE;
		float_exception_flags &= ~float_flag_underflow;
	}
	if (float_exception_flags & float_flag_inexact)
	{
		m_x87_sw |= X87_SW_PE;
		float_exception_flags &= ~float_flag_inexact;
	}

	if ((m_x87_sw & ~m_x87_cw) & 0x3f)
	{
		// m_device->execute().set_input_line(INPUT_LINE_FERR, RAISE_LINE);
		logerror("Unmasked x87 exception (CW:%.4x, SW:%.4x)\n", m_x87_cw, m_x87_sw);
		if (m_cr[0] & 0x20) // FIXME: 486 and up only
		{
			m_ext = 1;
			i386_trap(FAULT_MF, 0, 0);
		}
		return 0;
	}

	return 1;
}

void i386_device::x87_write_cw(UINT16 cw)
{
	m_x87_cw = cw;

	/* Update the SoftFloat rounding mode */
	float_rounding_mode = x87_to_sf_rc[(m_x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK];
}

void i386_device::x87_reset()
{
	x87_write_cw(0x0037f);

	m_x87_sw = 0;
	m_x87_tw = 0xffff;

	// TODO: FEA=0, FDS=0, FIP=0 FOP=0 FCS=0
	m_x87_data_ptr = 0;
	m_x87_inst_ptr = 0;
	m_x87_opcode = 0;
}


/*************************************
 *
 * Core arithmetic
 *
 *************************************/

floatx80 i386_device::x87_add(floatx80 a, floatx80 b)
{
	floatx80 result = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32 a32 = floatx80_to_float32(a);
			float32 b32 = floatx80_to_float32(b);
			result = float32_to_floatx80(float32_add(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64 a64 = floatx80_to_float64(a);
			float64 b64 = floatx80_to_float64(b);
			result = float64_to_floatx80(float64_add(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			result = floatx80_add(a, b);
			break;
		}
	}

	return result;
}

floatx80 i386_device::x87_sub(floatx80 a, floatx80 b)
{
	floatx80 result = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32 a32 = floatx80_to_float32(a);
			float32 b32 = floatx80_to_float32(b);
			result = float32_to_floatx80(float32_sub(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64 a64 = floatx80_to_float64(a);
			float64 b64 = floatx80_to_float64(b);
			result = float64_to_floatx80(float64_sub(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			result = floatx80_sub(a, b);
			break;
		}
	}

	return result;
}

floatx80 i386_device::x87_mul(floatx80 a, floatx80 b)
{
	floatx80 val = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32 a32 = floatx80_to_float32(a);
			float32 b32 = floatx80_to_float32(b);
			val = float32_to_floatx80(float32_mul(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64 a64 = floatx80_to_float64(a);
			float64 b64 = floatx80_to_float64(b);
			val = float64_to_floatx80(float64_mul(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			val = floatx80_mul(a, b);
			break;
		}
	}

	return val;
}


floatx80 i386_device::x87_div(floatx80 a, floatx80 b)
{
	floatx80 val = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32 a32 = floatx80_to_float32(a);
			float32 b32 = floatx80_to_float32(b);
			val = float32_to_floatx80(float32_div(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64 a64 = floatx80_to_float64(a);
			float64 b64 = floatx80_to_float64(b);
			val = float64_to_floatx80(float64_div(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			val = floatx80_div(a, b);
			break;
		}
	}
	return val;
}


/*************************************
 *
 * Instructions
 *
 *************************************/

/*************************************
 *
 * Add
 *
 *************************************/

void i386_device::x87_fadd_m32real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fadd_m64real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fadd_st_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fadd_sti_st(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(i, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_faddp(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(8);
}

void i386_device::x87_fiadd_m32int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(19);
}

void i386_device::x87_fiadd_m16int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(20);
}


/*************************************
 *
 * Subtract
 *
 *************************************/

void i386_device::x87_fsub_m32real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsub_m64real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsub_st_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsub_sti_st(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(i, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsubp(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(8);
}

void i386_device::x87_fisub_m32int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(19);
}

void i386_device::x87_fisub_m16int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(20);
}


/*************************************
 *
 * Reverse Subtract
 *
 *************************************/

void i386_device::x87_fsubr_m32real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(ea);

		floatx80 a = float32_to_floatx80(m32real);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsubr_m64real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(ea);

		floatx80 a = float64_to_floatx80(m64real);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsubr_st_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsubr_sti_st(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(i, result, TRUE);

	CYCLES(8);
}

void i386_device::x87_fsubrp(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(8);
}

void i386_device::x87_fisubr_m32int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(ea);

		floatx80 a = int32_to_floatx80(m32int);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(19);
}

void i386_device::x87_fisubr_m16int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(ea);

		floatx80 a = int32_to_floatx80(m16int);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(20);
}


/*************************************
 *
 * Divide
 *
 *************************************/

void i386_device::x87_fdiv_m32real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdiv_m64real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdiv_st_sti(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(0, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdiv_sti_st(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivp(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
		x87_inc_stack();
	}

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fidiv_m32int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fidiv_m16int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}


/*************************************
 *
 * Reverse Divide
 *
 *************************************/

void i386_device::x87_fdivr_m32real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(ea);

		floatx80 a = float32_to_floatx80(m32real);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivr_m64real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(ea);

		floatx80 a = float64_to_floatx80(m64real);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivr_st_sti(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(0, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivr_sti_st(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivrp(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
		x87_inc_stack();
	}

	// 73, 62, 35
	CYCLES(73);
}


void i386_device::x87_fidivr_m32int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(ea);

		floatx80 a = int32_to_floatx80(m32int);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fidivr_m16int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ32(ea);

		floatx80 a = int32_to_floatx80(m16int);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	// 73, 62, 35
	CYCLES(73);
}


/*************************************
 *
 * Multiply
 *
 *************************************/

void i386_device::x87_fmul_m32real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(11);
}

void i386_device::x87_fmul_m64real(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(14);
}

void i386_device::x87_fmul_st_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(16);
}

void i386_device::x87_fmul_sti_st(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(i, result, TRUE);

	CYCLES(16);
}

void i386_device::x87_fmulp(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(16);
}

void i386_device::x87_fimul_m32int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(22);
}

void i386_device::x87_fimul_m16int(UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(22);
}

/*************************************
*
* Conditional Move
*
*************************************/

void i386_device::x87_fcmovb_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (m_CF == 1)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmove_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (m_ZF == 1)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmovbe_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if ((m_CF | m_ZF) == 1)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmovu_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (m_PF == 1)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmovnb_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (m_CF == 0)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmovne_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (m_ZF == 0)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmovnbe_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if ((m_CF == 0) && (m_ZF == 0))
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

void i386_device::x87_fcmovnu_sti(UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (m_PF == 0)
	{
		if (X87_IS_ST_EMPTY(i))
		{
			x87_set_stack_underflow();
			result = fx80_inan;
		}
		else
			result = ST(i);

		if (x87_check_exceptions())
		{
			ST(0) = result;
		}
	}

	CYCLES(4);
}

/*************************************
 *
 * Miscellaneous arithmetic
 *
 *************************************/

void i386_device::x87_fprem(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a0 = ST(0);
		floatx80 b1 = ST(1);

		m_x87_sw &= ~X87_SW_C2;

		//int d=extractFloatx80Exp(a0)-extractFloatx80Exp(b1);
		int d = (a0.high & 0x7FFF) - (b1.high & 0x7FFF);
		if (d < 64) {
			floatx80 t=floatx80_div(a0, b1);
			int64 q = floatx80_to_int64_round_to_zero(t);
			floatx80 qf = int64_to_floatx80(q);
			floatx80 tt = floatx80_mul(b1, qf);
			result = floatx80_sub(a0, tt);
			// C2 already 0
			m_x87_sw &= ~(X87_SW_C0|X87_SW_C3|X87_SW_C1);
			if (q & 1)
				m_x87_sw |= X87_SW_C1;
			if (q & 2)
				m_x87_sw |= X87_SW_C3;
			if (q & 4)
				m_x87_sw |= X87_SW_C0;
		}
		else {
			m_x87_sw |= X87_SW_C2;
			int n = 63;
			int e = 1 << (d - n);
			floatx80 ef = int32_to_floatx80(e);
			floatx80 t=floatx80_div(a0, b1);
			floatx80 td = floatx80_div(t, ef);
			int64 qq = floatx80_to_int64_round_to_zero(td);
			floatx80 qqf = int64_to_floatx80(qq);
			floatx80 tt = floatx80_mul(b1, qqf);
			floatx80 ttt = floatx80_mul(tt, ef);
			result = floatx80_sub(a0, ttt);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(84);
}

void i386_device::x87_fprem1(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(1);

		m_x87_sw &= ~X87_SW_C2;

		// TODO: Implement Cx bits
		result = floatx80_rem(a, b);
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(94);
}

void i386_device::x87_fsqrt(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 value = ST(0);

		if ((!floatx80_is_zero(value) && (value.high & 0x8000)) ||
				floatx80_is_denormal(value))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = floatx80_sqrt(value);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(8);
}

/*************************************
 *
 * Trigonometric
 *
 *************************************/

void i386_device::x87_f2xm1(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		// TODO: Inaccurate
		double x = fx80_to_double(ST(0));
		double res = pow(2.0, x) - 1;
		result = double_to_fx80(res);
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(0, result, TRUE);
	}

	CYCLES(242);
}

void i386_device::x87_fyl2x(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 x = ST(0);
		floatx80 y = ST(1);

		if (x.high & 0x8000)
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			// TODO: Inaccurate
			double d64 = fx80_to_double(x);
			double l2x = log(d64)/log(2.0);
			result = floatx80_mul(double_to_fx80(l2x), y);
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(1, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(250);
}

void i386_device::x87_fyl2xp1(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		floatx80 x = ST(0);
		floatx80 y = ST(1);

		// TODO: Inaccurate
		double d64 = fx80_to_double(x);
		double l2x1 = log(d64 + 1.0)/log(2.0);
		result = floatx80_mul(double_to_fx80(l2x1), y);
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(1, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(313);
}

void i386_device::x87_fptan(UINT8 modrm)
{
	floatx80 result1, result2;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result1 = fx80_inan;
		result2 = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		x87_set_stack_overflow();
		result1 = fx80_inan;
		result2 = fx80_inan;
	}
	else
	{
		result1 = ST(0);
		result2 = fx80_one;

#if 0 // TODO: Function produces bad values
		if (floatx80_ftan(result1) != -1)
			m_x87_sw &= ~X87_SW_C2;
		else
			m_x87_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result1);
		x = tan(x);
		result1 = double_to_fx80(x);

		m_x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(0, result1, TRUE);
		x87_dec_stack();
		x87_write_stack(0, result2, TRUE);
	}

	CYCLES(244);
}

void i386_device::x87_fpatan(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		// TODO: Inaccurate
		double val = atan2(fx80_to_double(ST(1)) , fx80_to_double(ST(0)));
		result = double_to_fx80(val);
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(1, result, TRUE);
		x87_inc_stack();
	}

	CYCLES(289);
}

void i386_device::x87_fsin(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 0 // TODO: Function produces bad values
		if (floatx80_fsin(result) != -1)
			m_x87_sw &= ~X87_SW_C2;
		else
			m_x87_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result);
		x = sin(x);
		result = double_to_fx80(x);

		m_x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(241);
}

void i386_device::x87_fcos(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 0 // TODO: Function produces bad values
		if (floatx80_fcos(result) != -1)
			m_x87_sw &= ~X87_SW_C2;
		else
			m_x87_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result);
		x = cos(x);
		result = double_to_fx80(x);

		m_x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, TRUE);

	CYCLES(241);
}

void i386_device::x87_fsincos(UINT8 modrm)
{
	floatx80 s_result, c_result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		s_result = c_result = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		x87_set_stack_overflow();
		s_result = c_result = fx80_inan;
	}
	else
	{
		extern int sf_fsincos(floatx80 a, floatx80 *sin_a, floatx80 *cos_a);

		s_result = c_result = ST(0);

#if 0 // TODO: Function produces bad values
		if (sf_fsincos(s_result, &s_result, &c_result) != -1)
			m_x87_sw &= ~X87_SW_C2;
		else
			m_x87_sw |= X87_SW_C2;
#else
		double s = fx80_to_double(s_result);
		double c = fx80_to_double(c_result);
		s = sin(s);
		c = cos(c);

		s_result = double_to_fx80(s);
		c_result = double_to_fx80(c);

		m_x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(0, s_result, TRUE);
		x87_dec_stack();
		x87_write_stack(0, c_result, TRUE);
	}

	CYCLES(291);
}


/*************************************
 *
 * Load data
 *
 *************************************/

void i386_device::x87_fld_m32real(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (x87_dec_stack())
	{
		UINT32 m32real = READ32(ea);

		value = float32_to_floatx80(m32real);

		m_x87_sw &= ~X87_SW_C1;

		if (floatx80_is_signaling_nan(value) || floatx80_is_denormal(value))
		{
			m_x87_sw |= X87_SW_IE;
			value = fx80_inan;
		}
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(3);
}

void i386_device::x87_fld_m64real(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (x87_dec_stack())
	{
		UINT64 m64real = READ64(ea);

		value = float64_to_floatx80(m64real);

		m_x87_sw &= ~X87_SW_C1;

		if (floatx80_is_signaling_nan(value) || floatx80_is_denormal(value))
		{
			m_x87_sw |= X87_SW_IE;
			value = fx80_inan;
		}
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(3);
}

void i386_device::x87_fld_m80real(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (x87_dec_stack())
	{
		m_x87_sw &= ~X87_SW_C1;
		value = READ80(ea);
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(6);
}

void i386_device::x87_fld_sti(UINT8 modrm)
{
	floatx80 value;

	if (x87_dec_stack())
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST((modrm + 1) & 7);
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(4);
}

void i386_device::x87_fild_m16int(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (!x87_dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		INT16 m16int = READ16(ea);
		value = int32_to_floatx80(m16int);
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(13);
}

void i386_device::x87_fild_m32int(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (!x87_dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		INT32 m32int = READ32(ea);
		value = int32_to_floatx80(m32int);
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(9);
}

void i386_device::x87_fild_m64int(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (!x87_dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		INT64 m64int = READ64(ea);
		value = int64_to_floatx80(m64int);
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(10);
}

void i386_device::x87_fbld(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 0);
	if (!x87_dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		UINT64 m64val = 0;
		UINT16 sign;

		value = READ80(ea);

		sign = value.high & 0x8000;
		m64val += ((value.high >> 4) & 0xf) * 10;
		m64val += ((value.high >> 0) & 0xf);

		for (int i = 60; i >= 0; i -= 4)
		{
			m64val *= 10;
			m64val += (value.low >> i) & 0xf;
		}

		value = int64_to_floatx80(m64val);
		value.high |= sign;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(75);
}


/*************************************
 *
 * Store data
 *
 *************************************/

void i386_device::x87_fst_m32real(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 1);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions())
	{
		UINT32 m32real = floatx80_to_float32(value);
		WRITE32(ea, m32real);
	}

	CYCLES(7);
}

void i386_device::x87_fst_m64real(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 1);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions())
	{
		UINT64 m64real = floatx80_to_float64(value);
		WRITE64(ea, m64real);
	}

	CYCLES(8);
}

void i386_device::x87_fst_sti(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions())
		x87_write_stack(i, value, TRUE);

	CYCLES(3);
}

void i386_device::x87_fstp_m32real(UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(modrm, 1);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions())
	{
		UINT32 m32real = floatx80_to_float32(value);
		WRITE32(ea, m32real);
		x87_inc_stack();
	}

	CYCLES(7);
}

void i386_device::x87_fstp_m64real(UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}


	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		UINT64 m64real = floatx80_to_float64(value);
		WRITE64(ea, m64real);
		x87_inc_stack();
	}

	CYCLES(8);
}

void i386_device::x87_fstp_m80real(UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE80(ea, value);
		x87_inc_stack();
	}

	CYCLES(6);
}

void i386_device::x87_fstp_sti(UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(i, value, TRUE);
		x87_inc_stack();
	}

	CYCLES(3);
}

void i386_device::x87_fist_m16int(UINT8 modrm)
{
	INT16 m16int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m16int = -32768;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(-32768);
		floatx80 upperLim = int32_to_floatx80(32767);

		m_x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m16int = floatx80_to_int32(fx80);
		else
			m16int = -32768;
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE16(ea, m16int);
	}

	CYCLES(29);
}

void i386_device::x87_fist_m32int(UINT8 modrm)
{
	INT32 m32int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m32int = 0x80000000;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(0x80000000);
		floatx80 upperLim = int32_to_floatx80(0x7fffffff);

		m_x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m32int = floatx80_to_int32(fx80);
		else
			m32int = 0x80000000;
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE32(ea, m32int);
	}

	CYCLES(28);
}

void i386_device::x87_fistp_m16int(UINT8 modrm)
{
	INT16 m16int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m16int = (UINT16)0x8000;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(-32768);
		floatx80 upperLim = int32_to_floatx80(32767);

		m_x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m16int = floatx80_to_int32(fx80);
		else
			m16int = (UINT16)0x8000;
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE16(ea, m16int);
		x87_inc_stack();
	}

	CYCLES(29);
}

void i386_device::x87_fistp_m32int(UINT8 modrm)
{
	INT32 m32int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m32int = 0x80000000;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(0x80000000);
		floatx80 upperLim = int32_to_floatx80(0x7fffffff);

		m_x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m32int = floatx80_to_int32(fx80);
		else
			m32int = 0x80000000;
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE32(ea, m32int);
		x87_inc_stack();
	}

	CYCLES(29);
}

void i386_device::x87_fistp_m64int(UINT8 modrm)
{
	INT64 m64int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m64int = U64(0x8000000000000000);
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int64_to_floatx80(U64(0x8000000000000000));
		floatx80 upperLim = int64_to_floatx80(U64(0x7fffffffffffffff));

		m_x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m64int = floatx80_to_int64(fx80);
		else
			m64int = U64(0x8000000000000000);
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE64(ea, m64int);
		x87_inc_stack();
	}

	CYCLES(29);
}

void i386_device::x87_fbstp(UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		UINT64 u64 = floatx80_to_int64(floatx80_abs(ST(0)));
		result.low = 0;

		for (int i = 0; i < 64; i += 4)
		{
			result.low += (u64 % 10) << i;
			u64 /= 10;
		}

		result.high = (u64 % 10);
		result.high += ((u64 / 10) % 10) << 4;
		result.high |= ST(0).high & 0x8000;
	}

	UINT32 ea = GetEA(modrm, 1);
	if (x87_check_exceptions())
	{
		WRITE80(ea, result);
		x87_inc_stack();
	}

	CYCLES(175);
}


/*************************************
 *
 * Constant load
 *
 *************************************/

void i386_device::x87_fld1(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		m_x87_sw &= ~X87_SW_C1;
		value = fx80_one;
		tag = X87_TW_VALID;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(4);
}

void i386_device::x87_fldl2t(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		tag = X87_TW_VALID;
		value.high = 0x4000;

		if (X87_RC == X87_CW_RC_UP)
			value.low =  U64(0xd49a784bcd1b8aff);
		else
			value.low = U64(0xd49a784bcd1b8afe);

		m_x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(8);
}

void i386_device::x87_fldl2e(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x3fff;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0xb8aa3b295c17f0bc);
		else
			value.low = U64(0xb8aa3b295c17f0bb);

		m_x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(8);
}

void i386_device::x87_fldpi(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x4000;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0xc90fdaa22168c235);
		else
			value.low = U64(0xc90fdaa22168c234);

		m_x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(8);
}

void i386_device::x87_fldlg2(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x3ffd;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0x9a209a84fbcff799);
		else
			value.low = U64(0x9a209a84fbcff798);

		m_x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(8);
}

void i386_device::x87_fldln2(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x3ffe;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0xb17217f7d1cf79ac);
		else
			value.low = U64(0xb17217f7d1cf79ab);

		m_x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(8);
}

void i386_device::x87_fldz(UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack())
	{
		value = fx80_zero;
		tag = X87_TW_ZERO;
		m_x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions())
	{
		x87_set_tag(ST_TO_PHYS(0), tag);
		x87_write_stack(0, value, FALSE);
	}

	CYCLES(4);
}


/*************************************
 *
 * Miscellaneous
 *
 *************************************/

void i386_device::x87_fnop(UINT8 modrm)
{
	CYCLES(3);
}

void i386_device::x87_fchs(UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		value = ST(0);
		value.high ^= 0x8000;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, FALSE);

	CYCLES(6);
}

void i386_device::x87_fabs(UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		value = ST(0);
		value.high &= 0x7fff;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, FALSE);

	CYCLES(6);
}

void i386_device::x87_fscale(UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = ST(0);

		// Set the rounding mode to truncate
		UINT16 old_cw = m_x87_cw;
		UINT16 new_cw = (old_cw & ~(X87_CW_RC_MASK << X87_CW_RC_SHIFT)) | (X87_CW_RC_ZERO << X87_CW_RC_SHIFT);
		x87_write_cw(new_cw);

		// Interpret ST(1) as an integer
		UINT32 st1 = floatx80_to_int32(floatx80_round_to_int(ST(1)));

		// Restore the rounding mode
		x87_write_cw(old_cw);

		// Get the unbiased exponent of ST(0)
		INT16 exp = (ST(0).high & 0x7fff) - 0x3fff;

		// Calculate the new exponent
		exp = (exp + st1 + 0x3fff) & 0x7fff;

		// Write it back
		value.high = (value.high & ~0x7fff) + exp;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, FALSE);

	CYCLES(31);
}

void i386_device::x87_frndint(UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		value = floatx80_round_to_int(ST(0));
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, TRUE);

	CYCLES(21);
}

void i386_device::x87_fxtract(UINT8 modrm)
{
	floatx80 sig80, exp80;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		sig80 = exp80 = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		x87_set_stack_overflow();
		sig80 = exp80 = fx80_inan;
	}
	else
	{
		floatx80 value = ST(0);

		if (floatx80_eq(value, fx80_zero))
		{
			m_x87_sw |= X87_SW_ZE;

			exp80 = fx80_ninf;
			sig80 = fx80_zero;
		}
		else
		{
			// Extract the unbiased exponent
			exp80 = int32_to_floatx80((value.high & 0x7fff) - 0x3fff);

			// For the significand, replicate the original value and set its true exponent to 0.
			sig80 = value;
			sig80.high &= ~0x7fff;
			sig80.high |=  0x3fff;
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(0, exp80, TRUE);
		x87_dec_stack();
		x87_write_stack(0, sig80, TRUE);
	}

	CYCLES(21);
}

/*************************************
 *
 * Comparison
 *
 *************************************/

void i386_device::x87_ftst(UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		if (floatx80_is_nan(ST(0)))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(ST(0), fx80_zero))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(ST(0), fx80_zero))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fxam(UINT8 modrm)
{
	floatx80 value = ST(0);

	m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

	// TODO: Unsupported and denormal values
	if (X87_IS_ST_EMPTY(0))
	{
		m_x87_sw |= X87_SW_C3 | X87_SW_C0;
	}
	else if (floatx80_is_zero(value))
	{
		m_x87_sw |= X87_SW_C3;
	}
	if (floatx80_is_nan(value))
	{
		m_x87_sw |= X87_SW_C0;
	}
	else if (floatx80_is_inf(value))
	{
		m_x87_sw |= X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw |= X87_SW_C2;
	}

	if (value.high & 0x8000)
		m_x87_sw |= X87_SW_C1;

	CYCLES(8);
}

void i386_device::x87_ficom_m16int(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT16 m16int = READ16(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(16);
}

void i386_device::x87_ficom_m32int(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT32 m32int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(15);
}

void i386_device::x87_ficomp_m16int(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT16 m16int = READ16(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(16);
}

void i386_device::x87_ficomp_m32int(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT32 m32int = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(15);
}


void i386_device::x87_fcom_m32real(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT32 m32real = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fcom_m64real(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT64 m64real = READ64(ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fcom_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fcomp_m32real(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT32 m32real = READ32(ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4);
}

void i386_device::x87_fcomp_m64real(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT64 m64real = READ64(ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4);
}

void i386_device::x87_fcomp_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4);
}

void i386_device::x87_fcomi_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_ZF = 1;
		m_PF = 1;
		m_CF = 1;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			m_ZF = 0;
			m_PF = 0;
			m_CF = 0;

			if (floatx80_eq(a, b))
				m_ZF = 1;

			if (floatx80_lt(a, b))
				m_CF = 1;
		}
	}

	x87_check_exceptions();

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fcomip_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_ZF = 1;
		m_PF = 1;
		m_CF = 1;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			m_ZF = 0;
			m_PF = 0;
			m_CF = 0;

			if (floatx80_eq(a, b))
				m_ZF = 1;

			if (floatx80_lt(a, b))
				m_CF = 1;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fucomi_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_ZF = 1;
		m_PF = 1;
		m_CF = 1;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_quiet_nan(a) || floatx80_is_quiet_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
		}
		else if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			m_ZF = 0;
			m_PF = 0;
			m_CF = 0;

			if (floatx80_eq(a, b))
				m_ZF = 1;

			if (floatx80_lt(a, b))
				m_CF = 1;
		}
	}

	x87_check_exceptions();

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fucomip_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_ZF = 1;
		m_PF = 1;
		m_CF = 1;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_quiet_nan(a) || floatx80_is_quiet_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
		}
		else if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			m_ZF = 0;
			m_PF = 0;
			m_CF = 0;

			if (floatx80_eq(a, b))
				m_ZF = 1;

			if (floatx80_lt(a, b))
				m_CF = 1;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fcompp(UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(1);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
	{
		x87_inc_stack();
		x87_inc_stack();
	}

	CYCLES(5);
}


/*************************************
 *
 * Unordererd comparison
 *
 *************************************/

void i386_device::x87_fucom_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
				m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fucomp_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
				m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4);
}

void i386_device::x87_fucompp(UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(1);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
				m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
	{
		x87_inc_stack();
		x87_inc_stack();
	}

	CYCLES(4);
}


/*************************************
 *
 * Control
 *
 *************************************/

void i386_device::x87_fdecstp(UINT8 modrm)
{
	m_x87_sw &= ~X87_SW_C1;

	x87_dec_stack();
	x87_check_exceptions();

	CYCLES(3);
}

void i386_device::x87_fincstp(UINT8 modrm)
{
	m_x87_sw &= ~X87_SW_C1;

	x87_inc_stack();
	x87_check_exceptions();

	CYCLES(3);
}

void i386_device::x87_fclex(UINT8 modrm)
{
	m_x87_sw &= ~0x80ff;

	CYCLES(7);
}

void i386_device::x87_ffree(UINT8 modrm)
{
	x87_set_tag(ST_TO_PHYS(modrm & 7), X87_TW_EMPTY);

	CYCLES(3);
}

void i386_device::x87_finit(UINT8 modrm)
{
	x87_reset();

	CYCLES(17);
}

void i386_device::x87_fldcw(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);
	UINT16 cw = READ16(ea);

	x87_write_cw(cw);

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fstcw(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 1);
	WRITE16(ea, m_x87_cw);

	CYCLES(3);
}

void i386_device::x87_fldenv(UINT8 modrm)
{
	// TODO: Pointers and selectors
	UINT32 ea = GetEA(modrm, 0);

	if (m_operand_size)
	{
		// 32-bit real/protected mode
		x87_write_cw(READ16(ea));
		m_x87_sw = READ16(ea + 4);
		m_x87_tw = READ16(ea + 8);
	}
	else
	{
		// 16-bit real/protected mode
		x87_write_cw(READ16(ea));
		m_x87_sw = READ16(ea + 2);
		m_x87_tw = READ16(ea + 4);
	}

	x87_check_exceptions();

	CYCLES((m_cr[0] & 1) ? 34 : 44);
}

void i386_device::x87_fstenv(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 1);

	// TODO: Pointers and selectors
	switch((m_cr[0] & 1)|(m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
//          WRITE16(ea + 6, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 10, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, (m_fpu_inst_ptr & 0x0f0000) >> 4);
			break;
		case 1: // 16-bit protected mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
//          WRITE16(ea + 6, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 10, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, (m_fpu_inst_ptr & 0x0f0000) >> 4);
			break;
		case 2: // 32-bit real mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 4, m_x87_sw);
			WRITE16(ea + 8, m_x87_tw);
//          WRITE16(ea + 12, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 20, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE32(ea + 24, (m_fpu_data_ptr >> 16) << 12);
			break;
		case 3: // 32-bit protected mode
			WRITE16(ea + 0,  m_x87_cw);
			WRITE16(ea + 4,  m_x87_sw);
			WRITE16(ea + 8,  m_x87_tw);
//          WRITE32(ea + 12, m_fpu_inst_ptr);
//          WRITE32(ea + 16, m_fpu_opcode);
//          WRITE32(ea + 20, m_fpu_data_ptr);
//          WRITE32(ea + 24, m_fpu_inst_ptr);
			break;
	}

	CYCLES((m_cr[0] & 1) ? 56 : 67);
}

void i386_device::x87_fsave(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 1);

	// TODO: Pointers and selectors
	switch((m_cr[0] & 1)|(m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
//          WRITE16(ea + 6, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 10, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, (m_fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
//          WRITE16(ea + 6, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 10, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, (m_fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 4, m_x87_sw);
			WRITE16(ea + 8, m_x87_tw);
//          WRITE16(ea + 12, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 20, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE32(ea + 24, (m_fpu_data_ptr >> 16) << 12);
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			WRITE16(ea + 0,  m_x87_cw);
			WRITE16(ea + 4,  m_x87_sw);
			WRITE16(ea + 8,  m_x87_tw);
//          WRITE32(ea + 12, m_fpu_inst_ptr);
//          WRITE32(ea + 16, m_fpu_opcode);
//          WRITE32(ea + 20, m_fpu_data_ptr);
//          WRITE32(ea + 24, m_fpu_inst_ptr);
			ea += 28;
			break;
	}

	for (int i = 0; i < 8; ++i)
		WRITE80(ea + i*10, ST(i));

	CYCLES((m_cr[0] & 1) ? 56 : 67);
}

void i386_device::x87_frstor(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 0);

	// TODO: Pointers and selectors
	switch((m_cr[0] & 1)|(m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 2);
			m_x87_tw = READ16(ea + 4);
//          WRITE16(ea + 6, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 10, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, (m_fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 2);
			m_x87_tw = READ16(ea + 4);
//          WRITE16(ea + 6, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 10, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, (m_fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 4);
			m_x87_tw = READ16(ea + 8);
//          WRITE16(ea + 12, m_fpu_inst_ptr & 0xffff);
//          WRITE16(ea + 8, (m_fpu_opcode & 0x07ff) | ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(ea + 20, m_fpu_data_ptr & 0xffff);
//          WRITE16(ea + 12, ((m_fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE32(ea + 24, (m_fpu_data_ptr >> 16) << 12);
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 4);
			m_x87_tw = READ16(ea + 8);
//          WRITE32(ea + 12, m_fpu_inst_ptr);
//          WRITE32(ea + 16, m_fpu_opcode);
//          WRITE32(ea + 20, m_fpu_data_ptr);
//          WRITE32(ea + 24, m_fpu_inst_ptr);
			ea += 28;
			break;
	}

	for (int i = 0; i < 8; ++i)
		x87_write_stack(i, READ80(ea + i*10), FALSE);

	CYCLES((m_cr[0] & 1) ? 34 : 44);
}

void i386_device::x87_fxch(UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
		x87_set_stack_underflow();

	if (x87_check_exceptions())
	{
		floatx80 tmp = ST(0);
		ST(0) = ST(1);
		ST(1) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		x87_set_tag(ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(1)));
		x87_set_tag(ST_TO_PHYS(1), tag0);
	}

	CYCLES(4);
}

void i386_device::x87_fxch_sti(UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0))
	{
		ST(0) = fx80_inan;
		x87_set_tag(ST_TO_PHYS(0), X87_TW_SPECIAL);
		x87_set_stack_underflow();
	}
	if (X87_IS_ST_EMPTY(i))
	{
		ST(i) = fx80_inan;
		x87_set_tag(ST_TO_PHYS(i), X87_TW_SPECIAL);
		x87_set_stack_underflow();
	}

	if (x87_check_exceptions())
	{
		floatx80 tmp = ST(0);
		ST(0) = ST(i);
		ST(i) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		x87_set_tag(ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(i)));
		x87_set_tag(ST_TO_PHYS(i), tag0);
	}

	CYCLES(4);
}

void i386_device::x87_fstsw_ax(UINT8 modrm)
{
	REG16(AX) = m_x87_sw;

	CYCLES(3);
}

void i386_device::x87_fstsw_m2byte(UINT8 modrm)
{
	UINT32 ea = GetEA(modrm, 1);

	WRITE16(ea, m_x87_sw);

	CYCLES(3);
}

void i386_device::x87_invalid(UINT8 modrm)
{
	// TODO
	fatalerror("x87 invalid instruction (PC:%.4x)\n", m_pc);
}



/*************************************
 *
 * Instruction dispatch
 *
 *************************************/

void i386_device::i386_x87_group_d8()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_d8[modrm])(modrm);
}

void i386_device::i386_x87_group_d9()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_d9[modrm])(modrm);
}

void i386_device::i386_x87_group_da()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_da[modrm])(modrm);
}

void i386_device::i386_x87_group_db()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_db[modrm])(modrm);
}

void i386_device::i386_x87_group_dc()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_dc[modrm])(modrm);
}

void i386_device::i386_x87_group_dd()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_dd[modrm])(modrm);
}

void i386_device::i386_x87_group_de()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_de[modrm])(modrm);
}

void i386_device::i386_x87_group_df()
{
	UINT8 modrm = FETCH();
	(this->*m_opcode_table_x87_df[modrm])(modrm);
}


/*************************************
 *
 * Opcode table building
 *
 *************************************/

void i386_device::build_x87_opcode_table_d8()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fadd_m32real;  break;
				case 0x01: ptr = &i386_device::x87_fmul_m32real;  break;
				case 0x02: ptr = &i386_device::x87_fcom_m32real;  break;
				case 0x03: ptr = &i386_device::x87_fcomp_m32real; break;
				case 0x04: ptr = &i386_device::x87_fsub_m32real;  break;
				case 0x05: ptr = &i386_device::x87_fsubr_m32real; break;
				case 0x06: ptr = &i386_device::x87_fdiv_m32real;  break;
				case 0x07: ptr = &i386_device::x87_fdivr_m32real; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i386_device::x87_fadd_st_sti;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i386_device::x87_fmul_st_sti;  break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = &i386_device::x87_fcom_sti;     break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i386_device::x87_fcomp_sti;    break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i386_device::x87_fsub_st_sti;  break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i386_device::x87_fsubr_st_sti; break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i386_device::x87_fdiv_st_sti;  break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = &i386_device::x87_fdivr_st_sti; break;
			}
		}

		m_opcode_table_x87_d8[modrm] = ptr;
	}
}


void i386_device::build_x87_opcode_table_d9()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fld_m32real;   break;
				case 0x02: ptr = &i386_device::x87_fst_m32real;   break;
				case 0x03: ptr = &i386_device::x87_fstp_m32real;  break;
				case 0x04: ptr = &i386_device::x87_fldenv;        break;
				case 0x05: ptr = &i386_device::x87_fldcw;         break;
				case 0x06: ptr = &i386_device::x87_fstenv;        break;
				case 0x07: ptr = &i386_device::x87_fstcw;         break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0:
				case 0xc1:
				case 0xc2:
				case 0xc3:
				case 0xc4:
				case 0xc5:
				case 0xc6:
				case 0xc7: ptr = &i386_device::x87_fld_sti;   break;

				case 0xc8:
				case 0xc9:
				case 0xca:
				case 0xcb:
				case 0xcc:
				case 0xcd:
				case 0xce:
				case 0xcf: ptr = &i386_device::x87_fxch_sti;  break;

				case 0xd0: ptr = &i386_device::x87_fnop;      break;
				case 0xe0: ptr = &i386_device::x87_fchs;      break;
				case 0xe1: ptr = &i386_device::x87_fabs;      break;
				case 0xe4: ptr = &i386_device::x87_ftst;      break;
				case 0xe5: ptr = &i386_device::x87_fxam;      break;
				case 0xe8: ptr = &i386_device::x87_fld1;      break;
				case 0xe9: ptr = &i386_device::x87_fldl2t;    break;
				case 0xea: ptr = &i386_device::x87_fldl2e;    break;
				case 0xeb: ptr = &i386_device::x87_fldpi;     break;
				case 0xec: ptr = &i386_device::x87_fldlg2;    break;
				case 0xed: ptr = &i386_device::x87_fldln2;    break;
				case 0xee: ptr = &i386_device::x87_fldz;      break;
				case 0xf0: ptr = &i386_device::x87_f2xm1;     break;
				case 0xf1: ptr = &i386_device::x87_fyl2x;     break;
				case 0xf2: ptr = &i386_device::x87_fptan;     break;
				case 0xf3: ptr = &i386_device::x87_fpatan;    break;
				case 0xf4: ptr = &i386_device::x87_fxtract;   break;
				case 0xf5: ptr = &i386_device::x87_fprem1;    break;
				case 0xf6: ptr = &i386_device::x87_fdecstp;   break;
				case 0xf7: ptr = &i386_device::x87_fincstp;   break;
				case 0xf8: ptr = &i386_device::x87_fprem;     break;
				case 0xf9: ptr = &i386_device::x87_fyl2xp1;   break;
				case 0xfa: ptr = &i386_device::x87_fsqrt;     break;
				case 0xfb: ptr = &i386_device::x87_fsincos;   break;
				case 0xfc: ptr = &i386_device::x87_frndint;   break;
				case 0xfd: ptr = &i386_device::x87_fscale;    break;
				case 0xfe: ptr = &i386_device::x87_fsin;      break;
				case 0xff: ptr = &i386_device::x87_fcos;      break;
			}
		}

		m_opcode_table_x87_d9[modrm] = ptr;
	}
}

void i386_device::build_x87_opcode_table_da()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fiadd_m32int;  break;
				case 0x01: ptr = &i386_device::x87_fimul_m32int;  break;
				case 0x02: ptr = &i386_device::x87_ficom_m32int;  break;
				case 0x03: ptr = &i386_device::x87_ficomp_m32int; break;
				case 0x04: ptr = &i386_device::x87_fisub_m32int;  break;
				case 0x05: ptr = &i386_device::x87_fisubr_m32int; break;
				case 0x06: ptr = &i386_device::x87_fidiv_m32int;  break;
				case 0x07: ptr = &i386_device::x87_fidivr_m32int; break;
			}
		}
		else
		{
			switch (modrm)
			{
			case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i386_device::x87_fcmovb_sti;  break;
			case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i386_device::x87_fcmove_sti;  break;
			case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = &i386_device::x87_fcmovbe_sti; break;
			case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i386_device::x87_fcmovu_sti;  break;
			case 0xe9: ptr = &i386_device::x87_fucompp;       break;
			}
		}

		m_opcode_table_x87_da[modrm] = ptr;
	}
}


void i386_device::build_x87_opcode_table_db()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fild_m32int;   break;
				case 0x02: ptr = &i386_device::x87_fist_m32int;   break;
				case 0x03: ptr = &i386_device::x87_fistp_m32int;  break;
				case 0x05: ptr = &i386_device::x87_fld_m80real;   break;
				case 0x07: ptr = &i386_device::x87_fstp_m80real;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i386_device::x87_fcmovnb_sti;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i386_device::x87_fcmovne_sti;  break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = &i386_device::x87_fcmovnbe_sti; break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i386_device::x87_fcmovnu_sti;  break;
				case 0xe0: ptr = &i386_device::x87_fnop;          break; /* FENI */
				case 0xe1: ptr = &i386_device::x87_fnop;          break; /* FDISI */
				case 0xe2: ptr = &i386_device::x87_fclex;         break;
				case 0xe3: ptr = &i386_device::x87_finit;         break;
				case 0xe4: ptr = &i386_device::x87_fnop;          break; /* FSETPM */
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i386_device::x87_fucomi_sti;  break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i386_device::x87_fcomi_sti; break;
			}
		}

		m_opcode_table_x87_db[modrm] = ptr;
	}
}


void i386_device::build_x87_opcode_table_dc()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fadd_m64real;  break;
				case 0x01: ptr = &i386_device::x87_fmul_m64real;  break;
				case 0x02: ptr = &i386_device::x87_fcom_m64real;  break;
				case 0x03: ptr = &i386_device::x87_fcomp_m64real; break;
				case 0x04: ptr = &i386_device::x87_fsub_m64real;  break;
				case 0x05: ptr = &i386_device::x87_fsubr_m64real; break;
				case 0x06: ptr = &i386_device::x87_fdiv_m64real;  break;
				case 0x07: ptr = &i386_device::x87_fdivr_m64real; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i386_device::x87_fadd_sti_st;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i386_device::x87_fmul_sti_st;  break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i386_device::x87_fsubr_sti_st; break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i386_device::x87_fsub_sti_st;  break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i386_device::x87_fdivr_sti_st; break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = &i386_device::x87_fdiv_sti_st;  break;
			}
		}

		m_opcode_table_x87_dc[modrm] = ptr;
	}
}


void i386_device::build_x87_opcode_table_dd()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fld_m64real;   break;
				case 0x02: ptr = &i386_device::x87_fst_m64real;   break;
				case 0x03: ptr = &i386_device::x87_fstp_m64real;  break;
				case 0x04: ptr = &i386_device::x87_frstor;        break;
				case 0x06: ptr = &i386_device::x87_fsave;         break;
				case 0x07: ptr = &i386_device::x87_fstsw_m2byte;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i386_device::x87_ffree;        break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i386_device::x87_fxch_sti;     break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = &i386_device::x87_fst_sti;      break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i386_device::x87_fstp_sti;     break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i386_device::x87_fucom_sti;    break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i386_device::x87_fucomp_sti;   break;
			}
		}

		m_opcode_table_x87_dd[modrm] = ptr;
	}
}


void i386_device::build_x87_opcode_table_de()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fiadd_m16int;  break;
				case 0x01: ptr = &i386_device::x87_fimul_m16int;  break;
				case 0x02: ptr = &i386_device::x87_ficom_m16int;  break;
				case 0x03: ptr = &i386_device::x87_ficomp_m16int; break;
				case 0x04: ptr = &i386_device::x87_fisub_m16int;  break;
				case 0x05: ptr = &i386_device::x87_fisubr_m16int; break;
				case 0x06: ptr = &i386_device::x87_fidiv_m16int;  break;
				case 0x07: ptr = &i386_device::x87_fidivr_m16int; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i386_device::x87_faddp;    break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i386_device::x87_fmulp;    break;
				case 0xd9: ptr = &i386_device::x87_fcompp; break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i386_device::x87_fsubrp;   break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i386_device::x87_fsubp;    break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i386_device::x87_fdivrp;   break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = &i386_device::x87_fdivp;    break;
			}
		}

		m_opcode_table_x87_de[modrm] = ptr;
	}
}


void i386_device::build_x87_opcode_table_df()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		i386_modrm_func ptr = &i386_device::x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i386_device::x87_fild_m16int;   break;
				case 0x02: ptr = &i386_device::x87_fist_m16int;   break;
				case 0x03: ptr = &i386_device::x87_fistp_m16int;  break;
				case 0x04: ptr = &i386_device::x87_fbld;          break;
				case 0x05: ptr = &i386_device::x87_fild_m64int;   break;
				case 0x06: ptr = &i386_device::x87_fbstp;         break;
				case 0x07: ptr = &i386_device::x87_fistp_m64int;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xe0: ptr = &i386_device::x87_fstsw_ax;      break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i386_device::x87_fucomip_sti;    break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i386_device::x87_fcomip_sti;    break;
			}
		}

		m_opcode_table_x87_df[modrm] = ptr;
	}
}

void i386_device::build_x87_opcode_table()
{
	build_x87_opcode_table_d8();
	build_x87_opcode_table_d9();
	build_x87_opcode_table_da();
	build_x87_opcode_table_db();
	build_x87_opcode_table_dc();
	build_x87_opcode_table_dd();
	build_x87_opcode_table_de();
	build_x87_opcode_table_df();
}
