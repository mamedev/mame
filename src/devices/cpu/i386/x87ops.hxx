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

   Corrections and Additions [8-December-2017 Andrey Merkulov)
     FXAM, FPREM - fixed
     FINCSTP, FDECSTP - tags and exceptions corrected

***************************************************************************/

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

void i386_device::x87_write_stack(int i, extFloat80_t value, bool update_tag)
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

int i386_device::x87_ck_over_stack()
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

	return ret;
}

/*************************************
 *
 * Exception handling
 *
 *************************************/

int i386_device::x87_mf_fault()
{
	if ((m_x87_sw & X87_SW_ES) && (m_cr[0] & CR0_NE)) // FIXME: 486 and up only
	{
		m_ext = 1;
		i386_trap(FAULT_MF, 0);
		return 1;
	}
	return 0;
}

uint32_t i386_device::Getx87EA(uint8_t modrm, int rwn)
{
	uint8_t segment;
	uint32_t ea;
	modrm_to_EA(modrm, &ea, &segment);
	uint32_t ret = i386_translate(segment, ea, rwn);
	m_x87_ds = m_sreg[segment].selector;
	if (PROTECTED_MODE && !V8086_MODE)
		m_x87_data_ptr = ea;
	else
		m_x87_data_ptr = ea + (m_x87_ds << 4);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	return ret;
}

int i386_device::x87_check_exceptions(bool store)
{
	m_x87_cs = m_sreg[CS].selector;
	if (PROTECTED_MODE && !V8086_MODE)
		m_x87_inst_ptr = m_prev_eip;
	else
		m_x87_inst_ptr = m_prev_eip + (m_x87_cs << 4);

	/* Update the exceptions from SoftFloat */
	if (softfloat_exceptionFlags & softfloat_flag_invalid)
	{
		m_x87_sw |= X87_SW_IE;
		softfloat_exceptionFlags &= ~softfloat_flag_invalid;
	}
	if (softfloat_exceptionFlags & softfloat_flag_overflow)
	{
		m_x87_sw |= X87_SW_OE;
		softfloat_exceptionFlags &= ~softfloat_flag_overflow;
	}
	if (softfloat_exceptionFlags & softfloat_flag_underflow)
	{
		m_x87_sw |= X87_SW_UE;
		softfloat_exceptionFlags &= ~softfloat_flag_underflow;
	}
	if (softfloat_exceptionFlags & softfloat_flag_inexact)
	{
		m_x87_sw |= X87_SW_PE;
		softfloat_exceptionFlags &= ~softfloat_flag_inexact;
	}
	if (softfloat_exceptionFlags & softfloat_flag_infinite)
	{
		m_x87_sw |= X87_SW_ZE;
		softfloat_exceptionFlags &= ~softfloat_flag_infinite;
	}

	uint16_t unmasked = (m_x87_sw & ~m_x87_cw) & 0x3f;
	if ((m_x87_sw & ~m_x87_cw) & 0x3f)
	{
		// m_device->execute().set_input_line(INPUT_LINE_FERR, RAISE_LINE);
		LOG("Unmasked x87 exception (CW:%.4x, SW:%.4x)\n", m_x87_cw, m_x87_sw);
		// interrupt handler
		m_x87_sw |= X87_SW_ES;
		m_ferr_handler(1);
		if (store || !(unmasked & (X87_SW_OE | X87_SW_UE)))
			return 0;
	}

	return 1;
}

