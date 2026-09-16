// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Koulun erityispalvelu PS-64 speech cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|                      SW1|
    |=|       SC02              |
    |=|                         |
    |=|                      CN1|
    |=|       ROM        LF347  |
    |=|                         |
    |=|                         |
    |===========================|

    SC02  - Votrax SSI-263AP Speech Synthesizer
    ROM   - Hynix Semiconductor HY27C64D-20 8Kx8 EPROM
    LF347 - National Instruments LF347N JFET Operational Amplifier
    SW1   - Module on/off switch
    CN1   - connector to C64 video/audio port

    Designed by Raimo Laukkanen

*/

#include "emu.h"
#include "ps64.h"
#include "speaker.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define SSI263_TAG      "ssi263"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_PS64, c64_ps64_cartridge_device, "c64_ps64", "C64 PS-64")


//-------------------------------------------------
//  INPUT_PORTS( c64_ps64 )
//-------------------------------------------------

static INPUT_PORTS_START( c64_ps64 )
	PORT_START("SW1")
	PORT_CONFNAME( 0x01, 0x01, "Module" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_ps64_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_ps64 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_ps64_cartridge_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SSI263HLE(config, m_ssi263, 756000);
	m_ssi263->add_route(0, "mono", 1.00, 0);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ps64_cartridge_device - constructor
//-------------------------------------------------

c64_ps64_cartridge_device::c64_ps64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_PS64, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_ssi263(*this, SSI263_TAG),
	m_sw1(*this, "SW1"),
	m_io_space_config("io", ENDIANNESS_LITTLE, 8, 3, 0, address_map_constructor(FUNC(c64_ps64_cartridge_device::io_map), this)),
	m_enabled(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ps64_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ps64_cartridge_device::device_reset()
{
	m_enabled = m_sw1->read();
}


//-------------------------------------------------
//  memory_space_config - memory space configuration
//-------------------------------------------------

device_memory_interface::space_config_vector c64_ps64_cartridge_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(0, &m_io_space_config) };
}


//-------------------------------------------------
//  io_map - SSI-263 register map
//-------------------------------------------------

void c64_ps64_cartridge_device::io_map(address_map &map)
{
	map(0x00, 0x07).m(m_ssi263, FUNC(ssi263hle_device::map));
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_ps64_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_enabled)
		return data;

	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!io1)
	{
		data = space(0).read_byte(offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ps64_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_enabled && !io1)
	{
		space(0).write_byte(offset & 0x07, data);
	}
}


//-------------------------------------------------
//  c64_exrom_r - cartridge exrom line read
//-------------------------------------------------

int c64_ps64_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
    return !m_enabled;
}
