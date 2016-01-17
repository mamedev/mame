// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    MC-1502 ROM cartridge device

**********************************************************************/

#include "mc1502_rom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MC1502_ROM = &device_creator<mc1502_rom_device>;


//-------------------------------------------------
//  ROM( mc1502_rom )
//-------------------------------------------------

ROM_START( mc1502_rom )
	ROM_REGION( 0x8000, "mc1502_rom", 0 )
	ROM_LOAD( "basic.rom", 0x00000, 0x8000, CRC(173d69fa) SHA1(003f872e12f00800e22ab6bbc009d36bfde67b9d))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *mc1502_rom_device::device_rom_region() const
{
	return ROM_NAME( mc1502_rom );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc1502_rom_device - constructor
//-------------------------------------------------

mc1502_rom_device::mc1502_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MC1502_ROM, "MC-1502 ROM cart", tag, owner, clock, "mc1502_rom", __FILE__),
	device_isa8_card_interface( mconfig, *this )
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc1502_rom_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xe8000, 0xeffff, 0, 0, "XXX", "mc1502_rom");
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc1502_rom_device::device_reset()
{
}
