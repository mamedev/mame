// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Hot Pinball
Gals Pinball

driver by Nicola Salmoria

Notes:
- to start a 2 or more players game, press the start button multiple times
- the sprite graphics contain a "(c) Tecmo", and the sprite system is
  indeed similar to other Tecmo games like Ninja Gaiden.
- Clearly based on Temco's Super Pinball Action (see spbactn.c)
- There seems to be a bug in Hot Pinball's Demo Sounds. If you start the
  game normally you get no demo sounds. However if you select the "Slide
  Show" and run all the way through the game will start with demo sounds.
  Gals Pinball's Demo Sounds works as expected according to DSW setting.

TODO:
- scrolling is wrong.
- sprite/tile priority might be wrong. There is an unknown bit in the fg
  tilemap, marking some tiles that I'm not currently drawing.

Manuals for both games define the controls as 4 push buttons:

   Left Push Buttons                     Right Push Buttons

|   o    |      o       |             |    o    |      o       |
|--------+--------------+-------------+------------------------|
| Select |     Set      | Select Mode |                        |
|  Stage |    Stage     |             |                        |
|--------+--------------+-------------+---------+--------------|
|Flipper | Shot & Shake |  Game Mode  | Flipper | Shot & Shake |
| Left   |    Left      |             |  Right  |    Right     |
|--------+--------------+-------------+---------+--------------|

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "includes/galspnbl.h"


WRITE16_MEMBER(galspnbl_state::soundcommand_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space,offset,data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, galspnbl_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x700000, 0x703fff) AM_RAM     /* galspnbl work RAM */
	AM_RANGE(0x708000, 0x70ffff) AM_RAM     /* galspnbl work RAM, bitmaps are decompressed here */
	AM_RANGE(0x800000, 0x803fff) AM_RAM     /* hotpinbl work RAM */
	AM_RANGE(0x808000, 0x80ffff) AM_RAM     /* hotpinbl work RAM, bitmaps are decompressed here */
	AM_RANGE(0x880000, 0x880fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8ff400, 0x8fffff) AM_WRITENOP    /* ??? */
	AM_RANGE(0x900000, 0x900fff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0x901000, 0x903fff) AM_WRITENOP    /* ??? */
	AM_RANGE(0x904000, 0x904fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x905000, 0x907fff) AM_WRITENOP    /* ??? */
	AM_RANGE(0x980000, 0x9bffff) AM_RAM AM_SHARE("bgvideoram")
	AM_RANGE(0xa00000, 0xa00fff) AM_WRITENOP    /* more palette ? */
	AM_RANGE(0xa01000, 0xa017ff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xa01800, 0xa027ff) AM_WRITENOP    /* more palette ? */
	AM_RANGE(0xa80000, 0xa80001) AM_READ_PORT("IN0")
	AM_RANGE(0xa80010, 0xa80011) AM_READ_PORT("IN1") AM_WRITE(soundcommand_w)
	AM_RANGE(0xa80020, 0xa80021) AM_READ_PORT("SYSTEM") AM_WRITENOP     /* w - could be watchdog, but causes resets when picture is shown */
	AM_RANGE(0xa80030, 0xa80031) AM_READ_PORT("DSW1") AM_WRITENOP       /* w - irq ack? */
	AM_RANGE(0xa80040, 0xa80041) AM_READ_PORT("DSW2")
	AM_RANGE(0xa80050, 0xa80051) AM_WRITEONLY AM_SHARE("scroll")    /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, galspnbl_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xf810, 0xf811) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0xfc00, 0xfc00) AM_NOP /* irq ack ?? */
	AM_RANGE(0xfc20, 0xfc20) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( galspnbl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* tested at 0x0016c6 - doesn't seem tilt related */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* tested at 0x0016d2 - doesn't seem tilt related */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flippers" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball / Shake (Left Side)" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* uncalled (?) code at 0x007ed2 ('hotpinbl') or 0x008106 ('galspnbl') */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Launch Ball / Shake (Right Side)" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* uncalled (?) code at 0x007e90 ('hotpinbl') or 0x0080c4 ('galspnbl') */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start" )  /* needed to avoid confusion with # of players. Press mulitple times for multiple players */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  /* 0x700018.b */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2") /* see code at 0x0085c6 ('hotpinbl') or 0x008994 ('galspnbl') */
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Ball" )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "100K 500K" )
	PORT_DIPSETTING(    0x0c, "200K 800K" )
	PORT_DIPSETTING(    0x08, "200K only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, "Hit Difficulty" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Slide Show" )        PORT_DIPLOCATION("SW2:6") /* Listed as unused on manuals for both games */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW2")  /* 0x700019.b */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Balls" )         PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( hotpinbl )
	PORT_INCLUDE( galspnbl )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Launch Ball / Shake (Right Side)" )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+1*4, 2*4, 3*4, RGN_FRAC(1,2)+2*4, RGN_FRAC(1,2)+3*4,
			16*8+0*4, 16*8+1*4, 16*8+RGN_FRAC(1,2)+0*4, 16*8+RGN_FRAC(1,2)+1*4, 16*8+2*4, 16*8+3*4, 16*8+RGN_FRAC(1,2)+2*4, 16*8+RGN_FRAC(1,2)+3*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	32*8
};

