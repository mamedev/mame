// license:BSD-3-Clause
// copyright-holders:Philip Bennett, Carl
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
     - Maybe merge with 80486 fpu (mmx is a problem here)

***************************************************************************/

#include "emu.h"
#include "i8087.h"
#include "softfloat3/bochs_ext/softfloat-extra.h"
#include "softfloat3/bochs_ext/softfloat3_ext.h"
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
#define X87_CW_IEM              0x0080
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

#define ST_TO_PHYS(x)           (((m_sw >> X87_SW_TOP_SHIFT) + (x)) & X87_SW_TOP_MASK)
#define ST(x)                   (m_reg[ST_TO_PHYS(x)])
#define X87_TW_FIELD_SHIFT(x)   ((x) << 1)
#define X87_TAG(x)              ((m_tw >> X87_TW_FIELD_SHIFT(x)) & X87_TW_MASK)
#define X87_RC                  ((m_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK)
#define X87_IS_ST_EMPTY(x)      (X87_TAG(ST_TO_PHYS(x)) == X87_TW_EMPTY)
#define X87_SW_C3_0             X87_SW_C0


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
static const int to_sf_rc[4] =
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

DEFINE_DEVICE_TYPE(I8087, i8087_device, "i8087", "Intel 8087")

i8087_device::i8087_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_int_handler(*this),
	m_busy_handler(*this)
{
}

i8087_device::i8087_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		i8087_device(mconfig, I8087, tag, owner, clock)
{
}

void i8087_device::device_start()
{
	save_item(STRUCT_MEMBER(m_reg, signExp));
	save_item(STRUCT_MEMBER(m_reg, signif));

	save_item(NAME(m_ea));
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_opcode));
	save_item(NAME(m_cw));
	save_item(NAME(m_sw));
	save_item(NAME(m_tw));

	m_timer = timer_alloc(FUNC(i8087_device::release_busy), this);
	build_opcode_table();
}

TIMER_CALLBACK_MEMBER(i8087_device::release_busy)
{
	m_busy_handler(1);
}

void i8087_device::execute()
{
	u8 opcode = FETCH();
	u8 modrm = FETCH();
	m_opcode = (opcode << 8) | modrm;
	switch(opcode)
	{
		case 0xd8:
			group_d8(modrm);
			break;
		case 0xd9:
			group_d9(modrm);
			break;
		case 0xda:
			group_da(modrm);
			break;
		case 0xdb:
			group_db(modrm);
			break;
		case 0xdc:
			group_dc(modrm);
			break;
		case 0xdd:
			group_dd(modrm);
			break;
		case 0xde:
			group_de(modrm);
			break;
		case 0xdf:
			group_df(modrm);
			break;
		default:
			return;
	}
	m_busy_handler(0);
//  if(!((m_sw & ~m_cw) & 0x3f)) // no exceptions
		m_timer->adjust(attotime::from_hz((m_icount ? m_icount : 1) * clock()));
}

void i8087_device::insn_w(uint32_t data)
{
	m_ppc = m_pc;
	m_pc = data;
}

void i8087_device::addr_w(uint32_t data)
{
	m_ea = data;
	execute();
}

u8 i8087_device::READ8(offs_t addr)
{
	return space().read_byte(addr);
}

u16 i8087_device::READ16(offs_t addr)
{
	return space().read_word_unaligned(addr);
}

u32 i8087_device::READ32(offs_t addr)
{
	return space().read_dword_unaligned(addr);
}

u64 i8087_device::READ64(offs_t addr)
{
	return space().read_qword_unaligned(addr);
}

void i8087_device::WRITE8(offs_t addr, u8 data)
{
	space().write_byte(addr, data);
}

void i8087_device::WRITE16(offs_t addr, u16 data)
{
	space().write_word_unaligned(addr, data);
}

void i8087_device::WRITE32(offs_t addr, u32 data)
{
	space().write_dword_unaligned(addr, data);
}

void i8087_device::WRITE64(offs_t addr, u64 data)
{
	space().write_qword_unaligned(addr, data);
}

extFloat80_t i8087_device::READ80(offs_t ea)
{
	extFloat80_t t;

	t.signif = READ64(ea);
	t.signExp = READ16(ea + 8);

	return t;
}

void i8087_device::WRITE80(offs_t ea, extFloat80_t t)
{
	WRITE64(ea, t.signif);
	WRITE16(ea + 8, t.signExp);
}

u8 i8087_device::FETCH()
{
	return READ8(m_pc++);
}

/*************************************
 *
 * x87 stack handling
 *
 *************************************/

void i8087_device::set_stack_top(int top)
{
	m_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT);
	m_sw |= (top << X87_SW_TOP_SHIFT);
}

void i8087_device::set_tag(int reg, int tag)
{
	int shift = X87_TW_FIELD_SHIFT(reg);

	m_tw &= ~(X87_TW_MASK << shift);
	m_tw |= (tag << shift);
}

void i8087_device::write_stack(int i, extFloat80_t value, bool update_tag)
{
	ST(i) = value;

	if (update_tag)
	{
		int tag;

		if (floatx80_is_zero(value))
		{
			tag = X87_TW_ZERO;
		}
		else if (floatx80_is_inf(value) || extFloat80_is_nan(value))
		{
			tag = X87_TW_SPECIAL;
		}
		else
		{
			tag = X87_TW_VALID;
		}

		set_tag(ST_TO_PHYS(i), tag);
	}
}

void i8087_device::set_stack_underflow()
{
	m_sw &= ~X87_SW_C1;
	m_sw |= X87_SW_IE | X87_SW_SF;
}

void i8087_device::set_stack_overflow()
{
	m_sw |= X87_SW_C1 | X87_SW_IE | X87_SW_SF;
}

int i8087_device::inc_stack()
{
	int ret = 1;

	// Check for stack underflow
	if (X87_IS_ST_EMPTY(0))
	{
		ret = 0;
		set_stack_underflow();

		// Don't update the stack if the exception is unmasked
		if (~m_cw & X87_CW_IM)
			return ret;
	}

	set_tag(ST_TO_PHYS(0), X87_TW_EMPTY);
	set_stack_top(ST_TO_PHYS(1));
	return ret;
}

