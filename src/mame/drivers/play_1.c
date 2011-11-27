/*
    Playmatic MPU 1
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/cosmac/cosmac.h"

class play_1_state : public driver_device
{
public:
	play_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cosmac_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};

static ADDRESS_MAP_START( play_1_map, AS_PROGRAM, 8, play_1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( play_1 )
INPUT_PORTS_END

void play_1_state::machine_reset()
{
}

static DRIVER_INIT( play_1 )
{
}

static COSMAC_INTERFACE( cdp1802_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( play_1, play_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COSMAC, 400000)
	MCFG_CPU_PROGRAM_MAP(play_1_map)
	MCFG_CPU_CONFIG(cdp1802_config)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Space Gambler (03/78)
/-------------------------------------------------------------------*/
ROM_START(spcgambl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spcgamba.bin", 0x0000, 0x0400, CRC(3b6e5287) SHA1(4d2fae779bb4117a99a9311b96ab79799f40067b))
	ROM_LOAD("spcgambb.bin", 0x0400, 0x0400, CRC(5c61f25c) SHA1(44b2d74926bf5678146b6d2347b4147e8a29a660))
ROM_END

/*-------------------------------------------------------------------
/ Big Town  (04/78)
/-------------------------------------------------------------------*/
ROM_START(bigtown)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bigtowna.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("bigtownb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Last Lap (09/78)
/-------------------------------------------------------------------*/
ROM_START(lastlap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lastlapa.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("lastlapb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Chance (09/78)
/-------------------------------------------------------------------*/
ROM_START(chance)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("chance_a.bin", 0x0000, 0x0400, CRC(3cd9d5a6) SHA1(c1d9488495a67198f7f60f70a889a9a3062c71d7))
	ROM_LOAD("chance_b.bin", 0x0400, 0x0400, CRC(a281b0f1) SHA1(1d2d26ce5f50294d5a95f688c82c3bdcec75de95))
	ROM_LOAD("chance_c.bin", 0x0800, 0x0200, CRC(369afee3) SHA1(7fa46c7f255a5ef21b0d1cc018056bc4889d68b8))
ROM_END

/*-------------------------------------------------------------------
/ Party  (05/79)
/-------------------------------------------------------------------*/
ROM_START(party)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("party_a.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("party_b.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

GAME(1978,	bigtown,	0,		play_1,	play_1,	play_1,	ROT0,	"Playmatic",	"Big Town",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	chance,		0,		play_1,	play_1,	play_1,	ROT0,	"Playmatic",	"Chance",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	lastlap,	0,		play_1,	play_1,	play_1,	ROT0,	"Playmatic",	"Last Lap",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	spcgambl,	0,		play_1,	play_1,	play_1,	ROT0,	"Playmatic",	"Space Gambler",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	party,		0,		play_1,	play_1,	play_1,	ROT0,	"Playmatic",	"Party",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
