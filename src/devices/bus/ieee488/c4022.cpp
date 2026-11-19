// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 4022 Printer emulation

**********************************************************************/

#include "emu.h"
#include "c4022.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GPIB_C4022, c4022_device, "c4022", "Commodore 4022 Printer")


//-------------------------------------------------
//  ROM( c4022 )
//-------------------------------------------------

ROM_START( c4022 )
	ROM_REGION( 0x2000, "firmware", 0 )
	ROM_DEFAULT_BIOS("901631-02")
	ROM_SYSTEM_BIOS( 0, "324764-01", "324764-01" )
	ROMX_LOAD( "324764-01.bin", 0x0000, 0x2000, CRC(3a68bc5b) SHA1(9bbb631ddf2f5e78175dbb845cce8aba140fd01c), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "901490-01", "901490-01" )
	ROMX_LOAD( "901490-01.bin", 0x0000, 0x2000, CRC(adf74f94) SHA1(b5ede0de9f1257e393803a4e3ac1aa6eaf37c3be), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "901631-02", "901631-02" )
	ROMX_LOAD( "901631-02.bin", 0x0000, 0x2000, CRC(78a3c6a6) SHA1(3a5de52bc2eafd26b33cc5276fca8bc5cbed12e7), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c4022_device::device_rom_region() const
{
	return ROM_NAME( c4022 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c4022_device - constructor
//-------------------------------------------------

c4022_device::c4022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GPIB_C4022, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c4022_device::device_start()
{
}
