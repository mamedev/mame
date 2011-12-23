/*
    DataEast/Sega Version 2
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class de_2_state : public driver_device
{
public:
	de_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( de_2_map, AS_PROGRAM, 8, de_2_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( de_2 )
INPUT_PORTS_END

void de_2_state::machine_reset()
{
}

static DRIVER_INIT( de_2 )
{
}

static MACHINE_CONFIG_START( de_2, de_2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(de_2_map)
MACHINE_CONFIG_END

/*-----------------------------------------------------------------------------------
/ Monday Night Football - CPU Rev 2 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/----------------------------------------------------------------------------------*/
ROM_START(mnfb_c27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mnfb2-7.b5", 0x4000, 0x4000, CRC(995eb9b8) SHA1(d05d74393fda59ffd8d7b5546313779cdb10d23e))
	ROM_LOAD("mnfb2-7.c5", 0x8000, 0x8000, CRC(579d81df) SHA1(9c96da34d37d3369513003e208222bd6e8698638))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mnf-f7.256", 0x8000, 0x8000, CRC(fbc2d6f6) SHA1(33173c081de776d32e926481e94b265ec48d770b))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("mnf-f5-6.512", 0x00000, 0x10000, CRC(0c6ea963) SHA1(8c88fa588222ef8a6c872b8c5b49639b108384d4))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("mnf-f4-5.512", 0x20000, 0x10000, CRC(efca5d80) SHA1(9655c885dd64aa170205170b6a0c052bd9367379))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

/*-------------------------------------------------------------------------------
/ Phantom of the Opera - CPU Rev 3 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/-------------------------------------------------------------------------------*/
ROM_START(poto_a32)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("potob5.3-2", 0x4000, 0x4000, CRC(bdc39205) SHA1(67b3f56655ef2cc056912ab6e351cf83352abaa9))
	ROM_LOAD("potoc5.3-2", 0x8000, 0x8000, CRC(e6026455) SHA1(c1441fda6181e9014a8a6f93b7405998a952f508))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("potof7.rom", 0x8000, 0x8000, CRC(2e60b2e3) SHA1(0be89fc9b2c6548392febb35c1ace0eb912fc73f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("potof6.rom", 0x00000, 0x10000, CRC(62b8f74b) SHA1(f82c706b88f49341bab9014bd83371259eb53b47))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("potof5.rom", 0x20000, 0x10000, CRC(5a0537a8) SHA1(26724441d7e2edd7725337b262d95448499151ad))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

/*-----------------------------------------------------------------------------------
/ Playboy 35th Anniversary - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32/64K Sound Roms
/-----------------------------------------------------------------------------------*/
ROM_START(play_a24)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("play2-4.b5", 0x0000, 0x8000, CRC(bc8d7b32) SHA1(3b57dea2feb12315586283548e0bffdc8173b8fb))
	ROM_LOAD("play2-4.c5", 0x8000, 0x8000, CRC(47c30bc2) SHA1(c62e192ec01f4884226e9628baa2cad10cc57bd9))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pbsnd7.dat", 0x8000, 0x8000, CRC(c2cf2cc5) SHA1(1277704b1b38558c341b52da5e06ffa9f07942ad))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("pbsnd6.dat", 0x00000, 0x10000, CRC(c2570631) SHA1(135db5b923689884c73aa5ce48f566db7f1cf831))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("pbsnd5.dat", 0x20000, 0x10000, CRC(0fd30569) SHA1(0bf53fe4b5dffb5e15212c3371f51e98ad14e258))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

/*------------------------------------------------------------------
/ Robocop - CPU Rev 3 /Alpha Type 3 - 32K Roms - 32/64K Sound Roms
/-----------------------------------------------------------------*/
ROM_START(robo_a34)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("robob5.a34", 0x0000, 0x8000, CRC(5a611004) SHA1(08722f8f4386bbc467cfbe8854f0d45c4537bdc6))
	ROM_LOAD("roboc5.a34", 0x8000, 0x8000, CRC(c8705f47) SHA1(a29ad9e4e0269ab19dae77b1e70ff84c8c8d9e85))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("robof7.rom", 0x8000, 0x8000, CRC(fa0891bd) SHA1(332d03c7802989abf717564230993b54819ebc0d))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("robof6.rom", 0x00000, 0x10000, CRC(9246e107) SHA1(e8e72c0d099b17ea9e59ea7794011bad4c072c5e))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("robof4.rom", 0x20000, 0x10000, CRC(27d31df3) SHA1(1611a508ce74eb62a07296d69782ea4fa14503fc))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

/*-------------------------------------------------------------------------
/ Secret Service - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32K/64K Sound Roms
/-------------------------------------------------------------------------*/
ROM_START(ssvc_a26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ssvc2-6.b5", 0x0000, 0x8000, CRC(e5eab8cd) SHA1(63cb678084d4fb2131ba64ed9de1294830057960))
	ROM_LOAD("ssvc2-6.c5", 0x8000, 0x8000, CRC(171b97ae) SHA1(9d678b7b91a5d50ea3cf4f2352094c2355f917b2))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sssndf7.rom", 0x8000, 0x8000, CRC(980778d0) SHA1(7c1f14d327b6d0e6d0fef058f96bb1cb440c9330))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ssv1f6.rom", 0x00000, 0x10000, CRC(ccbc72f8) SHA1(c5c13fb8d05d7fb4005636655073d88b4d12d65e))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("ssv2f4.rom", 0x20000, 0x10000, CRC(53832d16) SHA1(2227eb784e0221f1bf2bdf7ea48ecd122433f1ea))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

/*--------------------------------------------------------------------------
/ Time Machine - CPU Rev 2 /Alpha Type 2 16/32K Roms - 32/64K Sound Roms
/--------------------------------------------------------------------------*/
ROM_START(tmac_a24)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmach2-4.b5", 0x4000, 0x4000, CRC(6ef3cf07) SHA1(3fabfbb2166273bf5bfab06d92fff094d3331d1a))
	ROM_LOAD("tmach2-4.c5", 0x8000, 0x8000, CRC(b61035f5) SHA1(08436b68f37323f50c1fec86aba303a1690af653))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("tmachf4.rom", 0x20000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

ROM_START(tmac_a18)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmach1-8.b5", 0x4000, 0x4000, CRC(5dabdc4c) SHA1(67fe261888ddaa088abe2f8a331eaa5ac34be92e))
	ROM_LOAD("tmach1-8.c5", 0x8000, 0x8000, CRC(5a348def) SHA1(bf2b9a69d516d38e6f87c5886e0ba768c2dc28ab))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("tmachf4.rom", 0x20000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

/*-----------------------------------------------------------------------
/ Torpedo Alley - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32/64K Sound Roms
/------------------------------------------------------------------------*/
ROM_START(torp_e21)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("torpe2-1.b5", 0x0000, 0x8000, CRC(ac0b03e3) SHA1(0ac57b2fec29cdc90ab35cba49844f0cf545d959))
	ROM_LOAD("torpe2-1.c5", 0x8000, 0x8000, CRC(9ad33882) SHA1(c4504d8e136f667652f79b54d4e8d775169c6ac3))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("torpef7.rom", 0x8000, 0x8000, CRC(26f4c33e) SHA1(114f85e93e7b699c4cd6ce1298f95228d439deba))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("torpef6.rom", 0x00000, 0x10000, CRC(b214a7ea) SHA1(d972148395581844e3eaed08f755f3e2217dbbc0))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("torpef4.rom", 0x20000, 0x10000, CRC(83a4e7f3) SHA1(96deac9251fe68cc0319ac009becd424c4e444c5))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

GAME(1989,	mnfb_c27,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"Monday Night Football (2.7, 50cts)",		GAME_IS_SKELETON_MECHANICAL)
GAME(1990,	poto_a32,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"The Phantom of the Opera (3.2)",			GAME_IS_SKELETON_MECHANICAL)
GAME(1989,	play_a24,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"Playboy 35th Anniversary (2.4)",			GAME_IS_SKELETON_MECHANICAL)
GAME(1989,	robo_a34,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"Robocop (3.4)",							GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	ssvc_a26,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"Secret Service (2.6)",						GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	tmac_a24,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"Time Machine (2.4)",						GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	tmac_a18,		tmac_a24,	de_2,	de_2,	de_2,	ROT0,	"Data East",		"Time Machine (1.8)",						GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	torp_e21,		0,			de_2,	de_2,	de_2,	ROT0,	"Data East",		"Torpedo Alley (2.1, Europe)",				GAME_IS_SKELETON_MECHANICAL)
