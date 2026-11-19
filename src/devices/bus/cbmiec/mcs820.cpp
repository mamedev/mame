// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MCS 820 Color Printer emulation

**********************************************************************/

#include "emu.h"
#include "mcs820.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MCS820, mcs820_device, "mcs820", "Commodore MCS 820 Color Printer")


//-------------------------------------------------
//  ROM( mcs820 )
//-------------------------------------------------

ROM_START( mcs820 )
	ROM_REGION( 0x4000, "firmware", 0 )
	ROM_LOAD( "mcs820-65-1437.bin", 0x0000, 0x4000, CRC(0371981f) SHA1(31bffce523fc20ce4a000062eef48135b1915ab7) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mcs820_device::device_rom_region() const
{
	return ROM_NAME( mcs820 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mcs820_device - constructor
//-------------------------------------------------

mcs820_device::mcs820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MCS820, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcs820_device::device_start()
{
}
