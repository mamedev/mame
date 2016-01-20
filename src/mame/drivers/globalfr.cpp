// license:BSD-3-Clause
// copyright-holders:David Haywood
/*******************************************************************************

  Global Games 'Stealth' Hardware

  CPU is a M37702S 1AFP  /7231

  Motherboard contains very few major components

  Missing sound roms? (or is sound data in the program roms?)
  NOTE: VFD is guessed as Samsung 16 segment, like other Global products
  need to know more
*******************************************************************************/


#include "emu.h"
#include "cpu/m37710/m37710.h"
#include "machine/roc10937.h"
#include "globalfr.lh"

/******************************************************************************/

class globalfr_state : public driver_device
{
public:
	globalfr_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vfd(*this, "vfd")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<s16lf01_t> m_vfd;

// serial vfd

	DECLARE_WRITE16_MEMBER(vfd_w);

};

/******************************************************************************/

WRITE16_MEMBER(globalfr_state::vfd_w)
{
//  m_vfd->(data & 0x20) need to find reset
	{
		m_vfd->sclk((data&0x40));
		m_vfd->data(!(data&0x80));
	}
}

static ADDRESS_MAP_START( globalfr_map, AS_PROGRAM, 16, globalfr_state )
	AM_RANGE(0x002000, 0x002fff) AM_RAM
	AM_RANGE(0x008000, 0x07ffff) AM_ROM AM_REGION("maincpu", 0x8000)
	AM_RANGE(0x0a0000, 0x0a01ff) AM_RAM
	AM_RANGE(0x7e0040, 0x7e0041) AM_WRITE(vfd_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( globalfr )
INPUT_PORTS_END


/******************************************************************************/

static MACHINE_CONFIG_START( globalfr, globalfr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M37702S1, 4000000)
	MCFG_CPU_PROGRAM_MAP(globalfr_map)
	MCFG_S16LF01_ADD("vfd",0)
	MCFG_DEFAULT_LAYOUT(layout_globalfr)
MACHINE_CONFIG_END

/******************************************************************************/

#define GL_SNBEV_SOUND \
	ROM_REGION( 0x200000, "snd", 0 ) \
	ROM_LOAD( "3pbv0-8t.u1", 0x000000, 0x100000, CRC(330d891f) SHA1(3bf0f4a107289a079371cd3a9dcce6dd9b304dfe) ) \
	ROM_LOAD( "3pbv0-8t.u2", 0x100000, 0x100000, CRC(b4a0d108) SHA1(cb63bf3cb399c91015b70ef0ebd95e2bdc405b27) )
ROM_START( gl_snbev )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "3pbv1-3n.g0", 0x000000, 0x020000, CRC(591eccbe) SHA1(aa5d38af8f8cbb4b2b0439dba29537feac9ddd29) )
	GL_SNBEV_SOUND
ROM_END

ROM_START( gl_snbeva )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "3pbv1-4n.g0", 0x000000, 0x020000, CRC(a722af41) SHA1(452322ab7ccfab44996295db0a0b122b43fe4cab) )
	GL_SNBEV_SOUND
ROM_END


ROM_START( gl_grncl )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "grid3-3n.p1", 0x000001, 0x020000, CRC(26c8d7cb) SHA1(fadeea78abc0b90c7527acaff5f08f9f24e8b67a) )
ROM_END

ROM_START( gl_grncla )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "grid3-4p.p1", 0x000000, 0x020000, CRC(410f5571) SHA1(38536c41f103ec94e866cad4d7d80d4b66028b2e) )
ROM_END



ROM_START( gl_dow )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "deal1-4n.p1", 0x0000, 0x080000, CRC(2bcc595b) SHA1(d22e1d25784f536ec12a534eee12bcc1abad4a5e) )
ROM_END

ROM_START( gl_dowp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "deal1-4p.p1", 0x0000, 0x080000, CRC(1a962488) SHA1(f933e3e53b892b146664e0462d8f18263b026f7a) )
ROM_END

ROM_START( gl_dowcl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "deal1-6n.p1", 0x0000, 0x080000, CRC(0844fa2c) SHA1(76ac663b260bfba1c1dcf446ce611628c7276e89) )
ROM_END

ROM_START( gl_dowclp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "deal1-6p.p1", 0x0000, 0x080000, CRC(04b285de) SHA1(77df44354d18a981ab0c09cfcb6f5799db5662f0) )
ROM_END

