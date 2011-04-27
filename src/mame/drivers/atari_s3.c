/*
    Atari Generation/System 3
*/
#include "emu.h"
#include "cpu/m6800/m6800.h"

extern const char layout_pinball[];

class atari_s3_state : public driver_device
{
public:
	atari_s3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( atari_s3_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM // NVRAM
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s3 )
INPUT_PORTS_END

static MACHINE_RESET( atari_s3 )
{
}

static DRIVER_INIT( atari_s3 )
{
}

static MACHINE_CONFIG_START( atari_s3, atari_s3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(atari_s3_map)

	MCFG_MACHINE_RESET( atari_s3 )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Road Runner (??/1979)
/-------------------------------------------------------------------*/
ROM_START(roadrunr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0000.716", 0x2800, 0x0800, CRC(62f5f394) SHA1(ff91066d43d788119e3337788abd86e5c0bf2d92))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("3000.716", 0x3000, 0x0800, CRC(2fc01359) SHA1(d3df20c764bb68a5316367bb18d34a03293e7fa6))
	ROM_RELOAD(0xb000, 0x0800)
	ROM_LOAD("3800.716", 0x3800, 0x0800, CRC(77262408) SHA1(3045a732c39c96002f495f64ed752279f7d43ee7))
	ROM_RELOAD(0xb800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

GAME( 1979, roadrunr, 0, atari_s3, atari_s3, atari_s3, ROT0, "Atari","Road Runner", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
