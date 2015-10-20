// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/****************************************************************************

    Gotya / The Hand driver by Zsolt Vasvari


TODO: Emulated sound

      Hitachi HD38880BP
              HD38882PA06

      I think HD38880 is a CPU/MCU, because the game just sends it a sound command (0-0x1a)

      couriersud:
         The chips above are speech synthesis chips. HD38880 is the main chip
         whereas HD38882 is an eprom interface. PARCOR based.
         http://www.freepatentsonline.com/4435832.html
         Datasheet lists no parcor coefficients

****************************************************************************/

/****************************************************************************
 About GotYa (from the board owner)

 I believe it is a prototype for several reasons.
 There were quite a few jumpers on the board, hand written labels with
 the dates on them. I also have the manual, the game name is clearly Got-Ya
 and is a Game-A-Tron game.  The game itself had a few flyers from GAT inside
 so I have a hard time believing it was a bootleg.

----

 so despite the fact that 'gotya' might look like its a bootleg of thehand,
 its more likely just a prototype / alternate version, its hard to tell
****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "includes/gotya.h"


static ADDRESS_MAP_START( gotya_map, AS_PROGRAM, 8, gotya_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("P1")
	AM_RANGE(0x6001, 0x6001) AM_READ_PORT("P2")
	AM_RANGE(0x6002, 0x6002) AM_READ_PORT("DSW")
	AM_RANGE(0x6004, 0x6004) AM_WRITE(gotya_video_control_w)
	AM_RANGE(0x6005, 0x6005) AM_WRITE(gotya_soundlatch_w)
	AM_RANGE(0x6006, 0x6006) AM_WRITEONLY AM_SHARE("scroll")
	AM_RANGE(0x6007, 0x6007) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(gotya_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(gotya_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd000, 0xd3df) AM_RAM AM_SHARE("videoram2")
	AM_RANGE(0xd3e0, 0xd3ff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END


static INPUT_PORTS_START( gotya )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Paper") PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Scissors") PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Rock") PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x10, 0x10, "Sound Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Paper") PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Scissors") PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Rock") PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Game Type" )         /* Manual Says:  Before main switch on: Test Pattern */
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )   /*                After main switch on: Endless game */
	PORT_DIPSETTING(    0x00, "Endless" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the bitplanes are packed in one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 characters */
	64,     /* 64 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the bitplanes are packed in one byte */
	{ 0, 1, 2, 3, 24*8+0, 24*8+1, 24*8+2, 24*8+3,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
		7*8,  6*8,  5*8,  4*8,  3*8,  2*8,  1*8,  0*8 },
	64*8    /* every char takes 64 consecutive bytes */
};

static GFXDECODE_START( gotya )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 16 )
GFXDECODE_END


static const char *const sample_names[] =
{                                               // Address triggered at
	"*thehand",
	"01",   /* game start tune */           // 075f
	"02",   /* coin in */                   // 0074
	"03",   /* eat dot */                   // 0e45
	"05",   /* eat dollar sign */           // 0e45

	"06",   /* door open */                 // 19e1
	"07",   /* door close */                // 1965

	"08",   /* theme song */                // 0821
	//"09"                                  // 1569

	/* one of these two is played after eating the last dot */
	"0a",   /* piccolo */                   // 17af
	"0b",   /* tune */                      // 17af

	//"0f"                                  // 08ee
	"10",   /* 'We're even. Bye Bye!' */    // 162a
	"11",   /* 'You got me!' */             // 1657
	"12",   /* 'You have lost out' */       // 085e

	"13",   /* 'Rock' */                    // 14de
	"14",   /* 'Scissors' */                // 14f3
	"15",   /* 'Paper' */                   // 1508

	/* one of these is played when going by the girl between levels */
	"16",   /* 'Very good!' */              // 194a
	"17",   /* 'Wonderful!' */              // 194a
	"18",   /* 'Come on!' */                // 194a
	"19",   /* 'I love you!' */             // 194a
	"1a",   /* 'See you again!' */          // 194a
	0
};

void gotya_state::machine_start()
{
	save_item(NAME(m_scroll_bit_8));
	save_item(NAME(m_theme_playing));
}

void gotya_state::machine_reset()
{
	m_scroll_bit_8 = 0;
	m_theme_playing = 0;
}

