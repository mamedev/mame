/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/xxx.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type xxx = &device_creator<xxx_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  xxx_device - constructor
//-------------------------------------------------

xxx_device::xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, xxx, "xxx", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool xxx_device::device_validity_check(emu_options &options, const game_driver &driver) const
{
	bool error = false;
	return error;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void xxx_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void xxx_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( xxx_device::read )
{
	return 0;
}

WRITE8_MEMBER( xxx_device::write )
{
}
