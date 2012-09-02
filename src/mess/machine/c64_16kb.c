/**********************************************************************

    Commodore 64 16KB EPROM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_16kb.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_16KB = &device_creator<c64_16kb_cartridge_device>;


//-------------------------------------------------
//  ROM( c64_16kb )
//-------------------------------------------------

ROM_START( c64_16kb )
	ROM_REGION( 0x2000, "roml", 0 )
	ROM_CART_LOAD( "roml", 0x0000, 0x2000, ROM_MIRROR )

	ROM_REGION( 0x2000, "romh", 0 )
	ROM_CART_LOAD( "romh", 0x0000, 0x2000, ROM_MIRROR )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_16kb_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_16kb );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_16kb )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_16kb )
	MCFG_CARTSLOT_ADD("roml")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin,80")

	MCFG_CARTSLOT_ADD("romh")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin,a0,e0")
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

c64_16kb_cartridge_device::c64_16kb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_16KB, "C64 16KB EPROM cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_16kb_cartridge_device::device_start()
{
	m_roml = memregion("roml")->base();
	m_romh = memregion("romh")->base();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_16kb_cartridge_device::device_reset()
{
	UINT8 mode = device().ioport("SW1")->read();

	m_exrom = BIT(mode, 0);
	m_game = BIT(mode, 1);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_16kb_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!romh)
	{
		data = m_romh[offset & 0x1fff];
	}

	return data;
}
