// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

*************************************************************************************************************/

#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"
#include "debugger.h"


// General Helpers

const uint8_t tlcs870_device::get_reg8(const int reg)
{
	return m_intram[((m_RBS & 0xf) * 8) + (reg & 0x7)];
}

void tlcs870_device::set_reg8(const int reg, uint8_t val)
{
	m_intram[((m_RBS & 0xf) * 8) + (reg & 0x7)] = val;
}

const uint16_t tlcs870_device::get_reg16(const int reg)
{
	uint16_t res = 0;
	res |= get_reg8(((reg & 0x3) * 2) + 1) << 8;
	res |= get_reg8(((reg & 0x3) * 2) + 0) << 0;
	return res;
}

void tlcs870_device::set_reg16(const int reg, uint16_t val)
{
	set_reg8(((reg & 0x3) * 2) + 1, (val & 0xff00) >> 8);
	set_reg8(((reg & 0x3) * 2) + 0, (val & 0x00ff) >> 0);
}

const uint8_t tlcs870_device::get_PSW()
{
	return (m_F & 0xf0) | (m_RBS & 0x0f);
}

void tlcs870_device::set_PSW(uint8_t data)
{
	// used by the push/pop opcodes, flags can't be written by memory access
	m_F = data & 0xf0;
	m_RBS = data & 0x0f;
}

void tlcs870_device::handle_div(const int reg)
{
	const uint16_t temp16 = get_reg16(reg);
	const uint8_t temp8 = get_reg8(REG_C);

	if (!temp8)
	{
		// divide by zero
		set_CF();
		// does the ZF change too?
	}
	else
	{
		const uint16_t tempres16 = temp16 / temp8;
		const uint8_t tempres8 = temp16 % temp8;

		const uint16_t tempfull = (tempres8 << 8) | (tempres16 & 0x00ff);

		set_reg16(reg, tempfull);

		if (tempres16 & 0xff00)
		{
			// result is also 'undefined' in this case
			set_CF();
		}
		else
		{
			clear_CF();
		}

		if (!tempres8)
		{
			set_ZF();
			set_JF();
		}
		else
		{
			clear_ZF();
			clear_JF();
		}
	}
}

void tlcs870_device::handle_mul(const int reg)
{
	const uint16_t temp16 = get_reg16(reg);
	const uint16_t tempres = (temp16 & 0xff) * ((temp16 & 0xff00) >> 8);
	set_reg16(reg, temp16);

	if (!(tempres & 0xff00))
	{
		set_ZF();
		set_JF();
	}
	else
	{
		clear_ZF();
		clear_JF();
	}
}

void tlcs870_device::handle_swap(const int reg)
{
	uint8_t temp = get_reg8(reg);
	temp = ((temp & 0x0f) << 4) | ((temp & 0xf0) >> 4);
	set_reg8(reg, temp);

	set_JF();
}


uint16_t tlcs870_device::get_addr(uint16_t opbyte0, uint16_t val)
{
	uint16_t addr = 0x0000;

	switch (opbyte0)
	{
	case ADDR_IN_IMM_X:
		addr = val;
		break;
	case ADDR_IN_PC_PLUS_REG_A:
		addr = m_tmppc + 2 + get_reg8(REG_A);
		break;
	case ADDR_IN_DE:
		addr = get_reg16(REG_DE);
		break;
	case ADDR_IN_HL:
		addr = get_reg16(REG_HL);
		break;
	case ADDR_IN_HL_PLUS_IMM_D:
		addr = get_reg16(REG_HL) + val;
		break;
	case ADDR_IN_HL_PLUS_REG_C:
		addr = get_reg16(REG_HL) + get_reg8(REG_C);
		break;
	case ADDR_IN_HLINC:
	{
		uint16_t tmpHL = get_reg16(REG_HL);
		addr = tmpHL;
		tmpHL++;
		set_reg16(REG_HL, tmpHL);
		break;
	}
	case ADDR_IN_DECHL:
	{
		uint16_t tmpHL = get_reg16(REG_HL);
		tmpHL--;
		set_reg16(REG_HL, tmpHL);
		addr = tmpHL;
		break;
	}
	}

	return addr;
}

