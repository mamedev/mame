// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1525 Graphic Printer emulation

**********************************************************************/

#include "emu.h"
#include "vic1525.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1525, vic1525_device, "vic1525", "VIC-1525 Graphic Printer")


//-------------------------------------------------
//  ROM( vic1525 )
//-------------------------------------------------

ROM_START( vic1525 )
	ROM_REGION( 0x1000, "firmware", 0 )
	ROM_LOAD( "vic1525-japan.bin", 0x0000, 0x1000, CRC(4f2d056f) SHA1(fcbe4dadf4ac8963bdfa8b7873ed47b152d55eb5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vic1525_device::device_rom_region() const
{
	return ROM_NAME( vic1525 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1525_device - constructor
//-------------------------------------------------

vic1525_device::vic1525_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, VIC1525, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1525_device::device_start()
{
}
