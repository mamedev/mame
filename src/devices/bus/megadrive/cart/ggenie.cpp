// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Damian Rogers
/***********************************************************************************************

Codemasters/Galoob Game Genie pass-through cart

SGA001 ASIC + 32 KB program ROM. The ASIC watches /CE_0 ($000000-$3fffff) and can substitute
data for up to six word addresses of the cart plugged on top.

Register map ($000000-$00003f; written in any mode until locked, read in mode 1):
$00 mode: bits 10-9 select what /CE_0 reads return (0 = program ROM, 1 = registers,
    2/3 = cart), bit 8 locks all registers until the next reset, bits 5-0 enable codes 6-1
$02 reset status: bit 0 set by any write to it, cleared only at power on
$04-$26 codes 1-6: address bits 23-16, address bits 15-1 (no A0: a code patches the whole word), data
$28-$3e read-only mirrors of codes 5 and 6

Reset returns to the program with the codes kept ("press Reset and then Start" replays the game
with them, per the instruction manual); only a power cycle clears them.

References:
- Galoob, Game Genie for Sega Genesis instruction manual
- Charles MacDonald, "Genesis/SNES Game Genie Hardware Notes" (genie.txt, 2007); it lists the
  two address words the other way round, but the program ROM writes the checksum patch as
  move.l #$18e,$22.w, which puts the high word first

TODO:
- Effects On/Off switch and LED
- Forward /VRES driven by the cart on top to the slot
- Writes pass through to the cart on top as is; there are informal reports of battery-backed
  RAM getting corrupted on some carts when used through a Game Genie

***********************************************************************************************/

#include "emu.h"
#include "ggenie.h"

#include "options.h"

#include <algorithm>
#include <iterator>

DEFINE_DEVICE_TYPE(MEGADRIVE_GGENIE, megadrive_ggenie_device, "megadrive_ggenie", "Megadrive Game Genie cart")

megadrive_ggenie_device::megadrive_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_GGENIE, tag, owner, clock)
	, m_lockon_cart(*this, "cart")
	, m_gg_view(*this, "gg_view")
{
}

void megadrive_ggenie_device::device_add_mconfig(machine_config &config)
{
	MEGADRIVE_CART_SLOT(config, m_lockon_cart, this->clock(), megadrive_cart_options, nullptr).set_must_be_loaded(false);
}

void megadrive_ggenie_device::device_start()
{
	megadrive_rom_device::device_start();

	std::fill(std::begin(m_regs), std::end(m_regs), 0);

	save_item(NAME(m_regs));
}

void megadrive_ggenie_device::device_reset()
{
	megadrive_rom_device::device_reset();

	// reset releases the lock and gives /CE_0 back to the program ROM; the code enables, codes and
	// reset status stay (the program reads them back after a warm boot)
	m_regs[0] &= 0x003f;
	update_view();
}

int megadrive_ggenie_device::patch_match(offs_t offset) const
{
	// the lowest numbered matching code wins
	for (u32 code = 0; code < 6; code++)
	{
		if (BIT(m_regs[0], code) && (offset == (patch_address(code) >> 1)))
			return code;
	}
	return -1;
}

void megadrive_ggenie_device::update_view()
{
	if (!BIT(m_regs[0], 10))
		m_gg_view.select(BIT(m_regs[0], 9));
	else if (!locked() || (m_regs[0] & 0x3f))
		m_gg_view.select(2);
	else
		m_gg_view.disable(); // locked with no codes: plain pass-through
}

u16 megadrive_ggenie_device::regs_r(offs_t offset)
{
	// $28-$3e mirror codes 5 and 6
	if (offset >= std::size(m_regs))
		return m_regs[14 + ((offset - std::size(m_regs)) % 6)];
	return m_regs[offset];
}

void megadrive_ggenie_device::regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (locked() || (offset >= std::size(m_regs)))
		return;

	if (offset == 1)
	{
		m_regs[1] |= 1;
		return;
	}

	COMBINE_DATA(&m_regs[offset]);

	if (offset == 0)
		update_view();
}

u16 megadrive_ggenie_device::cart_r(offs_t offset, u16 mem_mask)
{
	int const code = patch_match(offset);
	if (code >= 0)
		return m_regs[4 + code * 3];
	return m_lockon_cart->base_r(offset, mem_mask);
}

void megadrive_ggenie_device::cart_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ((offset < std::size(m_regs)) && !locked())
		regs_w(offset, data, mem_mask);
	else if (patch_match(offset) < 0) // a matching code also keeps the cart deselected on writes
		m_lockon_cart->base_w(offset, data, mem_mask);
}

void megadrive_ggenie_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x7f'ffff).rw(m_lockon_cart, FUNC(megadrive_cart_slot_device::base_r), FUNC(megadrive_cart_slot_device::base_w));
	map(0x00'0000, 0x3f'ffff).view(m_gg_view);
	m_gg_view[0](0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	m_gg_view[0](0x00'0000, 0x00'003f).w(FUNC(megadrive_ggenie_device::regs_w));
	m_gg_view[1](0x00'0000, 0x00'003f).mirror(0x3f'ffc0).r(FUNC(megadrive_ggenie_device::regs_r));
	m_gg_view[1](0x00'0000, 0x00'003f).w(FUNC(megadrive_ggenie_device::regs_w));
	m_gg_view[2](0x00'0000, 0x3f'ffff).rw(FUNC(megadrive_ggenie_device::cart_r), FUNC(megadrive_ggenie_device::cart_w));
}

void megadrive_ggenie_device::time_io_map(address_map &map)
{
	map(0x00, 0xff).rw(m_lockon_cart, FUNC(megadrive_cart_slot_device::time_r), FUNC(megadrive_cart_slot_device::time_w));
}
