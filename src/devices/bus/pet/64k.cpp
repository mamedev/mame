// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET 64KB RAM Expansion emulation

**********************************************************************/

#include "emu.h"
#include "64k.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PET_64K, pet_64k_expansion_device, "pet_64k", "PET 64KB RAM")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_64k_expansion_device - constructor
//-------------------------------------------------

pet_64k_expansion_device::pet_64k_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_64K, tag, owner, clock),
	device_pet_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x10000, ENDIANNESS_LITTLE),
	m_ctrl(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_64k_expansion_device::device_start()
{
	// state saving
	save_item(NAME(m_ctrl));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pet_64k_expansion_device::device_reset()
{
	m_ctrl = 0;
	update_window();
}

void pet_64k_expansion_device::device_post_load()
{
	update_window();
}

void pet_64k_expansion_device::ctrl_w(uint8_t data)
{
	if (BIT(m_ctrl, 7) && !BIT(m_ctrl, 1))
		m_ram[0x8000 | (BIT(m_ctrl, 3) << 14) | 0x3ff0] = data;

	m_ctrl = data;
	update_window();
}

void pet_64k_expansion_device::update_window()
{
	memory_view &window = m_slot->window();
	int const slot = BIT(m_ctrl, 7) ? 1 + BIT(m_ctrl, 5) + (BIT(m_ctrl, 6) << 1) : 0;

	if (slot)
	{
		auto &view = window[slot];
		offs_t const low_start = BIT(m_ctrl, 5) ? 0x9000 : 0x8000;
		offs_t const high_end = BIT(m_ctrl, 6) ? 0xe7ff : 0xffff;
		uint8_t *const low = &m_ram[0] + (BIT(m_ctrl, 2) << 14) + (low_start - 0x8000);
		uint8_t *const high = &m_ram[0] + 0x8000 + (BIT(m_ctrl, 3) << 14);

		if (BIT(m_ctrl, 0))
		{
			view.install_rom(low_start, 0xbfff, low);
			view.nop_write(low_start, 0xbfff);
		}
		else
			view.install_ram(low_start, 0xbfff, low);

		if (BIT(m_ctrl, 1))
		{
			view.install_rom(0xc000, high_end, high);
			view.nop_write(0xc000, high_end);
		}
		else
			view.install_ram(0xc000, high_end, high);

		if (BIT(m_ctrl, 6))
		{
			if (BIT(m_ctrl, 1))
			{
				view.install_rom(0xf000, 0xffff, high + 0x3000);
				view.nop_write(0xf000, 0xffff);
			}
			else
				view.install_ram(0xf000, 0xffff, high + 0x3000);
		}
	}

	window[slot].install_write_handler(0xfff0, 0xfff0, write8smo_delegate(*this, FUNC(pet_64k_expansion_device::ctrl_w)));
	window.select(slot);
}
