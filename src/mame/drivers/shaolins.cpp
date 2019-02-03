// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

Shaolin's Road

driver by Allard van der Bas

***************************************************************************/

#include "emu.h"
#include "includes/shaolins.h"

#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK XTAL(18'432'000)

TIMER_DEVICE_CALLBACK_MEMBER(shaolins_state::interrupt)
{
	int scanline = param;

	if(scanline == 240)
			m_maincpu->set_input_line(0, HOLD_LINE);
	else if((scanline % 32) == 0)
		if (m_nmi_enable & 0x02) m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



void shaolins_state::shaolins_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(shaolins_state::nmi_w));   /* bit 0 = flip screen, bit 1 = nmi enable, bit 2 = ? */
														/* bit 3, bit 4 = coin counters */
	map(0x0100, 0x0100).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x0300, 0x0300).w("sn1", FUNC(sn76489a_device::write)); /* trigger chip to read from latch. The program always */
	map(0x0400, 0x0400).w("sn2", FUNC(sn76489a_device::write)); /* writes the same number as the latch, so we don't */
															/* bother emulating them. */
	map(0x0500, 0x0500).portr("DSW1");
	map(0x0600, 0x0600).portr("DSW2");
	map(0x0700, 0x0700).portr("SYSTEM");
	map(0x0701, 0x0701).portr("P1");
	map(0x0702, 0x0702).portr("P2");
	map(0x0703, 0x0703).portr("DSW3");
	map(0x0800, 0x0800).nopw();                    /* latch for 76496 #0 */
	map(0x1000, 0x1000).nopw();                    /* latch for 76496 #1 */
	map(0x1800, 0x1800).w(FUNC(shaolins_state::palettebank_w));
	map(0x2000, 0x2000).w(FUNC(shaolins_state::scroll_w));
	map(0x2800, 0x2bff).ram();                         /* RAM BANK 2 */
	map(0x3000, 0x30ff).ram();                         /* RAM BANK 1 */
	map(0x3100, 0x33ff).ram().share("spriteram");
	map(0x3800, 0x3bff).ram().w(FUNC(shaolins_state::colorram_w)).share("colorram");
	map(0x3c00, 0x3fff).ram().w(FUNC(shaolins_state::videoram_w)).share("videoram");
	map(0x4000, 0x5fff).rom();                         /* Machine checks for extra rom */
	map(0x6000, 0xffff).rom();
}


