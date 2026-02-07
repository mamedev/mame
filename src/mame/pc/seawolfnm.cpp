// license:BSD-3-Clause
// copyright-holders:

/****************************************************************************

Skeleton driver for "Sea Wolf - Next Mission", from Coastal Amusements Inc.

Based on an Foxconn "45CMX" PCB:
-Intel Celeron SL9XN 1.8GHz/512/800/06 CPU.
-1024MB DDR2-667 RAM.
-North Bridge: Intel 945GC/945G chipset; South Bridge: Intel ICH7.

Jaton Video-PX8400GS-EX video board with 512 MB RAM (NVIDIA GeForce 8400GS).

Coastal Game I/O PCB:
-Atmel 89C5131A near a 24MHz cristal.
-Buttons for set and calibration.
-Etc.

****************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class seawolfnm_state : public driver_device
{
public:
	seawolfnm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void seawolfnm(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void seawolfnm_io(address_map &map) ATTR_COLD;
	void seawolfnm_map(address_map &map) ATTR_COLD;
};


void seawolfnm_state::seawolfnm_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

void seawolfnm_state::seawolfnm_io(address_map &map)
{
}

static INPUT_PORTS_START(seawolfnm)
INPUT_PORTS_END

void seawolfnm_state::seawolfnm(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Actually an Intel Celeron SL9XN 1.8GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seawolfnm_state::seawolfnm_map);
	m_maincpu->set_addrmap(AS_IO, &seawolfnm_state::seawolfnm_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(seawolfnm)
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("25l4005.bin", 0x00000, 0x80000, CRC(2a6b7cf2) SHA1(0be15605c686bec5109b1cbe91b45edca6c46d35))

	ROM_REGION(0x8000, "io", 0)
	ROM_LOAD("sea_wolf_1_89c5131a.bin", 0x0000, 0x8000, NO_DUMP)

	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("sea_wolf_the_next_mission_0.9.9_12-25-08_coastal_amusements_inc", 0, BAD_DUMP SHA1(dc47d60ba04d33aece91838598828edbf79e8eb5)) // Image tested on real hardware, but may contain operator or players data
ROM_END

} // anonymous namespace


GAME(2008, seawolfnm, 0, seawolfnm, seawolfnm, seawolfnm_state, empty_init, ROT0, "Coastal Amusements", "Sea Wolf - Next Mission", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
