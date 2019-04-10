// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Thermal Printer Card emulation

**********************************************************************/

#include "emu.h"
#include "thermal.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMX_THM, comx_thm_device, "comx_thm", "COMX-35 Thermal Printer Card")


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

const tiny_rom_entry *comx_thm_device::device_rom_region() const
{
	return ROM_NAME( comx_thm );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_thm_device - constructor
//-------------------------------------------------

comx_thm_device::comx_thm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COMX_THM, tag, owner, clock),
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

uint8_t comx_thm_device::comx_mrd_r(offs_t offset, int *extrom)
{
	uint8_t data = 0;

	if (offset >= 0xc000 && offset < 0xd000)
	{
		data = m_rom->base()[offset & 0xfff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

uint8_t comx_thm_device::comx_io_r(offs_t offset)
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

void comx_thm_device::comx_io_w(offs_t offset, uint8_t data)
{
	/*
	    OUT 2 is used to control the thermal printer where:
	    Q = 0, b0-7: Pixel 1 to 8
	    Q = 1, b7: Pixel 9 (if b0-6=#21)
	    Q = 1, b3=1: Move head right
	    Q = 1, b0-7=#12: Move head left
	*/
}
