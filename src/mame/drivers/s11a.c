/*
    Williams System 11a
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s11a_state : public driver_device
{
public:
	williams_s11a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( williams_s11a_map, AS_PROGRAM, 8, williams_s11a_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s11a )
INPUT_PORTS_END

void williams_s11a_state::machine_reset()
{
}

static DRIVER_INIT( williams_s11a )
{
}

static MACHINE_CONFIG_START( williams_s11a, williams_s11a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(williams_s11a_map)
MACHINE_CONFIG_END

/*--------------------
/ F14 Tomcat 5/87
/--------------------*/

ROM_START(f14_p3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_l3.u26", 0x4000, 0x4000, CRC(cd607556) SHA1(2ec95085784370a071cbf5df7ae5c6b4749605e2))
	ROM_LOAD("f14_l3.u27", 0x8000, 0x8000, CRC(72951fd1) SHA1(b5f3fe1859e0abf9ab558b4b4f6754134d528c23))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("f14_u21.l1", 0x0000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x8000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("f14_u19.l1", 0x10000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

ROM_START(f14_p4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26_l4.128", 0x4000, 0x4000, CRC(7b39706a) SHA1(0dc0b1a1dfd12bc73e6fd8b825fe72ddc8fc1497))
	ROM_LOAD("u27_l4.256", 0x8000, 0x8000, CRC(189f9488) SHA1(7536d56cb83bf29f8d8b03b226a5f60200776095))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("f14_u21.l1", 0x0000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x8000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("f14_u19.l1", 0x10000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

ROM_START(f14_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_u26.l1", 0x4000, 0x4000, CRC(62c2e615) SHA1(456ce0d1f74fa5e619c272880ba8ac6819848ddc))
	ROM_LOAD("f14_u27.l1", 0x8000, 0x8000, CRC(da1740f7) SHA1(1395a4f3891a043cfedc5426ec88af35eab8d4ea))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("f14_u21.l1", 0x0000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x8000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("f14_u19.l1", 0x10000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

/*--------------------
/ Fire! 8/87
/--------------------*/
ROM_START(fire_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fire_u26.l3", 0x4000, 0x4000, CRC(48abae33) SHA1(00ce24316aa007eec090ae74818003e11a141214))
	ROM_LOAD("fire_u27.l3", 0x8000, 0x8000, CRC(4ebf4888) SHA1(45dc0231404ed70be2ab5d599a673aac6271550e))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("fire_u21.l2", 0x0000, 0x8000, CRC(2edde0a4) SHA1(de292a340a3a06b0b996fc69fee73eb7bbfbbe64))
	ROM_LOAD("fire_u22.l2", 0x8000, 0x8000, CRC(16145c97) SHA1(523e99df3907a2c843c6e27df4d16799c4136a46))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("fire_u4.l1", 0x00000, 0x8000, CRC(0e058918) SHA1(4d6bf2290141119174787f8dd653c47ea4c73693))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

/*--------------------
/ Fire! Champagne Edition 9/87
/--------------------*/

/*--------------------
/ Millionaire 1/87
/--------------------*/
ROM_START(milln_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mill_u26.l3", 0x4000, 0x4000, CRC(07bc9fff) SHA1(b16082fb51df3e4d2fb786cb8894b1c232521ef3))
	ROM_LOAD("mill_u27.l3", 0x8000, 0x8000, CRC(ba789c43) SHA1(c066a304882bea4cba1e215642416fcb22585aa4))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("mill_u21.l1", 0x0000, 0x8000, CRC(4cd1ee90) SHA1(4e24b96138ced16eff9036303ca6347e3423dbfc))
	ROM_LOAD("mill_u22.l1", 0x8000, 0x8000, CRC(73735cfc) SHA1(f74c873a20990263e0d6b35609fc51c08c9f8e31))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("mill_u4.l1", 0x00000, 0x8000, CRC(cf766506) SHA1(a6e4df19a513102abbce2653d4f72245f54407b1))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("mill_u19.l1", 0x10000, 0x8000, CRC(e073245a) SHA1(cbaddde6bb19292ace574a8329e18c97c2ee9763))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

/*--------------------
/ Pinbot 10/86
/--------------------*/
ROM_START(pb_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pbot_u26.l5", 0x4000, 0x4000, CRC(daa0c8e4) SHA1(47289b350eb0d84aa0d37e53383e18625451bbe8))
	ROM_LOAD("pbot_u27.l5", 0x8000, 0x8000, CRC(e625d6ce) SHA1(1858dc2183954342b8e2e5eb9a14edcaa8dad5ae))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("pbot_u21.l1", 0x0000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x8000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("pbot_u19.l1", 0x10000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

ROM_START(pb_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l2.rom", 0x8000, 0x8000, CRC(0a334fc5) SHA1(d08afe6ddc141e37f97ea588d184a316ff7f6db7))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("pbot_u21.l1", 0x0000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x8000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("pbot_u19.l1", 0x10000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

ROM_START(pb_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l3.rom", 0x8000, 0x8000, CRC(6f40ee84) SHA1(85453137e3fdb1e422e3903dd053e04c9f2b9607))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("pbot_u21.l1", 0x0000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x8000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("pbot_u19.l1", 0x10000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
ROM_END

GAME(1987,	f14_l1,		0,		williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"F14 Tomcat (L-1)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	f14_p3,		f14_l1,	williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"F14 Tomcat (P-3)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	f14_p4,		f14_l1,	williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"F14 Tomcat (P-4)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	fire_l3,	0,		williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"Fire! (L-3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	milln_l3,	0,		williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"Millionaire (L-3)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	pb_l5,		0,		williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"Pinbot (L-5)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	pb_l2,		pb_l5,	williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"Pinbot (L-2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	pb_l3,		pb_l5,	williams_s11a,	williams_s11a,	williams_s11a,	ROT0,	"Williams",				"Pinbot (L-3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
