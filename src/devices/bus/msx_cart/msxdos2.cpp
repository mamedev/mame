// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msxdos2.h"

DEFINE_DEVICE_TYPE(MSX_CART_MSXDOS2, msx_cart_msxdos2_device, "msx_cart_msxdos2", "MSX Cartridge - MSXDOS2")


msx_cart_msxdos2_device::msx_cart_msxdos2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MSXDOS2, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(0)
	, m_bank_base(nullptr)
{
}


void msx_cart_msxdos2_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_msxdos2_device::device_post_load()
{
	restore_banks();
}


void msx_cart_msxdos2_device::restore_banks()
{
	m_bank_base = get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
}


void msx_cart_msxdos2_device::device_reset()
{
	m_selected_bank = 0;
}


void msx_cart_msxdos2_device::initialize_cartridge()
{
	if (get_rom_size() != 0x10000)
	{
		fatalerror("msxdos2: Invalid ROM size\n");
	}

	restore_banks();
}


uint8_t msx_cart_msxdos2_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0x8000)
	{
		return m_bank_base[offset & 0x3fff];
	}

	return 0xff;
}


void msx_cart_msxdos2_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset == 0x6000)
	{
		m_selected_bank = data;
		restore_banks();
	}
}