static MACHINE_CONFIG_START( gotya, gotya_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,18432000/6) /* 3.072 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(gotya_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gotya_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(36*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 36*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gotya_state, screen_update_gotya)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gotya)
	MCFG_PALETTE_ADD("palette", 16*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(gotya_state, gotya)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( thehand )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hand6.bin",  0x0000, 0x1000, CRC(a33b806c) SHA1(1e552af5362e7b003f55e78bb59589e1db55557c) )
	ROM_LOAD( "hand5.bin",  0x1000, 0x1000, CRC(89bcde82) SHA1(d074bb6a1975160eb533d5fd9289170a68209046) )
	ROM_LOAD( "hand4.bin",  0x2000, 0x1000, CRC(c6844a83) SHA1(84e220dce3f5ddee9dd0377f3bebdd4027fc9108) )
	ROM_LOAD( "gb-03.bin",  0x3000, 0x1000, CRC(f34d90ab) SHA1(bec5f6a34a273f308083a280f2b425d9c273c69b) )

	ROM_REGION( 0x1000,  "gfx1", 0 )    /* characters */
	ROM_LOAD( "hand12.bin", 0x0000, 0x1000, CRC(95773b46) SHA1(db8d7ace4eafd4c72edfeff6003ca6e96e0239b5) )

	ROM_REGION( 0x1000,  "gfx2", 0 )    /* sprites */
	ROM_LOAD( "gb-11.bin",  0x0000, 0x1000, CRC(5d5eca1b) SHA1(d7c6b5f4d398d5e33cc411ed593d6f53a9979493) )

	ROM_REGION( 0x0120,  "proms", 0 )
	ROM_LOAD( "prom.1a",    0x0000, 0x0020, CRC(4864a5a0) SHA1(5b49f60b085fa026d4e8d4a5ad28ee7037a8ff9c) )    /* color PROM */
	ROM_LOAD( "prom.4c",    0x0020, 0x0100, CRC(4745b5f6) SHA1(02a7f759e9bc8089cbd9213a71bbe671f9641638) )    /* lookup table */

	ROM_REGION( 0x1000,  "user1", 0 )       /* no idea what these are */
	ROM_LOAD( "hand1.bin",  0x0000, 0x0800, CRC(ccc537e0) SHA1(471fd49225aa14b91d085178e1b58b6c4ae76481) )
	ROM_LOAD( "gb-02.bin",  0x0800, 0x0800, CRC(65a7e284) SHA1(91e9c34dcf20608863ad5475dc0c4309971c8eee) )

	ROM_REGION( 0x8000,  "user2", 0 )       /* HD38880 code/samples? */
	ROM_LOAD( "gb-10.bin",  0x4000, 0x1000, CRC(8101915f) SHA1(c4d21b1938ea7e0d47c48e74037f005280ac101b) )
	ROM_LOAD( "gb-09.bin",  0x5000, 0x1000, CRC(619bba76) SHA1(2a2deffe6f058fc840329fbfffbc0c70a0147c14) )
	ROM_LOAD( "gb-08.bin",  0x6000, 0x1000, CRC(82f59528) SHA1(6bfa2329eb291040bfc229c56420865253b0132a) )
	ROM_LOAD( "hand7.bin",  0x7000, 0x1000, CRC(fbf1c5de) SHA1(dd3181a8da1972e3c997678bb868256a10f33d04) )
ROM_END

ROM_START( gotya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gb-06.bin",  0x0000, 0x1000, CRC(7793985a) SHA1(23aa8bd161e700bea59b92075423cdf55e9a26c3) )
	ROM_LOAD( "gb-05.bin",  0x1000, 0x1000, CRC(683d188b) SHA1(5341c62f5cf384c73be0d7a0a230bb8cebfbe709) )
	ROM_LOAD( "gb-04.bin",  0x2000, 0x1000, CRC(15b72f09) SHA1(bd941722ed1310d5c8ca8a44899368cba3815f3b) )
	ROM_LOAD( "gb-03.bin",  0x3000, 0x1000, CRC(f34d90ab) SHA1(bec5f6a34a273f308083a280f2b425d9c273c69b) )    /* this is the only ROM that passes the ROM test */

	ROM_REGION( 0x1000,  "gfx1", 0 )    /* characters */
	ROM_LOAD( "gb-12.bin",  0x0000, 0x1000, CRC(4993d735) SHA1(9e47876238a8af3659721191a5f75c33507ed1a5) )

	ROM_REGION( 0x1000,  "gfx2", 0 )    /* sprites */
	ROM_LOAD( "gb-11.bin",  0x0000, 0x1000, CRC(5d5eca1b) SHA1(d7c6b5f4d398d5e33cc411ed593d6f53a9979493) )

	ROM_REGION( 0x0120,  "proms", 0 )
	ROM_LOAD( "prom.1a",    0x0000, 0x0020, CRC(4864a5a0) SHA1(5b49f60b085fa026d4e8d4a5ad28ee7037a8ff9c) )    /* color PROM */
	ROM_LOAD( "prom.4c",    0x0020, 0x0100, CRC(4745b5f6) SHA1(02a7f759e9bc8089cbd9213a71bbe671f9641638) )    /* lookup table */

	ROM_REGION( 0x1000,  "user1", 0 )       /* no idea what these are */
	ROM_LOAD( "gb-01.bin",  0x0000, 0x0800, CRC(c31dba64) SHA1(15ae54b7d475ca3f0a3acc45cd8da2916c5fdef2) )
	ROM_LOAD( "gb-02.bin",  0x0800, 0x0800, CRC(65a7e284) SHA1(91e9c34dcf20608863ad5475dc0c4309971c8eee) )

	ROM_REGION( 0x8000,  "user2", 0 )       /* HD38880 code/samples? */
	ROM_LOAD( "gb-10.bin",  0x4000, 0x1000, CRC(8101915f) SHA1(c4d21b1938ea7e0d47c48e74037f005280ac101b) )
	ROM_LOAD( "gb-09.bin",  0x5000, 0x1000, CRC(619bba76) SHA1(2a2deffe6f058fc840329fbfffbc0c70a0147c14) )
	ROM_LOAD( "gb-08.bin",  0x6000, 0x1000, CRC(82f59528) SHA1(6bfa2329eb291040bfc229c56420865253b0132a) )
	ROM_LOAD( "gb-07.bin",  0x7000, 0x1000, CRC(92a9f8bf) SHA1(9231cd86f24f1e6a585c3a919add50c1f8e42a4c) )
ROM_END

GAME( 1981, thehand, 0,       gotya, gotya, driver_device, 0, ROT270, "T.I.C.",      "The Hand",                        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, gotya,   thehand, gotya, gotya, driver_device, 0, ROT270, "Game-A-Tron", "Got-Ya (12/24/1981, prototype?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
