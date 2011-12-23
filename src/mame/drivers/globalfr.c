/*******************************************************************************

  Global Games 'Stealth' Hardware

  CPU is a M37702S 1AFP  /7231

  Motherboard contains very few major components

  Missing sound roms? (or is sound data in the program roms?)

*******************************************************************************/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m37710/m37710.h"

/******************************************************************************/

class globalfr_state : public driver_device
{
public:
	globalfr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
    { }

	required_device<cpu_device> m_maincpu;
};

/******************************************************************************/

static ADDRESS_MAP_START( globalfr_map, AS_PROGRAM, 8, globalfr_state )
    AM_RANGE(0x002000, 0x002fff) AM_RAM
	AM_RANGE(0x008000, 0x07ffff) AM_ROM AM_REGION("maincpu", 0x8000)
    AM_RANGE(0x0a0000, 0x0a01ff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( globalfr )
INPUT_PORTS_END

/******************************************************************************/

static MACHINE_CONFIG_START( globalfr, globalfr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M37710, 4000000)
	MCFG_CPU_PROGRAM_MAP(globalfr_map)
MACHINE_CONFIG_END

/******************************************************************************/


ROM_START( gl_dow )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "deal1-4n.p1", 0x0000, 0x080000, CRC(2bcc595b) SHA1(d22e1d25784f536ec12a534eee12bcc1abad4a5e) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "deal1-4p.p1", 0x0000, 0x080000, CRC(1a962488) SHA1(f933e3e53b892b146664e0462d8f18263b026f7a) )
ROM_END

ROM_START( gl_dowcl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "deal1-6n.p1", 0x0000, 0x080000, CRC(0844fa2c) SHA1(76ac663b260bfba1c1dcf446ce611628c7276e89) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "deal1-6p.p1", 0x0000, 0x080000, CRC(04b285de) SHA1(77df44354d18a981ab0c09cfcb6f5799db5662f0) )
ROM_END

ROM_START( gl_wywh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wish2-9n.p1", 0x0000, 0x020000, CRC(4b248e64) SHA1(24f27d7742b89893ac5ac5e73b11bcc417a304be) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "wish2-9p.p1", 0x0000, 0x020000, CRC(092d66ce) SHA1(50bb47aca15ced3de9c07c46970ff361d2c84ffd) )
	ROM_LOAD( "wsds2-4n.p1", 0x0000, 0x020000, CRC(b83681ef) SHA1(e609e83213ec992a88645f3e025699db1f59d57a) )
	ROM_LOAD( "wsds2-4p.p1", 0x0000, 0x020000, CRC(018346f4) SHA1(c0a47753c4c06c089888ff0759b4d4ae35dab7ba) )
ROM_END

ROM_START( gl_coc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "clbd2-9n.p1", 0x0000, 0x020000, CRC(f2c5387d) SHA1(72210686ea29ca8d5f9514c30ede342fdc146a38) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "clbd2-9p.p1", 0x0000, 0x020000, CRC(4a1509a3) SHA1(f0cb393a29ae6852b669caf0ca153a0bb316a5a1) )
	ROM_LOAD( "clbv3-0.pro", 0x0000, 0x020000, CRC(beadf377) SHA1(b18ee3d214ea7048c6bc8154613e0a693f080a12) )
	ROM_LOAD( "clbv3-0.ste", 0x0000, 0x020000, CRC(5551ed2e) SHA1(56e3421c223f90fea1d48e4b8ef962b2c0cbc01e) )
ROM_END

ROM_START( gl_uyr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rigm2-8n.p1", 0x0000, 0x080000, CRC(6226a3e7) SHA1(84feafc1c630e466142fcd5ef32af09b6d15b5d8) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "rigm2-8p.p1", 0x0000, 0x080000, CRC(3bb758c6) SHA1(df570f8263102920113345febb31a602d8302de5) )
ROM_END

ROM_START( gl_hbh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hote1-0n.p1", 0x0000, 0x080000, CRC(7d0b2f21) SHA1(bcdfe920d71973b8d9769e80635cf0149fd06b1d) )
ROM_END

ROM_START( gl_hbhcl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mhot1-9n.p1", 0x0000, 0x080000, CRC(769ed4b8) SHA1(b725d1d2942521e145580ae3103ddecdd557b447) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "hbrk_hot.p1", 0x0000, 0x080000, CRC(17b4e037) SHA1(394e73109d3f327544db2b8aa37513b3df1ffbf2) )
	ROM_LOAD( "mhot1-9p.p1", 0x0000, 0x080000, CRC(ecea7177) SHA1(831d56dfd48800b0736435d153625f3e21526e19) )
ROM_END



/******************************************************************************/

GAME( 199?, gl_dow,  0,        globalfr, globalfr, 0, ROT0, "Global", "Deals On Wheels (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_dowcl,0,        globalfr, globalfr, 0, ROT0, "Global", "Deals On Wheels Club (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_wywh, 0,        globalfr, globalfr, 0, ROT0, "Global", "Wish You Were Here Club (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_coc,  0,        globalfr, globalfr, 0, ROT0, "Global", "Carry On Clubbin' (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_uyr,  0,        globalfr, globalfr, 0, ROT0, "Global", "Up Yer Riggin Club (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_hbh,  0,        globalfr, globalfr, 0, ROT0, "Global", "Heartbreak Hotel (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_hbhcl,0,        globalfr, globalfr, 0, ROT0, "Global", "Heartbreak Hotel Club (Global) (Stealth)", GAME_IS_SKELETON_MECHANICAL)

