// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "hfox.h"


DEFINE_DEVICE_TYPE(MSX_CART_HFOX, msx_cart_hfox_device, "msx_cart_hfox", "MSX Cartridge - Harry Fox")


msx_cart_hfox_device::msx_cart_hfox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_HFOX, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank{ 0, 0 }
	, m_bank_base{ nullptr, nullptr }
{
}


void msx_cart_hfox_device::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_hfox_device::restore_banks), this));
}


void msx_cart_hfox_device::restore_banks()
{
	m_bank_base[0] = get_rom_base() + ((m_selected_bank[0] & 0x01) * 0x8000);
	m_bank_base[1] = get_rom_base() + ((m_selected_bank[1] & 0x01) * 0x8000) + 0x4000;
}


void msx_cart_hfox_device::device_reset()
{
	m_selected_bank[0] = m_selected_bank[1] = 0;
}


void msx_cart_hfox_device::initialize_cartridge()
{
	if (get_rom_size() < 0x10000)
	{
		fatalerror("rtype: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_hfox_device::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_hfox_device::write_cart)
{
	switch (offset)
	{
		case 0x6000:
			m_selected_bank[0] = data;
			restore_banks();
			break;

		case 0x7000:
			m_selected_bank[1] = data;
			restore_banks();
			break;

		default:
			logerror("msx_cart_hfox_device: unhandled write %02x to %04x\n", data, offset);
			break;
	}
}
