// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 16KB EPROM cartridge emulation

**********************************************************************/

#include "16kb.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_16KB = &device_creator<c64_16kb_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_16kb )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_16kb )
	MCFG_GENERIC_CARTSLOT_ADD("roml", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("rom,bin,80")

	MCFG_GENERIC_CARTSLOT_ADD("romh", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("rom,bin,a0,e0")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_16kb_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_16kb );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_16kb )
//-------------------------------------------------

static INPUT_PORTS_START( c64_16kb )
	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Mode" )
	PORT_DIPSETTING(    0x03, "Off" )
	PORT_DIPSETTING(    0x02, "8 KB" )
	PORT_DIPSETTING(    0x00, "16 KB" )
	PORT_DIPSETTING(    0x01, "Ultimax" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_16kb_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_16kb );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_16kb_cartridge_device - constructor
//-------------------------------------------------

c64_16kb_cartridge_device::c64_16kb_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_16KB, "C64 16KB EPROM cartridge", tag, owner, clock, "c64_16kb", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_sw1(*this, "SW1"),
	m_low(*this, "roml"),
	m_high(*this, "romh")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_16kb_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_16kb_cartridge_device::device_reset()
{
	UINT8 mode = m_sw1->read();

	m_exrom = BIT(mode, 0);
	m_game = BIT(mode, 1);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_16kb_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_low->read_rom(space, offset & 0x1fff);
	}
	else if (!romh)
	{
		data = m_high->read_rom(space, offset & 0x1fff);
	}

	return data;
}
