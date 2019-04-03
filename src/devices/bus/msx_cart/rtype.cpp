// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "rtype.h"

DEFINE_DEVICE_TYPE(MSX_CART_RTYPE, msx_cart_rtype_device, "msx_cart_rtype", "MSX Cartridge - R-Type")


msx_cart_rtype_device::msx_cart_rtype_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_RTYPE, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(0)
{
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
}


void msx_cart_rtype_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_rtype_device::device_post_load()
{
	restore_banks();
}


void msx_cart_rtype_device::restore_banks()
{
	m_bank_base[0] = get_rom_base() + 15 * 0x4000;
	if (m_selected_bank & 0x10)
	{
		m_selected_bank &= 0x17;
	}
	m_bank_base[1] = get_rom_base() + m_selected_bank * 0x4000;
}


void msx_cart_rtype_device::device_reset()
{
	m_selected_bank = 15;
}


void msx_cart_rtype_device::initialize_cartridge()
{
	if ( get_rom_size() != 0x80000 && get_rom_size() != 0x60000 )
	{
		fatalerror("rtype: Invalid ROM size\n");
	}

	restore_banks();
}


uint8_t msx_cart_rtype_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}
	return 0xff;
}


void msx_cart_rtype_device::write_cart(offs_t offset, uint8_t data)
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
