// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "dooly.h"


DEFINE_DEVICE_TYPE(MSX_CART_DOOLY, msx_cart_dooly_device, "msx_cart_dooly", "MSX Cartridge - Dooly")


msx_cart_dooly_device::msx_cart_dooly_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_DOOLY, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_prot(0)
{
}


void msx_cart_dooly_device::device_start()
{
	save_item(NAME(m_prot));
}


void msx_cart_dooly_device::device_reset()
{
	m_prot = 0;
}


void msx_cart_dooly_device::initialize_cartridge()
{
	if (get_rom_size() != 0x8000)
	{
		fatalerror("dooly: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_dooly_device::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		uint8_t data = get_rom_base()[offset - 0x4000];

		switch (m_prot)
		{
			case 0x04:
				data = bitswap<8>(data, 7, 6, 5, 4, 3, 1, 0, 2);
				break;
		}
		return data;
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_dooly_device::write_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		m_prot = data & 0x07;
		if (m_prot != 0 && m_prot != 4)
		{
			logerror("msx_cart_dooly_device: unhandled write %02x to %04x\n", data, offset);
		}
	}
}
