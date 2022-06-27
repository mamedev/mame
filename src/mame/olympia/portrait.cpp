// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Pierpaolo Prazzoli
/**************************************************************************
 Portraits
 (c) 1983 Olympia

Preliminary Driver by Steve Ellenoff & Peo

Changes:

Pierpaolo Prazzoli, xx-07-2004
  - fixed scrolling
  - fixed screen resolution
  - added NVRAM
  - added fake photo when you get the best score
  - fixed service switches and coins
  - added missing roms and the 2nd set

  SW = service switch

  SW1 - SW2
   ON   OFF -> grid test
   ON    ON -> camera test

TODO:
 - add sound;
 - add colors (maybe not RGB555);
 - fix sprites positions (zooming?);
 - video priority bits;
 - offset background scrolling positions (i.e. monkey climbing on trees);
 - camera device (type?);
 - misc unknown input/outputs;


RAM Location 9240: Controls what level you are on: 0-3 (for each scene)

-------------------------------------------------------------------------

Board layout


Top board

8039                                   rom p3f

              74s288                   rom p2f

rom SA                                 rom p1f

rom M/A                                rom p0f

rom W         18318 18318      rom 15  rom 14

8253          18318 18318      rom 05  rom 04
              18318 18318
8253          18318 18318      rom 13  rom 12
              18318 18318
TMS5200       18318 18318      rom 03  rom 02
              18318 18318
                               rom 11  rom 10
              18318 18318
                               rom 01  rom 00

Bottom board

93Z511DC      93425
DM81LS95      93425
              93425            18318
              2148
              2148             18318

                               18318

                               18318

                               18318

              74s288           18318
                     2114
                     2114 4016
                     2114
                     2114 4016

        Z80
DIP1
DIP2    XD2210


XD2210 or 8202
DM81LS95 = TriState buffer
**************************************************************************/

#include "emu.h"
#include "portrait.h"

#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"


void portrait_state::ctrl_w(uint8_t data)
{
	/* bits 4 and 5 are unknown */

	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
	machine().bookkeeping().coin_counter_w(2, data & 0x04);

	/* the 2 lamps near the camera */
	m_lamps[0] = BIT(data, 3);
	m_lamps[1] = BIT(data, 6);

	/* shows the black and white photo from the camera */
	output().set_value("photo", (data >> 7) & 1);
}

void portrait_state::positive_scroll_w(uint8_t data)
{
	m_scroll = data;
}

void portrait_state::negative_scroll_w(uint8_t data)
{
	m_scroll = - (data ^ 0xff);
}

void portrait_state::portrait_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(FUNC(portrait_state::bgvideo_write)).share("bgvideoram");
	map(0x8800, 0x8fff).ram().w(FUNC(portrait_state::fgvideo_write)).share("fgvideoram");
	map(0x9000, 0x91ff).ram().share("spriteram");
	map(0x9200, 0x97ff).ram();
	map(0xa000, 0xa000).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xa010, 0xa010).nopw(); // ?
	map(0xa000, 0xa000).portr("DSW1");
	map(0xa004, 0xa004).portr("DSW2");
	map(0xa008, 0xa008).portr("SYSTEM").w(FUNC(portrait_state::ctrl_w));
	map(0xa010, 0xa010).portr("INPUTS");
	map(0xa018, 0xa018).nopr().w(FUNC(portrait_state::positive_scroll_w));
	map(0xa019, 0xa019).w(FUNC(portrait_state::negative_scroll_w));
	map(0xa800, 0xa83f).ram().share("nvram");
	map(0xffff, 0xffff).nopr();
}


void portrait_state::portrait_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}


