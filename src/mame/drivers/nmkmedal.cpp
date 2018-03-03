// license:BSD-3-Clause
// copyright-holders:

// Skeleton driver for medal games on NMK hardware.

#include "emu.h"
#include "cpu/tlcs90/tlcs90.h"
#include "sound/okim6376.h"
#include "speaker.h"


/*
Trocana by NMK / NTC. Possibly distributed by Face?

Video of the game: https://www.youtube.com/watch?v=s63Gokcyn8M
Recording of some of the music: https://www.youtube.com/watch?v=TZMr-MX_M0w

PCBs:

NMK MEC95110 - maincpu board
- Toshiba TMP90C041AN
- 16.5000 MHz XTAL
- maincpu ROM
- Oki M6650
- OKI ROM
- 8 x connectors

NMK MEC95110-SUB2
- 4 x connectors

NMK MEC95110-SUB3
- 3 x connectors
*/

class nmkmedal_state : public driver_device
{
public:
	nmkmedal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void trocana(machine_config &config);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_START(nmkmedal_state::mem_map)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( trocana )
INPUT_PORTS_END

MACHINE_CONFIG_START(nmkmedal_state::trocana)
	MCFG_CPU_ADD("maincpu", TMP90841, 16500000 / 2) // actually TMP90C041AN
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("oki", OKIM6376, 16500000 / 16) // actually MSM6650
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( trocana)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "TRO1E.u12", 0x00000, 0x10000, CRC(f285043f) SHA1(6691091c1ecdab10c390db1d82c9d1d1dd0ded1f) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD( "TRO2.u16",  0x00000, 0x80000, CRC(c801d8ca) SHA1(f57026f5386467c054299556dd8665e62557aa91) )
ROM_END


GAME( 1996, trocana, 0, trocana, trocana, nmkmedal_state, 0, ROT0, "NTC / NMK", "Trocana", MACHINE_IS_SKELETON_MECHANICAL ) // NMK LTD, NTC LTD, V96313 strings
