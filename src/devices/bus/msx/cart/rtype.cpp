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

void msx_cart_rtype_device::initialize_cartridge()
{
	if (get_rom_size() != 0x80000 && get_rom_size() != 0x60000)
	{
		fatalerror("rtype: Invalid ROM size\n");
	}

	m_rombank->configure_entries(0, 24, get_rom_base(), 0x4000);

	page(1)->install_rom(0x4000, 0x7fff, get_rom_base() + 15 * 0x4000);
	page(1)->install_write_handler(0x7000, 0x7fff, write8smo_delegate(*this, FUNC(msx_cart_rtype_device::bank_w)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);
}

void msx_cart_rtype_device::bank_w(u8 data)
{
	data &= 0x1f;
	if (data & 0x10)
		data &= 0x17;
	m_rombank->set_entry(data);
}
