/**********************************************************************

    MOS Technology 6529 Single Port Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "mos6529.h"
#include "devhelpr.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type MOS6529 = mos6529_device_config::static_alloc_device_config;




//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(mos6529, "MOS6529")


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6529_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const mos6529_interface *intf = reinterpret_cast<const mos6529_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mos6529_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_p_func, 0, sizeof(m_in_p_func));
		memset(&m_out_p_func, 0, sizeof(m_out_p_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6529_device - constructor
//-------------------------------------------------

mos6529_device::mos6529_device(running_machine &_machine, const mos6529_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6529_device::device_start()
{
	// resolve callbacks
	devcb_resolve_read8(&m_in_p_func, &m_config.m_in_p_func, this);
	devcb_resolve_write8(&m_out_p_func, &m_config.m_out_p_func, this);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mos6529_device::read )
{
	return devcb_call_read8(&m_in_p_func, 0);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6529_device::write )
{
	devcb_call_write8(&m_out_p_func, 0, data);
}
