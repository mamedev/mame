// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Star NL-10 Printer Interface Cartridge emulation

**********************************************************************/

#include "c64_nl10.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_NL10_INTERFACE = &device_creator<c64_nl10_interface_device>;


//-------------------------------------------------
//  ROM( c64_nl10_interface )
//-------------------------------------------------

ROM_START( c64_nl10_interface )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "nlc 1.5.ic2", 0x0000, 0x8000, CRC(748840b6) SHA1(5b3b9e8a93d5d77a49160b3d0c2489ba7be99c9a) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_nl10_interface_device::device_rom_region() const
{
	return ROM_NAME( c64_nl10_interface );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_nl10_interface_device - constructor
//-------------------------------------------------

c64_nl10_interface_device::c64_nl10_interface_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, C64_NL10_INTERFACE, "Star NL-10 C64 Interface Cartridge", tag, owner, clock, "c64_nl10", __FILE__),
		device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_nl10_interface_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_nl10_interface_device::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c64_nl10_interface_device::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c64_nl10_interface_device::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c64_nl10_interface_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}
