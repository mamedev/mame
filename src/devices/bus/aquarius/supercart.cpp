// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Aquarius SuperCart Cartridge

***************************************************************************/

#include "emu.h"
#include "supercart.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_SC1, aquarius_sc1_device, "aquarius_sc1", "Aquarius SuperCart I Cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_sc1_device - constructor
//-------------------------------------------------

aquarius_sc1_device::aquarius_sc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_SC1, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
	, m_bank(0)
	, m_mode(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_sc1_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_mode));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aquarius_sc1_device::device_reset()
{
	static const unsigned char SC08_HEADER[16] = { 0x53, 0x43, 0x30, 0x38, 0x4b, 0x9c, 0xb5, 0xb0, 0xa8, 0x6c, 0xac, 0x64, 0xcc, 0xa8, 0x06, 0x70 };
	static const unsigned char SC16_HEADER[16] = { 0x53, 0x43, 0x31, 0x36, 0x4b, 0x9c, 0xb5, 0xb0, 0xa8, 0x6c, 0xac, 0x64, 0xcc, 0xa8, 0x08, 0x70 };

	uint8_t* header = get_rom_base() + get_rom_size() - 0x2000;

	// select bank switching mode
	if (!memcmp(header, SC08_HEADER, 16))
		m_mode = 0;
	else if (!memcmp(header, SC16_HEADER, 16))
		m_mode = 1;
	else
		fatalerror("Invalid SuperCart mode header\n");
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t aquarius_sc1_device::mreq_ce_r(offs_t offset)
{
	switch (m_mode)
	{
	case 0: // 8K Mode
		if (offset & 0x2000)
		{
			offset |= get_rom_size() - 0x2000;
		}
		else
		{
			offset |= m_bank << 13;
		}
		break;

	case 1: // 16K Mode
		offset |= (m_bank & 0x7e) << 13;
		break;
	}

	return get_rom_base()[offset & (get_rom_size() - 1)];
}

void aquarius_sc1_device::mreq_ce_w(offs_t offset, uint8_t data)
{
	if (offset & 0x2000)
	{
		m_bank = data & 0x7f;
	}
}
