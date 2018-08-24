// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
  Emulation for the internal firmware mapper in the National FS-4600.
*/

#include "emu.h"
#include "fs4600.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_FS4600, msx_slot_fs4600_device, "msx_slot_fs4600", "MSX Internal FS4600 Firmware")


msx_slot_fs4600_device::msx_slot_fs4600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_FS4600, tag, owner, clock)
	, msx_internal_slot_interface()
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
	, m_selected_bank{ 0, 0, 0, 0 }
	, m_bank_base{ nullptr, nullptr, nullptr, nullptr }
	, m_sram_address(0)
	, m_control(0)
{
	memset(m_sram, 0, sizeof(m_sram));
}


void msx_slot_fs4600_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void msx_slot_fs4600_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the FS4600 firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;
	m_nvram->set_base(m_sram, 0x1000);

	save_item(NAME(m_selected_bank));
	save_item(NAME(m_sram_address));
	save_item(NAME(m_control));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_fs4600_device::restore_banks), this));

	restore_banks();
}


void msx_slot_fs4600_device::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		m_bank_base[i] = m_rom + ( ( m_selected_bank[i] * 0x4000 ) & 0x0fffff );
	}
}


READ8_MEMBER(msx_slot_fs4600_device::read)
{
	if ((m_control & 0x02) && ((offset & 0x3fff) == 0x3ffd))
	{
		return m_sram[m_sram_address++ & 0xfff];
	}
	if ((m_control & 0x04) && (offset& 0x7ff8) == 0x7ff0)
	{
		return m_selected_bank[(offset >> 1) & 0x03];
	}
	return m_bank_base[offset >> 14][offset & 0x3fff];
}


WRITE8_MEMBER(msx_slot_fs4600_device::write)
{
	if (offset == 0x7ff9)
	{
		m_control = data;
	}
	else
	{
		if (m_control & 0x02)
		{
			switch (offset & 0x3fff)
			{
				case 0x3ffa:
					m_sram_address = (m_sram_address & 0x00ffff) | (data << 16);
					break;

				case 0x3ffb:
					m_sram_address = (m_sram_address & 0xff00ff) | (data << 8);
					break;

				case 0x3ffc:
					m_sram_address = (m_sram_address & 0xffff00) | data;
					break;

				case 0x3ffd:
					m_sram[m_sram_address++ & 0xfff] = data;
					break;

				default:
					logerror("msx_slot_fs4600: Unhandled write %02x to %04x\n", data, offset);
					break;
			}
		}
		else
		{
			switch (offset)
			{
				case 0x6000:
					m_selected_bank[1] = data;
					m_bank_base[1] = m_rom + ( ( m_selected_bank[1] * 0x4000 ) & 0x0fffff );
					break;

				case 0x6400:
					m_selected_bank[0] = data;
					m_bank_base[0] = m_rom + ( ( m_selected_bank[0] * 0x4000 ) & 0x0fffff );
					break;

				case 0x7000:
					m_selected_bank[2] = data;
					m_bank_base[2] = m_rom + ( ( m_selected_bank[2] * 0x4000 ) & 0x0fffff );
					break;

				default:
					logerror("msx_slot_fs4600: Unhandled write %02x to %04x\n", data, offset);;
					break;
			}
		}
	}
}