static INPUT_PORTS_START( portrait )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x08, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0e, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x0f, "1 Coin / 12 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x01, "3 Coins / 5 Credits" )
	PORT_DIPSETTING(    0x02, "3 Coins / 7 Credits" )
	PORT_DIPSETTING(    0x03, "3 Coins / 10 Credits" )
	PORT_DIPNAME( 0x70, 0x40, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x70, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_5C ) )
	PORT_DIPNAME( 0x80, 0x00, "Service Coin" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Game Play" )
	PORT_DIPSETTING(    0x00, "Normal Play" )
	PORT_DIPSETTING(    0x01, "Freeplay (255 Cameras)" )
	PORT_DIPNAME( 0x02, 0x00, "High Score" )
	PORT_DIPSETTING(    0x00, "11.350 Points" )
	PORT_DIPSETTING(    0x02, "1.350 Points" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Extra Camera" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "Every 10.000 Points" )
	PORT_DIPSETTING(    0x20, "Every 20.000 Points" )
	PORT_DIPSETTING(    0x30, "Every 30.000 Points" )
	PORT_DIPNAME( 0x40, 0x00, "Ostrich Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, "Quick" )
	PORT_DIPNAME( 0x80, 0x80, "Obstacles" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )    PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x40, 0x40, "Service Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE2 ) // hold during boot to clear the NVRAM
	PORT_DIPNAME( 0x40, 0x40, "Service Switch 2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout tile_layout =
{
	16,16, /* tile width, height   */
	1024,  /* number of characters  */
	3,     /* bits per pixel */
	{ 0x8000*8, 0x4000*8, 0x0000*8 }, /* bitplane offsets */
	{
		RGN_FRAC(1,2)+7, RGN_FRAC(1,2)+6, RGN_FRAC(1,2)+5, RGN_FRAC(1,2)+4,
		RGN_FRAC(1,2)+3, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0,
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12, 8*13, 8*14, 8*15 },
	8*16 /* character offset */
};

static GFXDECODE_START( gfx_portrait )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tile_layout, 0, 0x800/8 )
GFXDECODE_END


void portrait_state::portrait(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);     /* 4 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &portrait_state::portrait_map);
	m_maincpu->set_vblank_int("screen", FUNC(portrait_state::irq0_line_hold));

	i8039_device &audiocpu(I8039(config, "audiocpu", 3120000));  /* ? */
	audiocpu.set_addrmap(AS_PROGRAM, &portrait_state::portrait_sound_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 54*8-1, 0*8, 40*8-1);
	screen.set_screen_update(FUNC(portrait_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_portrait);
	PALETTE(config, m_palette, FUNC(portrait_state::portrait_palette), 0x800, 0x40);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	TMS5200(config, m_tms, 640000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( portrait )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for the cpu */
	ROM_LOAD( "prt-p0.bin",  0x0000, 0x2000, CRC(a21874fa) SHA1(3db863f465a35d7d14dd71b47aa7dfe7b39fccf0) )
	ROM_LOAD( "prt-p1.bin",  0x2000, 0x2000, CRC(4d4d7793) SHA1(f828950ebbf285fc92c65f24421a20ceacef1cb9) )
	ROM_LOAD( "prt-p2.bin",  0x4000, 0x2000, CRC(83d88c9c) SHA1(c876f72b66537a49620fa27a5cb8a4aecd378f0a) )
	ROM_LOAD( "prt-p3.bin",  0x6000, 0x2000, CRC(bd32d007) SHA1(cdf814b00c22f9a4503fa54d43fb5781251b67a7) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "port_w.bin",  0x0000, 0x0800, CRC(d3a4e950) SHA1(0a399d43c7690d568874f3b1d55135f803fc223f) )
	ROM_LOAD( "port_ma.bin", 0x0800, 0x0800, CRC(ee242e4f) SHA1(fb67e0d136927e04f4fa819f684c97b0d52ee48c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "port_00.a1", 0x00000, 0x2000, CRC(eb3e1c12) SHA1(2d38b66f52546b40553244c8a5c961279559f5b6) ) /*bit plane 1*/
	ROM_LOAD( "port_10.b1", 0x02000, 0x2000, CRC(0f44e377) SHA1(1955f9f4deab2166f637f43c1f326bd65fc90f6a) ) /*bit plane 1*/

	ROM_LOAD( "port_02.d1", 0x04000, 0x2000, CRC(bd93a3f9) SHA1(9cb479b8840cafd6043ff0cb9d5ca031dcd332ba) ) /*bit plane 2*/
	ROM_LOAD( "port_12.e1", 0x06000, 0x2000, CRC(656b9f20) SHA1(c1907aba3d19be79d92cd73784b8e7ae94910da6) ) /*bit plane 2*/

	ROM_LOAD( "port_04.g1", 0x08000, 0x2000, CRC(2a99feb5) SHA1(b373d2a2bd28aad6dd7a15a2166e03a8b7a34d9b) ) /*bit plane 3*/
	ROM_LOAD( "port_14.g1", 0x0a000, 0x2000, CRC(224b7a58) SHA1(b84e70d22d1cab41e5773fc9daa2e4e55ec9d96e) ) /*bit plane 3*/

	ROM_LOAD( "port_01.a2", 0x10000, 0x2000, CRC(70d27508) SHA1(d011f85b31bb3aa6f386e8e0edb91df10f4c4eb6) ) /*bit plane 1*/
	ROM_LOAD( "port_11.b2", 0x12000, 0x2000, CRC(f498e395) SHA1(beb1d12433a350e5b773126de3f2803a9f5620c1) ) /*bit plane 1*/

	ROM_LOAD( "port_03.d2", 0x14000, 0x2000, CRC(03d4153a) SHA1(7ce69ce6a101870dbfca1a9787fb1e660024bc02) ) /*bit plane 2*/
	ROM_LOAD( "port_13.e2", 0x16000, 0x2000, CRC(10fa22b8) SHA1(e8f4c24fcdda0ce5e33bc600acd574a232a9bb21) ) /*bit plane 2*/

	ROM_LOAD( "port_05.g2", 0x18000, 0x2000, CRC(43ea7951) SHA1(df0ae7fa802365979514063e1d67cdd45ecada90) ) /*bit plane 3*/
	ROM_LOAD( "port_15.h2", 0x1a000, 0x2000, CRC(ab20b438) SHA1(ea5d60f6a9f06397bd0c6ee028b463c684090c01) ) /*bit plane 3*/

	ROM_REGION( 0x0800, "user1", 0 ) // sound related?
	ROM_LOAD( "port_sa.bin", 0x0000, 0x0800, CRC(50510897) SHA1(8af0f42699602a5b33500968c958e3784e03377f) )

	ROM_REGION( 0x800, "tileattr", 0 ) // tile attributes
	ROM_LOAD( "93z511.bin",   0x0000, 0x0800, CRC(d66d9036) SHA1(7a25efbd8f2f94a01aad9e2be9cb18da7b9ec1d1) )

	ROM_REGION( 0x40, "proms", 0 ) // colors
	ROM_LOAD( "port_pr1.bin", 0x00, 0x0020, CRC(1e2deabb) SHA1(8357e53dba26bca9bc5d7a25c715836f0b3700b9) )
	ROM_LOAD( "port_pr2.n4",  0x20, 0x0020, CRC(008634f3) SHA1(7cde6b09ede672d562569866d944428198f2ba9c) )
ROM_END

ROM_START( portraita )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for the cpu */
	ROM_LOAD( "portp0f.m1",   0x0000, 0x2000, CRC(333eace3) SHA1(8f02df09d8b50d7e37d5abf7d539624c59a7201e) )
	ROM_LOAD( "portp0f.p1",   0x2000, 0x2000, CRC(fe258052) SHA1(f453eb05c68d61dfd644688732ff5c07366c68c0) )
	ROM_LOAD( "portp2f.r1",   0x4000, 0x2000, CRC(bc0104d5) SHA1(7707b85cde2dc9bd95391d4e1dbed219c52618cd) )
	ROM_LOAD( "portp3f.s1",   0x6000, 0x2000, CRC(3f5a3bdf) SHA1(cc4b5d24d0df0962b0cfd4d5c66baac5e4718237) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "port_w.bin",  0x0000, 0x0800, CRC(d3a4e950) SHA1(0a399d43c7690d568874f3b1d55135f803fc223f) )
	ROM_LOAD( "port_ma.bin", 0x0800, 0x0800, CRC(ee242e4f) SHA1(fb67e0d136927e04f4fa819f684c97b0d52ee48c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "port_00.a1", 0x00000, 0x2000, CRC(eb3e1c12) SHA1(2d38b66f52546b40553244c8a5c961279559f5b6) ) /*bit plane 1*/
	ROM_LOAD( "port_10.b1", 0x02000, 0x2000, CRC(0f44e377) SHA1(1955f9f4deab2166f637f43c1f326bd65fc90f6a) ) /*bit plane 1*/
	ROM_LOAD( "port_02.d1", 0x04000, 0x2000, CRC(bd93a3f9) SHA1(9cb479b8840cafd6043ff0cb9d5ca031dcd332ba) ) /*bit plane 2*/
	ROM_LOAD( "port_12.e1", 0x06000, 0x2000, CRC(656b9f20) SHA1(c1907aba3d19be79d92cd73784b8e7ae94910da6) ) /*bit plane 2*/
	ROM_LOAD( "port_04.g1", 0x08000, 0x2000, CRC(2a99feb5) SHA1(b373d2a2bd28aad6dd7a15a2166e03a8b7a34d9b) ) /*bit plane 3*/
	ROM_LOAD( "port_14.g1", 0x0a000, 0x2000, CRC(224b7a58) SHA1(b84e70d22d1cab41e5773fc9daa2e4e55ec9d96e) ) /*bit plane 3*/

	ROM_LOAD( "port_01.a2", 0x10000, 0x2000, CRC(70d27508) SHA1(d011f85b31bb3aa6f386e8e0edb91df10f4c4eb6) ) /*bit plane 1*/
	ROM_LOAD( "port_11.b2", 0x12000, 0x2000, CRC(f498e395) SHA1(beb1d12433a350e5b773126de3f2803a9f5620c1) ) /*bit plane 1*/
	ROM_LOAD( "port_03.d2", 0x14000, 0x2000, CRC(03d4153a) SHA1(7ce69ce6a101870dbfca1a9787fb1e660024bc02) ) /*bit plane 2*/
	ROM_LOAD( "port_13.e2", 0x16000, 0x2000, CRC(10fa22b8) SHA1(e8f4c24fcdda0ce5e33bc600acd574a232a9bb21) ) /*bit plane 2*/
	ROM_LOAD( "port_05.g2", 0x18000, 0x2000, CRC(43ea7951) SHA1(df0ae7fa802365979514063e1d67cdd45ecada90) ) /*bit plane 3*/
	ROM_LOAD( "port_15.h2", 0x1a000, 0x2000, CRC(ab20b438) SHA1(ea5d60f6a9f06397bd0c6ee028b463c684090c01) ) /*bit plane 3*/

	ROM_REGION( 0x800, "tileattr", 0 ) // tile attributes (see notes)
	ROM_LOAD( "93z511.bin",   0x0000, 0x0800, CRC(d66d9036) SHA1(7a25efbd8f2f94a01aad9e2be9cb18da7b9ec1d1) )

	ROM_REGION( 0x40, "proms", 0 ) // colors
	ROM_LOAD( "port_pr1.bin", 0x00, 0x0020, CRC(1e2deabb) SHA1(8357e53dba26bca9bc5d7a25c715836f0b3700b9) )
	ROM_LOAD( "port_pr2.n4",  0x20, 0x0020, CRC(008634f3) SHA1(7cde6b09ede672d562569866d944428198f2ba9c) )
ROM_END

/* tileattr rom

  this appears to be divided into 2 0x400 banks

  0x000 - 0x3ff relates to tiles 0x000-0x0ff

  0x400 - 0x7ff relates to tiles 0x100-0x1ff, 0x200-0x2ff, and 0x300-0x3ff

  every 2 tiles are somehow related to 8 bytes in the data

   so tiles 0x00 and 0x01 use bytes 0x000-0x007
            0x02                    0x008
            0x04                    0x010
            0x06                    0x018
            0x08                    0x020
            0x0a                    0x028
            0x0c                    0x030
            0x0e                    0x038
            0x10                    0x040
               .......
            0xfe and 0xff use bytes 0x3f8-0x3ff
            etc.

    it's probably some kind of lookup table for the colours (6bpp = 8 colours, maybe every 2 tiles share the same 8 colours)
    I guess either the bank (0/1) can be selected, or bank 0 is hardcoded to tiles 0x000-0x0ff (because tilemaps can use
     these tiles too, so it's not a case of it being a sprite/tilemap lookup split)

    anyway.. this is why the portraits logo is broken across 3 areas (0x1f2, 0x2f2, 0x3f2) so that they can share the same
    attributes from this rom

  */



GAME( 1983, portrait, 0,        portrait, portrait, portrait_state, empty_init, ROT270, "Olympia", "Portraits (set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1983, portraita,portrait, portrait, portrait, portrait_state, empty_init, ROT270, "Olympia", "Portraits (set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
