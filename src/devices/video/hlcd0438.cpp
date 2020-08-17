// license:BSD-3-Clause
// copyright-holders:hap
/*

Hughes HLCD 0438 LCD Driver
32 segment outputs, may also be used as a column driver

TODO:
- OSC (LCD phi pin)

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
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0438_device::device_start()
{
	m_write_segs.resolve_safe();
	m_write_data.resolve_safe();

	// register for savestates
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_clk));
	save_item(NAME(m_load));
	save_item(NAME(m_shift));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void hlcd0438_device::update_output()
{
	// load output latches while LOAD pin is high
	if (m_load)
		m_write_segs(0, m_shift);
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

		// output
		m_write_data(m_data_out);
		update_output();
	}

	m_clk = state;
}