const bool tlcs870_device::check_jump_condition(int param1)
{
	bool takejump = true;

	switch (param1)
	{
	case COND_EQ_Z:
		if (is_ZF() == 1) takejump = true;
		else takejump = false;
		break;

	case COND_NE_NZ:
		if (is_ZF() == 0) takejump = true;
		else takejump = false;
		break;

	case COND_LT_CS:
		if (is_CF() == 1) takejump = true;
		else takejump = false;
		break;

	case COND_GE_CC:
		if (is_CF() == 0) takejump = true;
		else takejump = false;
		break;

	case COND_LE:
		if ((is_CF() || is_ZF()) == 1) takejump = true;
		else takejump = false;
		break;

	case COND_GT:
		if ((is_CF() || is_ZF()) == 0) takejump = true;
		else takejump = false;
		break;

	case COND_T:
		if (is_JF() == 1) takejump = true;
		else takejump = false;
		break;

	case COND_F:
		if (is_JF() == 0) takejump = true;
		else takejump = false;
		break;
	}

	return takejump;
}

/*
    All 16-bit ALU ops that would set the 'H' flag list the behavior as undefined.
    Logically the half flag would be the 8/9 bit carry (usual C flag) in a 16-bit
    op, but since this isn't listed as being the case there's a chance the behavior
    is something unexpected, such as still using the 3/4 carry, or, if it's
    internally handled as 4 4-bit operations, maybe the 12/13 bit carry

    This needs testing on hardware.

    (8-bit)        JF ZF CF HF
    ADDC           C  Z  C  H
    ADD            C  Z  C  H
    SUBB           C  Z  C  H
    SUB            C  Z  C  H
    AND            Z  Z  -  -
    XOR            Z  Z  -  -
    OR             Z  Z  -  -
    CMP            Z  Z  C  H

    (16-bit)
    ADDC           C  Z  C  U
    ADD            C  Z  C  U
    SUBB           C  Z  C  U
    SUB            C  Z  C  U
    AND            Z  Z  -  -
    XOR            Z  Z  -  -
    OR             Z  Z  -  -
    CMP            Z  Z  C  U

*/

uint8_t tlcs870_device::do_add_8bit(uint16_t param1, uint16_t param2)
{
	uint16_t result = param1 + param2;

	if (result & 0x100)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	if ((result & 0xff) == 0x00)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	uint8_t temp = (param1 & 0xf) + (param2 & 0xf);

	if (temp & 0x10)
	{
		set_HF();
	}
	else
	{
		clear_HF();
	}

	// JF is copied from CF
	is_ZF() ? set_CF() : clear_CF();

	return result;
}

uint16_t tlcs870_device::do_add_16bit(uint32_t param1, uint32_t param2)
{
	uint32_t result = param1 + param2;

	if (result & 0x10000)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	if ((result & 0xffff) == 0x00)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// unknown, manual says undefined, see note above
	uint16_t temp = (param1 & 0xff) + (param2 & 0xff);

	if (temp & 0x100)
	{
		set_HF();
	}
	else
	{
		clear_HF();
	}

	// JF is copied from CF
	is_ZF() ? set_CF() : clear_CF();

	return result;
}

uint8_t tlcs870_device::do_sub_8bit(uint16_t param1, uint16_t param2)
{
	uint16_t result = param1 - param2;

	if (param1 < param2)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	if ((param1 & 0xf) < (param2 & 0xf))
	{
		set_HF();
	}
	else
	{
		clear_HF();
	}

	if ((result & 0xff) == 0x00)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from CF
	is_ZF() ? set_CF() : clear_CF();

	return result;
}


uint16_t tlcs870_device::do_sub_16bit(uint32_t param1, uint32_t param2)
{
	uint32_t result = param1 - param2;

	if (param1 < param2)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	// unknown, manual says undefined, see note above
	if ((param1 & 0xff) < (param2 & 0xff))
	{
		set_HF();
	}
	else
	{
		clear_HF();
	}

	if ((result & 0xffff) == 0x0000)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from CF
	is_ZF() ? set_CF() : clear_CF();

	return result;
}

void tlcs870_device::do_cmp_8bit(uint16_t param1, uint16_t param2)
{
	if (param1 < param2)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	if ((param1 & 0xf) < (param2 & 0xf)) // see note above about half flag
	{
		set_HF();
	}
	else
	{
		clear_HF();
	}

	if (param1 == param2)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from ZF
	is_ZF() ? set_JF() : clear_JF();
}

void tlcs870_device::do_cmp_16bit(uint32_t param1, uint32_t param2)
{
	if (param1 < param2)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	// unknown, manual says undefined, see note above
	if ((param1 & 0xff) < (param2 & 0xff))
	{
		set_HF();
	}
	else
	{
		clear_HF();
	}

	if (param1 == param2)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from ZF
	is_ZF() ? set_JF() : clear_JF();
}


uint16_t tlcs870_device::do_and(uint16_t param1, uint16_t param2)
{
	uint16_t result = param1 & param2;

	if (result == 0x00)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from ZF
	is_ZF() ? set_JF() : clear_JF();

	return result;
}

