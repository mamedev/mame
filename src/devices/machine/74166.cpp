// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SN54/74166

    8-Bit Parallel-In/Serial-Out Shift Register

***************************************************************************/

#include "emu.h"
#include "74166.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TTL166, ttl166_device, "ttl166", "SN54/74166")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ttl166_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, m_timer).configure_generic(FUNC(ttl166_device::qh_output));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ttl153_device - constructor
//-------------------------------------------------

ttl166_device::ttl166_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TTL166, tag, owner, clock),
	m_timer(*this, "timer"),
	m_data_cb(*this), m_qh_cb(*this),
	m_data(0x00),
	m_ser(0), m_clk(0), m_shld(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl166_device::device_start()
{
	// resolve callbacks
	m_data_cb.resolve_safe(0x00);
	m_qh_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_data));
	save_item(NAME(m_ser));
	save_item(NAME(m_clk));
	save_item(NAME(m_shld));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl166_device::device_reset()
{
	m_data = 0x00;
	m_ser = 0;
	m_clk = 0;
	m_shld = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER( ttl166_device::qh_output )
{
	m_qh_cb(param);
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( ttl166_device::serial_w )
{
	m_ser = state;
}

WRITE_LINE_MEMBER( ttl166_device::clock_w )
{
	if (m_clk == 0 && state == 1)
	{
		if (m_shld)
		{
			// shift next bit
			m_data <<= 1;
			m_data |= m_ser;
		}
		else
		{
			// load external data
			m_data = m_data_cb(0);
		}

		// we need to delay the output a bit to allow for serial input
		m_timer->adjust(attotime::from_nsec(25), BIT(m_data, 7));
	}

	m_clk = state;
}

WRITE_LINE_MEMBER( ttl166_device::shift_load_w )
{
	m_shld = state;
}
