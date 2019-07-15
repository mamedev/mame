// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Haunted Castle

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/hcastle.h"
#include "includes/konamipt.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/3812intf.h"
#include "sound/k051649.h"

#include "screen.h"
#include "speaker.h"


WRITE8_MEMBER(hcastle_state::hcastle_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x1f);
}

WRITE8_MEMBER(hcastle_state::hcastle_soundirq_w)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(hcastle_state::hcastle_coin_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x40);
	machine().bookkeeping().coin_counter_w(1, data & 0x80);
}



void hcastle_state::hcastle_map(address_map &map)
{
	map(0x0000, 0x0007).w(FUNC(hcastle_state::hcastle_pf1_control_w));
	map(0x0020, 0x003f).ram(); /* rowscroll? */
	map(0x0200, 0x0207).w(FUNC(hcastle_state::hcastle_pf2_control_w));
	map(0x0220, 0x023f).ram(); /* rowscroll? */
	map(0x0400, 0x0400).w(FUNC(hcastle_state::hcastle_bankswitch_w));
	map(0x0404, 0x0404).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0408, 0x0408).w(FUNC(hcastle_state::hcastle_soundirq_w));
	map(0x040c, 0x040c).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x0410, 0x0410).portr("SYSTEM").w(FUNC(hcastle_state::hcastle_coin_w));
	map(0x0411, 0x0411).portr("P1");
	map(0x0412, 0x0412).portr("P2");
	map(0x0413, 0x0413).portr("DSW3");
	map(0x0414, 0x0414).portr("DSW1");
	map(0x0415, 0x0415).portr("DSW2");
	map(0x0418, 0x0418).rw(FUNC(hcastle_state::hcastle_gfxbank_r), FUNC(hcastle_state::hcastle_gfxbank_w));
	map(0x0600, 0x06ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0x0700, 0x1fff).ram();
	map(0x2000, 0x2fff).ram().w(FUNC(hcastle_state::hcastle_pf1_video_w)).share("pf1_videoram");
	map(0x3000, 0x3fff).ram().share("spriteram");
	map(0x4000, 0x4fff).ram().w(FUNC(hcastle_state::hcastle_pf2_video_w)).share("pf2_videoram");
	map(0x5000, 0x5fff).ram().share("spriteram2");
	map(0x6000, 0x7fff).bankr("bank1");
	map(0x8000, 0xffff).rom();
}

/*****************************************************************************/

WRITE8_MEMBER(hcastle_state::sound_bank_w)
{
	int bank_A=(data&0x3);
	int bank_B=((data>>2)&0x3);
	m_k007232->set_bank(bank_A, bank_B );
}

void hcastle_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9800, 0x98ff).m("k051649", FUNC(k051649_device::scc_map));
	map(0xa000, 0xa001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc000).w(FUNC(hcastle_state::sound_bank_w)); /* 7232 bankswitch */
	map(0xd000, 0xd000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

/*****************************************************************************/

static INPUT_PORTS_START( hcastle )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_4WAY_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_4WAY_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, "Difficulty 1 (Game)" )       PORT_DIPLOCATION("SW2:4,5") /* Overall difficulty of game */
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x60, 0x40, "Difficulty 2 (Strength)" )       PORT_DIPLOCATION("SW2:6,7") /* Listed in manual as "Strength of Player" */
	PORT_DIPSETTING(    0x00, "Very Weak" )                         /* Takes most damage per hit */
	PORT_DIPSETTING(    0x20, "Weak" )                          /* Takes more damage per hit */
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )                       /* Takes average damage per hit */
	PORT_DIPSETTING(    0x60, "Strong" )                            /* Takes least damage per hit */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "Up to 3 Times" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	32768,
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( gfx_hcastle )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 8*16 ) /* 007121 #0 */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 8*16*16, 8*16 ) /* 007121 #1 */
GFXDECODE_END

/*****************************************************************************/

WRITE8_MEMBER(hcastle_state::volume_callback)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void hcastle_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 16, &ROM[0x10000], 0x2000);

	save_item(NAME(m_pf2_bankbase));
	save_item(NAME(m_pf1_bankbase));
	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_old_pf1));
	save_item(NAME(m_old_pf2));
}

