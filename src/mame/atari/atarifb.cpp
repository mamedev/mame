// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Patrick Lawrence, Brad Oliver
/***************************************************************************

    Atari Football hardware

    driver by Mike Balfour, Patrick Lawrence, Brad Oliver

    Games supported:
        * Atari Football
        * Atari Baseball
        * Atari Soccer

    TODO:
        * these are cocktail arcade cabs, should view be rotated?
        * atarifb play-select LED sometimes remains on, or is this normal?
        * confirm sprite 'colors' in soccer

****************************************************************************

    Memory Map:
        0000-01FF   Working RAM
        0200-025F   Playfield - Player 1
        03A0-03FF   Playfield - Player 2
        1000-13BF   Scrollfield
        13C0-13FF   Motion Object Parameters:

        13C0        Motion Object 1 Picture #
        13C1        Motion Object 1 Vertical Position
        13C2        Motion Object 2 Picture #
        13C3        Motion Object 2 Vertical Position
        ...
        13DE        Motion Object 16 Picture #
        13DF        Motion Object 16 Vertical Position

        13E0        Motion Object 1 Horizontal Position
        13E1        Spare
        13E2        Motion Object 2 Horizontal Position
        13E3        Spare
        ...
        13FE        Motion Object 16 Horizontal Position
        13FF        Spare

        2000-2003   Output ports:

        2000        (OUT 0) Scrollfield Offset (8 bits)
        2001        (OUT 1)
                    D0 = Whistle
                    D1 = Hit
                    D2 = Kicker
                    D5 = CTRLD
        2002        (OUT 2)
                    D0-D3 = Noise Amplitude
                    D4 = Coin Counter
                    D5 = Attract
        2003        (OUT 3)
                    D0-D3 = LED Cathodes
                    D4-D5 Spare

        3000        Interrupt Acknowledge
        4000-4003   Input ports:

        4000        (IN 0) = 0
                    D0 = Trackball Direction PL2VD
                    D1 = Trackball Direction PL2HD
                    D2 = Trackball Direction PL1VD
                    D3 = Trackball Direction PL1HD
                    D4 = Select 1
                    D5 = Slam
                    D6 = End Screen
                    D7 = Coin 1
        4000        (CTRLD) = 1
                    D0-D3 = Track-ball Horiz. 1
                    D4-D7 = Track-ball Vert. 1
        4002        (IN 2) = 0
                    D0-D3 = Option Switches
                    D4 = Select 2
                    D5 = Spare
                    D6 = Test
                    D7 = Coin 2
        4002        (CTRLD) = 1
                    D0-D3 = Track-ball Horiz. 2
                    D4-D7 = Track-ball Vert. 2

        5000        Watchdog
        6800-7FFF   Program
        (F800-FFFF) - only needed for the 6502 vectors

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    Changes:
        LBO - lots of cleanup, now it's playable.

***************************************************************************/

#include "emu.h"
#include "atarifb.h"

#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "speaker.h"

#include "atarifb.lh"
#include "atarifb4.lh"
#include "abaseb.lh"

/*************************************
 *
 *  Interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(atarifb_state::interrupt)
{
	int scanline = param;
	m_screen->update_partial(scanline - 1);

	// periodic interrupts from _32V
	// 4 interrupts per frame, including vblank irq
	if (scanline != 0)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	// LED latch is cleared
	m_led_pwm->clear();
}

void atarifb_state::intack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void atarifb_state::atarifb_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x025f).ram().w(FUNC(atarifb_state::atarifb_alpha1_videoram_w)).share("p1_videoram");
	map(0x0260, 0x039f).ram();
	map(0x03a0, 0x03ff).ram().w(FUNC(atarifb_state::atarifb_alpha2_videoram_w)).share("p2_videoram");
	map(0x1000, 0x13bf).ram().w(FUNC(atarifb_state::atarifb_field_videoram_w)).share("field_videoram");
	map(0x13c0, 0x13ff).ram().share("spriteram");
	map(0x2000, 0x2000).writeonly().share("scroll_register"); /* OUT 0 */
	map(0x2001, 0x2001).w(FUNC(atarifb_state::atarifb_out1_w)); /* OUT 1 */
	map(0x2002, 0x2002).w(FUNC(atarifb_state::atarifb_out2_w)); /* OUT 2 */
	map(0x2003, 0x2003).w(FUNC(atarifb_state::atarifb_out3_w)); /* OUT 3 */
	map(0x3000, 0x3000).w(FUNC(atarifb_state::intack_w));
	map(0x4000, 0x4000).r(FUNC(atarifb_state::atarifb_in0_r));
	map(0x4002, 0x4002).r(FUNC(atarifb_state::atarifb_in2_r));
	map(0x5000, 0x5000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x6000, 0x7fff).rom();
}


