// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*
 * Signetics 2650 CPU Games
 *
 * Zaccaria - The Invaders
 * Sidam    - Super Invader Attack
 * Zaccaria - Dodgem
 *
 * mike@the-coates.com
 *
 *
 * TODO: discrete sound
 *
 */

#include "emu.h"
#include "zac2650.h"

#include "cpu/s2650/s2650.h"
#include "speaker.h"

#include "tinv2650.lh"


/***********************************************************************************************/

void zac2650_state::main_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x1bff).ram().w(FUNC(zac2650_state::tinvader_videoram_w)).share("videoram");
	map(0x1c00, 0x1cff).ram();
	map(0x1d00, 0x1dff).ram();
	map(0x1e80, 0x1e80).r(FUNC(zac2650_state::tinvader_port_0_r)).w(FUNC(zac2650_state::tinvader_sound_w));
	map(0x1e81, 0x1e81).portr("1E81");
	map(0x1e82, 0x1e82).portr("1E82");
	map(0x1e85, 0x1e85).portr("1E85");                   /* Dodgem Only */
	map(0x1e86, 0x1e86).portr("1E86").nopw();       /* Dodgem Only */
	map(0x1f00, 0x1fff).rw(FUNC(zac2650_state::zac_s2636_r), FUNC(zac2650_state::zac_s2636_w)).share("s2636_0_ram");
}

static INPUT_PORTS_START( tinvader )
	PORT_START("1E80")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Missile-Background Collision */

	PORT_START("1E81")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Lightning Speed" )   /* Velocita Laser Inv */
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPNAME( 0x1C, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1C, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x20, "1500" )
	PORT_DIPNAME( 0x40, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )

	PORT_START("1E82")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1E85")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1E86")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Almost identical, no number of bases selection */
static INPUT_PORTS_START( sinvader )
	PORT_INCLUDE( tinvader )

	PORT_MODIFY("1E80")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("1E81")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED  )
INPUT_PORTS_END

static INPUT_PORTS_START( dodgem )
	PORT_START("1E80")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Missile-Background Collision */

	PORT_START("1E81")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Time" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1C, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1C, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x00, "Show High Scores" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("1E82")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1E85")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("1E86")
	PORT_DIPNAME( 0x01, 0x01, "Collision Detection (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


void zac2650_state::zac2650_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t::white());
	palette.set_pen_color(2, rgb_t::black());
	palette.set_pen_color(3, rgb_t::black());
}

/************************************************************************************************

 Video is slightly odd on these zac boards

 background is 256 x 240 pixels, but the sprite chips run at a different frequency, which means
 that the output of 196x240 is stretched to fill the same screen space.

 to 'properly' accomplish this, we set the screen up as 768x720 and do the background at 3 times
 the size, and the sprites as 4 times the size - everything then matches up correctly.

************************************************************************************************/


static const gfx_layout tinvader_character =
{
	8,8,
	128,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static const gfx_layout s2636_character =
{
	8,10,
	16,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8), STEP2(8*8,8) },
	8*8
};

static GFXDECODE_START( gfx_tinvader )
	GFXDECODE_SCALE( "gfx1", 0, tinvader_character,   0, 2, 3, 3 )
	GFXDECODE_SCALE( nullptr,   0x1f00, s2636_character, 0, 2, 4, 3 )  // dynamic
	GFXDECODE_SCALE( nullptr,   0x1f00, s2636_character, 0, 2, 8, 6 )  // dynamic
GFXDECODE_END

