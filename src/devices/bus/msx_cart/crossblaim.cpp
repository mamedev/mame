// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "crossblaim.h"

const device_type MSX_CART_CROSSBLAIM = &device_creator<msx_cart_crossblaim>;


msx_cart_crossblaim::msx_cart_crossblaim(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_CROSSBLAIM, "MSX Cartridge - Cross Blaim", tag, owner, clock, "msx_cart_crossblaim", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(1)
{
	for (int i = 0; i < 4; i++)
	{
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_crossblaim::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_crossblaim::restore_banks), this));
}


void msx_cart_crossblaim::setup_bank()
{
	m_bank_base[0] = ( m_selected_bank & 2 ) ? nullptr : get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
	m_bank_base[2] = get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
	m_bank_base[3] = ( m_selected_bank & 2 ) ? nullptr : get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
}


void msx_cart_crossblaim::restore_banks()
{
	m_bank_base[1] = get_rom_base();
	setup_bank();
}


void msx_cart_crossblaim::device_reset()
{
	m_selected_bank = 1;
}


void msx_cart_crossblaim::initialize_cartridge()
{
	if (get_rom_size() != 0x10000)
	{
		fatalerror("crossblaim: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_crossblaim::read_cart)
{
	UINT8 *bank_base = m_bank_base[offset >> 14];

	if (bank_base != nullptr)
	{
		return bank_base[offset & 0x3fff];
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_crossblaim::write_cart)
{
	m_selected_bank = data & 3;
	if (m_selected_bank == 0)
	{
		m_selected_bank = 1;
	}
	setup_bank();
}
