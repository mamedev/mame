// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

Solomon's Key

driver by Mirko Buffoni

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/solomon.h"

WRITE8_MEMBER(solomon_state::solomon_sh_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/* this is checked on the title screen and when you reach certain scores in the game
   it could be a form of protection.  the real board needs to be analysed to find out
   what really lives here */

READ8_MEMBER(solomon_state::solomon_0xe603_r)
{
	if (space.device().safe_pc() == 0x161) // all the time .. return 0 to act as before  for coin / startup etc.
	{
		return 0;
	}
	else if (space.device().safe_pc() == 0x4cf0) // stop it clearing the screen at certain scores
	{
		return (space.device().state().state_int(Z80_BC) & 0x08);
	}
	else
	{
		osd_printf_debug("unhandled solomon_0xe603_r %04x\n", space.device().safe_pc());
		return 0;
	}
}

WRITE8_MEMBER(solomon_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, solomon_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(solomon_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(solomon_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(solomon_colorram2_w) AM_SHARE("colorram2")
	AM_RANGE(0xdc00, 0xdfff) AM_RAM_WRITE(solomon_videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xe000, 0xe07f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe400, 0xe5ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe600, 0xe600) AM_READ_PORT("P1")
	AM_RANGE(0xe601, 0xe601) AM_READ_PORT("P2")
	AM_RANGE(0xe602, 0xe602) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe603, 0xe603) AM_READ(solomon_0xe603_r)
	AM_RANGE(0xe604, 0xe604) AM_READ_PORT("DSW1")
	AM_RANGE(0xe605, 0xe605) AM_READ_PORT("DSW2")
	AM_RANGE(0xe606, 0xe606) AM_READNOP /* watchdog? */
	AM_RANGE(0xe600, 0xe600) AM_WRITE(nmi_mask_w)
	AM_RANGE(0xe604, 0xe604) AM_WRITE(solomon_flipscreen_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(solomon_sh_command_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, solomon_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xffff, 0xffff) AM_WRITENOP    /* watchdog? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, solomon_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x11) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x20, 0x21) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x30, 0x31) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( solomon )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x0c, 0x00, "Timer Speed" )       PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Faster" )
	PORT_DIPSETTING(    0x0c, "Fastest" )
	PORT_DIPNAME( 0x10, 0x00, "Extra" )         PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "30k 200k 500k" )
	PORT_DIPSETTING(    0x80, "100k 300k 800k" )
	PORT_DIPSETTING(    0x40, "30k 200k" )
	PORT_DIPSETTING(    0xc0, "100k 300k" )
	PORT_DIPSETTING(    0x20, "30k" )
	PORT_DIPSETTING(    0xa0, "100k" )
	PORT_DIPSETTING(    0x60, "200k" )
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	2048,   /* 2048 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 8*8 sprites */
	512,    /* 512 sprites */
	4,      /* 4 bits per pixel */
	{ 0, 512*32*8, 2*512*32*8, 3*512*32*8 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,   /* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( solomon )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )  /* colors   0-127 */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   128, 8 )  /* colors 128-255 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   0, 8 )  /* colors   0-127 */
GFXDECODE_END

INTERRUPT_GEN_MEMBER(solomon_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}



static MACHINE_CONFIG_START( solomon, solomon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4.0 MHz (?????) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", solomon_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, 3072000)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(solomon_state, irq0_line_hold, 2*60)   /* ??? */
						/* NMIs are caused by the main CPU */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(solomon_state, screen_update_solomon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", solomon)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ay3", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( solomon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6.3f",  0x00000, 0x4000, CRC(645eb0f3) SHA1(b911aa157ad94aa539dbe329a197d8902860c809) )
	ROM_LOAD( "7.3h",  0x08000, 0x4000, CRC(1bf5c482) SHA1(3b6a8dde72cddf95438539c5dc4622c95f932a04) )
	ROM_CONTINUE(      0x04000, 0x4000 )
	ROM_LOAD( "8.3jk", 0x0f000, 0x1000, CRC(0a6cdefc) SHA1(101acaa19b779cb8b4fffddbe63fe011c7d4b6e9) )
	ROM_IGNORE(                 0x7000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.3jk",  0x0000, 0x4000, CRC(fa6e562e) SHA1(713036c0a80b623086aa674bb5f8a135b6fedb01) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "12.3t",  0x00000, 0x08000, CRC(b371291c) SHA1(27302898c64330870c47025e61bd5acbd9483865) )    /* characters */
	ROM_LOAD( "11.3r",  0x08000, 0x08000, CRC(6f94d2af) SHA1(2e070c0fd5b9d7eb9b7e0d53f25b1a5063ef3095) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "10.3p",  0x00000, 0x08000, CRC(8310c2a1) SHA1(8cc87ab8faacdb1973791d207bb25ea9b444b66d) )
	ROM_LOAD( "9.3m",   0x08000, 0x08000, CRC(ab7e6c42) SHA1(0fc3b4a0bd2b17b79e2d1f7d4fe445c09ce4e730) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "2.5lm",  0x00000, 0x04000, CRC(80fa2be3) SHA1(8e7a78186473a6b5c42577ac9e4591ee2d1151f2) )    /* sprites */
	ROM_LOAD( "3.6lm",  0x04000, 0x04000, CRC(236106b4) SHA1(8eaf3150568c407bd8dc1cdf874b8417e5cca3d2) )
	ROM_LOAD( "4.7lm",  0x08000, 0x04000, CRC(088fe5d9) SHA1(e29ffb9fcff50ce982d5e502e10a8e29a4c47390) )
	ROM_LOAD( "5.8lm",  0x0c000, 0x04000, CRC(8366232a) SHA1(1c7a01dab056ec7d787a6f55772b9fa6fe67305a) )
ROM_END


ROM_START( solomonj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "slmn_06.bin",  0x00000, 0x4000, CRC(e4d421ff) SHA1(9599fa6e2d42bf0cfe77d62c6b162f56eae5efff) )
	ROM_LOAD( "slmn_07.bin",  0x08000, 0x4000, CRC(d52d7e38) SHA1(8439eeeedd1e47d2b9719a05c85a05283c11d7a8) )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_LOAD( "slmn_08.bin",  0x0f000, 0x1000, CRC(b924d162) SHA1(6299b791ec874bc3ef0424b277ec8a736c8cdd9a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "slmn_01.bin",  0x0000, 0x4000, CRC(fa6e562e) SHA1(713036c0a80b623086aa674bb5f8a135b6fedb01) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "slmn_12.bin",  0x00000, 0x08000, CRC(aa26dfcb) SHA1(71748eaceeca878ae9f871e30d5951ca4dde37d6) )  /* characters */
	ROM_LOAD( "slmn_11.bin",  0x08000, 0x08000, CRC(6f94d2af) SHA1(2e070c0fd5b9d7eb9b7e0d53f25b1a5063ef3095) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "slmn_10.bin",  0x00000, 0x08000, CRC(8310c2a1) SHA1(8cc87ab8faacdb1973791d207bb25ea9b444b66d) )
	ROM_LOAD( "slmn_09.bin",  0x08000, 0x08000, CRC(ab7e6c42) SHA1(0fc3b4a0bd2b17b79e2d1f7d4fe445c09ce4e730) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "slmn_02.bin",  0x00000, 0x04000, CRC(80fa2be3) SHA1(8e7a78186473a6b5c42577ac9e4591ee2d1151f2) )  /* sprites */
	ROM_LOAD( "slmn_03.bin",  0x04000, 0x04000, CRC(236106b4) SHA1(8eaf3150568c407bd8dc1cdf874b8417e5cca3d2) )
	ROM_LOAD( "slmn_04.bin",  0x08000, 0x04000, CRC(088fe5d9) SHA1(e29ffb9fcff50ce982d5e502e10a8e29a4c47390) )
	ROM_LOAD( "slmn_05.bin",  0x0c000, 0x04000, CRC(8366232a) SHA1(1c7a01dab056ec7d787a6f55772b9fa6fe67305a) )
ROM_END



GAME( 1986, solomon,  0,       solomon, solomon, driver_device, 0, ROT0, "Tecmo", "Solomon's Key (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, solomonj, solomon, solomon, solomon, driver_device, 0, ROT0, "Tecmo", "Solomon no Kagi (Japan)", MACHINE_SUPPORTS_SAVE )
