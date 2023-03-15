// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "rtype.h"

DEFINE_DEVICE_TYPE(MSX_CART_RTYPE, msx_cart_rtype_device, "msx_cart_rtype", "MSX Cartridge - R-Type")


msx_cart_rtype_device::msx_cart_rtype_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_RTYPE, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank")
{
}

image_init_result msx_cart_rtype_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_rtype_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (cart_rom_region()->bytes() != 0x80000 && cart_rom_region()->bytes() != 0x60000)
	{
		message = "msx_cart_rtype_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_rombank->configure_entries(0, 24, cart_rom_region()->base(), 0x4000);

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base() + 15 * 0x4000);
	page(1)->install_write_handler(0x7000, 0x7fff, write8smo_delegate(*this, FUNC(msx_cart_rtype_device::bank_w)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);

	return image_init_result::PASS;
}

void msx_cart_rtype_device::bank_w(u8 data)
{
	data &= 0x1f;
	if (data & 0x10)
		data &= 0x17;
	m_rombank->set_entry(data);
}
