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
	m_lcd_pwm(*this, "lcd_pwm"),
	m_digits(*this, "digit%u", 0U),
	m_output_digit(*this)
{ }


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void mephisto_display1_device::device_add_mconfig(machine_config &config)
{
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 8);
	m_lcd_pwm->set_segmask(0xf, 0xff);
	m_lcd_pwm->set_bri_levels(0.75);
	m_lcd_pwm->output_digit().set(FUNC(mephisto_display1_device::lcd_pwm_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_display1_device::device_start()
{
	if (m_output_digit.isunset())
		m_digits.resolve();

	// initialize
	m_common = 0;
	m_digit_data = 0;

	// register for savestates
	save_item(NAME(m_common));
	save_item(NAME(m_digit_data));
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void mephisto_display1_device::lcd_pwm_w(offs_t offset, u64 data)
{
	if (m_output_digit.isunset())
		m_digits[offset] = data;
	else
		m_output_digit(offset, data);
}

void mephisto_display1_device::update_lcd()
{
	for (int i = 0; i < 4; i++)
	{
		const u32 data = m_common ? ~m_digit_data : m_digit_data;
		m_lcd_pwm->write_row(i, data >> (i * 8) & 0xff);
	}
}

void mephisto_display1_device::common_w(int state)
{
	m_common = state ? 1 : 0;
	update_lcd();
}

void mephisto_display1_device::data_w(u8 data)
{
	// writing data also clocks shift registers
	m_digit_data = (m_digit_data >> 8) | (data << 24);
	update_lcd();
}
