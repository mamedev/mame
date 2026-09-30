// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-803 Dot Matrix Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps803.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS803, mps803_device, "mps803", "Commodore MPS-803 Dot Matrix Printer")


//-------------------------------------------------
//  ROM( mps803 )
//-------------------------------------------------

ROM_START( mps803 )
	ROM_REGION( 0x1000, "firmware", 0 )
	ROM_DEFAULT_BIOS("std")
	ROM_SYSTEM_BIOS( 0, "std", "Standard" )
	ROMX_LOAD( "mps803.bin", 0x0000, 0xe00, CRC(74f9282d) SHA1(3b887700d4a6876e76235e9b6770321a5ca884a1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "78c11", "78C11" )
	ROMX_LOAD( "mps803-78c11.bin", 0x0000, 0x1000, CRC(8500525c) SHA1(69ee52b09d87ac3c0a5105a4f0af1e862d2a5814), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps803_device::device_rom_region() const
{
	return ROM_NAME( mps803 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps803_device - constructor
//-------------------------------------------------

mps803_device::mps803_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS803, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps803_device::device_start()
{
}
