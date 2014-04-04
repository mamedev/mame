// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9212 Double Row Buffer (DRB) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

	TODO:

	- RCLK and WCLK are clocked simultaneously since that's what tandy2k does and we have no other users

*/

#include "crt9212.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CRT9212 = &device_creator<crt9212_t>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt9212_t - constructor
//-------------------------------------------------

crt9212_t::crt9212_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CRT9212, "SMC CRT9212", tag, owner, clock, "crt9212", __FILE__),
	m_write_dout(*this),
	m_write_rof(*this),
	m_write_wof(*this),
	m_data(0),
	m_clrcnt(0),
	m_tog(0),
	m_ren(0),
	m_wen1(1),
	m_wen2(0),
	m_oe(0),
	m_rclk(0),
	m_wclk(0),
	m_buffer(0),
	m_rac(0),
	m_wac(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9212_t::device_start()
{
	// resolve callbacks
	m_write_dout.resolve_safe();
	m_write_rof.resolve_safe();
	m_write_wof.resolve_safe();

	// allocate timers
	m_rwclk_timer = timer_alloc();

	if (clock())
	{
		m_rwclk_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	}

	// state saving
	save_item(NAME(m_data));
	save_item(NAME(m_clrcnt));
	save_item(NAME(m_tog));
	save_item(NAME(m_ren));
	save_item(NAME(m_wen1));
	save_item(NAME(m_wen2));
	save_item(NAME(m_oe));
	save_item(NAME(m_rclk));
	save_item(NAME(m_wclk));
	save_item(NAME(m_ram[0]));
	save_item(NAME(m_ram[1]));
	save_item(NAME(m_buffer));
	save_item(NAME(m_rac));
	save_item(NAME(m_wac));
}


//-------------------------------------------------
//  device_clock_changed - handle clock change
//-------------------------------------------------

void crt9212_t::device_clock_changed()
{
	m_rwclk_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void crt9212_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	rclk_w(1);
	rclk_w(0);

	wclk_w(1);
	wclk_w(0);
}


//-------------------------------------------------
//  rclk_w - read clock
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_t::rclk_w )
{
	if (m_rclk && !state)
	{
		if (!m_clrcnt)
		{
			if (!m_tog)
			{
				// switch buffer
				m_buffer = !m_buffer;

				// clear write address counter
				m_wac = 0;
				m_write_wof(0);
			}
			else
			{
				// clear read address counter
				m_rac = 0;
				m_write_rof(0);
			}
		}
		else
		{
			if (m_ren && (m_rac < CRT9212_RAM_SIZE))
			{
				//
				m_write_dout(m_ram[m_rac][!m_buffer]);

				// increment read address counter
				m_rac++;

				if (m_rac == CRT9212_RAM_SIZE)
				{
					// set read overflow
					m_write_rof(1);
				}
			}
		}
	}

	m_rclk = state;
}


//-------------------------------------------------
//  wclk_w - write clock
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_t::wclk_w )
{
	if (!m_wclk && state)
	{
		if (m_wen1 && m_wen2 && (m_wac < CRT9212_RAM_SIZE))
		{
			//
			m_ram[m_rac][m_buffer] = m_data;

			// increment read address counter
			m_wac++;

			if (m_wac == CRT9212_RAM_SIZE)
			{
				// set write overflow
				m_write_wof(1);
			}
		}
	}

	m_wclk = state;
}
