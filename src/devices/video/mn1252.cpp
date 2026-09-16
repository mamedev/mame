// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

Panasonic MN1252 LCD controller

***************************************************************************/

#include "emu.h"
#include "mn1252.h"

#include <cassert>


DEFINE_DEVICE_TYPE(MN1252, mn1252_device, "mn1252", "Panasonic MN1252 LCD controller")

const u8 mn1252_device::OUTPUT_DIGITS[0x40] =
{
	// bit 0..7 = segment a..h (from datasheet)
	0x00, 0x77, 0x7f, 0x39, 0x3f, 0x79, 0x71, 0x3d,
	0x76, 0x06, 0x1e, 0xf0, 0x38, 0xb7, 0xb6, 0xbf,
	0x73, 0xbf, 0xf3, 0x6d, 0x07, 0x3e, 0xa6, 0xbe,
	0xf2, 0x6e, 0x5b, 0x48, 0x0f, 0x46, 0x49, 0x44,
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
	0x7c, 0x58, 0x5e, 0x54, 0x5c, 0x40, 0x21, 0x0c,
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
	0x7f, 0x6f, 0x01, 0x36, 0x5f, 0x7b, 0x74, 0x62
};

mn1252_device::mn1252_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MN1252, tag, owner, clock)
{
}

/**************************************************************************/
void mn1252_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_first_nibble));
	save_item(NAME(m_nibble_count));
	save_item(NAME(m_ce));
	save_item(NAME(m_std));
	save_item(NAME(m_output));
}

/**************************************************************************/
void mn1252_device::device_reset()
{
	m_data = m_first_nibble = 0;
	m_nibble_count = 0;
	m_ce = m_std = 0;

	std::fill(std::begin(m_output), std::end(m_output), 0);
}

/**************************************************************************/
u16 mn1252_device::output(offs_t digit) const
{
	assert(digit < 6);
	return m_output[digit];
}

/**************************************************************************/
void mn1252_device::data_w(u8 data)
{
	m_data = data & 0xf;
}

/**************************************************************************/
void mn1252_device::ce_w(int state)
{
	if (!m_ce && state)
	{
		m_nibble_count = 0;
	}

	m_ce = state;
}

/**************************************************************************/
void mn1252_device::std_w(int state)
{
	if (m_ce && m_std && !state && m_nibble_count < 12)
	{
		if (!(m_nibble_count % 2))
		{
			m_first_nibble = m_data;
		}
		else
		{
			const u8 data = (m_first_nibble << 4) | m_data;
			u16 output = OUTPUT_DIGITS[data & 0x3f];
			if (BIT(data, 6)) output |= 0x80;  // segment h
			if (BIT(data, 7)) output |= 0x100; // segment p

			m_output[m_nibble_count / 2] = output;
		}

		m_nibble_count++;
	}

	m_std = state;
}
