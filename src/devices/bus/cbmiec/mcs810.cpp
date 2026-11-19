// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MCS 810 Color Printer emulation

**********************************************************************/

#include "emu.h"
#include "mcs810.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MCS810, mcs810_device, "mcs810", "Commodore MCS 810 Color Printer")


//-------------------------------------------------
//  ROM( mcs810 )
//-------------------------------------------------

ROM_START( mcs810 )
	ROM_REGION( 0x4000, "firmware", 0 )
	ROM_LOAD( "mcs810-65-1115.bin", 0x0000, 0x4000, CRC(7d88d6f5) SHA1(b62396eba1910bc809979dd5444a16c7ffd431c5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mcs810_device::device_rom_region() const
{
	return ROM_NAME( mcs810 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mcs810_device - constructor
//-------------------------------------------------

mcs810_device::mcs810_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MCS810, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcs810_device::device_start()
{
}
