// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "crossblaim.h"

DEFINE_DEVICE_TYPE(MSX_CART_CROSSBLAIM, msx_cart_crossblaim_device, "msx_cart_crossblaim", "MSX Cartridge Cross Blaim")


msx_cart_crossblaim_device::msx_cart_crossblaim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_CROSSBLAIM, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_selected_bank(1)
{
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
}


void msx_cart_crossblaim_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_crossblaim_device::device_post_load()
{
	restore_banks();
}


void msx_cart_crossblaim_device::setup_bank()
{
	m_bank_base[0] = ( m_selected_bank & 2 ) ? nullptr : get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
	m_bank_base[2] = get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
	m_bank_base[3] = ( m_selected_bank & 2 ) ? nullptr : get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
}


void msx_cart_crossblaim_device::restore_banks()
{
	m_bank_base[1] = get_rom_base();
	setup_bank();
}


void msx_cart_crossblaim_device::device_reset()
{
	m_selected_bank = 1;
}


void msx_cart_crossblaim_device::initialize_cartridge()
{
	if (get_rom_size() != 0x10000)
	{
		fatalerror("crossblaim: Invalid ROM size\n");
	}

	restore_banks();
}


uint8_t msx_cart_crossblaim_device::read_cart(offs_t offset)
{
	uint8_t *bank_base = m_bank_base[offset >> 14];

	if (bank_base != nullptr)
	{
		return bank_base[offset & 0x3fff];
	}

	return 0xff;
}


void msx_cart_crossblaim_device::write_cart(offs_t offset, uint8_t data)
{
	m_selected_bank = data & 3;
	if (m_selected_bank == 0)
	{
		m_selected_bank = 1;
	}
	setup_bank();
}