ROM_START( gl_wywh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wish2-9n.p1", 0x0000, 0x020000, CRC(4b248e64) SHA1(24f27d7742b89893ac5ac5e73b11bcc417a304be) )
ROM_END

ROM_START( gl_wywhp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wish2-9p.p1", 0x0000, 0x020000, CRC(092d66ce) SHA1(50bb47aca15ced3de9c07c46970ff361d2c84ffd) )
ROM_END

ROM_START( gl_wywh24 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wsds2-4n.p1", 0x0000, 0x020000, CRC(b83681ef) SHA1(e609e83213ec992a88645f3e025699db1f59d57a) )
ROM_END

ROM_START( gl_wywh24p )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wsds2-4p.p1", 0x0000, 0x020000, CRC(018346f4) SHA1(c0a47753c4c06c089888ff0759b4d4ae35dab7ba) )
ROM_END

ROM_START( gl_coc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "clbv3-0.ste", 0x0000, 0x020000, CRC(5551ed2e) SHA1(56e3421c223f90fea1d48e4b8ef962b2c0cbc01e) )
ROM_END

ROM_START( gl_cocp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "clbv3-0.pro", 0x0000, 0x020000, CRC(beadf377) SHA1(b18ee3d214ea7048c6bc8154613e0a693f080a12) )
ROM_END

ROM_START( gl_coc29 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "clbd2-9n.p1", 0x0000, 0x020000, CRC(f2c5387d) SHA1(72210686ea29ca8d5f9514c30ede342fdc146a38) )
ROM_END

ROM_START( gl_coc29p )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "clbd2-9p.p1", 0x0000, 0x020000, CRC(4a1509a3) SHA1(f0cb393a29ae6852b669caf0ca153a0bb316a5a1) )
ROM_END

ROM_START( gl_uyr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rigm2-8n.p1", 0x0000, 0x080000, CRC(6226a3e7) SHA1(84feafc1c630e466142fcd5ef32af09b6d15b5d8) )
ROM_END

ROM_START( gl_uyrp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rigm2-8p.p1", 0x0000, 0x080000, CRC(3bb758c6) SHA1(df570f8263102920113345febb31a602d8302de5) )
ROM_END

ROM_START( gl_hbh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hote1-0n.p1", 0x0000, 0x080000, CRC(7d0b2f21) SHA1(bcdfe920d71973b8d9769e80635cf0149fd06b1d) )
ROM_END

ROM_START( gl_hbhcl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mhot1-9n.p1", 0x0000, 0x080000, CRC(769ed4b8) SHA1(b725d1d2942521e145580ae3103ddecdd557b447) )
ROM_END

ROM_START( gl_hbhclp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mhot1-9p.p1", 0x0000, 0x080000, CRC(ecea7177) SHA1(831d56dfd48800b0736435d153625f3e21526e19) )
ROM_END

ROM_START( gl_hbhcla )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hbrk_hot.p1", 0x0000, 0x080000, CRC(17b4e037) SHA1(394e73109d3f327544db2b8aa37513b3df1ffbf2) )
ROM_END



/******************************************************************************/

// standalone game or topbox?
GAME( 199?, gl_snbev,   0,          globalfr,  globalfr, driver_device,  0,  ROT0, "Global",        "Saturday Night Beaver (Global) (Stealth?) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, gl_snbeva,  gl_snbev,   globalfr,  globalfr, driver_device,  0,  ROT0, "Global",        "Saturday Night Beaver (Global) (Stealth?) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
// standalone game or topbox?
GAME( 199?, gl_grncl,   0,          globalfr,  globalfr, driver_device,  0,  ROT0, "Global",        "Grid Runner Club (Global) (Stealth?) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, gl_grncla,  gl_grncl,   globalfr,  globalfr, driver_device,  0,  ROT0, "Global",        "Grid Runner Club (Global) (Stealth?) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, gl_dow,  0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Deals On Wheels (Global) (v1.4) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_dowp, gl_dow,   globalfr, globalfr, driver_device, 0, ROT0, "Global", "Deals On Wheels (Global) (v1.4 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 199?, gl_dowcl,0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Deals On Wheels Club (Global) (v1.6) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_dowclp,gl_dowcl,globalfr, globalfr, driver_device, 0, ROT0, "Global", "Deals On Wheels Club (Global) (v1.6 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 199?, gl_wywh,  0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Wish You Were Here Club (Global) (v2.9) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_wywhp,  gl_wywh, globalfr, globalfr, driver_device, 0, ROT0, "Global", "Wish You Were Here Club (Global) (v2.9 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_wywh24, gl_wywh, globalfr, globalfr, driver_device, 0, ROT0, "Global", "Wish You Were Here Club (Global) (v2.4) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_wywh24p,gl_wywh, globalfr, globalfr, driver_device, 0, ROT0, "Global", "Wish You Were Here Club (Global) (v2.4 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 199?, gl_coc,   0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Carry On Clubbin' (Global) (v3.0) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_cocp,  gl_coc,   globalfr, globalfr, driver_device, 0, ROT0, "Global", "Carry On Clubbin' (Global) (v3.0 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_coc29, gl_coc,   globalfr, globalfr, driver_device, 0, ROT0, "Global", "Carry On Clubbin' (Global) (v2.9) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_coc29p,gl_coc,   globalfr, globalfr, driver_device, 0, ROT0, "Global", "Carry On Clubbin' (Global) (v2.9 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 199?, gl_uyr,  0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Up Yer Riggin Club (Global) (v2.8) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_uyrp, gl_uyr,   globalfr, globalfr, driver_device, 0, ROT0, "Global", "Up Yer Riggin Club (Global) (v2.8 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 199?, gl_hbh,  0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Heartbreak Hotel (Global) (v1.0) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_hbhcl,0,        globalfr, globalfr, driver_device, 0, ROT0, "Global", "Heartbreak Hotel Club (Global) (v1.9) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_hbhclp,gl_hbhcl, globalfr, globalfr, driver_device, 0, ROT0, "Global", "Heartbreak Hotel Club (Global) (v1.9 Protocol) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, gl_hbhcla,gl_hbhcl, globalfr, globalfr, driver_device, 0, ROT0, "Global", "Heartbreak Hotel Club (Global) (Set 2) (Stealth)", MACHINE_IS_SKELETON_MECHANICAL)
