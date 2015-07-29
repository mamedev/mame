// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Sky Diver hardware

    driver by Mike Balfour

    Games supported:
        * Sky Diver

    Known issues:
        * There is a problem with coin input not starting when in demo mode.
        * The NMI interrupt needs to be more accurate, to do 32V, adjusted
          to VBLANK.  This also affects sound.
        * The current value of 5 interrupts per frame, works pretty good,
          but is not 100% accurate timing wise.

****************************************************************************

    Memory Map:
    0000-00FF    R/W    PAGE ZERO RAM
    0010         R/W    H POS PLANE 1
    0011         R/W    H POS PLANE 2
    0012         R/W    H POS MAN 1
    0013         R/W    H POS MAN 2
    0014         R/W    RANGE LOAD
    0015         R/W    NOTE LOAD
    0016         R/W    NAM LD
    0017         R/W    UNUSED
    0018         R/W    V POS PLANE 1
    0019         R/W    PICTURE PLANE 1
    001A         R/W    V POS PLANE 2
    001B         R/W    PICTURE PLANE 2
    001C         R/W    V POS MAN 1
    001D         R/W    PICTURE MAN 1
    001E         R/W    V POS MAN 2
    001F         R/W    PICTURE MAN 2
    0400-077F    R/W    PLAYFIELD
    0780-07FF    R/W    MAPS TO 0000-D0
    0800-0801     W     S LAMP
    0802-0803     W     K LAMP
    0804-0805     W     START LITE 1
    0806-0807     W     START LITE 2
    0808-0809     W     Y LAMP
    080A-080B     W     D LAMP
    080C-080D     W     SOUND ENABLE
    1000-1001     W     JUMP LITE 1
    1002-1003     W     COIN LOCK OUT
    1006-1007     W     JUMP LITE 2
    1008-1009     W     WHISTLE 1
    100A-100B     W     WHISTLE 2
    100C-100D     W     NMION
    100E-100F     W     WIDTH
    1800          R     D6=LEFT 1, D7=RIGHT 1
    1801          R     D6=LEFT 2, D7=RIGHT 2
    1802          R     D6=JUMP 1, D7=CHUTE 1
    1803          R     D6=JUMP 2, D7=CHUTE 2
    1804          R     D6=(D) OPT SW: NEXT TEST, D7=(F) OPT SW
    1805          R     D6=(E) OPT SW, D7= (H) OPT SW: DIAGNOSTICS
    1806          R     D6=START 1, D7=COIN 1
    1807          R     D6=START 2, D7=COIN 2
    1808          R     D6=MISSES 2, D7=MISSES 1
    1809          R     D6=COIN 2, D7=COIN1
    180A          R     D6=HARD/EASY, D7=EXTENDED PLAY
    180B          R     D6=LANGUAGE 2, D7=LANGUAGE 1
    1810          R     D6=TEST, D7=!VBLANK
    1811          R     D6=!SLAM, D7=UNUSED
    2000          W     TIMER RESET
    2002-2003     W     I LAMP
    2004-2005     W     V LAMP
    2006-2007     W     E LAMP
    2008-2009     W     R LAMP
    200A-200B     W     OCT 1
    200C-200D     W     OCT 2
    200E-200F     W     NOISE RESET
    2800-2FFF     R     ROM 0
    3000-37FF     R     ROM 1
    3800-3FFF     R     ROM 2A
    7800-7FFF     R     ROM 2B

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    Notes:

    The NMI interrupts are only used to read the coin switches.

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/skydiver.h"
#include "sound/discrete.h"

#include "skydiver.lh"

#define MASTER_CLOCK (XTAL_12_096MHz)




/*************************************
 *
 *  Palette generation
 *
 *************************************/

static const int colortable_source[] =
{
	0x02, 0x00,
	0x02, 0x01,
	0x00, 0x02,
	0x01, 0x02
};

