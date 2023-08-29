// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

A5200 Bryan Edewaard's Super Cart

Maps on the full A5200 cart space, uses $bfc0-$bfff reading for banking.
A5200 BIOS will read to $bfe0-$bfff range, this will automatically fallback to the last
available bank for convenience.

TODO:
- Checkout if support for non-512K carts works right, supposedly M.U.L.E. conversion
  runs on a 64K ROM version, should be right in theory.

**************************************************************************************************/

#include "emu.h"
#include "a5200_supercart.h"

DEFINE_DEVICE_TYPE(A5200_ROM_SUPERCART, a5200_rom_supercart_device, "a5200_supercart", "Atari 5200 Super Cart")


a5200_rom_supercart_device::a5200_rom_supercart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a5200_rom_device(mconfig, A5200_ROM_SUPERCART, tag, owner, clock)
	, m_bank(0)
{
}

void a5200_rom_supercart_device::device_start()
{
	save_item(NAME(m_bank));
}

void a5200_rom_supercart_device::device_reset()
{
	m_bank_mask = (m_rom_size / 0x8000) - 1;
	m_bank = m_bank_mask;
}

/*
 * --1- ---- fallback to last available bank
 * ---1 xx-- selects lower banks
 * ---0 xx-- selects upper banks
 *
 * On non-512K carts upper lines will just be ignored
 * i.e. a 64K will only respond to bits 5 or 4 high, with bit 2 as bank selector for latter.
 */
u8 a5200_rom_supercart_device::bank_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (BIT(offset, 5))
			m_bank = m_bank_mask;
		else
		{
			if (BIT(offset, 4))
				m_bank = (m_bank & 0xc) | ((offset & 0xc) >> 2);
			else
				m_bank = (m_bank & 3) | (offset & 0xc);
		}

		m_bank &= m_bank_mask;
	}

	return m_rom[(offset & 0x3f) + (m_bank * 0x8000) + 0x7fc0];
}

void a5200_rom_supercart_device::cart_map(address_map &map)
{
	map(0x0000, 0x7fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x7fff) + (m_bank * 0x8000)]; })
	);
	map(0x7fc0, 0x7fff).r(FUNC(a5200_rom_supercart_device::bank_r));
}
