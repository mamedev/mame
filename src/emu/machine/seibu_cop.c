/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/seibu_cop.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SEIBU_COP = &device_creator<seibu_cop_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  seibu_cop_device - constructor
//-------------------------------------------------

seibu_cop_device::seibu_cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEIBU_COP, "seibu_cop", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void seibu_cop_device::device_config_complete()
{
	// inherit a copy of the static data
	const seibu_cop_interface *intf = reinterpret_cast<const seibu_cop_interface *>(static_config());
	if (intf != NULL)
		*static_cast<seibu_cop_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_mreq_cb, 0, sizeof(m_in_mreq_cb));
		memset(&m_out_mreq_cb, 0, sizeof(m_out_mreq_cb));
	}
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void seibu_cop_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_cop_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void seibu_cop_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( seibu_cop_device::read )
{
	return 0;
}

WRITE8_MEMBER( seibu_cop_device::write )
{
}
