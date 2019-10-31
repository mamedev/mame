// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= American Speedway =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU  :  Z80A x 2
Sound:  YM2151


(c)1987 Enerdyne Technologies, Inc. / PGD

***************************************************************************/

#include "emu.h"
#include "includes/amspdwy.h"
#include "cpu/z80/z80.h"
#include "speaker.h"


/***************************************************************************


                                    Main CPU


***************************************************************************/

/*
    765-----    Buttons
    ---4----    Sgn(Wheel Delta)
    ----3210    Abs(Wheel Delta)

    Or last value when wheel delta = 0
*/

uint8_t amspdwy_state::amspdwy_wheel_r( int index )
{
	static const char *const portnames[] = { "WHEEL1", "WHEEL2", "AN1", "AN2" };
	uint8_t wheel = ioport(portnames[2 + index])->read();
	if (wheel != m_wheel_old[index])
	{
		wheel = (wheel & 0x7fff) - (wheel & 0x8000);
		if (wheel > m_wheel_old[index])
			m_wheel_return[index] = ((+wheel) & 0xf) | 0x00;
		else
			m_wheel_return[index] = ((-wheel) & 0xf) | 0x10;

		m_wheel_old[index] = wheel;
	}
	return m_wheel_return[index] | ioport(portnames[index])->read();
}

READ8_MEMBER(amspdwy_state::amspdwy_wheel_0_r)
{
	// player 1
	return amspdwy_wheel_r(0);
}

READ8_MEMBER(amspdwy_state::amspdwy_wheel_1_r)
{
	// player 2
	return amspdwy_wheel_r(1);
}

READ8_MEMBER(amspdwy_state::amspdwy_sound_r)
{
	return (m_ym2151->status_r() & ~0x30) | ioport("IN0")->read();
}

void amspdwy_state::amspdwy_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x801f).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x9000, 0x93ff).mirror(0x0400).ram().w(FUNC(amspdwy_state::amspdwy_videoram_w)).share("videoram");
	map(0x9800, 0x9bff).ram().w(FUNC(amspdwy_state::amspdwy_colorram_w)).share("colorram");
	map(0x9c00, 0x9fff).ram(); // unused?
//  map(0xa000, 0xa000).nopw(); // ?
	map(0xa000, 0xa000).portr("DSW1");
	map(0xa400, 0xa400).portr("DSW2").w(FUNC(amspdwy_state::amspdwy_flipscreen_w));
	map(0xa800, 0xa800).r(FUNC(amspdwy_state::amspdwy_wheel_0_r));
	map(0xac00, 0xac00).r(FUNC(amspdwy_state::amspdwy_wheel_1_r));
	map(0xb000, 0xb000).nopw(); // irq ack?
	map(0xb400, 0xb400).r(FUNC(amspdwy_state::amspdwy_sound_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc000, 0xc0ff).ram().share("spriteram");
	map(0xe000, 0xe7ff).ram();
}

void amspdwy_state::amspdwy_portmap(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("tracks", 0);
}



/***************************************************************************


                                Sound CPU


***************************************************************************/

void amspdwy_state::amspdwy_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
//  map(0x8000, 0x8000).nopw(); // ? writes 0 at start
	map(0x9000, 0x9000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xa000, 0xa001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc000, 0xdfff).ram();
	map(0xffff, 0xffff).nopr(); // ??? IY = FFFF at the start ?
}




/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( amspdwy )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Character Test" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Show Arrows" )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_HIGH, "SW1:5" )
	PORT_DIPNAME( 0x10, 0x00, "Steering Test" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:1" )        /* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x00, "Time To Qualify" )       PORT_DIPLOCATION("SW2:3,4") /* code at 0x1770 */
	PORT_DIPSETTING(    0x30, "20 sec" )
	PORT_DIPSETTING(    0x20, "30 sec" )
	PORT_DIPSETTING(    0x10, "45 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:1" )        /* Listed as "Unused" */

	PORT_START("WHEEL1")    // Player 1 Wheel + Coins
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )   // wheel
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2) // 2-3f

	PORT_START("WHEEL2")    // Player 2 Wheel + Coins
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("IN0")   // Player 1&2 Pedals + YM2151 Sound Status
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM )

	PORT_START("AN1")   // Player 1 Analog Fake Port
	PORT_BIT( 0xffff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_PLAYER(1)

	PORT_START("AN2")   // Player 2 Analog Fake Port
	PORT_BIT( 0xffff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20) PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_G) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( amspdwya )
	PORT_INCLUDE(amspdwy)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x00, "Time To Qualify" )       PORT_DIPLOCATION("SW2:4") /* code at 0x2696 */
	PORT_DIPSETTING(    0x10, "45 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:3" )        /* Listed as "Unused" */
INPUT_PORTS_END



/***************************************************************************


                                Graphics Layouts


***************************************************************************/

static const gfx_layout layout_8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_amspdwy )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x2, 0, 8 )
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/


void amspdwy_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_wheel_old));
	save_item(NAME(m_wheel_return));
}

void amspdwy_state::machine_reset()
{
	m_flipscreen = 0;
	m_wheel_old[0] = 0;
	m_wheel_old[1] = 0;
	m_wheel_return[0] = 0;
	m_wheel_return[1] = 0;
}

