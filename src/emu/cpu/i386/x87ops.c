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

#define ST_TO_PHYS(x)           (((cpustate->x87_sw >> X87_SW_TOP_SHIFT) + (x)) & X87_SW_TOP_MASK)
#define ST(x)                   (cpustate->x87_reg[ST_TO_PHYS(x)])
#define X87_TW_FIELD_SHIFT(x)   ((x) << 1)
#define X87_TAG(x)              ((cpustate->x87_tw >> X87_TW_FIELD_SHIFT(x)) & X87_TW_MASK)
#define X87_RC                  ((cpustate->x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK)
#define X87_IS_ST_EMPTY(x)      (X87_TAG(ST_TO_PHYS(x)) == X87_TW_EMPTY)
#define X87_SW_C3_0             X87_SW_C0

#define UNIMPLEMENTED           fatalerror("Unimplemented x87 op: %s (PC:%x)\n", __FUNCTION__, cpustate->pc)


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

INLINE floatx80 READ80(i386_state *cpustate, UINT32 ea)
{
	floatx80 t;

	t.low = READ64(cpustate, ea);
	t.high = READ16(cpustate, ea + 8);

	return t;
}

INLINE void WRITE80(i386_state *cpustate, UINT32 ea, floatx80 t)
{
	WRITE64(cpustate, ea, t.low);
	WRITE16(cpustate, ea + 8, t.high);
}


/*************************************
 *
 * x87 stack handling
 *
 *************************************/

INLINE void x87_set_stack_top(i386_state *cpustate, int top)
{
	cpustate->x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT);
	cpustate->x87_sw |= (top << X87_SW_TOP_SHIFT);
}

INLINE void x87_set_tag(i386_state *cpustate, int reg, int tag)
{
	int shift = X87_TW_FIELD_SHIFT(reg);

	cpustate->x87_tw &= ~(X87_TW_MASK << shift);
	cpustate->x87_tw |= (tag << shift);
}

void x87_write_stack(i386_state *cpustate, int i, floatx80 value, int update_tag)
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

		x87_set_tag(cpustate, ST_TO_PHYS(i), tag);
	}
}

INLINE void x87_set_stack_underflow(i386_state *cpustate)
{
	cpustate->x87_sw |= X87_SW_C1 | X87_SW_IE | X87_SW_SF;
}

INLINE void x87_set_stack_overflow(i386_state *cpustate)
{
	cpustate->x87_sw &= ~X87_SW_C1;
	cpustate->x87_sw |= X87_SW_IE | X87_SW_SF;
}

int x87_inc_stack(i386_state *cpustate)
{
	int ret = 1;

	// Check for stack underflow
	if (X87_IS_ST_EMPTY(0))
	{
		ret = 0;
		x87_set_stack_underflow(cpustate);

		// Don't update the stack if the exception is unmasked
		if (~cpustate->x87_cw & X87_CW_IM)
			return ret;
	}

	x87_set_tag(cpustate, ST_TO_PHYS(0), X87_TW_EMPTY);
	x87_set_stack_top(cpustate, ST_TO_PHYS(1));
	return ret;
}

int x87_dec_stack(i386_state *cpustate)
{
	int ret = 1;

	// Check for stack overflow
	if (!X87_IS_ST_EMPTY(7))
	{
		ret = 0;
		x87_set_stack_overflow(cpustate);

		// Don't update the stack if the exception is unmasked
		if (~cpustate->x87_cw & X87_CW_IM)
			return ret;
	}

	x87_set_stack_top(cpustate, ST_TO_PHYS(7));
	return ret;
}


/*************************************
 *
 * Exception handling
 *
 *************************************/

int x87_check_exceptions(i386_state *cpustate)
{
	/* Update the exceptions from SoftFloat */
	if (float_exception_flags & float_flag_invalid)
	{
		cpustate->x87_sw |= X87_SW_IE;
		float_exception_flags &= ~float_flag_invalid;
	}
	if (float_exception_flags & float_flag_overflow)
	{
		cpustate->x87_sw |= X87_SW_OE;
		float_exception_flags &= ~float_flag_overflow;
	}
	if (float_exception_flags & float_flag_underflow)
	{
		cpustate->x87_sw |= X87_SW_UE;
		float_exception_flags &= ~float_flag_underflow;
	}
	if (float_exception_flags & float_flag_inexact)
	{
		cpustate->x87_sw |= X87_SW_PE;
		float_exception_flags &= ~float_flag_inexact;
	}

	if ((cpustate->x87_sw & ~cpustate->x87_cw) & 0x3f)
	{
		// cpustate->device->execute().set_input_line(INPUT_LINE_FERR, RAISE_LINE);
		logerror("Unmasked x87 exception (CW:%.4x, SW:%.4x)\n", cpustate->x87_cw, cpustate->x87_sw);
		if (cpustate->cr[0] & 0x20) // FIXME: 486 and up only
		{
			cpustate->ext = 1;
			i386_trap(cpustate, FAULT_MF, 0, 0);
		}
		return 0;
	}

	return 1;
}

