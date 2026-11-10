// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 3022 Printer emulation

**********************************************************************/

#include "emu.h"
#include "c3022.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GPIB_C3022, c3022_device, "c3022", "Commodore 3022 Printer")


//-------------------------------------------------
//  ROM( c3022 )
//-------------------------------------------------

ROM_START( c3022 )
	ROM_REGION( 0x1000, "firmware", 0 )
	ROM_DEFAULT_BIOS("r07")
	ROM_SYSTEM_BIOS( 0, "r03", "901472-03" )
	ROMX_LOAD( "901472-03.bin", 0x0000, 0x1000, CRC(e445ad64) SHA1(cb87a8d152813c6071652abd866efb25de247769), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r04", "901472-04" )
	ROMX_LOAD( "901472-04.bin", 0x0000, 0x1000, CRC(9bf3c9f8) SHA1(62be6bcf3a3aa844d629e8a52b30e01864cc8266), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r05", "901472-05" )
	ROMX_LOAD( "901472-05.bin", 0x0000, 0x1000, CRC(918c0ead) SHA1(1eb54f6095a2979032d436c642af4a7139417665), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "r06", "901472-06" )
	ROMX_LOAD( "901472-06.bin", 0x0000, 0x1000, CRC(b6d8519c) SHA1(d20fea96c2cdee1f449b530e658e82082031114e), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "r07", "901472-07" )
	ROMX_LOAD( "901472-07.bin", 0x0000, 0x1000, CRC(90808512) SHA1(13ec74d1b958f78c897e1078ef6b91035216f146), ROM_BIOS(4) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c3022_device::device_rom_region() const
{
	return ROM_NAME( c3022 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c3022_device - constructor
//-------------------------------------------------

c3022_device::c3022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GPIB_C3022, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c3022_device::device_start()
{
}