void zac2650_state::tinvader(machine_config &config)
{
	/* basic machine hardware */
	s2650_device &maincpu(S2650(config, m_maincpu, 3800000/4));
	maincpu.set_addrmap(AS_PROGRAM, &zac2650_state::main_map);
	maincpu.sense_handler().set(m_screen, FUNC(screen_device::vblank)).invert();

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(55);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1041));
	m_screen->set_size(30*24, 32*24);
	m_screen->set_visarea(0, 719, 0, 767);
	m_screen->set_screen_update(FUNC(zac2650_state::screen_update_tinvader));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tinvader);
	PALETTE(config, m_palette, FUNC(zac2650_state::zac2650_palette), 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	S2636(config, m_s2636, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void zac2650_state::tinvader_sound_w(uint8_t data)
{
	/* sounds are NOT the same as space invaders */

	logerror("Register %x = Data %d\n",data & 0xfe,data & 0x01);

	/* 08 = hit invader */
	/* 20 = bonus (extra base) */
	/* 40 = saucer */
	/* 84 = fire */
	/* 90 = die */
	/* c4 = hit saucer */
}

ROM_START( sia2650 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "42_1.bin",   0x0000, 0x0800, CRC(a85550a9) SHA1(3f1e6b8e61894ff997e31b9c5ff819aa4678394e) )
	ROM_LOAD( "44_2.bin",   0x0800, 0x0800, CRC(48d5a3ed) SHA1(7f6421ba8225d49c1038595517f31b076d566586) )
	ROM_LOAD( "46_3.bin",   0x1000, 0x0800, CRC(d766e784) SHA1(88c113855c4cde8cefbe862d3e5abf80bd17aaa0) )

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "06_inv.bin", 0x0000, 0x0400, CRC(7bfed23e) SHA1(f754f0a4d6c8f9812bf333c30fa433b63d49a750) )
ROM_END

ROM_START( tinv2650 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "42_1.bin",   0x0000, 0x0800, CRC(a85550a9) SHA1(3f1e6b8e61894ff997e31b9c5ff819aa4678394e) )
	ROM_LOAD( "44_2t.bin",  0x0800, 0x0800, CRC(083c8621) SHA1(d9b33d532903b0e6dee2357b9e3b329856505a73) )
	ROM_LOAD( "46_3t.bin",  0x1000, 0x0800, CRC(12c0934f) SHA1(9fd67d425c533b0e09b201301020639eb9e452f7) )

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "06_inv.bin", 0x0000, 0x0400, CRC(7bfed23e) SHA1(f754f0a4d6c8f9812bf333c30fa433b63d49a750) )
ROM_END

ROM_START( dodgem )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin",     0x0000, 0x0400, CRC(a327b57d) SHA1(a9cb17e60ab7b4ed9d5a9e7f8451a8f29bb7d00d) )
	ROM_LOAD( "rom2.bin",     0x0400, 0x0400, CRC(2a06ec74) SHA1(34fd3cbb1ddadb81abde54046bf245e2285bb740) )
	ROM_LOAD( "rom3.bin",     0x0800, 0x0400, CRC(e9ed656d) SHA1(a36ec04fd7cdf26aa7fa36e18cd44b159ed53906) )
	ROM_LOAD( "rom4.bin",     0x0c00, 0x0400, CRC(ecbfd906) SHA1(89f921a3d69b30977cd09a62dff4be02e6604550) )
	ROM_LOAD( "rom5.bin",     0x1000, 0x0400, CRC(bdae09fe) SHA1(76517d432d9bff5a2eea438f6edc3e04b889448a) )
	ROM_LOAD( "rom6.bin",     0x1400, 0x0400, CRC(e131eacf) SHA1(6f5244a9d27b3c5696ed83843e46079d579f7b39) )

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "93451.bin",    0x0000, 0x0400, CRC(004b26d2) SHA1(0b825510e7a8afa9db589f87ec93467ab8c73f93) )

	/* unknown */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "74s571",       0x0000, 0x0200, CRC(cc0b407e) SHA1(e675e3d7ff82e1cff9001e367620208bffa8b42f) )
ROM_END


GAMEL( 1979?, tinv2650, 0,        tinvader, tinvader, zac2650_state, empty_init, ROT270, "Zaccaria / Zelco", "The Invaders", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_tinv2650 )
GAME(  1979?, sia2650,  tinv2650, tinvader, sinvader, zac2650_state, empty_init, ROT270, "bootleg (Sidam)",  "Super Invader Attack (bootleg of The Invaders)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 1980?
GAME(  1979,  dodgem,   0,        tinvader, dodgem,   zac2650_state, empty_init, ROT0,   "Zaccaria",         "Dodgem", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
