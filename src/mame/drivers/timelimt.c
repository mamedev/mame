// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Time Limit (c) 1983 Chuo
Progress   (c) 1984 Chuo

driver by Ernesto Corvi

Notes:
- Sprite colors are wrong (missing colortable?)
- driver should probably be merged with suprridr.c and thepit.c

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/timelimt.h"

/***************************************************************************/


void timelimt_state::machine_start()
{
	save_item(NAME(m_nmi_enabled));
}

void timelimt_state::machine_reset()
{
	m_nmi_enabled = 0;
}

WRITE8_MEMBER(timelimt_state::nmi_enable_w)
{
	m_nmi_enabled = data & 1;   /* bit 0 = nmi enable */
}

WRITE8_MEMBER(timelimt_state::sound_reset_w)
{
	if (data & 1)
		m_audiocpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}

/***************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, timelimt_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM     /* rom */
	AM_RANGE(0x8000, 0x87ff) AM_RAM     /* ram */
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram") /* video ram */
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")/* background ram */
	AM_RANGE(0x9800, 0x98ff) AM_RAM AM_SHARE("spriteram")   /* sprite ram */
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("INPUTS")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW")
	AM_RANGE(0xb000, 0xb000) AM_WRITE(nmi_enable_w) /* nmi enable */
	AM_RANGE(0xb003, 0xb003) AM_WRITE(sound_reset_w)/* sound reset ? */
	AM_RANGE(0xb800, 0xb800) AM_WRITE(soundlatch_byte_w) /* sound write */
	AM_RANGE(0xb800, 0xb800) AM_READNOP     /* NMI ack? */
	AM_RANGE(0xc800, 0xc800) AM_WRITE(scroll_x_lsb_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITE(scroll_x_msb_w)
	AM_RANGE(0xc802, 0xc802) AM_WRITE(scroll_y_w)
	AM_RANGE(0xc803, 0xc803) AM_WRITENOP        /* ???? bit 0 used only */
	AM_RANGE(0xc804, 0xc804) AM_WRITENOP        /* ???? not used */
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, timelimt_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, timelimt_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3800, 0x3bff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, timelimt_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(soundlatch_clear_byte_w)
	AM_RANGE(0x8c, 0x8d) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x8e, 0x8f) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_data_w)
ADDRESS_MAP_END

/***************************************************************************/

static INPUT_PORTS_START( timelimt )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  /* probably bonus */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )  /* probably screen-flip */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( progress )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20,000 & 100,000" )
	PORT_DIPSETTING(    0x20, "50,000 & 200,000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) /* Manual shows "SCREEN" Table=On / Upright=Off */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2)+0, RGN_FRAC(0,2)+4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( timelimt )
	GFXDECODE_ENTRY( "tiles_1", 0, charlayout,   32, 1 )    /* seems correct */
	GFXDECODE_ENTRY( "tiles_2", 0, charlayout,    0, 1 )    /* seems correct */
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 64, 4 )    /* seems correct */
GFXDECODE_END

/***************************************************************************/

