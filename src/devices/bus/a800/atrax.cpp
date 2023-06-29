// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Atrax 128KB cart ROM

Not to be confused with the Sparta DOS X variants, this one is just a standalone cart with
single CCTL register that covers RD5 and bank.

**************************************************************************************************/

#include "emu.h"
#include "atrax.h"

DEFINE_DEVICE_TYPE(A800_ROM_ATRAX, a800_rom_atrax_device, "a800_atrax", "Atari 8-bit Atrax 128KB cart")


a800_rom_atrax_device::a800_rom_atrax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_ATRAX, tag, owner, clock)
	, m_bank(0)
{
}

void a800_rom_atrax_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_atrax_device::device_reset()
{
	m_bank = 0;
}

void a800_rom_atrax_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)]; })
	);
}

void a800_rom_atrax_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).w(FUNC(a800_rom_atrax_device::config_bank_w));
}

/*
 * x--- ---- RD5 disabled (1) enabled (0)
 * ---- xxxx bank number
 */
void a800_rom_atrax_device::config_bank_w(offs_t offset, u8 data)
{
	rd5_w(!BIT(data, 7));
	m_bank = data & 0xf;
}
