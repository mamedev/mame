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

void msx_cart_crossblaim_device::initialize_cartridge()
{
	if (get_rom_size() != 0x10000)
	{
		fatalerror("crossblaim: Invalid ROM size\n");
	}

	m_rombank->configure_entries(0, 4, get_rom_base(), 0x4000);

	page(1)->install_rom(0x4000, 0x7fff, get_rom_base());
	page(1)->install_write_handler(0x4045, 0x4045, write8smo_delegate(*this, FUNC(msx_cart_crossblaim_device::mapper_write)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);
}

void msx_cart_crossblaim_device::mapper_write(u8 data)
{
	data &= 0x03;
	if (!data)
		data = 1;

	m_rombank->set_entry(data);
}
