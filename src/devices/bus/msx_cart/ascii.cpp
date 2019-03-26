// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "ascii.h"


DEFINE_DEVICE_TYPE(MSX_CART_ASCII8,       msx_cart_ascii8_device,       "msx_cart_ascii8",       "MSX Cartridge - ASCII8")
DEFINE_DEVICE_TYPE(MSX_CART_ASCII16,      msx_cart_ascii16_device,      "msx_cart_ascii16",      "MSX Cartridge - ASCII16")
DEFINE_DEVICE_TYPE(MSX_CART_ASCII8_SRAM,  msx_cart_ascii8_sram_device,  "msx_cart_ascii8_sram",  "MSX Cartridge - ASCII8 w/SRAM")
DEFINE_DEVICE_TYPE(MSX_CART_ASCII16_SRAM, msx_cart_ascii16_sram_device, "msx_cart_ascii16_sram", "MSX Cartridge - ASCII16 w/SRAM")
DEFINE_DEVICE_TYPE(MSX_CART_MSXWRITE,     msx_cart_msxwrite_device,     "msx_cart_msxwrite",     "MSX Cartridge - MSXWRITE")


msx_cart_ascii8_device::msx_cart_ascii8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_ASCII8, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_ascii8_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_ascii8_device::device_post_load()
{
	restore_banks();
}


void msx_cart_ascii8_device::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		m_bank_base[i] = get_rom_base() + (m_selected_bank[i] & m_bank_mask ) * 0x2000;
	}
}


void msx_cart_ascii8_device::device_reset()
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
}


void msx_cart_ascii8_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( size > 256 * 0x2000 )
	{
		fatalerror("ascii8: ROM is too big\n");
	}

	uint16_t banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("ascii8: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


uint8_t msx_cart_ascii8_device::read_cart(offs_t offset)
{
	if ( offset >= 0x4000 && offset < 0xC000 )
	{
		return m_bank_base[(offset - 0x4000) >> 13][offset & 0x1fff];
	}
	return 0xff;
}


void msx_cart_ascii8_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x6000 && offset < 0x8000)
	{
		uint8_t bank = (offset / 0x800) & 0x03;

		m_selected_bank[bank] = data;
		m_bank_base[bank] = get_rom_base() + (m_selected_bank[bank] & m_bank_mask ) * 0x2000;
	}
}



msx_cart_ascii16_device::msx_cart_ascii16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_ASCII16, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 2; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_ascii16_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_ascii16_device::device_post_load()
{
	restore_banks();
}


void msx_cart_ascii16_device::restore_banks()
{
	for (int i = 0; i < 2; i++)
	{
		m_bank_base[i] = get_rom_base() + (m_selected_bank[i] & m_bank_mask) * 0x4000;
	}
}


void msx_cart_ascii16_device::device_reset()
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
}


void msx_cart_ascii16_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( size > 256 * 0x4000 )
	{
		fatalerror("ascii16: ROM is too big\n");
	}

	uint16_t banks = size / 0x4000;

	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		fatalerror("ascii16: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


uint8_t msx_cart_ascii16_device::read_cart(offs_t offset)
{
	if ( offset >= 0x4000 && offset < 0xC000 )
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}
	return 0xff;
}


void msx_cart_ascii16_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x6000 && offset < 0x6800)
	{
		m_selected_bank[0] = data;
		m_bank_base[0] = get_rom_base() + (m_selected_bank[0] & m_bank_mask) * 0x4000;
	}

	if (offset >= 0x7000 && offset < 0x7800)
	{
		m_selected_bank[1] = data;
		m_bank_base[1] = get_rom_base() + (m_selected_bank[1] & m_bank_mask) * 0x4000;
	}
}





msx_cart_ascii8_sram_device::msx_cart_ascii8_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_ASCII8_SRAM, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
	, m_sram_select_mask(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_ascii8_sram_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_ascii8_sram_device::device_post_load()
{
	restore_banks();
}


void msx_cart_ascii8_sram_device::setup_bank(uint8_t bank)
{
	if (m_selected_bank[bank] & ~(m_sram_select_mask | m_bank_mask))
	{
		// Nothing is mapped
		m_bank_base[bank] = nullptr;
	}
	else if (m_selected_bank[bank] & m_sram_select_mask)
	{
		// SRAM is selected
		m_bank_base[bank] = get_sram_base();
	}
	else
	{
		m_bank_base[bank] = get_rom_base() + (m_selected_bank[bank] & m_bank_mask ) * 0x2000;
	}
}


void msx_cart_ascii8_sram_device::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		setup_bank(i);
	}
}


void msx_cart_ascii8_sram_device::device_reset()
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
}


void msx_cart_ascii8_sram_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( size > 128 * 0x2000 )
	{
		fatalerror("ascii8_sram: ROM is too big\n");
	}

	uint16_t banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("ascii8_sram: Invalid ROM size\n");
	}

	if (get_sram_size() != 0x2000)
	{
		fatalerror("ascii8_sram: Unsupported SRAM size\n");
	}

	m_bank_mask = banks - 1;
	m_sram_select_mask = banks;

	restore_banks();
}


