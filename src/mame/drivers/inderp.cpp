// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
// PINBALL
// Skeleton driver for early Inder pinballs on "Indertronic B-1" hardware.
// Known pinballs to be dumped: Topaz (1979), Skateboard (1980)
// Hardware listing and ROM definitions from PinMAME.

/*
 Inder (early games, "Indertronic B-1")
 --------------------------------------

   Hardware:
   ---------
CPU:     M6502, R/C combo with 100kOhms and 10pF for clock, manual says 4 microseconds min. instruction execution
    INT: ? (somewhere in between 200 .. 250 Hz seems likely)
IO:      DMA only
DISPLAY: 6-digit, both 9-segment & 7-segment panels with direct segment access
SOUND:   simple tones, needs comparison with real machine
 */

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6502/m6502.h"

class inderp_state : public genpin_class
{
public:
	inderp_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, inderp_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0180, 0x01ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( inderp )
INPUT_PORTS_END

static MACHINE_CONFIG_START( inderp, inderp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1000000)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	/* sound hardware */
	//discrete ?
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END


ROM_START(centauri)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cent2.bin", 0x8800, 0x0800, CRC(4f1ad0bc) SHA1(0622c16520275e629eac27ae2575e4e433de56ed))
	ROM_LOAD("cent3.bin", 0x9000, 0x0400, CRC(f87abd63) SHA1(c3f48ffd46fad076fd064cbc0fdcc31641f5b1b6))
	ROM_RELOAD(0x9400, 0x0400)
	ROM_LOAD("cent4.bin", 0x9800, 0x0400, CRC(b69e95b6) SHA1(2f053a5848110d084239e1fc960198b247b3b98e))
	ROM_RELOAD(0x9c00, 0x0400)
	ROM_RELOAD(0xfc00, 0x0400)
ROM_END

ROM_START(centauri2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cent2.bin", 0x8800, 0x0800, CRC(4f1ad0bc) SHA1(0622c16520275e629eac27ae2575e4e433de56ed))
	ROM_LOAD("cent3a.bin", 0x9000, 0x0400, CRC(7b8215b1) SHA1(7cb6c18ad88060b56785bbde398bff157d8417cd))
	ROM_RELOAD(0x9400, 0x0400)
	ROM_LOAD("cent4a.bin", 0x9800, 0x0400, CRC(7ee64ea6) SHA1(b751b757faab7e3bb56625e4d72c3aeeb84a3f28))
	ROM_RELOAD(0x9c00, 0x0400)
	ROM_RELOAD(0xfc00, 0x0400)
ROM_END


GAME( 1979, centauri,  0,        inderp, inderp, driver_device, 0, ROT0, "Inder", "Centaur (Inder)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, centauri2, centauri, inderp, inderp, driver_device, 0, ROT0, "Inder", "Centaur (alternate set)", MACHINE_IS_SKELETON_MECHANICAL )
