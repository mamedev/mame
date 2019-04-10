// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Konami Battlantis Hardware

    Supports:
     GX765 - Rack 'em Up/The Hustler (c) 1987 Konami
     GX777 - Battlantis (c) 1987 Konami

    Preliminary driver by: Manuel Abadia <emumanu+mame@gmail.com>

***************************************************************************/

#include "emu.h"
#include "includes/battlnts.h"
#include "includes/konamipt.h"

#include "cpu/m6809/hd6309.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/3812intf.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE_LINE_MEMBER(battlnts_state::vblank_irq)
{
	if (state && m_k007342->is_int_enabled())
		m_maincpu->set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(battlnts_state::battlnts_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

WRITE8_MEMBER(battlnts_state::battlnts_bankswitch_w)
{
	/* bits 6 & 7 = bank number */
	m_rombank->set_entry((data & 0xc0) >> 6);

	/* bits 4 & 5 = coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);

	/* other bits unknown */
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void battlnts_state::battlnts_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(m_k007342, FUNC(k007342_device::read), FUNC(k007342_device::write));    /* Color RAM + Video RAM */
	map(0x2000, 0x21ff).rw(m_k007420, FUNC(k007420_device::read), FUNC(k007420_device::write));    /* Sprite RAM */
	map(0x2200, 0x23ff).rw(m_k007342, FUNC(k007342_device::scroll_r), FUNC(k007342_device::scroll_w));      /* Scroll RAM */
	map(0x2400, 0x24ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");/* palette */
	map(0x2600, 0x2607).w(m_k007342, FUNC(k007342_device::vreg_w));          /* Video Registers */
	map(0x2e00, 0x2e00).portr("DSW1");
	map(0x2e01, 0x2e01).portr("P2");
	map(0x2e02, 0x2e02).portr("P1");
	map(0x2e03, 0x2e03).portr("DSW3");               /* coinsw, testsw, startsw */
	map(0x2e04, 0x2e04).portr("DSW2");
	map(0x2e08, 0x2e08).w(FUNC(battlnts_state::battlnts_bankswitch_w));    /* bankswitch control */
	map(0x2e0c, 0x2e0c).w(FUNC(battlnts_state::battlnts_spritebank_w));    /* sprite bank select */
	map(0x2e10, 0x2e10).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2e14, 0x2e14).w("soundlatch", FUNC(generic_latch_8_device::write)); /* sound code # */
	map(0x2e18, 0x2e18).w(FUNC(battlnts_state::battlnts_sh_irqtrigger_w)); /* cause interrupt on audio CPU */
	map(0x4000, 0x7fff).bankr("rombank");              /* banked ROM */
	map(0x8000, 0xffff).rom();                             /* ROM 777e02.bin */
}

void battlnts_state::battlnts_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                         /* ROM 777c01.rom */
	map(0x8000, 0x87ff).ram();                         /* RAM */
	map(0xa000, 0xa001).rw("ym1", FUNC(ym3812_device::read), FUNC(ym3812_device::write));      /* YM3812 (chip 1) */
	map(0xc000, 0xc001).rw("ym2", FUNC(ym3812_device::read), FUNC(ym3812_device::write));      /* YM3812 (chip 2) */
	map(0xe000, 0xe000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( battlnts )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30k And Every 70k" )
	PORT_DIPSETTING(    0x10, "40k And Every 80k" )
	PORT_DIPSETTING(    0x08, "40k" )
	PORT_DIPSETTING(    0x00, "50k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("P1")
	KONAMI8_B1(1)
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, "3 Times" )
	PORT_DIPSETTING(    0x00, "5 Times" )

	PORT_START("P2")
	KONAMI8_B1_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( rackemup )
	PORT_INCLUDE( battlnts )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, "Balls" )                 PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x10, "Time To Aim" )           PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "25s (Stage 1: 30s)" )
	PORT_DIPSETTING(    0x10, "20s (Stage 1: 25s)" )
	PORT_DIPSETTING(    0x08, "17s (Stage 1: 22s)" )
	PORT_DIPSETTING(    0x00, "15s (Stage 1: 20s)" )

	PORT_MODIFY("P1")
	KONAMI8_B12(1)
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:4" )

	PORT_MODIFY("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( thehustl )
	PORT_INCLUDE( rackemup )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:2" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,            /* 8 x 8 characters */
	0x40000/32,     /* 8192 characters */
	4,              /* 4bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8            /* every character takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,8,            /* 8*8 sprites */
	0x40000/32, /* 8192 sprites */
	4,              /* 4 bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8            /* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( gfx_battlnts )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 ) /* colors  0-15 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 4*16, 1 ) /* colors 64-79 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void battlnts_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_rombank->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_spritebank));
}

void battlnts_state::machine_reset()
{
	m_spritebank = 0;
}

void battlnts_state::battlnts(machine_config &config)
{
	/* basic machine hardware */
	HD6309(config, m_maincpu, XTAL(24'000'000) / 2 /* 3000000*4? */);
	m_maincpu->set_addrmap(AS_PROGRAM, &battlnts_state::battlnts_map);

	Z80(config, m_audiocpu, XTAL(24'000'000) / 6 /* 3579545? */);
	m_audiocpu->set_addrmap(AS_PROGRAM, &battlnts_state::battlnts_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(battlnts_state::screen_update_battlnts));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(battlnts_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_battlnts);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 128);

	K007342(config, m_k007342, 0);
	m_k007342->set_gfxnum(0);
	m_k007342->set_tile_callback(FUNC(battlnts_state::battlnts_tile_callback), this);
	m_k007342->set_gfxdecode_tag(m_gfxdecode);

	K007420(config, m_k007420, 0);
	m_k007420->set_bank_limit(0x3ff);
	m_k007420->set_sprite_callback(FUNC(battlnts_state::battlnts_sprite_callback), this);
	m_k007420->set_palette_tag("palette");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM3812(config, "ym1", XTAL(24'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 1.0);

	YM3812(config, "ym2", XTAL(24'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( battlnts )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "777_g02.7e", 0x08000, 0x08000, CRC(dbd8e17e) SHA1(586a22b714011c67a915c4a350ceca19ff875635) ) /* fixed ROM */
	ROM_LOAD( "777_g03.8e", 0x10000, 0x10000, CRC(7bd44fef) SHA1(308ec5246f5537b34e368535672ac687f456750a) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "777_c01.10a",  0x00000, 0x08000, CRC(c21206e9) SHA1(7b133e04be67dc061a186ab0481d848b69b370d7) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "777c04.13a",  0x00000, 0x40000, CRC(45d92347) SHA1(8537b4ccd0a80ea3260ef82fde177f1d65a49c03) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "777c05.13e",  0x00000, 0x40000, CRC(aeee778c) SHA1(fc58ada9c97361d13439b7b0918c947d48402445) ) /* sprites */
ROM_END

ROM_START( battlntsa )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "777_f02.7e", 0x08000, 0x08000, CRC(9f1dc5c1) SHA1(86ae471b276d90bbebb97747b673cd5c31ff9043) ) /* fixed ROM */
	ROM_LOAD( "777_f03.8e", 0x10000, 0x10000, CRC(040d00bf) SHA1(433afd38a80d79d6a95f2ef0b195a92688ace555) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "777_c01.10a",  0x00000, 0x08000, CRC(c21206e9) SHA1(7b133e04be67dc061a186ab0481d848b69b370d7) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "777c04.13a",  0x00000, 0x40000, CRC(45d92347) SHA1(8537b4ccd0a80ea3260ef82fde177f1d65a49c03) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "777c05.13e",  0x00000, 0x40000, CRC(aeee778c) SHA1(fc58ada9c97361d13439b7b0918c947d48402445) ) /* sprites */
ROM_END

ROM_START( battlntsj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "777_e02.7e",  0x08000, 0x08000, CRC(d631cfcb) SHA1(7787da0dd8cd218abc27204e517e04d7a1913a3b) ) /* fixed ROM */
	ROM_LOAD( "777_e03.8e",  0x10000, 0x10000, CRC(5ef1f4ef) SHA1(e3e6e1fc5a65328d94c23e2e76eef3504b70e58b) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "777_c01.10a",  0x00000, 0x08000, CRC(c21206e9) SHA1(7b133e04be67dc061a186ab0481d848b69b370d7) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "777c04.13a",  0x00000, 0x40000, CRC(45d92347) SHA1(8537b4ccd0a80ea3260ef82fde177f1d65a49c03) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "777c05.13e",  0x00000, 0x40000, CRC(aeee778c) SHA1(fc58ada9c97361d13439b7b0918c947d48402445) ) /* sprites */
ROM_END

ROM_START( rackemup )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "765_l02.7e",  0x08000, 0x08000, CRC(3dfc48bd) SHA1(9ba98e9f27dd0a6efec145bea2a5ae7df8567437) ) /* fixed ROM */
	ROM_LOAD( "765_j03.8e",  0x10000, 0x10000, CRC(a13fd751) SHA1(27ec66835c85b7ac0221a813d38e9cca0d9be3b8) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "765_j01.10a", 0x00000, 0x08000, CRC(77ae753e) SHA1(9e463a825d31bb79644b083d24b25670d96441c5) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "765_l04.13a", 0x00000, 0x40000, CRC(d8fb9c64) SHA1(37dac643aa492ef1ecc29c5030bc7fe5226027a2) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "765_l05.13e", 0x00000, 0x40000, CRC(1bb6855f) SHA1(251081564dfede8fa9a422081d58465fe5ca4ed1) ) /* sprites */
ROM_END

ROM_START( thehustl )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "765_m02.7e",  0x08000, 0x08000, CRC(934807b9) SHA1(84e13a5c1587ee28330f369f9a1180219edbda9d) ) /* fixed ROM */
	ROM_LOAD( "765_j03.8e",  0x10000, 0x10000, CRC(a13fd751) SHA1(27ec66835c85b7ac0221a813d38e9cca0d9be3b8) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "765_j01.10a", 0x00000, 0x08000, CRC(77ae753e) SHA1(9e463a825d31bb79644b083d24b25670d96441c5) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "765_e04.13a", 0x00000, 0x40000, CRC(08c2b72e) SHA1(02d9c690da839d6fee75fffdf66a4d3da35a0263) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "765_e05.13e", 0x00000, 0x40000, CRC(ef044655) SHA1(c8272283eab8fc2899979da398819cb72c92a299) ) /* sprites */
ROM_END

ROM_START( thehustlj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "765_j02.7e",  0x08000, 0x08000, CRC(2ac14c75) SHA1(b88f6279ab88719f4207e28486a0022554668382) ) /* fixed ROM */
	ROM_LOAD( "765_j03.8e",  0x10000, 0x10000, CRC(a13fd751) SHA1(27ec66835c85b7ac0221a813d38e9cca0d9be3b8) ) /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "765_j01.10a", 0x00000, 0x08000, CRC(77ae753e) SHA1(9e463a825d31bb79644b083d24b25670d96441c5) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "765_e04.13a", 0x00000, 0x40000, CRC(08c2b72e) SHA1(02d9c690da839d6fee75fffdf66a4d3da35a0263) ) /* tiles */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "765_e05.13e", 0x00000, 0x40000, CRC(ef044655) SHA1(c8272283eab8fc2899979da398819cb72c92a299) ) /* sprites */
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, battlnts,  0,        battlnts, battlnts, battlnts_state, empty_init, ROT90, "Konami", "Battlantis (program code G)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, battlntsa, battlnts, battlnts, battlnts, battlnts_state, empty_init, ROT90, "Konami", "Battlantis (program code F)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, battlntsj, battlnts, battlnts, battlnts, battlnts_state, empty_init, ROT90, "Konami", "Battlantis (Japan, program code E)",  MACHINE_SUPPORTS_SAVE )
GAME( 1987, rackemup,  0,        battlnts, rackemup, battlnts_state, empty_init, ROT90, "Konami", "Rack 'em Up (program code L)",        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, thehustl,  rackemup, battlnts, thehustl, battlnts_state, empty_init, ROT90, "Konami", "The Hustler (Japan, program code M)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, thehustlj, rackemup, battlnts, thehustl, battlnts_state, empty_init, ROT90, "Konami", "The Hustler (Japan, program code J)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
