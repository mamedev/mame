// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Hegener + Glaser Mephisto Display Module for modular chesscomputers, the 1st
version with a 4*7seg LCD. There is no LCD chip, it's handled with 4 4015 dual
shift registers.

*******************************************************************************/

#include "emu.h"
#include "mmdisplay1.h"


DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE1, mephisto_display1_device, "mdisplay1", "Mephisto Display Module 1")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

mephisto_display1_device::mephisto_display1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MEPHISTO_DISPLAY_MODULE1, tag, owner, clock),
	m_digits(*this, "digit%u", 0U),
	m_output_digit(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_display1_device::device_start()
{
	if (m_output_digit.isunset())
		m_digits.resolve();

	// initialize
	m_strobe = 1;
	m_digit_data = ~0;

	// register for savestates
	save_item(NAME(m_strobe));
	save_item(NAME(m_digit_data));
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void mephisto_display1_device::update_lcd()
{
	for (int i = 0; i < 4; i++)
	{
		u8 digit = m_digit_data >> (i * 8) ^ (m_strobe ? 0 : 0xff);

		if (m_output_digit.isunset())
			m_digits[i] = digit;
		else
			m_output_digit(i, digit);
	}
}

void mephisto_display1_device::strobe_w(int state)
{
	state = state ? 1 : 0;

	// update lcd on any edge
	if (state != m_strobe)
	{
		m_strobe = state;
		update_lcd();
	}
}

void mephisto_display1_device::data_w(u8 data)
{
	m_digit_data = (m_digit_data >> 8) | (data << 24);
}
