// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

    Mr. Jong
    (c)1983 Kiwako (This game is distributed by Sanritsu.)

    Crazy Blocks
    (c)1983 Kiwako/ECI

    Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 2000/03/20 -

    Block Buster
    (c)1983 Kiwako/ECI


PCB Layout
----------


C2-00154C
|-----------------------------------------|
|                    93422                |
|                    93422                |
|          4H  5H           PROM7J        |
|                           PAL    DSW1(8)|
|              PROM5G                     |
|                           76489         |
|                           76489         |
|               Z80                       |
|                                PAL      |
|15.468MHz PAL              6116     555  |
|                                         |
|         6116          6A 7A 8A 9A  6116 |
|                                         |
|-----------------------------------------|

Notes:
          Z80 clock: 2.576MHz (= XTAL / 6)
      XTAL measured: 15.459MHz
             PROM5G: MB7052 = 82S129
             PROM7J: MB7056 = 82S123
     ROMs 4H and 5h: 2732
ROMs 6A, 7A, 8A, 9A: 2764

***************************************************************************/

#include "emu.h"
#include "mrjong.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t mrjong_state::io_0x03_r()
{
	return 0x00;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void mrjong_state::mrjong_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa7ff).ram();
	map(0xe000, 0xe3ff).ram().w(FUNC(mrjong_state::mrjong_videoram_w)).share("videoram");
	map(0xe400, 0xe7ff).ram().w(FUNC(mrjong_state::mrjong_colorram_w)).share("colorram");
}

