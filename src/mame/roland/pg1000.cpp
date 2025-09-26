// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/****************************************************************************

    Skeleton device for Roland PG-1000 programmer.

****************************************************************************/

#include "emu.h"
#include "pg1000.h"

DEFINE_DEVICE_TYPE(PG1000, pg1000_device, "pg1000", "Roland PG-1000 Programmer")

pg1000_device::pg1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PG1000, tag, owner, clock)
	, m_pgcpu(*this, "pgcpu")
{
}

void pg1000_device::device_start()
{
}

void pg1000_device::device_add_mconfig(machine_config &config)
{
	UPD78C10(config, m_pgcpu, 12_MHz_XTAL);
}

ROM_START(pg1000)
	ROM_DEFAULT_BIOS("v2.00")
	ROM_SYSTEM_BIOS(0, "v2.00", "Version 2.00")
	ROM_SYSTEM_BIOS(1, "v1.01", "Version 1.01")
	ROM_SYSTEM_BIOS(2, "v1.00", "Version 1.00")

	ROM_REGION(0x8000, "pgcpu", 0)
	ROMX_LOAD("roland_pg-1000_v2.00.ic4", 0x000, 0x2000, CRC(c8bc1f62) SHA1(796d5efd09b411d370f93b32283aa33a4435dec4), ROM_BIOS(0))
	ROMX_LOAD("roland_pg-1000_v1.01.ic4", 0x000, 0x2000, CRC(9f9bcf76) SHA1(da5a45c65a04c35d7a615c6f043ccaf958b0d65e), ROM_BIOS(1))
	ROMX_LOAD("roland_pg-1000_v1.00.ic4", 0x000, 0x2000, CRC(c09ef84e) SHA1(d780d4d53e57918e6ea8098f54f5c9b43aeec287), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *pg1000_device::device_rom_region() const
{
	return ROM_NAME(pg1000);
}
