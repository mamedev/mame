// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor DS75160A IEEE-488 GPIB Transceiver emulation

**********************************************************************/

#include "ds75160a.h"



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

const device_type DS75160A = &device_creator<ds75160a_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ds75160a_device - constructor
//-------------------------------------------------

ds75160a_device::ds75160a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DS75160A, "DS75160A", tag, owner, clock, "ds75160a", __FILE__),
		m_read(*this),
		m_write(*this),
		m_data(0xff),
		m_te(0),
		m_pe(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds75160a_device::device_start()
{
	// resolve callbacks
	m_read.resolve_safe(0);
	m_write.resolve_safe();

	// register for state saving
	save_item(NAME(m_data));
	save_item(NAME(m_te));
	save_item(NAME(m_pe));
}


//-------------------------------------------------
//  read - read data bus
//-------------------------------------------------

READ8_MEMBER( ds75160a_device::read )
{
	UINT8 data = 0;

	if (!m_te)
	{
		data = m_read(0);
	}

	return data;
}


//-------------------------------------------------
//  write - write data bus
//-------------------------------------------------

WRITE8_MEMBER( ds75160a_device::write )
{
	m_data = data;

	if (m_te)
	{
		m_write((offs_t)0, m_data);
	}
}


//-------------------------------------------------
//  te_w - transmit enable
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75160a_device::te_w )
{
	if (m_te != state)
	{
		m_write((offs_t)0, m_te ? m_data : 0xff);
	}

	m_te = state;
}


//-------------------------------------------------
//  pe_w - parallel enable
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75160a_device::pe_w )
{
	m_pe = state;
}
