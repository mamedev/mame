// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "super_swangi.h"


DEFINE_DEVICE_TYPE(MSX_CART_SUPER_SWANGI, msx_cart_super_swangi_device, "msx_cart_super_swangi", "MSX Cartridge - Super Swangi")


msx_cart_super_swangi_device::msx_cart_super_swangi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_SUPER_SWANGI, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank")
{
}

image_init_result msx_cart_super_swangi_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_super_swangi_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (cart_rom_region()->bytes() < 0x10000)
	{
		message = "msx_cart_super_swangi_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_rombank->configure_entries(0, 4, cart_rom_region()->base(), 0x4000);

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);
	page(2)->install_write_handler(0x8000, 0x8000, write8smo_delegate(*this, FUNC(msx_cart_super_swangi_device::bank_w)));

	return image_init_result::PASS;
}

void msx_cart_super_swangi_device::bank_w(u8 data)
{
	m_rombank->set_entry((data >> 1) & 0x03);
}
