// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista

// Skeleton driver for Joctronic pinballs.
// Known pinballs to be dumped: Rider's Surf (1986), Pin Ball (1987)
// ROM definitions from PinMAME.

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/genpin.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

class joctronic_state : public genpin_class
{
public:
	joctronic_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, joctronic_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( maincpu_io_map, AS_IO, 8, joctronic_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START( audiocpu_map, AS_PROGRAM, 8, joctronic_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audiocpu_io_map, AS_IO, 8, joctronic_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( joctronic )
INPUT_PORTS_END

static MACHINE_CONFIG_START( joctronic, joctronic_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3000000)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
	MCFG_CPU_IO_MAP(maincpu_io_map)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)
	MCFG_CPU_PROGRAM_MAP(audiocpu_map)
	MCFG_CPU_IO_MAP(audiocpu_io_map)

	//z80ctc

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	/* sound hardware */
	//ay8910
	//dac
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ Punky Willy (1986)
/-------------------------------------------------------------------*/
ROM_START(punkywil)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("pw_game.bin", 0x0000, 0x4000, NO_DUMP)

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("pw_sound.bin", 0x0000, 0x4000, CRC(b2e3a201) SHA1(e3b0a5b22827683382b61c21607201cd470062ee))
ROM_END


/*-------------------------------------------------------------------
/ Walkyria (198?)
/-------------------------------------------------------------------*/
ROM_START(walkyria)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("wk_game.bin", 0x0000, 0x4000, CRC(709722bc) SHA1(4d7b68e9d4a50846cf8eb308ba92f5e2115395d5))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("wk_sound.bin", 0x0000, 0x4000, CRC(81f74b0a) SHA1(1ef73bf42f5b1f54526202d3ff89698a04c7b41a))
ROM_END


GAME( 1986, punkywil, 0, joctronic, joctronic, driver_device, 0, ROT0, "Joctronic", "Punky Willy", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 198?, walkyria, 0, joctronic, joctronic, driver_device, 0, ROT0, "Joctronic", "Walkyria", MACHINE_IS_SKELETON_MECHANICAL )
