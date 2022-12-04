// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "hfox.h"


DEFINE_DEVICE_TYPE(MSX_CART_HFOX, msx_cart_hfox_device, "msx_cart_hfox", "MSX Cartridge - Hurry Fox")


msx_cart_hfox_device::msx_cart_hfox_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_HFOX, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank%u", 0U)
{
}

void msx_cart_hfox_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}

image_init_result msx_cart_hfox_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_hfox_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x8000;

	if (size > 256 * 0x8000 || size < 0x10000 || size != banks * 0x8000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_hfox_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bank_mask = banks - 1;

	m_rombank[0]->configure_entries(0, 4, cart_rom_region()->base(), 0x8000);
	m_rombank[1]->configure_entries(0, 4, cart_rom_region()->base() + 0x4000, 0x8000);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	page(1)->install_write_handler(0x6000, 0x6000, write8smo_delegate(*this, FUNC(msx_cart_hfox_device::bank_w<0>)));
	page(1)->install_write_handler(0x7000, 0x7000, write8smo_delegate(*this, FUNC(msx_cart_hfox_device::bank_w<1>)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank[1]);

	return image_init_result::PASS;
}

template <int Bank>
void msx_cart_hfox_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
}
