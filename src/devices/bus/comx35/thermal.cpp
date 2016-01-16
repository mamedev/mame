// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Thermal Printer Card emulation

**********************************************************************/

#include "thermal.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_THM = &device_creator<comx_thm_device>;


//-------------------------------------------------
//  ROM( comx_thm )
//-------------------------------------------------

ROM_START( comx_thm )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_LOAD( "thermal.bin", 0x0000, 0x1000, CRC(41a72ba8) SHA1(3a8760c78bd8c7bec2dbf26657b930c9a6814803) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_thm_device::device_rom_region() const
{
	return ROM_NAME( comx_thm );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_thm_device - constructor
//-------------------------------------------------

comx_thm_device::comx_thm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_THM, "COMX-35 Thermal Printer Card", tag, owner, clock, "comx_thm", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_rom(*this, "c000")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_thm_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_thm_device::device_reset()
{
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_thm_device::comx_mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0;

	if (offset >= 0xc000 && offset < 0xd000)
	{
		data = m_rom->base()[offset & 0xfff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

UINT8 comx_thm_device::comx_io_r(address_space &space, offs_t offset)
{
	/*
	    INP 2 is used for the printer status, where:
	    b0=1: Printer Not Ready
	    b1=1: Energizing Head
	    b2=1: Head At Position 0
	*/

	return 0;
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_thm_device::comx_io_w(address_space &space, offs_t offset, UINT8 data)
{
	/*
	    OUT 2 is used to control the thermal printer where:
	    Q = 0, b0-7: Pixel 1 to 8
	    Q = 1, b7: Pixel 9 (if b0-6=#21)
	    Q = 1, b3=1: Move head right
	    Q = 1, b0-7=#12: Move head left
	*/
}
