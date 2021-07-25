// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    VTech Laser Lightpen Interface

    Skeleton just to document the I/O ports used

***************************************************************************/

#include "emu.h"
#include "lpen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_LPEN_INTERFACE, vtech_lpen_interface_device, "vtech_lpen", "Laser/VZ Lightpen Interface")

//-------------------------------------------------
//  io_map - memory space address map
//-------------------------------------------------

void vtech_lpen_interface_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x40, 0x4f).r(FUNC(vtech_lpen_interface_device::lpen_r));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_lpen_interface_device - constructor
//-------------------------------------------------

vtech_lpen_interface_device::vtech_lpen_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_ioexp_device(mconfig, VTECH_LPEN_INTERFACE, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_lpen_interface_device::device_start()
{
	vtech_ioexp_device::device_start();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t vtech_lpen_interface_device::lpen_r(offs_t offset)
{
	logerror("lpen_r %02x\n", offset);
	return 0xff;
}
