// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

	SNK NEO-ZMC Memory controller
	reference : https://wiki.neogeodev.org/index.php?title=NEO-ZMC

***************************************************************************/

#include "emu.h"
#include "neo_zmc.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

void neo_zmc_device::banked_map(address_map &map)
{
	map(0x0000, 0xf7ff).rw(FUNC(neo_zmc_device::banked_space_r), FUNC(neo_zmc_device::banked_space_w));
}

// device type definition
DEFINE_DEVICE_TYPE(NEO_ZMC, neo_zmc_device, "neo_zmc", "SNK NEO-ZMC Memory controller")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neo_zmc_device - constructor
//-------------------------------------------------

neo_zmc_device::neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NEO_ZMC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 19) // 19 bit address (max 256 of 0x800 bank)
	//, m_data_config("data", ENDIANNESS_LITTLE, 8, 22) // 22 bit address (max 256 of 0x4000 bank)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neo_zmc_device::device_start()
{
	m_data = &space(0);
	// Find our direct access
	m_cache = space().cache<0, 0, ENDIANNESS_LITTLE>();

	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neo_zmc_device::device_reset()
{
	std::fill(std::begin(m_bank), std::end(m_bank), 0);
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector neo_zmc_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 neo_zmc_device::banked_space_r(offs_t offset)
{
	if (offset & 0x8000) // SDA15(0x8000-0xf7ff)
	{
		if (offset & 0x4000) // SDA14(0xc000-0xf7ff)
		{
			if (offset & 0x2000) // SDA13(0xe000-0xf7ff)
			{
				if (offset & 0x1000) // SDA12(0xf000-0xf7ff)
				{
					return m_cache->read_byte((m_bank[0] << 11) | (offset & 0x7ff));
				}
				// 0xe000-0xefff
				return m_cache->read_byte((m_bank[1] << 12) | (offset & 0xfff));
			}
			// 0xc000-dfff
			return m_cache->read_byte((m_bank[2] << 13) | (offset & 0x1fff));
		}
		// 0x8000-0xbfff
		return m_cache->read_byte((m_bank[3] << 14) | (offset & 0x3fff));
	}
	// 0x0000-0x7fff (fixed)
	return m_cache->read_byte(offset);
}

void neo_zmc_device::banked_space_w(offs_t offset, u8 data)
{
	if (offset & 0x8000) // SDA15(0x8000-0xf7ff)
	{
		if (offset & 0x4000) // SDA14(0xc000-0xf7ff)
		{
			if (offset & 0x2000) // SDA13(0xe000-0xf7ff)
			{
				if (offset & 0x1000) // SDA12(0xf000-0xf7ff)
				{
					m_data->write_byte((m_bank[0] << 11) | (offset & 0x7ff), data);
				}
				// 0xe000-0xefff
				m_data->write_byte((m_bank[1] << 12) | (offset & 0xfff), data);
			}
			// 0xc000-dfff
			m_data->write_byte((m_bank[2] << 13) | (offset & 0x1fff), data);
		}
		// 0x8000-0xbfff
		m_data->write_byte((m_bank[3] << 14) | (offset & 0x3fff), data);
	}
	// 0x0000-0x7fff (fixed)
	m_data->write_byte(offset, data);
}

u8 neo_zmc_device::set_bank_r(offs_t offset)
{
	// SDA0-1 for bank windows, SDA8-15 for bank number
	if (!machine().side_effects_disabled())
		m_bank[offset & 0x3] = (offset >> 8);
	return 0;
}
