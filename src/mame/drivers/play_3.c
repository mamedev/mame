/*
    Playmatic MPU 3
*/
#include "emu.h"
#include "cpu/cosmac/cosmac.h"

class play_3_state : public driver_device
{
public:
	play_3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( play_3_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( play_3 )
INPUT_PORTS_END

static MACHINE_RESET( play_3 )
{
}

static DRIVER_INIT( play_3 )
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

static MACHINE_CONFIG_START( play_3, play_3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COSMAC, 2950000)
	MCFG_CPU_PROGRAM_MAP(play_3_map)
	MCFG_CPU_CONFIG(cdp1802_config)

	MCFG_MACHINE_RESET( play_3 )
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Meg Aaton (1983)
/-------------------------------------------------------------------*/
ROM_START(megaaton)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpumegat.bin", 0x0000, 0x2000, CRC(7e7a4ede) SHA1(3194b367cbbf6e0cb2629cd5d82ddee6fe36985a))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(92fa0742) SHA1(ef3100a53323fd67e23b47fc3e72fdb4671e9b0a))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
ROM_END

GAME(1983,	megaaton,	0,	play_3,	play_3,	play_3,	ROT0,	"Playmatic",		"Meg Aaton",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
