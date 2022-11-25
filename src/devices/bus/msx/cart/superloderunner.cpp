// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "superloderunner.h"

DEFINE_DEVICE_TYPE(MSX_CART_SUPERLODERUNNER, msx_cart_superloderunner_device, "msx_cart_superloderunner", "MSX Cartridge - Super Lode Runner")


msx_cart_superloderunner_device::msx_cart_superloderunner_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_SUPERLODERUNNER, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank")
{
}

void msx_cart_superloderunner_device::device_start()
{
	// Install evil system wide memory write handler
	memory_space().install_write_handler(0x0000, 0x0000, write8smo_delegate(*this, FUNC(msx_cart_superloderunner_device::bank_w)));
}

void msx_cart_superloderunner_device::initialize_cartridge()
{
	if (get_rom_size() != 0x20000)
	{
		fatalerror("superloderunner: Invalid ROM size\n");
	}

	m_rombank->configure_entries(0, 8, get_rom_base(), 0x4000);

	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);
}

void msx_cart_superloderunner_device::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x07);
}
