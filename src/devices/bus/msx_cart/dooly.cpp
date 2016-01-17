// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "dooly.h"


const device_type MSX_CART_DOOLY = &device_creator<msx_cart_dooly>;


msx_cart_dooly::msx_cart_dooly(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_DOOLY, "MSX Cartridge - Dooly", tag, owner, clock, "msx_cart_dooly", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_prot(0)
{
}


void msx_cart_dooly::device_start()
{
	save_item(NAME(m_prot));
}


void msx_cart_dooly::device_reset()
{
	m_prot = 0;
}


void msx_cart_dooly::initialize_cartridge()
{
	if (get_rom_size() != 0x8000)
	{
		fatalerror("dooly: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_dooly::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		UINT8 data = get_rom_base()[offset - 0x4000];

		switch (m_prot)
		{
			case 0x04:
				data = BITSWAP8(data, 7, 6, 5, 4, 3, 1, 0, 2);
				break;
		}
		return data;
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_dooly::write_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		m_prot = data & 0x07;
		if (m_prot != 0 && m_prot != 4)
		{
			logerror("msx_cart_dooly: unhandled write %02x to %04x\n", data, offset);
		}
	}
}
