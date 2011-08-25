#include "emu.h"
#include "cpu/m6809/m6809.h"

class icecold_state : public driver_device
{
public:
	icecold_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( icecold_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( icecold )
INPUT_PORTS_END

static MACHINE_RESET( icecold )
{
}

static DRIVER_INIT( icecold )
{
}

static MACHINE_CONFIG_START( icecold, icecold_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 1000000)
	MCFG_CPU_PROGRAM_MAP(icecold_map)

	MCFG_MACHINE_RESET( icecold )
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Ice Cold Beer
/-------------------------------------------------------------------*/
ROM_START(icecold)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("icb23b.bin", 0xe000, 0x2000, CRC(b5b69d0a) SHA1(86f5444700adebb7b2d9da702b6d5425c8d682e3))
	ROM_LOAD("icb24.bin",  0xc000, 0x2000, CRC(2d1e7282) SHA1(6f170e24f71d1504195face5f67176b55c933eef))
ROM_END

GAME(1983,	icecold,	0,		icecold,	icecold,	icecold,	ROT0,	"Taito",			"Ice Cold Beer",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
