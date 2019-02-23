// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dela 7x8K EPROM cartridge emulation

**********************************************************************/

#include "emu.h"
#include "dela_ep7x8.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_DELA_EP7X8, c64_dela_ep7x8_cartridge_device, "c64_dela_ep7x8", "C64 Dela 7x8KB EPROM cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_dela_ep7x8_cartridge_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SOCKET(config, m_eprom[0], generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, m_eprom[1], generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, m_eprom[2], generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, m_eprom[3], generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, m_eprom[4], generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, m_eprom[5], generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, m_eprom[6], generic_linear_slot, nullptr, "bin,rom");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_dela_ep7x8_cartridge_device - constructor
//-------------------------------------------------

c64_dela_ep7x8_cartridge_device::c64_dela_ep7x8_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_DELA_EP7X8, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_eprom(*this, "rom%u", 1U)
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

uint8_t c64_dela_ep7x8_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = offset & 0x1fff;

		if (!BIT(m_bank, 0)) data |= m_roml[addr];
		if (!BIT(m_bank, 1)) data |= m_eprom[0]->read_rom(addr);
		if (!BIT(m_bank, 2)) data |= m_eprom[1]->read_rom(addr);
		if (!BIT(m_bank, 3)) data |= m_eprom[2]->read_rom(addr);
		if (!BIT(m_bank, 4)) data |= m_eprom[3]->read_rom(addr);
		if (!BIT(m_bank, 5)) data |= m_eprom[4]->read_rom(addr);
		if (!BIT(m_bank, 6)) data |= m_eprom[5]->read_rom(addr);
		if (!BIT(m_bank, 7)) data |= m_eprom[6]->read_rom(addr);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_dela_ep7x8_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_bank = data;

		m_exrom = (data == 0xff);
	}
}