INLINE void x87_write_cw(i386_state *cpustate, UINT16 cw)
{
	cpustate->x87_cw = cw;

	/* Update the SoftFloat rounding mode */
	float_rounding_mode = x87_to_sf_rc[(cpustate->x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK];
}

void x87_reset(i386_state *cpustate)
{
	x87_write_cw(cpustate, 0x0037f);

	cpustate->x87_sw = 0;
	cpustate->x87_tw = 0xffff;

	// TODO: FEA=0, FDS=0, FIP=0 FOP=0 FCS=0
	cpustate->x87_data_ptr = 0;
	cpustate->x87_inst_ptr = 0;
	cpustate->x87_opcode = 0;
}


/*************************************
 *
 * Core arithmetic
 *
 *************************************/

static floatx80 x87_add(i386_state *cpustate, floatx80 a, floatx80 b)
{
	floatx80 result = { 0 };

	switch ((cpustate->x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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

static floatx80 x87_sub(i386_state *cpustate, floatx80 a, floatx80 b)
{
	floatx80 result = { 0 };

	switch ((cpustate->x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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

static floatx80 x87_mul(i386_state *cpustate, floatx80 a, floatx80 b)
{
	floatx80 val = { 0 };

	switch ((cpustate->x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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


static floatx80 x87_div(i386_state *cpustate, floatx80 a, floatx80 b)
{
	floatx80 val = { 0 };

	switch ((cpustate->x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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

void x87_fadd_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fadd_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fadd_st_sti(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fadd_sti_st(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, i, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_faddp(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 8);
}

void x87_fiadd_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 19);
}

void x87_fiadd_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 20);
}


/*************************************
 *
 * Subtract
 *
 *************************************/

void x87_fsub_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsub_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsub_st_sti(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsub_sti_st(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, i, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsubp(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 8);
}

void x87_fisub_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 19);
}

void x87_fisub_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 20);
}


/*************************************
 *
 * Reverse Subtract
 *
 *************************************/

void x87_fsubr_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = float32_to_floatx80(m32real);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsubr_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = float64_to_floatx80(m64real);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsubr_st_sti(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsubr_sti_st(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, i, result, TRUE);

	CYCLES(cpustate, 8);
}

void x87_fsubrp(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 8);
}

void x87_fisubr_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = int32_to_floatx80(m32int);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 19);
}

void x87_fisubr_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(cpustate, ea);

		floatx80 a = int32_to_floatx80(m16int);
		floatx80 b = ST(0);

		if ((floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.high ^ b.high) & 0x8000)))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 20);
}


/*************************************
 *
 * Divide
 *
 *************************************/

void x87_fdiv_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdiv_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdiv_st_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 0, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdiv_sti_st(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdivp(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
		x87_inc_stack(cpustate);
	}

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fidiv_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fidiv_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}


/*************************************
 *
 * Reverse Divide
 *
 *************************************/

void x87_fdivr_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = float32_to_floatx80(m32real);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdivr_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = float64_to_floatx80(m64real);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdivr_st_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(i);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 0, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdivr_sti_st(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
	}

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fdivrp(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
		x87_inc_stack(cpustate);
	}

	// 73, 62, 35
	CYCLES(cpustate, 73);
}


void x87_fidivr_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = int32_to_floatx80(m32int);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}

void x87_fidivr_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ32(cpustate, ea);

		floatx80 a = int32_to_floatx80(m16int);
		floatx80 b = ST(0);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_div(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	// 73, 62, 35
	CYCLES(cpustate, 73);
}


/*************************************
 *
 * Multiply
 *
 *************************************/

void x87_fmul_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 11);
}

void x87_fmul_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 14);
}

void x87_fmul_st_sti(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 16);
}

void x87_fmul_sti_st(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, i, result, TRUE);

	CYCLES(cpustate, 16);
}

void x87_fmulp(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 16);
}

void x87_fimul_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 22);
}

void x87_fimul_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		INT16 m16int = READ16(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_mul(cpustate, a, b);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 22);
}


/*************************************
 *
 * Miscellaneous arithmetic
 *
 *************************************/

void x87_fprem(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(1);

		cpustate->x87_sw &= ~X87_SW_C0;

		// TODO: Implement Cx bits
		result = floatx80_rem(a, b);
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 84);
}

void x87_fprem1(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 a = ST(0);
		floatx80 b = ST(1);

		cpustate->x87_sw &= ~X87_SW_C0;

		// TODO: Implement Cx bits
		result = floatx80_rem(a, b);
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 94);
}

void x87_fsqrt(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 value = ST(0);

		if ((!floatx80_is_zero(value) && (value.high & 0x8000)) ||
				floatx80_is_denormal(value))
		{
			cpustate->x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = floatx80_sqrt(value);
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 8);
}

/*************************************
 *
 * Trigonometric
 *
 *************************************/

void x87_f2xm1(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		// TODO: Inaccurate
		double x = fx80_to_double(ST(0));
		double res = pow(2.0, x) - 1;
		result = double_to_fx80(res);
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 0, result, TRUE);
	}

	CYCLES(cpustate, 242);
}

void x87_fyl2x(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		floatx80 x = ST(0);
		floatx80 y = ST(1);

		if (x.high & 0x8000)
		{
			cpustate->x87_sw |= X87_SW_IE;
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

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 1, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 250);
}

void x87_fyl2xp1(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
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

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 1, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 313);
}

void x87_fptan(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result1, result2;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result1 = fx80_inan;
		result2 = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		x87_set_stack_overflow(cpustate);
		result1 = fx80_inan;
		result2 = fx80_inan;
	}
	else
	{
		result1 = ST(0);
		result2 = fx80_one;

#if 0 // TODO: Function produces bad values
		if (floatx80_ftan(result1) != -1)
			cpustate->x87_sw &= ~X87_SW_C2;
		else
			cpustate->x87_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result1);
		x = tan(x);
		result1 = double_to_fx80(x);

		cpustate->x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 0, result1, TRUE);
		x87_dec_stack(cpustate);
		x87_write_stack(cpustate, 0, result2, TRUE);
	}

	CYCLES(cpustate, 244);
}

void x87_fpatan(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		// TODO: Inaccurate
		double val = atan(fx80_to_double(ST(1)) / fx80_to_double(ST(0)));
		result = double_to_fx80(val);
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 1, result, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 289);
}

void x87_fsin(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 0 // TODO: Function produces bad values
		if (floatx80_fsin(result) != -1)
			cpustate->x87_sw &= ~X87_SW_C2;
		else
			cpustate->x87_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result);
		x = sin(x);
		result = double_to_fx80(x);

		cpustate->x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 241);
}

