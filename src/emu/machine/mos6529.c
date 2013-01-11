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
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type MOS6529 = &device_creator<mos6529_device>;

//-------------------------------------------------
//  mos6529_device - constructor
//-------------------------------------------------

mos6529_device::mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6529, "MOS6529", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6529_device::device_config_complete()
{
	// inherit a copy of the static data
	const mos6529_interface *intf = reinterpret_cast<const mos6529_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mos6529_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_p_cb, 0, sizeof(m_in_p_cb));
		memset(&m_out_p_cb, 0, sizeof(m_out_p_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6529_device::device_start()
{
	// resolve callbacks
	m_in_p_func.resolve(m_in_p_cb, *this);
	m_out_p_func.resolve(m_out_p_cb, *this);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mos6529_device::read )
{
	return m_in_p_func(0);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6529_device::write )
{
	m_out_p_func(0, data);
}
