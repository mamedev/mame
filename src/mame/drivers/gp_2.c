/*
    Game Plan MPU-2
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class gp_2_state : public driver_device
{
public:
	gp_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( gp_2_map, AS_PROGRAM, 8, gp_2_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( gp_2 )
INPUT_PORTS_END

void gp_2_state::machine_reset()
{
}

static DRIVER_INIT( gp_2 )
{
}

static MACHINE_CONFIG_START( gp_2, gp_2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2457600)
	MCFG_CPU_PROGRAM_MAP(gp_2_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Agents 777 (November 1984) - Model #770
/-------------------------------------------------------------------*/
ROM_START(agent777)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "770a", 0x0000, 0x0800, CRC(fc4eebcd) SHA1(742a201e89c1357d2a1f24b0acf3b78ffec96c74))
	ROM_LOAD( "770b", 0x0800, 0x0800, CRC(ea62aece) SHA1(32be10bc76a59e03c3fd3294daefc8d28c20386a))
	ROM_LOAD( "770c", 0x1000, 0x0800, CRC(59280db7) SHA1(8f199be7bfbc01466541c07dc4c365e20055a66c))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("770snd", 0x3800, 0x0800, CRC(e4e66c9f) SHA1(f373facefb18c64377da47308a8bbd5fc80e9c2d))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Andromeda (August 1985) - Model #850
/-------------------------------------------------------------------*/
ROM_START(andromep)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "850.a", 0x0000, 0x1000, CRC(67ed03ee) SHA1(efe7c495766ffb73545a77ab24f02925ac0395f1))
	ROM_LOAD( "850.b", 0x1000, 0x1000, CRC(37c244e8) SHA1(5cef0a1a6f2c34f2d01bdd12ce11da40c8be4296))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("850.snd", 0x3800, 0x0800, CRC(18e084a6) SHA1(56efbabe60305f168ca479295577bff7f3a4dace))
	ROM_RELOAD(0x7800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

ROM_START(andromepa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "850.a", 0x0000, 0x1000, CRC(67ed03ee) SHA1(efe7c495766ffb73545a77ab24f02925ac0395f1))
	ROM_LOAD( "850b.rom", 0x1000, 0x1000, CRC(fc1829a5) SHA1(9761543d17c0a5c08b0fec45c35648ce769a3463))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("850.snd", 0x3800, 0x0800, CRC(18e084a6) SHA1(56efbabe60305f168ca479295577bff7f3a4dace))
	ROM_RELOAD(0x7800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Attila the Hun (April 1984) - Model #260
/-------------------------------------------------------------------*/
ROM_START(attila)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "260.a", 0x0000, 0x0800, CRC(b31c11d8) SHA1(d3f2ad84cc28e99acb54349b232dbf8abdf15b21))
	ROM_LOAD( "260.b", 0x0800, 0x0800, CRC(e8cca86d) SHA1(ed0797175a573537be2d5119ad68b1847e49e578))
	ROM_LOAD( "260.c", 0x1000, 0x0800, CRC(206605c3) SHA1(14f61a2f43c29370bcb6db29969e8dfcfe3da1ab))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("260.snd", 0x3800, 0x0800, CRC(21e6b188) SHA1(84148942e6007d49bb4085ec3678954d48e4439e))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Captain Hook (April 1985) - Model #780
/-------------------------------------------------------------------*/
ROM_START(cpthook)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "780.a", 0x0000, 0x0800, CRC(6bd5a495) SHA1(8462e0c68176daee6b23dce9091f5aee99e62631))
	ROM_LOAD( "780.b", 0x0800, 0x0800, CRC(3d1c5555) SHA1(ecb0d40f5e6e37acfc8589816e24b26525273393))
	ROM_LOAD( "780.c", 0x1000, 0x0800, CRC(e54bc51f) SHA1(3480e0cdd43f9ac3fda8cd466b2f039210525e8b))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("780.snd", 0x3800, 0x0800, CRC(95af3392) SHA1(73a2b583b7fc423c2e4390667aebc90ad41f4f93))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Cyclopes (November 1985) - Model #800