void x87_fcos(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 0 // TODO: Function produces bad values
		if (floatx80_fcos(result) != -1)
			cpustate->x87_sw &= ~X87_SW_C2;
		else
			cpustate->x87_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result);
		x = cos(x);
		result = double_to_fx80(x);

		cpustate->x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, result, TRUE);

	CYCLES(cpustate, 241);
}

void x87_fsincos(i386_state *cpustate, UINT8 modrm)
{
	floatx80 s_result, c_result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		s_result = c_result = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		x87_set_stack_overflow(cpustate);
		s_result = c_result = fx80_inan;
	}
	else
	{
		extern int sf_fsincos(floatx80 a, floatx80 *sin_a, floatx80 *cos_a);

		s_result = c_result = ST(0);

#if 0 // TODO: Function produces bad values
		if (sf_fsincos(s_result, &s_result, &c_result) != -1)
			cpustate->x87_sw &= ~X87_SW_C2;
		else
			cpustate->x87_sw |= X87_SW_C2;
#else
		double s = fx80_to_double(s_result);
		double c = fx80_to_double(c_result);
		s = sin(s);
		c = cos(c);

		s_result = double_to_fx80(s);
		c_result = double_to_fx80(c);

		cpustate->x87_sw &= ~X87_SW_C2;
#endif
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 0, s_result, TRUE);
		x87_dec_stack(cpustate);
		x87_write_stack(cpustate, 0, c_result, TRUE);
	}

	CYCLES(cpustate, 291);
}


/*************************************
 *
 * Load data
 *
 *************************************/

void x87_fld_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (x87_dec_stack(cpustate))
	{
		UINT32 m32real = READ32(cpustate, ea);

		value = float32_to_floatx80(m32real);

		cpustate->x87_sw &= ~X87_SW_C1;

		if (floatx80_is_signaling_nan(value) || floatx80_is_denormal(value))
		{
			cpustate->x87_sw |= X87_SW_IE;
			value = fx80_inan;
		}
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 3);
}

void x87_fld_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (x87_dec_stack(cpustate))
	{
		UINT64 m64real = READ64(cpustate, ea);

		value = float64_to_floatx80(m64real);

		cpustate->x87_sw &= ~X87_SW_C1;

		if (floatx80_is_signaling_nan(value) || floatx80_is_denormal(value))
		{
			cpustate->x87_sw |= X87_SW_IE;
			value = fx80_inan;
		}
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 3);
}

void x87_fld_m80real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (x87_dec_stack(cpustate))
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = READ80(cpustate, ea);
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 6);
}

void x87_fld_sti(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (x87_dec_stack(cpustate))
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST((modrm + 1) & 7);
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 4);
}

void x87_fild_m16int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (!x87_dec_stack(cpustate))
	{
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		INT16 m16int = READ16(cpustate, ea);
		value = int32_to_floatx80(m16int);
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 13);
}

void x87_fild_m32int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (!x87_dec_stack(cpustate))
	{
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		INT32 m32int = READ32(cpustate, ea);
		value = int32_to_floatx80(m32int);
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 9);
}

void x87_fild_m64int(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (!x87_dec_stack(cpustate))
	{
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		INT64 m64int = READ64(cpustate, ea);
		value = int64_to_floatx80(m64int);
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 10);
}

void x87_fbld(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (!x87_dec_stack(cpustate))
	{
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		UINT64 m64val = 0;
		UINT16 sign;

		value = READ80(cpustate, ea);

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

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 75);
}


/*************************************
 *
 * Store data
 *
 *************************************/

void x87_fst_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions(cpustate))
	{
		UINT32 m32real = floatx80_to_float32(value);
		WRITE32(cpustate, ea, m32real);
	}

	CYCLES(cpustate, 7);
}

void x87_fst_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions(cpustate))
	{
		UINT64 m64real = floatx80_to_float64(value);
		WRITE64(cpustate, ea, m64real);
	}

	CYCLES(cpustate, 8);
}

void x87_fst_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, i, value, TRUE);

	CYCLES(cpustate, 3);
}

void x87_fstp_m32real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions(cpustate))
	{
		UINT32 m32real = floatx80_to_float32(value);
		WRITE32(cpustate, ea, m32real);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 7);
}

void x87_fstp_m64real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}


	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		UINT64 m64real = floatx80_to_float64(value);
		WRITE64(cpustate, ea, m64real);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 8);
}

void x87_fstp_m80real(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE80(cpustate, ea, value);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 6);
}

void x87_fstp_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, i, value, TRUE);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 3);
}

void x87_fist_m16int(i386_state *cpustate, UINT8 modrm)
{
	INT16 m16int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		m16int = -32768;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(-32768);
		floatx80 upperLim = int32_to_floatx80(32767);

		cpustate->x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m16int = floatx80_to_int32(fx80);
		else
			m16int = -32768;
	}

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE16(cpustate, ea, m16int);
	}

	CYCLES(cpustate, 29);
}

void x87_fist_m32int(i386_state *cpustate, UINT8 modrm)
{
	INT32 m32int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		m32int = 0x80000000;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(0x80000000);
		floatx80 upperLim = int32_to_floatx80(0x7fffffff);

		cpustate->x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m32int = floatx80_to_int32(fx80);
		else
			m32int = 0x80000000;
	}

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE32(cpustate, ea, m32int);
	}

	CYCLES(cpustate, 28);
}

void x87_fistp_m16int(i386_state *cpustate, UINT8 modrm)
{
	INT16 m16int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		m16int = (UINT16)0x8000;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(-32768);
		floatx80 upperLim = int32_to_floatx80(32767);

		cpustate->x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m16int = floatx80_to_int32(fx80);
		else
			m16int = (UINT16)0x8000;
	}

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE16(cpustate, ea, m16int);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 29);
}