uint16_t tlcs870_device::do_xor(uint16_t param1, uint16_t param2)
{
	uint16_t result = param1 ^ param2;

	if (result == 0x00)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from ZF
	is_ZF() ? set_JF() : clear_JF();

	return result;
}

uint16_t tlcs870_device::do_or(uint16_t param1, uint16_t param2)
{
	uint16_t result = param1 | param2;

	if (result == 0x00)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	// JF is copied from ZF
	is_ZF() ? set_JF() : clear_JF();

	return result;
}

uint8_t tlcs870_device::do_alu_8bit(int op, uint16_t param1, uint16_t param2)
{
	uint16_t result = 0x00;

	switch (op)
	{
	case 0x0: // ADDC
		param2 += is_CF();
		result = do_add_8bit(param1, param2);
		break;

	case 0x1: // ADD
		result = do_add_8bit(param1, param2);
		break;

	case 0x2: // SUBB
		param2 += is_CF();
		result = do_sub_8bit(param1, param2);
		break;

	case 0x3: // SUB
		result = do_sub_8bit(param1, param2);
		break;

	case 0x4: // AND
		result = do_and(param1, param2);
		break;

	case 0x5: // XOR
		result = do_xor(param1, param2);
		break;

	case 0x6: // OR
		result = do_or(param1, param2);
		break;

	case 0x7: // CMP
		do_cmp_8bit(param1, param2);
		break;
	}

	return result;
}

uint16_t tlcs870_device::do_alu_16bit(int op, uint32_t param1, uint32_t param2)
{
	uint32_t result = 0x0000;

	switch (op)
	{
	case 0x0: // ADDC
		param2 += is_CF();
		result = do_add_16bit(param1, param2);
		break;

	case 0x1: // ADD
		result = do_add_16bit(param1, param2);
		break;

	case 0x2: // SUBB
		param2 += is_CF();
		result = do_sub_16bit(param1, param2);
		break;

	case 0x3: // SUB
		result = do_sub_16bit(param1, param2);
		break;

	case 0x4: // AND
		result = do_and(param1, param2);
		break;

	case 0x5: // XOR
		result = do_xor(param1, param2);
		break;

	case 0x6: // OR
		result = do_or(param1, param2);
		break;

	case 0x7: // CMP
		do_cmp_16bit(param1, param2);
		break;
	}

	return result;
}

uint8_t tlcs870_device::handle_SHLC(uint8_t val)
{
	if (val & 0x80)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	val = (val << 1);

	// JF gets set to CF
	if (is_CF())
	{
		set_JF();
	}
	else
	{
		clear_JF();
	}

	if (val == 0)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	return val;
}


uint8_t tlcs870_device::handle_SHRC(uint8_t val)
{
	if (val & 0x01)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	val = (val >> 1);

	// JF gets set to CF
	if (is_CF())
	{
		set_JF();
	}
	else
	{
		clear_JF();
	}

	if (val == 0)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	return val;
}

uint8_t tlcs870_device::handle_DAS(uint8_t val)
{
	if (((val & 0x0f) > 9) || (is_HF() == 1))
	{
		val = val - 0x06;
		set_HF();
	}
	else
	{
		clear_HF();
	}

	if ((val > 0x9f) || (is_CF() == 1))
	{
		val = val - 0x60;
		set_CF();
	}
	else
	{
		clear_CF();
	}

	return val;
}

uint8_t tlcs870_device::handle_DAA(uint8_t val)
{
	if (((val & 0x0f) > 9) || (is_HF() == 1))
	{
		val = val + 0x06;
		set_HF();
	}
	else
	{
		clear_HF();
	}

	if ((val > 0x9f) || (is_CF() == 1))
	{
		val = val + 0x60;
		set_CF();
	}
	else
	{
		clear_CF();
	}

	return val;
}

uint8_t tlcs870_device::handle_ROLC(uint8_t val)
{
	const int tempcarry = is_CF();

	if (val & 0x80)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	val = (val << 1) | tempcarry;

	// JF gets set to CF
	if (is_CF())
	{
		set_JF();
	}
	else
	{
		clear_JF();
	}

	if (val == 0)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	return val;
}

uint8_t tlcs870_device::handle_RORC(uint8_t val)
{
	const int tempcarry = (is_CF()) << 7;

	if (val & 0x01)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	val = (val >> 1) | tempcarry;

	// JF gets set to CF
	if (is_CF())
	{
		set_JF();
	}
	else
	{
		clear_JF();
	}

	if (val == 0)
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	return val;
}
