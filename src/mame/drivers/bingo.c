#include "emu.h"
#include "cpu/s2650/s2650.h"

extern const char layout_pinball[];

class bingo_state : public driver_device
{
public:
	bingo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};


static ADDRESS_MAP_START( bingo_map, AS_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1eff) AM_ROM
	AM_RANGE(0x1f00, 0x1fff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( bingo )
INPUT_PORTS_END

static MACHINE_RESET( bingo )
{
}

static DRIVER_INIT( bingo )
{
}

static MACHINE_CONFIG_START( bingo, bingo_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 1000000)
	MCFG_CPU_PROGRAM_MAP(bingo_map)

	MCFG_MACHINE_RESET( bingo )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

ROM_START(cntinntl)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("bingo.u37", 0x1800, 0x0800, CRC(3b21b22c) SHA1(21b002dd0dd11ee55674955c67c627470f427591))
	ROM_LOAD("bingo.u40", 0x1000, 0x0800, CRC(67160fc8) SHA1(6b93c1a7edcd7079a1e7d8a926e72febe2b39e9e))
	ROM_LOAD("bingo.u44", 0x0800, 0x0800, CRC(068acc49) SHA1(34fa2977513276bd5adc0b06cf258bb5a3702ed2))
	ROM_LOAD("bingo.u48", 0x0000, 0x0800, CRC(81bbcb19) SHA1(17c3d900d1cbe3cb5332d830288ef2c578afe8f8))
ROM_END

ROM_START(goldgame)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("h9925_1.e", 0x80000, 0x10000, CRC(c5ec9181) SHA1(fac7fc0fbfddca44c728c78973ee5273a3d0bc43))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE("h9925_1.o", 0x80001, 0x10000, CRC(2a019eea) SHA1(3f013f97b0a92fc9085c7be3903cbf42e67c41e5))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(goldgam2)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("ah0127.evn", 0x80000, 0x10000, CRC(6456a021) SHA1(98137d3b63aa7453c624f477a0c6ea1e0996d3c2))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE("ah0127.ods", 0x80001, 0x10000, CRC(b538f435) SHA1(4d939554e997d630ffe7337e1f21ee53d6f06130))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END
/*
ROM_START(penalty)
    ROM_REGION(0x10000, "maincpu", 0)
    ROM_LOAD("13006-1.epr", 0x8000, 0x8000, CRC(93cfbec9) SHA1(c245604ac42c88c647950db4497a6f9dd3504955))
    ROM_LOAD("13006-2.epr", 0x0000, 0x4000, CRC(41470cc1) SHA1(7050df563fddbe8216317d96664d12567b618645))
ROM_END

ROM_START(brooklyn)
    ROM_REGION(0x10000, "maincpu", 0)
    ROM_LOAD("n10207-1.epr", 0x8000, 0x8000, CRC(7851f870) SHA1(8da400108a352954ced8fc942663c0635bec4d1c))
    ROM_LOAD("n10207-2.epr", 0x0000, 0x4000, CRC(861dae09) SHA1(d808fbbf6b50e1482a512b9bd1b18a023694adb2))
ROM_END

ROM_START(newdixie)
    ROM_REGION(0x10000, "maincpu", 0)
    ROM_LOAD("10307-1.epr", 0x8000, 0x8000, CRC(7b6b2e9c) SHA1(149c9e1d2a3e7db735835c6fa795e41b2eb45175))
    ROM_LOAD("10307-2.epr", 0x0000, 0x4000, CRC(d99a7866) SHA1(659a0107bc970d2578dcfd7cdd43661da778fd5c))
ROM_END

*/

GAME(1980,	cntinntl,		0,			bingo,	bingo,	bingo,	ROT0,	"Bally",		"Continental (Bingo)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(19??,	goldgame,		0,			bingo,	bingo,	bingo,	ROT0,	"Splin",		"Golden Game (Bingo)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(19??,	goldgam2,		goldgame,	bingo,	bingo,	bingo,	ROT0,	"Splin",		"Golden Game Stake 6/10 (Bingo)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)

/*CORE_GAMEDEFNV (penalty,  "Penalty (Bingo)",  19??, "Seeben (Belgium)", seeben, GAME_NOT_WORKING)
CORE_GAMEDEFNV (brooklyn, "Brooklyn (Bingo)", 19??, "Seeben (Belgium)", seeben, GAME_NOT_WORKING)
CORE_GAMEDEFNV (newdixie, "New Dixieland (Bingo)", 19??, "Sirmo (Belgium)", seeben, GAME_NOT_WORKING)*/