int i8087_device::dec_stack()
{
	int ret = 1;

	// Check for stack overflow
	if (!X87_IS_ST_EMPTY(7))
	{
		ret = 0;
		set_stack_overflow();

		// Don't update the stack if the exception is unmasked
		if (~m_cw & X87_CW_IM)
			return ret;
	}

	set_stack_top(ST_TO_PHYS(7));
	return ret;
}


/*************************************
 *
 * Exception handling
 *
 *************************************/

int i8087_device::check_exceptions(bool store)
{
	/* Update the exceptions from SoftFloat */
	if (softfloat_exceptionFlags & softfloat_flag_invalid)
	{
		m_sw |= X87_SW_IE;
		softfloat_exceptionFlags &= ~softfloat_flag_invalid;
	}
	if (softfloat_exceptionFlags & softfloat_flag_overflow)
	{
		m_sw |= X87_SW_OE;
		softfloat_exceptionFlags &= ~softfloat_flag_overflow;
	}
	if (softfloat_exceptionFlags & softfloat_flag_underflow)
	{
		m_sw |= X87_SW_UE;
		softfloat_exceptionFlags &= ~softfloat_flag_underflow;
	}
	if (softfloat_exceptionFlags & softfloat_flag_inexact)
	{
		m_sw |= X87_SW_PE;
		softfloat_exceptionFlags &= ~softfloat_flag_inexact;
	}
	if (softfloat_exceptionFlags & softfloat_flag_infinite)
	{
		m_sw |= X87_SW_ZE;
		softfloat_exceptionFlags &= ~softfloat_flag_infinite;
	}

	u16 unmasked = (m_sw & ~m_cw) & 0x3f;

	if ((m_sw & ~m_cw) & 0x3f)
	{
		// interrupt handler
		if (!(m_cw & X87_CW_IEM)) { m_sw |= X87_SW_ES; m_int_handler(1); }
		logerror("Unmasked x87 exception (CW:%.4x, SW:%.4x)\n", m_cw, m_sw);
		if (store || !(unmasked & (X87_SW_OE | X87_SW_UE)))
			return 0;
	}

	return 1;
}

