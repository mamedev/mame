// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1002A-WX1 Winchester Disk Controller emulation

**********************************************************************/

#include "wd1002a_wx1.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISA8_WD1002A_WX1 = &device_creator<isa8_wd1002a_wx1_device>;


//-------------------------------------------------
//  ROM( wd1002a_wx1 )
//-------------------------------------------------

ROM_START( wd1002a_wx1 )
	ROM_REGION( 0x2000, "wd1002a_wx1", 0 )
	ROM_LOAD( "600693-001 type 5.u12", 0x0000, 0x2000, CRC(f3daf85f) SHA1(3bd29538832d3084cbddeec92593988772755283) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_wd1002a_wx1_device::device_rom_region() const
{
	return ROM_NAME( wd1002a_wx1 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_wd1002a_wx1_device - constructor
//-------------------------------------------------

isa8_wd1002a_wx1_device::isa8_wd1002a_wx1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISA8_WD1002A_WX1, "WD1002A-WX1", tag, owner, clock, "wd1002a_wx1", __FILE__),
		device_isa8_card_interface( mconfig, *this )
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_reset()
{
}