void x87_fistp_m32int(i386_state *cpustate, UINT8 modrm)
{
	INT32 m32int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		m32int = 0x80000000;
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int32_to_floatx80(0x80000000);
		floatx80 upperLim = int32_to_floatx80(0x7fffffff);

		cpustate->x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m32int = floatx80_to_int32(fx80);
		else
			m32int = 0x80000000;
	}

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE32(cpustate, ea, m32int);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 29);
}

void x87_fistp_m64int(i386_state *cpustate, UINT8 modrm)
{
	INT64 m64int;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		m64int = U64(0x8000000000000000);
	}
	else
	{
		floatx80 fx80 = floatx80_round_to_int(ST(0));

		floatx80 lowerLim = int64_to_floatx80(U64(0x8000000000000000));
		floatx80 upperLim = int64_to_floatx80(U64(0x7fffffffffffffff));

		cpustate->x87_sw &= ~X87_SW_C1;

		if (!floatx80_lt(fx80, lowerLim) && floatx80_le(fx80, upperLim))
			m64int = floatx80_to_int64(fx80);
		else
			m64int = U64(0x8000000000000000);
	}

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE64(cpustate, ea, m64int);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 29);
}

void x87_fbstp(i386_state *cpustate, UINT8 modrm)
{
	floatx80 result;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
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

	UINT32 ea = GetEA(cpustate, modrm, 1);
	if (x87_check_exceptions(cpustate))
	{
		WRITE80(cpustate, ea, result);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 175);
}


/*************************************
 *
 * Constant load
 *
 *************************************/

void x87_fld1(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = fx80_one;
		tag = X87_TW_VALID;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 4);
}

void x87_fldl2t(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		tag = X87_TW_VALID;
		value.high = 0x4000;

		if (X87_RC == X87_CW_RC_UP)
			value.low =  U64(0xd49a784bcd1b8aff);
		else
			value.low = U64(0xd49a784bcd1b8afe);

		cpustate->x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 8);
}

void x87_fldl2e(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x3fff;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0xb8aa3b295c17f0bc);
		else
			value.low = U64(0xb8aa3b295c17f0bb);

		cpustate->x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 8);
}

void x87_fldpi(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x4000;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0xc90fdaa22168c235);
		else
			value.low = U64(0xc90fdaa22168c234);

		cpustate->x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 8);
}

void x87_fldlg2(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x3ffd;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0x9a209a84fbcff799);
		else
			value.low = U64(0x9a209a84fbcff798);

		cpustate->x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 8);
}

void x87_fldln2(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.high = 0x3ffe;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.low = U64(0xb17217f7d1cf79ac);
		else
			value.low = U64(0xb17217f7d1cf79ab);

		cpustate->x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 8);
}

void x87_fldz(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;
	int tag;

	if (x87_dec_stack(cpustate))
	{
		value = fx80_zero;
		tag = X87_TW_ZERO;
		cpustate->x87_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_set_tag(cpustate, ST_TO_PHYS(0), tag);
		x87_write_stack(cpustate, 0, value, FALSE);
	}

	CYCLES(cpustate, 4);
}


/*************************************
 *
 * Miscellaneous
 *
 *************************************/

void x87_fnop(i386_state *cpustate, UINT8 modrm)
{
	CYCLES(cpustate, 3);
}

void x87_fchs(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		value = ST(0);
		value.high ^= 0x8000;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, FALSE);

	CYCLES(cpustate, 6);
}

void x87_fabs(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		value = ST(0);
		value.high &= 0x7fff;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, FALSE);

	CYCLES(cpustate, 6);
}

void x87_fscale(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;
		value = ST(0);

		// Set the rounding mode to truncate
		UINT16 old_cw = cpustate->x87_cw;
		UINT16 new_cw = (old_cw & ~(X87_CW_RC_MASK << X87_CW_RC_SHIFT)) | (X87_CW_RC_ZERO << X87_CW_RC_SHIFT);
		x87_write_cw(cpustate, new_cw);

		// Interpret ST(1) as an integer
		UINT32 st1 = floatx80_to_int32(floatx80_round_to_int(ST(1)));

		// Restore the rounding mode
		x87_write_cw(cpustate, old_cw);

		// Get the unbiased exponent of ST(0)
		INT16 exp = (ST(0).high & 0x7fff) - 0x3fff;

		// Calculate the new exponent
		exp = (exp + st1 + 0x3fff) & 0x7fff;

		// Write it back
		value.high = (value.high & ~0x7fff) + exp;
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, FALSE);

	CYCLES(cpustate, 31);
}

void x87_frndint(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		value = fx80_inan;
	}
	else
	{
		cpustate->x87_sw &= ~X87_SW_C1;

		value = floatx80_round_to_int(ST(0));
	}

	if (x87_check_exceptions(cpustate))
		x87_write_stack(cpustate, 0, value, TRUE);

	CYCLES(cpustate, 21);
}

