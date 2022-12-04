// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "fs_sr022.h"


DEFINE_DEVICE_TYPE(MSX_CART_FS_SR022, msx_cart_fs_sr022_device, "msx_cart_fs_sr022", "MSX Cartridge - FS-SR022")


msx_cart_fs_sr022_device::msx_cart_fs_sr022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_FS_SR022, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bunsetsu_rom(nullptr)
	, m_bunsetsu_address(0)
{
}

void msx_cart_fs_sr022_device::device_start()
{
	save_item(NAME(m_bunsetsu_address));
}

void msx_cart_fs_sr022_device::device_reset()
{
	m_bunsetsu_address = 0;
}

image_init_result msx_cart_fs_sr022_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_fs_sr022_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (cart_rom_region()->bytes() != 0x40000)
	{
		message = "msx_cart_fs_sr022_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bunsetsu_rom = cart_rom_region()->base() + 0x20000;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);
	page(2)->install_read_handler(0xbfff, 0xbfff, read8smo_delegate(*this, FUNC(msx_cart_fs_sr022_device::buns_r)));
	page(2)->install_write_handler(0xbffc, 0xbffe, write8sm_delegate(*this, FUNC(msx_cart_fs_sr022_device::buns_w)));

	return image_init_result::PASS;
}

u8 msx_cart_fs_sr022_device::buns_r()
{
	u8 data = m_bunsetsu_rom[m_bunsetsu_address & 0x1ffff];
	if (!machine().side_effects_disabled())
		m_bunsetsu_address++;
	return data;
}

void msx_cart_fs_sr022_device::buns_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_bunsetsu_address = (m_bunsetsu_address & 0xffff00) | data;
			break;

		case 1:
			m_bunsetsu_address = (m_bunsetsu_address & 0xff00ff) | (data << 8);
			break;

		case 2:
			m_bunsetsu_address = (m_bunsetsu_address & 0x00ffff) | (data << 16);
			break;
	}
}
