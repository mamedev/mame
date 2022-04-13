// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton device for Roland PG-200 programmer.

****************************************************************************/

#include "emu.h"
#include "pg200.h"

DEFINE_DEVICE_TYPE(PG200, pg200_device, "pg200", "Roland PG-200 Programmer")

pg200_device::pg200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PG200, tag, owner, clock)
	, m_pgcpu(*this, "pgcpu")
{
}

void pg200_device::device_start()
{
}

void pg200_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_pgcpu, 6_MHz_XTAL);
}

ROM_START(pg200)
	ROM_REGION(0x400, "pgcpu", 0)
	ROM_LOAD("m5l8048-067p_b4d4.ic1", 0x000, 0x400, CRC(4306aad7) SHA1(145e12e5cf22b6db4958651a04d892f4a4215bb1))
ROM_END

const tiny_rom_entry *pg200_device::device_rom_region() const
{
	return ROM_NAME(pg200);
}
