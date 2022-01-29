// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Peartree MRxx00 ROM/RAM Boards

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Peartree_ROMExpansion.html

**********************************************************************/


#include "emu.h"
#include "peartree.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MR3000, bbc_mr3000_device, "bbc_mr3000", "Peartree MR3000 ROM Board");
DEFINE_DEVICE_TYPE(BBC_MR4200, bbc_mr4200_device, "bbc_mr4200", "Peartree MR4200 RAM Board");
DEFINE_DEVICE_TYPE(BBC_MR4300, bbc_mr4300_device, "bbc_mr4300", "Peartree MR4300 ROM/RAM Board");
DEFINE_DEVICE_TYPE(BBC_MR4800, bbc_mr4800_device, "bbc_mr4800", "Peartree MR4800 RAM Board");


//-------------------------------------------------
//  INPUT_PORTS( peartree )
//-------------------------------------------------

static INPUT_PORTS_START(peartree)
	PORT_START("wp")
	PORT_CONFNAME(0x01, 0x00, "Write Protect Sideways RAM")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_mr4200_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(peartree);
}

ioport_constructor bbc_mr4300_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(peartree);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_mr3000_device::device_add_mconfig(machine_config &config)
{
	/* 4 rom sockets */
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, nullptr);
}

void bbc_mr4200_device::device_add_mconfig(machine_config &config)
{
	/* 32K static ram */
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

void bbc_mr4300_device::device_add_mconfig(machine_config &config)
{
	/* 3 rom sockets, 32K static ram */
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

void bbc_mr4800_device::device_add_mconfig(machine_config &config)
{
	/* 128K static ram (battery backed) */
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "nvram").set_fixed_ram(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_mr3000_device - constructor
//-------------------------------------------------

bbc_mr3000_device::bbc_mr3000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}

bbc_mr3000_device::bbc_mr3000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_mr3000_device(mconfig, BBC_MR3000, tag, owner, clock)
{
}

bbc_mr4200_device::bbc_mr4200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_mr3000_device(mconfig, BBC_MR4200, tag, owner, clock)
	, m_wp(*this, "wp")
{
}

//-------------------------------------------------
//  bbc_mr4300_device - constructor
//-------------------------------------------------

bbc_mr4300_device::bbc_mr4300_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
	, m_wp(*this, "wp")
{
}

bbc_mr4300_device::bbc_mr4300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_mr4300_device(mconfig, BBC_MR4300, tag, owner, clock)
{
}

bbc_mr4800_device::bbc_mr4800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_mr4300_device(mconfig, BBC_MR4800, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_mr3000_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_romsel));
}

void bbc_mr4300_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_romsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_mr3000_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 8: case 9: case 10: case 11:
		/* motherboard rom sockets 0-3 are re-assigned to 8-11 */
		if (m_mb_rom[m_romsel & 0x03] && m_mb_rom[m_romsel & 0x03]->present())
		{
			data = m_mb_rom[m_romsel & 0x03]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14) - 0x20000];
		}
		break;
	case 12: case 13: case 14: case 15:
		/* expansion board sockets */
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbc_mr3000_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 8: case 9: case 10: case 11:
		/* motherboard rom sockets 0-3 are re-assigned to 8-11 */
		if (m_mb_rom[m_romsel & 0x03])
		{
			m_mb_rom[m_romsel & 0x03]->write(offset, data);
		}
		break;
	case 12: case 13: case 14: case 15:
		/* expansion board sockets */
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}


void bbc_mr4200_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 8: case 9: case 10: case 11:
		/* motherboard rom sockets 0-3 are re-assigned to 8-11 */
		if (m_mb_rom[m_romsel & 0x03])
		{
			m_mb_rom[m_romsel & 0x03]->write(offset, data);
		}
		break;
	case 12: case 13: case 14: case 15:
		/* expansion board sockets (if not write protected) */
		if (m_rom[m_romsel] && !m_wp->read())
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}


uint8_t bbc_mr4300_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 4: case 5: case 6: case 7:
		/* motherboard rom sockets 0-3 are re-assigned to 4-7 */
		if (m_mb_rom[m_romsel & 0x03] && m_mb_rom[m_romsel & 0x03]->present())
		{
			data = m_mb_rom[m_romsel & 0x03]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14) - 0x10000];
		}
		break;
	case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		/* expansion board sockets */
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbc_mr4300_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 4: case 5: case 6: case 7:
		/* motherboard rom sockets 0-3 are re-assigned to 4-7 */
		if (m_mb_rom[m_romsel & 0x03])
		{
			m_mb_rom[m_romsel & 0x03]->write(offset, data);
		}
		break;
	case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		/* expansion board sockets (if not write protected) */
		if (m_rom[m_romsel] && !m_wp->read())
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}
