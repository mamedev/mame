/*
    Gottlieb System 3
*/


#include "emu.h"
#include "cpu/m6502/m65c02.h"

class gts3_state : public driver_device
{
public:
	gts3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(gts3);
};


static ADDRESS_MAP_START( gts3_map, AS_PROGRAM, 8, gts3_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( gts3 )
INPUT_PORTS_END

void gts3_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(gts3_state,gts3)
{
}

static MACHINE_CONFIG_START( gts3, gts3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02, 2000000)
	MCFG_CPU_PROGRAM_MAP(gts3_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Amazon Hunt III
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Bell Ringer (N103)
/-------------------------------------------------------------------*/
ROM_START(bellring)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("br_gprom.bin", 0x0000, 0x10000, CRC(a9a59b36) SHA1(ca6d0e54a5c85ef72485975c632660831a3b8c82))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("br_drom1.bin", 0x8000, 0x8000, CRC(99f38229) SHA1(f63d743e63e88728e8d53320b21b2fda1b6385f8))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("br_yrom1.bin", 0x8000, 0x8000, CRC(d5aab379) SHA1(b3995f8aa2e54f91f2a0fd010c807fbfbf9ae847))
ROM_END

/*-------------------------------------------------------------------
/ Cactus Jack's (#729)
/-------------------------------------------------------------------*/
ROM_START(cactjack)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(5661ab06) SHA1(12b7066110feab0aef36ff7bdc74690fc8da4ed3))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(78c099e1) SHA1(953111237fdc3e20562d823eb2b6430e5a4afe4d))

	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(c890475f) SHA1(1cf6ed0dbd003a76a5cf889f62b489c0a62e9d25))
	ROM_RELOAD(0x00000+0x40000, 0x20000)
	ROM_RELOAD(0x00000+0x80000, 0x20000)
	ROM_RELOAD(0x00000+0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(aba8fd98) SHA1(81b8af4d2d8e40b5b44f114c095371afe5539549))
	ROM_RELOAD(0x20000+0x40000, 0x20000)
	ROM_RELOAD(0x20000+0x80000, 0x20000)
	ROM_RELOAD(0x20000+0xc0000, 0x20000)

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4554ed0d) SHA1(df0a9225f961e0ee876c3e63ad54c6e4eac080ae))
ROM_END

/*-------------------------------------------------------------------
/ Car Hop (#725)
/-------------------------------------------------------------------*/
ROM_START(carhop)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(164b2c9c) SHA1(49cf7e3a3acb5de8dbfd2ad22f8bd9a352ff2899))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9dec74e7) SHA1(8234bdca5536d30dc1eabcb3a5505d2fd824ce0f))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(831ee812) SHA1(57056cde36b17cb7d7f34275b1bb5dc3d52bde4e))
ROM_END

/*-------------------------------------------------------------------
/ Caribbean Cruise (#C102)
/-------------------------------------------------------------------*/
ROM_START(ccruise)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(668b5757) SHA1(8ff955e8598ffdc68eab7fd69c6a67c4eed13f0f))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000,  0x8000, CRC(4480257e) SHA1(50b93d4496816ef7cdf007ac75c72c6aaa956aba))

	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(f8cec60c) SHA1(e52f3a5890a3bb5eb6c932c3d0ed471ed76909c9))
	ROM_RELOAD(0x40000,  0x40000)

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6e424e53) SHA1(90a9bf5ce84680972f9d12eb386215494c584b9b))
ROM_END

/*-------------------------------------------------------------------
/ Class of 1812 (#730)
/-------------------------------------------------------------------*/
ROM_START(clas1812)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(564349bf) SHA1(458eb2ece924a20d309dce7117c94e75b4a21fd7))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3863a9df) SHA1(1759abbfcb127a6909f70845f41daf3ac8e80cef))

	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(357b0069) SHA1(870b0b84c6b3754f89b4e4e0b4594613ef589204))
	ROM_RELOAD(0x00000+0x40000, 0x20000)
	ROM_RELOAD(0x00000+0x80000, 0x20000)
	ROM_RELOAD(0x00000+0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(5be02ff7) SHA1(51af73a26bbed0915ec57cde8f9cac552978b2dc))
	ROM_RELOAD(0x20000+0x40000, 0x20000)
	ROM_RELOAD(0x20000+0x80000, 0x20000)
	ROM_RELOAD(0x20000+0xc0000, 0x20000)

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ecf6ecb) SHA1(92469ccdedcc8e61edcddaedd688ef990a9ad5ad))
ROM_END

