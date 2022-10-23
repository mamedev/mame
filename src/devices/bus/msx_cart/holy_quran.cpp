// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

Al-Alamiah Al-Qur'an Al-Karim
Only works on Arabian MSXes.

GCMK-16X PCB, 2 ROM chips, Yamaha XE297A0 mapper chip.

******************************************************************************/

#include "emu.h"
#include "holy_quran.h"


DEFINE_DEVICE_TYPE(MSX_CART_HOLY_QURAN, msx_cart_holy_quran_device, "msx_cart_holy_quran", "MSX Cartridge - Holy Quran")


msx_cart_holy_quran_device::msx_cart_holy_quran_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_HOLY_QURAN, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
{
}

void msx_cart_holy_quran_device::device_start()
{
	// zerofill
	m_decrypt = false;
	std::fill_n(&m_selected_bank[0], 4, 0);
	std::fill_n(&m_bank_base[0], 4, nullptr);

	// register for savestates
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_decrypt));

	// protection uses a simple rotation on databus, some lines inverted
	for (int i = 0; i < 0x100; i++)
		m_lookup_prot[i] = bitswap<8>(i,6,2,4,0,1,5,7,3) ^ 0x4d;
}

void msx_cart_holy_quran_device::initialize_cartridge()
{
	if (get_rom_size() != 0x100000)
	{
		fatalerror("holy_quran: Invalid ROM size\n");
	}

	restore_banks();
}

void msx_cart_holy_quran_device::device_reset()
{
	m_decrypt = false;

	std::fill_n(&m_selected_bank[0], 4, 0);
	restore_banks();
}

void msx_cart_holy_quran_device::restore_banks()
{
	for (int i = 0; i < 4; i++)
		m_bank_base[i] = get_rom_base() + (m_selected_bank[i] & 0x7f) * 0x2000;
}


// mapper interface

uint8_t msx_cart_holy_quran_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		uint8_t data = m_bank_base[(offset - 0x4000) >> 13][offset & 0x1fff];

		if (m_decrypt)
			return m_lookup_prot[data];

		// The decryption should actually start working after the first M1 cycle executing something from the cartridge.
		if (offset == ((m_rom[3] << 8) | m_rom[2]) && !machine().side_effects_disabled())
			m_decrypt = true;

		return data;
	}

	return 0xff;
}

void msx_cart_holy_quran_device::write_cart(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x5000: case 0x5400: case 0x5800: case 0x5c00:
			m_selected_bank[offset >> 10 & 3] = data;
			restore_banks();
			break;

		default:
			logerror("msx_cart_holy_quran_device: unhandled write %02x to %04x\n", data, offset);
			break;
	}
}
