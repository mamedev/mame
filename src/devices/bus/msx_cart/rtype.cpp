// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "rtype.h"

const device_type MSX_CART_RTYPE = &device_creator<msx_cart_rtype>;


msx_cart_rtype::msx_cart_rtype(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_RTYPE, "MSX Cartridge - R-Type", tag, owner, clock, "msx_cart_rtype", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(0)
{
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
}


void msx_cart_rtype::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_rtype::restore_banks), this));
}


void msx_cart_rtype::restore_banks()
{
	m_bank_base[0] = get_rom_base() + 15 * 0x4000;
	if (m_selected_bank & 0x10)
	{
		m_selected_bank &= 0x17;
	}
	m_bank_base[1] = get_rom_base() + m_selected_bank * 0x4000;
}


void msx_cart_rtype::device_reset()
{
	m_selected_bank = 15;
}


void msx_cart_rtype::initialize_cartridge()
{
	if ( get_rom_size() != 0x80000 && get_rom_size() != 0x60000 )
	{
		fatalerror("rtype: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_rtype::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_rtype::write_cart)
{
	if (offset >= 0x7000 && offset < 0x8000)
	{
		m_selected_bank = data & 0x1f;
		if (m_selected_bank & 0x10)
		{
			m_selected_bank &= 0x17;
		}
		m_bank_base[1] = get_rom_base() + m_selected_bank * 0x4000;
	}
}
