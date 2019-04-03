// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "halnote.h"


DEFINE_DEVICE_TYPE(MSX_CART_HALNOTE, msx_cart_halnote_device, "msx_cart_halnote", "MSX Cartridge - Halnote")


msx_cart_halnote_device::msx_cart_halnote_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_HALNOTE, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank{ 0, 0, 0, 0, 0, 0, 0, 0 }
	, m_bank_base{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
{
}


void msx_cart_halnote_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_halnote_device::device_post_load()
{
	restore_banks();
}


void msx_cart_halnote_device::map_bank(int bank)
{
	if (bank < 2)
	{
		return;
	}

	// Special banks
	if (bank == 6 || bank == 7)
	{
		m_bank_base[bank] = get_rom_base() + 0x80000 + (m_selected_bank[bank] * 0x800);
		return;
	}

	m_bank_base[bank] = get_rom_base() + ((m_selected_bank[bank] * 0x2000) & 0xFFFFF);
	if (bank == 2)
	{
		if (m_selected_bank[bank] & 0x80)
		{
			m_bank_base[0] = get_sram_base();
			m_bank_base[1] = get_sram_base() + 0x2000;
		}
		else
		{
			m_bank_base[0] = nullptr;
			m_bank_base[1] = nullptr;
		}
	}
}


void msx_cart_halnote_device::restore_banks()
{
	for (int i = 0; i < 8; i++)
	{
		map_bank(i);
	}
}


void msx_cart_halnote_device::device_reset()
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
}


void msx_cart_halnote_device::initialize_cartridge()
{
	if (get_rom_size() != 0x100000)
	{
		fatalerror("halnote: Invalid ROM size\n");
	}

	restore_banks();
}


uint8_t msx_cart_halnote_device::read_cart(offs_t offset)
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
	return 0xff;
}


void msx_cart_halnote_device::write_cart(offs_t offset, uint8_t data)
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
			logerror("msx_cart_halnote_device: Unhandled write %02x to %04x\n", data, offset);
			break;
	}
}
