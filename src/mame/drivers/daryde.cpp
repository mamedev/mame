// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Daryde Panther Darts.

*******************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/timekpr.h"

class daryde_state : public driver_device
{
public:
	daryde_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void pandart(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);
};

void daryde_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("program", 0);
	map(0xef000, 0xef005).unmapw();
	map(0xf0000, 0xf1fff).rw("timekpr", FUNC(mk48t08_device::read), FUNC(mk48t08_device::write));
}

void daryde_state::io_map(address_map &map)
{
	map(0x0000, 0x00ff).noprw();
	map(0x4000, 0x4000).unmapr();
	map(0xc000, 0xc000).unmaprw();
}

static INPUT_PORTS_START(pandart)
INPUT_PORTS_END

void daryde_state::pandart(machine_config &config)
{
	cpu_device &maincpu(Z180(config, "maincpu", XTAL(18'432'000)));
	maincpu.set_addrmap(AS_PROGRAM, &daryde_state::mem_map);
	maincpu.set_addrmap(AS_IO, &daryde_state::io_map);

	MK48T08(config, "timekpr");
}

/* Daryde Panther Darts PCB
 ____________________________________________________________________________
 | ____________________   _____________________   _______   _______          |
 | ||||CN8 (16 pins)|||   ||||CN5 (16 pins)||||   ||CN6||   |CN15||     ___  |
 | ___                           ___________  ___________  __________   |__| |
 | |CN3                          |_74HC273N_| |_74HC273N_| |_ULN2803A|  |__| |
 | |__|                          ___________  ___________  __________   |C_| |
 |  __                           |_74HC273N_| |_74HC273N_| |_ULN2803A|  |N_| |
 | |CN7                          ___________  ___________  __________   |4_| |
 |  ____                         |_74HC273N_| |_74HC273N_| |_ULN2803A|  |__| |
 | |CN12|                          _________  ___________               |__| |
 | |DB9 |   _________              74HC138AN| |_74HC273N_|                   |
 | |    |   |MAX232N_|             _________  ___________               ___  |
 | |____| ___________              CD4051BCN| |_74HC244N_|              |__| |
 |  ___   |PALCE16V8H|       _______________                            |__| |
 |  |CN14 ______________     |              | ___________               |C_| |
 |  |__|  |            |     |M48T08-150PCI | |_74HC244N_|              |N_| |
 |  ___   |  CPU (IC6) |     |______________|                           |9_| |
 |  |CN11 |            |     _______________  ___________  ___          |__| |
 |  |  |  |            |     |AM27C040 (IC5)| |_74HC244N_| |H606016     |__| |
 |  |  |  |____________|     |______________|                           |__| |
 |  |__|  _____       _________   _________  ___________     ____            |
 |        |XTAL|      |||CN2|||   |||CN1|||  |||CN13||||     |CN10           |
 |___________________________________________________________________________|

Xtal = 18.432 MHz

CN1 = 6 pins
CN2 = 8 pins
CN4 = 20 pins
CN5 = 16 pins
CN6 = 6 pins
CN8 = 16 pins
CN9 = 26 pins
CN10 = 4 pins
CN11 = 10 pins
CN12 = 9 pins (DB9)
CN13 = 16 pins, but no connector, 2200μF25V capacitor between last pins
CN14 = 6 pins
CN15 = 5 pins */
ROM_START(pandart)
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("27c040.ic5", 0x00000, 0x80000, CRC(b1bd5c14) SHA1(7164dcaebf0f23f5330b225e44ee87d9a8c79f4f))

	ROM_REGION(0x117, "pal", 0)
	ROM_LOAD("palce16v8h.ic1", 0x000, 0x117, NO_DUMP) // protected
ROM_END

GAME(1999, pandart, 0, pandart, pandart, daryde_state, empty_init, ROT0, "Daryde S. L.", "Panther Darts", MACHINE_IS_SKELETON_MECHANICAL)