void x87_fxtract(i386_state *cpustate, UINT8 modrm)
{
	floatx80 sig80, exp80;

	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		sig80 = exp80 = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		x87_set_stack_overflow(cpustate);
		sig80 = exp80 = fx80_inan;
	}
	else
	{
		floatx80 value = ST(0);

		if (floatx80_eq(value, fx80_zero))
		{
			cpustate->x87_sw |= X87_SW_ZE;

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

	if (x87_check_exceptions(cpustate))
	{
		x87_write_stack(cpustate, 0, exp80, TRUE);
		x87_dec_stack(cpustate);
		x87_write_stack(cpustate, 0, sig80, TRUE);
	}

	CYCLES(cpustate, 21);
}

/*************************************
 *
 * Comparison
 *
 *************************************/

void x87_ftst(i386_state *cpustate, UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		if (floatx80_is_nan(ST(0)))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(ST(0), fx80_zero))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(ST(0), fx80_zero))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fxam(i386_state *cpustate, UINT8 modrm)
{
	floatx80 value = ST(0);

	cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

	// TODO: Unsupported and denormal values
	if (X87_IS_ST_EMPTY(0))
	{
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C0;
	}
	else if (floatx80_is_zero(value))
	{
		cpustate->x87_sw |= X87_SW_C3;
	}
	if (floatx80_is_nan(value))
	{
		cpustate->x87_sw |= X87_SW_C0;
	}
	else if (floatx80_is_inf(value))
	{
		cpustate->x87_sw |= X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw |= X87_SW_C2;
	}

	if (value.high & 0x8000)
		cpustate->x87_sw |= X87_SW_C1;

	CYCLES(cpustate, 8);
}

void x87_ficom_m16int(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT16 m16int = READ16(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_nan(a))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 16);
}

void x87_ficom_m32int(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_nan(a))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 15);
}

void x87_ficomp_m16int(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT16 m16int = READ16(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m16int);

		if (floatx80_is_nan(a))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_inc_stack(cpustate);

	CYCLES(cpustate, 16);
}

void x87_ficomp_m32int(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		INT32 m32int = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = int32_to_floatx80(m32int);

		if (floatx80_is_nan(a))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_inc_stack(cpustate);

	CYCLES(cpustate, 15);
}


