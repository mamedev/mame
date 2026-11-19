// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-801 Dot Matrix Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps801.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS801, mps801_device, "mps801", "Commodore MPS-801 Dot Matrix Printer")


//-------------------------------------------------
//  ROM( mps801 )
//-------------------------------------------------

ROM_START( mps801 )
	ROM_REGION( 0x1000, "firmware", 0 )
	ROM_DEFAULT_BIOS("std")
	ROM_SYSTEM_BIOS( 0, "std", "Standard" )
	ROMX_LOAD( "mps801.bin", 0x0000, 0x1000, CRC(c1fbd1f3) SHA1(fbc1fdef19d1c6b238ede63f6632ab8bccd1b75a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "swe", "Swedish" )
	ROMX_LOAD( "mps801-swe.bin", 0x0000, 0x1000, CRC(5abb058b) SHA1(57c7207d9e67c94aa506e11f2d5924de7d616c92), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "unknown", "Unknown 2732" )
	ROMX_LOAD( "mps801-unknown2732.bin", 0x0000, 0x1000, CRC(15405739) SHA1(d32c68079f1ea3488f6fc1e0778b5afc5b7c556b), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps801_device::device_rom_region() const
{
	return ROM_NAME( mps801 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps801_device - constructor
//-------------------------------------------------

mps801_device::mps801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS801, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps801_device::device_start()
{
}