void atarifb_state::atarifb4_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x025f).ram().w(FUNC(atarifb_state::atarifb_alpha1_videoram_w)).share("p1_videoram");
	map(0x0260, 0x039f).ram();
	map(0x03a0, 0x03ff).ram().w(FUNC(atarifb_state::atarifb_alpha2_videoram_w)).share("p2_videoram");
	map(0x1000, 0x13bf).ram().w(FUNC(atarifb_state::atarifb_field_videoram_w)).share("field_videoram");
	map(0x13c0, 0x13ff).ram().share("spriteram");
	map(0x2000, 0x2000).writeonly().share("scroll_register"); /* OUT 0 */
	map(0x2001, 0x2001).w(FUNC(atarifb_state::atarifb4_out1_w)); /* OUT 1 */
	map(0x2002, 0x2002).w(FUNC(atarifb_state::atarifb_out2_w)); /* OUT 2 */
	map(0x2003, 0x2003).w(FUNC(atarifb_state::atarifb_out3_w)); /* OUT 3 */
	map(0x3000, 0x3000).w(FUNC(atarifb_state::intack_w));
	map(0x4000, 0x4000).r(FUNC(atarifb_state::atarifb4_in0_r));
	map(0x4001, 0x4001).portr("EXTRA");
	map(0x4002, 0x4002).r(FUNC(atarifb_state::atarifb4_in2_r));
	map(0x5000, 0x5000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x6000, 0x7fff).rom();
}


void atarifb_state::abaseb_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x025f).ram().w(FUNC(atarifb_state::atarifb_alpha1_videoram_w)).share("p1_videoram");
	map(0x0260, 0x039f).ram();
	map(0x03a0, 0x03ff).ram().w(FUNC(atarifb_state::atarifb_alpha2_videoram_w)).share("p2_videoram");
	map(0x1000, 0x13bf).ram().w(FUNC(atarifb_state::atarifb_field_videoram_w)).share("field_videoram");
	map(0x13c0, 0x13ff).ram().share("spriteram");
	map(0x2000, 0x2000).writeonly().share("scroll_register"); /* OUT 0 */
	map(0x2001, 0x2001).w(FUNC(atarifb_state::abaseb_out1_w)); /* OUT 1 */
	map(0x2002, 0x2002).w(FUNC(atarifb_state::atarifb_out2_w)); /* OUT 2 */
	map(0x2003, 0x2003).w(FUNC(atarifb_state::atarifb_out3_w)); /* OUT 3 */
	map(0x3000, 0x3000).w(FUNC(atarifb_state::intack_w));
	map(0x4000, 0x4000).r(FUNC(atarifb_state::atarifb_in0_r));
	map(0x4002, 0x4002).r(FUNC(atarifb_state::atarifb_in2_r));
	map(0x5000, 0x5000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x6000, 0x7fff).rom();
}


