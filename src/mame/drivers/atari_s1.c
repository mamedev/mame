/*
    Atari Generation/System 1
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

extern const char layout_pinball[];
class atari_s1_state : public driver_device
{
public:
	atari_s1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};

static ADDRESS_MAP_START( atari_s1_map, AS_PROGRAM, 8, atari_s1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x7000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s1 )
INPUT_PORTS_END

void atari_s1_state::machine_reset()
{
}

static DRIVER_INIT( atari_s1 )
{
}

static MACHINE_CONFIG_START( atari_s1, atari_s1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(atari_s1_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ The Atarians (11/1976)
/-------------------------------------------------------------------*/
ROM_START(atarians)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("atarian.e00", 0x7000, 0x0800, CRC(6066bd63) SHA1(e993497d0ca9f056e18838494089def8bdc265c9))
	ROM_LOAD("atarian.e0", 0x7800, 0x0800, CRC(45cb0427) SHA1(e286930ca36bdd0f79acefd142d2a5431fa8005b))
ROM_END

/*-------------------------------------------------------------------
/ The Atarians (working bootleg)
/-------------------------------------------------------------------*/
//ROM_START(atarianb)
//  ROM_REGION(0x10000, "maincpu", 0)
//  ROM_LOAD("atarianb.e00", 0x7000, 0x0800, CRC(74fc86e4) SHA1(135d75e5c03feae0929fa84caa3c802353cdd94e))
//  ROM_LOAD("atarian.e0", 0x7800, 0x0800, CRC(45cb0427) SHA1(e286930ca36bdd0f79acefd142d2a5431fa8005b))
//  ROM_RELOAD(0xf800, 0x0800)
//ROM_END

/*-------------------------------------------------------------------
/ Time 2000 (06/1977)
/-------------------------------------------------------------------*/
ROM_START(time2000)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("time.e00", 0x7000, 0x0800, CRC(e380f35c) SHA1(f2b4c508c8b7a2ce9924da97c05fb31d5115f36f))
	ROM_LOAD("time.e0", 0x7800, 0x0800, CRC(1e79c133) SHA1(54ce5d59a00334fcec8b12c077d70e3629549af0))
ROM_END

/*-------------------------------------------------------------------
/ Airborne Avenger (09/1977)
/-------------------------------------------------------------------*/
ROM_START(aavenger)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("airborne.e00", 0x7000, 0x0800, CRC(05ac26b8) SHA1(114d587923ade9370d606e428af02a407d272c85))
	ROM_LOAD("airborne.e0", 0x7800, 0x0800, CRC(44e67c54) SHA1(7f94189c12e322c41908d651cf6a3b6061426959))
ROM_END

/*-------------------------------------------------------------------
/ Middle Earth (02/1978)
/-------------------------------------------------------------------*/
ROM_START(midearth)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("609.bin", 0x7000, 0x0800, CRC(589df745) SHA1(4bd3e4f177e8d86bab41f3a14c169b936eeb480a))
	ROM_LOAD("608.bin", 0x7800, 0x0800, CRC(28b92faf) SHA1(8585770f4059049f1dcbc0c6ef5718b6ff1a5431))
ROM_END

/*-------------------------------------------------------------------
/ Space Riders (09/1978)
/-------------------------------------------------------------------*/
ROM_START(spcrider)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spacer.bin", 0x7000, 0x0800, CRC(3cf1cd73) SHA1(c46044fb815b439f12fb3e21c470c8b93ebdfd55))
	ROM_LOAD("spacel.bin", 0x7800, 0x0800, CRC(66ffb04e) SHA1(42d8b7fb7206b30478f631d0e947c0908dcf5419))
ROM_END


GAME( 1976, atarians, 0,		atari_s1, atari_s1, atari_s1, ROT0, "Atari","The Atarians", 	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
//GAME( 2002, atarianb, atarians,   atari_s1, atari_s1, atari_s1, ROT0, "Atari / Gaston","The Atarians (working bootleg)",  GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1977, time2000, 0,		atari_s1, atari_s1, atari_s1, ROT0, "Atari","Time 2000",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1977, aavenger, 0,		atari_s1, atari_s1, atari_s1, ROT0, "Atari","Airborne Avenger", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1978, midearth, 0,		atari_s1, atari_s1, atari_s1, ROT0, "Atari","Middle Earth", 	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1978, spcrider, 0,		atari_s1, atari_s1, atari_s1, ROT0, "Atari","Space Riders",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