/*-------------------------------------------------------------------
/ Deadly Weapon (#724)
/-------------------------------------------------------------------*/
ROM_START(deadweap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(07d84b32) SHA1(25d8772a5c8655b3406df94563076719b07129cd))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f55dd7ec) SHA1(fe306c40bf3d98e4076d0d8a935c3671469d4cff))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(93369ed3) SHA1(3340478ffc00cf9991beabd4f0ecd89d0c88965e))
ROM_END

/*-------------------------------------------------------------------
/ Hoops (#727)
/-------------------------------------------------------------------*/
ROM_START(hoops)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(78391273) SHA1(dbf91597ce2910e526fb5e82355ad862706b4975))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(e72c00eb) SHA1(5b9f85083b38d916afb0f9b72b061501504725ff))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9718b958) SHA1(bac806267bab4852c0f3fdb48f8d872992f61ace))
ROM_END

/*-------------------------------------------------------------------
/ Lights, Camera, Action (#720)
/-------------------------------------------------------------------*/
ROM_START(lca)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x8000, 0x8000, CRC(52957d70) SHA1(0c24d824b1aa966eb3af3db3ff02870ba463dcd6))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(a258d72d) SHA1(eeb4768c8b2f57509a088d3ac8d35aa34f2cfc2c))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20919ebb) SHA1(a2ea79863b41a04aa23ea596932825408cca64e3))
ROM_END

ROM_START(lca2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x8000, 0x8000, CRC(937a8426) SHA1(6bc2d1b0c3dc273577376654ba72b60febe32529))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(a258d72d) SHA1(eeb4768c8b2f57509a088d3ac8d35aa34f2cfc2c))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20919ebb) SHA1(a2ea79863b41a04aa23ea596932825408cca64e3))
ROM_END

/*-------------------------------------------------------------------
/ Nudge-It (N102)
/-------------------------------------------------------------------*/
ROM_START(nudgeit)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(3d9e0309) SHA1(caaa28482e7f260668aa05b39b551acb8e4cc41a))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ae0c4b1d) SHA1(c8aa409c9b54fd8ecf70eb2926f4e98fc5eb11fe))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(65fc2e60) SHA1(6377c220753d9e4b5c76d445056409526d95772f))
ROM_END

/*-------------------------------------------------------------------
/ Operation: Thunder (#732)
/-------------------------------------------------------------------*/
ROM_START(opthund)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(96a128c2) SHA1(4032c5191b167a0498371207666a1f73155b7a74))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(db28be69) SHA1(6c505c34c8bdccc43dd8f310f01dd3a6b49e8059))

	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(0fbb130a) SHA1(a171c20f861dac5918c5b410e2a2bdd6e7c0553b))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0f7632b3) SHA1(a122a062448139d5c1a9daa7d827c3073aa194f7))
	ROM_RELOAD(0x80000+0x40000, 0x40000)

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(169816d1) SHA1(d23b1d8d1b841ca065a485e80805ecc6342ce57b))
ROM_END

/*-------------------------------------------------------------------
/ Silver Slugger (#722)
/-------------------------------------------------------------------*/
ROM_START(silvslug)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(a6c524e2) SHA1(dc12dd8e814a37aada021f84c58475efe72cb846))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(eac3e1cc) SHA1(2725457231854e4f3d54fbba745b8fc6f55b1688))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20bc9797) SHA1(5d17b5f0423d8854fb7c4816d53a223ecc7c50c6))
ROM_END

