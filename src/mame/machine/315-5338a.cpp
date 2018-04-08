// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5338A

    I/O Controller

***************************************************************************/

#include "315-5338a.h"
#include "emu.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_315_5338A, sega_315_5338a_device, "315_5338a", "315-5338A I/O Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_315_5338a_device - constructor
//-------------------------------------------------

sega_315_5338a_device::sega_315_5338a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_315_5338A, tag, owner, clock),
	m_an_cb{ {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this} },
	m_di_cb{ {*this}, {*this}, {*this} },
	m_do_cb(*this),
	m_out(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5338a_device::device_start()
{
	// resolve callbacks
	for (int i = 0; i < 8; i++)
		m_an_cb[i].resolve_safe(0xff);

	for (int i = 0; i < 3; i++)
		m_di_cb[i].resolve_safe(0xff);

	m_do_cb.resolve_safe();
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

READ8_MEMBER( sega_315_5338a_device::read )
{
	uint8_t data = 0xff;

	switch (offset)
	{
	// analog inputs
	case 0x00: data = m_an_cb[0](0); break;
	case 0x01: data = m_an_cb[1](0); break;
	case 0x02: data = m_an_cb[2](0); break;
	case 0x03: data = m_an_cb[3](0); break;
	case 0x04: data = m_an_cb[4](0); break;
	case 0x05: data = m_an_cb[5](0); break;
	case 0x06: data = m_an_cb[6](0); break;
	case 0x07: data = m_an_cb[7](0); break;

	// digital inputs
	case 0x08: data = m_di_cb[0](0); break;
	case 0x09: data = m_di_cb[1](0); break;
	case 0x0a: data = m_di_cb[2](0); break;

	// unknown
	case 0x0b: break;
	case 0x0c: break;
	case 0x0d: break;
	case 0x0e: break; // bit 7654 input (vr board 0-3)

	// digital output
	case 0x0f: data = m_out;
	}

	if (VERBOSE)
		logerror("RD %02x = %02x\n", offset, data);

	return data;
}

WRITE8_MEMBER( sega_315_5338a_device::write )
{
	if (VERBOSE)
		logerror("WR %02x = %02x\n", offset, data);

	switch (offset)
	{
	// digital output
	case 0x0f:
		m_out = data;
		m_do_cb(m_out);
		break;
	}
}
