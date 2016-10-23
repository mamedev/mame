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
const device_type XXX = &device_creator<xxx_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  xxx_device - constructor
//-------------------------------------------------

xxx_device::xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XXX, "xxx_longname", tag, owner, clock, "xxx", __FILE__)
{
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

uint8_t xxx_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0;
}

void xxx_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}
