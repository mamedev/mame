/*
    Williams System 4
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s4_state : public driver_device
{
public:
	williams_s4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};

static ADDRESS_MAP_START( williams_s4_map, AS_PROGRAM, 8, williams_s4_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s4 )
INPUT_PORTS_END

void williams_s4_state::machine_reset()
{
}

static DRIVER_INIT( williams_s4 )
{
}

static MACHINE_CONFIG_START( williams_s4, williams_s4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(williams_s4_map)
MACHINE_CONFIG_END

/*--------------------------------
/ Flash - Sys.4 (Game #486)
/-------------------------------*/
ROM_START(flash_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

ROM_START(flash_t1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Phoenix - Sys.4 (Game #485)
/-------------------------------*/
ROM_START(phnix_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(3aba6eac) SHA1(3a9f669216b3214bc42a1501aa2b10cfbcc36315))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Pokerino - Sys.4 (Game #488)
/-------------------------------*/
ROM_START(pkrno_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(9b4d01a8) SHA1(1bd51745f38381ffc66fde4b28b76aab33b573ca))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Stellar Wars - Sys.4 (Game #490)
/-------------------------------*/
ROM_START(stlwr_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(874e7ef7) SHA1(271aeac2a0e61cb195811ae2e8d908cb1ab45874))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("yellow2.716", 0x7800, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("yellow1.716", 0x7000, 0x0800, CRC(d251738c) SHA1(65ddbf5c36e429243331a4c5d2339df87a8a7f64))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Pompeii (Shuffle)
/----------------------------*/
ROM_START(pomp_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Aristocrat (Shuffle)
/----------------------------*/
ROM_START(arist_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Topaz (Shuffle)
/----------------------------*/
ROM_START(topaz_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(cb287b10) SHA1(7fb6b6a26237cf85d5e02cf35271231267f90fc1))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Taurus (Shuffle)
/----------------------------*/
ROM_START(taurs_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(3246e285) SHA1(4f76784ecb5063a49c24795ae61db043a51e2c89))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ King Tut
/----------------------------*/
ROM_START(kingt_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(54d3280a) SHA1(ca74636e35d2c3e0b3133f89b1ff1233d5d72a5c))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Omni (Shuffle)
/----------------------------*/
ROM_START(omni_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("omni-1a.u21", 0x6000, 0x0800, CRC(443bd170) SHA1(cc1ebd72d77ec2014cbd84534380e5ea1f12c022))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound.716", 0x7800, 0x0800, CRC(db085cbb) SHA1(9a57abbad183ba16b3dba16d16923c3bfc46a0c3))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Big Strike (Shuffle)
/----------------------------*/
ROM_START(bstrk_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(323dbcde) SHA1(a75cbb5de97cb9afc1d36e9b6ff593bb482fcf8b))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_RELOAD( 0xf000, 0x0800)
ROM_END

/*----------------------------
/ Triple Strike (Shuffle)
/----------------------------*/
ROM_START(tstrk_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(b034c059) SHA1(76b3926b87b3c137fcaf33021a586827e3c030af))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("ic20.716", 0x7000, 0x0800, CRC(f163fc88) SHA1(988b60626f3d4dc8f4a1dbd0c99282418bc53aae))
	ROM_RELOAD( 0xf000, 0x0800)
ROM_END


GAME(1979,	flash_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Flash (L-1)",						GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	flash_t1,		flash_l1,	williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Flash (T-1) Ted Estes",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	phnix_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Phoenix (L-1)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	pkrno_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Pokerino (L-1)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	stlwr_l2,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Stellar Wars (L-2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	pomp_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Pompeii (Shuffle) (L-1)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	arist_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Aristocrat (Shuffle) (L-1)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	topaz_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Topaz (Shuffle) (L-1)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	taurs_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Taurus (Shuffle) (L-1)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	kingt_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"King Tut (Shuffle) (L-1)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	omni_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Omni (Shuffle) (L-1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1983,	bstrk_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Big Strike (Shuffle) (L-1)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1983,	tstrk_l1,		0,			williams_s4,	williams_s4,	williams_s4,	ROT0,	"Williams",				"Triple Strike (Shuffle) (L-1)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
