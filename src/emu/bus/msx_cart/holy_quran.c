// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "holy_quran.h"


const device_type MSX_CART_HOLY_QURAN = &device_creator<msx_cart_holy_quran>;


msx_cart_holy_quran::msx_cart_holy_quran(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_HOLY_QURAN, "MSX Cartridge - Holy Quran", tag, owner, clock, "msx_cart_holy_quran", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_decrypt(false)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = NULL;
	}

	/* protection uses a simple rotation on databus, some lines inverted:
	    D0   D4                 D4   D5
	    D1 ~ D3                 D5 ~ D2
	    D2 ~ D6                 D6   D7
	    D3 ~ D0                 D7   D1 */
	for (int i=0; i < 0x100; i++)
	{
		m_lookup_prot[i] = (((i << 4) & 0x50) | ((i >> 3) & 5) | ((i << 1) & 0xa0) | ((i << 2) & 8) | ((i >> 6) & 2)) ^ 0x4d;
	}
}


void msx_cart_holy_quran::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_decrypt));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_holy_quran::restore_banks), this));
}


void msx_cart_holy_quran::restore_banks()
{
	m_bank_base[0] = get_rom_base() + (m_selected_bank[0] & 0x7f) * 0x2000;
	m_bank_base[1] = get_rom_base() + (m_selected_bank[1] & 0x7f) * 0x2000;
	m_bank_base[2] = get_rom_base() + (m_selected_bank[2] & 0x7f) * 0x2000;
	m_bank_base[3] = get_rom_base() + (m_selected_bank[3] & 0x7f) * 0x2000;
}


void msx_cart_holy_quran::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = 0;
	}
}


void msx_cart_holy_quran::initialize_cartridge()
{
	if (get_rom_size() != 0x100000)
	{
		fatalerror("holy_quran: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_holy_quran::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		UINT8 data = m_bank_base[(offset - 0x4000) >> 13][offset & 0x1fff];

		if (m_decrypt)
		{
			return m_lookup_prot[data];
		}

		// The decryption should actually start working after the first M1 cycle executing something
		// from the cartridge.
		if (offset == ((m_rom[3] << 8) | m_rom[2]) && !space.debugger_access())
		{
			m_decrypt = true;
		}

		return data;
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_holy_quran::write_cart)
{
	switch (offset)
	{
		case 0x5000:
			m_selected_bank[0] = data;
			restore_banks();
			break;
		case 0x5400:
			m_selected_bank[1] = data;
			restore_banks();
			break;
		case 0x5800:
			m_selected_bank[2] = data;
			restore_banks();
			break;
		case 0x5c00:
			m_selected_bank[3] = data;
			restore_banks();
			break;
		default:
			logerror("msx_cart_holy_quran: unhandled write %02x to %04x\n", data, offset);
			break;
	}
}
