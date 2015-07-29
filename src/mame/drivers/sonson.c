// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

Son Son memory map (preliminary)

driver by Mirko Buffoni


MAIN CPU:

0000-0fff RAM
1000-13ff Video RAM
1400-17ff Color RAM
2020-207f Sprites
4000-ffff ROM

read:
3002      IN0
3003      IN1
3004      IN2
3005      DSW0
3006      DSW1

write:
3000      horizontal scroll
3008      watchdog reset
3018      flipscreen (inverted)
3010      command for the audio CPU
3019      trigger FIRQ on audio CPU


SOUND CPU:
0000-07ff RAM
e000-ffff ROM

read:
a000      command from the main CPU

write:
2000      8910 #1 control
2001      8910 #1 write
4000      8910 #2 control
4001      8910 #2 write

TODO:

- Fix Service Mode Output Test: press p1/p2 shot to insert coin
- Flip Screen DIP is noted in service manual and added to DIP LOCATIONS, but not working.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "includes/sonson.h"


WRITE8_MEMBER(sonson_state::sonson_sh_irqtrigger_w)
{
	data &= 1;

	if (m_last_irq == 0 && data == 1)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		m_audiocpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}

	m_last_irq = data;
}

WRITE8_MEMBER(sonson_state::sonson_coin1_counter_w)
{
	coin_counter_w(machine(), 0, data & 1);
}

WRITE8_MEMBER(sonson_state::sonson_coin2_counter_w)
{
	coin_counter_w(machine(), 1, data & 1);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, sonson_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(sonson_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1400, 0x17ff) AM_RAM_WRITE(sonson_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x2020, 0x207f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x3000, 0x3000) AM_WRITE(sonson_scrollx_w)
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("P1")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("P2")
	AM_RANGE(0x3004, 0x3004) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3005, 0x3005) AM_READ_PORT("DSW1")
	AM_RANGE(0x3006, 0x3006) AM_READ_PORT("DSW2")
	AM_RANGE(0x3008, 0x3008) AM_WRITENOP    // might be Y scroll, but the game always sets it to 0
	AM_RANGE(0x3010, 0x3010) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x3018, 0x3018) AM_WRITE(sonson_flipscreen_w)
	AM_RANGE(0x3019, 0x3019) AM_WRITE(sonson_sh_irqtrigger_w)
	AM_RANGE(0x301e, 0x301e) AM_WRITE(sonson_coin2_counter_w)
	AM_RANGE(0x301f, 0x301f) AM_WRITE(sonson_coin1_counter_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, sonson_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x4000, 0x4001) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( sonson )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage affects" ) PORT_DIPLOCATION("SW1:5")  /* Not documented in manual */
	PORT_DIPSETTING(    0x10, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Coin_B ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:7")
		PORT_DIPNAME( 0X80, 0x80, DEF_STR( Flip_Screen )) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, "2 Players Game" ) PORT_DIPLOCATION("SW2:3")  /* Not documented in manual */
	PORT_DIPSETTING(    0x04, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x08, "20000 80000 100000" )
	PORT_DIPSETTING(    0x00, "30000 90000 120000" )
	PORT_DIPSETTING(    0x18, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 8*16+7, 8*16+6, 8*16+5, 8*16+4, 8*16+3, 8*16+2, 8*16+1, 8*16+0,
			7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( sonson )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 64*4, 32 )
GFXDECODE_END




void sonson_state::machine_start()
{
	save_item(NAME(m_last_irq));
}

void sonson_state::machine_reset()
{
	m_last_irq = 0;
}

static MACHINE_CONFIG_START( sonson, sonson_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_12MHz/8)   /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sonson_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", M6809, XTAL_12MHz/8)  /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(sonson_state, irq0_line_hold, 4*60)    /* FIRQs are triggered by the main CPU */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.37)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sonson_state, screen_update_sonson)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sonson)

	MCFG_PALETTE_ADD("palette", 64*4+32*8)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(sonson_state, sonson)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8)   /* 1.5 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_12MHz/8)   /* 1.5 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sonson )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "ss.01e",       0x4000, 0x4000, CRC(cd40cc54) SHA1(4269586099638d31dd30381e94538701982e9f5a) )
	ROM_LOAD( "ss.02e",       0x8000, 0x4000, CRC(c3476527) SHA1(499b879a12b55443ec833e5a2819e9da20e3b033) )
	ROM_LOAD( "ss.03e",       0xc000, 0x4000, CRC(1fd0e729) SHA1(e04215b0c3d11ce844ab250ff3e1a845dd0b6c3e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ss_6.c11",     0xe000, 0x2000, CRC(1135c48a) SHA1(bfc10363fc42fb589088675a6e8e3d1668d8a6b8) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "ss_7.b6",      0x00000, 0x2000, CRC(990890b1) SHA1(0ae5da75e8ff013d32f2a6e3a015d5e1623fbb19) )   /* characters */
	ROM_LOAD( "ss_8.b5",      0x02000, 0x2000, CRC(9388ff82) SHA1(31ff5e61d062262754bbf6763d094495c1d2e838) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "ss_9.m5",      0x00000, 0x2000, CRC(8cb1cacf) SHA1(41b479dae84176ceb4eacb30b4dad58b7767606e) )   /* sprites */
	ROM_LOAD( "ss_10.m6",     0x02000, 0x2000, CRC(f802815e) SHA1(968145680483620cb0c9e7c00b4927aeace99e0c) )
	ROM_LOAD( "ss_11.m3",     0x04000, 0x2000, CRC(4dbad88a) SHA1(721612555714e116564d2b301cfa04980d21ad3b) )
	ROM_LOAD( "ss_12.m4",     0x06000, 0x2000, CRC(aa05e687) SHA1(4988d540e3deb9107f0448cd8ef47fa73ec926fe) )
	ROM_LOAD( "ss_13.m1",     0x08000, 0x2000, CRC(66119bfa) SHA1(73790be24287d8136c844b26cf36a679e489a37b) )
	ROM_LOAD( "ss_14.m2",     0x0a000, 0x2000, CRC(e14ef54e) SHA1(69ab42defff2cb91c6e07ea8805f64868a028630) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "ssb4.b2",      0x0000, 0x0020, CRC(c8eaf234) SHA1(d39dfab6dcad6b0a719c466b5290d2d081e4b58d) )    /* red/green component */
	ROM_LOAD( "ssb5.b1",      0x0020, 0x0020, CRC(0e434add) SHA1(238c281813d6079b9ae877bd0ced33abbbe39442) )    /* blue component */
	ROM_LOAD( "ssb2.c4",      0x0040, 0x0100, CRC(c53321c6) SHA1(439d98a98cdf2118b887c725a7759a98e2c377d9) )    /* character lookup table */
	ROM_LOAD( "ssb3.h7",      0x0140, 0x0100, CRC(7d2c324a) SHA1(3dcf09bd3f58bddb9760183d2c1b0fe5d77536ea) )    /* sprite lookup table */
	ROM_LOAD( "ssb1.k11",     0x0240, 0x0100, CRC(a04b0cfe) SHA1(89ab33c6b0aa313ebda2f11516cea667a9951a81) )    /* unknown (not used) */
