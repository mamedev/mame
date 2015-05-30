// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMOS 40105 FIFO Register emulation

**********************************************************************/

#include "40105.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CMOS_40105 = &device_creator<cmos_40105_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cmos_40105_device - constructor
//-------------------------------------------------

cmos_40105_device::cmos_40105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CMOS_40105, "40105", tag, owner, clock, "40105", __FILE__),
		m_write_dir(*this),
		m_write_dor(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cmos_40105_device::device_start()
{
	// resolve callbacks
	m_write_dir.resolve_safe();
	m_write_dor.resolve_safe();

	// state saving
	save_item(NAME(m_d));
	save_item(NAME(m_q));
	save_item(NAME(m_dir));
	save_item(NAME(m_dor));
	save_item(NAME(m_si));
	save_item(NAME(m_so));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cmos_40105_device::device_reset()
{
	m_fifo = std::queue<UINT8>();

	m_dir = 1;
	m_dor = 0;
	m_si = 0;

	m_write_dir(m_dir);
	m_write_dor(m_dor);
}


//-------------------------------------------------
//  read - read Q
//-------------------------------------------------

UINT8 cmos_40105_device::read()
{
	return m_q;
}


//-------------------------------------------------
//  write - write D
//-------------------------------------------------

void cmos_40105_device::write(UINT8 data)
{
	m_d = data & 0x0f;
}


//-------------------------------------------------
//  si_w - shift in write
//-------------------------------------------------

WRITE_LINE_MEMBER( cmos_40105_device::si_w )
{
	if (m_dir && !m_si && state)
	{
		m_fifo.push(m_d);

		if (m_fifo.size() == 16)
		{
			m_dir = 0;
			m_write_dir(m_dir);
		}

		if (!m_dor)
		{
			m_dor = 1;
			m_write_dor(m_dor);
		}

	}

	m_si = state;
}


//-------------------------------------------------
//  so_w - shift out write
//-------------------------------------------------

WRITE_LINE_MEMBER( cmos_40105_device::so_w )
{
	if (m_dor && m_so && !m_so)
	{
		m_dor = 0;
		m_write_dor(m_dor);

		m_q = m_fifo.front();
		m_fifo.pop();

		if (m_fifo.size() > 0)
		{
			m_dor = 1;
			m_write_dor(m_dor);
		}
	}

	m_so = state;
}


//-------------------------------------------------
//  dir_r - data in ready read
//-------------------------------------------------

READ_LINE_MEMBER( cmos_40105_device::dir_r )
{
	return m_dir;
}


//-------------------------------------------------
//  dor_r - data out ready read
//-------------------------------------------------

READ_LINE_MEMBER( cmos_40105_device::dor_r )
{
	return m_dor;
}
