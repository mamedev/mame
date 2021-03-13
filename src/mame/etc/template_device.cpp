// license:<license_opt>
// copyright-holders:<author_name>
/***************************************************************************

<device_longname>

***************************************************************************/

#include "emu.h"
#include "<device_classname>.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(<device_typename>, <device_classname>_device, "<device_classname>", "<device_longname>")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  <device_classname>_device - constructor
//-------------------------------------------------


<device_classname>_device::<device_classname>_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, <device_typename>, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------


void <device_classname>_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------


void <device_classname>_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void <device_classname>_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


uint8_t <device_classname>_device::read(address_space &space, offs_t offset, uint8_t mem_mask = ~0)
{
	return 0;
}


void <device_classname>_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = ~0)
{
}