void atarifb_state::soccer_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x025f).ram().w(FUNC(atarifb_state::atarifb_alpha1_videoram_w)).share("p1_videoram");
	map(0x0260, 0x039f).ram();
	map(0x03a0, 0x03ff).ram().w(FUNC(atarifb_state::atarifb_alpha2_videoram_w)).share("p2_videoram");
	map(0x0800, 0x0bbf).ram().w(FUNC(atarifb_state::atarifb_field_videoram_w)).share("field_videoram");
	map(0x0bc0, 0x0bff).ram().share("spriteram");
	map(0x1000, 0x1000).writeonly().share("scroll_register"); /* OUT 0 */
	map(0x1001, 0x1001).w(FUNC(atarifb_state::soccer_out1_w)); /* OUT 1 */
	map(0x1002, 0x1002).w(FUNC(atarifb_state::soccer_out2_w)); /* OUT 2 */
	map(0x1004, 0x1004).w(FUNC(atarifb_state::intack_w));
	map(0x1005, 0x1005).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1800, 0x1800).r(FUNC(atarifb_state::atarifb4_in0_r));
	map(0x1801, 0x1801).portr("EXTRA");
	map(0x1802, 0x1802).r(FUNC(atarifb_state::atarifb4_in2_r));
	map(0x1803, 0x1803).portr("DSW1");
	map(0x2000, 0x3fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( atarifb )
	PORT_START("IN0")
	PORT_BIT ( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT ( 0x80, IP_ACTIVE_LOW,  IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, "Time Per Coin" )     PORT_DIPLOCATION("K10:1,2")
	PORT_DIPSETTING(    0x00, "1:30" )
	PORT_DIPSETTING(    0x01, "2:00" )
	PORT_DIPSETTING(    0x02, "2:30" )
	PORT_DIPSETTING(    0x03, "3:00" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "K10:3" )    /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x00, "Atari Logo" )        PORT_DIPLOCATION("K10:4") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN2")   /* IN2 - Player 1 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
	/* The lower 4 bits are the input */

	PORT_START("IN3")   /* IN3 - Player 1 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
	/* The lower 4 bits are the input */

	PORT_START("IN4")   /* IN4 - Player 2 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
	/* The lower 4 bits are the input */

	PORT_START("IN5")   /* IN5 - Player 2 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	/* The lower 4 bits are the input */
INPUT_PORTS_END


static INPUT_PORTS_START( atarifb4 )
	PORT_INCLUDE( atarifb )

	PORT_MODIFY("IN0")
	PORT_BIT ( 0xff, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN6")   /* IN6 - Player 3 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)
	/* The lower 4 bits are the input */

	PORT_START("IN7")   /* IN7 - Player 3 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
	/* The lower 4 bits are the input */

	PORT_START("IN8")   /* IN8 - Player 4 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(4)
	/* The lower 4 bits are the input */

	PORT_START("IN9")   /* IN9 - Player 4 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)
	/* The lower 4 bits are the input */

	PORT_START("EXTRA")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT ( 0x38, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
INPUT_PORTS_END


static INPUT_PORTS_START( abaseb )
	PORT_INCLUDE( atarifb )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("K10:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easiest ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "K10:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "K10:4" )        /* Listed as "Unused" */
INPUT_PORTS_END


static INPUT_PORTS_START( soccer )
	PORT_START("IN0")
	PORT_BIT ( 0xff, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2/4 Player Toggle") PORT_DIPLOCATION("SW2:4") /* Listed as "Unused" */
	PORT_DIPNAME( 0x02, 0x00, "Rule Switch" )       PORT_DIPLOCATION("SW2:3") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( German ) )
	PORT_DIPSETTING(    0x08, DEF_STR( French ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Spanish ) )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN2")   /* IN2 - Player 1 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
	/* The lower 4 bits are the input */

	PORT_START("IN3")   /* IN3 - Player 1 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
	/* The lower 4 bits are the input */

	PORT_START("IN4")   /* IN4 - Player 2 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
	/* The lower 4 bits are the input */

	PORT_START("IN5")   /* IN5 - Player 2 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	/* The lower 4 bits are the input */

	PORT_START("IN6")   /* IN6 - Player 3 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)
	/* The lower 4 bits are the input */

	PORT_START("IN7")   /* IN7 - Player 3 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
	/* The lower 4 bits are the input */

	PORT_START("IN8")   /* IN8 - Player 4 trackball, y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(4)
	/* The lower 4 bits are the input */

	PORT_START("IN9")   /* IN9 - Player 4 trackball, x */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)
	/* The lower 4 bits are the input */

	PORT_START("EXTRA")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* unused on schematics */
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* unused on schematics */
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Time per coin" )     PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "1:00" )
	PORT_DIPSETTING(    0x01, "1:20" )
	PORT_DIPSETTING(    0x02, "1:40" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x04, "2:30" )
	PORT_DIPSETTING(    0x05, "3:00" )
	PORT_DIPSETTING(    0x06, "3:30" )
	PORT_DIPSETTING(    0x07, "4:00" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "1 Coin Minimum" )
	PORT_DIPSETTING(    0x40, "2 Coin Minimum" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )    /* Listed as "Unused" */
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
	{ 15, 14, 13, 12, 7, 6, 5, 4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};


static const gfx_layout fieldlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout soccer_fieldlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};


static const gfx_layout spritelayout =
{
	8,16,
	64,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};


static const gfx_layout spritemasklayout =
{
	8,16,
	64,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};


static GFXDECODE_START( gfx_atarifb )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0x00, 0x01 )
	GFXDECODE_ENTRY( "gfx2", 0, fieldlayout, 0x02, 0x02 )
GFXDECODE_END


static GFXDECODE_START( gfx_soccer )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,         0x00, 0x01 )
	GFXDECODE_ENTRY( "gfx3", 0x0400, soccer_fieldlayout, 0x02, 0x01 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout,       0x04, 0x02 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, spritemasklayout,   0x08, 0x03 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void atarifb_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_ctrld));
	save_item(NAME(m_sign_x_1));
	save_item(NAME(m_sign_x_2));
	save_item(NAME(m_sign_x_3));
	save_item(NAME(m_sign_x_4));
	save_item(NAME(m_sign_y_1));
	save_item(NAME(m_sign_y_2));
	save_item(NAME(m_sign_y_3));
	save_item(NAME(m_sign_y_4));
	save_item(NAME(m_counter_x_in0));
	save_item(NAME(m_counter_y_in0));
	save_item(NAME(m_counter_x_in0b));
	save_item(NAME(m_counter_y_in0b));
	save_item(NAME(m_counter_x_in2));
	save_item(NAME(m_counter_y_in2));
	save_item(NAME(m_counter_x_in2b));
	save_item(NAME(m_counter_y_in2b));
}