void i8087_device::write_cw(u16 cw)
{
	m_cw = cw;

	/* Update the SoftFloat rounding mode */
	softfloat_roundingMode = to_sf_rc[(m_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK];
}

void i8087_device::device_reset()
{
	write_cw(0x0037f);

	m_sw = 0;
	m_tw = 0xffff;

	// TODO: FEA=0, FDS=0, FIP=0 FOP=0 FCS=0
	m_opcode = 0;
	m_ea = 0;
	m_pc = 0;
	m_ppc = 0;
	m_int_handler(0);
	m_busy_handler(0);
}


/*************************************
 *
 * Core arithmetic
 *
 *************************************/

extFloat80_t i8087_device::add(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t result = { 0 };

	switch ((m_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32_t a32 = extF80_to_f32(a);
			float32_t b32 = extF80_to_f32(b);
			result = f32_to_extF80(f32_add(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64_t a64 = extF80_to_f64(a);
			float64_t b64 = extF80_to_f64(b);
			result = f64_to_extF80(f64_add(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			result = extF80_add(a, b);
			break;
		}
	}

	return result;
}

extFloat80_t i8087_device::sub(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t result = { 0 };

	switch ((m_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32_t a32 = extF80_to_f32(a);
			float32_t b32 = extF80_to_f32(b);
			result = f32_to_extF80(f32_sub(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64_t a64 = extF80_to_f64(a);
			float64_t b64 = extF80_to_f64(b);
			result = f64_to_extF80(f64_sub(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			result = extF80_sub(a, b);
			break;
		}
	}

	return result;
}

extFloat80_t i8087_device::mul(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t val = { 0 };

	switch ((m_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32_t a32 = extF80_to_f32(a);
			float32_t b32 = extF80_to_f32(b);
			val = f32_to_extF80(f32_mul(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64_t a64 = extF80_to_f64(a);
			float64_t b64 = extF80_to_f64(b);
			val = f64_to_extF80(f64_mul(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			val = extF80_mul(a, b);
			break;
		}
	}

	return val;
}


extFloat80_t i8087_device::div(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t val = { 0 };

	switch ((m_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
	{
		case X87_CW_PC_SINGLE:
		{
			float32_t a32 = extF80_to_f32(a);
			float32_t b32 = extF80_to_f32(b);
			val = f32_to_extF80(f32_div(a32, b32));
			break;
		}
		case X87_CW_PC_DOUBLE:
		{
			float64_t a64 = extF80_to_f64(a);
			float64_t b64 = extF80_to_f64(b);
			val = f64_to_extF80(f64_div(a64, b64));
			break;
		}
		case X87_CW_PC_EXTEND:
		{
			val = extF80_div(a, b);
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

void i8087_device::fadd_m32real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fadd_m64real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fadd_st_sti(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fadd_sti_st(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
		write_stack(i, result, true);

	CYCLES(8);
}

void i8087_device::faddp(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
		inc_stack();
	}

	CYCLES(8);
}

void i8087_device::fiadd_m32int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(19);
}

void i8087_device::fiadd_m16int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = add(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(20);
}


/*************************************
 *
 * Subtract
 *
 *************************************/

void i8087_device::fsub_m32real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fsub_m64real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fsub_st_sti(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fsub_sti_st(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(i, result, true);

	CYCLES(8);
}

void i8087_device::fsubp(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
		inc_stack();
	}

	CYCLES(8);
}

void i8087_device::fisub_m32int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(19);
}

void i8087_device::fisub_m16int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(20);
}


/*************************************
 *
 * Reverse Subtract
 *
 *************************************/

void i8087_device::fsubr_m32real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = f32_to_extF80(m32real);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fsubr_m64real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = f64_to_extF80(m64real);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fsubr_st_sti(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

void i8087_device::fsubr_sti_st(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(i, result, true);

	CYCLES(8);
}

void i8087_device::fsubrp(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
		inc_stack();
	}

	CYCLES(8);
}

void i8087_device::fisubr_m32int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = i32_to_extF80(m32int);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(19);
}

void i8087_device::fisubr_m16int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ16(ea);

		extFloat80_t a = i32_to_extF80(m16int);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = sub(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(20);
}


/*************************************
 *
 * Divide
 *
 *************************************/

void i8087_device::fdiv_m32real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdiv_m64real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdiv_st_sti(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(0, result, true);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdiv_sti_st(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdivp(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
		inc_stack();
	}

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fidiv_m32int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fidiv_m16int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}


/*************************************
 *
 * Reverse Divide
 *
 *************************************/

void i8087_device::fdivr_m32real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = f32_to_extF80(m32real);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdivr_m64real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = f64_to_extF80(m64real);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdivr_st_sti(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(0, result, true);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdivr_sti_st(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
	}

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fdivrp(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
		inc_stack();
	}

	// 73, 62, 35
	CYCLES(73);
}


void i8087_device::fidivr_m32int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = i32_to_extF80(m32int);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i8087_device::fidivr_m16int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ32(ea);

		extFloat80_t a = i32_to_extF80(m16int);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = div(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}


/*************************************
 *
 * Multiply
 *
 *************************************/

void i8087_device::fmul_m32real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(11);
}

void i8087_device::fmul_m64real(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(14);
}

void i8087_device::fmul_st_sti(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(16);
}

void i8087_device::fmul_sti_st(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
		write_stack(i, result, true);

	CYCLES(16);
}

void i8087_device::fmulp(u8 modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
	{
		write_stack(i, result, true);
		inc_stack();
	}

	CYCLES(16);
}

void i8087_device::fimul_m32int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(22);
}

void i8087_device::fimul_m16int(u8 modrm)
{
	extFloat80_t result;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = mul(a, b);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(22);
}

/*************************************
 *
 * Miscellaneous arithmetic
 *
 *************************************/

void i8087_device::fprem(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		uint64_t q;

		m_sw &= ~X87_SW_C2;

		if (!extFloat80_remainder(ST(0), ST(1), result, q))
		{
			m_sw &= ~(X87_SW_C0 | X87_SW_C3 | X87_SW_C1);
			if (q & 1)
				m_sw |= X87_SW_C1;
			if (q & 2)
				m_sw |= X87_SW_C3;
			if (q & 4)
				m_sw |= X87_SW_C0;
		}
		else
			m_sw |= X87_SW_C2;
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(84);
}

void i8087_device::fprem1(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(1);

		m_sw &= ~X87_SW_C2;

		// TODO: Implement Cx bits
		result = extF80_rem(a, b);
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(94);
}

void i8087_device::fsqrt(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t value = ST(0);

		if ((!floatx80_is_zero(value) && (value.signExp & 0x8000)) ||
				floatx80_is_denormal(value))
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = extF80_sqrt(value);
		}
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(8);
}

/*************************************
 *
 * Trigonometric
 *
 *************************************/

void i8087_device::f2xm1(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = extFloat80_2xm1(ST(0));
	}

	if (check_exceptions())
	{
		write_stack(0, result, true);
	}

	CYCLES(242);
}

void i8087_device::fyl2x(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t x = ST(0);

		if (x.signExp & 0x8000)
		{
			m_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = extFloat80_fyl2x(ST(0), ST(1));
		}
	}

	if (check_exceptions())
	{
		write_stack(1, result, true);
		inc_stack();
	}

	CYCLES(250);
}

void i8087_device::fyl2xp1(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = extFloat80_fyl2xp1(ST(0), ST(1));
	}

	if (check_exceptions())
	{
		write_stack(1, result, true);
		inc_stack();
	}

	CYCLES(313);
}

void i8087_device::fptan(u8 modrm)
{
	extFloat80_t result1, result2;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result1 = fx80_inan;
		result2 = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		set_stack_overflow();
		result1 = fx80_inan;
		result2 = fx80_inan;
	}
	else
	{
		result1 = ST(0);
		result2 = fx80_one;

#if 1 // TODO: Function produces bad values
		if (extFloat80_tan(result1) != -1)
			m_sw &= ~X87_SW_C2;
		else
			m_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result1);
		x = tan(x);
		result1 = double_to_fx80(x);

		m_sw &= ~X87_SW_C2;
#endif
	}

	if (check_exceptions())
	{
		write_stack(0, result1, true);
		dec_stack();
		write_stack(0, result2, true);
	}

	CYCLES(244);
}

void i8087_device::fpatan(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = extFloat80_atan(ST(0), ST(1));
	}

	if (check_exceptions())
	{
		write_stack(1, result, true);
		inc_stack();
	}

	CYCLES(289);
}

void i8087_device::fsin(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 1 // TODO: Function produces bad values
		if (extFloat80_sin(result) != -1)
			m_sw &= ~X87_SW_C2;
		else
			m_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result);
		x = sin(x);
		result = double_to_fx80(x);

		m_sw &= ~X87_SW_C2;
#endif
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(241);
}

void i8087_device::fcos(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 1 // TODO: Function produces bad values
		if (extFloat80_cos(result) != -1)
			m_sw &= ~X87_SW_C2;
		else
			m_sw |= X87_SW_C2;
#else
		double x = fx80_to_double(result);
		x = cos(x);
		result = double_to_fx80(x);

		m_sw &= ~X87_SW_C2;
#endif
	}

	if (check_exceptions())
		write_stack(0, result, true);

	CYCLES(241);
}

void i8087_device::fsincos(u8 modrm)
{
	extFloat80_t s_result, c_result;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		s_result = c_result = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		set_stack_overflow();
		s_result = c_result = fx80_inan;
	}
	else
	{
		s_result = c_result = ST(0);

#if 1 // TODO: Function produces bad values
		if (extFloat80_sincos(s_result, &s_result, &c_result) != -1)
			m_sw &= ~X87_SW_C2;
		else
			m_sw |= X87_SW_C2;
#else
		double s = fx80_to_double(s_result);
		double c = fx80_to_double(c_result);
		s = sin(s);
		c = cos(c);

		s_result = double_to_fx80(s);
		c_result = double_to_fx80(c);

		m_sw &= ~X87_SW_C2;
#endif
	}

	if (check_exceptions())
	{
		write_stack(0, s_result, true);
		dec_stack();
		write_stack(0, c_result, true);
	}

	CYCLES(291);
}


/*************************************
 *
 * Load data
 *
 *************************************/

void i8087_device::fld_m32real(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (dec_stack())
	{
		float32_t m32real{ READ32(ea) };

		value = f32_to_extF80(m32real);

		m_sw &= ~X87_SW_C1;

		if (extF80_isSignalingNaN(value) || floatx80_is_denormal(value))
		{
			m_sw |= X87_SW_IE;
			value = fx80_inan;
		}
	}
	else
	{
		value = fx80_inan;
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(3);
}

void i8087_device::fld_m64real(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (dec_stack())
	{
		float64_t m64real{ READ64(ea) };

		value = f64_to_extF80(m64real);

		m_sw &= ~X87_SW_C1;

		if (extF80_isSignalingNaN(value) || floatx80_is_denormal(value))
		{
			m_sw |= X87_SW_IE;
			value = fx80_inan;
		}
	}
	else
	{
		value = fx80_inan;
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(3);
}

void i8087_device::fld_m80real(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (dec_stack())
	{
		m_sw &= ~X87_SW_C1;
		value = READ80(ea);
	}
	else
	{
		value = fx80_inan;
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(6);
}

void i8087_device::fld_sti(u8 modrm)
{
	extFloat80_t value;

	if (dec_stack())
	{
		m_sw &= ~X87_SW_C1;
		value = ST((modrm + 1) & 7);
	}
	else
	{
		value = fx80_inan;
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(4);
}

void i8087_device::fild_m16int(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (!dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		int16_t m16int = READ16(ea);
		value = i32_to_extF80(m16int);
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(13);
}

void i8087_device::fild_m32int(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (!dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		int32_t m32int = READ32(ea);
		value = i32_to_extF80(m32int);
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(9);
}

void i8087_device::fild_m64int(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (!dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		int64_t m64int = READ64(ea);
		value = i64_to_extF80(m64int);
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(10);
}

void i8087_device::fbld(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (!dec_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		uint64_t m64val = 0;
		u16 sign;

		value = READ80(ea);

		sign = value.signExp & 0x8000;
		m64val += ((value.signExp >> 4) & 0xf) * 10;
		m64val += ((value.signExp >> 0) & 0xf);

		for (int i = 60; i >= 0; i -= 4)
		{
			m64val *= 10;
			m64val += (value.signif >> i) & 0xf;
		}

		value = i64_to_extF80(m64val);
		value.signExp |= sign;
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(75);
}


/*************************************
 *
 * Store data
 *
 *************************************/

void i8087_device::fst_m32real(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	float32_t m32real = extF80_to_f32(value);
	if (check_exceptions(true))
		WRITE32(ea, m32real.v);

	CYCLES(7);
}

void i8087_device::fst_m64real(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	float64_t m64real = extF80_to_f64(value);
	if (check_exceptions(true))
		WRITE64(ea, m64real.v);

	CYCLES(8);
}

void i8087_device::fst_sti(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (check_exceptions())
		write_stack(i, value, true);

	CYCLES(3);
}

void i8087_device::fstp_m32real(u8 modrm)
{
	extFloat80_t value;

	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	float32_t m32real = extF80_to_f32(value);
	if (check_exceptions(true))
	{
		WRITE32(ea, m32real.v);
		inc_stack();
	}

	CYCLES(7);
}

void i8087_device::fstp_m64real(u8 modrm)
{
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}


	u32 ea = m_ea;
	float64_t m64real = extF80_to_f64(value);
	if (check_exceptions(true))
	{
		WRITE64(ea, m64real.v);
		inc_stack();
	}

	CYCLES(8);
}

void i8087_device::fstp_m80real(u8 modrm)
{
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE80(ea, value);
		inc_stack();
	}

	CYCLES(6);
}

void i8087_device::fstp_sti(u8 modrm)
{
	int i = modrm & 7;
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = ST(0);
	}

	if (check_exceptions())
	{
		write_stack(i, value, true);
		inc_stack();
	}

	CYCLES(3);
}

void i8087_device::fist_m16int(u8 modrm)
{
	int16_t m16int;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m16int = -32768;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(-32768);
		extFloat80_t upperLim = i32_to_extF80(32767);

		m_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m16int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
			m16int = -32768;
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE16(ea, m16int);
	}

	CYCLES(29);
}

void i8087_device::fist_m32int(u8 modrm)
{
	int32_t m32int;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m32int = 0x80000000;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(0x80000000);
		extFloat80_t upperLim = i32_to_extF80(0x7fffffff);

		m_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m32int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
			m32int = 0x80000000;
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE32(ea, m32int);
	}

	CYCLES(28);
}

void i8087_device::fistp_m16int(u8 modrm)
{
	int16_t m16int;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m16int = (u16)0x8000;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(-32768);
		extFloat80_t upperLim = i32_to_extF80(32767);

		m_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m16int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
			m16int = (u16)0x8000;
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE16(ea, m16int);
		inc_stack();
	}

	CYCLES(29);
}

void i8087_device::fistp_m32int(u8 modrm)
{
	int32_t m32int;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m32int = 0x80000000;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(0x80000000);
		extFloat80_t upperLim = i32_to_extF80(0x7fffffff);

		m_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m32int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
			m32int = 0x80000000;
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE32(ea, m32int);
		inc_stack();
	}

	CYCLES(29);
}

void i8087_device::fistp_m64int(u8 modrm)
{
	int64_t m64int;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m64int = 0x8000000000000000U;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i64_to_extF80(0x8000000000000000U);
		extFloat80_t upperLim = i64_to_extF80(0x7fffffffffffffffU);

		m_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m64int = extF80_to_i64(fx80, softfloat_roundingMode, true);
		else
			m64int = 0x8000000000000000U;
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE64(ea, m64int);
		inc_stack();
	}

	CYCLES(29);
}

void i8087_device::fbstp(u8 modrm)
{
	extFloat80_t result;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		uint64_t u64 = extF80_to_i64(floatx80_abs(ST(0)), softfloat_roundingMode, true);
		result.signif = 0;

		for (int i = 0; i < 64; i += 4)
		{
			result.signif += (u64 % 10) << i;
			u64 /= 10;
		}

		result.signExp = (u64 % 10);
		result.signExp += ((u64 / 10) % 10) << 4;
		result.signExp |= ST(0).signExp & 0x8000;
	}

	u32 ea = m_ea;
	if (check_exceptions(true))
	{
		WRITE80(ea, result);
		inc_stack();
	}

	CYCLES(175);
}


/*************************************
 *
 * Constant load
 *
 *************************************/

void i8087_device::fld1(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		m_sw &= ~X87_SW_C1;
		value = fx80_one;
		tag = X87_TW_VALID;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(4);
}

void i8087_device::fldl2t(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		tag = X87_TW_VALID;
		value.signExp = 0x4000;

		if (X87_RC == X87_CW_RC_UP)
			value.signif =  0xd49a784bcd1b8affU;
		else
			value.signif = 0xd49a784bcd1b8afeU;

		m_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(8);
}

void i8087_device::fldl2e(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x3fff;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0xb8aa3b295c17f0bcU;
		else
			value.signif = 0xb8aa3b295c17f0bbU;

		m_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(8);
}

void i8087_device::fldpi(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x4000;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0xc90fdaa22168c235U;
		else
			value.signif = 0xc90fdaa22168c234U;

		m_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(8);
}

void i8087_device::fldlg2(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x3ffd;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0x9a209a84fbcff799U;
		else
			value.signif = 0x9a209a84fbcff798U;

		m_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(8);
}

void i8087_device::fldln2(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x3ffe;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0xb17217f7d1cf79acU;
		else
			value.signif = 0xb17217f7d1cf79abU;

		m_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(8);
}

void i8087_device::fldz(u8 modrm)
{
	extFloat80_t value;
	int tag;

	if (dec_stack())
	{
		value = fx80_zero;
		tag = X87_TW_ZERO;
		m_sw &= ~X87_SW_C1;
	}
	else
	{
		value = fx80_inan;
		tag = X87_TW_SPECIAL;
	}

	if (check_exceptions())
	{
		set_tag(ST_TO_PHYS(0), tag);
		write_stack(0, value, false);
	}

	CYCLES(4);
}


/*************************************
 *
 * Miscellaneous
 *
 *************************************/

void i8087_device::fnop(u8 modrm)
{
	CYCLES(3);
}

void i8087_device::fchs(u8 modrm)
{
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		value = ST(0);
		value.signExp ^= 0x8000;
	}

	if (check_exceptions())
		write_stack(0, value, false);

	CYCLES(6);
}

void i8087_device::fabs(u8 modrm)
{
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		value = ST(0);
		value.signExp &= 0x7fff;
	}

	if (check_exceptions())
		write_stack(0, value, false);

	CYCLES(6);
}

