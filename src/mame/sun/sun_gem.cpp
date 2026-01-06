// license:BSD-3-Clause
// copyright-holders:

/************************************************************************************************

    Skeleton device for Sun Microsystems Gigabit Ethernet 2.0/3.0 [GEM] PCI cards (P/N 501-4373).
    Main components:
     -Agilent HDMP-1646A.
     -125 MHz xtal.
     -Sun Microsystems 100-5234-03 SME2005PBGAC.
     -LT1587.
     -Infineon V23826-K305-C13.

************************************************************************************************/

#include "emu.h"
#include "sun_gem.h"

DEFINE_DEVICE_TYPE(SUN_GEM, sun_gem_device, "sun_gem", "Sun Microsystems Gigabit Ethernet 2.0/3.0 [GEM] card")

sun_gem_device::sun_gem_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUN_GEM, tag, owner, clock)
{
}

void sun_gem_device::device_start()
{
}

void sun_gem_device::device_add_mconfig(machine_config &config)
{
	// set_ids(0x108e2bad ...
}

ROM_START(sun_gem)
	ROM_REGION(0x10000, "bios", 0)
	ROM_LOAD("517543-ee17c8_at29c512.u0901", 0x00000, 0x10000, CRC(d0797304) SHA1(b22046b8b574acf7818ebb00dc8d78c04c170ac0)) // 1.7.0, 22-Jun-1998
ROM_END

const tiny_rom_entry *sun_gem_device::device_rom_region() const
{
	return ROM_NAME(sun_gem);
}
