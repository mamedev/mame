// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Ink cartridge (Matra, 2006)

48KB ROM data on an AMD Am29F040B(label scratched off), the game uses it
for protection.

******************************************************************************/

#include "emu.h"
#include "ink.h"

DEFINE_DEVICE_TYPE(MSX_CART_INK, msx_cart_ink_device, "msx_cart_ink", "MSX Cartridge - Ink")


msx_cart_ink_device::msx_cart_ink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_INK, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_flash(*this, "flash")
{
}

ROM_START( msx_cart_ink )
	ROM_REGION(0x80000, "flash", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *msx_cart_ink_device::device_rom_region() const
{
	return ROM_NAME( msx_cart_ink );
}

void msx_cart_ink_device::device_add_mconfig(machine_config &config)
{
	AMD_29F040(config, m_flash);
}

void msx_cart_ink_device::initialize_cartridge()
{
	size_t size = get_rom_size() > 0x80000 ? 0x80000 : get_rom_size();
	memcpy(memregion("flash")->base(), get_rom_base(), size);
}


uint8_t msx_cart_ink_device::read_cart(offs_t offset)
{
	return m_flash->read(offset);
}

void msx_cart_ink_device::write_cart(offs_t offset, uint8_t data)
{
	// /RD connects to flashrom A16-A18
	m_flash->write(offset | 0x70000, data);
}