/-------------------------------------------------------------------*/
ROM_START(cyclopes)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "800.a", 0x0000, 0x1000, CRC(3e9628e5) SHA1(4dad9e082a9f4140162bc155f2b0f0a948ba012f))
	ROM_LOAD( "800.b", 0x1000, 0x1000, CRC(3f945c46) SHA1(25eb543e0b0edcd0a0dcf8e4aa1405cda55ebe2e))
	ROM_LOAD( "800.c", 0x2000, 0x1000, CRC(7ea18e65) SHA1(e86d82e3ba659499dfbf14920b196252784724f7))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("800.snd", 0x3800, 0x0800, CRC(290db3d2) SHA1(a236594f7a89969981bd5707d6dfbb5120fb8f46))
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Global Warfare (June 1981)  - Model #240
/-------------------------------------------------------------------*/
ROM_START(gwarfare)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "240a.716", 0x0000, 0x0800, CRC(30206428) SHA1(7a9029e4fd4c4c00da3256ed06464c0bd8022168))
	ROM_LOAD( "240b.716", 0x0800, 0x0800, CRC(a54eb15d) SHA1(b9235bd188c1251eb213789800b7686b5e3c557f))
	ROM_LOAD( "240c.716", 0x1000, 0x0800, CRC(60d115a8) SHA1(e970fdd7cbbb2c81ab8c8209edfb681798c683b9))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("gw240bot.rom", 0x3800, 0x0800, CRC(3245a206) SHA1(b321b2d276fbd74199eff2d8c0d1b8a2f5c93604))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("gw240top.rom",0x3000, 0x0800, CRC(faaf3de1) SHA1(9c984d1ac696eb16f7bf35463a69a470344314a7))
ROM_END

/*-------------------------------------------------------------------
/ Lady Sharpshooter (May 1985) - Cocktail Model #830
/-------------------------------------------------------------------*/
ROM_START(ladyshot)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "830a.716", 0x0000, 0x0800, CRC(c055b993) SHA1(a9a7156e5ec0a32db1ffe36b3c6280953a2606ff))
	ROM_LOAD( "830b.716", 0x0800, 0x0800, CRC(1e3308ea) SHA1(a5955a6a15b33c4cf35105ab524a8e7e03d748b6))
	ROM_LOAD( "830c.716", 0x1000, 0x0800, CRC(f5e1db15) SHA1(e8168ab37ba30211045fc96b23dad5f06592b38d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("830.snd", 0x3800, 0x0800, NO_DUMP)
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END
ROM_START(ladyshota)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "830a2.716", 0x0000, 0x0800, CRC(2c1f1629) SHA1(9233ce4328d779ff6548cdd5d6819cd368bef313))
	ROM_LOAD( "830b2.716", 0x0800, 0x0800, CRC(2105a538) SHA1(0360d3e740d8b6f816cfe7fe1fb32ac476251b9f))
	ROM_LOAD( "830c2.716", 0x1000, 0x0800, CRC(2d96bdde) SHA1(7c03a29a91f03fba9ed5e53a93335113a7cbafb3))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD ("830.snd", 0x3800, 0x0800, NO_DUMP)
	ROM_CONTINUE(0x7800, 0x0800)
	ROM_RELOAD (0xf000, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Loch Ness Monster (November 1985) - Model #???
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Mike Bossy (January 1982) - Model #???
/-------------------------------------------------------------------*/
ROM_START(mbossy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "mb_a.716", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "mb_b.716", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "mb_c.716", 0x1000, 0x0800, NO_DUMP)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mb.u9", 0x3800, 0x0800, CRC(dfa98db5) SHA1(65361630f530383e67837c428050bcdb15373c0b))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("mb.u10",0x3000, 0x0800, CRC(2d3c91f9) SHA1(7e1f067af29d9e484da234382d7dc821ca07b6c4))
ROM_END

/*-------------------------------------------------------------------
/ Old Coney Island! (December 1979) - Model #180
/-------------------------------------------------------------------*/
ROM_START(coneyis)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "130c.716", 0x1000, 0x0800, CRC(b956f67b) SHA1(ff64383d7f59e9bbec588553e35a21fb94c7203b))
ROM_END

/*-------------------------------------------------------------------
/ Pinball Lizard (June / July 1980) - Model #210
/-------------------------------------------------------------------*/
ROM_START(lizard)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "130c.716", 0x1000, 0x0800, CRC(b956f67b) SHA1(ff64383d7f59e9bbec588553e35a21fb94c7203b))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("lizard.u9", 0x3800, 0x0800, CRC(2d121b24) SHA1(55c16951538229571165c35a353da53e22d11f81))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("lizard.u10",0x3000, 0x0800, CRC(28b8f1f0) SHA1(db6d816366e0bca59376f6f8bf87e6a2d849aa72))
ROM_END

