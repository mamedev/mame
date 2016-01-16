// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 ROM cartridge device

**********************************************************************/

#include "p1_rom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type P1_ROM = &device_creator<p1_rom_device>;


//-------------------------------------------------
//  ROM( p1_rom )
//-------------------------------------------------

ROM_START( p1_rom )
	ROM_REGION( 0x2000, "p1_rom", 0 )
	ROM_SYSTEM_BIOS(0, "ram", "Test 3 -- RAM test")
	ROMX_LOAD( "p1_t_ram.rf4", 0x00000, 0x2000, CRC(e42f5a61) SHA1(ce2554eae8f0d2b6d482890dd198cf7e2d29c655), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "io", "Test 4 -- I/O test")
	ROMX_LOAD( "p1_t_i_o.rf4", 0x00000, 0x2000, CRC(18a781de) SHA1(7267970ee27e3ea1d972bee8e74b17bac1051619), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "pls", "\"MB test\"")
	ROMX_LOAD( "p1_t_pls.rf4", 0x00000, 0x2000, CRC(c8210ffb) SHA1(f2d1a6c90e4708bcc56186b2fb906fa852667084), ROM_BIOS(3))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *p1_rom_device::device_rom_region() const
{
	return ROM_NAME( p1_rom );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p1_rom_device - constructor
//-------------------------------------------------

p1_rom_device::p1_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, P1_ROM, "Poisk-1 ROM cart", tag, owner, clock, "p1_rom", __FILE__),
	device_isa8_card_interface( mconfig, *this )
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p1_rom_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xc0000, 0xc1fff, 0, 0, "XXX", "p1_rom");
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void p1_rom_device::device_reset()
{
}
