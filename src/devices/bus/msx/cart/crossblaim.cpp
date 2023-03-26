// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "crossblaim.h"

DEFINE_DEVICE_TYPE(MSX_CART_CROSSBLAIM, msx_cart_crossblaim_device, "msx_cart_crossblaim", "MSX Cartridge Cross Blaim")


msx_cart_crossblaim_device::msx_cart_crossblaim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_CROSSBLAIM, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank")
{
}

image_init_result msx_cart_crossblaim_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_crossblaim_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (cart_rom_region()->bytes() != 0x10000)
	{
		message = "msx_cart_crossblaim_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_rombank->configure_entries(0, 4, cart_rom_region()->base(), 0x4000);

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(1)->install_write_handler(0x4045, 0x4045, write8smo_delegate(*this, FUNC(msx_cart_crossblaim_device::mapper_write)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);

	return image_init_result::PASS;
}

void msx_cart_crossblaim_device::mapper_write(u8 data)
{
	data &= 0x03;
	if (!data)
		data = 1;

	m_rombank->set_entry(data);
}
