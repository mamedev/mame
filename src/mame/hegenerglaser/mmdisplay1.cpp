// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

Hegener + Glaser Mephisto Display Module for modular chesscomputers,
the 1st version with a 4*7seg LCD. There is no LCD chip, it's handled
with 4 4015 dual shift registers.

*********************************************************************/

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
	m_output_digit.resolve();
	if (m_output_digit.isnull())
		m_digits.resolve();

	// register for savestates
	save_item(NAME(m_strobe));
	save_item(NAME(m_digit_idx));
	save_item(NAME(m_digit_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_display1_device::device_reset()
{
	m_strobe = 0;
	m_digit_idx = 0;

	// clear display
	for (int i = 0; i < 4; i++)
		m_digit_data[i] = 0;
	update_lcd();
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void mephisto_display1_device::update_lcd()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_output_digit.isnull())
			m_digits[i] = m_digit_data[i];
		else
			m_output_digit(i, m_digit_data[i]);
	}
}

void mephisto_display1_device::strobe_w(int state)
{
	state = state ? 1 : 0;

	// update lcd on any edge
	if (state != m_strobe)
		update_lcd();

	m_strobe = state;
}

void mephisto_display1_device::data_w(u8 data)
{
	m_digit_data[m_digit_idx] = m_strobe ? ~data : data;
	m_digit_idx = (m_digit_idx + 1) & 3;
}
