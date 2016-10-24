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

ds75160a_device::ds75160a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
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

uint8_t ds75160a_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t data = 0;

	if (!m_te)
	{
		data = m_read(0);
	}

	return data;
}


//-------------------------------------------------
//  write - write data bus
//-------------------------------------------------

void ds75160a_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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

void ds75160a_device::te_w(int state)
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

void ds75160a_device::pe_w(int state)
{
	m_pe = state;
}
