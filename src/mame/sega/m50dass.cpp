// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************************************

    Skeleton device for Sega DASS (Dual Active Seat System), found on the Megalo 50 cabinet.
    Sega PCB 837-8683:
     -Z0840004PSC Z80 CPU
     -YM-2413
     -Two Sega 315-5296 I/O chips
     -3 banks of eight dip switches
    There is at least an undumped "A" revision.

*******************************************************************************************/

#include "m50dass.h"

#include "emu.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(MEGALO50_DASS, m50dass_device, "m50dass", "Sega DASS")

m50dass_device::m50dass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MEGALO50_DASS, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
	, m_ym2413(*this, "ym2413")
{
}

void m50dass_device::device_start()
{
}

void m50dass_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // Z0840004PSC, divider not verified

	SPEAKER(config, "mono").front_center();
	YM2413(config, m_ym2413, 8_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "mono", 0.8); // divider and configuration unknown
}

ROM_START(m50dass)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("epr-14705.ic18", 0x00000, 0x20000, CRC(b863d2c5) SHA1(f6aa309bc8be15c26d91f3cf048ea140a9ca12eb)) // Strings "MEGALO50  910322", "**  SHUNICHI  **" on ROM

	ROM_REGION(0x117, "pld", 0)
	ROM_LOAD("315-5592_gal16v8a.ic19", 0x000, 0x117, NO_DUMP)
ROM_END
