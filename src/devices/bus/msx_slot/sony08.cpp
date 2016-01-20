// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Sony HB-F1XDJ and HB-F1XV machines.

*/

#include "emu.h"
#include "sony08.h"


const device_type MSX_SLOT_SONY08 = &device_creator<msx_slot_sony08_device>;


msx_slot_sony08_device::msx_slot_sony08_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_SLOT_SONY08, "MSX Internal SONY08", tag, owner, clock, "msx_slot_sony08", __FILE__)
	, msx_internal_slot_interface()
	, m_nvram(*this, "nvram")
	, m_region(nullptr)
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


static MACHINE_CONFIG_FRAGMENT( sony08 )
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


machine_config_constructor msx_slot_sony08_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sony08 );
}


void msx_slot_sony08_device::set_rom_start(device_t &device, const char *region, UINT32 offset)
{
	msx_slot_sony08_device &dev = downcast<msx_slot_sony08_device &>(device);

	dev.m_region = region;
	dev.m_region_offset = offset;
}


void msx_slot_sony08_device::device_start()
{
	assert(m_region != nullptr );

	memory_region *m_rom_region = owner()->memregion(m_region);

	// Sanity checks
	if (m_rom_region == nullptr )
	{
		fatalerror("Rom slot '%s': Unable to find memory region '%s'\n", tag().c_str(), m_region);
	}
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the SONY08 firmware\n", m_region);
	}

	m_rom = m_rom_region->base() + m_region_offset;

	m_nvram->set_base(m_sram, 0x4000);

	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_sony08_device::restore_banks), this));

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


READ8_MEMBER(msx_slot_sony08_device::read)
{
	if (offset >= 0xc000)
	{
		return 0xFF;
	}

	if ((offset & 0xf000) == 0x7000 && (m_selected_bank[3] & 0x80))
	{
		return m_bank_base[6 + ((offset >> 11) & 0x01)][offset & 0x7ff];
	}

	const UINT8 *mem = m_bank_base[offset >> 13];

	if (mem)
	{
		return mem[offset & 0x1fff];
	}
	return 0xFF;
}


WRITE8_MEMBER(msx_slot_sony08_device::write)
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