/*-------------------------------------------------------------------
/ Surf'n Safari (#731)
/-------------------------------------------------------------------*/
ROM_START(surfnsaf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(ac3393bd) SHA1(f9c533b937b5ca5698b805ed6ed573cb22383d9d))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ec8fc963) SHA1(247e76d87beb3339e7d55292f9eadd2351621cfa))

	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(38b569b2) SHA1(93be47916a92541d097233b60a42eb7ca587ce52))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(224c2021) SHA1(6b426097a2870b3b32d786be6e66ba6be9f54c29))
	ROM_RELOAD(0x80000+0x40000, 0x40000)

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(a0480418) SHA1(a982564d5dbf52275c2e7223687b07cf4ca0a115))
ROM_END

/*-------------------------------------------------------------------
/ Title Fight (#726)
/-------------------------------------------------------------------*/
ROM_START(tfight)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(43b3193a) SHA1(bd185fe67c147a6acca8e78da4b77c384124fc46))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9514739f) SHA1(2794549f549d68e064a9a962a4e91fff7dcf0160))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8591d421) SHA1(74402cf8b419e0cb05069851b0d5616e66b2f0a9))
ROM_END

/*-------------------------------------------------------------------
/ Vegas (#723)
/-------------------------------------------------------------------*/
ROM_START(vegas)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(48189981) SHA1(95144af4b222158becd4d5748d15b7b6c6021bd2))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(46eb5755) SHA1(94ec2d0cf41f68a8c3d7505186b11b4abb4803db))

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(af1095f1) SHA1(06609085cd74b969e4f2ec962c427c5c42ebc6ff))
ROM_END


/************************************/
/* NOT OFFICIALLY LISTED             */
/************************************/

/*-------------------------------------------------------------------
/ Unnamed game? by Toptronic HGmbH, Germany
/-------------------------------------------------------------------*/
ROM_START(tt_game)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(e7944b75) SHA1(b73f2e0004556c8aa88baef0cddcdefb5b905b8d))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, NO_DUMP)

	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(b0983d90) SHA1(72e6a71f20fd5849543ca13813f062a3fc1d7dcf))
	ROM_RELOAD(0x00000+0x40000, 0x20000)
	ROM_RELOAD(0x00000+0x80000, 0x20000)
	ROM_RELOAD(0x00000+0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(3e31ce58) SHA1(a2ef72d7b2bb821d1f62dce7212e31a1df3e7791))
	ROM_RELOAD(0x20000+0x40000, 0x20000)
	ROM_RELOAD(0x20000+0x80000, 0x20000)
	ROM_RELOAD(0x20000+0xc0000, 0x20000)

	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, NO_DUMP)
ROM_END

GAME(1989,  lca,        0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Lights...Camera...Action!",                GAME_IS_SKELETON_MECHANICAL)
GAME(1989,  lca2,       lca,        gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Lights...Camera...Action! (rev.2)",                GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  silvslug,   0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Silver Slugger",               GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  vegas,      0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Vegas",                GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  deadweap,   0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Deadly Weapon",                GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  tfight,     0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Title Fight",              GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  nudgeit,    0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Nudge-It",             GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  bellring,   0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Bell Ringer",              GAME_IS_SKELETON_MECHANICAL)
GAME(1991,  carhop,     0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Car Hop",              GAME_IS_SKELETON_MECHANICAL)
GAME(1991,  hoops,      0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Hoops",                GAME_IS_SKELETON_MECHANICAL)
GAME(1991,  cactjack,   0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Cactus Jack's",                GAME_IS_SKELETON_MECHANICAL)
GAME(1991,  clas1812,   0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Class of 1812",                GAME_IS_SKELETON_MECHANICAL)
GAME(1991,  surfnsaf,   0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Surf'n Safari",                GAME_IS_SKELETON_MECHANICAL)
GAME(1992,  opthund,    0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Gottlieb",             "Operation: Thunder",               GAME_IS_SKELETON_MECHANICAL)
GAME(19??,  tt_game,    0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "Toptronic",            "unknown Toptronic pinball game",               GAME_IS_SKELETON_MECHANICAL)
GAME(1989,  ccruise,    0,          gts3,   gts3, gts3_state,   gts3,   ROT0,   "International Concepts","Caribbean Cruise",                GAME_IS_SKELETON_MECHANICAL)
