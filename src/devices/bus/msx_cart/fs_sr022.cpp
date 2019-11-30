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


void msx_cart_fs_sr022_device::initialize_cartridge()
{
	if (get_rom_size() != 0x40000)
	{
		fatalerror("fs_sr022: Invalid ROM size\n");
	}
	m_bunsetsu_rom = get_rom_base() + 0x20000;
}


uint8_t msx_cart_fs_sr022_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		if (offset == 0xbfff) {
			return m_bunsetsu_rom[m_bunsetsu_address++ & 0x1ffff];
		}

		return get_rom_base()[offset - 0x4000];
	}
	return 0xff;
}


void msx_cart_fs_sr022_device::write_cart(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0xbffc:
			m_bunsetsu_address = (m_bunsetsu_address & 0xffff00) | data;
			break;

		case 0xbffd:
			m_bunsetsu_address = (m_bunsetsu_address & 0xff00ff) | (data << 8);
			break;

		case 0xbffe:
			m_bunsetsu_address = (m_bunsetsu_address & 0x00ffff) | (data << 16);
			break;
	}
}
