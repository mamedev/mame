// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Chip Level Design RapiDOS Professional

**********************************************************************/

#include "emu.h"
#include "rapidos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_RAPIDOS_PROFESSIONAL, c1541_rapidos_professional_device, "c1541rp", "Chip Level Design RapiDOS Professional")


//-------------------------------------------------
//  ROM( c1541rp )
//-------------------------------------------------

ROM_START( c1541rp )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "rapidos_pro_20_drive.bin", 0x0000, 0x8000, CRC(6dbe0668) SHA1(06242e082a36f6751dca3d4c8c386737627a494c) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_rapidos_professional_device::device_rom_region() const
{
	return ROM_NAME( c1541rp );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_rapidos_professional_device - constructor
//-------------------------------------------------

c1541_rapidos_professional_device::c1541_rapidos_professional_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541_RAPIDOS_PROFESSIONAL, tag, owner, clock) { }