void atarifb_state::machine_reset()
{
	m_ctrld = 0;
	m_sign_x_1 = 0;
	m_sign_y_1 = 0;
	m_sign_x_2 = 0;
	m_sign_y_2 = 0;
	m_sign_x_3 = 0;
	m_sign_y_3 = 0;
	m_sign_x_4 = 0;
	m_sign_y_4 = 0;
	m_counter_x_in0 = 0;
	m_counter_y_in0 = 0;
	m_counter_x_in0b = 0;
	m_counter_y_in0b = 0;
	m_counter_x_in2 = 0;
	m_counter_y_in2 = 0;
	m_counter_x_in2b = 0;
	m_counter_y_in2b = 0;
}

void atarifb_state::atarifb(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12.096_MHz_XTAL/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &atarifb_state::atarifb_map);
	m_maincpu->set_vblank_int("screen", FUNC(atarifb_state::irq0_line_assert)); // overrides irq_timer
	TIMER(config, "irq_timer").configure_scanline(FUNC(atarifb_state::interrupt), "screen", 0, 64);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12.096_MHz_XTAL/2, 384, 0, 304, 262, 8, 248);
	m_screen->set_screen_update(FUNC(atarifb_state::screen_update_atarifb));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_atarifb);
	PALETTE(config, m_palette, FUNC(atarifb_state::atarifb_palette), 14);

	PWM_DISPLAY(config, m_led_pwm).set_size(3, 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, atarifb_discrete).add_route(ALL_OUTPUTS, "mono", 0.18);
}


void atarifb_state::atarifb4(machine_config &config)
{
	atarifb(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &atarifb_state::atarifb4_map);
}


void atarifb_state::abaseb(machine_config &config)
{
	atarifb(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &atarifb_state::abaseb_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(atarifb_state::screen_update_abaseb));

	/* sound hardware */
	DISCRETE(config.replace(), m_discrete, abaseb_discrete).add_route(ALL_OUTPUTS, "mono", 0.24);
}


void atarifb_state::soccer(machine_config &config)
{
	atarifb(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &atarifb_state::soccer_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(atarifb_state::screen_update_soccer));
	m_gfxdecode->set_info(gfx_soccer);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( atarifb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "03302602.m1", 0x6800, 0x0800, CRC(352e35db) SHA1(ae3f1bdb274858edf203dbffe4ba2912c065cff2) )
	ROM_LOAD( "03302801.p1", 0x7000, 0x0800, CRC(a79c79ca) SHA1(7791b431e9aadb09fd286ae56699c4beda54830a) )
	ROM_LOAD( "03302702.n1", 0x7800, 0x0800, CRC(e7e916ae) SHA1(d3a188809e83c311699cb103040c4525b36a56e3) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "033029.n7", 0x0000, 0x0400, CRC(12f43dca) SHA1(a463f5068d5522ddf74052429aa6da23e5475844) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "033030.c5", 0x0000, 0x0200, CRC(eac9ef90) SHA1(0e6284392852695ab7323be82105d32f57ad00f1) )
	ROM_LOAD_NIB_HIGH( "033031.d5", 0x0000, 0x0200, CRC(89d619b8) SHA1(0af5d1f4e6f9a377dc2d49a8039866b1857af01f) )
ROM_END


ROM_START( atarifb1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "03302601.m1", 0x6800, 0x0800, CRC(f8ce7ed8) SHA1(54520d7d31c6c8f9028b7253a33aba3b2c35ae7c) )
	ROM_LOAD( "03302801.p1", 0x7000, 0x0800, CRC(a79c79ca) SHA1(7791b431e9aadb09fd286ae56699c4beda54830a) )
	ROM_LOAD( "03302701.n1", 0x7800, 0x0800, CRC(7740be51) SHA1(3f610061f081eb5589b00a496877bc58f6e0f09f) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "033029.n7", 0x0000, 0x0400, CRC(12f43dca) SHA1(a463f5068d5522ddf74052429aa6da23e5475844) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "033030.c5", 0x0000, 0x0200, CRC(eac9ef90) SHA1(0e6284392852695ab7323be82105d32f57ad00f1) )
	ROM_LOAD_NIB_HIGH( "033031.d5", 0x0000, 0x0200, CRC(89d619b8) SHA1(0af5d1f4e6f9a377dc2d49a8039866b1857af01f) )
ROM_END

ROM_START( atarifb2 ) // built from original Atari source code
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035348-01.m1", 0x6800, 0x0800, CRC(eec61633) SHA1(e578f9392d18a5dbcb678c4e922da10c87ff6670) )
	ROM_LOAD( "035350-01.p1", 0x7000, 0x0800, CRC(e3ec76d6) SHA1(2b9b544e7bf1624e6d0ec1fdc7b9d096bae87b30) )
	ROM_LOAD( "035349-01.n1", 0x7800, 0x0800, CRC(c1e9d379) SHA1(77320dd8b74447cc55ba1cc3a50e7a9085f96c76) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "033029.n7", 0x0000, 0x0400, CRC(12f43dca) SHA1(a463f5068d5522ddf74052429aa6da23e5475844) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "033030.c5", 0x0000, 0x0200, CRC(eac9ef90) SHA1(0e6284392852695ab7323be82105d32f57ad00f1) )
	ROM_LOAD_NIB_HIGH( "033031.d5", 0x0000, 0x0200, CRC(89d619b8) SHA1(0af5d1f4e6f9a377dc2d49a8039866b1857af01f) )
ROM_END

ROM_START( amerug )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "28.lm1", 0x6800, 0x0800, CRC(f8ce7ed8) SHA1(54520d7d31c6c8f9028b7253a33aba3b2c35ae7c) )
	ROM_LOAD( "26.p1",  0x7000, 0x0800, CRC(a79c79ca) SHA1(7791b431e9aadb09fd286ae56699c4beda54830a) )
	ROM_LOAD( "27.n1",  0x7800, 0x0800, CRC(7740be51) SHA1(3f610061f081eb5589b00a496877bc58f6e0f09f) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "am27s33dc.n7", 0x0000, 0x0400, CRC(12f43dca) SHA1(a463f5068d5522ddf74052429aa6da23e5475844) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "30.c5", 0x0000, 0x0200, CRC(00b6d316) SHA1(cbe094075e682b68bc79cdd3a0b3e88c6ef9f521) )
	ROM_LOAD_NIB_HIGH( "31.d5", 0x0000, 0x0200, CRC(e5ea13e4) SHA1(95e7f71765abf4063904705964042e05705774db) )
