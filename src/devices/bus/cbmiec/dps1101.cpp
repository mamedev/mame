// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore DPS 1101 Daisy Wheel Printer emulation

**********************************************************************/

#include "emu.h"
#include "dps1101.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DPS1101, dps1101_device, "dps1101", "Commodore DPS 1101 Daisy Wheel Printer")


//-------------------------------------------------
//  ROM( dps1101 )
//-------------------------------------------------

ROM_START( dps1101 )
	ROM_REGION( 0x2000, "firmware", 0 )
	ROM_LOAD( "dps1101-0-8b.bin", 0x0000, 0x2000, CRC(1798fdd3) SHA1(506f9076b130292828c3b8e9db17e1c7b51362fa) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dps1101_device::device_rom_region() const
{
	return ROM_NAME( dps1101 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dps1101_device - constructor
//-------------------------------------------------

dps1101_device::dps1101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, DPS1101, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dps1101_device::device_start()
{
}
