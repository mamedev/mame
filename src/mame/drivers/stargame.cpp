// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
// PINBALL
// Skeleton driver for Stargame pinballs (2 x Z80, Z80CTC, DAC, AY8910, MEA8000).
// Hardware listing and ROM definitions from PinMAME.



#include "emu.h"
#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/mea8000.h"


class stargame_state : public genpin_class
{
public:
	stargame_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, stargame_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( stargame )
INPUT_PORTS_END

static MACHINE_CONFIG_START( stargame, stargame_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 1000000)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	/* sound hardware */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

ROM_START(spcship)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("sss-1g.bin", 0x0000, 0x4000, CRC(119a3064) SHA1(d915ecf44279a9e16a50a723eb9523afec1fb380))
	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sss-1a0.bin", 0x0000, 0x4000, CRC(eae78e63) SHA1(9fa3587ae3ee6f674bb16102680e70069e9d275e))
ROM_END


ROM_START(whtforce)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m5l.bin", 0x0000, 0x4000, CRC(22495322) SHA1(b34a34dec875f215d566d18a5e877b9185a22ab7))
	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.bin", 0x0000, 0x4000, CRC(4b2a1580) SHA1(62133fd186b1aab4f5aecfbff8151ba416328021))
ROM_END


GAME( 1986, spcship,  0, stargame, stargame, driver_device, 0, ROT0, "Stargame", "Space Ship (Pinball)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, whtforce, 0, stargame, stargame, driver_device, 0, ROT0, "Stargame", "White Force", MACHINE_IS_SKELETON_MECHANICAL )