INTERRUPT_GEN_MEMBER(timelimt_state::irq)
{
	if ( m_nmi_enabled )
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/***************************************************************************/

static MACHINE_CONFIG_START( timelimt, timelimt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 5000000)   /* 5.000 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", timelimt_state,  irq)

	MCFG_CPU_ADD("audiocpu", Z80,18432000/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", timelimt_state,  irq0_line_hold) /* ? */

	MCFG_QUANTUM_TIME(attotime::from_hz(3000))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(timelimt_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", timelimt)
	MCFG_PALETTE_ADD("palette", 64+32)
	MCFG_PALETTE_INIT_OWNER(timelimt_state, timelimt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 18432000/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 18432000/12)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/***************************************************************************

  Game ROM(s)

***************************************************************************/

ROM_START( timelimt )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROMs */
	ROM_LOAD( "t8",     0x0000, 0x2000, CRC(006767ca) SHA1(a5d528c58cd73c0101ffa9ab783ec870668256db) )
	ROM_LOAD( "t7",     0x2000, 0x2000, CRC(cbe7cd86) SHA1(502a78c14c9717a466ea24cdc63da4c0f3bec1f9) )
	ROM_LOAD( "t6",     0x4000, 0x2000, CRC(f5f17e39) SHA1(7d78f551ce73276725c349703a790f2a63bb5503) )
	ROM_LOAD( "t9",     0x6000, 0x2000, CRC(2d72ab45) SHA1(01d4afacc01b9e7c49355123efd5f5ad4d79a9cd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* ROMs */
	ROM_LOAD( "tl5",    0x0000, 0x1000, CRC(5b782e4a) SHA1(2f4fe2beb8efa5a636fefc1ee172d0200d1c9497) )
	ROM_LOAD( "tl4",    0x1000, 0x1000, CRC(a32883a9) SHA1(26e1725b67be87db28855672facb1504b8ac84d6) )

	ROM_REGION( 0x2000, "tiles_1", 0 )  /* tiles */
	ROM_LOAD( "tl11",   0x0000, 0x1000, CRC(46676307) SHA1(38fe80722972b6b3ba32705469a0dcb868fb76a9) )
	ROM_LOAD( "tl10",   0x1000, 0x1000, CRC(2336908a) SHA1(345fc209ce891cc6f8f111c6d3a9e0f65ee6d818) )

	ROM_REGION( 0x2000, "tiles_2", 0 )  /* tiles */
	ROM_LOAD( "tl13",   0x0000, 0x1000, CRC(072e4053) SHA1(209edf7b371078e38d1c2812fa6a3d1a78193b3f) )
	ROM_LOAD( "tl12",   0x1000, 0x1000, CRC(ce960389) SHA1(57ee52cfa1b5a3832b362b38c8b7aa411dfc782b) )

	ROM_REGION( 0x6000, "sprites", 0 )  /* sprites */
	ROM_LOAD( "tl3",    0x0000, 0x2000, CRC(01a9fd95) SHA1(cd1078700c97a3539c9d9447c55efbd27540a1b3) )
	ROM_LOAD( "tl2",    0x2000, 0x2000, CRC(4693b849) SHA1(fbebedde53599fb1eaedc648bd704b321ab096b5) )
	ROM_LOAD( "tl1",    0x4000, 0x2000, CRC(c4007caf) SHA1(ae05af3319545d5ca98a046bfc100138a5a3ed96) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "clr.35", 0x0000, 0x0020, CRC(9c9e6073) SHA1(98496175bf19a8cdb0018705bc1a2193b8a782e1) )
	ROM_LOAD( "clr.48", 0x0020, 0x0020, BAD_DUMP CRC(a0bcac59) SHA1(e5832831b21981363509b79d89766757bd9273b0) ) /* FIXED BITS (xxxxxx1x) */
	ROM_LOAD( "clr.57", 0x0040, 0x0020, NO_DUMP )   /* missing sprite color prom? */
ROM_END

ROM_START( progress )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROMs */
	ROM_LOAD( "pg8.bin",     0x0000, 0x2000, CRC(e8779658) SHA1(3eca574d7328d54e544e663f58be789dbf151e77) )
	ROM_LOAD( "pg7.bin",     0x2000, 0x2000, CRC(5dcf6b6f) SHA1(550f02ff5ed2935f4c3c9055c5742fea46f42351) )
	ROM_LOAD( "pg6.bin",     0x4000, 0x2000, CRC(f21d2a08) SHA1(b2542e895d6d011895abec641b056ad8d7dc0d15) )
	ROM_LOAD( "pg9.bin",     0x6000, 0x2000, CRC(052ab4ac) SHA1(a2bfb575f2dfde862f9b1e8a4378f9b6b6200831) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* ROMs */
	ROM_LOAD( "pg4.bin",    0x0000, 0x1000, CRC(b1cc2fe8) SHA1(c9045e7b65311b052c337ad3bedadf108d1c24c3) )

	ROM_REGION( 0x2000, "tiles_1", 0 )  /* tiles */
	ROM_LOAD( "pg11.bin",   0x0000, 0x1000, CRC(bd8462e4) SHA1(91b1bd2d69aa1b1a84ee8e642b2c1131a7697dd9) )
	ROM_LOAD( "pg10.bin",   0x1000, 0x1000, CRC(c4bbf0b8) SHA1(d149eda9637474febdafd565a60eb2940702f162) )

	ROM_REGION( 0x2000, "tiles_2", 0 )  /* tiles */
	ROM_LOAD( "pg13.bin",   0x0000, 0x1000, CRC(25ec45be) SHA1(1271b7a5632934a82ccae35de8c2968247a233bb) )
	ROM_LOAD( "pg12.bin",   0x1000, 0x1000, CRC(c837c5f5) SHA1(dbfc0d8afe0a8e9dd213cb4095b23b7aa8e2b6f4) )

	ROM_REGION( 0x6000, "sprites", 0 )  /* sprites */
	ROM_LOAD( "pg1.bin",    0x0000, 0x2000, CRC(155c8f7f) SHA1(0d32ebceb9b2a0b3faf1f91b7a6800999889b331) )
	ROM_LOAD( "pg2.bin",    0x2000, 0x2000, CRC(a6ca4dfc) SHA1(4243c9ea98e365bf342cf928ff97cafb35cdc7b6) )
	ROM_LOAD( "pg3.bin",    0x4000, 0x2000, CRC(2b21c2fb) SHA1(8c95889a19057d32790c9ccddc0977980eddbd0e) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "35.bin", 0x0000, 0x0020, CRC(8c5ca730) SHA1(be2554e1aa4a74d976919e2c37bce5fb4d40352b) )
	ROM_LOAD( "48.bin", 0x0020, 0x0020, CRC(12dd62cd) SHA1(8322b02d73c3eb44b587f76daeaabe6beea58456) )
	ROM_LOAD( "57.bin", 0x0040, 0x0020, CRC(18455a79) SHA1(e4d64368560e3116a922588129f5f91a4c520f7d) )
ROM_END

GAME( 1983, timelimt, 0, timelimt, timelimt, driver_device, 0, ROT90, "Chuo Co. Ltd", "Time Limit", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, progress, 0, timelimt, progress, driver_device, 0, ROT90, "Chuo Co. Ltd", "Progress", MACHINE_SUPPORTS_SAVE )
