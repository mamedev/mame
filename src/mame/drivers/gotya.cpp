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

 ----
According to Andrew Welburn:

'The Hand' is the original game, GAT licensed it for US manufacture.
It wasn't a runaway seller, they didn't make many, but they had to
change certain aspect of the game to 'localise' it and re-badge it as
their own. There are at least 3-4 scans of manuals online, if it was
proto, a full manual would be rare, and then having several independent
people find and scan the manual would be super rare.

The hand labelling of ROMs is entirely normal for small-run games. The
wire patching is on the underside and is minor tracking changes, common
on small-runs of PCBs.

The games themselves show the most obvious changes. The classic layout
at the top of the screen for 'The hand' with the 'hi-score' is replaced
with 'Got-Ya' in text, a fairly minor hack. The Copyright symbol is
still there in Got-Ya but with the company name scrubbed out.

All in all, Got-Ya should NOT be marked as a prototype in MAME, its a
US territory license hack of another game 'The Hand'. Nothing about it
says prototype, and the original base game is 'The Hand'.
****************************************************************************/

#include "emu.h"
#include "includes/gotya.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/samples.h"
#include "screen.h"
#include "speaker.h"


void gotya_state::gotya_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x5fff).ram();
	map(0x6000, 0x6000).portr("P1");
	map(0x6001, 0x6001).portr("P2");
	map(0x6002, 0x6002).portr("DSW");
	map(0x6004, 0x6004).w(FUNC(gotya_state::gotya_video_control_w));
	map(0x6005, 0x6005).w(FUNC(gotya_state::gotya_soundlatch_w));
	map(0x6006, 0x6006).writeonly().share("scroll");
	map(0x6007, 0x6007).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xc000, 0xc7ff).ram().w(FUNC(gotya_state::gotya_videoram_w)).share("videoram");
	map(0xc800, 0xcfff).ram().w(FUNC(gotya_state::gotya_colorram_w)).share("colorram");
	map(0xd000, 0xd3df).ram().share("videoram2");
	map(0xd3e0, 0xd3ff).ram().share("spriteram");
}


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

static GFXDECODE_START( gfx_gotya )
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
	nullptr
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

void gotya_state::gotya(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18432000/6); /* 3.072 MHz ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &gotya_state::gotya_map);
	m_maincpu->set_vblank_int("screen", FUNC(gotya_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(36*8, 32*8);
	screen.set_visarea(0, 36*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gotya_state::screen_update_gotya));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gotya);
	PALETTE(config, m_palette, FUNC(gotya_state::gotya_palette), 16*4, 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}

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

GAME( 1981, thehand, 0,       gotya, gotya, gotya_state, empty_init, ROT270, "T.I.C.",      "The Hand",            MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, gotya,   thehand, gotya, gotya, gotya_state, empty_init, ROT270, "Game-A-Tron", "Got-Ya (12/24/1981)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
