// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Piggy Pass

hw platform unknown
game details unknown

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

class piggypas_state : public driver_device
{
public:
	piggypas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( piggypas_map, AS_PROGRAM, 8, piggypas_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( piggypas )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



void piggypas_state::machine_start()
{
}

void piggypas_state::machine_reset()
{
}

static MACHINE_CONFIG_START( piggypas, piggypas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000) // wrong CPU? (not valid Z80 code)
	MCFG_CPU_PROGRAM_MAP(piggypas_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", piggypas_state,  irq0_line_hold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( piggypas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pigypass.u6", 0x00000, 0x10000, CRC(c8dc4e26) SHA1(f9643945f84fe2679742922abf5a92b77bf59e4c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pigypass.u14", 0x00000, 0x40000, CRC(855504c1) SHA1(dfe91943057fa66798c8395348cf703cb11468d2) )
ROM_END


ROM_START( hoopshot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hoopshot.u6", 0x00000, 0x08000, CRC(fa8ae8aa) SHA1(2266a775fba7c8f8e3e24441aca6c4b89a6d1ec7) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "hoopshot.u14", 0x00000, 0x40000, CRC(748462b5) SHA1(ccb8f1dbb6471b134c1e97699383c3ef139c42c3) )
ROM_END



ROM_START( rndrndqs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round.u6", 0x00000, 0x08000, CRC(3eb64b10) SHA1(66051cdd6be33f4f7249be1c8d56e5e43c838163) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "round.u14", 0x00000, 0x40000, CRC(36d1c07a) SHA1(3c978d4d03d8dbf79e1afe7dc46209d9ac4d3cc3) )
ROM_END

ROM_START( fidlstix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fiddle.u6", 0x00000, 0x08000, CRC(48125bf1) SHA1(5772fe3c0987fc6b2508da5efe3c4c3c179b76a1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fiddle.u14", 0x00000, 0x40000, CRC(baf4e1cd) SHA1(ae153f832cbd188e9f3f357a1a1f68cc8264d346) )
ROM_END

// bad dump of program rom
ROM_START( jackbean )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beanstlk.u6", 0x00000, 0x10000, BAD_DUMP CRC(127c4d6c) SHA1(c864293f42e81a1b8e5dcb12abc1c0019853792e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "beanstlk.u14", 0x00000, 0x40000, CRC(e33ef0a3) SHA1(337ce3d0c901b0b3241d76601eaad6e3e2724e1a) )
ROM_END

// bad dump of program rom
ROM_START( dumpump )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dump-ump.u6", 0x00000, 0x08000, BAD_DUMP CRC(410fc27e) SHA1(d9505c11f4844b9b58c12b3ff6b860357a4be75e))

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "dump-ump.u14", 0x00000, 0x20000, CRC(08bc7bb5) SHA1(2355783ec614d8f4e1dca3cb415a97a28411157b))
ROM_END

// bad dump of program rom
ROM_START( 3lilpigs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3-pigs.u6", 0x00000, 0x10000, BAD_DUMP CRC(1db9d754) SHA1(9b1db9c9bb155ebb5509970476b20b9dda6d3021) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "3-pigs.u14", 0x00000, 0x40000, CRC(62eb76e2) SHA1(c4cad241dedf2c290f9bf80038415fe39b3ce17d) )
ROM_END





// COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 04.40
GAME( 1992, piggypas,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Piggy Pass (version 04.40)", MACHINE_IS_SKELETON_MECHANICAL )
// COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 05.22
GAME( 1992, hoopshot,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Hoop Shot (version 05.22)", MACHINE_IS_SKELETON_MECHANICAL )
// Quick $ilver   Development Co.    10/08/96      ROUND  REV 6 
GAME( 1996, rndrndqs,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Quick $ilver Development Co.", "Round and Round (Rev 6) (Quick $ilver)", MACHINE_IS_SKELETON_MECHANICAL )
//  Quick$ilver   Development Co.    10/02/95      -FIDDLESTIX-       REV 15T   
GAME( 1995, fidlstix,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Quick $ilver Development Co.", "Fiddle Stix (1st Rev)", MACHINE_IS_SKELETON_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, jackbean,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Jack & The Beanstalk (Doyle & Assoc.?)", MACHINE_IS_SKELETON_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, dumpump,   0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Dump The Ump", MACHINE_IS_SKELETON_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, 3lilpigs,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "3 Lil' Pigs", MACHINE_IS_SKELETON_MECHANICAL )
