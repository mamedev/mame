// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "dooly.h"


DEFINE_DEVICE_TYPE(MSX_CART_DOOLY, msx_cart_dooly_device, "msx_cart_dooly", "MSX Cartridge - Dooly")


msx_cart_dooly_device::msx_cart_dooly_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_DOOLY, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_view1(*this, "view1")
	, m_view2(*this, "view2")
{
}

void msx_cart_dooly_device::device_reset()
{
	m_view1.select(0);
	m_view2.select(0);
}

void msx_cart_dooly_device::initialize_cartridge()
{
	if (get_rom_size() != 0x8000)
	{
		fatalerror("dooly: Invalid ROM size\n");
	}

	page(1)->install_view(0x4000, 0x7fff, m_view1);
	m_view1[0].install_rom(0x4000, 0x7fff, get_rom_base());
	m_view1[1].install_read_handler(0x4000, 0x7fff, read8sm_delegate(*this, FUNC(msx_cart_dooly_device::mode4_page1_r)));
	page(2)->install_view(0x8000, 0xbfff, m_view2);
	m_view2[0].install_rom(0x8000, 0xbfff, get_rom_base() + 0x4000);
	m_view2[1].install_read_handler(0x8000, 0xbfff, read8sm_delegate(*this, FUNC(msx_cart_dooly_device::mode4_page2_r)));

	page(1)->install_write_handler(0x4000, 0x7fff, write8smo_delegate(*this, FUNC(msx_cart_dooly_device::prot_w)));
	page(2)->install_write_handler(0x8000, 0xbfff, write8smo_delegate(*this, FUNC(msx_cart_dooly_device::prot_w)));
}

u8 msx_cart_dooly_device::mode4_page1_r(offs_t offset)
{
	return bitswap<8>(get_rom_base()[offset], 7, 6, 5, 4, 3, 1, 0, 2);
}

u8 msx_cart_dooly_device::mode4_page2_r(offs_t offset)
{
	return bitswap<8>(get_rom_base()[0x4000 | offset], 7, 6, 5, 4, 3, 1, 0, 2);
}

void msx_cart_dooly_device::prot_w(u8 data)
{
	data &= 0x07;
	m_view1.select(BIT(data, 2) ? 1 : 0);
	m_view2.select(BIT(data, 2) ? 1 : 0);
	if (data != 0 && data != 4)
	{
		logerror("msx_cart_dooly_device: unhandled protection mode %02x\n", data);
	}
}
