// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Batteries Included BusCard cartridge emulation

    Enable BASIC 4.0 with SYS 61000
    Disable BASIC 4.0 with SYS 61003

**********************************************************************/

#include "emu.h"
#include "buscard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define I8255_TAG		"i8255"
#define CENTRONICS_TAG	"centronics"
#define EXPANSION_TAG	"exp"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_BUSCARD, buscard_t, "c64_buscard", "C64 BusCard cartridge")


//-------------------------------------------------
//  ROM( buscard )
//-------------------------------------------------

ROM_START( buscard )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD( "buscardv0.9-tms2564.bin", 0x0000, 0x2000, CRC(175e8c96) SHA1(8fb4ba7e3d0b58dc01b66ef962955596f1b125b5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *buscard_t::device_rom_region() const
{
	return ROM_NAME( buscard );
}


//-------------------------------------------------
//  INPUT_PORTS( buscard )
//-------------------------------------------------

static INPUT_PORTS_START( buscard )
	PORT_START("SW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW:8" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor buscard_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( buscard );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void buscard_t::device_add_mconfig(machine_config &config)
{
	I8255A(config, m_ppi, 0);

	IEEE488(config, m_bus, 0);
	ieee488_slot_device::add_cbm_defaults(config, nullptr);

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  buscard_t - constructor
//-------------------------------------------------

buscard_t::buscard_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_BUSCARD, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ppi(*this, I8255_TAG),
	m_bus(*this, IEEE488_TAG),
	m_centronics(*this, CENTRONICS_TAG),
	m_exp(*this, EXPANSION_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void buscard_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buscard_t::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t buscard_t::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void buscard_t::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - cartridge GAME read
//-------------------------------------------------

int buscard_t::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->game_r(offset, sphi2, ba, rw, m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - cartridge EXROM read
//-------------------------------------------------

int buscard_t::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->hiram());
}
