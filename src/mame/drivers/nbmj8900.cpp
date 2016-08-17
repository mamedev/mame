// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj8900 - Nichibutsu Mahjong games for years 1989

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2007/05/13 -

******************************************************************************/
/******************************************************************************

Notes:

TODO:

- Real machine has ROMs for protection, but I don't know how to access the ROM,
  so I'm doing something that works but is probably wrong.
  The interesting thing about that ROM is that it comes from other, older games,
  so it isn't needed, it's just verified for protection.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flipping is not perfect.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/3812intf.h"
#include "includes/nbmj8900.h"


DRIVER_INIT_MEMBER(nbmj8900_state,ohpaipee)
{
#if 0
	UINT8 *prot = memregion("protdata")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x8374 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */

	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i],2,7,3,5,0,6,4,1);
	}
#else
	unsigned char *ROM = memregion("maincpu")->base();

	// Protection ROM check skip
	ROM[0x00e4] = 0x00;
	ROM[0x00e5] = 0x00;
	ROM[0x00e6] = 0x00;
	// Program ROM SUM check skip
	ROM[0x025c] = 0x00;
	ROM[0x025d] = 0x00;
#endif
}

DRIVER_INIT_MEMBER(nbmj8900_state,togenkyo)
{
#if 0
	UINT8 *prot = memregion("protdata")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x5ece checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i],2,7,3,5,0,6,4,1);
	}
#else
	unsigned char *ROM = memregion("maincpu")->base();

	// Protection ROM check skip
	ROM[0x010b] = 0x00;
	ROM[0x010c] = 0x00;
	ROM[0x010d] = 0x00;
	// Program ROM SUM check skip
//  ROM[0x025c] = 0x00;
//  ROM[0x025d] = 0x00;
#endif
}


static ADDRESS_MAP_START( ohpaipee_map, AS_PROGRAM, 8, nbmj8900_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf00f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( togenkyo_map, AS_PROGRAM, 8, nbmj8900_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf00f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ohpaipee_io_map, AS_IO, 8, nbmj8900_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x20, 0x27) AM_WRITE(blitter_w)

	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)

	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)

	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)

	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport3_r)
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(vramsel_w)
	AM_RANGE(0xf0, 0xf0) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw1_r)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw2_r, outcoin_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( ohpaipee )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )         // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( togenkyo )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )         // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static MACHINE_CONFIG_START( ohpaipee, nbmj8900_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 20000000/4)    /* 5.00 MHz ? */
	MCFG_CPU_PROGRAM_MAP(ohpaipee_map)
	MCFG_CPU_IO_MAP(ohpaipee_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nbmj8900_state, irq0_line_hold)

	MCFG_NB1413M3_ADD("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_OHPAIPEE )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 8, 248-1)
	MCFG_SCREEN_UPDATE_DRIVER(nbmj8900_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 2500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.85)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( togenkyo, ohpaipee )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(togenkyo_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_TOGENKYO )
MACHINE_CONFIG_END


ROM_START( ohpaipee )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "02.3h",  0x00000, 0x10000, CRC(2b6c9afc) SHA1(591a7016ebd99d4a2bfdef5e99da3a1ac9d30d75) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "01.2k",  0x00000, 0x10000, CRC(6ea76e01) SHA1(194a80e4a3a9d660aea0a9790ce1f4e295bae7ab) )

	ROM_REGION( 0x200000, "gfx", 0 ) /* gfx */
	ROM_LOAD( "03.8c",  0x000000, 0x10000, CRC(33b12763) SHA1(62b9753b65bebad9255a60375d2cf257496a085d) )
	ROM_LOAD( "04.8d",  0x010000, 0x10000, CRC(303fcf10) SHA1(6275a38f319665c4352ca6814f8bf2b3d1739b41) )
	ROM_LOAD( "05.8f",  0x020000, 0x10000, CRC(ce394575) SHA1(bdfcafed983b705474f3be3ce8f9a5aea8b33bc1) )
	ROM_LOAD( "06.8f",  0x030000, 0x10000, CRC(9d943b6e) SHA1(d605f6e95cbc09124a73987941d17c53bd4fabf2) )
	ROM_LOAD( "07.8h",  0x040000, 0x10000, CRC(40c25d2f) SHA1(11fe84be8f15a37a505dd8d5c82dbaf0366f266a) )
	ROM_LOAD( "08.8j",  0x050000, 0x10000, CRC(65520a0e) SHA1(b62cfc3d1ee00e309196a16645ff58a31cb45081) )
	ROM_LOAD( "09.8k",  0x060000, 0x10000, CRC(3f4940f9) SHA1(410ab6e429b65d577eccef3ffcfdec912442a4b0) )
	ROM_LOAD( "10.8l",  0x070000, 0x10000, CRC(325c80ff) SHA1(c9612db209b74a56fd40ddd534d24a44e4df3874) )
	ROM_LOAD( "11.8m",  0x080000, 0x10000, CRC(d779661b) SHA1(914f29a1dde2861542ced28735441b05a520409a) )

	ROM_REGION( 0x40000, "protdata", 0 ) /* protection data */
	ROM_LOAD( "4i.bin", 0x000000, 0x40000, CRC(88f33049) SHA1(8b2d019b09ed854f40a8b0c7782645f50b1f2900) )   // same as housemnq/4i.bin gfx data
