// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-1270A Inkjet Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps1270a.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1270A, mps1270a_device, "mps1270a", "Commodore MPS-1270A Inkjet Printer")


//-------------------------------------------------
//  ROM( mps1270a )
//-------------------------------------------------

ROM_START( mps1270a )
	ROM_REGION( 0x10000, "firmware", 0 )
	ROM_DEFAULT_BIOS("601250-54")
	ROM_SYSTEM_BIOS( 0, "601250-54", "601250-54" )
	ROMX_LOAD( "mps1270-601250-54.bin", 0x0000, 0x10000, CRC(41cdedf9) SHA1(8815e01cc0a4c499b9c569c46aed17d821a4c95c), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "revcsc", "Rev. CSC" )
	ROMX_LOAD( "mps1270a-revcsc.bin", 0x0000, 0x10000, CRC(9d0377c6) SHA1(b74d2da6a66b02e271249fb3d1926e46224d4c5d), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1270a_device::device_rom_region() const
{
	return ROM_NAME( mps1270a );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1270a_device - constructor
//-------------------------------------------------

mps1270a_device::mps1270a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS1270A, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1270a_device::device_start()
{
}