void amspdwy_state::amspdwy(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &amspdwy_state::amspdwy_map);
	m_maincpu->set_addrmap(AS_IO, &amspdwy_state::amspdwy_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(amspdwy_state::irq0_line_hold)); /* IRQ: 60Hz, NMI: retn */

	Z80(config, m_audiocpu, 3000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &amspdwy_state::amspdwy_sound_map);

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(amspdwy_state::screen_update_amspdwy));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_amspdwy);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233_inverted, 32);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2151(config, m_ym2151, 3000000);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 0);
	m_ym2151->add_route(0, "lspeaker", 1.0);
	m_ym2151->add_route(1, "rspeaker", 1.0);
}




/***************************************************************************


                                ROMs Loading


***************************************************************************/



/***************************************************************************

                            American Speedway

USES TWO Z80 CPU'S W/YM2151 SOUND
THE NUMBERS WITH THE NAMES ARE PROBABLY CHECKSUMS

NAME    LOCATION    TYPE
------------------------
AUDI9363 U2         27256   CONN BD
GAME5807 U33         "       "
TRKS6092 U34         "       "
HIHIE12A 4A         2732    REAR BD
HILO9B3C 5A          "       "
LOHI4644 2A          "       "
LOLO1D51 1A          "       "

                        American Speedway (Set 2)

1987 Enerdyne Technologies, Inc. Has Rev 4 PGD written on the top board.

Processors
------------------
Dual Z80As
YM2151     (sound)

RAM
------------------
12 2114
5  82S16N

Eproms
==================

Name        Loc   TYpe   Checksum
----------  ----  -----  --------
Game.u22    U33   27256  A222
Tracks.u34  U34   27256  6092
Audio.U02   U2    27256  9363
LOLO1.1A    1A    2732   1D51
LOHI.2A     2A    2732   4644
HIHI.4A     3/4A  2732   E12A
HILO.5A     5A    2732   9B3C

***************************************************************************/

ROM_START( amspdwy )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "game5807.u33", 0x0000, 0x8000, CRC(88233b59) SHA1(bfdf10dde1731cde5c579a9a5173cafe9295a80c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound Z80 Code */
	ROM_LOAD( "audi9463.u2",  0x0000, 0x8000, CRC(61b0467e) SHA1(74509e7712838dd760919893aeda9241d308d0c3) )

	ROM_REGION( 0x8000, "tracks", 0 )
	ROM_LOAD( "trks6092.u34", 0x0000, 0x8000, CRC(74a4e7b7) SHA1(b4f6e3faaf048351c6671205f52378a64b81bcb1) )

	ROM_REGION( 0x4000, "gfx1", 0 ) /* Layer + Sprites */
	ROM_LOAD( "hilo9b3c.5a",  0x0000, 0x1000, CRC(f50f864c) SHA1(5b2412c1558b30a04523fcdf1d5cf6fdae1ba88d) )
	ROM_LOAD( "hihie12a.4a",  0x1000, 0x1000, CRC(3d7497f3) SHA1(34820ba42d9c9dab1d6fdda15795450ce08392c1) )
	ROM_LOAD( "lolo1d51.1a",  0x2000, 0x1000, CRC(58701c1c) SHA1(67b476e697652a6b684bd76ae6c0078ed4b3e3a2) )
	ROM_LOAD( "lohi4644.2a",  0x3000, 0x1000, CRC(a1d802b1) SHA1(1249ce406b1aa518885a02ab063fa14906ccec2e) )
ROM_END

ROM_START( amspdwya )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "game.u33",     0x0000, 0x8000, CRC(facab102) SHA1(e232969eaaad8b89ac8e28ee0a7996107a7de9a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound Z80 Code */
	ROM_LOAD( "audi9463.u2",  0x0000, 0x8000, CRC(61b0467e) SHA1(74509e7712838dd760919893aeda9241d308d0c3) )

	ROM_REGION( 0x8000, "tracks", 0 )
	ROM_LOAD( "trks6092.u34", 0x0000, 0x8000, CRC(74a4e7b7) SHA1(b4f6e3faaf048351c6671205f52378a64b81bcb1) )

	ROM_REGION( 0x4000, "gfx1", 0 ) /* Layer + Sprites */
	ROM_LOAD( "hilo9b3c.5a",  0x0000, 0x1000, CRC(f50f864c) SHA1(5b2412c1558b30a04523fcdf1d5cf6fdae1ba88d) )
	ROM_LOAD( "hihie12a.4a",  0x1000, 0x1000, CRC(3d7497f3) SHA1(34820ba42d9c9dab1d6fdda15795450ce08392c1) )
	ROM_LOAD( "lolo1d51.1a",  0x2000, 0x1000, CRC(58701c1c) SHA1(67b476e697652a6b684bd76ae6c0078ed4b3e3a2) )
	ROM_LOAD( "lohi4644.2a",  0x3000, 0x1000, CRC(a1d802b1) SHA1(1249ce406b1aa518885a02ab063fa14906ccec2e) )
ROM_END


/* (C) 1987 ETI 8402 MAGNOLIA ST. #C SANTEE, CA 92071 */

GAME( 1987, amspdwy,  0,       amspdwy, amspdwy,  amspdwy_state, empty_init, ROT0, "Enerdyne Technologies Inc.", "American Speedway (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, amspdwya, amspdwy, amspdwy, amspdwya, amspdwy_state, empty_init, ROT0, "Enerdyne Technologies Inc.", "American Speedway (set 2)", MACHINE_SUPPORTS_SAVE )
