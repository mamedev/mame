// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

https://segaretro.org/TC_2000
https://segaretro.org/Truco_%2796

Unknown protection chip, simple strobe write then read

**************************************************************************************************/

#include "emu.h"
#include "miky.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_TC2000, megadrive_unl_tc2000_device, "megadrive_unl_tc2000", "Megadrive TC2000 cart")

megadrive_unl_tc2000_device::megadrive_unl_tc2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_TC2000, tag, owner, clock)
{
}

void megadrive_unl_tc2000_device::device_start()
{
	megadrive_rom_device::device_start();
	save_item(NAME(m_prot_latch));
}

void megadrive_unl_tc2000_device::device_reset()
{
	megadrive_rom_device::device_reset();
	// undefined, initialized by game anyway
	m_prot_latch = 0;
}


void megadrive_unl_tc2000_device::cart_map(address_map &map)
{
	// writes often in ROM space, buggy?
	map(0x00'0000, 0x0f'ffff).bankr(m_rom).nopw();
	map(0x10'0000, 0x10'0000).mirror(0x0f'fffe).lr8(NAME([this] () { return m_prot_latch; }));
	// truco96a
	map(0x10'0000, 0x10'0000).mirror(0x0f'fff0).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_prot_latch = 0x00; }));
	// tc2000
	map(0x10'0008, 0x10'0008).mirror(0x0f'fff0).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_prot_latch = 0x50; }));
	map(0x10'000c, 0x10'000c).mirror(0x0f'fff0).lw8(NAME([this] (offs_t offset, u8 data) { (void)data; m_prot_latch = 0xa0; }));
}

/*
 * Futbol Argentino 96
 * https://segaretro.org/J.League_Pro_Striker_2/Bootlegs
 *
 * Unlike jlps2 saving a league doesn't really work without playing at least one match first.
 * Is this is based on an undumped earlier rev? (No protection access on that)
 *
 * TODO:
 * - protection not really understood (game does a very small use of it, just expects the read
 *   values to always return fixed values)
 *
 */

 DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_FUTBOL_ARG96, megadrive_unl_futbol_arg96_device, "megadrive_unl_futbol_arg96", "Megadrive Futbol Argentino 96 cart")

megadrive_unl_futbol_arg96_device::megadrive_unl_futbol_arg96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, MEGADRIVE_UNL_FUTBOL_ARG96, tag, owner, clock)
{
}

u16 megadrive_unl_futbol_arg96_device::get_nvram_length()
{
	return 0x2000;
}

void megadrive_unl_futbol_arg96_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).mirror(0x20'0000).bankr(m_rom);
	map(0x20'0000, 0x20'3fff).rw(FUNC(megadrive_unl_futbol_arg96_device::nvram_r), FUNC(megadrive_unl_futbol_arg96_device::nvram_w));
	// writes (in this order, always 0):
	// 4c'6000 x1
	// 4c'6400 x1
	// 4c'6800 x2
	// 4c'6c00 x3
	// 4c'7000 x4
	map(0x4c'6201, 0x4c'6201).lr8(NAME([] () { return 0xa; }));
	map(0x4c'6601, 0x4c'6601).lr8(NAME([] () { return 0x9; }));
	map(0x4c'6a01, 0x4c'6a01).lr8(NAME([] () { return 0x7; }));
}


