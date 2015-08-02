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
#include "cpu/z80/z80.h"
#include "includes/amspdwy.h"

/***************************************************************************


                                    Main CPU


***************************************************************************/

/*
    765-----    Buttons
    ---4----    Sgn(Wheel Delta)
    ----3210    Abs(Wheel Delta)

    Or last value when wheel delta = 0
*/

UINT8 amspdwy_state::amspdwy_wheel_r( int index )
{
	static const char *const portnames[] = { "WHEEL1", "WHEEL2", "AN1", "AN2" };
	UINT8 wheel = ioport(portnames[2 + index])->read();
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
	return (m_ym2151->status_r(space, 0) & ~0x30) | ioport("IN0")->read();
}

WRITE8_MEMBER(amspdwy_state::amspdwy_sound_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( amspdwy_map, AS_PROGRAM, 8, amspdwy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x801f) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM_WRITE(amspdwy_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x9bff) AM_RAM_WRITE(amspdwy_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x9c00, 0x9fff) AM_RAM // unused?
//  AM_RANGE(0xa000, 0xa000) AM_WRITENOP // ?
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("DSW1")
	AM_RANGE(0xa400, 0xa400) AM_READ_PORT("DSW2") AM_WRITE(amspdwy_flipscreen_w)
	AM_RANGE(0xa800, 0xa800) AM_READ(amspdwy_wheel_0_r)
	AM_RANGE(0xac00, 0xac00) AM_READ(amspdwy_wheel_1_r)
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP // irq ack?
	AM_RANGE(0xb400, 0xb400) AM_READWRITE(amspdwy_sound_r, amspdwy_sound_w)
	AM_RANGE(0xc000, 0xc0ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( amspdwy_portmap, AS_IO, 8, amspdwy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("tracks", 0)
ADDRESS_MAP_END



/***************************************************************************


                                Sound CPU


***************************************************************************/

static ADDRESS_MAP_START( amspdwy_sound_map, AS_PROGRAM, 8, amspdwy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
//  AM_RANGE(0x8000, 0x8000) AM_WRITENOP // ? writes 0 at start
	AM_RANGE(0x9000, 0x9000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xffff, 0xffff) AM_READNOP // ??? IY = FFFF at the start ?
ADDRESS_MAP_END




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
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_SPECIAL )   // wheel
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2) // 2-3f

	PORT_START("WHEEL2")    // Player 2 Wheel + Coins
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("IN0")   // Player 1&2 Pedals + YM2151 Sound Status
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL )

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

static GFXDECODE_START( amspdwy )
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

static MACHINE_CONFIG_START( amspdwy, amspdwy_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3000000)
	MCFG_CPU_PROGRAM_MAP(amspdwy_map)
	MCFG_CPU_IO_MAP(amspdwy_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", amspdwy_state, irq0_line_hold) /* IRQ: 60Hz, NMI: retn */

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)
	MCFG_CPU_PROGRAM_MAP(amspdwy_sound_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(amspdwy_state, screen_update_amspdwy)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", amspdwy)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_FORMAT(BBGGGRRR_inverted)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 3000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END




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

GAME( 1987, amspdwy,  0,       amspdwy, amspdwy, driver_device,  0, ROT0, "Enerdyne Technologies Inc.", "American Speedway (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, amspdwya, amspdwy, amspdwy, amspdwya, driver_device, 0, ROT0, "Enerdyne Technologies Inc.", "American Speedway (set 2)", MACHINE_SUPPORTS_SAVE )
