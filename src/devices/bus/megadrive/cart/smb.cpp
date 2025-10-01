// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Super Mario Bros cart mappers

"decode error" printed if protection fails

TODO:
- are these really hacked versions? Rockman X3 does a few Gamtec style checks, ignored by the code

**************************************************************************************************/

#include "emu.h"
#include "smb.h"

/*
 * Super Mario World / Super Mario Bros.
 * https://segaretro.org/Super_Mario_World
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SMB, megadrive_unl_smb_device, "megadrive_unl_smb", "Megadrive Super Mario World cart")

megadrive_unl_smb_device::megadrive_unl_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SMB, tag, owner, clock)
{
}

void megadrive_unl_smb_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x001c; }));
}

/*
 * Super Mario Bros. 2
 * https://segaretro.org/Super_Mario_2_1998
 *
 */


DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SMB2, megadrive_unl_smb2_device, "megadrive_unl_smb2", "Megadrive Super Mario Bros 2 cart")

megadrive_unl_smb2_device::megadrive_unl_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SMB2, tag, owner, clock)
{
}

void megadrive_unl_smb2_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x000a; }));
}

/*
 * Rockman X3
 * https://segaretro.org/Rockman_X3_(Mega_Drive)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_ROCKMANX3, megadrive_unl_rockmanx3_device, "megadrive_unl_rockmanx3", "Megadrive Rockman X3 cart")

megadrive_unl_rockmanx3_device::megadrive_unl_rockmanx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_ROCKMANX3, tag, owner, clock)
{
}

void megadrive_unl_rockmanx3_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xc9; })); // PC=a9d2, startup, anded with 0x9c
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0xf0; })); // PC=12e0, start of level

}

void megadrive_unl_rockmanx3_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x000c; }));
}

