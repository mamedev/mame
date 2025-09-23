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
