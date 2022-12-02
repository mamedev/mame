// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msxdos2.h"

// This is the bare minimum to get a msx-dos 2 rom working in a system
// TODO:
// - Add subslots for regular rom and kanji driver rom
// - Add support for 128KB and 256KB memory mapper versions

DEFINE_DEVICE_TYPE(MSX_CART_MSXDOS2, msx_cart_msxdos2_device, "msx_cart_msxdos2", "MSX Cartridge - MSXDOS2")


msx_cart_msxdos2_device::msx_cart_msxdos2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_MSXDOS2, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank")
{
}

void msx_cart_msxdos2_device::device_reset()
{
	m_rombank->set_entry(1);
}

void msx_cart_msxdos2_device::initialize_cartridge()
{
	if (get_rom_size() != 0x10000)
	{
		fatalerror("msxdos2: Invalid ROM size\n");
	}

	m_rombank->configure_entries(0, 4, get_rom_base(), 0x4000);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank);
	page(1)->install_write_handler(0x7ffe, 0x7ffe, write8smo_delegate(*this, FUNC(msx_cart_msxdos2_device::bank_w)));
}

void msx_cart_msxdos2_device::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x03);
}
