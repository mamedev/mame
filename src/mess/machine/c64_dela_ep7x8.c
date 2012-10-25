/**********************************************************************

    Dela 7x8K EPROM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_dela_ep7x8.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_DELA_EP7X8 = &device_creator<c64_dela_ep7x8_cartridge_device>;


//-------------------------------------------------
//  ROM( c64_dela_ep7x8 )
//-------------------------------------------------

ROM_START( c64_dela_ep7x8 )
	ROM_REGION( 0x10000, "rom", 0 )
	ROM_CART_LOAD( "rom1", 0x2000, 0x2000, ROM_MIRROR )
	ROM_CART_LOAD( "rom2", 0x4000, 0x2000, ROM_MIRROR )
	ROM_CART_LOAD( "rom3", 0x6000, 0x2000, ROM_MIRROR )
	ROM_CART_LOAD( "rom4", 0x8000, 0x2000, ROM_MIRROR )
	ROM_CART_LOAD( "rom5", 0xa000, 0x2000, ROM_MIRROR )
	ROM_CART_LOAD( "rom6", 0xc000, 0x2000, ROM_MIRROR )
	ROM_CART_LOAD( "rom7", 0xe000, 0x2000, ROM_MIRROR )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_dela_ep7x8_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_dela_ep7x8 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_dela_ep7x8 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_dela_ep7x8 )
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
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_dela_ep7x8_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_dela_ep7x8 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_dela_ep7x8_cartridge_device - constructor
//-------------------------------------------------

c64_dela_ep7x8_cartridge_device::c64_dela_ep7x8_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_DELA_EP7X8, "C64 Dela 7x8KB EPROM cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_dela_ep7x8_cartridge_device::device_start()
{
	m_rom = memregion("rom")->base();

	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_dela_ep7x8_cartridge_device::device_reset()
{
	m_bank = 0xfe;
	m_exrom = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_dela_ep7x8_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = offset & 0x1fff;

		if (!BIT(m_bank, 0)) data |= m_roml[addr];
		if (!BIT(m_bank, 1)) data |= m_rom[0x2000 + addr];
		if (!BIT(m_bank, 2)) data |= m_rom[0x4000 + addr];
		if (!BIT(m_bank, 3)) data |= m_rom[0x6000 + addr];
		if (!BIT(m_bank, 4)) data |= m_rom[0x8000 + addr];
		if (!BIT(m_bank, 5)) data |= m_rom[0xa000 + addr];
		if (!BIT(m_bank, 6)) data |= m_rom[0xc000 + addr];
		if (!BIT(m_bank, 7)) data |= m_rom[0xe000 + addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_dela_ep7x8_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_bank = data;

		m_exrom = (data == 0xff);
	}
}
