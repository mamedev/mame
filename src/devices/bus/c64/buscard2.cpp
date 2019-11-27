// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Batteries Included BusCard II cartridge emulation

    SYS 61000 -> Enable BASIC 4.0
    SYS 61003 -> Disable BASIC 4.0
    SYS 61006 -> Enter Machine Language Monitor

**********************************************************************/

#include "emu.h"
#include "buscard2.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_BUSCARD2, c64_buscard2_device, "c64_buscard2", "C64 BusCard II cartridge")


//-------------------------------------------------
//  ROM( buscard2 )
//-------------------------------------------------

ROM_START( buscard2 )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD( "v2.12.bin", 0x0000, 0x2000, CRC(1c9b2edb) SHA1(04f0a248370281fd42389928e32d11aba597cf01) )

	// dumps coming soon
	ROM_REGION( 0x200, "prom", 0 )
	ROM_LOAD( "82s129.1", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "82s129.2", 0x100, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_buscard2_device::device_rom_region() const
{
	return ROM_NAME( buscard2 );
}


//-------------------------------------------------
//  INPUT_PORTS( buscard2 )
//-------------------------------------------------

static INPUT_PORTS_START( buscard2 )
	PORT_START("S1")
	PORT_DIPNAME( 0x03, 0x00, "Device #4" ) PORT_DIPLOCATION("S1:1,2")
	PORT_DIPSETTING(    0x00, "Serial" )
	PORT_DIPSETTING(    0x01, "Parallel w/conv." )
	PORT_DIPSETTING(    0x02, "IEEE" )
	PORT_DIPSETTING(    0x03, "Parallel" )
	PORT_DIPNAME( 0x04, 0x04, "Device #5" ) PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x04, "Serial" )
	PORT_DIPNAME( 0x08, 0x08, "Device #6" ) PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x08, "Serial" )
	PORT_DIPNAME( 0x10, 0x10, "Device #7" ) PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x10, "Serial" )
	PORT_DIPNAME( 0x20, 0x20, "Device #8" ) PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x20, "Serial" )
	PORT_DIPNAME( 0x40, 0x40, "Device #9" ) PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x40, "Serial" )
	PORT_DIPNAME( 0x80, 0x80, "Device #10" ) PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x80, "Serial" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_buscard2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( buscard2 );
}


//-------------------------------------------------
//  Centronics interface
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_buscard2_device::busy_w )
{
	m_busy = state;
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_buscard2_device::device_add_mconfig(machine_config &config)
{
	RIOT6532(config, m_riot, 0);

	PIA6821(config, m_pia, 0);

	IEEE488(config, m_bus, 0);
	ieee488_slot_device::add_cbm_defaults(config, nullptr);

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->busy_handler().set(FUNC(c64_buscard2_device::busy_w));

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_buscard2_device - constructor
//-------------------------------------------------

c64_buscard2_device::c64_buscard2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_BUSCARD2, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_riot(*this, "riot"),
	m_pia(*this, "pia"),
	m_bus(*this, IEEE488_TAG),
	m_centronics(*this, "centronics"),
	m_exp(*this, "exp"),
	m_s1(*this, "S1"),
	m_rom(*this, "rom"),
	m_prom(*this, "prom"),
	m_busy(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_buscard2_device::device_start()
{
	// state saving
	save_item(NAME(m_busy));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_buscard2_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_buscard2_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_buscard2_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - cartridge GAME read
//-------------------------------------------------

int c64_buscard2_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - cartridge EXROM read
//-------------------------------------------------

int c64_buscard2_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}
