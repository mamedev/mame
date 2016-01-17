// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    NEC uPD4701

    Incremental Encoder Control

    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "upd4701.h"

#define MASK_SWITCHES ( 7 )
#define MASK_COUNTER ( 0xfff )

const device_type UPD4701 = &device_creator<upd4701_device>;

upd4701_device::upd4701_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD4701, "uPD4701 Encoder", tag, owner, clock, "upd4701", __FILE__), m_cs(0), m_xy(0), m_ul(0), m_resetx(0), m_resety(0), m_latchx(0), m_latchy(0),
	m_startx(0), m_starty(0), m_x(0), m_y(0), m_switches(0), m_latchswitches(0), m_cf(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd4701_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4701_device::device_start()
{
	save_item(NAME(m_cs));
	save_item(NAME(m_xy));
	save_item(NAME(m_ul));
	save_item(NAME(m_resetx));
	save_item(NAME(m_resety));
	save_item(NAME(m_latchx));
	save_item(NAME(m_latchy));
	save_item(NAME(m_startx));
	save_item(NAME(m_starty));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_switches));
	save_item(NAME(m_latchswitches));
	save_item(NAME(m_cf));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd4701_device::device_reset()
{
	m_cs = 1;
	m_xy = 0;
	m_ul = 0;
	m_resetx = 0;
	m_resety = 0;
	m_latchx = 0;
	m_latchy = 0;
	m_startx = 0;
	m_starty = 0;
	m_x = 0;
	m_y = 0;
	m_switches = 0;
	m_latchswitches = 0;
	m_cf = 1;
}

/* x,y increments can be 12bit (see MASK_COUNTER), hence we need a couple of
16bit handlers in the following  */

/*-------------------------------------------------
    ul_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( upd4701_device::ul_w )
{
	m_ul = state;
}

/*-------------------------------------------------
    xy_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( upd4701_device::xy_w )
{
	m_xy = state;
}

/*-------------------------------------------------
    cs_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( upd4701_device::cs_w )
{
	if (m_cs != state)
	{
		m_cs = state;

		if (!m_cs)
		{
			m_latchx = (m_x - m_startx) & MASK_COUNTER;
			m_latchy = (m_y - m_starty) & MASK_COUNTER;

			m_latchswitches = (~m_switches) & MASK_SWITCHES;
			if (m_latchswitches != 0)
			{
				m_latchswitches |= 8;
			}

			m_cf = 1;
		}
	}
}

/*-------------------------------------------------
    resetx_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( upd4701_device::resetx_w )
{
	if (m_resetx != state)
	{
		m_resetx = state;

		if (m_resetx)
		{
			m_startx = m_x;
		}
	}
}

/*-------------------------------------------------
    resety_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( upd4701_device::resety_w )
{
	if (m_resety != state)
	{
		m_resety = state;

		if (m_resety)
		{
			m_starty = m_y;
		}
	}
}

/*-------------------------------------------------
    x_add
-------------------------------------------------*/

void upd4701_device::x_add( INT16 data )
{
	if (!m_resetx && data != 0)
	{
		m_x += data;

		if (m_cs)
		{
			m_cf = 0;
		}
	}
}

/*-------------------------------------------------
    y_add
-------------------------------------------------*/

void upd4701_device::y_add( INT16 data )
{
	if (!m_resety && data != 0)
	{
		m_y += data;

		if (m_cs)
		{
			m_cf = 0;
		}
	}
}

/*-------------------------------------------------
    switches_set
-------------------------------------------------*/

void upd4701_device::switches_set( UINT8 data )
{
	m_switches = data;
}

/*-------------------------------------------------
    d_r
-------------------------------------------------*/

READ16_MEMBER( upd4701_device::d_r )
{
	int data;

	if (m_cs)
	{
		return 0xff;
	}

	if (m_xy)
	{
		data = m_latchy;
	}
	else
	{
		data = m_latchx;
	}

	data |= m_latchswitches << 12;

	if (m_ul)
	{
		return data >> 8;
	}
	else
	{
		return data & 0xff;
	}
}

/*-------------------------------------------------
    sf_r
-------------------------------------------------*/

READ_LINE_MEMBER( upd4701_device::sf_r )
{
	if ((m_switches & MASK_SWITCHES) != MASK_SWITCHES)
	{
		return 0;
	}

	return 1;
}

/*-------------------------------------------------
    cf_r
-------------------------------------------------*/

READ_LINE_MEMBER( upd4701_device::cf_r )
{
	return m_cf;
}
