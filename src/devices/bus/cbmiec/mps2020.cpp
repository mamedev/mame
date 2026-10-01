// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-2020 Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps2020.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS2020, mps2020_device, "mps2020", "Commodore MPS-2020 Printer")


//-------------------------------------------------
//  ROM( mps2020 )
//-------------------------------------------------

ROM_START( mps2020 )
	ROM_REGION( 0x2000, "rom1", 0 )
	ROM_LOAD( "mps2020.bin", 0x0000, 0x2000, CRC(966f001c) SHA1(1ac0eace12628bf4ed4803462dc88c571f4ba9de) )
	ROM_REGION( 0x8000, "rom2", 0 )
	ROM_LOAD( "2020pmq2.bin", 0x0000, 0x8000, CRC(024b2738) SHA1(9ab4d548466f34e41cee51b46bf4250c0ace933e) )
	ROM_REGION( 0x10000, "rom3", 0 )
	ROM_LOAD( "2020pmq1.bin", 0x0000, 0x10000, CRC(71265acd) SHA1(e1d5a86d068f407db8337054ac112865fdf64bde) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps2020_device::device_rom_region() const
{
	return ROM_NAME( mps2020 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps2020_device - constructor
//-------------------------------------------------

mps2020_device::mps2020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS2020, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps2020_device::device_start()
{
}
