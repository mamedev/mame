// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_kanji12.h"


DEFINE_DEVICE_TYPE(MSX_KANJI12, msx_kanji12_device, "msx_kanji12", "MSX Kanji12")

msx_kanji12_device::msx_kanji12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_KANJI12, tag, owner, clock)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_selected(false)
{
}


void msx_kanji12_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() != 0x20000)
	{
		fatalerror("Memory region '%s' has incorrect size for rom slot '%s'\n", m_rom_region.finder_tag(), tag());
	}

	save_item(NAME(m_selected));
	save_item(NAME(m_row));
	save_item(NAME(m_col));
	save_item(NAME(m_address));
}


u8 msx_kanji12_device::switched_read(offs_t offset)
{
	if (m_selected)
	{
		switch (offset)
		{
		case 0:
			// Manufacturer ID number register
			return MANUFACTURER_ID ^ 0xff;

		case 9:
			if (m_address < m_rom_region->bytes())
			{
				uint8_t data = m_rom_region->base()[m_address];
				if (!machine().side_effects_disabled())
					m_address++;
				return data;
			}
			break;

		default:
			logerror("msx_kanji12: unhandled read from offset %02x\n", offset);
			break;
		}
	}

	return 0xff;
}


void msx_kanji12_device::switched_write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		// Manufacturer ID number register
		m_selected = (data == MANUFACTURER_ID);
	}
	else if (m_selected)
	{
		switch (offset)
		{
		case 7:
			m_row = data;
			m_address = 0x800 + (m_row * 192 + m_col) * 18;
			break;

		case 8:
			m_col = data;
			m_address = 0x800 + (m_row * 192 + m_col) * 18;
			break;

		default:
			logerror("msx_kanji12: unhandled write %02x to offset %02x\n", data, offset);
			break;
		}
	}
}