ROM_END

ROM_START( atarifb4 )
	ROM_REGION( 0x8000, "maincpu", 0 ) /* 64k for code, the ROMs are nibble-wide */
	ROM_LOAD_NIB_LOW ( "34889.m1", 0x6000, 0x0400, CRC(5c63974a) SHA1(e91f318be80d985a09ff92f4db5792290a06dc0f) )
	ROM_LOAD_NIB_HIGH( "34891.m2", 0x6000, 0x0400, CRC(9d03baa1) SHA1(1b57f39fa4d43e3f3d22f2d9a5478b5f5e4d0cb1) )
	ROM_LOAD_NIB_LOW ( "34890.n1", 0x6400, 0x0400, CRC(2deb5844) SHA1(abc7cc80d5fcac13f50f6cc550ea7a8f322434c9) )
	ROM_LOAD_NIB_HIGH( "34892.n2", 0x6400, 0x0400, CRC(ad212d2d) SHA1(df77ed3d59b497d0f4fe7b275f1cce6c4a5aa0b2) )
	ROM_LOAD_NIB_LOW ( "34885.k1", 0x6800, 0x0400, CRC(fdd272a1) SHA1(619c7b1ced1e397a4aa5fcaf0afe84c2b39ba5fd) )
	ROM_LOAD_NIB_HIGH( "34887.k2", 0x6800, 0x0400, CRC(fa2b8b52) SHA1(aff26efcf70fe63819a80977853e8f58c17cb32b) )
	ROM_LOAD_NIB_LOW ( "34886.l1", 0x6c00, 0x0400, CRC(be912ccb) SHA1(6ed05d011a1fe06831883fdbdf7153b0ec624de9) )
	ROM_LOAD_NIB_HIGH( "34888.l2", 0x6c00, 0x0400, CRC(3f8e96c1) SHA1(c188eb39a00943d9eb62b8a70ad3bd108fc768e9) )
	ROM_LOAD_NIB_LOW ( "34877.e1", 0x7000, 0x0400, CRC(fd8832fa) SHA1(83f874d5c178846bdfb7609c2738c03e3369743b) )
	ROM_LOAD_NIB_HIGH( "34879.e2", 0x7000, 0x0400, CRC(7053ffbc) SHA1(cec5efb005833da448f67b9811719099d6980dcd) )
	ROM_LOAD_NIB_LOW ( "34878.f1", 0x7400, 0x0400, CRC(329eb720) SHA1(fa9e8c25c9e20fea72d1314297b77ffe599a5a74) )
	ROM_LOAD_NIB_HIGH( "34880.f2", 0x7400, 0x0400, CRC(e0c9b4c2) SHA1(1cc0900bb62c672a870fc465f5691039bb487571) )
	ROM_LOAD_NIB_LOW ( "34881.h1", 0x7800, 0x0400, CRC(d9055541) SHA1(ffbf86c5cc325587d89e17da0560518244d3d8e9) )
	ROM_LOAD_NIB_HIGH( "34883.h2", 0x7800, 0x0400, CRC(8a912448) SHA1(1756874964eedb75e066a4d6dccecf16a652f6bb) )
	ROM_LOAD_NIB_LOW ( "34882.j1", 0x7c00, 0x0400, CRC(060c9cdb) SHA1(3c6d04c535195dfa8f8405ff8e80f4693844d1a1) )
	ROM_LOAD_NIB_HIGH( "34884.j2", 0x7c00, 0x0400, CRC(aa699a3a) SHA1(2c13eb9cda3fe9cfd348ef5cf309625f77c75056) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "033029.n7", 0x0000, 0x0400, CRC(12f43dca) SHA1(a463f5068d5522ddf74052429aa6da23e5475844) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "033030.c5", 0x0000, 0x0200, CRC(eac9ef90) SHA1(0e6284392852695ab7323be82105d32f57ad00f1) )
	ROM_LOAD_NIB_HIGH( "033031.d5", 0x0000, 0x0200, CRC(89d619b8) SHA1(0af5d1f4e6f9a377dc2d49a8039866b1857af01f) )