ROM_END

ROM_START( togenkyo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "02.3h",  0x00000, 0x10000, CRC(a0cc6700) SHA1(49132e00d15aa00f065bbac5e850d08032845ac7) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "01.2k",  0x00000, 0x10000, CRC(5346786b) SHA1(7141b38339ec4f2b1c6d0a604b5e70cb9beccdf1) )

	ROM_REGION( 0x200000, "gfx", 0 ) /* gfx */
	ROM_LOAD( "03.8c",  0x000000, 0x10000, CRC(ce64cb8b) SHA1(b1f99991e19be49d1aac774c79acc527c9379245) )
	ROM_LOAD( "04.8d",  0x010000, 0x10000, CRC(50dd908f) SHA1(28bd5824c55e16c1d62c24577831723c9aded057) )
	ROM_LOAD( "05.8f",  0x020000, 0x10000, CRC(903004e0) SHA1(c017af37cb90b5335bd53b5b62e883be590353fb) )
	ROM_LOAD( "06.8f",  0x030000, 0x10000, CRC(fa47c1e6) SHA1(ebd365fec250056366422361d3c722ae7b30a0a4) )
	ROM_LOAD( "07.8h",  0x040000, 0x10000, CRC(741bde2a) SHA1(9bf1680dc93def5ea6de929eebf05697f6ea43d1) )
	ROM_LOAD( "08.8j",  0x050000, 0x10000, CRC(c3dd2339) SHA1(6cae7b26ff09dae3758c0c8060731f6c777c5b40) )
	ROM_LOAD( "09.8k",  0x060000, 0x10000, CRC(afb1c766) SHA1(58d4c6d00276ebebaca7a42fc47350d5ea310b8e) )
	ROM_LOAD( "10.8l",  0x070000, 0x10000, CRC(18e2a1d4) SHA1(82750e34ba28e10ae3ab935eafd49643d3d057cc) )
	ROM_LOAD( "11.8m",  0x080000, 0x10000, CRC(811682c2) SHA1(5dfc78ce409d8932cf078ced2616e950a52d5d0e) )
	ROM_LOAD( "12.8n",  0x090000, 0x10000, CRC(97808a68) SHA1(3738b2d0ec0dcd1aea19103ffccafde0e84d6c71) )
	ROM_LOAD( "13.10c", 0x0a0000, 0x10000, CRC(ac61612d) SHA1(29854c5e758a2962daa6e281f7c6af87624c53d8) )
	ROM_LOAD( "14.10d", 0x0b0000, 0x10000, CRC(cb472acc) SHA1(82b4089412ecded903745e5382a301c53a483698) )

	ROM_REGION( 0x40000, "protdata", 0 ) /* protection data */
	ROM_LOAD( "4i.bin", 0x000000, 0x40000, CRC(88f33049) SHA1(8b2d019b09ed854f40a8b0c7782645f50b1f2900) )   // same as housemnq/4i.bin gfx data
ROM_END

//    YEAR,     NAME,   PARENT,  MACHINE,    INPUT,     INIT, MONITOR,COMPANY,FULLNAME,FLAGS)
GAME( 1989, ohpaipee,        0, ohpaipee, ohpaipee, nbmj8900_state, ohpaipee,  ROT270, "Nichibutsu", "Oh! Paipee (Japan 890227)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, togenkyo,        0, togenkyo, togenkyo, nbmj8900_state, togenkyo,    ROT0, "Nichibutsu", "Tougenkyou (Japan 890418)", MACHINE_SUPPORTS_SAVE )
