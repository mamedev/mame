// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Atari PBI/ECI slot interface

Parallel Bus Interface (PBI) is a 50-pin expansion on the back of all XL machines minus a1200xl

Enhanced Cartridge Interface ECI slot is a 14-pin replacement for XE machines,
aligned near the cartridge slot and where it takes the remaining address/data bus lines
(i.e. would occupt both slots at once)

TODO:
- IRQ line;
- EXSEL line;
- Audio input line;
- figure out a way to override main system graphics for the Atari 1090 80 Column Video Card
  (monochrome MC68B45 with 80x25 display)
- figure out of a way to knock off cartridge slot for ECI case;
- DIXX line for ECI slot (exclusive pin not present on PBI or just an alias?)

**************************************************************************************************/

#include "emu.h"
#include "slot.h"

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ATARI_PBI_SLOT, atari_pbi_slot_device, "atari_pbi_slot", "Atari Parallel Bus Interface slot")

atari_pbi_interface::atari_pbi_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "ataripbi")
	, m_slot(dynamic_cast<atari_pbi_slot_device *>(device.owner()))
{
}

atari_pbi_interface::~atari_pbi_interface()
{
}

atari_pbi_slot_device::atari_pbi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ATARI_PBI_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<atari_pbi_interface>(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_card(nullptr)
	, m_space_config("pbi_space", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor())
	, m_space(nullptr)
	, m_mpd_cb(*this)
{
}

void atari_pbi_slot_device::device_start()
{
	m_card = get_card_device();
	m_space = &space(0);
	m_space->unmap_value_high();
}

void atari_pbi_slot_device::device_reset()
{
	m_mpd_cb(0);
}

device_memory_interface::space_config_vector atari_pbi_slot_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_config)
	};
}

u8 atari_pbi_slot_device::read(offs_t offset)
{
	return space(0).read_byte(offset);
}

void atari_pbi_slot_device::write(offs_t offset, u8 data)
{
	space(0).write_byte(offset, data);
}

// Math overlay punch has
u8 atari_pbi_slot_device::read_d8xx(offs_t offset)
{
	return space(0).read_byte(offset + 0xd800);
}

void atari_pbi_slot_device::write_d8xx(offs_t offset, u8 data)
{
	space(0).write_byte(offset + 0xd800, data);
}

u8 atari_pbi_slot_device::pdvi_r(offs_t offset)
{
	if (m_card)
	{
		if (!machine().side_effects_disabled())
			LOG("PDVI read\n");
		return m_card->pdvi_slot_r();
	}
	return 0xff;
}

void atari_pbi_slot_device::pdvs_w(offs_t offset, u8 data)
{
	if (m_card)
	{
		// PBI/ECI has no real PDVS implementation, it's all handled by the card.
		const int mpd_line = m_card->pdvs_slot_w(data);
		LOG("PDVS: %02x mpd_line %d\n", data, mpd_line);
		m_mpd_cb(mpd_line);
	}
}


atari_pbi_card_device::atari_pbi_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, atari_pbi_interface(mconfig, *this)
{
}

void atari_pbi_card_device::device_start()
{

}


