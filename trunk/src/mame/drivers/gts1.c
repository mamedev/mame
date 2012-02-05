/*
    Gottlieb System 1
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/pps4/pps4.h"

class gts1_state : public driver_device
{
public:
	gts1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( gts1_map, AS_PROGRAM, 8, gts1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( gts1 )
INPUT_PORTS_END

void gts1_state::machine_reset()
{
}

static DRIVER_INIT( gts1 )
{
}

static MACHINE_CONFIG_START( gts1, gts1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPS4, 198864)
	MCFG_CPU_PROGRAM_MAP(gts1_map)
MACHINE_CONFIG_END


ROM_START( gts1 )
    ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
ROM_END

ROM_START( gts1s )
    ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
ROM_END

/*-------------------------------------------------------------------
/ Asteroid Annie and the Aliens (12/1980)
/-------------------------------------------------------------------*/
ROM_START(astannie)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("442.cpu", 0x2000, 0x0400, CRC(579521e0) SHA1(b1b19473e1ca3373955ee96104b87f586c4c311c))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("442.snd", 0x0400, 0x0400, CRC(c70195b4) SHA1(ff06197f07111d6a4b8942dcfe8d2279bda6f281))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Buck Rogers (01/1980)
/-------------------------------------------------------------------*/
ROM_START(buckrgrs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("437.cpu", 0x2000, 0x0400, CRC(e57d9278) SHA1(dfc4ebff1e14b9a074468671a8e5ac7948d5b352))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("437.snd", 0x0400, 0x0400, CRC(732b5a27) SHA1(7860ea54e75152246c3ac3205122d750b243b40c))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Charlie's Angels (11/1978)
/-------------------------------------------------------------------*/
ROM_START(charlies)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("425.cpu", 0x2000, 0x0400, CRC(928b4279) SHA1(51096d45e880d6a8263eaeaa0cdab0f61ad2f58d))
ROM_END
/*-------------------------------------------------------------------
/ Cleopatra (11/1977)
/-------------------------------------------------------------------*/
ROM_START(cleoptra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("409.cpu", 0x2000, 0x0400, CRC(8063ff71) SHA1(205f09f067bf79544d2ce2a48d23259901f935dd))
ROM_END

/*-------------------------------------------------------------------
/ Close Encounters of the Third Kind (10/1978)
/-------------------------------------------------------------------*/
ROM_START(closeenc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("424.cpu", 0x2000, 0x0400, CRC(a7a5dd13) SHA1(223c67b9484baa719c91de52b363ff22813db160))
ROM_END

/*-------------------------------------------------------------------
/ Count-Down (05/1979)
/-------------------------------------------------------------------*/
ROM_START(countdwn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("422.cpu", 0x2000, 0x0400, CRC(51bc2df0) SHA1(d4b555d106c6b4e420b0fcd1df8871f869476c22))
ROM_END

/*-------------------------------------------------------------------
/ Dragon (10/1978)
/-------------------------------------------------------------------*/
ROM_START(dragon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("419.cpu", 0x2000, 0x0400, CRC(018d9b3a) SHA1(da37ef5017c71bc41bdb1f30d3fd7ac3b7e1ee7e))
ROM_END

/*-------------------------------------------------------------------
/ Genie (11/1979)
/-------------------------------------------------------------------*/
ROM_START(geniep)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("435.cpu", 0x2000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("435.snd", 0x0400, 0x0400, CRC(4a98ceed) SHA1(f1d7548e03107033c39953ee04b043b5301dbb47))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Joker Poker (08/1978)
/-------------------------------------------------------------------*/
ROM_START(jokrpokr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("417.cpu", 0x2000, 0x0400, CRC(33dade08) SHA1(23b8dbd7b6c84b806fc0d2da95478235cbf9f80a))
ROM_END

/*-------------------------------------------------------------------
/ Jungle Queen (1985)
/-------------------------------------------------------------------*/
/*-------------------------------------------------------------------
/ L'Hexagone (04/1986)
/-------------------------------------------------------------------*/
ROM_START(hexagone)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("435.cpu", 0x2000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hexagone.bin", 0, 0x4000, CRC(002b5464) SHA1(e2d971c4e85b4fb6580c2d3945c9946ea0cebc2e))
ROM_END
/*-------------------------------------------------------------------
/ Movie
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Pinball Pool (08/1979)
/-------------------------------------------------------------------*/
ROM_START(pinpool)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("427.cpu", 0x2000, 0x0400, CRC(c496393d) SHA1(e91d9596aacdb4277fa200a3f8f9da099c278f32))
ROM_END

/*-------------------------------------------------------------------
/ Roller Disco (02/1980)
/-------------------------------------------------------------------*/
ROM_START(roldisco)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("440.cpu", 0x2000, 0x0400, CRC(bc50631f) SHA1(6aa3124d09fc4e369d087a5ad6dd1737ace55e41))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("440.snd", 0x0400, 0x0400, CRC(4a0a05ae) SHA1(88f21b5638494d8e78dc0b6b7d69873b76b5f75d))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Sahara Love (1984)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Sinbad (05/1978)
/-------------------------------------------------------------------*/
ROM_START(sinbad)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("412.cpu", 0x2000, 0x0400, CRC(84a86b83) SHA1(f331f2ffd7d1b279b4ffbb939aa8649e723f5fac))
ROM_END

ROM_START(sinbadn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("412no1.cpu", 0x2000, 0x0400, CRC(f5373f5f) SHA1(027840501416ff01b2adf07188c7d667adf3ad5f))
ROM_END

/*-------------------------------------------------------------------
/ Sky Warrior (1983)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Solar Ride (02/1979)
/-------------------------------------------------------------------*/
ROM_START(solaride)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("421.cpu", 0x2000, 0x0400, CRC(6b5c5da6) SHA1(a09b7009473be53586f53f48b7bfed9a0c5ecd55))
ROM_END

/*-------------------------------------------------------------------
/ The Incredible Hulk (10/1979)
/-------------------------------------------------------------------*/
ROM_START(hulk)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("433.cpu", 0x2000, 0x0400, CRC(c05d2b52) SHA1(393fe063b029246317c90ee384db95a84d61dbb7))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("433.snd", 0x0400, 0x0400, CRC(20cd1dff) SHA1(93e7c47ff7051c3c0dc9f8f95aa33ba094e7cf25))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Torch (02/1980)
/-------------------------------------------------------------------*/
ROM_START(torch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("438.cpu", 0x2000, 0x0400, CRC(2d396a64) SHA1(38a1862771500faa471071db08dfbadc6e8759e8))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("438.snd", 0x0400, 0x0400, CRC(a9619b48) SHA1(1906bc1b059bf31082e3b4546f5a30159479ad3c))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Totem (10/1979)
/-------------------------------------------------------------------*/
ROM_START(totem)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("429.cpu", 0x2000, 0x0400, CRC(7885a384) SHA1(1770662af7d48ad8297097a9877c5c497119978d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("429.snd", 0x0400, 0x0400, CRC(5d1b7ed4) SHA1(4a584f880e907fb21da78f3b3a0617f20599688f))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ System 1 Test prom
/-------------------------------------------------------------------*/
ROM_START(sys1test)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("test.cpu", 0x2000, 0x0400, CRC(8b0704bb) SHA1(5f0eb8d5af867b815b6012c9d078927398efe6d8))
ROM_END


GAME(1977,	gts1,		0,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"System 1",								GAME_IS_BIOS_ROOT)

//Exact same roms as gts1 with added hardware we'll likely need roms for to emulate properly
GAME(1979,	gts1s,		gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"System 1 with sound board",						GAME_IS_BIOS_ROOT)

GAME(1980,	astannie,	gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Asteroid Annie and the Aliens",		GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	buckrgrs,	gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Buck Rogers",							GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	charlies,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Charlie's Angels",						GAME_IS_SKELETON_MECHANICAL)
GAME(1977,	cleoptra,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Cleopatra",							GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	closeenc,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Close Encounters of the Third Kind",	GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	countdwn,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Count-Down",							GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	dragon,		gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Dragon",								GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	geniep,		gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Genie (Pinball)",								GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	jokrpokr,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Joker Poker",							GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	pinpool,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Pinball Pool",							GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	roldisco,	gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Roller Disco",							GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	sinbad,		gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Sinbad",								GAME_IS_SKELETON_MECHANICAL)
GAME(1978,	sinbadn,	sinbad,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Sinbad (Norway)",						GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	solaride,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Solar Ride",							GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	hulk,		gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Incredible Hulk,The",					GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	torch,		gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Torch",								GAME_IS_SKELETON_MECHANICAL)
GAME(1979,	totem,		gts1s,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"Totem",								GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	hexagone,	gts1s,		gts1,	gts1,	gts1,	ROT0,	"Christian Tabart (France)",		"L'Hexagone",		GAME_IS_SKELETON_MECHANICAL)
GAME(19??,	sys1test,	gts1,		gts1,	gts1,	gts1,	ROT0,	"Gottlieb",		"System 1 Test prom",					GAME_IS_SKELETON_MECHANICAL)