/*-------------------------------------------------------------------
/ Sharp Shooter II (November 1983) - Model #730
/-------------------------------------------------------------------*/
ROM_START(sshootr2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "730c", 0x1000, 0x0800, CRC(d1af712b) SHA1(9dce2ec1c2d9630a29dd21f4685c09019e59b147))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("730u9.snd", 0x3800, 0x0800, CRC(dfa98db5) SHA1(65361630f530383e67837c428050bcdb15373c0b))
	ROM_RELOAD(0xf800, 0x0800)
	ROM_LOAD("730u10.snd",0x3000, 0x0800, CRC(6d3dcf44) SHA1(3703313d4172ebfec1dcacca949076541ee35cb7))
ROM_END

/*-------------------------------------------------------------------
/ Sharpshooter (May 1979) - Model #130
/-------------------------------------------------------------------*/
ROM_START(sshootep)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "130b.716", 0x0800, 0x0800, CRC(19a86f5e) SHA1(bc4a87314fc9c4e74e492c3f6e44d5d6cae72939))
	ROM_LOAD( "130c.716", 0x1000, 0x0800, CRC(b956f67b) SHA1(ff64383d7f59e9bbec588553e35a21fb94c7203b))
ROM_END

/*-------------------------------------------------------------------
/ Super Nova (May 1982) - Model #150
/-------------------------------------------------------------------*/
ROM_START(suprnova)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "130a.716", 0x0000, 0x0800, CRC(dc402b37) SHA1(90c46391a1e5f000f3b235d580463bf96b45bd3e))
	ROM_LOAD( "150b.716", 0x0800, 0x0800, CRC(8980a8bb) SHA1(129816fe85681b760307a713c667737a750b0c04))
	ROM_LOAD( "150c.716", 0x1000, 0x0800, CRC(6fe08f96) SHA1(1309619a2400674fa1d05dc9214fdb85419fd1c3))
ROM_END

/*-------------------------------------------------------------------
/ Vegas (August 1979) - Cocktail Model #140
/-------------------------------------------------------------------*/
ROM_START(vegasgp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "140a.12", 0x0000, 0x0800, CRC(2c00bc19) SHA1(521d4b44f46dea0a08e90cd3aea5799462215863))
	ROM_LOAD( "140b.13", 0x0800, 0x0800, CRC(cf26d67b) SHA1(05481e880e23a7bc1d1716b52ac1effc0db437f2))
ROM_END


GAME(1984,	agent777,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Agents 777",				GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	andromep,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Andromeda",				GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	andromepa,	andromep,	gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Andromeda (alternate set)",GAME_IS_SKELETON_MECHANICAL)
GAME(1984,	attila,		0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Attila The Hun",			GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	cpthook,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Captain Hook",				GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	cyclopes,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Cyclopes",					GAME_IS_SKELETON_MECHANICAL)
GAME(1981,	gwarfare,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Global Warfare",			GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	ladyshot,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Lady Sharpshooter",		GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	ladyshota,	ladyshot,	gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Lady Sharpshooter (alternate set)",		GAME_IS_SKELETON_MECHANICAL)
GAME(1982,	mbossy,		0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Mike Bossy",				GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	coneyis,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Old Coney Island!",		GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	lizard,		0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Pinball Lizard",			GAME_IS_SKELETON_MECHANICAL)
GAME(1983,	sshootr2,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Sharp Shooter II",			GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	sshootep,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Sharpshooter",				GAME_IS_SKELETON_MECHANICAL)
GAME(1982,	suprnova,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Super Nova",				GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	vegasgp,	0,			gp_2,	gp_2,	gp_2,	ROT0,	"Game Plan",	"Vegas (Game Plan)",		GAME_IS_SKELETON_MECHANICAL)
