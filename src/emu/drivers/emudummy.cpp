// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emudummy.c

    Dummy driver file that references CPU devices which are in turn
    referenced by devices in libemu.a.

    The reason we need this is due to link ordering issues with gcc
    if the actual drivers being linked don't reference these CPU
    devices. Since we link libcpu first, if libemu needs stuff from
    libcpu that wasn't previously referenced, it will fail the link.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"


MACHINE_CONFIG_START( __dummy, driver_device )
	MCFG_CPU_ADD("dummy1", I8049, 1000000)
	MCFG_CPU_ADD("dummy2", I8748, 1000000)
	MCFG_CPU_ADD("dummy3", Z80, 1000000)
MACHINE_CONFIG_END


ROM_START( __dummy )
	ROM_REGION( 0x1000, "dummy1", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000, "dummy2", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000, "dummy3", ROMREGION_ERASEFF )
ROM_END


GAME( 1900, __dummy, 0, __dummy, 0, driver_device, 0, ROT0, "(none)", "Dummy", MACHINE_NO_SOUND )
