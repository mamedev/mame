// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8023P/MPP-1361 Printer emulation

**********************************************************************/

#include "emu.h"
#include "c8023p.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GPIB_C8023P, c8023p_device, "c8023p", "Commodore 8023P Printer")


//-------------------------------------------------
//  ROM( c8023p )
//-------------------------------------------------

ROM_START( c8023p )
	ROM_REGION( 0x2000, "firmware", 0 )
	ROM_DEFAULT_BIOS("325320-02")
	ROM_SYSTEM_BIOS( 0, "325320-01", "325320-01" )
	ROMX_LOAD( "325320-01.bin", 0x0000, 0x2000, CRC(95b2e4a8) SHA1(24901d2b254892beda98e81286d6775fd36249ec), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "325320-02", "325320-02" )
	ROMX_LOAD( "325320-02.bin", 0x0000, 0x2000, CRC(e5c4a58c) SHA1(31c2ad049e368cc90b47fb20f5f4ed155a191b59), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c8023p_device::device_rom_region() const
{
	return ROM_NAME( c8023p );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c8023p_device - constructor
//-------------------------------------------------

c8023p_device::c8023p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GPIB_C8023P, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8023p_device::device_start()
{
}
