// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

**********************************************************************/

#include "cdp1852.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CDP1852 = &device_creator<cdp1852_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	MODE_INPUT = 0,
	MODE_OUTPUT
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1852_device - constructor
//-------------------------------------------------

cdp1852_device::cdp1852_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CDP1852, "CDP1852 I/O", tag, owner, clock, "cdp1852", __FILE__),
	m_read_mode(*this),
	m_write_sr(*this),
	m_read_data(*this),
	m_write_data(*this), m_new_data(0), m_data(0), m_next_data(0), m_sr(0), m_next_sr(0), m_scan_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1852_device::device_start()
{
	// resolve callbacks
	m_read_mode.resolve_safe(0);
	m_write_sr.resolve_safe();
	m_read_data.resolve_safe(0);
	m_write_data.resolve_safe();

	// allocate timers
	if (clock() > 0)
	{
		m_scan_timer = timer_alloc();
		m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));
	}

	// register for state saving
	save_item(NAME(m_new_data));
	save_item(NAME(m_data));
	save_item(NAME(m_next_data));
	save_item(NAME(m_sr));
	save_item(NAME(m_next_sr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1852_device::device_reset()
{
	// reset data register
	m_data = 0;

	if (!m_read_mode())
	{
		// reset service request flip-flop
		set_sr_line(1);
	}
	else
	{
		// output data
		m_write_data((offs_t)0, m_data);

		// reset service request flip-flop
		set_sr_line(0);
	}
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cdp1852_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (!m_read_mode())
	{
		// input data into register
		m_data = m_read_data(0);

		// signal processor
		set_sr_line(0);
	}
	else
	{
		if (m_new_data)
		{
			m_new_data = 0;

			// latch data into register
			m_data = m_next_data;

			// output data
			m_write_data((offs_t)0, m_data);

			// signal peripheral device
			set_sr_line(1);

			m_next_sr = 0;
		}
		else
		{
			set_sr_line(m_next_sr);
		}
	}
}


//-------------------------------------------------
//  set_sr_line -
//-------------------------------------------------

void cdp1852_device::set_sr_line(int state)
{
	if (m_sr != state)
	{
		m_sr = state;

		m_write_sr(m_sr);
	}
}


//-------------------------------------------------
//  read - data read
//-------------------------------------------------

READ8_MEMBER( cdp1852_device::read )
{
	if (!m_read_mode() && !clock())
	{
		// input data into register
		m_data = m_read_data(0);
	}

	set_sr_line(1);

	return m_data;
}


//-------------------------------------------------
//  write - data write
//-------------------------------------------------

WRITE8_MEMBER( cdp1852_device::write )
{
	if (m_read_mode())
	{
		m_next_data = data;
		m_new_data = 1;
	}
}