void i8087_device::fscale(u8 modrm)
{
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;
		value = extFloat80_scale(ST(0), ST(1));
	}

	if (check_exceptions())
		write_stack(0, value, false);

	CYCLES(31);
}

void i8087_device::frndint(u8 modrm)
{
	extFloat80_t value;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_sw &= ~X87_SW_C1;

		value = extF80_roundToInt(ST(0), softfloat_roundingMode, true);
	}

	if (check_exceptions())
		write_stack(0, value, true);

	CYCLES(21);
}

void i8087_device::fxtract(u8 modrm)
{
	extFloat80_t sig80, exp80;

	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		sig80 = exp80 = fx80_inan;
	}
	else if (!X87_IS_ST_EMPTY(7))
	{
		set_stack_overflow();
		sig80 = exp80 = fx80_inan;
	}
	else
	{
		extFloat80_t value = ST(0);

		if (extF80_eq(value, fx80_zero))
		{
			m_sw |= X87_SW_ZE;

			exp80 = fx80_ninf;
			sig80 = fx80_zero;
		}
		else
		{
			// Extract the unbiased exponent
			exp80 = i32_to_extF80((value.signExp & 0x7fff) - 0x3fff);

			// For the significand, replicate the original value and set its true exponent to 0.
			sig80 = value;
			sig80.signExp &= ~0x7fff;
			sig80.signExp |=  0x3fff;
		}
	}

	if (check_exceptions())
	{
		write_stack(0, exp80, true);
		dec_stack();
		write_stack(0, sig80, true);
	}

	CYCLES(21);
}