void x87_fcom_m32real(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fcom_m64real(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fcom_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fcomp_m32real(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT32 m32real = READ32(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float32_to_floatx80(m32real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_inc_stack(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fcomp_m64real(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		UINT64 m64real = READ64(cpustate, ea);

		floatx80 a = ST(0);
		floatx80 b = float64_to_floatx80(m64real);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_inc_stack(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fcomp_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_inc_stack(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fcompp(i386_state *cpustate, UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(1);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_inc_stack(cpustate);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 5);
}


/*************************************
 *
 * Unordererd comparison
 *
 *************************************/

void x87_fucom_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
				cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fucomp_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(i);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
				cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
		x87_inc_stack(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fucompp(i386_state *cpustate, UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow(cpustate);
		cpustate->x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		cpustate->x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		floatx80 a = ST(0);
		floatx80 b = ST(1);

		if (floatx80_is_nan(a) || floatx80_is_nan(b))
		{
			cpustate->x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (floatx80_is_signaling_nan(a) || floatx80_is_signaling_nan(b))
				cpustate->x87_sw |= X87_SW_IE;
		}
		else
		{
			if (floatx80_eq(a, b))
				cpustate->x87_sw |= X87_SW_C3;

			if (floatx80_lt(a, b))
				cpustate->x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions(cpustate))
	{
		x87_inc_stack(cpustate);
		x87_inc_stack(cpustate);
	}

	CYCLES(cpustate, 4);
}


/*************************************
 *
 * Control
 *
 *************************************/

void x87_fdecstp(i386_state *cpustate, UINT8 modrm)
{
	cpustate->x87_sw &= ~X87_SW_C1;

	x87_dec_stack(cpustate);
	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 3);
}

void x87_fincstp(i386_state *cpustate, UINT8 modrm)
{
	cpustate->x87_sw &= ~X87_SW_C1;

	x87_inc_stack(cpustate);
	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 3);
}

void x87_fclex(i386_state *cpustate, UINT8 modrm)
{
	cpustate->x87_sw &= ~0x80ff;

	CYCLES(cpustate, 7);
}

void x87_ffree(i386_state *cpustate, UINT8 modrm)
{
	x87_set_tag(cpustate, ST_TO_PHYS(modrm & 7), X87_TW_EMPTY);

	CYCLES(cpustate, 3);
}

void x87_finit(i386_state *cpustate, UINT8 modrm)
{
	x87_reset(cpustate);

	CYCLES(cpustate, 17);
}

void x87_fldcw(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);
	UINT16 cw = READ16(cpustate, ea);

	x87_write_cw(cpustate, cw);

	x87_check_exceptions(cpustate);

	CYCLES(cpustate, 4);
}

void x87_fstcw(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 1);
	WRITE16(cpustate, ea, cpustate->x87_cw);

	CYCLES(cpustate, 3);
}

void x87_fldenv(i386_state *cpustate, UINT8 modrm)
{
	// TODO: Pointers and selectors
	UINT32 ea = GetEA(cpustate, modrm, 0);

	if (cpustate->operand_size)
	{
		// 32-bit real/protected mode
		x87_write_cw(cpustate, READ16(cpustate, ea));
		cpustate->x87_sw = READ16(cpustate, ea + 4);
		cpustate->x87_tw = READ16(cpustate, ea + 8);
	}
	else
	{
		// 16-bit real/protected mode
		x87_write_cw(cpustate, READ16(cpustate, ea));
		cpustate->x87_sw = READ16(cpustate, ea + 2);
		cpustate->x87_tw = READ16(cpustate, ea + 4);
	}

	x87_check_exceptions(cpustate);

	CYCLES(cpustate,(cpustate->cr[0] & 1) ? 34 : 44);
}

void x87_fstenv(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 1);

	// TODO: Pointers and selectors
	switch((cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			WRITE16(cpustate, ea + 0, cpustate->x87_cw);
			WRITE16(cpustate, ea + 2, cpustate->x87_sw);
			WRITE16(cpustate, ea + 4, cpustate->x87_tw);
//          WRITE16(cpustate, ea + 6, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate, ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate, ea + 10, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate, ea + 12, (cpustate->fpu_inst_ptr & 0x0f0000) >> 4);
			break;
		case 1: // 16-bit protected mode
			WRITE16(cpustate,ea + 0, cpustate->x87_cw);
			WRITE16(cpustate,ea + 2, cpustate->x87_sw);
			WRITE16(cpustate,ea + 4, cpustate->x87_tw);
//          WRITE16(cpustate,ea + 6, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate,ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate,ea + 10, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate,ea + 12, (cpustate->fpu_inst_ptr & 0x0f0000) >> 4);
			break;
		case 2: // 32-bit real mode
			WRITE16(cpustate, ea + 0, cpustate->x87_cw);
			WRITE16(cpustate, ea + 4, cpustate->x87_sw);
			WRITE16(cpustate, ea + 8, cpustate->x87_tw);
//          WRITE16(cpustate, ea + 12, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate, ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate, ea + 20, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate, ea + 12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE32(cpustate, ea + 24, (cpustate->fpu_data_ptr >> 16) << 12);
			break;
		case 3: // 32-bit protected mode
			WRITE16(cpustate, ea + 0,  cpustate->x87_cw);
			WRITE16(cpustate, ea + 4,  cpustate->x87_sw);
			WRITE16(cpustate, ea + 8,  cpustate->x87_tw);
//          WRITE32(cpustate, ea + 12, cpustate->fpu_inst_ptr);
//          WRITE32(cpustate, ea + 16, cpustate->fpu_opcode);
//          WRITE32(cpustate, ea + 20, cpustate->fpu_data_ptr);
//          WRITE32(cpustate, ea + 24, cpustate->fpu_inst_ptr);
			break;
	}

	CYCLES(cpustate,(cpustate->cr[0] & 1) ? 56 : 67);
}

void x87_fsave(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 1);

	// TODO: Pointers and selectors
	switch((cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			WRITE16(cpustate, ea + 0, cpustate->x87_cw);
			WRITE16(cpustate, ea + 2, cpustate->x87_sw);
			WRITE16(cpustate, ea + 4, cpustate->x87_tw);
//          WRITE16(cpustate, ea + 6, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate, ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate, ea + 10, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate, ea + 12, (cpustate->fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			WRITE16(cpustate,ea + 0, cpustate->x87_cw);
			WRITE16(cpustate,ea + 2, cpustate->x87_sw);
			WRITE16(cpustate,ea + 4, cpustate->x87_tw);
//          WRITE16(cpustate,ea + 6, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate,ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate,ea + 10, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate,ea + 12, (cpustate->fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			WRITE16(cpustate, ea + 0, cpustate->x87_cw);
			WRITE16(cpustate, ea + 4, cpustate->x87_sw);
			WRITE16(cpustate, ea + 8, cpustate->x87_tw);
//          WRITE16(cpustate, ea + 12, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate, ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate, ea + 20, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate, ea + 12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE32(cpustate, ea + 24, (cpustate->fpu_data_ptr >> 16) << 12);
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			WRITE16(cpustate, ea + 0,  cpustate->x87_cw);
			WRITE16(cpustate, ea + 4,  cpustate->x87_sw);
			WRITE16(cpustate, ea + 8,  cpustate->x87_tw);
//          WRITE32(cpustate, ea + 12, cpustate->fpu_inst_ptr);
//          WRITE32(cpustate, ea + 16, cpustate->fpu_opcode);
//          WRITE32(cpustate, ea + 20, cpustate->fpu_data_ptr);
//          WRITE32(cpustate, ea + 24, cpustate->fpu_inst_ptr);
			ea += 28;
			break;
	}

	for (int i = 0; i < 8; ++i)
		x87_write_stack(cpustate, i, READ80(cpustate, ea + i*10), FALSE);

	CYCLES(cpustate,(cpustate->cr[0] & 1) ? 56 : 67);
}

void x87_frstor(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 0);

	// TODO: Pointers and selectors
	switch((cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			x87_write_cw(cpustate, READ16(cpustate, ea));
			cpustate->x87_sw = READ16(cpustate, ea + 2);
			cpustate->x87_tw = READ16(cpustate, ea + 4);
//          WRITE16(cpustate, ea + 6, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate, ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate, ea + 10, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate, ea + 12, (cpustate->fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			x87_write_cw(cpustate, READ16(cpustate, ea));
			cpustate->x87_sw = READ16(cpustate, ea + 2);
			cpustate->x87_tw = READ16(cpustate, ea + 4);
//          WRITE16(cpustate,ea + 6, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate,ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate,ea + 10, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate,ea + 12, (cpustate->fpu_inst_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			x87_write_cw(cpustate, READ16(cpustate, ea));
			cpustate->x87_sw = READ16(cpustate, ea + 4);
			cpustate->x87_tw = READ16(cpustate, ea + 8);
//          WRITE16(cpustate, ea + 12, cpustate->fpu_inst_ptr & 0xffff);
//          WRITE16(cpustate, ea + 8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE16(cpustate, ea + 20, cpustate->fpu_data_ptr & 0xffff);
//          WRITE16(cpustate, ea + 12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
//          WRITE32(cpustate, ea + 24, (cpustate->fpu_data_ptr >> 16) << 12);
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			x87_write_cw(cpustate, READ16(cpustate, ea));
			cpustate->x87_sw = READ16(cpustate, ea + 4);
			cpustate->x87_tw = READ16(cpustate, ea + 8);
//          WRITE32(cpustate, ea + 12, cpustate->fpu_inst_ptr);
//          WRITE32(cpustate, ea + 16, cpustate->fpu_opcode);
//          WRITE32(cpustate, ea + 20, cpustate->fpu_data_ptr);
//          WRITE32(cpustate, ea + 24, cpustate->fpu_inst_ptr);
			ea += 28;
			break;
	}

	for (int i = 0; i < 8; ++i)
		WRITE80(cpustate, ea + i*10, ST(i));

	CYCLES(cpustate,(cpustate->cr[0] & 1) ? 34 : 44);
}

void x87_fxch(i386_state *cpustate, UINT8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
		x87_set_stack_underflow(cpustate);

	if (x87_check_exceptions(cpustate))
	{
		floatx80 tmp = ST(0);
		ST(0) = ST(1);
		ST(1) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		x87_set_tag(cpustate, ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(1)));
		x87_set_tag(cpustate, ST_TO_PHYS(1), tag0);
	}

	CYCLES(cpustate, 4);
}

void x87_fxch_sti(i386_state *cpustate, UINT8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0))
	{
		ST(0) = fx80_inan;
		x87_set_tag(cpustate, ST_TO_PHYS(0), X87_TW_SPECIAL);
		x87_set_stack_underflow(cpustate);
	}
	if (X87_IS_ST_EMPTY(i))
	{
		ST(i) = fx80_inan;
		x87_set_tag(cpustate, ST_TO_PHYS(i), X87_TW_SPECIAL);
		x87_set_stack_underflow(cpustate);
	}

	if (x87_check_exceptions(cpustate))
	{
		floatx80 tmp = ST(0);
		ST(0) = ST(i);
		ST(i) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		x87_set_tag(cpustate, ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(i)));
		x87_set_tag(cpustate, ST_TO_PHYS(i), tag0);
	}

	CYCLES(cpustate, 4);
}

void x87_fstsw_ax(i386_state *cpustate, UINT8 modrm)
{
	REG16(AX) = cpustate->x87_sw;

	CYCLES(cpustate, 3);
}

void x87_fstsw_m2byte(i386_state *cpustate, UINT8 modrm)
{
	UINT32 ea = GetEA(cpustate, modrm, 1);

	WRITE16(cpustate, ea, cpustate->x87_sw);

	CYCLES(cpustate, 3);
}

void x87_invalid(i386_state *cpustate, UINT8 modrm)
{
	// TODO
	fatalerror("x87 invalid instruction (PC:%.4x)\n", cpustate->pc);
}



/*************************************
 *
 * Instruction dispatch
 *
 *************************************/

static void I386OP(x87_group_d8)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_d8[modrm](cpustate, modrm);
}

