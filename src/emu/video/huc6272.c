/***************************************************************************

    Hudson/NEC HuC6272 "King" device

***************************************************************************/

#include "emu.h"
#include "video/huc6272.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type huc6272 = &device_creator<huc6272_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6272_device - constructor
//-------------------------------------------------

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, huc6272, "huc6272", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void huc6272_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6272_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void huc6272_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER( huc6272_device::read )
{
	return 0;
}

WRITE16_MEMBER( huc6272_device::write )
{
}
