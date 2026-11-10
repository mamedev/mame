// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 6400 Printer emulation

**********************************************************************/

#include "emu.h"
#include "c6400.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GPIB_C6400, c6400_device, "c6400", "Commodore 6400 Printer")


//-------------------------------------------------
//  ROM( c6400 )
//-------------------------------------------------

ROM_START( c6400 )
	ROM_REGION( 0x2000, "firmware", 0 )
	ROM_DEFAULT_BIOS("601140")
	ROM_SYSTEM_BIOS( 0, "601140", "601140-28/29" )
	ROMX_LOAD( "601140-28.bin", 0x0000, 0x1000, CRC(7aa1c73f) SHA1(082a045e10cad5a297a4f408daa7e083d0e13cb5), ROM_BIOS(0) )
	ROMX_LOAD( "601140-29.bin", 0x1000, 0x1000, CRC(ab67949b) SHA1(7f0f44ba9bf79996efb4b1d70480454a1371cb37), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "cbm6400", "CBM 6400" )
	ROMX_LOAD( "cbm6400-rom0.bin", 0x0000, 0x1000, CRC(dafab9f6) SHA1(a8e993616730cb42d376abd9e2a445bc2b6b415d), ROM_BIOS(1) )
	ROMX_LOAD( "cbm6400-rom1.bin", 0x1000, 0x1000, CRC(ded6354f) SHA1(951629e1f1c942a5e9cdee124e1113b5385ea2cb), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c6400_device::device_rom_region() const
{
	return ROM_NAME( c6400 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c6400_device - constructor
//-------------------------------------------------

c6400_device::c6400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GPIB_C6400, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c6400_device::device_start()
{
}
