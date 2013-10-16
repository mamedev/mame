// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dela 64KB EPROM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "dela_ep64.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_DELA_EP64 = &device_creator<c64_dela_ep64_cartridge_device>;


//-------------------------------------------------
//  ROM( c64_dela_ep64 )
//-------------------------------------------------

ROM_START( c64_dela_ep64 )
	ROM_REGION( 0x10000, "eprom", 0 )
	ROM_CART_LOAD( "rom1", 0x0000, 0x08000, ROM_MIRROR )
	ROM_CART_LOAD( "rom2", 0x8000, 0x08000, ROM_MIRROR )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_dela_ep64_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_dela_ep64 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_dela_ep64 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_dela_ep64 )
	MCFG_CARTSLOT_ADD("rom1")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_ADD("rom2")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_dela_ep64_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_dela_ep64 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_dela_ep64_cartridge_device - constructor
//-------------------------------------------------

c64_dela_ep64_cartridge_device::c64_dela_ep64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_DELA_EP64, "C64 Rex 64KB EPROM cartridge", tag, owner, clock, "c64_dela_ep64", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_eprom(*this, "eprom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_dela_ep64_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_reset));
	save_item(NAME(m_rom0_ce));
	save_item(NAME(m_rom1_ce));
	save_item(NAME(m_rom2_ce));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_dela_ep64_cartridge_device::device_reset()
{
	m_exrom = 0;

	m_reset = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_dela_ep64_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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

			if (!m_rom0_ce) data |= m_roml[offset & 0x1fff];
			if (!m_rom1_ce) data |= m_eprom->base()[0x0000 + addr];
			if (!m_rom2_ce) data |= m_eprom->base()[0x8000 + addr];
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_dela_ep64_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		/*

		    bit     description

		    0       IC6 _CE
		    1       IC5 _CE
		    2
		    3       IC4 _CE
		    4       A13
		    5       A14
		    6
		    7       EXROM

		*/

		m_reset = 0;

		m_rom0_ce = BIT(data, 3);
		m_rom1_ce = BIT(data, 1);
		m_rom2_ce = BIT(data, 0);

		m_bank = (data >> 4) & 0x03;

		m_exrom = BIT(data, 7);
	}
}