ROM_END

ROM_START( sonsonj )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "ss_0.l9",      0x4000, 0x2000, CRC(705c168f) SHA1(28d3b186cd0b927d96664051fb759b64ecc18908) )
	ROM_LOAD( "ss_1.j9",      0x6000, 0x2000, CRC(0f03b57d) SHA1(7d14a88f43952d5c4df2951a5b62e399ba5ef37b) )
	ROM_LOAD( "ss_2.l8",      0x8000, 0x2000, CRC(a243a15d) SHA1(a736a163fbb20fa0e318f53ccf29d155b6f77781) )
	ROM_LOAD( "ss_3.j8",      0xa000, 0x2000, CRC(cb64681a) SHA1(f902e462df34016a28a5d7705294e31c9185135a) )
	ROM_LOAD( "ss_4.l7",      0xc000, 0x2000, CRC(4c3e9441) SHA1(4316bf4ada6598dd7a7b089f2720b1e1d59123be) )
	ROM_LOAD( "ss_5.j7",      0xe000, 0x2000, CRC(847f660c) SHA1(33fe54622765ca68992d22b2d62778a027db1719) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ss_6.c11",     0xe000, 0x2000, CRC(1135c48a) SHA1(bfc10363fc42fb589088675a6e8e3d1668d8a6b8) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "ss_7.b6",      0x00000, 0x2000, CRC(990890b1) SHA1(0ae5da75e8ff013d32f2a6e3a015d5e1623fbb19) )   /* characters */
	ROM_LOAD( "ss_8.b5",      0x02000, 0x2000, CRC(9388ff82) SHA1(31ff5e61d062262754bbf6763d094495c1d2e838) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "ss_9.m5",      0x00000, 0x2000, CRC(8cb1cacf) SHA1(41b479dae84176ceb4eacb30b4dad58b7767606e) )   /* sprites */
	ROM_LOAD( "ss_10.m6",     0x02000, 0x2000, CRC(f802815e) SHA1(968145680483620cb0c9e7c00b4927aeace99e0c) )
	ROM_LOAD( "ss_11.m3",     0x04000, 0x2000, CRC(4dbad88a) SHA1(721612555714e116564d2b301cfa04980d21ad3b) )
	ROM_LOAD( "ss_12.m4",     0x06000, 0x2000, CRC(aa05e687) SHA1(4988d540e3deb9107f0448cd8ef47fa73ec926fe) )
	ROM_LOAD( "ss_13.m1",     0x08000, 0x2000, CRC(66119bfa) SHA1(73790be24287d8136c844b26cf36a679e489a37b) )
	ROM_LOAD( "ss_14.m2",     0x0a000, 0x2000, CRC(e14ef54e) SHA1(69ab42defff2cb91c6e07ea8805f64868a028630) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "ssb4.b2",      0x0000, 0x0020, CRC(c8eaf234) SHA1(d39dfab6dcad6b0a719c466b5290d2d081e4b58d) )    /* red/green component */
	ROM_LOAD( "ssb5.b1",      0x0020, 0x0020, CRC(0e434add) SHA1(238c281813d6079b9ae877bd0ced33abbbe39442) )    /* blue component */
	ROM_LOAD( "ssb2.c4",      0x0040, 0x0100, CRC(c53321c6) SHA1(439d98a98cdf2118b887c725a7759a98e2c377d9) )    /* character lookup table */
	ROM_LOAD( "ssb3.h7",      0x0140, 0x0100, CRC(7d2c324a) SHA1(3dcf09bd3f58bddb9760183d2c1b0fe5d77536ea) )    /* sprite lookup table */
	ROM_LOAD( "ssb1.k11",     0x0240, 0x0100, CRC(a04b0cfe) SHA1(89ab33c6b0aa313ebda2f11516cea667a9951a81) )    /* unknown (not used) */
ROM_END


GAME( 1984, sonson,  0,      sonson, sonson, driver_device, 0, ROT0, "Capcom", "Son Son", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sonsonj, sonson, sonson, sonson, driver_device, 0, ROT0, "Capcom", "Son Son (Japan)", MACHINE_SUPPORTS_SAVE )
