// license:BSD-3-Clause
// copyright-holders:Sean Gonsalves
/***************************************************************************

    Konami 007452 multiplier/divider

***************************************************************************/

#include "emu.h"
#include "k007452.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(KONAMI_007452_MATH, k007452_device, "konami_007452", "Konami 007452 math chip")

//-------------------------------------------------
//  k007452_device - constructor
//-------------------------------------------------

k007452_device::k007452_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KONAMI_007452_MATH, tag, owner, clock)
{
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

u8 k007452_device::read(offs_t offset)
{
	switch (offset & 7)
	{
		case 0: return u8(m_multiply_result & 0xff);
		case 1: return u8((m_multiply_result >> 8) & 0xff);
		case 2: return u8(m_divide_remainder & 0xff);
		case 3: return u8((m_divide_remainder >> 8) & 0xff);
		case 4: return u8(m_divide_quotient & 0xff);
		case 5: return u8((m_divide_quotient >> 8) & 0xff);
		default: return 0;
	}
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

void k007452_device::write(offs_t offset, u8 data)
{
	if (offset < 6)
		m_math_regs[offset] = data;

	if (offset == 1)
	{
		// Starts multiplication process
		m_multiply_result = u16(m_math_regs[0]) * m_math_regs[1];
	}
	else if (offset == 5)
	{
		// Starts division process
		u16 const dividend = (u16(m_math_regs[4]) << 8) | m_math_regs[5];
		u16 const divisor = (u16(m_math_regs[2]) << 8) | m_math_regs[3];
		if (!divisor)
		{
			m_divide_quotient = 0xffff;
			m_divide_remainder = 0x0000;
		}
		else
		{
			m_divide_quotient = dividend / divisor;
			m_divide_remainder = dividend % divisor;
		}
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007452_device::device_start()
{
	save_item(NAME(m_math_regs));
	save_item(NAME(m_multiply_result));
	save_item(NAME(m_divide_quotient));
	save_item(NAME(m_divide_remainder));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k007452_device::device_reset()
{
	std::fill(std::begin(m_math_regs), std::end(m_math_regs), 0);
	m_multiply_result = 0;
	m_divide_quotient = 0;
	m_divide_remainder = 0;
}
