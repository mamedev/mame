// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "arc.h"


const device_type MSX_CART_ARC = &device_creator<msx_cart_arc>;


msx_cart_arc::msx_cart_arc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_ARC, "MSX Cartridge - Arc", tag, owner, clock, "msx_cart_arc", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_7f(0)
{
}


void msx_cart_arc::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0x7f, 0x7f, write8_delegate(FUNC(msx_cart_arc::io_7f_w), this));
	space.install_read_handler(0x7f, 0x7f, read8_delegate(FUNC(msx_cart_arc::io_7f_r), this));
}


void msx_cart_arc::device_reset()
{
	m_7f = 0;
}


void msx_cart_arc::initialize_cartridge()
{
	if (get_rom_size() != 0x8000)
	{
		fatalerror("arc: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_arc::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return get_rom_base()[offset - 0x4000];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_arc::io_7f_w)
{
	if (data == 0x35)
	{
		m_7f++;
	}
}


READ8_MEMBER(msx_cart_arc::io_7f_r)
{
	return ((m_7f & 0x03) == 0x03) ? 0xda : 0xff;
}
