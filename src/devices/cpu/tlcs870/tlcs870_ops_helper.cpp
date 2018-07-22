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
	m_RBS = data & 0xf0;
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


uint16_t tlcs870_device::do_alu(int op, uint16_t param1, uint16_t param2)
{
	uint16_t result = 0x00;

	// TODO: flags
	switch (op)
	{
	case 0x0: // ADDC
		result = param1 + param2;
		result += is_CF();
		break;

	case 0x1: // ADD
		result = param1 + param2;
		break;

	case 0x2: // SUBB
		result = param1 - param2;
		result -= is_CF();
		break;

	case 0x3: // SUB
		result = param1 - param2;
		break;

	case 0x4: // AND
		result = param1 & param2;
		break;

	case 0x5: // XOR
		result = param1 ^ param2;
		break;

	case 0x6: // OR
		result = param1 | param2;
		break;

	case 0x7: // CMP
		if (param1 < param2)
		{
			set_CF();
		}
		else
		{
			clear_CF();
		}
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

/*

(Priority Low - 15)
FFE0 INT5    (External Interrupt 5) 
FFE2 INTTC2  (16-bit TC2 Interrupt)
FFE4 INTSIO2 (Serial Interface 2 Interrupt)
FFE6 INT4    (External Interrupt 4)
FFE8 INT3    (External Interrupt 3)
FFEA INTTC4  (8-bit TC4 Interrupt)
FFEC INTSIO1 (Serial Interface 1 Interrupt)
FFEE INTTC3  (8-bit TC3 Interrupt)
FFF0 INT2    (External Interrupt 2)
FFF2 INTTBT  (Time Base Timer Interrupt)
FFF4 INT1    (External Interrupt 1)
FFF6 INTTC1  (16-bit TC1 Interrupt)
FFF8 INT0    (External Interrupt 0)
FFFA INTWDT  (Watchdog Timer Interrupt)
FFFC INTSW   (Software Interrupt)
FFFE RESET   (Reset Vector)
(Priority High - 0)

*/

void tlcs870_device::handle_take_interrupt(int level)
{
	WM8(m_sp.d - 1, get_PSW());
	WM16(m_sp.d - 2, m_addr);
	m_sp.d -= 3;

	m_addr = RM16(0xffe0 + ((level &0xf)*2));
}