/*************************************
 *
 * Comparison
 *
 *************************************/

void i8087_device::ftst(u8 modrm)
{
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		if (extFloat80_is_nan(ST(0)))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(ST(0), fx80_zero))
				m_sw |= X87_SW_C3;

			if (extF80_lt(ST(0), fx80_zero))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(4);
}

void i8087_device::fxam(u8 modrm)
{
	extFloat80_t value = ST(0);

	m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

	// TODO: Unsupported and denormal values
	if (X87_IS_ST_EMPTY(0))
	{
		m_sw |= X87_SW_C3 | X87_SW_C0;
	}
	else if (floatx80_is_zero(value))
	{
		m_sw |= X87_SW_C3;
	}
	else if (extFloat80_is_nan(value))
	{
		m_sw |= X87_SW_C0;
	}
	else if (floatx80_is_inf(value))
	{
		m_sw |= X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw |= X87_SW_C2;
	}

	if (value.signExp & 0x8000)
		m_sw |= X87_SW_C1;

	CYCLES(8);
}

void i8087_device::ficom_m16int(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extFloat80_is_nan(a))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(16);
}

void i8087_device::ficom_m32int(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extFloat80_is_nan(a))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(15);
}

void i8087_device::ficomp_m16int(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extFloat80_is_nan(a))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
		inc_stack();

	CYCLES(16);
}

