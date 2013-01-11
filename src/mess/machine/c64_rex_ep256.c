/**********************************************************************

    Rex Datentechnik 256KB EPROM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_rex_ep256.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_REX_EP256 = &device_creator<c64_rex_ep256_cartridge_device>;


//-------------------------------------------------
//  ROM( c64_rex_ep256 )
//-------------------------------------------------

ROM_START( c64_rex_ep256 )
	ROM_REGION( 0x40000, "rom", 0 )
	ROM_CART_LOAD( "rom1", 0x00000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom2", 0x08000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom3", 0x10000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom4", 0x18000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom5", 0x20000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom6", 0x28000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom7", 0x30000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom8", 0x38000, 0x08000, ROM_MIRROR )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_rex_ep256_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_rex_ep256 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_rex_ep256 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_rex_ep256 )
	MCFG_CARTSLOT_ADD("rom1")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom2")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom3")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom4")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom5")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom6")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom7")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom8")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_rex_ep256_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_rex_ep256 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_rex_ep256_cartridge_device - constructor
//-------------------------------------------------

c64_rex_ep256_cartridge_device::c64_rex_ep256_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_REX_EP256, "C64 Rex 256KB EPROM cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_rex_ep256_cartridge_device::device_start()
{
	m_rom = memregion("rom")->base();

	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_reset));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_rex_ep256_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_reset = 1;
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_rex_ep256_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		if (m_reset)
		{
			data = m_roml[offset & 0x1fff];
		}
		else
		{
			offs_t addr = (m_bank << 13) | (offset & 0x1fff);
			data = m_rom[addr];
		}
	}
	else if (!io2)
	{
		if ((offset & 0xf0) == 0xc0)
		{
			m_exrom = 1;
		}
		else if ((offset & 0xf0) == 0xe0)
		{
			m_exrom = 0;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_rex_ep256_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && ((offset & 0xf0) == 0xa0))
	{
		/*

		    bit     description

		    0       socket selection bit 0
		    1       socket selection bit 1
		    2       socket selection bit 2
		    3
		    4
		    5       bank selection bit 0
		    6       bank selection bit 1
		    7

		*/

		m_reset = 0;

		m_bank = ((data & 0x07) << 2) | ((data >> 5) & 0x03);
	}
}
