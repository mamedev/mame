// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Panasonic FS-A1WX andFS-A1WSX machines.

Todo:
- Anything besides the basic mapping
- SRAM?
*/

#include "emu.h"
#include "panasonic08.h"


const device_type MSX_SLOT_PANASONIC08 = &device_creator<msx_slot_panasonic08_device>;


msx_slot_panasonic08_device::msx_slot_panasonic08_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_SLOT_PANASONIC08, "MSX Internal Panasonic08", tag, owner, clock, "msx_slot_panasonic08", __FILE__)
	, msx_internal_slot_interface()
	, m_nvram(*this, "nvram")
	, m_region(NULL)
	, m_region_offset(0)
	, m_rom(NULL)
	, m_control(0)
{
	for (int i = 0; i < 8; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = 0;
	}
}


static MACHINE_CONFIG_FRAGMENT( panasonic08 )
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


machine_config_constructor msx_slot_panasonic08_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( panasonic08 );
}


void msx_slot_panasonic08_device::set_rom_start(device_t &device, const char *region, UINT32 offset)
{
	msx_slot_panasonic08_device &dev = downcast<msx_slot_panasonic08_device &>(device);

	dev.m_region = region;
	dev.m_region_offset = offset;
}


void msx_slot_panasonic08_device::device_start()
{
	assert(m_region != NULL );

	memory_region *m_rom_region = owner()->memregion(m_region);

	// Sanity checks
	if (m_rom_region == NULL )
	{
		fatalerror("Rom slot '%s': Unable to find memory region '%s'\n", tag(), m_region);
	}
	if (m_rom_region->bytes() < m_region_offset + 0x200000)
	{
		fatalerror("Memory region '%s' is too small for the FS4600 firmware\n", m_region);
	}

	m_sram.resize(0x4000);

	m_nvram->set_base(&m_sram[0], 0x4000);

	m_rom = m_rom_region->base() + m_region_offset;

	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_panasonic08_device::restore_banks), this));

	restore_banks();
}


void msx_slot_panasonic08_device::map_bank(int bank)
{
	if (m_selected_bank[bank] >= 0x80 && m_selected_bank[bank] < 0x84)   // Are these banks were sram is present? Mirroring?
	{
		logerror("panasonic08: mapping bank %d to sram\n", bank);
		m_bank_base[bank] = &m_sram[((m_selected_bank[bank] & 0x7f) * 0x2000) & 0x3fff];
	}
	else
	{
		m_bank_base[bank] = m_rom + ( ( m_selected_bank[bank] * 0x2000 ) & 0x1fffff );
	}
}


void msx_slot_panasonic08_device::restore_banks()
{
	for (int i = 0; i < 8; i++)
	{
		map_bank(i);
	}
}


READ8_MEMBER(msx_slot_panasonic08_device::read)
{
	if (m_control & 0x04)
	{
		// 7ff0 - 6000
		// 7ff1 - 6400
		// 7ff2 - 6800
		// 7ff3 - 6c00
		// 7ff4 - 7000
		// 7ff5 - 7800
		if (offset >= 0x7ff0 && offset < 0x7ff6)     // maybe 7ff8 would make more sense here??
		{
			return m_selected_bank[offset - 0x7ff0];
		}
	}
	return m_bank_base[offset >> 13][offset & 0x1fff];
}


WRITE8_MEMBER(msx_slot_panasonic08_device::write)
{
	if ((offset & 0xc000) == 0x8000 || (offset & 0xc000) == 0x0000)
	{
		UINT8 bank = m_selected_bank[offset >> 13];
		if (bank >= 0x80 && bank < 0x84)   // Are these banks were sram is present? Mirroring?
		{
			logerror("msx_slot_panasonic08: writing %02x to sram %04x, bank = %02x\n", data, offset & 0x1fff, bank);
			m_sram[((bank & 0x01) * 0x2000) + (offset & 0x1fff)] = data;
		}
		return;
	}

	switch (offset)
	{
		case 0x6000:    /* Switched 0x0000-0x1FFF */
			m_selected_bank[0] = data;
			map_bank(0);
			break;

		case 0x6400:    /* Switches 0x2000-0x3FFF */
			m_selected_bank[1] = data;
			map_bank(1);
			break;

		case 0x6800:    /* Switches 0x4000-0x5FFF */
			m_selected_bank[2] = data;
			map_bank(2);
			break;

		case 0x6c00:    /* Switches 0x6000-0x7FFF */
			m_selected_bank[3] = data;
			map_bank(3);
			break;

		case 0x7000:    /* Switches 0x8000-0x9FFF */
			m_selected_bank[4] = data;
			map_bank(4);
			break;

		case 0x7800:    /* Switches 0xA000-0xBFFF */
			m_selected_bank[5] = data;
			map_bank(5);
			break;

		case 0x7ff9:
			m_control = data;
			break;

		default:
			logerror("msx_slot_panasonic08: Unhandled write %02x to %04x\n", data, offset);
			break;
	}
}
