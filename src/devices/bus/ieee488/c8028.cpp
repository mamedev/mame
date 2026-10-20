// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8028/MPP-1361 Printer emulation

**********************************************************************/

#include "emu.h"
#include "c8028.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GPIB_C8028, c8028_device, "c8028", "Commodore 8028 Printer")


//-------------------------------------------------
//  ROM( c8028 )
//-------------------------------------------------

ROM_START( c8028 )
	ROM_REGION( 0x2000, "firmware", 0 )
	ROM_LOAD( "ua5-m-07a.bin", 0x0000, 0x2000, CRC(9d662c74) SHA1(0ac76d646c80d906d19e279ee6593338ffb43af2) )

	ROM_REGION( 0x0800, "carriage", 0 )
	ROM_LOAD( "ua10-car-06.bin", 0x0000, 0x0800, CRC(209e70a1) SHA1(684d666cfb44443105900ccef3f456bc88a084bd) )

	ROM_REGION( 0x0800, "daisy", 0 )
	ROM_DEFAULT_BIOS("06")
	ROM_SYSTEM_BIOS( 0, "06", "UA8-DSY 06" )
	ROMX_LOAD( "ua8-dsy-06.bin", 0x0000, 0x0800, CRC(05d30d5f) SHA1(f8d63fa1e63ab142d6fd31ce82c34dca167d1c46), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "062", "UA8-DSY 06-2" )
	ROMX_LOAD( "ua8-dsy-06-2.bin", 0x0000, 0x0800, CRC(ee54119c) SHA1(a108ac5f6e08440f1fa1220846e5030c2843843a), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c8028_device::device_rom_region() const
{
	return ROM_NAME( c8028 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c8028_device - constructor
//-------------------------------------------------

c8028_device::c8028_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GPIB_C8028, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8028_device::device_start()
{
}