ROM_END


ROM_START( abaseb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "34738-01.n0", 0x6000, 0x0800, CRC(edcfffe8) SHA1(a445668352da5039ed1a090bcdf2ce092215f165) )
	ROM_LOAD( "34737-03.m1", 0x6800, 0x0800, CRC(7250863f) SHA1(83ec735a60d74ca9c3e3f5d4b248071f3e3330af) )
	ROM_LOAD( "34735-01.p1", 0x7000, 0x0800, CRC(54854d7c) SHA1(536d57b00929bf9d1cd1b209b41004cb78e2cd93) )
	ROM_LOAD( "34736-01.n1", 0x7800, 0x0800, CRC(af444eb0) SHA1(783293426cec6938a2cd9c66c491f073cfb2683f) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "034710.d5", 0x0000, 0x0400, CRC(31275d86) SHA1(465ff2032e62bcd5a7bb5c947212da4ea4d59353) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "034708.n7", 0x0000, 0x0200, CRC(8a0f971b) SHA1(f7de50eeb15c8291f1560e299e3b1b29bba58422) )
	ROM_LOAD_NIB_HIGH( "034709.c5", 0x0000, 0x0200, CRC(021d1067) SHA1(da0fa8e4f6c0240a4feb41312fa057c65d809e62) )
ROM_END


