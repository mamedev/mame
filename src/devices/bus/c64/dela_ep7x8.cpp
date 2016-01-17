// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dela 7x8K EPROM cartridge emulation

**********************************************************************/

#include "dela_ep7x8.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_DELA_EP7X8 = &device_creator<c64_dela_ep7x8_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_dela_ep7x8 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_dela_ep7x8 )
	MCFG_GENERIC_SOCKET_ADD("rom1", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_SOCKET_ADD("rom2", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_SOCKET_ADD("rom3", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_SOCKET_ADD("rom4", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_SOCKET_ADD("rom5", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_SOCKET_ADD("rom6", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_SOCKET_ADD("rom7", generic_linear_slot, nullptr)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
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

c64_dela_ep7x8_cartridge_device::c64_dela_ep7x8_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_DELA_EP7X8, "C64 Dela 7x8KB EPROM cartridge", tag, owner, clock, "ep7x8", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_eprom1(*this, "rom1"),
	m_eprom2(*this, "rom2"),
	m_eprom3(*this, "rom3"),
	m_eprom4(*this, "rom4"),
	m_eprom5(*this, "rom5"),
	m_eprom6(*this, "rom6"),
	m_eprom7(*this, "rom7"), m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_dela_ep7x8_cartridge_device::device_start()
{
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
		if (!BIT(m_bank, 1)) data |= m_eprom1->read_rom(space, addr);
		if (!BIT(m_bank, 2)) data |= m_eprom2->read_rom(space, addr);
		if (!BIT(m_bank, 3)) data |= m_eprom3->read_rom(space, addr);
		if (!BIT(m_bank, 4)) data |= m_eprom4->read_rom(space, addr);
		if (!BIT(m_bank, 5)) data |= m_eprom5->read_rom(space, addr);
		if (!BIT(m_bank, 6)) data |= m_eprom6->read_rom(space, addr);
		if (!BIT(m_bank, 7)) data |= m_eprom7->read_rom(space, addr);
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