static void I386OP(x87_group_d9)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_d9[modrm](cpustate, modrm);
}

static void I386OP(x87_group_da)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_da[modrm](cpustate, modrm);
}

static void I386OP(x87_group_db)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_db[modrm](cpustate, modrm);
}

static void I386OP(x87_group_dc)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_dc[modrm](cpustate, modrm);
}

static void I386OP(x87_group_dd)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_dd[modrm](cpustate, modrm);
}

static void I386OP(x87_group_de)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_de[modrm](cpustate, modrm);
}

static void I386OP(x87_group_df)(i386_state *cpustate)
{
	UINT8 modrm = FETCH(cpustate);
	cpustate->opcode_table_x87_df[modrm](cpustate, modrm);
}


/*************************************
 *
 * Opcode table building
 *
 *************************************/

void build_x87_opcode_table_d8(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fadd_m32real;  break;
				case 0x01: ptr = x87_fmul_m32real;  break;
				case 0x02: ptr = x87_fcom_m32real;  break;
				case 0x03: ptr = x87_fcomp_m32real; break;
				case 0x04: ptr = x87_fsub_m32real;  break;
				case 0x05: ptr = x87_fsubr_m32real; break;
				case 0x06: ptr = x87_fdiv_m32real;  break;
				case 0x07: ptr = x87_fdivr_m32real; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = x87_fadd_st_sti;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = x87_fmul_st_sti;  break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = x87_fcom_sti;     break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = x87_fcomp_sti;    break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = x87_fsub_st_sti;  break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = x87_fsubr_st_sti; break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = x87_fdiv_st_sti;  break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = x87_fdivr_st_sti; break;
			}
		}

		cpustate->opcode_table_x87_d8[modrm] = ptr;
	}
}


void build_x87_opcode_table_d9(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fld_m32real;   break;
				case 0x02: ptr = x87_fst_m32real;   break;
				case 0x03: ptr = x87_fstp_m32real;  break;
				case 0x04: ptr = x87_fldenv;        break;
				case 0x05: ptr = x87_fldcw;         break;
				case 0x06: ptr = x87_fstenv;        break;
				case 0x07: ptr = x87_fstcw;         break;
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
				case 0xc7: ptr = x87_fld_sti;   break;

				case 0xc8:
				case 0xc9:
				case 0xca:
				case 0xcb:
				case 0xcc:
				case 0xcd:
				case 0xce:
				case 0xcf: ptr = x87_fxch_sti;  break;

				case 0xd0: ptr = x87_fnop;      break;
				case 0xe0: ptr = x87_fchs;      break;
				case 0xe1: ptr = x87_fabs;      break;
				case 0xe4: ptr = x87_ftst;      break;
				case 0xe5: ptr = x87_fxam;      break;
				case 0xe8: ptr = x87_fld1;      break;
				case 0xe9: ptr = x87_fldl2t;    break;
				case 0xea: ptr = x87_fldl2e;    break;
				case 0xeb: ptr = x87_fldpi;     break;
				case 0xec: ptr = x87_fldlg2;    break;
				case 0xed: ptr = x87_fldln2;    break;
				case 0xee: ptr = x87_fldz;      break;
				case 0xf0: ptr = x87_f2xm1;     break;
				case 0xf1: ptr = x87_fyl2x;     break;
				case 0xf2: ptr = x87_fptan;     break;
				case 0xf3: ptr = x87_fpatan;    break;
				case 0xf4: ptr = x87_fxtract;   break;
				case 0xf5: ptr = x87_fprem1;    break;
				case 0xf6: ptr = x87_fdecstp;   break;
				case 0xf7: ptr = x87_fincstp;   break;
				case 0xf8: ptr = x87_fprem;     break;
				case 0xf9: ptr = x87_fyl2xp1;   break;
				case 0xfa: ptr = x87_fsqrt;     break;
				case 0xfb: ptr = x87_fsincos;   break;
				case 0xfc: ptr = x87_frndint;   break;
				case 0xfd: ptr = x87_fscale;    break;
				case 0xfe: ptr = x87_fsin;      break;
				case 0xff: ptr = x87_fcos;      break;
			}
		}

		cpustate->opcode_table_x87_d9[modrm] = ptr;
	}
}

