// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Taito Super Rider driver

    driver by Aaron Giles

    Games supported:
        * Super Rider

    Known issues:
        * clocks on sound chips and CPU not verified yet
        * the board seems to contain a discrete sound portion

    Questions:
        * there appears to be a second color bank for the tilemaps, where
          is it used, and how is it activated (if at all)?
        * what are the writes to $08DB and $08E8 for?
          (guess: a discrete sound effect)

    ** driver should probably be merged with timelimt.c

****************************************************************************

    PCB Layout
    ----------

    Top board

    REF. SR-8327A-B
    |----------------------------------------------------------------|
    |                 SR-11                                          |
    |                                                                |
    |          DIPSW  SR-10                                          |
    |                                                                |
    |                                                                |
    |                                                                |
    |                 NE555                                          |
    |                                     NEC-D780C                  |
    |  AY-3-8910                                                     |
    |                                                                |
    |  AY-3-8910                                                     |
    |                 SR-09           HM6116   SR-06    SR-03        |
    |  NEC-D780C                                                     |
    |                                 SR-08    SR-05    SR-02        |
    |                                                                |
    |                       ?.000MHz  SR-07    SR-04    SR-01        |
    |----------------------------------------------------------------|


    Bottom board

    REF. SR-8327B-B
    |----------------------------------------------------------------|
    |                                                                |
    |    SR-12                                                       |
    |                                                                |
    |    SR-13                                                       |
    |                                                                |
    |    SR-14                                                       |
    |                                        SR-15                   |
    |                                                                |
    |                                        SR-16                   |
    |                                                                |
    |                                        SR-17                   |
    |                                                                |
    |                                       M58725P                  |
    |                                                                |
    |                                                                |
    |    18.432MHz                                                   |
    |----------------------------------------------------------------|


    Epoxy module (exact layout unknown)

    REF. ???
    |-------------------------------|
    |                               |
    |      1        2        3      |
    |                               |
    |-------------------------------|

***************************************************************************/

#include "emu.h"
#include "includes/suprridr.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


void suprridr_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
}

void suprridr_state::machine_reset()
{
	m_soundlatch->acknowledge_w();
}

/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

WRITE8_MEMBER(suprridr_state::nmi_enable_w)
{
	m_nmi_enable = data;
}


INTERRUPT_GEN_MEMBER(suprridr_state::main_nmi_gen)
{
	if (m_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



/*************************************
 *
 *  Misc handlers
 *
 *************************************/

WRITE8_MEMBER(suprridr_state::coin_lock_w)
{
	/* cleared when 9 credits are hit, but never reset! */
/*  machine().bookkeeping().coin_lockout_global_w(~data & 1); */
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void suprridr_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(suprridr_state::bgram_w)).share("bgram");
	map(0x9000, 0x97ff).ram().w(FUNC(suprridr_state::fgram_w)).share("fgram");
	map(0x9800, 0x983f).ram();
	map(0x9840, 0x987f).ram().share("spriteram");
	map(0x9880, 0x9bff).ram();
	map(0xa000, 0xa000).portr("INPUTS");
	map(0xa800, 0xa800).portr("SYSTEM");
	map(0xb000, 0xb000).portr("DSW").w(FUNC(suprridr_state::nmi_enable_w));
	map(0xb002, 0xb003).w(FUNC(suprridr_state::coin_lock_w));
	map(0xb006, 0xb006).w(FUNC(suprridr_state::flipx_w));
	map(0xb007, 0xb007).w(FUNC(suprridr_state::flipy_w));
	map(0xb800, 0xb800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc801, 0xc801).w(FUNC(suprridr_state::fgdisable_w));
	map(0xc802, 0xc802).w(FUNC(suprridr_state::fgscrolly_w));
	map(0xc804, 0xc804).w(FUNC(suprridr_state::bgscrolly_w));
	map(0xc000, 0xefff).rom();
}


void suprridr_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void suprridr_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x3800, 0x3bff).ram();
}


void suprridr_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
	map(0x8c, 0x8d).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x8d, 0x8d).r("ay1", FUNC(ay8910_device::data_r));
	map(0x8e, 0x8f).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x8f, 0x8f).r("ay2", FUNC(ay8910_device::data_r));
}




/*************************************
 *
 *  Port definitions and helpers
 *
 *************************************/

#define SUPRRIDR_P1_CONTROL_PORT_TAG    ("CONTP1")
#define SUPRRIDR_P2_CONTROL_PORT_TAG    ("CONTP2")

CUSTOM_INPUT_MEMBER(suprridr_state::control_r)
{
	uint32_t ret;

	/* screen flip multiplexes controls */
	if (is_screen_flipped())
		ret = ioport(SUPRRIDR_P2_CONTROL_PORT_TAG)->read();
	else
		ret = ioport(SUPRRIDR_P1_CONTROL_PORT_TAG)->read();

	return ret;
}