void i8087_device::ficomp_m32int(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extFloat80_is_nan(a))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
		inc_stack();

	CYCLES(15);
}


void i8087_device::fcom_m32real(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(4);
}

void i8087_device::fcom_m64real(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(4);
}

void i8087_device::fcom_sti(u8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(4);
}

void i8087_device::fcomp_m32real(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
		inc_stack();

	CYCLES(4);
}

void i8087_device::fcomp_m64real(u8 modrm)
{
	u32 ea = m_ea;
	if (X87_IS_ST_EMPTY(0))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
		inc_stack();

	CYCLES(4);
}

void i8087_device::fcomp_sti(u8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
		inc_stack();

	CYCLES(4);
}

void i8087_device::fcompp(u8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(1);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
	{
		inc_stack();
		inc_stack();
	}

	CYCLES(5);
}


/*************************************
 *
 * Unordererd comparison
 *
 *************************************/

void i8087_device::fucom_sti(u8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
				m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	check_exceptions();

	CYCLES(4);
}

void i8087_device::fucomp_sti(u8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
				m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
		inc_stack();

	CYCLES(4);
}

void i8087_device::fucompp(u8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		set_stack_underflow();
		m_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(1);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
				m_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_sw |= X87_SW_C0;
		}
	}

	if (check_exceptions())
	{
		inc_stack();
		inc_stack();
	}

	CYCLES(4);
}


/*************************************
 *
 * Control
 *
 *************************************/

void i8087_device::fdecstp(u8 modrm)
{
	m_sw &= ~X87_SW_C1;

	set_stack_top(ST_TO_PHYS(7));

	CYCLES(3);
}

void i8087_device::fincstp(u8 modrm)
{
	m_sw &= ~X87_SW_C1;

	set_stack_top(ST_TO_PHYS(1));

	CYCLES(3);
}

void i8087_device::fclex(u8 modrm)
{
	m_sw &= ~0x80ff;

	m_int_handler(0);
	m_busy_handler(0);
	CYCLES(7);
}

void i8087_device::ffree(u8 modrm)
{
	set_tag(ST_TO_PHYS(modrm & 7), X87_TW_EMPTY);

	CYCLES(3);
}

void i8087_device::feni(u8 modrm)
{
	m_cw &= ~X87_CW_IEM;
	check_exceptions();

	CYCLES(5);
}

void i8087_device::fdisi(u8 modrm)
{
	m_cw |= X87_CW_IEM;

	CYCLES(5);
}

void i8087_device::finit(u8 modrm)
{
	reset();

	CYCLES(17);
}

void i8087_device::fldcw(u8 modrm)
{
	u32 ea = m_ea;
	u16 cw = READ16(ea);

	write_cw(cw);

	check_exceptions();

	CYCLES(4);
}

void i8087_device::fstcw(u8 modrm)
{
	u32 ea = m_ea;
	WRITE16(ea, m_cw);

	CYCLES(3);
}

void i8087_device::fldenv(u8 modrm)
{
	u32 ea = m_ea;
	u16 temp;

	write_cw(READ16(ea));
	m_sw = READ16(ea + 2);
	m_tw = READ16(ea + 4);
	m_ppc = READ16(ea + 6);
	temp = READ16(ea + 8);
	m_opcode = temp & 0x7ff;
	m_ppc |= ((temp & 0xf000) << 4);
	m_ea = READ16(ea + 10) | ((READ16(ea + 12) & 0xf000) << 4);

	check_exceptions();

	CYCLES(44);
}

void i8087_device::fstenv(u8 modrm)
{
	u32 ea = m_ea;
	m_cw |= 0x3f;   // set all masks

	WRITE16(ea + 0, m_cw);
	WRITE16(ea + 2, m_sw);
	WRITE16(ea + 4, m_tw);
	WRITE16(ea + 6, m_ppc & 0xffff);
	WRITE16(ea + 8, (m_opcode & 0x07ff) | ((m_ppc & 0x0f0000) >> 4));
	WRITE16(ea + 10, m_ea & 0xffff);
	WRITE16(ea + 12, (m_ea & 0x0f0000) >> 4);
	CYCLES(67);
}

void i8087_device::fsave(u8 modrm)
{
	u32 ea = m_ea;

	WRITE16(ea + 0, m_cw);
	WRITE16(ea + 2, m_sw);
	WRITE16(ea + 4, m_tw);
	WRITE16(ea + 6, m_ppc & 0xffff);
	WRITE16(ea + 8, (m_opcode & 0x07ff) | ((m_ppc & 0x0f0000) >> 4));
	WRITE16(ea + 10, m_ea & 0xffff);
	WRITE16(ea + 12, (m_ea & 0x0f0000) >> 4);

	for (int i = 0; i < 8; ++i)
		WRITE80(ea + 14 + i*10, ST(i));
	reset();

	CYCLES(67);
}

void i8087_device::frstor(u8 modrm)
{
	u32 ea = m_ea;
	u16 temp;

	write_cw(READ16(ea));
	m_sw = READ16(ea + 2);
	m_tw = READ16(ea + 4);
	m_ppc = READ16(ea + 6);
	temp = READ16(ea + 8);
	m_opcode = temp & 0x7ff;
	m_ppc |= ((temp & 0xf000) << 4);
	m_ea = READ16(ea + 10) | ((READ16(ea + 12) & 0xf000) << 4);

	for (int i = 0; i < 8; ++i)
		write_stack(i, READ80(ea + 14 + i*10), false);

	CYCLES(44);
}

void i8087_device::fxch(u8 modrm)
{
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
		set_stack_underflow();

	if (check_exceptions())
	{
		extFloat80_t tmp = ST(0);
		ST(0) = ST(1);
		ST(1) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		set_tag(ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(1)));
		set_tag(ST_TO_PHYS(1), tag0);
	}

	CYCLES(4);
}