void hcastle_state::machine_reset()
{
	m_pf2_bankbase = 0;
	m_pf1_bankbase = 0;
	m_gfx_bank = 0;
	m_old_pf1 = -1;
	m_old_pf2 = -1;
}

void hcastle_state::hcastle(machine_config &config)
{
	/* basic machine hardware */
	KONAMI(config, m_maincpu, 3000000);    /* Derived from 24 MHz clock */
	m_maincpu->set_addrmap(AS_PROGRAM, &hcastle_state::hcastle_map);
	m_maincpu->set_vblank_int("screen", FUNC(hcastle_state::irq0_line_hold));

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &hcastle_state::sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	BUFFERED_SPRITERAM8(config, m_spriteram);
	BUFFERED_SPRITERAM8(config, m_spriteram2);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));  /* frames per second verified by comparison with real board */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(hcastle_state::screen_update_hcastle));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_hcastle);
	PALETTE(config, m_palette, FUNC(hcastle_state::hcastle_palette)).set_format(palette_device::xBGR_555, 2*8*16*16, 128);

	K007121(config, m_k007121_1, 0);
	m_k007121_1->set_palette_tag(m_palette);
	K007121(config, m_k007121_2, 0);
	m_k007121_2->set_palette_tag(m_palette);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(hcastle_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.44);
	m_k007232->add_route(1, "mono", 0.50);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 3579545));
	ymsnd.irq_handler().set_inputline("audiocpu", INPUT_LINE_NMI); /* from schematic; NMI handler is just a retn */
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.70);

	K051649(config, "k051649", 3579545/2).add_route(ALL_OUTPUTS, "mono", 0.45);
}

/***************************************************************************/

