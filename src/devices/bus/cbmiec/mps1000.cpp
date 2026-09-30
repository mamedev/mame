// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-1000 Dot Matrix Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps1000.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1000, mps1000_device, "mps1000p", "Commodore MPS-1000 Dot Matrix Printer")


//-------------------------------------------------
//  ROM( mps1000 )
//-------------------------------------------------

ROM_START( mps1000 )
	ROM_REGION( 0x8000, "firmware", 0 )
	ROM_LOAD( "mps1000-e2-ce8.bin", 0x0000, 0x8000, CRC(412bd4e2) SHA1(af88d10077d990fa1edbc6cdbd059af64738b8d4) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1000_device::device_rom_region() const
{
	return ROM_NAME( mps1000 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1000_device - constructor
//-------------------------------------------------

mps1000_device::mps1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS1000, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1000_device::device_start()
{
}
