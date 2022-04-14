// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_s1985.h"

const uint8_t manufacturer_id = 0xfe;

DEFINE_DEVICE_TYPE(MSX_S1985, msx_s1985_device, "msx_s1985", "MSX-Engine S1985")

msx_s1985_device::msx_s1985_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_S1985, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_selected(false)
	, m_backup_ram_address(0)
	, m_color1(0)
	, m_color2(0)
	, m_pattern(0)
{
}


void msx_s1985_device::device_start()
{
	save_item(NAME(m_selected));
	save_item(NAME(m_backup_ram_address));
	save_item(NAME(m_backup_ram));
	save_item(NAME(m_color1));
	save_item(NAME(m_color2));
	save_item(NAME(m_pattern));
}


void msx_s1985_device::nvram_default()
{
	memset(m_backup_ram, 0xff, sizeof(m_backup_ram));
}


bool msx_s1985_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_backup_ram, sizeof(m_backup_ram), actual) && actual == sizeof(m_backup_ram);
}


bool msx_s1985_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_backup_ram, sizeof(m_backup_ram), actual) && actual == sizeof(m_backup_ram);
}


uint8_t msx_s1985_device::switched_read(offs_t offset)
{
	if (m_selected)
	{
		switch (offset)
		{
		case 0:
			/// Manufacturer ID number register
			return manufacturer_id ^ 0xff;

		case 2:
			/// Back-up RAM read
			return m_backup_ram[m_backup_ram_address];

		case 7:
		{
			// Pattern and foreground/background color read
			uint8_t data = (m_pattern & 0x80) ? m_color2 : m_color1;

			if(!machine().side_effects_disabled())
				m_pattern = (m_pattern << 1) | (m_pattern >> 7);

			return data;
		}

		default:
			printf("msx_s1985: unhandled read from offset %02x\n", offset);
			break;
		}
	}

	return 0xff;
}


void msx_s1985_device::switched_write(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		/// Manufacturer ID number register
		m_selected = (data == manufacturer_id);
	}
	else if (m_selected)
	{
		switch (offset)
		{
		case 1:
			/// Back-up RAM address latch
			m_backup_ram_address = data & 0x0f;
			break;

		case 2:
			/// Back-up RAM write
			m_backup_ram[m_backup_ram_address] = data;
			break;

		case 6:
			// Foreground/background color write
			m_color2 = m_color1;
			m_color1 = data;
			break;

		case 7:
			// Pattern write
			m_pattern = data;
			break;

		default:
			printf("msx_s1985: unhandled write %02x to offset %02x\n", data, offset);
			break;
		}
	}
}