ROM_START( hcastle )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "m03.k12",      0x08000, 0x08000, CRC(d85e743d) SHA1(314e2a2bbe650540306b85c8b89ec5bcaef11a0d) )
	ROM_LOAD( "b06.k8",       0x10000, 0x20000, CRC(abd07866) SHA1(a261d0cd90f5909abd06e8b691669e63d890c3be) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "768e01.e4",    0x00000, 0x08000, CRC(b9fff184) SHA1(c55f468c0da6afdaa2af65a111583c0c42868bd1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "768c09.g21",   0x000000, 0x80000, CRC(e3be3fdd) SHA1(01a686af33a0a700066b1a5334d8552454ff186f) )
	ROM_LOAD( "768c08.g19",   0x080000, 0x80000, CRC(9633db8b) SHA1(fe1b117c2566288b88f000106c649c2fa5648ddc) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "768c04.j5",    0x000000, 0x80000, CRC(2960680e) SHA1(72e1f025496c907de8516e3b5f1781e73d5b2c6c) )
	ROM_LOAD( "768c05.j6",    0x080000, 0x80000, CRC(65a2f227) SHA1(43f368e533d6a164dc68d54130b81883e0d1bafe) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "768c13.j21",   0x0000, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #0 sprite lookup table */
	ROM_LOAD( "768c14.j22",   0x0100, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #0 char lookup table */
	ROM_LOAD( "768c11.i4",    0x0200, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #1 sprite lookup table (same) */
	ROM_LOAD( "768c10.i3",    0x0300, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #1 char lookup table (same) */
	ROM_LOAD( "768b12.d20",   0x0400, 0x0100, CRC(362544b8) SHA1(744c8d2ccfa980fc9a7354b4d241c569b3c1fffe) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for the samples */
	ROM_LOAD( "768c07.e17",   0x00000, 0x80000, CRC(01f9889c) SHA1(01252d2ce7b14cfbe39ac8d7a5bd7417f1c2fc22) )
ROM_END

ROM_START( hcastlek )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "768k03.k12",   0x08000, 0x08000, CRC(40ce4f38) SHA1(1ab6d62a75c818b2ccbbb714373d6c7418500eb7) )
	ROM_LOAD( "768g06.k8",    0x10000, 0x20000, CRC(cdade920) SHA1(e15b7458ded4e4c811a737575ec3f16e5eec4121) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "768e01.e4",    0x00000, 0x08000, CRC(b9fff184) SHA1(c55f468c0da6afdaa2af65a111583c0c42868bd1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "768c09.g21",   0x000000, 0x80000, CRC(e3be3fdd) SHA1(01a686af33a0a700066b1a5334d8552454ff186f) )
	ROM_LOAD( "768c08.g19",   0x080000, 0x80000, CRC(9633db8b) SHA1(fe1b117c2566288b88f000106c649c2fa5648ddc) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "768c04.j5",    0x000000, 0x80000, CRC(2960680e) SHA1(72e1f025496c907de8516e3b5f1781e73d5b2c6c) )
	ROM_LOAD( "768c05.j6",    0x080000, 0x80000, CRC(65a2f227) SHA1(43f368e533d6a164dc68d54130b81883e0d1bafe) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "768c13.j21",   0x0000, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #0 sprite lookup table */
	ROM_LOAD( "768c14.j22",   0x0100, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #0 char lookup table */
	ROM_LOAD( "768c11.i4",    0x0200, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #1 sprite lookup table (same) */
	ROM_LOAD( "768c10.i3",    0x0300, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #1 char lookup table (same) */
	ROM_LOAD( "768b12.d20",   0x0400, 0x0100, CRC(362544b8) SHA1(744c8d2ccfa980fc9a7354b4d241c569b3c1fffe) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for the samples */
	ROM_LOAD( "768c07.e17",   0x00000, 0x80000, CRC(01f9889c) SHA1(01252d2ce7b14cfbe39ac8d7a5bd7417f1c2fc22) )
ROM_END

ROM_START( hcastlee )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "768e03.k12",   0x08000, 0x08000, CRC(0b32619c) SHA1(a62ef0a90d061ff642350bd50e900144b4cef00e) )
	ROM_LOAD( "768e06.k8",    0x10000, 0x20000, CRC(0431b8c0) SHA1(54b576f958fe78ec0a603e8daaf81ee09107a184) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "768e01.e4",    0x00000, 0x08000, CRC(b9fff184) SHA1(c55f468c0da6afdaa2af65a111583c0c42868bd1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "768c09.g21",   0x000000, 0x80000, CRC(e3be3fdd) SHA1(01a686af33a0a700066b1a5334d8552454ff186f) )
	ROM_LOAD( "768c08.g19",   0x080000, 0x80000, CRC(9633db8b) SHA1(fe1b117c2566288b88f000106c649c2fa5648ddc) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "768c04.j5",    0x000000, 0x80000, CRC(2960680e) SHA1(72e1f025496c907de8516e3b5f1781e73d5b2c6c) )
	ROM_LOAD( "768c05.j6",    0x080000, 0x80000, CRC(65a2f227) SHA1(43f368e533d6a164dc68d54130b81883e0d1bafe) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "768c13.j21",   0x0000, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #0 sprite lookup table */
	ROM_LOAD( "768c14.j22",   0x0100, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #0 char lookup table */
	ROM_LOAD( "768c11.i4",    0x0200, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #1 sprite lookup table (same) */
	ROM_LOAD( "768c10.i3",    0x0300, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #1 char lookup table (same) */
	ROM_LOAD( "768b12.d20",   0x0400, 0x0100, CRC(362544b8) SHA1(744c8d2ccfa980fc9a7354b4d241c569b3c1fffe) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for the samples */
	ROM_LOAD( "768c07.e17",   0x00000, 0x80000, CRC(01f9889c) SHA1(01252d2ce7b14cfbe39ac8d7a5bd7417f1c2fc22) )
ROM_END

ROM_START( akumajou )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "768p03.k12",0x08000, 0x08000, CRC(d509e340) SHA1(3a8078bd89a80ab9529e4ee8658fcafb8dd65258) )
	ROM_LOAD( "768j06.k8", 0x10000, 0x20000, CRC(42283c3e) SHA1(565a2eb607e262484f48919536c045d515cff89f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "768e01.e4",    0x00000, 0x08000, CRC(b9fff184) SHA1(c55f468c0da6afdaa2af65a111583c0c42868bd1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "768c09.g21",   0x000000, 0x80000, CRC(e3be3fdd) SHA1(01a686af33a0a700066b1a5334d8552454ff186f) )
	ROM_LOAD( "768c08.g19",   0x080000, 0x80000, CRC(9633db8b) SHA1(fe1b117c2566288b88f000106c649c2fa5648ddc) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "768c04.j5",    0x000000, 0x80000, CRC(2960680e) SHA1(72e1f025496c907de8516e3b5f1781e73d5b2c6c) )
	ROM_LOAD( "768c05.j6",    0x080000, 0x80000, CRC(65a2f227) SHA1(43f368e533d6a164dc68d54130b81883e0d1bafe) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "768c13.j21",   0x0000, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #0 sprite lookup table */
	ROM_LOAD( "768c14.j22",   0x0100, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #0 char lookup table */
	ROM_LOAD( "768c11.i4",    0x0200, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #1 sprite lookup table (same) */
	ROM_LOAD( "768c10.i3",    0x0300, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #1 char lookup table (same) */
	ROM_LOAD( "768b12.d20",   0x0400, 0x0100, CRC(362544b8) SHA1(744c8d2ccfa980fc9a7354b4d241c569b3c1fffe) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for the samples */
	ROM_LOAD( "768c07.e17",   0x00000, 0x80000, CRC(01f9889c) SHA1(01252d2ce7b14cfbe39ac8d7a5bd7417f1c2fc22) )
ROM_END

ROM_START( akumajoun )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "768n03.k12",0x08000, 0x08000, CRC(3e4dca2a) SHA1(cd70fdc42b970b89ae16ab6c81d1a5003fa53dbd) )
	ROM_LOAD( "768j06.k8", 0x10000, 0x20000, CRC(42283c3e) SHA1(565a2eb607e262484f48919536c045d515cff89f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "768e01.e4",    0x00000, 0x08000, CRC(b9fff184) SHA1(c55f468c0da6afdaa2af65a111583c0c42868bd1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "768c09.g21",   0x000000, 0x80000, CRC(e3be3fdd) SHA1(01a686af33a0a700066b1a5334d8552454ff186f) )
	ROM_LOAD( "768c08.g19",   0x080000, 0x80000, CRC(9633db8b) SHA1(fe1b117c2566288b88f000106c649c2fa5648ddc) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "768c04.j5",    0x000000, 0x80000, CRC(2960680e) SHA1(72e1f025496c907de8516e3b5f1781e73d5b2c6c) )
	ROM_LOAD( "768c05.j6",    0x080000, 0x80000, CRC(65a2f227) SHA1(43f368e533d6a164dc68d54130b81883e0d1bafe) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "768c13.j21",   0x0000, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #0 sprite lookup table */
	ROM_LOAD( "768c14.j22",   0x0100, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #0 char lookup table */
	ROM_LOAD( "768c11.i4",    0x0200, 0x0100, CRC(f5de80cb) SHA1(e8cc3e14a5d23b25fb7bf790e64786c6aa2df8b7) )    /* 007121 #1 sprite lookup table (same) */
	ROM_LOAD( "768c10.i3",    0x0300, 0x0100, CRC(b32071b7) SHA1(09a699a3f20c155eae1e63429f03ed91abc54784) )    /* 007121 #1 char lookup table (same) */
	ROM_LOAD( "768b12.d20",   0x0400, 0x0100, CRC(362544b8) SHA1(744c8d2ccfa980fc9a7354b4d241c569b3c1fffe) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for the samples */
	ROM_LOAD( "768c07.e17",   0x00000, 0x80000, CRC(01f9889c) SHA1(01252d2ce7b14cfbe39ac8d7a5bd7417f1c2fc22) )
ROM_END



GAME( 1988, hcastle,   0,       hcastle, hcastle, hcastle_state, empty_init, ROT0, "Konami", "Haunted Castle (version M)",          MACHINE_SUPPORTS_SAVE )
GAME( 1988, hcastlek,  hcastle, hcastle, hcastle, hcastle_state, empty_init, ROT0, "Konami", "Haunted Castle (version K)",          MACHINE_SUPPORTS_SAVE )
GAME( 1988, hcastlee,  hcastle, hcastle, hcastle, hcastle_state, empty_init, ROT0, "Konami", "Haunted Castle (version E)",          MACHINE_SUPPORTS_SAVE )
GAME( 1988, akumajou,  hcastle, hcastle, hcastle, hcastle_state, empty_init, ROT0, "Konami", "Akuma-Jou Dracula (Japan version P)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, akumajoun, hcastle, hcastle, hcastle, hcastle_state, empty_init, ROT0, "Konami", "Akuma-Jou Dracula (Japan version N)", MACHINE_SUPPORTS_SAVE )