void i386_device::x87_write_cw(uint16_t cw)
{
	m_x87_cw = cw;

	/* Update the SoftFloat rounding mode */
	softfloat_roundingMode = x87_to_sf_rc[(m_x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK];
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

	m_ferr_handler(0);
}

/*************************************
 *
 * Core arithmetic
 *
 *************************************/

extFloat80_t i386_device::x87_add(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t result = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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

extFloat80_t i386_device::x87_sub(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t result = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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

extFloat80_t i386_device::x87_mul(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t val = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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


extFloat80_t i386_device::x87_div(extFloat80_t a, extFloat80_t b)
{
	extFloat80_t val = { 0 };

	switch ((m_x87_cw >> X87_CW_PC_SHIFT) & X87_CW_PC_MASK)
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

void i386_device::x87_fadd_m32real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(8);
}

void i386_device::x87_fadd_m64real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(8);
}

void i386_device::x87_fadd_st_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fadd_sti_st(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(i, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_faddp(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(i, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fiadd_m32int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(19);
}

void i386_device::x87_fiadd_m16int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_add(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(20);
}


/*************************************
 *
 * Subtract
 *
 *************************************/

void i386_device::x87_fsub_m32real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(8);
}

void i386_device::x87_fsub_m64real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(8);
}

void i386_device::x87_fsub_st_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fsub_sti_st(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(i, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fsubp(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(i, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fisub_m32int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(19);
}

void i386_device::x87_fisub_m16int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(20);
}


/*************************************
 *
 * Reverse Subtract
 *
 *************************************/

void i386_device::x87_fsubr_m32real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(8);
}

void i386_device::x87_fsubr_m64real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(8);
}

void i386_device::x87_fsubr_st_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fsubr_sti_st(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(i, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fsubrp(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if ((extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
		|| (floatx80_is_inf(a) && floatx80_is_inf(b) && ((a.signExp ^ b.signExp) & 0x8000)))
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
		x87_write_stack(i, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fisubr_m32int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(19);
}

void i386_device::x87_fisubr_m16int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = x87_sub(a, b);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);

	CYCLES(20);
}


/*************************************
 *
 * Divide
 *
 *************************************/

void i386_device::x87_fdiv_m32real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdiv_m64real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdiv_st_sti(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdiv_sti_st(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(i, result, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivp(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(i, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fidiv_m32int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fidiv_m16int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}


/*************************************
 *
 * Reverse Divide
 *
 *************************************/

void i386_device::x87_fdivr_m32real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = f32_to_extF80(m32real);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivr_m64real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = f64_to_extF80(m64real);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivr_st_sti(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(i);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivr_sti_st(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(i, result, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fdivrp(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(i, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	// 73, 62, 35
	CYCLES(73);
}


void i386_device::x87_fidivr_m32int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = i32_to_extF80(m32int);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}

void i386_device::x87_fidivr_m16int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ32(ea);

		extFloat80_t a = i32_to_extF80(m16int);
		extFloat80_t b = ST(0);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	// 73, 62, 35
	CYCLES(73);
}


/*************************************
 *
 * Multiply
 *
 *************************************/

void i386_device::x87_fmul_m32real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	CYCLES(11);
}

void i386_device::x87_fmul_m64real(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	CYCLES(14);
}

void i386_device::x87_fmul_st_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(16);
}

void i386_device::x87_fmul_sti_st(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(i, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(16);
}

void i386_device::x87_fmulp(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(i, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(16);
}

void i386_device::x87_fimul_m32int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	CYCLES(22);
}

void i386_device::x87_fimul_m16int(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
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
		x87_write_stack(0, result, true);

	CYCLES(22);
}

/*************************************
*
* Conditional Move
*
*************************************/

void i386_device::x87_fcmovb_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmove_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmovbe_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmovu_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmovnb_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmovne_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmovnbe_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcmovnu_sti(uint8_t modrm)
{
	extFloat80_t result;
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

/*************************************
 *
 * Miscellaneous arithmetic
 *
 *************************************/
/* D9 F8 */
void i386_device::x87_fprem(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		uint64_t q;

		m_x87_sw &= ~X87_SW_C2;

		if (!extFloat80_remainder(ST(0), ST(1), result, q))
		{
			m_x87_sw &= ~(X87_SW_C0 | X87_SW_C3 | X87_SW_C1);
			if (q & 1)
				m_x87_sw |= X87_SW_C1;
			if (q & 2)
				m_x87_sw |= X87_SW_C3;
			if (q & 4)
				m_x87_sw |= X87_SW_C0;
		}
		else
			m_x87_sw |= X87_SW_C2;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(84);
}

void i386_device::x87_fprem1(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		uint64_t q;

		m_x87_sw &= ~X87_SW_C2;

		if (!extFloat80_ieee754_remainder(ST(0), ST(1), result, q)) {
			m_x87_sw &= ~(X87_SW_C0|X87_SW_C3|X87_SW_C1);
			if (q & 1)
				m_x87_sw |= X87_SW_C1;
			if (q & 2)
				m_x87_sw |= X87_SW_C3;
			if (q & 4)
				m_x87_sw |= X87_SW_C0;
		}
		else
			m_x87_sw |= X87_SW_C2;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(94);
}

void i386_device::x87_fsqrt(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t value = ST(0);

		if ((!floatx80_is_zero(value) && (value.signExp & 0x8000)) ||
				floatx80_is_denormal(value))
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = extF80_sqrt(value);
		}
	}

	if (x87_check_exceptions())
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

/*************************************
 *
 * Trigonometric
 *
 *************************************/

void i386_device::x87_f2xm1(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = extFloat80_2xm1(ST(0));
	}
	if (x87_check_exceptions())
	{
		x87_write_stack(0, result, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(242);
}

void i386_device::x87_fyl2x(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		extFloat80_t x = ST(0);

		if (x.signExp & 0x8000)
		{
			m_x87_sw |= X87_SW_IE;
			result = fx80_inan;
		}
		else
		{
			result = extFloat80_fyl2x(ST(0), ST(1));
		}
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(1, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(250);
}

void i386_device::x87_fyl2xp1(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = extFloat80_fyl2xp1(ST(0), ST(1));
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(1, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(313);
}
/* D9 F2 if 8087   0 < angle < pi/4 */
void i386_device::x87_fptan(uint8_t modrm)
{
	extFloat80_t result1, result2;

	if (x87_mf_fault())
		return;
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

#if 1 // TODO: Function produces bad values
		if (extFloat80_tan(result1) != -1)
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
		x87_write_stack(0, result1, true);
		x87_dec_stack();
		x87_write_stack(0, result2, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(244);
}
/* D9 F3 */
void i386_device::x87_fpatan(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = extFloat80_atan(ST(0), ST(1));
	}

	if (x87_check_exceptions())
	{
		x87_write_stack(1, result, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(289);
}
/* D9 FE  387 only */
void i386_device::x87_fsin(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = ST(0);


#if 1 // TODO: Function produces bad values    Result checked
		if (extFloat80_sin(result) != -1)
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
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(241);
}
/* D9 FF 387 only */
void i386_device::x87_fcos(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		result = fx80_inan;
	}
	else
	{
		result = ST(0);

#if 1 // TODO: Function produces bad values   to check!
		if (extFloat80_cos(result) != -1)
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
		x87_write_stack(0, result, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(241);
}
/* D9 FB  387 only */
void i386_device::x87_fsincos(uint8_t modrm)
{
	extFloat80_t s_result, c_result;

	if (x87_mf_fault())
		return;
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
		s_result = c_result = ST(0);

#if 1 // TODO: Function produces bad values
		if (extFloat80_sincos(s_result, &s_result, &c_result) != -1)
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
		x87_write_stack(0, s_result, true);
		x87_dec_stack();
		x87_write_stack(0, c_result, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(291);
}


/*************************************
 *
 * Load data
 *
 *************************************/

void i386_device::x87_fld_m32real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (x87_ck_over_stack())
	{
		float32_t m32real{ READ32(ea) };

		value = f32_to_extF80(m32real);

		m_x87_sw &= ~X87_SW_C1;

		if (extF80_isSignalingNaN(value) || floatx80_is_denormal(value))
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
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(3);
}

void i386_device::x87_fld_m64real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (x87_ck_over_stack())
	{
		float64_t m64real{ READ64(ea) };

		value = f64_to_extF80(m64real);

		m_x87_sw &= ~X87_SW_C1;

		if (extF80_isSignalingNaN(value) || floatx80_is_denormal(value))
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
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(3);
}

void i386_device::x87_fld_m80real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (x87_ck_over_stack())
	{
		m_x87_sw &= ~X87_SW_C1;
		value = READ80(ea);
	}
	else
	{
		value = fx80_inan;
	}

	if (x87_check_exceptions())
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(6);
}

void i386_device::x87_fld_sti(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
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
		x87_write_stack(0, value, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fild_m16int(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (!x87_ck_over_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		int16_t m16int = READ16(ea);
		value = i32_to_extF80(m16int);
	}

	if (x87_check_exceptions())
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(13);
}

void i386_device::x87_fild_m32int(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (!x87_ck_over_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		int32_t m32int = READ32(ea);
		value = i32_to_extF80(m32int);
	}

	if (x87_check_exceptions())
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(9);
}

void i386_device::x87_fild_m64int(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (!x87_ck_over_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		int64_t m64int = READ64(ea);
		value = i64_to_extF80(m64int);
	}

	if (x87_check_exceptions())
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(10);
}

void i386_device::x87_fbld(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (!x87_ck_over_stack())
	{
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		uint64_t m64val = 0;
		uint16_t sign;

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

	if (x87_check_exceptions())
	{
		x87_set_stack_top(ST_TO_PHYS(7));
		x87_write_stack(0, value, true);
	}

	CYCLES(75);
}


/*************************************
 *
 * Store data
 *
 *************************************/

void i386_device::x87_fst_m32real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 1);
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

	float32_t m32real = extF80_to_f32(value);
	if (x87_check_exceptions(true))
		WRITE32(ea, m32real.v);

	CYCLES(7);
}

void i386_device::x87_fst_m64real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 1);
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

	float64_t m64real = extF80_to_f64(value);
	if (x87_check_exceptions(true))
		WRITE64(ea, m64real.v);

	CYCLES(8);
}

void i386_device::x87_fst_sti(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t value;

	if (x87_mf_fault())
		return;
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
		x87_write_stack(i, value, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(3);
}

void i386_device::x87_fstp_m32real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 1);
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

	float32_t m32real = extF80_to_f32(value);
	if (x87_check_exceptions(true))
	{
		WRITE32(ea, m32real.v);
		x87_inc_stack();
	}

	CYCLES(7);
}

void i386_device::x87_fstp_m64real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
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


	uint32_t ea = Getx87EA(modrm, 1);
	float64_t m64real = extF80_to_f64(value);
	if (x87_check_exceptions(true))
	{
		WRITE64(ea, m64real.v);
		x87_inc_stack();
	}

	CYCLES(8);
}

void i386_device::x87_fstp_m80real(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
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

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
	{
		WRITE80(ea, value);
		x87_inc_stack();
	}

	CYCLES(6);
}

void i386_device::x87_fstp_sti(uint8_t modrm)
{
	int i = modrm & 7;
	extFloat80_t value;

	if (x87_mf_fault())
		return;
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
		x87_write_stack(i, value, true);
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(3);
}

void i386_device::x87_fist_m16int(uint8_t modrm)
{
	int16_t m16int;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m16int = -32768;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(-32768);
		extFloat80_t upperLim = i32_to_extF80(32767);

		m_x87_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m16int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
		{
			softfloat_exceptionFlags = softfloat_flag_invalid;
			m16int = -32768;
		}
	}

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
	{
		WRITE16(ea, m16int);
	}

	CYCLES(29);
}

void i386_device::x87_fist_m32int(uint8_t modrm)
{
	int32_t m32int;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m32int = 0x80000000;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(0x80000000);
		extFloat80_t upperLim = i32_to_extF80(0x7fffffff);

		m_x87_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m32int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
		{
			softfloat_exceptionFlags = softfloat_flag_invalid;
			m32int = 0x80000000;
		}
	}

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
	{
		WRITE32(ea, m32int);
	}

	CYCLES(28);
}

void i386_device::x87_fistp_m16int(uint8_t modrm)
{
	int16_t m16int;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m16int = (uint16_t)0x8000;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(-32768);
		extFloat80_t upperLim = i32_to_extF80(32767);

		m_x87_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m16int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
		{
			softfloat_exceptionFlags = softfloat_flag_invalid;
			m16int = (uint16_t)0x8000;
		}
	}

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
	{
		WRITE16(ea, m16int);
		x87_inc_stack();
	}

	CYCLES(29);
}

void i386_device::x87_fistp_m32int(uint8_t modrm)
{
	int32_t m32int;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m32int = 0x80000000;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i32_to_extF80(0x80000000);
		extFloat80_t upperLim = i32_to_extF80(0x7fffffff);

		m_x87_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m32int = extF80_to_i32(fx80, softfloat_roundingMode, true);
		else
		{
			softfloat_exceptionFlags = softfloat_flag_invalid;
			m32int = 0x80000000;
		}
	}

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
	{
		WRITE32(ea, m32int);
		x87_inc_stack();
	}

	CYCLES(29);
}

void i386_device::x87_fistp_m64int(uint8_t modrm)
{
	int64_t m64int;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m64int = 0x8000000000000000U;
	}
	else
	{
		extFloat80_t fx80 = extF80_roundToInt(ST(0), softfloat_roundingMode, true);

		extFloat80_t lowerLim = i64_to_extF80(0x8000000000000000U);
		extFloat80_t upperLim = i64_to_extF80(0x7fffffffffffffffU);

		m_x87_sw &= ~X87_SW_C1;

		if (!extF80_lt(fx80, lowerLim) && extF80_le(fx80, upperLim))
			m64int = extF80_to_i64(fx80, softfloat_roundingMode, true);
		else
		{
			softfloat_exceptionFlags = softfloat_flag_invalid;
			m64int = 0x8000000000000000U;
		}
	}

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
	{
		WRITE64(ea, m64int);
		x87_inc_stack();
	}

	CYCLES(29);
}

void i386_device::x87_fbstp(uint8_t modrm)
{
	extFloat80_t result;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
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

	uint32_t ea = Getx87EA(modrm, 1);
	if (x87_check_exceptions(true))
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

void i386_device::x87_fld1(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fldl2t(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
	if (x87_dec_stack())
	{
		tag = X87_TW_VALID;
		value.signExp = 0x4000;

		if (X87_RC == X87_CW_RC_UP)
			value.signif =  0xd49a784bcd1b8affU;
		else
			value.signif = 0xd49a784bcd1b8afeU;

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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fldl2e(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x3fff;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0xb8aa3b295c17f0bcU;
		else
			value.signif = 0xb8aa3b295c17f0bbU;

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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fldpi(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x4000;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0xc90fdaa22168c235U;
		else
			value.signif = 0xc90fdaa22168c234U;

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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fldlg2(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x3ffd;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0x9a209a84fbcff799U;
		else
			value.signif = 0x9a209a84fbcff798U;

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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fldln2(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
	if (x87_dec_stack())
	{
		int rc = X87_RC;
		tag = X87_TW_VALID;
		value.signExp = 0x3ffe;

		if (rc == X87_CW_RC_UP || rc == X87_CW_RC_NEAREST)
			value.signif = 0xb17217f7d1cf79acU;
		else
			value.signif = 0xb17217f7d1cf79abU;

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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(8);
}

void i386_device::x87_fldz(uint8_t modrm)
{
	extFloat80_t value;
	int tag;

	if (x87_mf_fault())
		return;
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
		x87_write_stack(0, value, false);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}


/*************************************
 *
 * Miscellaneous
 *
 *************************************/

void i386_device::x87_fnop(uint8_t modrm)
{
	x87_mf_fault();
	CYCLES(3);
}

void i386_device::x87_fchs(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		value = ST(0);
		value.signExp ^= 0x8000;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, false);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(6);
}

void i386_device::x87_fabs(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		value = ST(0);
		value.signExp &= 0x7fff;
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, false);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(6);
}

void i386_device::x87_fscale(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;
		value = extFloat80_scale(ST(0), ST(1));
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, false);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(31);
}

void i386_device::x87_frndint(uint8_t modrm)
{
	extFloat80_t value;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		value = fx80_inan;
	}
	else
	{
		m_x87_sw &= ~X87_SW_C1;

		value = extF80_roundToInt(ST(0), softfloat_roundingMode, true);
	}

	if (x87_check_exceptions())
		x87_write_stack(0, value, true);
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(21);
}

void i386_device::x87_fxtract(uint8_t modrm)
{
	extFloat80_t sig80, exp80;

	if (x87_mf_fault())
		return;
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
		extFloat80_t value = ST(0);

		if (extF80_eq(value, fx80_zero))
		{
			m_x87_sw |= X87_SW_ZE;

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

	if (x87_check_exceptions())
	{
		x87_write_stack(0, exp80, true);
		x87_dec_stack();
		x87_write_stack(0, sig80, true);
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(21);
}

/*************************************
 *
 * Comparison
 *
 *************************************/

void i386_device::x87_ftst(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		if (extFloat80_is_nan(ST(0)))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(ST(0), fx80_zero))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(ST(0), fx80_zero))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fxam(uint8_t modrm)
{
	extFloat80_t value = ST(0);

	if (x87_mf_fault())
		return;
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
	else if (extFloat80_is_nan(value))
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

	if (value.signExp & 0x8000)
		m_x87_sw |= X87_SW_C1;

	CYCLES(8);
}

void i386_device::x87_ficom_m16int(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extFloat80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(16);
}

void i386_device::x87_ficom_m32int(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extFloat80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(15);
}

void i386_device::x87_ficomp_m16int(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int16_t m16int = READ16(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m16int);

		if (extFloat80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(16);
}

void i386_device::x87_ficomp_m32int(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		int32_t m32int = READ32(ea);

		extFloat80_t a = ST(0);
		extFloat80_t b = i32_to_extF80(m32int);

		if (extFloat80_is_nan(a))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(15);
}


void i386_device::x87_fcom_m32real(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fcom_m64real(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fcom_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcomp_m32real(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float32_t m32real{ READ32(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f32_to_extF80(m32real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4);
}

void i386_device::x87_fcomp_m64real(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	if (X87_IS_ST_EMPTY(0))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		float64_t m64real{ READ64(ea) };

		extFloat80_t a = ST(0);
		extFloat80_t b = f64_to_extF80(m64real);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();

	CYCLES(4);
}

void i386_device::x87_fcomp_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fcomi_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
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

			if (extF80_eq(a, b))
				m_ZF = 1;

			if (extF80_lt(a, b))
				m_CF = 1;
		}
	}

	x87_check_exceptions();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fcomip_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
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

			if (extF80_eq(a, b))
				m_ZF = 1;

			if (extF80_lt(a, b))
				m_CF = 1;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fucomi_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (floatx80_is_quiet_nan(a) || floatx80_is_quiet_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
		}
		else if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
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

			if (extF80_eq(a, b))
				m_ZF = 1;

			if (extF80_lt(a, b))
				m_CF = 1;
		}
	}

	x87_check_exceptions();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fucomip_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (floatx80_is_quiet_nan(a) || floatx80_is_quiet_nan(b))
		{
			m_ZF = 1;
			m_PF = 1;
			m_CF = 1;
		}
		else if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
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

			if (extF80_eq(a, b))
				m_ZF = 1;

			if (extF80_lt(a, b))
				m_CF = 1;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4); // TODO: correct cycle count
}

void i386_device::x87_fcompp(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(1);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;
			m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
	{
		x87_inc_stack();
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(5);
}


/*************************************
 *
 * Unordererd comparison
 *
 *************************************/

void i386_device::x87_fucom_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
				m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	x87_check_exceptions();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fucomp_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(i))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(i);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
				m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
		x87_inc_stack();
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}

void i386_device::x87_fucompp(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
	{
		x87_set_stack_underflow();
		m_x87_sw |= X87_SW_C3 | X87_SW_C2 | X87_SW_C0;
	}
	else
	{
		m_x87_sw &= ~(X87_SW_C3 | X87_SW_C2 | X87_SW_C1 | X87_SW_C0);

		extFloat80_t a = ST(0);
		extFloat80_t b = ST(1);

		if (extFloat80_is_nan(a) || extFloat80_is_nan(b))
		{
			m_x87_sw |= X87_SW_C0 | X87_SW_C2 | X87_SW_C3;

			if (extF80_isSignalingNaN(a) || extF80_isSignalingNaN(b))
				m_x87_sw |= X87_SW_IE;
		}
		else
		{
			if (extF80_eq(a, b))
				m_x87_sw |= X87_SW_C3;

			if (extF80_lt(a, b))
				m_x87_sw |= X87_SW_C0;
		}
	}

	if (x87_check_exceptions())
	{
		x87_inc_stack();
		x87_inc_stack();
	}
	m_x87_opcode = ((m_opcode << 8) | modrm) & 0x7ff;
	m_x87_data_ptr = 0;
	m_x87_ds = 0;

	CYCLES(4);
}


/*************************************
 *
 * Control
 *
 *************************************/

void i386_device::x87_fdecstp(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	m_x87_sw &= ~X87_SW_C1;

	x87_set_stack_top(ST_TO_PHYS(7));

	CYCLES(3);
}

void i386_device::x87_fincstp(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	m_x87_sw &= ~X87_SW_C1;

	x87_set_stack_top(ST_TO_PHYS(1));

	CYCLES(3);
}

void i386_device::x87_fclex(uint8_t modrm)
{
	m_x87_sw &= ~0x80ff;
	m_ferr_handler(0);
	CYCLES(7);
}

void i386_device::x87_ffree(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	x87_set_tag(ST_TO_PHYS(modrm & 7), X87_TW_EMPTY);

	CYCLES(3);
}

void i386_device::x87_finit(uint8_t modrm)
{
	x87_reset();

	CYCLES(17);
}

void i386_device::x87_fldcw(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	uint16_t cw = READ16(ea);

	x87_write_cw(cw);

	x87_check_exceptions();

	CYCLES(4);
}

void i386_device::x87_fstcw(uint8_t modrm)
{
	uint32_t ea = GetEA(modrm, 1);
	WRITE16(ea, m_x87_cw);

	CYCLES(3);
}

void i386_device::x87_fldenv(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = Getx87EA(modrm, 0);
	uint32_t temp;

	switch(((PROTECTED_MODE && !V8086_MODE) ? 1 : 0) | (m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 2);
			m_x87_tw = READ16(ea + 4);
			m_x87_inst_ptr = READ16(ea + 6);
			temp = READ16(ea + 8);
			m_x87_opcode = temp & 0x7ff;
			m_x87_inst_ptr |= ((temp & 0xf000) << 4);
			m_x87_data_ptr = READ16(ea + 10) | ((READ16(ea + 12) & 0xf000) << 4);
			m_x87_cs = 0;
			m_x87_ds = 0;
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 2);
			m_x87_tw = READ16(ea + 4);
			m_x87_inst_ptr = READ16(ea + 6);
			m_x87_opcode = 0;
			m_x87_cs = READ16(ea + 8);
			m_x87_data_ptr = READ16(ea + 10);
			m_x87_ds = READ16(ea + 12);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 4);
			m_x87_tw = READ16(ea + 8);
			m_x87_inst_ptr = READ16(ea + 12);
			temp = READ32(ea + 16);
			m_x87_opcode = temp & 0x7ff;
			m_x87_inst_ptr |= ((temp & 0xffff000) << 4);
			m_x87_data_ptr = READ16(ea + 20) | ((READ32(ea + 24) & 0xffff000) << 4);
			m_x87_cs = 0;
			m_x87_ds = 0;
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 4);
			m_x87_tw = READ16(ea + 8);
			m_x87_inst_ptr = READ32(ea + 12);
			temp = READ32(ea + 16);
			m_x87_opcode = (temp >> 16) & 0x7ff;
			m_x87_cs = temp & 0xffff;
			m_x87_data_ptr = READ32(ea + 20);
			m_x87_ds = READ16(ea + 24);
			ea += 28;
			break;
	}

	x87_check_exceptions();

	CYCLES((m_cr[0] & CR0_PE) ? 34 : 44);
}

void i386_device::x87_fstenv(uint8_t modrm)
{
	uint32_t ea = GetEA(modrm, 1);

	switch(((PROTECTED_MODE && !V8086_MODE) ? 1 : 0) | (m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
			WRITE16(ea + 6, m_x87_inst_ptr & 0xffff);
			WRITE16(ea + 8, (m_x87_opcode & 0x07ff) | ((m_x87_inst_ptr & 0x0f0000) >> 4));
			WRITE16(ea + 10, m_x87_data_ptr & 0xffff);
			WRITE16(ea + 12, (m_x87_data_ptr & 0x0f0000) >> 4);
			break;
		case 1: // 16-bit protected mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
			WRITE16(ea + 6, m_x87_inst_ptr & 0xffff);
			WRITE16(ea + 8, m_x87_cs);
			WRITE16(ea + 10, m_x87_data_ptr & 0xffff);
			WRITE16(ea + 12, m_x87_ds);
			break;
		case 2: // 32-bit real mode
			WRITE32(ea + 0, 0xffff0000 | m_x87_cw);
			WRITE32(ea + 4, 0xffff0000 | m_x87_sw);
			WRITE32(ea + 8, 0xffff0000 | m_x87_tw);
			WRITE32(ea + 12, 0xffff0000 | (m_x87_inst_ptr & 0xffff));
			WRITE32(ea + 16, (m_x87_opcode & 0x07ff) | ((m_x87_inst_ptr & 0xffff0000) >> 4));
			WRITE32(ea + 20, 0xffff0000 | (m_x87_data_ptr & 0xffff));
			WRITE32(ea + 24, (m_x87_data_ptr & 0xffff0000) >> 4);
			break;
		case 3: // 32-bit protected mode
			WRITE32(ea + 0,  0xffff0000 | m_x87_cw);
			WRITE32(ea + 4,  0xffff0000 | m_x87_sw);
			WRITE32(ea + 8,  0xffff0000 | m_x87_tw);
			WRITE32(ea + 12, m_x87_inst_ptr);
			WRITE32(ea + 16, (m_x87_opcode << 16) | m_x87_cs);
			WRITE32(ea + 20, m_x87_data_ptr);
			WRITE32(ea + 24, 0xffff0000 | m_x87_ds);
			break;
	}
	m_x87_cw |= 0x3f;   // set all masks

	CYCLES((m_cr[0] & CR0_PE) ? 56 : 67);
}

void i386_device::x87_fsave(uint8_t modrm)
{
	uint32_t ea = GetEA(modrm, 1);

	switch(((PROTECTED_MODE && !V8086_MODE) ? 1 : 0) | (m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
			WRITE16(ea + 6, m_x87_inst_ptr & 0xffff);
			WRITE16(ea + 8, (m_x87_opcode & 0x07ff) | ((m_x87_inst_ptr & 0x0f0000) >> 4));
			WRITE16(ea + 10, m_x87_data_ptr & 0xffff);
			WRITE16(ea + 12, (m_x87_data_ptr & 0x0f0000) >> 4);
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			WRITE16(ea + 0, m_x87_cw);
			WRITE16(ea + 2, m_x87_sw);
			WRITE16(ea + 4, m_x87_tw);
			WRITE16(ea + 6, m_x87_inst_ptr & 0xffff);
			WRITE16(ea + 8, m_x87_cs);
			WRITE16(ea + 10, m_x87_data_ptr & 0xffff);
			WRITE16(ea + 12, m_x87_ds);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			WRITE32(ea + 0, 0xffff0000 | m_x87_cw);
			WRITE32(ea + 4, 0xffff0000 | m_x87_sw);
			WRITE32(ea + 8, 0xffff0000 | m_x87_tw);
			WRITE32(ea + 12, 0xffff0000 | (m_x87_inst_ptr & 0xffff));
			WRITE32(ea + 16, (m_x87_opcode & 0x07ff) | ((m_x87_inst_ptr & 0xffff0000) >> 4));
			WRITE32(ea + 20, 0xffff0000 | (m_x87_data_ptr & 0xffff));
			WRITE32(ea + 24, (m_x87_data_ptr & 0xffff0000) >> 4);
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			WRITE32(ea + 0,  0xffff0000 | m_x87_cw);
			WRITE32(ea + 4,  0xffff0000 | m_x87_sw);
			WRITE32(ea + 8,  0xffff0000 | m_x87_tw);
			WRITE32(ea + 12, m_x87_inst_ptr);
			WRITE32(ea + 16, (m_x87_opcode << 16) | m_x87_cs);
			WRITE32(ea + 20, m_x87_data_ptr);
			WRITE32(ea + 24, 0xffff0000 | m_x87_ds);
			ea += 28;
			break;
	}

	for (int i = 0; i < 8; ++i)
		WRITE80(ea + i*10, ST(i));
	x87_reset();

	CYCLES((m_cr[0] & CR0_PE) ? 56 : 67);
}

void i386_device::x87_frstor(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	uint32_t ea = GetEA(modrm, 0);
	uint32_t temp;

	switch(((PROTECTED_MODE && !V8086_MODE) ? 1 : 0) | (m_operand_size & 1)<<1)
	{
		case 0: // 16-bit real mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 2);
			m_x87_tw = READ16(ea + 4);
			m_x87_inst_ptr = READ16(ea + 6);
			temp = READ16(ea + 8);
			m_x87_opcode = temp & 0x7ff;
			m_x87_inst_ptr |= ((temp & 0xf000) << 4);
			m_x87_data_ptr = READ16(ea + 10) | ((READ16(ea + 12) & 0xf000) << 4);
			m_x87_cs = 0;
			m_x87_ds = 0;
			ea += 14;
			break;
		case 1: // 16-bit protected mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 2);
			m_x87_tw = READ16(ea + 4);
			m_x87_inst_ptr = READ16(ea + 6);
			m_x87_opcode = 0;
			m_x87_cs = READ16(ea + 8);
			m_x87_data_ptr = READ16(ea + 10);
			m_x87_ds = READ16(ea + 12);
			ea += 14;
			break;
		case 2: // 32-bit real mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 4);
			m_x87_tw = READ16(ea + 8);
			m_x87_inst_ptr = READ16(ea + 12);
			temp = READ32(ea + 16);
			m_x87_opcode = temp & 0x7ff;
			m_x87_inst_ptr |= ((temp & 0xffff000) << 4);
			m_x87_data_ptr = READ16(ea + 20) | ((READ32(ea + 24) & 0xffff000) << 4);
			m_x87_cs = 0;
			m_x87_ds = 0;
			ea += 28;
			break;
		case 3: // 32-bit protected mode
			x87_write_cw(READ16(ea));
			m_x87_sw = READ16(ea + 4);
			m_x87_tw = READ16(ea + 8);
			m_x87_inst_ptr = READ32(ea + 12);
			temp = READ32(ea + 16);
			m_x87_opcode = (temp >> 16) & 0x7ff;
			m_x87_cs = temp & 0xffff;
			m_x87_data_ptr = READ32(ea + 20);
			m_x87_ds = READ16(ea + 24);
			ea += 28;
			break;
	}

	for (int i = 0; i < 8; ++i)
		x87_write_stack(i, READ80(ea + i*10), false);

	CYCLES((m_cr[0] & CR0_PE) ? 34 : 44);
}

void i386_device::x87_fxch(uint8_t modrm)
{
	if (x87_mf_fault())
		return;
	if (X87_IS_ST_EMPTY(0) || X87_IS_ST_EMPTY(1))
		x87_set_stack_underflow();

	if (x87_check_exceptions())
	{
		extFloat80_t tmp = ST(0);
		ST(0) = ST(1);
		ST(1) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		x87_set_tag(ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(1)));
		x87_set_tag(ST_TO_PHYS(1), tag0);
	}

	CYCLES(4);
}

void i386_device::x87_fxch_sti(uint8_t modrm)
{
	int i = modrm & 7;

	if (x87_mf_fault())
		return;
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
		extFloat80_t tmp = ST(0);
		ST(0) = ST(i);
		ST(i) = tmp;

		// Swap the tags
		int tag0 = X87_TAG(ST_TO_PHYS(0));
		x87_set_tag(ST_TO_PHYS(0), X87_TAG(ST_TO_PHYS(i)));
		x87_set_tag(ST_TO_PHYS(i), tag0);
	}

	CYCLES(4);
}

void i386_device::x87_fstsw_ax(uint8_t modrm)
{
	REG16(AX) = m_x87_sw;

	CYCLES(3);
}

void i386_device::x87_fstsw_m2byte(uint8_t modrm)
{
	uint32_t ea = GetEA(modrm, 1);

	WRITE16(ea, m_x87_sw);

	CYCLES(3);
}

void i386_device::x87_invalid(uint8_t modrm)
{
	// TODO
	report_invalid_opcode();
	i386_trap(6, 0);
}



/*************************************
 *
 * Instruction dispatch
 *
 *************************************/

void i386_device::i386_x87_group_d8()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_d8[modrm])(modrm);
}

void i386_device::i386_x87_group_d9()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_d9[modrm])(modrm);
}

void i386_device::i386_x87_group_da()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_da[modrm])(modrm);
}

void i386_device::i386_x87_group_db()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_db[modrm])(modrm);
}

void i386_device::i386_x87_group_dc()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_dc[modrm])(modrm);
}

void i386_device::i386_x87_group_dd()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_dd[modrm])(modrm);
}

void i386_device::i386_x87_group_de()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
	(this->*m_opcode_table_x87_de[modrm])(modrm);
}

void i386_device::i386_x87_group_df()
{
	if (m_cr[0] & (CR0_TS | CR0_EM))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	uint8_t modrm = FETCH();
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
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: ptr = &i386_device::x87_fstp_sti;     break;
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