ROM_START( abaseb2 )
	ROM_REGION( 0x8000, "maincpu", 0 ) /* 64k for code, the ROMs are nibble-wide */
	ROM_LOAD_NIB_LOW ( "034725.c0", 0x6000, 0x0400, CRC(95912c58) SHA1(cb15b60e31ee212e30a81c170611be1e36d2a6dd) )
	ROM_LOAD_NIB_HIGH( "034723.m0", 0x6000, 0x0400, CRC(5eb1597f) SHA1(78f83d4e79de13d3723732d68738660c3f8d4787) )
	ROM_LOAD_NIB_LOW ( "034726.b0", 0x6400, 0x0400, CRC(1f8d506c) SHA1(875464ca2ee50b36ceb5989cd40a28c69953c641) )
	ROM_LOAD_NIB_HIGH( "034724.l0", 0x6400, 0x0400, CRC(ecd18ed2) SHA1(6ffbc9a4108ebf190455fad3725b72dda4125ac7) )
	ROM_LOAD_NIB_LOW ( "034721.d1", 0x6800, 0x0400, CRC(1a0541f2) SHA1(ba74f024deb173678166262c4c6b1c328248aa9a) )
	ROM_LOAD_NIB_HIGH( "034715.h1", 0x6800, 0x0400, CRC(accb96f5) SHA1(1cd6603c818dacf4f71fc350ebd3adf3369056b2) ) /* created from 8-bit set */
	ROM_LOAD_NIB_LOW ( "034722.d0", 0x6c00, 0x0400, CRC(f9c1174e) SHA1(9d1be9ce4985edd19e0969d8998946d05fbbdf1f) ) /* The code in these 2 differs */
	ROM_LOAD_NIB_HIGH( "034716.h0", 0x6c00, 0x0400, CRC(d5622749) SHA1(6a48d428751939857be6869b44a61b8f054d4206) ) /* from the 8-bit set */
	ROM_LOAD_NIB_LOW ( "034717.f1", 0x7000, 0x0400, CRC(c941f64b) SHA1(e4d309c8ae71adc42dab0ffeea8f58da310c52f3) )
	ROM_LOAD_NIB_HIGH( "034711.k1", 0x7000, 0x0400, CRC(fab61782) SHA1(01b6de2822d09ebe0725307eeeaeb667f53ca8f1) )
	ROM_LOAD_NIB_LOW ( "034718.f0", 0x7400, 0x0400, CRC(3fe7dc1c) SHA1(91c3af7d8acdb5c4275f5fa57c19dc589f4a63aa) )
	ROM_LOAD_NIB_HIGH( "034712.k0", 0x7400, 0x0400, CRC(0e368e1a) SHA1(29bbe4be07d8d441a4251ed6fbfa9e225487c2d8) )
	ROM_LOAD_NIB_LOW ( "034719.e1", 0x7800, 0x0400, CRC(85046ee5) SHA1(2e8559349460a44734c95a1440a84713c5344495) )
	ROM_LOAD_NIB_HIGH( "034713.j1", 0x7800, 0x0400, CRC(0c67c48d) SHA1(eec24da32632c1ba00aee22f1b9abb144b38cc8a) )
	ROM_LOAD_NIB_LOW ( "034720.e0", 0x7c00, 0x0400, CRC(37c5f149) SHA1(89ad4471b949f8318abbdb38c4f373f711130198) )
	ROM_LOAD_NIB_HIGH( "034714.j0", 0x7c00, 0x0400, CRC(920979ea) SHA1(aba499376c084b8ceb6f0cc6599bd51cec133cc7) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "034710.n7", 0x0000, 0x0400, CRC(31275d86) SHA1(465ff2032e62bcd5a7bb5c947212da4ea4d59353) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "034708.c5", 0x0000, 0x0200, CRC(8a0f971b) SHA1(f7de50eeb15c8291f1560e299e3b1b29bba58422) )
	ROM_LOAD_NIB_HIGH( "034709.d5", 0x0000, 0x0200, CRC(021d1067) SHA1(da0fa8e4f6c0240a4feb41312fa057c65d809e62) )
ROM_END