void i8087_device::fxch_sti(u8 modrm)
{
	int i = modrm & 7;

	if (X87_IS_ST_EMPTY(0))
	{
		ST(0) = fx80_inan;
		set_tag(ST_TO_PHYS(0), X87_TW_SPECIAL);
		set_stack_underflow();
	}
	if (X87_IS_ST_EMPTY(i))
	{
		ST(i) = fx80_inan;
		set_tag(ST_TO_PHYS(i), X87_TW_SPECIAL);
		set_stack_underflow();
	}

	if (check_exceptions())
	{
		extFloat80_t tmp = ST(0);
		ST(0) = ST(i);
		ST(i) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		set_tag(ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(i)));
		set_tag(ST_TO_PHYS(i), tag0);
	}

	CYCLES(4);
}

void i8087_device::fstsw_ax(u8 modrm)
{
	logerror("FSTSW not supported on 8087 (PC:%.4x)\n", m_pc);
	CYCLES(1);
}

void i8087_device::fstsw_m2byte(u8 modrm)
{
	u32 ea = m_ea;

	WRITE16(ea, m_sw);

	CYCLES(3);
}

void i8087_device::invalid(u8 modrm)
{
	logerror("x87 invalid instruction (PC:%.4x)\n", m_pc);
	CYCLES(1);
}



/*************************************
 *
 * Instruction dispatch
 *
 *************************************/

void i8087_device::group_d8(u8 modrm)
{
	(this->*m_opcode_table_d8[modrm])(modrm);
}

void i8087_device::group_d9(u8 modrm)
{
	(this->*m_opcode_table_d9[modrm])(modrm);
}

void i8087_device::group_da(u8 modrm)
{
	(this->*m_opcode_table_da[modrm])(modrm);
}

void i8087_device::group_db(u8 modrm)
{
	(this->*m_opcode_table_db[modrm])(modrm);
}

void i8087_device::group_dc(u8 modrm)
{
	(this->*m_opcode_table_dc[modrm])(modrm);
}

void i8087_device::group_dd(u8 modrm)
{
	(this->*m_opcode_table_dd[modrm])(modrm);
}

void i8087_device::group_de(u8 modrm)
{
	(this->*m_opcode_table_de[modrm])(modrm);
}

void i8087_device::group_df(u8 modrm)
{
	(this->*m_opcode_table_df[modrm])(modrm);
}


/*************************************
 *
 * Opcode table building
 *
 *************************************/

void i8087_device::build_opcode_table_d8()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fadd_m32real;  break;
				case 0x01: ptr = &i8087_device::fmul_m32real;  break;
				case 0x02: ptr = &i8087_device::fcom_m32real;  break;
				case 0x03: ptr = &i8087_device::fcomp_m32real; break;
				case 0x04: ptr = &i8087_device::fsub_m32real;  break;
				case 0x05: ptr = &i8087_device::fsubr_m32real; break;
				case 0x06: ptr = &i8087_device::fdiv_m32real;  break;
				case 0x07: ptr = &i8087_device::fdivr_m32real; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i8087_device::fadd_st_sti;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i8087_device::fmul_st_sti;  break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = &i8087_device::fcom_sti;     break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i8087_device::fcomp_sti;    break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i8087_device::fsub_st_sti;  break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i8087_device::fsubr_st_sti; break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i8087_device::fdiv_st_sti;  break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = &i8087_device::fdivr_st_sti; break;
			}
		}

		m_opcode_table_d8[modrm] = ptr;
	}
}


void i8087_device::build_opcode_table_d9()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fld_m32real;   break;
				case 0x02: ptr = &i8087_device::fst_m32real;   break;
				case 0x03: ptr = &i8087_device::fstp_m32real;  break;
				case 0x04: ptr = &i8087_device::fldenv;        break;
				case 0x05: ptr = &i8087_device::fldcw;         break;
				case 0x06: ptr = &i8087_device::fstenv;        break;
				case 0x07: ptr = &i8087_device::fstcw;         break;
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
				case 0xc7: ptr = &i8087_device::fld_sti;   break;

				case 0xc8:
				case 0xc9:
				case 0xca:
				case 0xcb:
				case 0xcc:
				case 0xcd:
				case 0xce:
				case 0xcf: ptr = &i8087_device::fxch_sti;  break;

				case 0xd0: ptr = &i8087_device::fnop;      break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i8087_device::fstp_sti;     break;
				case 0xe0: ptr = &i8087_device::fchs;      break;
				case 0xe1: ptr = &i8087_device::fabs;      break;
				case 0xe4: ptr = &i8087_device::ftst;      break;
				case 0xe5: ptr = &i8087_device::fxam;      break;
				case 0xe8: ptr = &i8087_device::fld1;      break;
				case 0xe9: ptr = &i8087_device::fldl2t;    break;
				case 0xea: ptr = &i8087_device::fldl2e;    break;
				case 0xeb: ptr = &i8087_device::fldpi;     break;
				case 0xec: ptr = &i8087_device::fldlg2;    break;
				case 0xed: ptr = &i8087_device::fldln2;    break;
				case 0xee: ptr = &i8087_device::fldz;      break;
				case 0xf0: ptr = &i8087_device::f2xm1;     break;
				case 0xf1: ptr = &i8087_device::fyl2x;     break;
				case 0xf2: ptr = &i8087_device::fptan;     break;
				case 0xf3: ptr = &i8087_device::fpatan;    break;
				case 0xf4: ptr = &i8087_device::fxtract;   break;
				case 0xf5: ptr = &i8087_device::fprem1;    break;
				case 0xf6: ptr = &i8087_device::fdecstp;   break;
				case 0xf7: ptr = &i8087_device::fincstp;   break;
				case 0xf8: ptr = &i8087_device::fprem;     break;
				case 0xf9: ptr = &i8087_device::fyl2xp1;   break;
				case 0xfa: ptr = &i8087_device::fsqrt;     break;
				case 0xfb: ptr = &i8087_device::fsincos;   break;
				case 0xfc: ptr = &i8087_device::frndint;   break;
				case 0xfd: ptr = &i8087_device::fscale;    break;
				case 0xfe: ptr = &i8087_device::fsin;      break;
				case 0xff: ptr = &i8087_device::fcos;      break;
			}
		}

		m_opcode_table_d9[modrm] = ptr;
	}
}

