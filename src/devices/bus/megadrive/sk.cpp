// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Sonic & Knuckles pass-thorugh cart emulation

 TODO: This could potentially make use of memory views. Investigate.


 ***********************************************************************************************************/

#include "emu.h"
#include "sk.h"
#include "rom.h"
#include "md_carts.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MD_ROM_SK, md_rom_sk_device, "md_rom_sk", "MD Sonic & Knuckles")


md_rom_sk_device::md_rom_sk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
	, m_exp(*this, "subslot")
	, m_map_upper(false)
{
}

md_rom_sk_device::md_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_rom_sk_device(mconfig, MD_ROM_SK, tag, owner, clock)
{
}


void md_rom_sk_device::device_start()
{
	save_item(NAME(m_map_upper));
}


void md_rom_sk_device::device_reset()
{
	m_map_upper = false;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint16_t md_rom_sk_device::read(offs_t offset)
{
	if (m_map_upper)
	{
		if (offset >= 0x300000/2)
			return m_rom[MD_ADDR(offset)]; // Sonic 2 patch ROM
		else if (m_exp->m_cart != nullptr && m_exp->m_cart->get_rom_base() != nullptr && offset >= 0x200000/2)
			return m_exp->m_cart->read(offset); // Sonic 3 ROM pass-through or FRAM
	}

	if (m_exp->m_cart != nullptr && m_exp->m_cart->get_rom_base() != nullptr && offset >= 0x200000/2)
		return m_exp->m_cart->read(offset - 0x200000/2);

	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_rom_sk_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_exp->m_cart == nullptr || m_exp->m_cart->get_rom_base() == nullptr)
		return;

	if (m_map_upper)
		m_exp->m_cart->write(offset, data, mem_mask);
	else if (offset >= 0x200000/2)
		m_exp->m_cart->write(offset - 0x200000/2, data, mem_mask);
}

uint16_t md_rom_sk_device::read_a13(offs_t offset)
{
	if (m_exp->m_cart != nullptr && m_exp->m_cart->get_rom_base() != nullptr)
		return m_exp->m_cart->read_a13(offset);
	return 0xffff;
}

void md_rom_sk_device::write_a13(offs_t offset, uint16_t data)
{
	if (m_exp->m_cart != nullptr && m_exp->m_cart->get_rom_base() != nullptr)
		m_exp->m_cart->write_a13(offset, data);
	if (offset == 0xf0/2)
		m_map_upper = BIT(data, 0);
}

static void sk_sub_cart(device_slot_interface &device)
{
	// Inherit all cartridge types; cartridges <= 2 megabytes in size are mirrored into the upper 2 megabytes by the Sonic & Knuckles cartridge.
	md_cart(device);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void md_rom_sk_device::device_add_mconfig(machine_config &config)
{
	MD_CART_SLOT(config, m_exp, sk_sub_cart, nullptr);
	m_exp->set_must_be_loaded(false);
}