static INPUT_PORTS_START( suprridr )
	PORT_START("INPUTS")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, suprridr_state, control_r, nullptr)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

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
	PORT_DIPNAME( 0x18, 0x00, "Timer Speed" )
	PORT_DIPSETTING(    0x18, "Slow" )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus" )
	PORT_DIPSETTING(    0x00, "200k" )
	PORT_DIPSETTING(    0x20, "400k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Invulnerability?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START(SUPRRIDR_P1_CONTROL_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START(SUPRRIDR_P2_CONTROL_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2),RGN_FRAC(1,2)+4, 0,4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,16) },
	8*8*2
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};


static GFXDECODE_START( gfx_suprridr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   32, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 64, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void suprridr_state::suprridr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(49'152'000)/16);     /* 3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &suprridr_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &suprridr_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(suprridr_state::main_nmi_gen));

	Z80(config, m_audiocpu, 10000000/4);       /* 2.5 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &suprridr_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &suprridr_state::sound_portmap);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(suprridr_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suprridr);
	PALETTE(config, m_palette, FUNC(suprridr_state::suprridr_palette), 96);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", XTAL(49'152'000)/32).add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(49'152'000)/32));
	ay2.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);
	m_soundlatch->set_separate_acknowledge(true);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( suprridr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sr8",    0x0000, 0x1000, CRC(4a1f0a6c) SHA1(cabdeafa3b9828d7a6e056fb037abb90484bb33a) )
	ROM_LOAD( "sr7",    0x1000, 0x1000, CRC(523ee717) SHA1(dd2a53a56b0f29b4d02c4207a7260b345cab0074) )
	ROM_LOAD( "sr4",    0x2000, 0x1000, CRC(300370ae) SHA1(bf43d800e1b2a5353625c1012d22df6419292d7d) )
	ROM_LOAD( "sr5",    0x3000, 0x1000, CRC(c5bca683) SHA1(4ebb1eb9dc72128286d60fce8b5c323adb25d332) )
	ROM_LOAD( "sr6",    0x4000, 0x1000, CRC(563bab28) SHA1(47dd5de9826360ccdf2df6866b0799a0390dd939) )
	ROM_LOAD( "sr3",    0x5000, 0x1000, CRC(4b9d2ec5) SHA1(773d53be5a3797c6c16ea8260f03c8e8272b2c32) )
	ROM_LOAD( "sr2",    0x6000, 0x1000, CRC(6fe18e1d) SHA1(9b247d2ab7bfddaa3cfdb5f034100881317e09a8) )
	ROM_LOAD( "sr1",    0x7000, 0x1000, CRC(f2ae64b3) SHA1(fd1878c7f1554e257a190084950a3bcf4b68a28e) )
	ROM_LOAD( "1",      0xc000, 0x1000, CRC(caf12fa2) SHA1(ff3f68cfb7817841cff1de6f78c9ee3d57b12db6) )
	ROM_LOAD( "2",      0xd000, 0x1000, CRC(2b3c638e) SHA1(af397cc9137888ccc503aff1b3554744a2327a4c) )
	ROM_LOAD( "3",      0xe000, 0x1000, CRC(2abdb5f4) SHA1(3003b3f5e70712339bf0d88e45ca0dd7ca8cf7d0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr9",    0x0000, 0x1000, CRC(1c5dba78) SHA1(c2232221ae9960295055fcf1bd75d798136e694c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr10",   0x0000, 0x1000, CRC(a57ac8d0) SHA1(1d4424dcbecb75b0e3e4ef5d296e252e7e9056ff) )
	ROM_LOAD( "sr11",   0x1000, 0x1000, CRC(aa7ec7b2) SHA1(bbc6a1022c15ffbf0f6f9828674c8c9947e7ea5a) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sr15",   0x0000, 0x1000, CRC(744f3405) SHA1(4df5932e15e68ba10f8b13ed5a59cc7d54af7b80) )
	ROM_LOAD( "sr16",   0x1000, 0x1000, CRC(3e1a876b) SHA1(15b1c40c4a6e8e3e4702699396ce0885027ab6d1) )

	ROM_REGION( 0x3000, "gfx3", 0 )
	ROM_LOAD( "sr12",   0x0000, 0x1000, CRC(81494fe8) SHA1(056de41952e6fd564ecc0ecb718caf467c03bfed) )
	ROM_LOAD( "sr13",   0x1000, 0x1000, CRC(63e94648) SHA1(05fdd285f6040aa349082845fcadd6bfbd2da2f5) )
	ROM_LOAD( "sr14",   0x2000, 0x1000, CRC(277a70af) SHA1(2235b369f1a30443f058bfe895b0d2dd294b587c) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "clr.1b", 0x0000, 0x0020, CRC(87a79fe8) SHA1(b0e982cfd7c2c8669841cf26625cd0912f4038f3) )
	ROM_LOAD( "clr.9c", 0x0020, 0x0020, CRC(10d63240) SHA1(74b1c53dacb5d30cd4cf189dda6b452d88dd22f3) )
	ROM_LOAD( "clr.8a", 0x0040, 0x0020, CRC(917eabcd) SHA1(df417ca42a4e9e7d32b443e73efaaf395f31e44a) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, suprridr, 0, suprridr, suprridr, suprridr_state, empty_init, ROT90, "Taito Corporation (Venture Line license)", "Super Rider", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
