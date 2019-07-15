// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

**********************************************************************/

#include "emu.h"
#include "cdp1852.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CDP1852, cdp1852_device, "cdp1852", "RCA CDP1852 I/O")



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

cdp1852_device::cdp1852_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CDP1852, tag, owner, clock),
	m_read_mode(*this),
	m_write_sr(*this),
	m_read_data(*this),
	m_write_data(*this),
	m_new_data(false), m_data(0),
	m_clock_active(true), m_sr(false), m_next_sr(false),
	m_update_do_timer(nullptr),
	m_update_sr_timer(nullptr)
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
	m_update_do_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdp1852_device::update_do), this));
	m_update_sr_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdp1852_device::update_sr), this));

	// register for state saving
	save_item(NAME(m_new_data));
	save_item(NAME(m_data));
	save_item(NAME(m_clock_active));
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
		set_sr_line(true);
	}
	else
	{
		// output data
		m_write_data((offs_t)0, m_data);

		// reset service request flip-flop
		set_sr_line(false);
	}
}


//-------------------------------------------------
//  clock_w - clock write
//-------------------------------------------------

WRITE_LINE_MEMBER(cdp1852_device::clock_w)
{
	if (m_clock_active != bool(state))
		return;

	m_clock_active = bool(state);

	if (!state)
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
				m_new_data = false;

				// signal peripheral device
				set_sr_line(true);

				m_next_sr = false;
			}
			else
			{
				set_sr_line(m_next_sr);
			}
		}
	}
}


//-------------------------------------------------
//  set_sr_line -
//-------------------------------------------------

void cdp1852_device::set_sr_line(bool state)
{
	if (m_sr != state)
	{
		m_sr = state;

		m_update_sr_timer->adjust(attotime::zero);
	}
}


//-------------------------------------------------
//  update_do - update data output
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(cdp1852_device::update_do)
{
	m_write_data(param);
}


//-------------------------------------------------
//  update_sr - update status request output
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(cdp1852_device::update_sr)
{
	m_write_sr(m_sr ? 1 : 0);
}


//-------------------------------------------------
//  read - data read
//-------------------------------------------------

READ8_MEMBER(cdp1852_device::read)
{
	if (!m_read_mode() && m_clock_active)
	{
		// input data into register
		m_data = m_read_data(0);
	}

	set_sr_line(true);

	return m_data;
}


//-------------------------------------------------
//  write - data write
//-------------------------------------------------

WRITE8_MEMBER(cdp1852_device::write)
{
	if (m_read_mode() && m_clock_active)
	{
		// output data
		m_update_do_timer->adjust(attotime::zero, data);

		m_new_data = true;
	}
}
