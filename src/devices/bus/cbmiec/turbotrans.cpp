// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rossmöller TurboTrans

**********************************************************************/

#include "emu.h"
#include "turbotrans.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_TURBOTRANS, c1541_turbotrans_device, "c1541tt", "Rossmöller TurboTrans")


//-------------------------------------------------
//  ROM( c1541tt )
//-------------------------------------------------

ROM_START( c1541tt )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "ttd34.uab5", 0x0000, 0x8000, CRC(518d34a1) SHA1(4d6ffdce6ab122e9627b0a839861687bcd4e03ec) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_turbotrans_device::device_rom_region() const
{
	return ROM_NAME( c1541tt );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_turbotrans_device - constructor
//-------------------------------------------------

c1541_turbotrans_device::c1541_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541_TURBOTRANS, tag, owner, clock) { }
