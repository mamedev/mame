// license:BSD-3-Clause
// copyright-holders:hap
/*

Hughes HLCD 0438 LCD Driver
32 segment outputs, may also be used as a column driver

LCD pin can be driven manually, or oscillating.

*/

#include "emu.h"
#include "video/hlcd0438.h"


DEFINE_DEVICE_TYPE(HLCD0438, hlcd0438_device, "hlcd0438", "Hughes HLCD 0438 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hlcd0438_device::hlcd0438_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, HLCD0438, tag, owner, clock),
	m_write_segs(*this), m_write_data(*this)
{
	m_load = 0;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0438_device::device_start()
{
	// resolve callbacks
	m_write_segs.resolve_safe();
	m_write_data.resolve_safe();

	// timer (when LCD pin is oscillator)
	m_lcd_timer = timer_alloc(FUNC(hlcd0438_device::toggle_lcd), this);
	attotime period = (clock() != 0) ? attotime::from_hz(2 * clock()) : attotime::never;
	m_lcd_timer->adjust(period, 0, period);

	// zerofill
	m_data_in = 0;
	m_data_out = 0;
	m_clk = 0;
	// m_load // not here
	m_lcd = 0;
	m_shift = 0;
	m_latch = 0;

	// register for savestates
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_clk));
	save_item(NAME(m_load));
	save_item(NAME(m_lcd));
	save_item(NAME(m_shift));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(hlcd0438_device::toggle_lcd)
{
	lcd_w(!m_lcd);
}

void hlcd0438_device::clock_w(int state)
{
	state = state ? 1 : 0;

	// shift on falling edge
	if (m_clk && !state)
	{
		// DATA OUT pin follows carry out
		m_data_out = BIT(m_shift, 31);

		m_shift = m_shift << 1 | m_data_in;

		m_write_data(m_data_out);
		load_w(m_load);
	}

	m_clk = state;
}

void hlcd0438_device::load_w(int state)
{
	m_load = state ? 1 : 0;

	// load to output latches while LOAD pin is high
	if (m_load)
		m_latch = m_shift;
}

void hlcd0438_device::lcd_w(int state)
{
	state = state ? 1 : 0;

	// LCD pin drives backplate
	if (state != m_lcd)
		m_write_segs(m_lcd, m_latch);

	m_lcd = state;
}
