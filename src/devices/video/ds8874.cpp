// license:BSD-3-Clause
// copyright-holders:hap
/*

National Semiconductor DS8874 9-Digit Shift Input LED Driver

It's basically a 9-bit generic shifter, outputs are active-low.
Only one output is active at a time.

TODO:
- add 1-bit callbacks/read functions if necessary
- datasheet info is very sparse, so implementation may have errors
- low battery pin

*/

#include "emu.h"
#include "ds8874.h"


DEFINE_DEVICE_TYPE(DS8874, ds8874_device, "ds8874", "DS8874 LED Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

ds8874_device::ds8874_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, DS8874, tag, owner, clock),
	m_write_output(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds8874_device::device_start()
{
	// zerofill
	m_data = 0;
	m_cp = 0;
	m_shift = 0xff;

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_cp));
	save_item(NAME(m_shift));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void ds8874_device::refresh_output()
{
	m_write_output(m_shift);
}

void ds8874_device::data_w(int state)
{
	state = (state) ? 1 : 0;

	// reset shift register at falling edge
	if (!state && m_data)
	{
		m_shift = 0x1fe;
		refresh_output();
	}

	m_data = state;
}

void ds8874_device::cp_w(int state)
{
	state = (state) ? 1 : 0;

	// clock shift register at any edge
	if (state != m_cp && m_shift != 0xff)
	{
		m_shift = (m_shift << 1 & 0x1ff) | 1;
		refresh_output();
	}

	m_cp = state;
}
