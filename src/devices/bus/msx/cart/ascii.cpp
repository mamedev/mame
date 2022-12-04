// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "ascii.h"


DEFINE_DEVICE_TYPE(MSX_CART_ASCII8,       msx_cart_ascii8_device,       "msx_cart_ascii8",       "MSX Cartridge - ASCII8")
DEFINE_DEVICE_TYPE(MSX_CART_ASCII16,      msx_cart_ascii16_device,      "msx_cart_ascii16",      "MSX Cartridge - ASCII16")
DEFINE_DEVICE_TYPE(MSX_CART_ASCII8_SRAM,  msx_cart_ascii8_sram_device,  "msx_cart_ascii8_sram",  "MSX Cartridge - ASCII8 w/SRAM")
DEFINE_DEVICE_TYPE(MSX_CART_ASCII16_SRAM, msx_cart_ascii16_sram_device, "msx_cart_ascii16_sram", "MSX Cartridge - ASCII16 w/SRAM")


msx_cart_ascii8_device::msx_cart_ascii8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_ASCII8, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank%u", 0U)
	, m_bank_mask(0)
{
}

void msx_cart_ascii8_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(0);
}

image_init_result msx_cart_ascii8_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_ascii8_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	if (size > 256 * BANK_SIZE || size != banks * BANK_SIZE || (~(banks - 1) % banks))
	{
		message = "msx_cart_ascii8_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), BANK_SIZE);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->install_write_handler(0x6000, 0x67ff, write8smo_delegate(*this, FUNC(msx_cart_ascii8_device::bank_w<0>)));
	page(1)->install_write_handler(0x6800, 0x6fff, write8smo_delegate(*this, FUNC(msx_cart_ascii8_device::bank_w<1>)));
	page(1)->install_write_handler(0x7000, 0x77ff, write8smo_delegate(*this, FUNC(msx_cart_ascii8_device::bank_w<2>)));
	page(1)->install_write_handler(0x7800, 0x7fff, write8smo_delegate(*this, FUNC(msx_cart_ascii8_device::bank_w<3>)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);

	return image_init_result::PASS;
}

template <int Bank>
void msx_cart_ascii8_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
}



msx_cart_ascii16_device::msx_cart_ascii16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_ASCII16, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank%u", 0U)
	, m_bank_mask(0)
{
}

void msx_cart_ascii16_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}

image_init_result msx_cart_ascii16_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_ascii16_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	if (size > 256 * BANK_SIZE || size != banks * BANK_SIZE || (~(banks - 1) % banks))
	{
		message = "msx_cart_ascii16_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 2; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), BANK_SIZE);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	page(1)->install_write_handler(0x6000, 0x67ff, write8smo_delegate(*this, FUNC(msx_cart_ascii16_device::bank_w<0>)));
	page(1)->install_write_handler(0x7000, 0x77ff, write8smo_delegate(*this, FUNC(msx_cart_ascii16_device::bank_w<1>)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank[1]);

	return image_init_result::PASS;
}

template <int Bank>
void msx_cart_ascii16_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
}





msx_cart_ascii8_sram_device::msx_cart_ascii8_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_ASCII8_SRAM, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank%u", 0U)
	, m_view2(*this, "view2")
	, m_view3(*this, "view3")
	, m_bank_mask(0)
	, m_sram_select_mask(0)
{
}

void msx_cart_ascii8_sram_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(0);
	m_view2.select(0);
	m_view3.select(0);
}

image_init_result msx_cart_ascii8_sram_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_ascii8_sram_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (!cart_sram_region())
	{
		message = "msx_cart_ascii8_sram_device: Required region 'sram' was not found.";
		return image_init_result::FAIL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	if (size > 128 * BANK_SIZE || size != banks * BANK_SIZE || (~(banks - 1) % banks))
	{
		message = "msx_cart_ascii8_sram_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	if (cart_sram_region()->bytes() < 0x2000)
	{
		message = "msx_cart_ascii8_sram_device: Region 'sram' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bank_mask = banks - 1;
	m_sram_select_mask = banks;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), BANK_SIZE);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->install_write_handler(0x6000, 0x7fff, write8sm_delegate(*this, FUNC(msx_cart_ascii8_sram_device::mapper_write)));
	page(2)->install_view(0x8000, 0x9fff, m_view2);
	m_view2[0].install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	m_view2[1].install_ram(0x8000, 0x9fff, cart_sram_region()->base());
	page(2)->install_view(0xa000, 0xbfff, m_view3);
	m_view3[0].install_read_bank(0xa000, 0xbfff, m_rombank[3]);
	m_view3[1].install_ram(0xa000, 0xbfff, cart_sram_region()->base());

	return image_init_result::PASS;
}

void msx_cart_ascii8_sram_device::mapper_write(offs_t offset, u8 data)
{
	u8 bank = (offset / 0x800) & 0x03;
	if (data & m_sram_select_mask)
	{
		if (bank == 2)
			m_view2.select(1);
		if (bank == 3)
			m_view3.select(1);
	}
	else
	{
		if (bank == 2)
			m_view2.select(0);
		if (bank == 3)
			m_view3.select(0);

		m_rombank[bank]->set_entry(data & m_bank_mask);
	}
}




msx_cart_ascii16_sram_device::msx_cart_ascii16_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_ASCII16_SRAM, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank%u", 0U)
	, m_view(*this, "view")
	, m_bank_mask(0)
	, m_sram_select_mask(0)
{
}

void msx_cart_ascii16_sram_device::device_start()
{
}

void msx_cart_ascii16_sram_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_view.select(0);
	m_rombank[1]->set_entry(0);
}

image_init_result msx_cart_ascii16_sram_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_ascii16_sram_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (!cart_sram_region())
	{
		message = "msx_cart_ascii16_sram_device: Required region 'sram' was not found.";
		return image_init_result::FAIL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	if (size > 128 * BANK_SIZE || size != banks * BANK_SIZE || (~(banks - 1) % banks))
	{
		message = "msx_cart_ascii16_sram_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	if (cart_sram_region()->bytes() < 0x800)
	{
		message = "msx_cart_ascii16_sram_device: Region 'sram' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bank_mask = banks - 1;
	m_sram_select_mask = banks;

	for (int i = 0; i < 2; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), BANK_SIZE);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	page(1)->install_write_handler(0x6000, 0x67ff, write8smo_delegate(*this, FUNC(msx_cart_ascii16_sram_device::mapper_write_6000)));
	page(1)->install_write_handler(0x7000, 0x77ff, write8smo_delegate(*this, FUNC(msx_cart_ascii16_sram_device::mapper_write_7000)));
	page(2)->install_view(0x8000, 0xbfff, m_view);
	m_view[0].install_read_bank(0x8000, 0xbfff, m_rombank[1]);
	m_view[1].install_ram(0x8000, 0x87ff, 0x3800, cart_sram_region()->base());

	return image_init_result::PASS;
}

void msx_cart_ascii16_sram_device::mapper_write_6000(u8 data)
{
	m_rombank[0]->set_entry(data & m_bank_mask);
}

void msx_cart_ascii16_sram_device::mapper_write_7000(u8 data)
{
	if (data & m_sram_select_mask)
	{
		m_view.select(1);
	}
	else
	{
		m_view.select(0);
		m_rombank[1]->set_entry(data & m_bank_mask);
	}
}