static const gfx_layout spritelayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 8+0, 8+4, 8+RGN_FRAC(1,2)+0, 8+RGN_FRAC(1,2)+4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( galspnbl )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   512, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   0, 16 )
GFXDECODE_END


void galspnbl_state::machine_start()
{
}

static MACHINE_CONFIG_START( galspnbl, galspnbl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz) /* 12 MHz ??? - Use value from Temco's Super Pinball Action - NEEDS VERIFICATION!! */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galspnbl_state,  irq3_line_hold)/* also has vector for 6, but it does nothing */

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_4MHz)    /* 4 MHz ??? - Use value from Temco's Super Pinball Action - NEEDS VERIFICATION!! */
	MCFG_CPU_PROGRAM_MAP(audio_map)
								/* NMI is caused by the main CPU */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(galspnbl_state, screen_update_galspnbl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(galspnbl_state,galspnbl)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galspnbl)

	MCFG_PALETTE_ADD("palette", 1024 + 32768)
	MCFG_PALETTE_INIT_OWNER(galspnbl_state, galspnbl)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("spritegen", TECMO_SPRITE, 0)
	MCFG_TECMO_SPRITE_GFX_REGION(1)
	MCFG_TECMO_SPRITE_BOOTLEG(1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_4MHz) /* Use value from Super Pinball Action - NEEDS VERIFICATION!! */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_4MHz/4, OKIM6295_PIN7_HIGH) /* Use value from Super Pinball Action - clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galspnbl )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "7.rom",        0x000000, 0x80000, CRC(ce0189bf) SHA1(06d8cc6f5b819fe2ca536ce553db6986547a15ba) )
	ROM_LOAD16_BYTE( "3.rom",        0x000001, 0x80000, CRC(9b0a8744) SHA1(ac80f22b8b2f4c559c225bf203af698bf59699e7) )
	ROM_LOAD16_BYTE( "8.rom",        0x100000, 0x80000, CRC(eee2f087) SHA1(37285ae7b49c9d20ad92b3971db89ba593975154) )
	ROM_LOAD16_BYTE( "4.rom",        0x100001, 0x80000, CRC(56298489) SHA1(6dce296d752fa5579e8a3c1db3e563c4f98b6bae) )
	ROM_LOAD16_BYTE( "9.rom",        0x200000, 0x80000, CRC(d9e4964c) SHA1(0bb63384ed57a6ee1d45d1f2c181172a3f0b39df) )
	ROM_LOAD16_BYTE( "5.rom",        0x200001, 0x80000, CRC(a5e71ee4) SHA1(f41ea4ab07be263157d6c1f4a96ef45c9bccfe39) )
	ROM_LOAD16_BYTE( "10.rom",       0x300000, 0x80000, CRC(3a20e1e5) SHA1(850be621547bc9c7519055211392f2684e440462) )
	ROM_LOAD16_BYTE( "6.rom",        0x300001, 0x80000, CRC(94927d20) SHA1(0ea1a179956ad9a93a99cccec92f0490044ad1d3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "2.rom",        0x0000, 0x10000, CRC(fae688a7) SHA1(e1ef7abd18f6a820d1a7f0ceb9a9b1a2c7de41f0) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "17.rom",       0x00000, 0x40000, CRC(7d435701) SHA1(f6a2241c95f101d09b18f550689d125abd3ea9c4) )
	ROM_LOAD( "18.rom",       0x40000, 0x40000, CRC(136adaac) SHA1(5f5e70a66d256cad9fcdbc5a7fff035f9a3279b9) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "15.rom",       0x00000, 0x20000, CRC(4beb840d) SHA1(351cd8da361a55794595d2cf7b0fed9233d0a5a0) )
	ROM_LOAD( "16.rom",       0x20000, 0x20000, CRC(93d3c610) SHA1(0cf1f311ec2646a436c37e121634731646c06437) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "1.rom",        0x00000, 0x40000, CRC(93c06d3d) SHA1(8620d274ca7824e7e72a1ad1da3eaa804d550653) )
ROM_END

ROM_START( hotpinbl )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "hp_07.bin",    0x000000, 0x80000, CRC(978cc13e) SHA1(0060aaf7259fdeeacb07e9ced01bdf69c27bdfb6) )
	ROM_LOAD16_BYTE( "hp_03.bin",    0x000001, 0x80000, CRC(68388726) SHA1(d8dca9050403be70097a0f833ba189bd2fa87e80) )
	ROM_LOAD16_BYTE( "hp_08.bin",    0x100000, 0x80000, CRC(bd16be12) SHA1(36e64705efba8ecdc96a62f55d68e959022fb98f) )
	ROM_LOAD16_BYTE( "hp_04.bin",    0x100001, 0x80000, CRC(655b0cf0) SHA1(4348d774155c61b1a29c2618f472a0032f4be9c0) )
	ROM_LOAD16_BYTE( "hp_09.bin",    0x200000, 0x80000, CRC(a6368624) SHA1(fa0b3dc4eb33a4b2d00e30676e59feec2fa64c3d) )
	ROM_LOAD16_BYTE( "hp_05.bin",    0x200001, 0x80000, CRC(48efd028) SHA1(8c968c4a73069a8f64e7bc339bab0eeb5254c580) )
	ROM_LOAD16_BYTE( "hp_10.bin",    0x300000, 0x80000, CRC(a5c63e34) SHA1(de27cfe20e09e8c13ee28d6eb42bfab1ebe33149) )
	ROM_LOAD16_BYTE( "hp_06.bin",    0x300001, 0x80000, CRC(513eda91) SHA1(14a43c00ad1f55bff525a14cd53913dd78e80f0c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "hp_02.bin",    0x0000, 0x10000, CRC(82698269) SHA1(5e27e89f1bdd7c3793d40867c50981f5fac0a7fb) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "hp_13.bin",    0x00000, 0x40000, CRC(d53b64b9) SHA1(347b6ec908e23f848e98eed46fb34b49b728556b) )
	ROM_LOAD( "hp_14.bin",    0x40000, 0x40000, CRC(2fe3fcee) SHA1(29f96aa3dded6cb0b2fe3d9507fb5638e9778ef3) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "hp_11.bin",    0x00000, 0x20000, CRC(deecd7f1) SHA1(752c944d941bfe8f21d32881f32676999ebc5a7f) )
	ROM_LOAD( "hp_12.bin",    0x20000, 0x20000, CRC(5fd603c2) SHA1(864686cd1ba5beb6cebfd394b60620106c929abd) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "hp_01.bin",    0x00000, 0x40000, CRC(93c06d3d) SHA1(8620d274ca7824e7e72a1ad1da3eaa804d550653) )
ROM_END


GAME( 1995, hotpinbl, 0, galspnbl, hotpinbl, driver_device, 0, ROT90, "Comad & New Japan System", "Hot Pinball", MACHINE_SUPPORTS_SAVE )
GAME( 1996, galspnbl, 0, galspnbl, galspnbl, driver_device, 0, ROT90, "Comad", "Gals Pinball", MACHINE_SUPPORTS_SAVE )
