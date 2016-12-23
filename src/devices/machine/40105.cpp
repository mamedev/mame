// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CD40105/HC40105 4-bit x 16-word FIFO Register

    Part of the 4000B series of CMOS TTL devices, the 40105 includes
    an asynchronous master reset pin intended to be connected to the
    system bus.

    Word size can be expanded from 4 bits to 8 bits by connecting two
    40105s in parallel, with external AND gates to combine the DIR and
    DOR outputs. They can also be connected in series to extend the
    FIFO capacity, with DOR -> SI and /SO <- DIR.

**********************************************************************/

#include "40105.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CD40105 = &device_creator<cmos_40105_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cmos_40105_device - constructor
//-------------------------------------------------

cmos_40105_device::cmos_40105_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CD40105, "40105 FIFO", tag, owner, clock, "cd40105", __FILE__),
		m_write_dir(*this),
		m_write_dor(*this),
		m_write_q(*this),
		m_d(0),
		m_q(0),
		m_dir(false),
		m_dor(false),
		m_si(false),
		m_so(false)
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
	m_write_q.resolve_safe();

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
	// invalidate data in queue
	m_fifo = std::queue<u8>();

	// reset control flip-flops
	m_dir = true;
	m_dor = false;
	m_write_dir(1);
	m_write_dor(0);
}


//-------------------------------------------------
//  read - read output buffer (Q0 to Q3)
//-------------------------------------------------

u8 cmos_40105_device::read()
{
	return m_q;
}


//-------------------------------------------------
//  write - write input buffer (D0 to D3)
//-------------------------------------------------

void cmos_40105_device::write(u8 data)
{
	m_d = data & 0x0f;
}


//-------------------------------------------------
//  si_w - shift in write
//-------------------------------------------------

WRITE_LINE_MEMBER( cmos_40105_device::si_w )
{
	// activate on rising edge when ready
	if (m_dir && !m_si && state)
	{
		m_fifo.push(m_d);

		// DIR remains low if FIFO is full, or else briefly pulses low
		m_write_dir(0);
		if (m_fifo.size() == 16)
			m_dir = false;
		else
			m_write_dir(1);

		// signal availability of propagated data
		if (!m_dor)
		{
			m_dor = true;
			m_write_dor(1);
		}
	}

	m_si = state;
}


//-------------------------------------------------
//  so_w - shift out write
//-------------------------------------------------

WRITE_LINE_MEMBER( cmos_40105_device::so_w )
{
	// activate on falling edge when ready
	if (m_dor && m_so && !state)
	{
		m_q = m_fifo.front();
		m_fifo.pop();
		m_write_dor(0);
		m_write_q(m_q);

		// DOR remains low if FIFO is now empty, or else briefly pulses low
		if (m_fifo.size() > 0)
			m_write_dor(1);
		else
			m_dor = false;

		// FIFO can no longer be full
		if (!m_dir)
		{
			m_dir = true;
			m_write_dir(1);
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
