// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "super_swangi.h"


const device_type MSX_CART_SUPER_SWANGI = &device_creator<msx_cart_super_swangi>;


msx_cart_super_swangi::msx_cart_super_swangi(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_SUPER_SWANGI, "MSX Cartridge - Super Swangi", tag, owner, clock, "msx_cart_super_swangi", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(0)
{
	m_bank_base[0] = m_bank_base[1] = nullptr;
}


void msx_cart_super_swangi::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_super_swangi::restore_banks), this));
}


void msx_cart_super_swangi::restore_banks()
{
	m_bank_base[0] = get_rom_base();
	m_bank_base[1] = get_rom_base() + (((m_selected_bank >> 1) * 0x4000) & 0xffff);
}


void msx_cart_super_swangi::device_reset()
{
	m_selected_bank = 0;
}


void msx_cart_super_swangi::initialize_cartridge()
{
	if (get_rom_size() < 0x10000)
	{
		fatalerror("rtype: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_super_swangi::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_super_swangi::write_cart)
{
	if (offset == 0x8000)
	{
		m_selected_bank = data;
		restore_banks();
	}
	else
	{
		logerror("msx_cart_super_swangi: unhandled write %02x to %04x\n", data, offset);
	}
}