void build_x87_opcode_table_da(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fiadd_m32int;  break;
				case 0x01: ptr = x87_fimul_m32int;  break;
				case 0x02: ptr = x87_ficom_m32int;  break;
				case 0x03: ptr = x87_ficomp_m32int; break;
				case 0x04: ptr = x87_fisub_m32int;  break;
				case 0x05: ptr = x87_fisubr_m32int; break;
				case 0x06: ptr = x87_fidiv_m32int;  break;
				case 0x07: ptr = x87_fidivr_m32int; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xe9: ptr = x87_fucompp;       break;
			}
		}

		cpustate->opcode_table_x87_da[modrm] = ptr;
	}
}


void build_x87_opcode_table_db(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fild_m32int;   break;
				case 0x02: ptr = x87_fist_m32int;   break;
				case 0x03: ptr = x87_fistp_m32int;  break;
				case 0x05: ptr = x87_fld_m80real;   break;
				case 0x07: ptr = x87_fstp_m80real;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xe0: ptr = x87_fnop;          break; /* FENI */
				case 0xe1: ptr = x87_fnop;          break; /* FDISI */
				case 0xe2: ptr = x87_fclex;         break;
				case 0xe3: ptr = x87_finit;         break;
				case 0xe4: ptr = x87_fnop;          break; /* FSETPM */
			}
		}

		cpustate->opcode_table_x87_db[modrm] = ptr;
	}
}


void build_x87_opcode_table_dc(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fadd_m64real;  break;
				case 0x01: ptr = x87_fmul_m64real;  break;
				case 0x02: ptr = x87_fcom_m64real;  break;
				case 0x03: ptr = x87_fcomp_m64real; break;
				case 0x04: ptr = x87_fsub_m64real;  break;
				case 0x05: ptr = x87_fsubr_m64real; break;
				case 0x06: ptr = x87_fdiv_m64real;  break;
				case 0x07: ptr = x87_fdivr_m64real; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = x87_fadd_sti_st;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = x87_fmul_sti_st;  break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = x87_fsubr_sti_st; break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = x87_fsub_sti_st;  break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = x87_fdivr_sti_st; break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = x87_fdiv_sti_st;  break;
			}
		}

		cpustate->opcode_table_x87_dc[modrm] = ptr;
	}
}


void build_x87_opcode_table_dd(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fld_m64real;   break;
				case 0x02: ptr = x87_fst_m64real;   break;
				case 0x03: ptr = x87_fstp_m64real;  break;
				case 0x04: ptr = x87_frstor;        break;
				case 0x06: ptr = x87_fsave;         break;
				case 0x07: ptr = x87_fstsw_m2byte;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = x87_ffree;        break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = x87_fxch_sti;     break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = x87_fst_sti;      break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = x87_fstp_sti;     break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = x87_fucom_sti;    break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = x87_fucomp_sti;   break;
			}
		}

		cpustate->opcode_table_x87_dd[modrm] = ptr;
	}
}


void build_x87_opcode_table_de(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fiadd_m16int;  break;
				case 0x01: ptr = x87_fimul_m16int;  break;
				case 0x02: ptr = x87_ficom_m16int;  break;
				case 0x03: ptr = x87_ficomp_m16int; break;
				case 0x04: ptr = x87_fisub_m16int;  break;
				case 0x05: ptr = x87_fisubr_m16int; break;
				case 0x06: ptr = x87_fidiv_m16int;  break;
				case 0x07: ptr = x87_fidivr_m16int; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = x87_faddp;    break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = x87_fmulp;    break;
				case 0xd9: ptr = x87_fcompp; break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = x87_fsubrp;   break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = x87_fsubp;    break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = x87_fdivrp;   break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = x87_fdivp;    break;
			}
		}

		cpustate->opcode_table_x87_de[modrm] = ptr;
	}
}


void build_x87_opcode_table_df(i386_state *cpustate)
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		void (*ptr)(i386_state *cpustate, UINT8 modrm) = x87_invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = x87_fild_m16int;   break;
				case 0x02: ptr = x87_fist_m16int;   break;
				case 0x03: ptr = x87_fistp_m16int;  break;
				case 0x04: ptr = x87_fbld;          break;
				case 0x05: ptr = x87_fild_m64int;   break;
				case 0x06: ptr = x87_fbstp;         break;
				case 0x07: ptr = x87_fistp_m64int;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xe0: ptr = x87_fstsw_ax;      break;
			}
		}

		cpustate->opcode_table_x87_df[modrm] = ptr;
	}
}

void build_x87_opcode_table(i386_state *cpustate)
{
	build_x87_opcode_table_d8(cpustate);
	build_x87_opcode_table_d9(cpustate);
	build_x87_opcode_table_da(cpustate);
	build_x87_opcode_table_db(cpustate);
	build_x87_opcode_table_dc(cpustate);
	build_x87_opcode_table_dd(cpustate);
	build_x87_opcode_table_de(cpustate);
	build_x87_opcode_table_df(cpustate);
}
