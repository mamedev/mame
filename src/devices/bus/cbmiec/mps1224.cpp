// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-1224 Dot Matrix Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps1224.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1224, mps1224_device, "mps1224", "Commodore MPS-1224 Dot Matrix Printer")


//-------------------------------------------------
//  ROM( mps1224 )
//-------------------------------------------------

ROM_START( mps1224 )
	ROM_REGION( 0x10000, "firmware", 0 )
	ROM_DEFAULT_BIOS("b")
	ROM_SYSTEM_BIOS( 0, "a", "LEV. A" )
	ROMX_LOAD( "mps1224-ic5-0140-8809-0.bin", 0x0000, 0x10000, CRC(ae939f33) SHA1(363eb0f9501f212d70880e63cb3ecbb84dd6176b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "b", "LEV. B" )
	ROMX_LOAD( "mps1224-ic5-0140-8809-1.bin", 0x0000, 0x10000, CRC(514e3f5f) SHA1(77df4966694ffb625df51a0018860d541c7973c8), ROM_BIOS(1) )

	ROM_REGION( 0x8000, "ic2", 0 )
	ROM_LOAD( "mps1224-font-ic2-sz347b-0141-8810-2.bin", 0x0000, 0x8000, CRC(6fdcc6b8) SHA1(3d177a0071f2108c6a1e59842da64ff491686f9d) )

	ROM_REGION( 0x8000, "ic3", 0 )
	ROM_LOAD( "mps1224-font-ic3-sz347b-0141-8810-2.bin", 0x0000, 0x8000, CRC(31032ef7) SHA1(bc392e7a49df3731ad183544d3e3b4721e6fe18c) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1224_device::device_rom_region() const
{
	return ROM_NAME( mps1224 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1224_device - constructor
//-------------------------------------------------

mps1224_device::mps1224_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS1224, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1224_device::device_start()
{
}
