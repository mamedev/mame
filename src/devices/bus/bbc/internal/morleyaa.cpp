// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics 'AA' Master ROM Expansion Board

**********************************************************************/


#include "emu.h"
#include "morleyaa.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MORLEYAA, bbc_morleyaa_device, "bbc_morleyaa", "Morley Electronics 'AA' Master ROM Expansion Board");


//-------------------------------------------------
//  ROM( morleyaa )
//-------------------------------------------------

ROM_START(morleyaa)
	ROM_REGION(0x4000, "aa_rom", 0)
	ROM_LOAD("masterboard_200.rom", 0x0000, 0x4000, CRC(d93738ec) SHA1(c80f47e3f661b1636d21ff542c7a39a586008098))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_morleyaa_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot0-3 are unavailable when board is fitted, replaced by sockets on board */;
	//config.device_remove(":romslot8");

	/* 8 x 16K rom sockets */
	BBC_ROMSLOT16(config, m_rom[0], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[1], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[3], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, nullptr);
	/* 4 x 32K rom sockets */
	BBC_ROMSLOT32(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[11], bbc_rom_devices, nullptr);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_morleyaa_device::device_rom_region() const
{
	return ROM_NAME(morleyaa);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_morleyaa_device - constructor
//-------------------------------------------------

bbc_morleyaa_device::bbc_morleyaa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MORLEYAA, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_aa_rom(*this, "aa_rom")
	, m_rom(*this, "romslot%u", 0U)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_morleyaa_device::device_start()
{
	/* move internal roms to board */
	memcpy(m_region_swr->base() + 0x20000, m_aa_rom->base(), 0x4000);

	/* register for save states */
	save_item(NAME(m_romsel));
	save_item(NAME(m_banksel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_morleyaa_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 8:
		data = m_aa_rom->base()[offset];
		break;
	case 4:
		if (m_banksel & 0x20)
		{
			/* motherboard ram selected */
			data = m_mb_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
			switch (m_banksel & 0x07)
			{
			case 2:
				data = m_rom[0]->read(offset);
				break;
			case 4:
				data = m_rom[2]->read(offset);
				break;
			case 0:
				data = m_rom[4]->read(offset);
				break;
			case 6:
				data = m_rom[6]->read(offset);
				break;
			case 7:
				data = m_rom[8]->read(offset);
				break;
			case 3:
				data = m_rom[9]->read(offset);
				break;
			case 5:
				data = m_rom[10]->read(offset);
				break;
			case 1:
				data = m_rom[11]->read(offset);
				break;
			}
		}
		break;
	case 5:
		if (m_banksel & 0x20)
		{
			/* motherboard ram selected */
			data = m_mb_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
			switch (m_banksel & 0x07)
			{
			case 2:
				data = m_rom[1]->read(offset);
				break;
			case 4:
				data = m_rom[3]->read(offset);
				break;
			case 0:
				data = m_rom[5]->read(offset);
				break;
			case 6:
				data = m_rom[7]->read(offset);
				break;
			case 7:
				data = m_rom[8]->read(offset | 0x4000);
				break;
			case 3:
				data = m_rom[9]->read(offset | 0x4000);
				break;
			case 5:
				data = m_rom[10]->read(offset | 0x4000);
				break;
			case 1:
				data = m_rom[11]->read(offset | 0x4000);
				break;
			}
		}
		break;
	case 6:
		if (m_banksel & 0x40)
		{
			/* motherboard ram selected */
			data = m_mb_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
			switch (bitswap<8>(m_banksel, 2, 6, 5, 0, 1, 7, 3, 4) & 0x07)
			{
			case 2:
				data = m_rom[0]->read(offset);
				break;
			case 4:
				data = m_rom[2]->read(offset);
				break;
			case 0:
				data = m_rom[4]->read(offset);
				break;
			case 6:
				data = m_rom[6]->read(offset);
				break;
			case 7:
				data = m_rom[8]->read(offset);
				break;
			case 3:
				data = m_rom[9]->read(offset);
				break;
			case 5:
				data = m_rom[10]->read(offset);
				break;
			case 1:
				data = m_rom[11]->read(offset);
				break;
			}
		}
		break;
	case 7:
		if (m_banksel & 0x40)
		{
			/* motherboard ram selected */
			data = m_mb_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
			switch (bitswap<8>(m_banksel, 2, 6, 5, 0, 1, 7, 3, 4) & 0x07)
			{
			case 2:
				data = m_rom[1]->read(offset);
				break;
			case 4:
				data = m_rom[3]->read(offset);
				break;
			case 0:
				data = m_rom[5]->read(offset);
				break;
			case 6:
				data = m_rom[7]->read(offset);
				break;
			case 7:
				data = m_rom[8]->read(offset | 0x4000);
				break;
			case 3:
				data = m_rom[9]->read(offset | 0x4000);
				break;
			case 5:
				data = m_rom[10]->read(offset | 0x4000);
				break;
			case 1:
				data = m_rom[11]->read(offset | 0x4000);
				break;
			}
		}
		break;
	default:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel] && m_mb_rom[m_romsel]->present())
		{
			data = m_mb_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbc_morleyaa_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 8:
		if (offset == 0x3f00)
		{
			/* bank latch at &BF00 */
			m_banksel = data;
		}
		break;
	case 4: case 5:
		if (m_banksel & 0x20)
		{
			/* motherboard ram selected */
			m_mb_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
		}
		break;
	case 6: case 7:
		if (m_banksel & 0x40)
		{
			/* motherboard ram selected */
			m_mb_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
		}
		break;
	default:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel])
		{
			m_mb_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}