void i8087_device::build_opcode_table_da()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fiadd_m32int;  break;
				case 0x01: ptr = &i8087_device::fimul_m32int;  break;
				case 0x02: ptr = &i8087_device::ficom_m32int;  break;
				case 0x03: ptr = &i8087_device::ficomp_m32int; break;
				case 0x04: ptr = &i8087_device::fisub_m32int;  break;
				case 0x05: ptr = &i8087_device::fisubr_m32int; break;
				case 0x06: ptr = &i8087_device::fidiv_m32int;  break;
				case 0x07: ptr = &i8087_device::fidivr_m32int; break;
			}
		}
		else
		{
			switch (modrm)
			{
			case 0xe9: ptr = &i8087_device::fucompp;       break;
			}
		}

		m_opcode_table_da[modrm] = ptr;
	}
}


void i8087_device::build_opcode_table_db()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fild_m32int;   break;
				case 0x02: ptr = &i8087_device::fist_m32int;   break;
				case 0x03: ptr = &i8087_device::fistp_m32int;  break;
				case 0x05: ptr = &i8087_device::fld_m80real;   break;
				case 0x07: ptr = &i8087_device::fstp_m80real;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xe0: ptr = &i8087_device::feni;          break; /* FENI */
				case 0xe1: ptr = &i8087_device::fdisi;         break; /* FDISI */
				case 0xe2: ptr = &i8087_device::fclex;         break;
				case 0xe3: ptr = &i8087_device::finit;         break;
				case 0xe4: ptr = &i8087_device::fnop;          break; /* FSETPM */
			}
		}

		m_opcode_table_db[modrm] = ptr;
	}
}


void i8087_device::build_opcode_table_dc()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fadd_m64real;  break;
				case 0x01: ptr = &i8087_device::fmul_m64real;  break;
				case 0x02: ptr = &i8087_device::fcom_m64real;  break;
				case 0x03: ptr = &i8087_device::fcomp_m64real; break;
				case 0x04: ptr = &i8087_device::fsub_m64real;  break;
				case 0x05: ptr = &i8087_device::fsubr_m64real; break;
				case 0x06: ptr = &i8087_device::fdiv_m64real;  break;
				case 0x07: ptr = &i8087_device::fdivr_m64real; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i8087_device::fadd_sti_st;  break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i8087_device::fmul_sti_st;  break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i8087_device::fsubr_sti_st; break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i8087_device::fsub_sti_st;  break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i8087_device::fdivr_sti_st; break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = &i8087_device::fdiv_sti_st;  break;
			}
		}

		m_opcode_table_dc[modrm] = ptr;
	}
}


void i8087_device::build_opcode_table_dd()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fld_m64real;   break;
				case 0x02: ptr = &i8087_device::fst_m64real;   break;
				case 0x03: ptr = &i8087_device::fstp_m64real;  break;
				case 0x04: ptr = &i8087_device::frstor;        break;
				case 0x06: ptr = &i8087_device::fsave;         break;
				case 0x07: ptr = &i8087_device::fstsw_m2byte;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i8087_device::ffree;        break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i8087_device::fxch_sti;     break;
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: ptr = &i8087_device::fst_sti;      break;
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i8087_device::fstp_sti;     break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i8087_device::fucom_sti;    break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i8087_device::fucomp_sti;   break;
			}
		}

		m_opcode_table_dd[modrm] = ptr;
	}
}


void i8087_device::build_opcode_table_de()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fiadd_m16int;  break;
				case 0x01: ptr = &i8087_device::fimul_m16int;  break;
				case 0x02: ptr = &i8087_device::ficom_m16int;  break;
				case 0x03: ptr = &i8087_device::ficomp_m16int; break;
				case 0x04: ptr = &i8087_device::fisub_m16int;  break;
				case 0x05: ptr = &i8087_device::fisubr_m16int; break;
				case 0x06: ptr = &i8087_device::fidiv_m16int;  break;
				case 0x07: ptr = &i8087_device::fidivr_m16int; break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: ptr = &i8087_device::faddp;    break;
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: ptr = &i8087_device::fmulp;    break;
				case 0xd9: ptr = &i8087_device::fcompp; break;
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: ptr = &i8087_device::fsubrp;   break;
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef: ptr = &i8087_device::fsubp;    break;
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: ptr = &i8087_device::fdivrp;   break;
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: ptr = &i8087_device::fdivp;    break;
			}
		}

		m_opcode_table_de[modrm] = ptr;
	}
}


void i8087_device::build_opcode_table_df()
{
	int modrm = 0;

	for (modrm = 0; modrm < 0x100; ++modrm)
	{
		x87_func ptr = &i8087_device::invalid;

		if (modrm < 0xc0)
		{
			switch ((modrm >> 3) & 0x7)
			{
				case 0x00: ptr = &i8087_device::fild_m16int;   break;
				case 0x02: ptr = &i8087_device::fist_m16int;   break;
				case 0x03: ptr = &i8087_device::fistp_m16int;  break;
				case 0x04: ptr = &i8087_device::fbld;          break;
				case 0x05: ptr = &i8087_device::fild_m64int;   break;
				case 0x06: ptr = &i8087_device::fbstp;         break;
				case 0x07: ptr = &i8087_device::fistp_m64int;  break;
			}
		}
		else
		{
			switch (modrm)
			{
				case 0xe0: ptr = &i8087_device::fstsw_ax;      break;
			}
		}

		m_opcode_table_df[modrm] = ptr;
	}
}

void i8087_device::build_opcode_table()
{
	build_opcode_table_d8();
	build_opcode_table_d9();
	build_opcode_table_da();
	build_opcode_table_db();
	build_opcode_table_dc();
	build_opcode_table_dd();
	build_opcode_table_de();
	build_opcode_table_df();
}
