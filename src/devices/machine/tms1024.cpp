// license:BSD-3-Clause
// copyright-holders:hap
/*

  Texas Instruments TMS1024/TMS1025 I/O expander

  No documentation was available, just a pinout.
  Other than more port pins, TMS1025 is assumed to be same as TMS1024.

  TODO:
  - writes to port 0
  - what's the MS pin?
  - strobe is on rising edge? or falling edge?

*/

#include "machine/tms1024.h"


const device_type TMS1024 = &device_creator<tms1024_device>;
const device_type TMS1025 = &device_creator<tms1025_device>;

//-------------------------------------------------
//  constructor
//-------------------------------------------------

tms1024_device::tms1024_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS1024, "TMS1024 I/O Expander", tag, owner, clock, "tms1024", __FILE__), m_h(0), m_s(0), m_std(0),
	m_write_port1(*this), m_write_port2(*this), m_write_port3(*this), m_write_port4(*this), m_write_port5(*this), m_write_port6(*this), m_write_port7(*this)
{
}

tms1024_device::tms1024_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source), m_h(0), m_s(0), m_std(0),
	m_write_port1(*this), m_write_port2(*this), m_write_port3(*this), m_write_port4(*this), m_write_port5(*this), m_write_port6(*this), m_write_port7(*this)
{
}

tms1025_device::tms1025_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: tms1024_device(mconfig, TMS1025, "TMS1025 I/O Expander", tag, owner, clock, "tms1025", __FILE__)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms1024_device::device_start()
{
	// resolve callbacks (there is no port 0)
	m_write_port1.resolve_safe(); m_write_port[0] = &m_write_port1;
	m_write_port2.resolve_safe(); m_write_port[1] = &m_write_port2;
	m_write_port3.resolve_safe(); m_write_port[2] = &m_write_port3;
	m_write_port4.resolve_safe(); m_write_port[3] = &m_write_port4;
	m_write_port5.resolve_safe(); m_write_port[4] = &m_write_port5;
	m_write_port6.resolve_safe(); m_write_port[5] = &m_write_port6;
	m_write_port7.resolve_safe(); m_write_port[6] = &m_write_port7;

	// zerofill
	m_h = 0;
	m_s = 0;
	m_std = 0;

	// register for savestates
	save_item(NAME(m_h));
	save_item(NAME(m_s));
	save_item(NAME(m_std));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms1024_device::device_reset()
{
}



//-------------------------------------------------
//  handlers
//-------------------------------------------------

WRITE8_MEMBER(tms1024_device::write_h)
{
	// H1,2,3,4: data for outputs A,B,C,D
	m_h = data & 0xf;
}

WRITE8_MEMBER(tms1024_device::write_s)
{
	// S0,1,2: select port
	m_s = data & 7;
}

WRITE_LINE_MEMBER(tms1024_device::write_std)
{
	state = (state) ? 1 : 0;

	// output on rising edge
	if (state && !m_std)
	{
		if (m_s != 0)
			(*m_write_port[m_s-1])((offs_t)(m_s-1), m_h);
	}

	m_std = state;
}
