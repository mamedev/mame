// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rex Datentechnik 256KB EPROM cartridge emulation

**********************************************************************/

#include "emu.h"
#include "rex_ep256.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_REX_EP256, c64_rex_ep256_cartridge_device, "rexexp256", "C64 Rex 256KB EPROM cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_rex_ep256_cartridge_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SOCKET(config, "rom1", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom2", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom3", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom4", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom5", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom6", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom7", generic_linear_slot, nullptr, "bin,rom");
	GENERIC_SOCKET(config, "rom8", generic_linear_slot, nullptr, "bin,rom");
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_rex_ep256_cartridge_device - constructor
//-------------------------------------------------

c64_rex_ep256_cartridge_device::c64_rex_ep256_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_REX_EP256, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
	for (int i = 0; i < 8; i++)
	{
		char str[6];
		sprintf(str, "rom%i", i + 1);
		m_eproms[i] = subdevice<generic_slot_device>(str);
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_rex_ep256_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_reset));
	save_item(NAME(m_socket));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_rex_ep256_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_reset = 1;
	m_bank = 0;
	m_socket = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_rex_ep256_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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
			data = m_eproms[m_socket]->read_rom(addr);
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

void c64_rex_ep256_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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

		m_socket = data & 0x07;
		m_bank = (data >> 5) & 0x03;
	}
}