ROM_START( soccer )
	ROM_REGION( 0x4000, "maincpu", 0 ) /* 64k for code, the ROMs are nibble-wide */
	ROM_LOAD_NIB_LOW ( "035222.e1", 0x2000, 0x0400, CRC(03ec6bce) SHA1(f81f2ac3bab5f1ae687543427e0187ca51d3be7e) )
	ROM_LOAD_NIB_HIGH( "035224.e2", 0x2000, 0x0400, CRC(a1aeaa70) SHA1(2018318a0e652b1dbea7696ef3dc2b7f12ebd632) )
	ROM_LOAD_NIB_LOW ( "035223.f1", 0x2400, 0x0400, CRC(9c600726) SHA1(f652b42b93e43124b0363b52f0f13cb9154987e3) )
	ROM_LOAD_NIB_HIGH( "035225.f2", 0x2400, 0x0400, CRC(2aa06521) SHA1(c03b02f62346a8e395f8c4b15f6f89fd96b790a4) )
	ROM_LOAD_NIB_LOW ( "035226.h1", 0x2800, 0x0400, CRC(d57c0cfb) SHA1(9ce05d9b30e8014137e20e4b0bbe414a3b9fa600) )
	ROM_LOAD_NIB_HIGH( "035228.h2", 0x2800, 0x0400, CRC(594574cb) SHA1(c8b42a44520e6a2a3e8831e9f9002c3c532f5fca) )
	ROM_LOAD_NIB_LOW ( "035227.j1", 0x2c00, 0x0400, CRC(4112b257) SHA1(997f4681a5cd4ca12977c52133e847afe61c58e1) )
	ROM_LOAD_NIB_HIGH( "035229.j2", 0x2c00, 0x0400, CRC(412d129c) SHA1(2680af645aa6935114e59c657e49b131e48661fc) )

	ROM_LOAD_NIB_LOW ( "035230.k1", 0x3000, 0x0400, CRC(747f6e4a) SHA1(b0cd8097e064ba6b0e22e97a7907bc287006aa8c) )
	ROM_LOAD_NIB_HIGH( "035232.k2", 0x3000, 0x0400, CRC(55f43e7f) SHA1(db44f658a521f859f11f9a638ba19e84bbb75d2d) )
	ROM_LOAD_NIB_LOW ( "035231.l1", 0x3400, 0x0400, CRC(d584c199) SHA1(55e86e4f1737bf02d5706f1e757d9c97007549ac) )
	ROM_LOAD_NIB_HIGH( "035233.l2", 0x3400, 0x0400, CRC(b343f500) SHA1(d15413759563bec2bc8f3fa28ae84e4ae902910b) )
	ROM_LOAD_NIB_LOW ( "035234.m1", 0x3800, 0x0400, CRC(83524bb7) SHA1(d45233b666463f789257c7366c3dfb4d9b55f87e) )
	ROM_LOAD_NIB_HIGH( "035236.m2", 0x3800, 0x0400, CRC(c53f4d13) SHA1(ebba48e50c98e7f74d19826cf559cf6633e24f3b) )
	ROM_LOAD_NIB_LOW ( "035235.n1", 0x3c00, 0x0400, CRC(d6855b0e) SHA1(379d010ebebde6f1b5fec5519a3c0aa4380be28b) )
	ROM_LOAD_NIB_HIGH( "035237.n2", 0x3c00, 0x0400, CRC(1d01b054) SHA1(7f3dc1130b2aadb13813e223420672c5baf25ad8) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD_NIB_LOW ( "035250.r2", 0x0000, 0x0400, CRC(12f43dca) SHA1(a463f5068d5522ddf74052429aa6da23e5475844) ) /* characters */

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD_NIB_LOW ( "035247.n7", 0x0000, 0x0400, CRC(3adb5f4e) SHA1(859df5dc97b06e0c06e4f71a511313aef1f08d87) ) /* sprites */
	ROM_LOAD_NIB_HIGH( "035248.m7", 0x0000, 0x0400, CRC(a890cd48) SHA1(34f52bc4b610491d3b81caae25ec3cafbc429373) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "035246.r6", 0x0000, 0x0800, CRC(4a996136) SHA1(535b6d5f70ab5bc2a47263a1c16877ba4c82b3ff) ) /* spritemask - playfield */

	/* ROM_LOAD_NIB_LOW ( "35242-01.r7", 0x0000, 0x0400, CRC(d783e3d3) SHA1(1753b907735ca75c4622e003ec0df5f32e886783) ) */
	/* ROM_LOAD_NIB_HIGH( "35243-01.r8", 0x0000, 0x0400, CRC(b63ae27c) SHA1(6106765871832e9fbe440fca2d8df1b6700d8b85) ) */
	/* ROM_LOAD_NIB_LOW ( "35244-01.p7", 0x0400, 0x0400, CRC(5236ae38) SHA1(472b51635a050ea1913d745fcfc07f30ce9afa09) ) */
	/* ROM_LOAD_NIB_HIGH( "35245-01.p8", 0x0400, 0x0400, CRC(e4c6fd5c) SHA1(62c80735726c4e11229438c27153203105b9cb66) ) */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "34006-01.n4", 0x0000, 0x0100, CRC(0da9918e) SHA1(405ee4396a20d05ecde27388093139d087d81f19) ) /* sync (not used) */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/*     YEAR  NAME      PARENT   MACHINE   INPUT */
GAMEL( 1978, atarifb,  0,       atarifb,  atarifb,  atarifb_state, empty_init, ROT0, "Atari", "Atari Football (revision 2)", MACHINE_SUPPORTS_SAVE, layout_atarifb )
GAMEL( 1978, atarifb1, atarifb, atarifb,  atarifb,  atarifb_state, empty_init, ROT0, "Atari", "Atari Football (revision 1)", MACHINE_SUPPORTS_SAVE, layout_atarifb )
GAMEL( 1978, atarifb2, atarifb, atarifb,  atarifb,  atarifb_state, empty_init, ROT0, "Atari", "Atari Football II", MACHINE_SUPPORTS_SAVE, layout_atarifb )
GAMEL( 1980, amerug,   atarifb, atarifb,  atarifb,  atarifb_state, empty_init, ROT0, "Shoei", "Amerug (Shoei bootleg of Atari Football)", MACHINE_SUPPORTS_SAVE, layout_atarifb )
GAMEL( 1979, atarifb4, atarifb, atarifb4, atarifb4, atarifb_state, empty_init, ROT0, "Atari", "Atari 4 Player Football", MACHINE_SUPPORTS_SAVE, layout_atarifb4 )
GAMEL( 1979, abaseb,   0,       abaseb,   abaseb,   atarifb_state, empty_init, ROT0, "Atari", "Atari Baseball (set 1)", MACHINE_SUPPORTS_SAVE, layout_abaseb )
GAMEL( 1979, abaseb2,  abaseb,  abaseb,   abaseb,   atarifb_state, empty_init, ROT0, "Atari", "Atari Baseball (set 2)", MACHINE_SUPPORTS_SAVE, layout_abaseb )
GAME(  1980, soccer,   0,       soccer,   soccer,   atarifb_state, empty_init, ROT0, "Atari", "Atari Soccer", MACHINE_SUPPORTS_SAVE )
