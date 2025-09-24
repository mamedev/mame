// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************************************

    Skeleton device for Sun Microsystems 270-4373-07 Fiber Channel Network Card.
    Main components:
     -Agilent HDMP-1646A.
     -125 MHz xtal.
     -Sun Microsystems 100-5234-03 SME2005PBGAC.
     -LT1587.
     -Infineon V23826-K305-C13.

*******************************************************************************************/

#include "emu.h"
#include "sun_fcnc_270_4373_07.h"

DEFINE_DEVICE_TYPE(SUN_FCNC_270_4373_07, sun_fcnc_270_4373_07_device, "sun_fcnc_270_4373_07", "Sun Microsystems 270-4373-07 Gigabit Fiber Channel Network Card")

sun_fcnc_270_4373_07_device::sun_fcnc_270_4373_07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUN_FCNC_270_4373_07, tag, owner, clock)
{
}

void sun_fcnc_270_4373_07_device::device_start()
{
}

void sun_fcnc_270_4373_07_device::device_add_mconfig(machine_config &config)
{
}

ROM_START(sun_fcnc_270_4373_07)
	ROM_REGION(0x10000, "bios", 0)
	ROM_LOAD("517543-ee17c8_at29c512.u0901", 0x00000, 0x10000, CRC(d0797304) SHA1(b22046b8b574acf7818ebb00dc8d78c04c170ac0)) // 1.7.0, 22-Jun-1998
ROM_END

const tiny_rom_entry *sun_fcnc_270_4373_07_device::device_rom_region() const
{
	return ROM_NAME(sun_fcnc_270_4373_07);
}