uint8_t msx_cart_ascii8_sram_device::read_cart(offs_t offset)
{
	if ( offset >= 0x4000 && offset < 0xC000 )
	{
		uint8_t *bank_base = m_bank_base[(offset - 0x4000) >> 13];

		if (bank_base != nullptr)
		{
			return bank_base[offset & 0x1fff];
		}
	}
	return 0xff;
}


void msx_cart_ascii8_sram_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x6000 && offset < 0x8000)
	{
		uint8_t bank = (offset / 0x800) & 0x03;

		m_selected_bank[bank] = data;
		setup_bank(bank);
	}
	else if (offset >= 0x8000 && offset < 0xc000)
	{
		uint8_t bank = (offset & 0x2000) ? 3 : 2;

		if ((m_selected_bank[bank] & m_sram_select_mask) && !(m_selected_bank[bank] & ~(m_sram_select_mask | m_bank_mask)))
		{
			// Write to SRAM
			m_bank_base[bank][offset & 0x1fff] = data;
		}
	}
}



msx_cart_ascii16_sram_device::msx_cart_ascii16_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_ASCII16_SRAM, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
	, m_sram_select_mask(0)
{
	for (int i = 0; i < 2; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_ascii16_sram_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_ascii16_sram_device::device_post_load()
{
	restore_banks();
}


void msx_cart_ascii16_sram_device::setup_bank(uint8_t bank)
{
	if (m_selected_bank[bank] & ~(m_sram_select_mask | m_bank_mask))
	{
		// Nothing is mapped
		m_bank_base[bank] = nullptr;
	}
	else if (m_selected_bank[bank] & m_sram_select_mask)
	{
		// SRAM is selected
		m_bank_base[bank] = get_sram_base();
	}
	else
	{
		m_bank_base[bank] = get_rom_base() + (m_selected_bank[bank] & m_bank_mask) * 0x4000;
	}
}


void msx_cart_ascii16_sram_device::restore_banks()
{
	for (int i = 0; i < 2; i++)
	{
		setup_bank(i);
	}
}


void msx_cart_ascii16_sram_device::device_reset()
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
}


void msx_cart_ascii16_sram_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( size > 128 * 0x4000 )
	{
		fatalerror("ascii16_sram: ROM is too big\n");
	}

	uint16_t banks = size / 0x4000;

	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		fatalerror("ascii16_sram: Invalid ROM size\n");
	}

	if (get_sram_size() != 0x800)
	{
		fatalerror("ascii16_sram: Unsupported SRAM size\n");
	}

	m_bank_mask = banks - 1;
	m_sram_select_mask = banks;

	restore_banks();
}


uint8_t msx_cart_ascii16_sram_device::read_cart(offs_t offset)
{
	if ( offset >= 0x4000 && offset < 0xC000 )
	{
		uint8_t bank = offset >> 15;

		if (m_bank_base[bank] != nullptr)
		{
			if (m_selected_bank[bank] & m_sram_select_mask)
			{
				return m_bank_base[bank][offset & 0x7ff];
			}
			else
			{
				return m_bank_base[bank][offset & 0x3fff];
			}
		}
	}
	return 0xff;
}


void msx_cart_ascii16_sram_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x6000 && offset < 0x6800)
	{
		m_selected_bank[0] = data;
		setup_bank(0);
	}

	if (offset >= 0x7000 && offset < 0x7800)
	{
		m_selected_bank[1] = data;
		setup_bank(1);
	}

	if (offset >= 0x8000 && offset < 0xc000)
	{
		if ((m_selected_bank[1] & m_sram_select_mask) && !(m_selected_bank[1] & ~(m_sram_select_mask | m_bank_mask)))
		{
			// Write to SRAM
			m_bank_base[1][offset & 0x7ff] = data;
		}
	}
}



msx_cart_msxwrite_device::msx_cart_msxwrite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MSXWRITE, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 2; i++)
	{
		m_selected_bank[i] = 0;
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_msxwrite_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_msxwrite_device::device_post_load()
{
	restore_banks();
}


void msx_cart_msxwrite_device::restore_banks()
{
	for (int i = 0; i < 2; i++)
	{
		m_bank_base[i] = get_rom_base() + (m_selected_bank[i] & m_bank_mask) * 0x4000;
	}
}


void msx_cart_msxwrite_device::device_reset()
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
}


void msx_cart_msxwrite_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( size > 256 * 0x4000 )
	{
		fatalerror("msxwrite: ROM is too big\n");
	}

	uint16_t banks = size / 0x4000;

	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		fatalerror("msxwrite: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


uint8_t msx_cart_msxwrite_device::read_cart(offs_t offset)
{
	if ( offset >= 0x4000 && offset < 0xC000 )
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}
	return 0xff;
}


void msx_cart_msxwrite_device::write_cart(offs_t offset, uint8_t data)
{
	// The rom writes to 6fff and 7fff for banking, unknown whether
	// other locations also trigger banking.
	switch (offset)
	{
		case 0x6fff:
			m_selected_bank[0] = data;
			m_bank_base[0] = get_rom_base() + (m_selected_bank[0] & m_bank_mask) * 0x4000;
			break;

		case 0x7fff:
			m_selected_bank[1] = data;
			m_bank_base[1] = get_rom_base() + (m_selected_bank[1] & m_bank_mask) * 0x4000;
			break;
	}
}