void mrjong_state::mrjong_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P2").w(FUNC(mrjong_state::mrjong_flipscreen_w));
	map(0x01, 0x01).portr("P1").w("sn1", FUNC(sn76489_device::write));
	map(0x02, 0x02).portr("DSW").w("sn2", FUNC(sn76489_device::write));
	map(0x03, 0x03).r(FUNC(mrjong_state::io_0x03_r));     // Unknown
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mrjong )
	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )        // ????

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30k")
	PORT_DIPSETTING(    0x04, "50k")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x10, "4")
	PORT_DIPSETTING(    0x20, "5")
	PORT_DIPSETTING(    0x30, "6")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8, 8,               /* 8*8 characters */
	512,                /* 512 characters */
	2,              /* 2 bits per pixel */
	{ 0, 512*8*8 },         /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* pretty straightforward layout */
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8             /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16, 16,             /* 16*16 sprites */
	128,                /* 128 sprites */
	2,              /* 2 bits per pixel */
	{ 0, 128*16*16 },       /* the bitplanes are separated */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,   /* pretty straightforward layout */
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8                /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_mrjong )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,      0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout,    0, 32 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mrjong_state::mrjong(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu,15468000/6); /* 2.578 MHz?? */
	m_maincpu->set_addrmap(AS_PROGRAM, &mrjong_state::mrjong_map);
	m_maincpu->set_addrmap(AS_IO, &mrjong_state::mrjong_io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 30*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(mrjong_state::screen_update_mrjong));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mrjong);
	PALETTE(config, m_palette, FUNC(mrjong_state::mrjong_palette), 4 * 32, 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76489(config, "sn1", 15468000 / 6).add_route(ALL_OUTPUTS, "mono", 1.0);
	SN76489(config, "sn2", 15468000 / 6).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mrjong )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* code */
	ROM_LOAD( "mj00", 0x0000, 0x2000, CRC(d211aed3) SHA1(01f252ca1d2399146fa3ed44cb2daa1d5925cae5) )
	ROM_LOAD( "mj01", 0x2000, 0x2000, CRC(49a9ca7e) SHA1(fc5279ba782da2c8288042bd17282366fcd788cc) )
	ROM_LOAD( "mj02", 0x4000, 0x2000, CRC(4b50ae6a) SHA1(6fa6bae926c5e4cc154f5f1a6dc7bb7ef5bb484a) )
	ROM_LOAD( "mj03", 0x6000, 0x2000, CRC(2c375a17) SHA1(9719485cdca535771b498a37d57734463858f2cd) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mj21", 0x0000, 0x1000, CRC(1ea99dab) SHA1(21a296d394e5cac0c7cb2ea8efaeeeee976ac4b5) )
	ROM_LOAD( "mj20", 0x1000, 0x1000, CRC(7eb1d381) SHA1(fa13700f132c03d2d2cee65abf24024db656aff7) )

	ROM_REGION( 0x0120, "proms", 0 )    /* color */
	ROM_LOAD( "mj61", 0x0000, 0x0020, CRC(a85e9b27) SHA1(55df208b771a98fcf6c2c19ffdf973891ebcabd1) )
	ROM_LOAD( "mj60", 0x0020, 0x0100, CRC(dd2b304f) SHA1(d7320521e83ddf269a9fc0c91f0e0e61428b187c) )
ROM_END

ROM_START( crazyblk )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* code */
	ROM_LOAD( "c1.a6", 0x0000, 0x2000, CRC(e2a211a2) SHA1(5bcf5a0cb25ce5adfb6519c8a3a4ee6e55e1e7de) )
	ROM_LOAD( "c2.a7", 0x2000, 0x2000, CRC(75070978) SHA1(7f59460c094e596a521014f956d76e5c714022a2) )
	ROM_LOAD( "c3.a7", 0x4000, 0x2000, CRC(696ca502) SHA1(8ce7e31e9a7161633fee7f28b215e4358d906c4b) )
	ROM_LOAD( "c4.a8", 0x6000, 0x2000, CRC(c7f5a247) SHA1(de79341f9c6c7032f76cead46d614e13d4af50f9) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "c6.h5", 0x0000, 0x1000, CRC(2b2af794) SHA1(d13bc8e8ea6c9bc2066ed692108151523d1f936b) )
	ROM_LOAD( "c5.h4", 0x1000, 0x1000, CRC(98d13915) SHA1(b51104f9f80128ff7a52ac2efa9519bf9d7b78bc) )

	ROM_REGION( 0x0120, "proms", 0 )    /* color */
	ROM_LOAD( "clr.j7", 0x0000, 0x0020, CRC(ee1cf1d5) SHA1(4f4cfde1a896da92d8265889584dd0c5678de033) )
	ROM_LOAD( "clr.g5", 0x0020, 0x0100, CRC(bcb1e2e3) SHA1(c09731836a9d4e50316a84b86f61b599a1ef944d) )
ROM_END

ROM_START( blkbustr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* code */
	ROM_LOAD( "6a.bin", 0x0000, 0x2000, CRC(9e4b426c) SHA1(831360c473ab2452f4d0da12609c96c601e21c17) )
	ROM_LOAD( "c2.a7",  0x2000, 0x2000, CRC(75070978) SHA1(7f59460c094e596a521014f956d76e5c714022a2) )
	ROM_LOAD( "8a.bin", 0x4000, 0x2000, CRC(0e803777) SHA1(bccc182ccbd7312fc6545ffcef4d54637416dae7) )
	ROM_LOAD( "c4.a8",  0x6000, 0x2000, CRC(c7f5a247) SHA1(de79341f9c6c7032f76cead46d614e13d4af50f9) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4h.bin", 0x0000, 0x1000, CRC(67dd6c19) SHA1(d3dc0cb9b108c2584c4844fc0eb4c9ee170986fe) )
	ROM_LOAD( "5h.bin", 0x1000, 0x1000, CRC(50fba1d4) SHA1(40ba480713284ae484c6687490f91bf62a7167e1) )

	ROM_REGION( 0x0120, "proms", 0 )    /* color */
	ROM_LOAD( "clr.j7", 0x0000, 0x0020, CRC(ee1cf1d5) SHA1(4f4cfde1a896da92d8265889584dd0c5678de033) )
	ROM_LOAD( "clr.g5", 0x0020, 0x0100, CRC(bcb1e2e3) SHA1(c09731836a9d4e50316a84b86f61b599a1ef944d) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, mrjong,   0,      mrjong, mrjong, mrjong_state, empty_init, ROT90, "Kiwako",               "Mr. Jong (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, crazyblk, mrjong, mrjong, mrjong, mrjong_state, empty_init, ROT90, "Kiwako (ECI license)", "Crazy Blocks",     MACHINE_SUPPORTS_SAVE )
GAME( 1983, blkbustr, mrjong, mrjong, mrjong, mrjong_state, empty_init, ROT90, "Kiwako (ECI license)", "BlockBuster",      MACHINE_SUPPORTS_SAVE )
