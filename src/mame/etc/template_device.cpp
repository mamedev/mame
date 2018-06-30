// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "xxx.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(XXX, xxx_device, "xxx", "XXX")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  xxx_device - constructor
//-------------------------------------------------

xxx_device::xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XXX, tag, owner, clock)
{
}




//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

MACHINE_CONFIG_START(xxx_device::device_add_mconfig)
	//MCFG_DEVICE_ADD(...)
MACHINE_CONFIG_END


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
