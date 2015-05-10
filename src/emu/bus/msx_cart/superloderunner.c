// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "superloderunner.h"

const device_type MSX_CART_SUPERLODERUNNER = &device_creator<msx_cart_superloderunner>;


msx_cart_superloderunner::msx_cart_superloderunner(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_SUPERLODERUNNER, "MSX Cartridge - Super Lode Runner", tag, owner, clock, "msx_cart_superloderunner", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(0)
	, m_bank_base(NULL)
{
}


void msx_cart_superloderunner::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_superloderunner::restore_banks), this));

	// Install evil memory write handler
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_PROGRAM);
	space.install_write_handler(0x0000, 0x0000, write8_delegate(FUNC(msx_cart_superloderunner::banking), this));
}


void msx_cart_superloderunner::restore_banks()
{
	m_bank_base = get_rom_base() + (m_selected_bank & 0x0f) * 0x4000;
}


void msx_cart_superloderunner::initialize_cartridge()
{
	if (get_rom_size() != 0x20000)
	{
		fatalerror("superloderunner: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_superloderunner::read_cart)
{
	if (offset >= 0x8000 && offset < 0xc000)
	{
		return m_bank_base[offset & 0x3fff];
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_superloderunner::banking)
{
	m_selected_bank = data;
	restore_banks();
}
