/**********************************************************************

    MOS Technology 6529 Single Port Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "mos6529.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type MOS6529 = &device_creator<mos6529_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6529_device - constructor
//-------------------------------------------------

mos6529_device::mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6529, "MOS6529", tag, owner, clock),
		m_read_port(*this),
		m_write_port(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6529_device::device_start()
{
	// resolve callbacks
	m_read_port.resolve_safe(0);
	m_write_port.resolve_safe();
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mos6529_device::read )
{
	return m_read_port(0);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6529_device::write )
{
	m_write_port((offs_t)0, data);
}
