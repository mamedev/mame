// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Sony HB-F1XDJ and HB-F1XV machines.

*/

#include "emu.h"
#include "sony08.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_SONY08, msx_slot_sony08_device, "msx_slot_sony08", "MSX Internal SONY08")


msx_slot_sony08_device::msx_slot_sony08_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_SONY08, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
{
	for (int i = 0; i < 8; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = nullptr;
	}
	memset(m_sram, 0, sizeof(m_sram));
}


void msx_slot_sony08_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void msx_slot_sony08_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the SONY08 firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;

	m_nvram->set_base(m_sram, 0x4000);

	save_item(NAME(m_selected_bank));

	restore_banks();
}


void msx_slot_sony08_device::device_post_load()
{
	restore_banks();
}


void msx_slot_sony08_device::map_bank(int bank)
{
	if (bank < 2)
	{
		return;
	}

	// Special banks
	if (bank == 6 || bank == 7)
	{
		m_bank_base[bank] = m_rom + 0x80000 + (m_selected_bank[bank] * 0x800);
		return;
	}

	m_bank_base[bank] = m_rom + ((m_selected_bank[bank] * 0x2000) & 0xFFFFF);
	if (bank == 2)
	{
		if (m_selected_bank[bank] & 0x80)
		{
			m_bank_base[0] = m_sram;
			m_bank_base[1] = m_sram + 0x2000;
		}
		else
		{
			m_bank_base[0] = nullptr;
			m_bank_base[1] = nullptr;
		}
	}
}


void msx_slot_sony08_device::restore_banks()
{
	for (int i = 0; i < 8; i++)
	{
		map_bank(i);
	}
}


uint8_t msx_slot_sony08_device::read(offs_t offset)
{
	if (offset >= 0xc000)
	{
		return 0xFF;
	}

	if ((offset & 0xf000) == 0x7000 && (m_selected_bank[3] & 0x80))
	{
		return m_bank_base[6 + ((offset >> 11) & 0x01)][offset & 0x7ff];
	}

	const uint8_t *mem = m_bank_base[offset >> 13];

	if (mem)
	{
		return mem[offset & 0x1fff];
	}
	return 0xFF;
}


void msx_slot_sony08_device::write(offs_t offset, uint8_t data)
{
	if (offset < 0x4000)
	{
		if (m_bank_base[0] != nullptr)
		{
			m_sram[offset & 0x3fff] = data;
			return;
		}
	}

	switch (offset)
	{
		case 0x4FFF:
			m_selected_bank[2] = data;
			map_bank(2);
			break;

		case 0x6FFF:     // 6000-7FFF
			m_selected_bank[3] = data;
			map_bank(3);
			break;

		case 0x77FF:
			m_selected_bank[6] = data;
			map_bank(6);
			break;

		case 0x7FFF:
			m_selected_bank[7] = data;
			map_bank(7);
			break;

		case 0x8FFF:
			m_selected_bank[4] = data;
			map_bank(4);
			break;

		case 0xAFFF:
			m_selected_bank[5] = data;
			map_bank(5);
			break;

		default:
			logerror("Unhandled write %02x to %04x\n", data, offset);
			break;
	}
}
