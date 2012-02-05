/*
    Bally MPU AS-2518-17
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class by17_state : public driver_device
{
public:
	by17_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( by17_map, AS_PROGRAM, 8, by17_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0200, 0x02ff) AM_RAM // CMOS NVRAM
	AM_RANGE(0x1000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( by17 )
INPUT_PORTS_END

void by17_state::machine_reset()
{
}

static DRIVER_INIT( by17 )
{
}

static MACHINE_CONFIG_START( by17, by17_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(by17_map)
MACHINE_CONFIG_END

/*--------------------------------
/ Black Jack
/-------------------------------*/
ROM_START(blackjck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "728-32_2.716", 0x1000, 0x0800, CRC(1333c9d1) SHA1(1fbb60d84db47ffaf7f291575b2705783a110678))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Bow and Arrow
/-------------------------------*/
ROM_START(bowarrow)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("b14.bin", 0x1400, 0x0200, CRC(d4d0f92a) SHA1(b996cbe9762fafd64115dc78e24626cf08f8abf7))
	ROM_LOAD("b16.bin", 0x1600, 0x0200, CRC(ad2102e7) SHA1(86887beea5e03e80f60c947d6d71431e5eab3d1b))
	ROM_LOAD("b18.bin", 0x1800, 0x0200, CRC(5d84656b) SHA1(d17350f5a0cc0cd00b60df4903034489dce7ade5))
	ROM_LOAD("b1a.bin", 0x1a00, 0x0200, CRC(6f083ce6) SHA1(624b00e72e223c6b9fbf38b831200c9a7aa0d8f7))
	ROM_LOAD("b1c.bin", 0x1c00, 0x0200, CRC(6ed4d39e) SHA1(1f6c57c7274c76246dd2f0b70ec459857a5cf1eb))
	ROM_LOAD("b1e.bin", 0x1e00, 0x0200, CRC(ff2f97de) SHA1(28a8fdeccb1382d3a1153c97466426459c9fa075))
	ROM_RELOAD( 0xfe00, 0x0200)
ROM_END

/*--------------------------------
/ Eight Ball
/-------------------------------*/
ROM_START(eightbll)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "723-20_2.716", 0x1000, 0x0800, CRC(33559e7b) SHA1(49008db95c8f012e7e3b613e6eee811512207fa9))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Evel Knievel
/-------------------------------*/
ROM_START(evelknie)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "722-17_2.716", 0x1000, 0x0800, CRC(b6d9a3fa) SHA1(1939e13f73a324e3d2fd269a54446f48cf530f50))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Freedom
/-------------------------------*/
ROM_START(freedom)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "720-08_1.474", 0x1400, 0x0200, CRC(b78bceeb) SHA1(acf6f1a497ada344211f12dbf4be619bee559950))
	ROM_LOAD( "720-10_2.474", 0x1600, 0x0200, CRC(ca90c8a7) SHA1(d9b5e95247e846e50a2a43c85ad5eb1fc761ab67))
	ROM_LOAD( "720-07_6.716", 0x1800, 0x0800, CRC(0f4e8b83) SHA1(faa05dde24eb60be0cdc4456ae2e660a15ed85ac))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Mata Hari
/-------------------------------*/
ROM_START(matahari)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "725-21_2.716", 0x1000, 0x0800, CRC(63acd9b0) SHA1(2347342f1281c097ea39c79236d85b00a1dfc7b2))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Night Rider
/-------------------------------*/
ROM_START(nightrdr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "721-21_1.716", 0x1000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(f394e357) SHA1(73444f848825a398515153d18de027792b57bcc7))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

ROM_START(nightr20)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "721-21_1.716", 0x1000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Power Play
/-------------------------------*/
ROM_START(pwerplay)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "724-25_2.716", 0x1000, 0x0800, CRC(43012f35) SHA1(f90d582e3394d949a637a09882ffdad7664c44c0))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Stellar Airship / Geiger-Automatenbau GMBH, of Germany (1981)
/-------------------------------*/

/*--------------------------------
/ Strikes and Spares
/-------------------------------*/
ROM_START(stk_sprs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "740-16_2.716", 0x1000, 0x0800, CRC(2be27024) SHA1(266dee3a5c4c115acc20543df2eb172f1e85dacb))
	ROM_LOAD( "720-20_6.716", 0x1800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END


GAME( 1978, blackjck, 0,		by17, by17, by17, ROT0, "Bally","Black Jack (Pinball)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1976, bowarrow, 0,		by17, by17, by17, ROT0, "Bally","Bow & Arrow (Prototype)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, eightbll, 0,		by17, by17, by17, ROT0, "Bally","Eight Ball", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, evelknie, 0,		by17, by17, by17, ROT0, "Bally","Evel Knievel", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, freedom,  0,		by17, by17, by17, ROT0, "Bally","Freedom", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, matahari, 0,		by17, by17, by17, ROT0, "Bally","Mata Hari", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, nightrdr, 0,		by17, by17, by17, ROT0, "Bally","Night Rider (rev. 21)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, nightr20, nightrdr,	by17, by17, by17, ROT0, "Bally","Night Rider (rev. 20)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, pwerplay, 0,		by17, by17, by17, ROT0, "Bally","Power Play (Pinball)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, stk_sprs, 0,		by17, by17, by17, ROT0, "Bally","Strikes and Spares", GAME_IS_SKELETON_MECHANICAL)
