// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msxdos2.h"

const device_type MSX_CART_MSXDOS2 = &device_creator<msx_cart_msxdos2>;


msx_cart_msxdos2::msx_cart_msxdos2(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSXDOS2, "MSX Cartridge - MSXDOS2", tag, owner, clock, "msx_cart_msxdos2", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(0)
	, m_bank_base(nullptr)
{
}


void msx_cart_msxdos2::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_msxdos2::restore_banks), this));
}


void msx_cart_msxdos2::restore_banks()
{
	m_bank_base = get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
}


void msx_cart_msxdos2::device_reset()
{
	m_selected_bank = 0;
}


void msx_cart_msxdos2::initialize_cartridge()
{
	if (get_rom_size() != 0x10000)
	{
		fatalerror("msxdos2: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_msxdos2::read_cart)
{
	if (offset >= 0x4000 && offset < 0x8000)
	{
		return m_bank_base[offset & 0x3fff];
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_msxdos2::write_cart)
{
	if (offset == 0x6000)
	{
		m_selected_bank = data;
		restore_banks();
	}
}