static INPUT_PORTS_START( shaolins )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "30000 and every 70000" )
	PORT_DIPSETTING(    0x10, "40000 and every 80000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

		/* This bank only have four switches */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
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
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	512,    /* 512 characters */
	4,  /* 4 bits per pixel */
	{ 512*16*8+4, 512*16*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	4,  /* 4 bits per pixel */
	{ 256*64*8+4, 256*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( gfx_shaolins )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 16*8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*8*16, 16*8 )
GFXDECODE_END


MACHINE_CONFIG_START(shaolins_state::shaolins)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", MC6809E, MASTER_CLOCK/12)        /* verified on pcb */
	MCFG_DEVICE_PROGRAM_MAP(shaolins_map)
	TIMER(config, "scantimer").configure_scanline(FUNC(shaolins_state::interrupt), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(shaolins_state, screen_update)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shaolins);
	PALETTE(config, m_palette, FUNC(shaolins_state::shaolins_palette), 16*8*16+16*8*16, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("sn1", SN76489A, MASTER_CLOCK/12)        /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_ADD("sn2", SN76489A, MASTER_CLOCK/6)        /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

#if 0 // a bootleg board was found with downgraded sound hardware, but is otherwise the same
static MACHINE_CONFIG_START( shaolinb )
	shaolins(config);

	MCFG_DEVICE_REPLACE("sn1", SN76489, MASTER_CLOCK/12) /* only type verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_REPLACE("sn2", SN76489, MASTER_CLOCK/6)  /* only type verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
#endif

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kicker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "477-l03.d9",   0x6000, 0x2000, CRC(2598dfdd) SHA1(70a9d81b73bbd4ff6b627a3e4102d5328a946d20) )
	ROM_LOAD( "477-l04.d10",  0x8000, 0x4000, CRC(0cf0351a) SHA1(a9da783b29a63a46912a29715e8d11dc4cd22265) )
	ROM_LOAD( "477-l05.d11",  0xC000, 0x4000, CRC(654037f8) SHA1(52d098386fe87ae97d4dfefab0bd3a902f66d70b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "477-k06.a10",  0x0000, 0x2000, CRC(4d156afc) SHA1(29eb66e2ebcf2f1c1d5ece5413d1ebf54663f9cf) )
	ROM_LOAD( "477-k07.a11",  0x2000, 0x2000, CRC(ff6ca5df) SHA1(dfcd445c8b233a0a4168eb249472e53784eda25d) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "477-k02.h15",  0x0000, 0x4000, CRC(b94e645b) SHA1(65ae48134a0fe1e910a787714f7ae721734ded5b) )
	ROM_LOAD( "477-k01.h14",  0x4000, 0x4000, CRC(61bbf797) SHA1(97d276099172975499f646f381a6fc587c022435) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "477j10.a12",   0x0000, 0x0100, CRC(b09db4b4) SHA1(d21176cdc7def760da109083eb52e5b6a515021f) ) /* palette red component */
	ROM_LOAD( "477j11.a13",   0x0100, 0x0100, CRC(270a2bf3) SHA1(c0aec04bd3bceccddf5f5a814a560a893b29ef6b) ) /* palette green component */
	ROM_LOAD( "477j12.a14",   0x0200, 0x0100, CRC(83e95ea8) SHA1(e0bfa20600488f5c66233e13ea6ad857f62acb7c) ) /* palette blue component */
	ROM_LOAD( "477j09.b8",    0x0300, 0x0100, CRC(aa900724) SHA1(c5343273d0a7101b8ba6876c4f22e43d77610c75) ) /* character lookup table */
	ROM_LOAD( "477j08.f16",   0x0400, 0x0100, CRC(80009cf5) SHA1(a367f3f55d75a9d5bf4d43f9d77272eb910a1344) ) /* sprite lookup table */
ROM_END

ROM_START( shaolins )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "477-l03.d9",   0x6000, 0x2000, CRC(2598dfdd) SHA1(70a9d81b73bbd4ff6b627a3e4102d5328a946d20) )
	ROM_LOAD( "477-l04.d10",  0x8000, 0x4000, CRC(0cf0351a) SHA1(a9da783b29a63a46912a29715e8d11dc4cd22265) )
	ROM_LOAD( "477-l05.d11",  0xC000, 0x4000, CRC(654037f8) SHA1(52d098386fe87ae97d4dfefab0bd3a902f66d70b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "shaolins.a10", 0x0000, 0x2000, CRC(ff18a7ed) SHA1(f28bfeff84bb6a08a8bee999a0b7a19e09a8dfc3) ) /* proper Konami rom labels unknown */
	ROM_LOAD( "shaolins.a11", 0x2000, 0x2000, CRC(5f53ae61) SHA1(ad29e2255855c503295c6b63eb4cd6700a1e3f0e) ) /* proper Konami rom labels unknown */

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "477-k02.h15",  0x0000, 0x4000, CRC(b94e645b) SHA1(65ae48134a0fe1e910a787714f7ae721734ded5b) )
	ROM_LOAD( "477-k01.h14",  0x4000, 0x4000, CRC(61bbf797) SHA1(97d276099172975499f646f381a6fc587c022435) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "477j10.a12",   0x0000, 0x0100, CRC(b09db4b4) SHA1(d21176cdc7def760da109083eb52e5b6a515021f) ) /* palette red component */
	ROM_LOAD( "477j11.a13",   0x0100, 0x0100, CRC(270a2bf3) SHA1(c0aec04bd3bceccddf5f5a814a560a893b29ef6b) ) /* palette green component */
	ROM_LOAD( "477j12.a14",   0x0200, 0x0100, CRC(83e95ea8) SHA1(e0bfa20600488f5c66233e13ea6ad857f62acb7c) ) /* palette blue component */
	ROM_LOAD( "477j09.b8",    0x0300, 0x0100, CRC(aa900724) SHA1(c5343273d0a7101b8ba6876c4f22e43d77610c75) ) /* character lookup table */
	ROM_LOAD( "477j08.f16",   0x0400, 0x0100, CRC(80009cf5) SHA1(a367f3f55d75a9d5bf4d43f9d77272eb910a1344) ) /* sprite lookup table */
ROM_END

/*
    Shao-lin's Road (Bootleg) - has also been found on an original board

    Main Board:    VWXYZ
    Daughterboard: QSTU (Replaces 3 custom Konami chips)
    Daughterboard: RSTU (Replaces 4 custom Konami chips)

    All the roms/proms are located on the main board.

    Board Layout with edge connector on the left.  ROM's/PROM's are marked too.
    PROM's have an asterick suffix to distinguish them from the ROM's.

         A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
    |-------------------------------------------------|
    |                                                 |
    |                            6  7  3* 4* 5*       | 1
    |                                                 |
    |                      2*                         | 2
    |--|                                              |
       |                                              | 3
    |--|                                              |
    |                         3  4  5                 | 4
    |                                                 |
    |                                                 | 5
    |--|                                              |
       |                                           1* | 6
    |--|                                              |
    |                                                 | 7
    |                                                 |
    |                                        1  2     | 8
    |                                                 |
    |-------------------------------------------------|
*/

ROM_START( shaolinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.h4", 0x6000, 0x2000, CRC(2598dfdd) SHA1(70a9d81b73bbd4ff6b627a3e4102d5328a946d20) ) /* 2764 */
	ROM_LOAD( "4.i4", 0x8000, 0x4000, CRC(0cf0351a) SHA1(a9da783b29a63a46912a29715e8d11dc4cd22265) ) /* 27128 */
	ROM_LOAD( "5.j4", 0xC000, 0x4000, CRC(654037f8) SHA1(52d098386fe87ae97d4dfefab0bd3a902f66d70b) ) /* 27128 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "6.i1", 0x0000, 0x2000, CRC(ff18a7ed) SHA1(f28bfeff84bb6a08a8bee999a0b7a19e09a8dfc3) ) /* 2764 */
	ROM_LOAD( "7.j1", 0x2000, 0x4000, CRC(d9a7cff6) SHA1(47244426b9a674326c5303347112aa9d33bcf1df) ) /* 27128 */

	ROM_REGION( 0x8000, "gfx2", 0 ) /* All roms are 27128 */
	ROM_LOAD( "2.m8", 0x0000, 0x4000, CRC(560521c7) SHA1(f8a50c66364995041e29ed7be2e4ea1ad16aa735) )
	ROM_LOAD( "1.l8", 0x4000, 0x4000, CRC(a79959b2) SHA1(9c58975c55f7be32add0dccb259d9680410fa9bc) )

	ROM_REGION( 0x0500, "proms", 0 ) /* All proms are N82S129N */
	ROM_LOAD( "3.k1", 0x0000, 0x0100, CRC(b09db4b4) SHA1(d21176cdc7def760da109083eb52e5b6a515021f) ) /* palette red component */
	ROM_LOAD( "4.l1", 0x0100, 0x0100, CRC(270a2bf3) SHA1(c0aec04bd3bceccddf5f5a814a560a893b29ef6b) ) /* palette green component */
	ROM_LOAD( "5.m1", 0x0200, 0x0100, CRC(83e95ea8) SHA1(e0bfa20600488f5c66233e13ea6ad857f62acb7c) ) /* palette blue component */
	ROM_LOAD( "2.g2", 0x0300, 0x0100, CRC(aa900724) SHA1(c5343273d0a7101b8ba6876c4f22e43d77610c75) ) /* character lookup table */
	ROM_LOAD( "1.o6", 0x0400, 0x0100, CRC(80009cf5) SHA1(a367f3f55d75a9d5bf4d43f9d77272eb910a1344) ) /* sprite lookup table */
ROM_END


/*    YEAR, NAME,     PARENT, MACHINE,  INPUT,    STATE,          INIT,       MONITOR, COMPANY,  FULLNAME,                  FLAGS */
GAME( 1985, kicker,   0,      shaolins, shaolins, shaolins_state, empty_init, ROT90,   "Konami",  "Kicker",                  MACHINE_SUPPORTS_SAVE )
GAME( 1985, shaolins, kicker, shaolins, shaolins, shaolins_state, empty_init, ROT90,   "Konami",  "Shao-lin's Road (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, shaolinb, kicker, shaolins, shaolins, shaolins_state, empty_init, ROT90,   "Konami",  "Shao-lin's Road (set 2)", MACHINE_SUPPORTS_SAVE )
