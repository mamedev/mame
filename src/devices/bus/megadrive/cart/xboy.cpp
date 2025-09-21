// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

X Boy published/developed cart mappers

**************************************************************************************************/


#include "emu.h"
#include "xboy.h"

#include "bus/generic/slot.h"


/*
 * The King of Fighters '99
 * https://segaretro.org/King_of_Fighters_98%27
 *
 * Earlier protection variant
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_KOF98, megadrive_unl_kof98_device, "megadrive_unl_kof98", "Megadrive The King of Fighters '98 cart")

megadrive_unl_kof98_device::megadrive_unl_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_KOF98, tag, owner, clock)
{
}

void megadrive_unl_kof98_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// everytime, trashes stack otherwise
	map(0x48'0000, 0x4b'ffff).lr16(NAME([] () { return 0xaa00; }));
	// when selecting Arcade or VS., pointer to reach above
	map(0x4c'0000, 0x4f'ffff).lr16(NAME([] () { return 0xf000; }));
}


/*
 * A Bug's Life
 * https://segaretro.org/A_Bug%27s_Life_(Mega_Drive)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_BUGSLIFE, megadrive_unl_bugslife_device, "megadrive_unl_bugslife", "Megadrive A Bug's Life cart")

megadrive_unl_bugslife_device::megadrive_unl_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_BUGSLIFE, tag, owner, clock)
{
}

void megadrive_unl_bugslife_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x28; }));
	// those two are patched by SW
	map(0x02, 0x03).lr16(NAME([] () { return 0x01; }));
	map(0x3e, 0x3f).lr16(NAME([] () { return 0x1f; }));
}

/*
 * Pokemon Monster
 * https://segaretro.org/Pocket_Monster
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_POKEMONA, megadrive_unl_pokemona_device, "megadrive_unl_pokemona", "Megadrive Pokemon Monster alt cart")

megadrive_unl_pokemona_device::megadrive_unl_pokemona_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_POKEMONA, tag, owner, clock)
{
}

void megadrive_unl_pokemona_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x14; }));
	// those two are patched by SW
	map(0x02, 0x03).lr16(NAME([] () { return 0x01; }));
	map(0x3e, 0x3f).lr16(NAME([] () { return 0x1f; }));
}

/*
 * The King of Fighters '99
 * https://segaretro.org/The_King_of_Fighters_%2799_(Mega_Drive)
 *
 * Writes "Secondary memory access failure" if $a13000 returns & 0xf != 0
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_KOF99, megadrive_unl_kof99_device, "megadrive_unl_kof99", "Megadrive The King of Fighters '99 cart")

megadrive_unl_kof99_device::megadrive_unl_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_KOF99, tag, owner, clock)
{
}

void megadrive_unl_kof99_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x00; }));
	map(0x02, 0x03).lr16(NAME([] () { return 0x01; }));
	map(0x3e, 0x3f).lr16(NAME([] () { return 0x1f; }));
}


