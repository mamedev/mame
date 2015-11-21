// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dela 64KB EPROM cartridge emulation

**********************************************************************/

#include "dela_ep64.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_DELA_EP64 = &device_creator<c64_dela_ep64_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_dela_ep64 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_dela_ep64 )
	MCFG_GENERIC_SOCKET_ADD("eprom1", generic_linear_slot, NULL)
	MCFG_GENERIC_EXTENSIONS("rom,bin")
	MCFG_GENERIC_SOCKET_ADD("eprom2", generic_linear_slot, NULL)
	MCFG_GENERIC_EXTENSIONS("rom,bin")
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
	m_eprom1(*this, "eprom1"),
	m_eprom2(*this, "eprom2"), m_bank(0), m_reset(0), m_rom0_ce(0), m_rom1_ce(0), m_rom2_ce(0)
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
			if (!m_rom1_ce) data |= m_eprom1->read_rom(space, addr);
			if (!m_rom2_ce) data |= m_eprom2->read_rom(space, addr);
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