PALETTE_INIT_MEMBER(skydiver_state, skydiver)
{
	int i;

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
	{
		rgb_t color;

		switch (colortable_source[i])
		{
		case 0:   color = rgb_t::black; break;
		case 1:   color = rgb_t::white; break;
		default:  color = rgb_t(0xa0, 0xa0, 0xa0); break; /* grey */
		}

		palette.set_pen_color(i, color);
	}
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

WRITE8_MEMBER(skydiver_state::nmion_w)
{
	m_nmion = offset;
}


INTERRUPT_GEN_MEMBER(skydiver_state::interrupt)
{
	/* Convert range data to divide value and write to sound */
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_discrete->write(space, SKYDIVER_RANGE_DATA, (0x01 << (~m_videoram[0x394] & 0x07)) & 0xff);   // Range 0-2

	m_discrete->write(space, SKYDIVER_RANGE3_EN,  m_videoram[0x394] & 0x08);       // Range 3 - note disable
	m_discrete->write(space, SKYDIVER_NOTE_DATA, ~m_videoram[0x395] & 0xff);       // Note - freq
	m_discrete->write(space, SKYDIVER_NOISE_DATA,  m_videoram[0x396] & 0x0f);  // NAM - Noise Amplitude

	if (m_nmion)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}



/*************************************
 *
 *  Sound handlers
 *
 *************************************/

WRITE8_MEMBER(skydiver_state::sound_enable_w)
{
	m_discrete->write(space, SKYDIVER_SOUND_EN, offset);
}

WRITE8_MEMBER(skydiver_state::whistle_w)
{
	m_discrete->write(space, NODE_RELATIVE(SKYDIVER_WHISTLE1_EN, (offset >> 1)), offset & 0x01);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( skydiver_map, AS_PROGRAM, 8, skydiver_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x4300) AM_READWRITE(wram_r, wram_w)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x4000) AM_RAM       /* RAM B1 */
	AM_RANGE(0x0400, 0x07ff) AM_MIRROR(0x4000) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")       /* RAMs K1,M1,P1,J1,N1,K/L1,L1,H/J1 */
	AM_RANGE(0x0800, 0x0801) AM_MIRROR(0x47f0) AM_WRITE(lamp_s_w)
	AM_RANGE(0x0802, 0x0803) AM_MIRROR(0x47f0) AM_WRITE(lamp_k_w)
	AM_RANGE(0x0804, 0x0805) AM_MIRROR(0x47f0) AM_WRITE(start_lamp_1_w)
	AM_RANGE(0x0806, 0x0807) AM_MIRROR(0x47f0) AM_WRITE(start_lamp_2_w)
	AM_RANGE(0x0808, 0x0809) AM_MIRROR(0x47f0) AM_WRITE(lamp_y_w)
	AM_RANGE(0x080a, 0x080b) AM_MIRROR(0x47f0) AM_WRITE(lamp_d_w)
	AM_RANGE(0x080c, 0x080d) AM_MIRROR(0x47f0) AM_WRITE(sound_enable_w)
	// AM_RANGE(0x1000, 0x1001) AM_MIRROR(0x47f0) AM_WRITE(jump1_lamps_w)
	AM_RANGE(0x1002, 0x1003) AM_MIRROR(0x47f0) AM_WRITE(coin_lockout_w)
	// AM_RANGE(0x1006, 0x1007) AM_MIRROR(0x47f0) AM_WRITE(jump2_lamps_w)
	AM_RANGE(0x1008, 0x100b) AM_MIRROR(0x47f0) AM_WRITE(whistle_w)
	AM_RANGE(0x100c, 0x100d) AM_MIRROR(0x47f0) AM_WRITE(nmion_w)
	AM_RANGE(0x100e, 0x100f) AM_MIRROR(0x47f0) AM_WRITE(width_w)
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x47e0) AM_READ_PORT("IN0")
	AM_RANGE(0x1801, 0x1801) AM_MIRROR(0x47e0) AM_READ_PORT("IN1")
	AM_RANGE(0x1802, 0x1802) AM_MIRROR(0x47e0) AM_READ_PORT("IN2")
	AM_RANGE(0x1803, 0x1803) AM_MIRROR(0x47e0) AM_READ_PORT("IN3")
	AM_RANGE(0x1804, 0x1804) AM_MIRROR(0x47e0) AM_READ_PORT("IN4")
	AM_RANGE(0x1805, 0x1805) AM_MIRROR(0x47e0) AM_READ_PORT("IN5")
	AM_RANGE(0x1806, 0x1806) AM_MIRROR(0x47e0) AM_READ_PORT("IN6")
	AM_RANGE(0x1807, 0x1807) AM_MIRROR(0x47e0) AM_READ_PORT("IN7")
	AM_RANGE(0x1808, 0x1808) AM_MIRROR(0x47e4) AM_READ_PORT("IN8")
	AM_RANGE(0x1809, 0x1809) AM_MIRROR(0x47e4) AM_READ_PORT("IN9")
	AM_RANGE(0x180a, 0x180a) AM_MIRROR(0x47e4) AM_READ_PORT("IN10")
	AM_RANGE(0x180b, 0x180b) AM_MIRROR(0x47e4) AM_READ_PORT("IN11")
	AM_RANGE(0x1810, 0x1810) AM_MIRROR(0x47e4) AM_READ_PORT("IN12")
	AM_RANGE(0x1811, 0x1811) AM_MIRROR(0x47e4) AM_READ_PORT("IN13")
	AM_RANGE(0x2000, 0x201f) AM_MIRROR(0x47e0) AM_READ(watchdog_reset_r) AM_WRITE(_2000_201F_w)
	AM_RANGE(0x2800, 0x2fff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0x3000, 0x37ff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0x3800, 0x3fff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( skydiver )
	PORT_START("IN0")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START("IN1")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )    /* Jump 1 */
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )    /* Chute 1 */

	PORT_START("IN3")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* Jump 2 */
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* Chute 2 */

	PORT_START("IN4")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("(D) OPT SW NEXT TEST") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("(F) OPT SW") PORT_CODE(KEYCODE_F)

	PORT_START("IN5")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("(E) OPT SW") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("(H) OPT SW DIAGNOSTICS") PORT_CODE(KEYCODE_H)

	PORT_START("IN6")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("IN7")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("IN8")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("IN9")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("IN10")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN11")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( German ) )

	PORT_START("IN12")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN13")
	PORT_BIT (0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHISTLE1")
	PORT_ADJUSTER( 33, "Whistle 1 Freq" )

	PORT_START("WHISTLE2")
	PORT_ADJUSTER( 25, "Whistle 2 Freq" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 15, 14, 13, 12 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const gfx_layout motion_layout =
{
	16,16,
	32,
	1,
	{ 0 },
	{ 4, 5, 6, 7, 4 + 0x400*8, 5 + 0x400*8, 6 + 0x400*8, 7 + 0x400*8,
		12, 13, 14, 15, 12 + 0x400*8, 13 + 0x400*8, 14 + 0x400*8, 15 + 0x400*8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	8*32
};


static GFXDECODE_START( skydiver )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, motion_layout, 0, 4 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( skydiver, skydiver_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800,MASTER_CLOCK/16)     /* ???? */
	MCFG_CPU_PROGRAM_MAP(skydiver_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(skydiver_state, interrupt,  5*60)
	MCFG_WATCHDOG_VBLANK_INIT(8)    // 128V clocks the same as VBLANK


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(skydiver_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", skydiver)
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(colortable_source))
	MCFG_PALETTE_INIT_OWNER(skydiver_state, skydiver)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(skydiver)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( skydiver )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "33167-02.f1", 0x2800, 0x0800, CRC(25a5c976) SHA1(50fbf5dceab5d78292dc14bf25f2076e8139a594) )
	ROM_LOAD( "33164-02.e1", 0x3000, 0x0800, CRC(a348ac39) SHA1(7401cbd2f7236bd1d6ad0e39eb3de2b7d75e8f45) )
	ROM_LOAD( "33165-02.d1", 0x3800, 0x0800, CRC(a1fc5504) SHA1(febaa78936de7703b708c0d1f350fe288e0a106b) )
	ROM_LOAD( "33166-02.c1", 0x7800, 0x0800, CRC(3d26da2b) SHA1(e515d5c13814b9732a6ca109272500a60edc208a) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "33163-01.h5", 0x0000, 0x0400, CRC(5b9bb7c2) SHA1(319f45b6dff96739f73f2089361239da47042dcd) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "33176-01.l5", 0x0000, 0x0400, CRC(6b082a01) SHA1(8facc94843ea041d205137056bd2035cf968125b) )
	ROM_LOAD( "33177-01.k5", 0x0400, 0x0400, CRC(f5541af0) SHA1(0967269518b6eac3c4e9ddaee39303086476c580) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL(1978, skydiver, 0, skydiver, skydiver, driver_device, 0, ROT0, "Atari", "Sky Diver", MACHINE_SUPPORTS_SAVE, layout_skydiver )
