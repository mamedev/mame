/*
    Williams System 3
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s3_state : public driver_device
{
public:
	williams_s3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};

static ADDRESS_MAP_START( williams_s3_map, AS_PROGRAM, 8, williams_s3_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x6000, 0x67ff) AM_ROM
	AM_RANGE(0x7000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s3 )
INPUT_PORTS_END

void williams_s3_state::machine_reset()
{
}

static DRIVER_INIT( williams_s3 )
{
}
static MACHINE_CONFIG_START( williams_s3, williams_s3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 3580000 / 4)
	MCFG_CPU_PROGRAM_MAP(williams_s3_map)
MACHINE_CONFIG_END

/*-------------------------------------
/ Contact - Sys.3 (Game #482)
/-------------------------------------*/
ROM_START(cntct_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(35359b60) SHA1(ab4c3328d93bdb4c952090b327c91b0ded36152c))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------
/ Disco Fever - Sys.3 (Game #483)
/-------------------------------------*/
ROM_START(disco_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(831d8adb) SHA1(99a9c3d5c8cbcdf3bb9c210ad9d05c34905b272e))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Hot Tip - Sys.3 (Game #477) - No Sound board
/----------------------------*/
ROM_START(httip_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(b1d4fd9b) SHA1(e55ecf1328a55979c4cf8f3fb4e6761747e0abc4))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
ROM_END

/*---------------------------------
/ Lucky Seven - Sys.3 (Game #480) - No Sound board
/---------------------------------*/
ROM_START(lucky_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(7cfbd4c7) SHA1(825e2245fd1615e932973f5e2b5ed5f2da9309e7))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
ROM_END

/*-------------------------------------
/ World Cup Soccer - Sys.3 (Game #481)
/-------------------------------------*/
ROM_START(wldcp_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(c8071956) SHA1(0452aaf2ec1bcc5717fe52a6c541d79402bebb17))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2wc.716", 0x7800, 0x0800, CRC(618d15b5) SHA1(527387893eeb2cd4aa563a4cfb1948a15d2ed741))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END


GAME( 1977, httip_l1, 0, williams_s3, williams_s3, williams_s3, ROT0, "Williams", "Hot Tip (L-1)",			GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, lucky_l1, 0, williams_s3, williams_s3, williams_s3, ROT0, "Williams", "Lucky Seven (L-1)",		GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, wldcp_l1, 0, williams_s3, williams_s3, williams_s3, ROT0, "Williams", "World Cup Soccer (L-1)", 	GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, cntct_l1, 0, williams_s3, williams_s3, williams_s3, ROT0, "Williams", "Contact (L-1)",			GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, disco_l1, 0, williams_s3, williams_s3, williams_s3, ROT0, "Williams", "Disco Fever (L-1)",		GAME_IS_SKELETON_MECHANICAL)
