// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/******************************************************************************

    ACI Prodigy chess computer driver

    TODO: Everything

    +-------------------------------------------------------------------------------------+
    |LEDS--------------------------------------------+        +-----------------+         |
    | | |              o o o o o o o o COLCN         |        |                 |         |
    | V |                                            |        | 4 char BCD LED  |         |
    | O | 8                                          |        +-----------------+         |
    |   |                                      ROWCN |        |||||||||||||||||||  ____   |
    | O | 7                                         o|+----------------------+    /    \  |
    |   |                                           o||  VIA                 |   ( beep ) |
    | O | 6                                         o|| R6522                |    \____/  |
    |   |                                           o|+----------------------+            |
    | O | 5                                         o|+----------------------+            |
    |   |                                           o||  CPU                 |            |
    | O | 4         8 x 8 button matrix             o|| R6502-13             |   +--+  __ |
    |   |                                           o|+----------------------+   |74| PWR||
    | O | 3                                         o|   o    o  +-------------+ |LS| SW >|
    |   |                                            | +=======+ |  ROM        | |14|  __||
    | O | 2                                          | | 2MHz  | | R2912       | +--+     |
    |   |                                            | |  XTAL | +-------------+     +--+ |
    | O | 1                                          |++-------+ +-------------+     |74| |
    |   |                                            || 74145N | |  RAM        |     |LS| |
    |   |   A     B    C    D    E    F     G    H   |+--------+ | M58725P     |     |00| |
    |   +--------------------------------------------+           +-------------+     +--+ |
    |LEDS-> O     O    O    O    O    O     O    O                 OOOOOOOOOOO KPDCN      |
    +-------------------------------------------------------------------------------------+

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"

class prodigy_state : public driver_device
{
public:
	prodigy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, "via")
	{ }
private:
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
};

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, prodigy_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x200f) AM_DEVREADWRITE("via", via6522_device, read, write)
	AM_RANGE(0x6000, 0x7fff) AM_ROM AM_REGION("roms", 0x0000) AM_MIRROR(0x8000)
ADDRESS_MAP_END

static INPUT_PORTS_START( prodigy )
INPUT_PORTS_END

static MACHINE_CONFIG_START( prodigy, prodigy_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
	MCFG_DEVICE_ADD("via", VIA6522, XTAL_2MHz)
MACHINE_CONFIG_END

ROM_START(prodigy)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("0x2000.bin",  0x0000, 0x02000, CRC(8d60345a) SHA1(fff18ff12e1b1be91f8eac1178605a682564eff2))
ROM_END

/*    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT      INIT,             COMPANY,                FULLNAME,              FLAGS */
CONS( 1981, prodigy,    0,        0,      prodigy,   prodigy,   driver_device, 0, "Applied Concepts Inc", "ACI Destiny Prodigy", MACHINE_IS_SKELETON)
