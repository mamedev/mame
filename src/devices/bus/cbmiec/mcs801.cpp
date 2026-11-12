// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MCS 801 Color Printer emulation

**********************************************************************/

#include "emu.h"
#include "mcs801.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MCS801, mcs801_device, "mcs801", "Commodore MCS 801 Color Printer")


//-------------------------------------------------
//  ROM( mcs801 )
//-------------------------------------------------

ROM_START( mcs801 )
	ROM_REGION( 0x1000, "firmware", 0 )
	ROM_DEFAULT_BIOS("7ja7")
	ROM_SYSTEM_BIOS( 0, "7ja7", "7JA-7" )
	ROMX_LOAD( "mcs801-7ja-7.bin", 0x0000, 0x1000, CRC(3389dcfe) SHA1(4200a3f2b8bf5e3157c42d265410a6df429fb9df), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "f7ja1", "F7JA-1" )
	ROMX_LOAD( "mcs801-f7ja-1.bin", 0x0000, 0x1000, CRC(55b6bb5c) SHA1(b4df5d1955ed2350bcc71db5695027c2dd4f5fdd), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mcs801_device::device_rom_region() const
{
	return ROM_NAME( mcs801 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mcs801_device - constructor
//-------------------------------------------------

mcs801_device::mcs801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MCS801, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcs801_device::device_start()
{
}
