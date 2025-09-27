// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

RockHeaven / RockWorld mapper


**************************************************************************************************/

#include "emu.h"
#include "rockworld.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_ROCKHEAVEN, megadrive_unl_rockheaven_device, "megadrive_unl_rockheaven", "Megadrive Rock Heaven cart")

megadrive_unl_rockheaven_device::megadrive_unl_rockheaven_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_ROCKHEAVEN, tag, owner, clock)
{
}

void megadrive_unl_rockheaven_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x50'0008, 0x50'0009).lr16(NAME([] () { return 0x5082; }));
}

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_ROCKWORLD, megadrive_unl_rockworld_device, "megadrive_unl_rockworld", "Megadrive Rock World cart")

megadrive_unl_rockworld_device::megadrive_unl_rockworld_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_ROCKWORLD, tag, owner, clock)
{
}

void megadrive_unl_rockworld_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x50'0008, 0x50'0009).lr16(NAME([] () { return 0x4000; }));
	map(0x50'0208, 0x50'0209).lr16(NAME([] () { return 0xa000; }));
}


