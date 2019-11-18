// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

Namco early 8080-based games

All these games run on the same hardware design, with minor differences.

Gee Bee was the very first videogame released by Namco, and runs on a b&w board.
The year after, the design was upgraded to add color and a second sound channel;
however, the b&w board was not abandoned, and more games were written for it with
the only difference of a larger gfx ROM (losing the ability to choose full or half
brightness for each character).


Gee Bee memory map
Navarone, Kaitei, SOS are the same but CGROM is twice as large

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
-000xxxxxxxxxxxx R   xxxxxxxx ROM2/ROM0 program ROM
-001xxxxxxxxxxxx R   xxxxxxxx ROM1      program ROM
-010--xxxxxxxxxx R/W xxxxxxxx VRAM      tile code
-011--xxxxxxxxxx R   xxxxxxxx CGROM     Character Generator ROM
-100----xxxxxxxx R/W xxxxxxxx RAM       work RAM
-101----------00*R   --xxxxxx SW READ 0 switch inputs bank #1 (coins, buttons)
-101----------01*R   --xxxxxx SW READ 1 switch inputs bank #2 (optional, only kaiteik uses it)
-101----------10*R   --xxxxxx SW READ 2 dip switches
-101----------11*R   xxxxxxxx VOL READ  paddle input (digital joystick is hacked in for the other games)
-110----------00*  W xxxxxxxx BHD       ball horizontal position
-110----------01*  W xxxxxxxx BVD       ball vertical position
-110----------10*  W -------- n.c.
-110----------11*  W ----xxxx SOUND     select frequency and wave shape for sound
-111---------000*  W -------x LAMP 1    player 1 start lamp
-111---------001*  W -------x LAMP 2    player 2 start lamp
-111---------010*  W -------x LAMP 3    serve lamp
-111---------011*  W -------x COUNTER   coin counter
-111---------100*  W -------x LOCK OUT COIL  coin lock out
-111---------101*  W -------x BGW       invert CGROM data
-111---------110*  W -------x BALL ON   ball enable
-111---------111*  W -------x INV       screen flip

* = usually accessed using the in/out instructions

Bomb Bee / Cutie Q memory map
Warp Warp is the same but A13 and A15 are swapped to make space for more program ROM.

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
-00xxxxxxxxxxxxx R   xxxxxxxx ROM       program ROM
-01---xxxxxxxxxx R/W xxxxxxxx RAM       work RAM
-10-00xxxxxxxxxx R/W xxxxxxxx V-RAM 1   tile code
-10-01xxxxxxxxxx R/W xxxxxxxx V-RAM 2   tile color
-10-1xxxxxxxxxxx R   xxxxxxxx CGROM     Character Generator ROM
-11-------00-xxx R   -------x SW READ   switch inputs (coins, buttons)
-11-------01---- R   xxxxxxxx VOL READ  paddle input (digital joystick is hacked in for warpwarp)
-11-------10-xxx R   -------x DIP SW 1  dip switches bank #1
-11-------11-xxx R   -------x DIP SW 2  dip switches bank #2 (optional, not used by any game)
-11-------00--00   W xxxxxxxx BHD       ball horizontal position
-11-------00--01   W xxxxxxxx BVD       ball vertical position
-11-------00--10   W ----xxxx SOUND     select frequency and wave shape for sound channel #1
-11-------00--11   W --------    WATCH DOG RESET
-11-------01----   W --xxxxxx MUSIC 1   sound channel #2 frequency
-11-------10----   W --xxxxxx MUSIC 2   sound channel #2 shape
-11-------11-000   W -------x LAMP 1    player 1 start lamp
-11-------11-001   W -------x LAMP 2    player 2 start lamp
-11-------11-010   W -------x LAMP 3    serve lamp (not used by warp warp)
-11-------11-011   W -------x n.c.
-11-------11-100   W -------x LOCK OUT  coin lock out
-11-------11-101   W -------x COUNTER   coin counter
-11-------11-110   W -------x BALL ON   ball enable + irq enable
-11-------11-111   W -------x INV       screen flip


Notes:
- Warp Warp Easter egg:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    2xR 6xD L 4xU
  (c) 1981 NAMCO LTD. will be added at the bottom of the screen.

- In the pinball games, there isn't a player 2 "serve" button - both players use
  the same input. I think this is correct behaviour, because there is nothing in
  the schematics suggesting otherwise (while there is a provision to switch from
  one paddle to the other). The Bomb Bee flyer shows that the cocktail cabinet
  did have one serve button per player, but I guess they were just wired together
  on the same input.

- The ball size changes from game to game. I have determined what I believe are
  the correct sizes by checking how the games handle the ball position in
  cocktail mode (the ball isn't automatically flipped by the hardware).

- kaitein and kaitei are intriguing. The game is more or less the same, but the
  code is radically different, and the gfx ROMs are arranged differently.
  kaitein is by Namco and kaitei is by "K'K-TOKKI". kaitein does a ROM/RAM
  test on startup, while kaitei jumps straight into the game. kaitein is
  locked in cocktail mode, while kaitei has support for a cabinet dip
  switch. The title screen in kaitein is nice and polished, while in kaitei
  it's confused, with fishes going over the text. There are several other
  differences.
  The code in kaitei is longer (0x1800 bytes vs. just 0x1000 in kaitein) and
  less efficient, while kaitein has some clever space optimizations.

  Kaitei (and SOS) is developed by Kazuharu Yoshioka of K.K.Tokki (Universal Tokki).
  Namco acquired license for these games.

- The coin counter doesn't work in kaitei. This might be the expected behaviour.

- sos does a "coffee break" every 2000 points, showing a girl. The first times,
  she wears a bikini. If the "nudity" switch is on, after 6000 points she's
  topless and every 10000 points she's nude.

- The only difference between 'warpwarpr' and 'warpwarpr2' is the copyright
  string on the first screen (when the scores are displayed) :
  * 'warpwarpr' : "(c) 1981 ROCK-OLA MFG.CORP."  (text stored at 0x33ff to 0x3417)
  * 'warpwarp2' : "(c) 1981 ROCK-OLA MFG.CO."    (text stored at 0x33ff to 0x3415)
  Note that the checksum at 0x37ff (used for checking ROM at 0x3000 to 0x37ff)
  is different of course.

- warpwarpr doesn't have an explicit Namco copyright notice, but in the default
  high score table the names are NNN AAA MMM CCC OOO. warpwarp doesn't have an
  high score table at all.

- There are images/videos on the net of kaitei and geebee running in 3bpp
  (aka 7 colors), this is assumed to be a homebrew repair or hack.
  Update: Gee Bee definitely uses an overlay while kaitei actually outputs
  colors depending on the monitor type used (color or b&w).
  sos is the odd one: it seems to be running in a b&w environment but a pic
  on the net shows it with reversed black and white compared to current
  implementation.
  Also the flyer shows the girl to be colorized purple, while the points
  numbers are in cyan and text in red (and this arrangement doesn't
  make much sense, different version maybe?).

***************************************************************************/

#include "emu.h"
#include "includes/warpwarp.h"

#include "cpu/i8085/i8085.h"
#include "screen.h"
#include "speaker.h"

#include "geebee.lh"
#include "navarone.lh"

#define MASTER_CLOCK        XTAL(18'432'000)

void warpwarp_state::machine_start()
{
	save_item(NAME(m_geebee_bgw));
	save_item(NAME(m_ball_on));
	save_item(NAME(m_ball_h));
	save_item(NAME(m_ball_v));
}

/* Interrupt Gen */
WRITE_LINE_MEMBER(warpwarp_state::vblank_irq)
{
	if (state && m_ball_on)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

/* B&W Games I/O */
READ8_MEMBER(warpwarp_state::geebee_in_r)
{
	int res;

	offset &= 3;
	res = m_ports[offset].read_safe(0);
	if (offset == 3)
	{
		res = (flip_screen() & 1) ? m_in2->read() : m_in1->read();  // read player 2 input in cocktail mode
		if (m_handle_joystick)
		{
			/* map digital two-way joystick to two fixed VOLIN values */
			if (res & 2) return 0x9f;
			if (res & 1) return 0x0f;
			return 0x60;
		}
	}
	return res;
}

WRITE8_MEMBER(warpwarp_state::geebee_out6_w)
{
	switch (offset & 3)
	{
		case 0:
			m_ball_h = data;
			break;
		case 1:
			m_ball_v = data;
			break;
		case 2:
			/* n.c. */
			break;
		case 3:
			m_geebee_sound->sound_w(data);
			break;
	}
}

WRITE_LINE_MEMBER(warpwarp_state::counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(warpwarp_state::lock_out_w)
{
	machine().bookkeeping().coin_lockout_global_w(!state);
}

WRITE_LINE_MEMBER(warpwarp_state::geebee_bgw_w)
{
	m_geebee_bgw = state;
	machine().tilemap().mark_all_dirty();
}

WRITE_LINE_MEMBER(warpwarp_state::ball_on_w)
{
	m_ball_on = state;
	if (!state)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(warpwarp_state::inv_w)
{
	flip_screen_set(state);
}


/* Color Games I/O */

/* Read Switch Inputs */
READ8_MEMBER(warpwarp_state::warpwarp_sw_r)
{
	return (m_in0->read() >> (offset & 7)) & 1;
}

/* Read Dipswitches */
READ8_MEMBER(warpwarp_state::warpwarp_dsw1_r)
{
	return (m_dsw1->read() >> (offset & 7)) & 1;
}

/* Read mux Controller Inputs */
READ8_MEMBER(warpwarp_state::warpwarp_vol_r)
{
	int res;

	res = (flip_screen() & 1) ?  m_volin2->read() : m_volin1->read();
	if (m_handle_joystick)
	{
		if (res & 1) return 0x0f;
		if (res & 2) return 0x3f;
		if (res & 4) return 0x6f;
		if (res & 8) return 0x9f;
		return 0xff;
	}
	return res;
}

WRITE8_MEMBER(warpwarp_state::warpwarp_out0_w)
{
	switch (offset & 3)
	{
		case 0:
			m_ball_h = data;
			break;
		case 1:
			m_ball_v = data;
			break;
		case 2:
			m_warpwarp_sound->sound_w(data);
			break;
		case 3:
			m_watchdog->watchdog_reset();
			break;
	}
}



void warpwarp_state::geebee_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).mirror(0x400).ram().w(FUNC(warpwarp_state::geebee_videoram_w)).share("geebee_videoram"); // mirror used by kaitei due to a bug
	map(0x3000, 0x37ff).rom().region("gfx1", 0); // 3000-33ff in geebee
	map(0x4000, 0x40ff).ram();
	map(0x5000, 0x53ff).r(FUNC(warpwarp_state::geebee_in_r));
	map(0x6000, 0x6fff).w(FUNC(warpwarp_state::geebee_out6_w));
	map(0x7000, 0x7007).mirror(0x0ff8).w(m_latch, FUNC(ls259_device::write_d0));
}

void warpwarp_state::geebee_port_map(address_map &map)
{
	map(0x50, 0x53).r(FUNC(warpwarp_state::geebee_in_r));
	map(0x60, 0x6f).w(FUNC(warpwarp_state::geebee_out6_w));
	map(0x70, 0x77).mirror(0x08).w(m_latch, FUNC(ls259_device::write_d0));
}


void warpwarp_state::bombbee_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x47ff).ram().w(FUNC(warpwarp_state::warpwarp_videoram_w)).share("videoram");
	map(0x4800, 0x4fff).rom().region("gfx1", 0);
	map(0x6000, 0x600f).rw(FUNC(warpwarp_state::warpwarp_sw_r), FUNC(warpwarp_state::warpwarp_out0_w));
	map(0x6010, 0x601f).r(FUNC(warpwarp_state::warpwarp_vol_r)).w(m_warpwarp_sound, FUNC(warpwarp_sound_device::music1_w));
	map(0x6020, 0x602f).r(FUNC(warpwarp_state::warpwarp_dsw1_r)).w(m_warpwarp_sound, FUNC(warpwarp_sound_device::music2_w));
	map(0x6030, 0x6037).mirror(0x0008).w(m_latch, FUNC(ls259_device::write_d0));
}

void warpwarp_state::warpwarp_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0x4000, 0x47ff).ram().w(FUNC(warpwarp_state::warpwarp_videoram_w)).share("videoram");
	map(0x4800, 0x4fff).rom().region("gfx1", 0);
	map(0xc000, 0xc00f).rw(FUNC(warpwarp_state::warpwarp_sw_r), FUNC(warpwarp_state::warpwarp_out0_w));
	map(0xc010, 0xc01f).r(FUNC(warpwarp_state::warpwarp_vol_r)).w(m_warpwarp_sound, FUNC(warpwarp_sound_device::music1_w));
	map(0xc020, 0xc02f).r(FUNC(warpwarp_state::warpwarp_dsw1_r)).w(m_warpwarp_sound, FUNC(warpwarp_sound_device::music2_w));
	map(0xc030, 0xc037).mirror(0x0008).w(m_latch, FUNC(ls259_device::write_d0));
}



static INPUT_PORTS_START( geebee )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x10, "Replay" )            PORT_DIPLOCATION("DSW2:5,6")    // awards 1 credit
	PORT_DIPSETTING(    0x10, "40k 80k" )           PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, "70k 140k" )          PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, "100k 200k" )         PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )     PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "60k 120k" )          PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x20, "100k 200k" )         PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x30, "150k 300k" )         PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )     PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x58, IPT_PADDLE ) PORT_MINMAX(0x10,0xa0) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IN2")   /* Cocktail */
	PORT_BIT( 0xff, 0x58, IPT_PADDLE ) PORT_MINMAX(0x10,0xa0) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( geebeeb )
	PORT_INCLUDE( geebee )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x30, 0x10, "Replay" )        // awards 1 credit
	PORT_DIPSETTING(    0x10, "40k" )           PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, "70k" )           PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, "100k" )          PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) ) PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "60k" )           PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x20, "100k" )          PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x30, "150k" )          PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) ) PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
INPUT_PORTS_END

static INPUT_PORTS_START( navarone )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x04, "5000" )                  PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "6000" )                  PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0c, "7000" )                  PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSW2", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "6000" )                  PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x08, "7000" )                  PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x0c, "8000" )                  PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSW2", 0x02, EQUALS, 0x02)
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* Fake input port to support digital joystick */
	PORT_BIT( 0x01, 0x00, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, 0x00, IPT_JOYSTICK_LEFT )

	PORT_START("IN2")   /* Fake input port to support digital joystick */
	PORT_BIT( 0x01, 0x00, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, 0x00, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( kaitei_config )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "Monochrome" )
	PORT_CONFSETTING(    0x01, "Color" )
INPUT_PORTS_END

static INPUT_PORTS_START( kaitein )
	PORT_INCLUDE( kaitei_config )

	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("SW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "4000" )
	PORT_DIPSETTING(    0x0c, "6000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("IN1")   /* Fake input port to support digital joystick */
	PORT_BIT( 0x01, 0x00, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, 0x00, IPT_JOYSTICK_LEFT )

	PORT_START("IN2")   /* Fake input port to support digital joystick */
	PORT_BIT( 0x01, 0x00, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, 0x00, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( kaitei )
	PORT_INCLUDE( kaitei_config )

	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0xf2, 0xa0, IPT_UNKNOWN ) // game verifies these bits and freezes if they don't match

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, 0x80, IPT_UNKNOWN ) // game verifies these two bits and freezes if they don't match

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "4000" )
	PORT_DIPSETTING(    0x10, "6000" )
	PORT_DIPSETTING(    0x08, "8000" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW2:6" )
	PORT_BIT( 0xc0, 0x80, IPT_UNKNOWN ) // game verifies these two bits and freezes if they don't match

	PORT_START("IN1")
	PORT_BIT( 0x3f, 0x00, IPT_UNKNOWN )
	PORT_BIT( 0xc0, 0x80, IPT_UNKNOWN ) // game verifies these two bits and freezes if they don't match

	PORT_START("IN2")
	PORT_BIT( 0x3f, 0x00, IPT_UNKNOWN )
	PORT_BIT( 0xc0, 0x80, IPT_UNKNOWN ) // game verifies these two bits and freezes if they don't match
INPUT_PORTS_END

static INPUT_PORTS_START( sos )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1   )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail) )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Lives ) )    PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x20, "Nudity" )            PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* Fake input port to support digital joystick */
	PORT_BIT( 0x01, 0x00, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, 0x00, IPT_JOYSTICK_LEFT )

	PORT_START("IN2")   /* Fake input port to support digital joystick */
	PORT_BIT( 0x01, 0x00, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, 0x00, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( bombbee )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
//  PORT_DIPSETTING(    0x08, "4" ) // duplicated setting
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW1:5" )
	PORT_DIPNAME( 0xe0, 0x00, "Replay" )            PORT_DIPLOCATION("DSW1:6,7,8") // awards 1 credit
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x40, "70000" )
	PORT_DIPSETTING(    0x60, "80000" )
	PORT_DIPSETTING(    0x80, "100000" )
	PORT_DIPSETTING(    0xa0, "120000" )
	PORT_DIPSETTING(    0xc0, "150000" )
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )

	PORT_START("VOLIN1")    /* Mux input - player 1 controller - handled by warpwarp_vol_r */
	PORT_BIT( 0xff, 0x60, IPT_PADDLE ) PORT_MINMAX(0x14,0xac) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("VOLIN2")    /* Mux input - player 2 controller - handled by warpwarp_vol_r */
	PORT_BIT( 0xff, 0x60, IPT_PADDLE ) PORT_MINMAX(0x14,0xac) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( cutieq )
	PORT_INCLUDE( bombbee )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW1:6,7,8")
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x40, "80000" )
	PORT_DIPSETTING(    0x60, "100000" )
	PORT_DIPSETTING(    0x80, "120000" )
	PORT_DIPSETTING(    0xa0, "150000" )
	PORT_DIPSETTING(    0xc0, "200000" )
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( warpwarp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, "8k 30k 30k+" )           PORT_CONDITION("DSW1", 0x0c, NOTEQUALS, 0x0c)
	PORT_DIPSETTING(    0x10, "10k 40k 40k+" )          PORT_CONDITION("DSW1", 0x0c, NOTEQUALS, 0x0c)
	PORT_DIPSETTING(    0x20, "15k 60k 60k+" )          PORT_CONDITION("DSW1", 0x0c, NOTEQUALS, 0x0c)
	PORT_DIPSETTING(0x30, DEF_STR( None ) )             PORT_CONDITION("DSW1", 0x0c, NOTEQUALS, 0x0c)
	PORT_DIPSETTING(    0x00, "30k" )                   PORT_CONDITION("DSW1", 0x0c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x10, "40k" )                   PORT_CONDITION("DSW1", 0x0c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x20, "60k" )                   PORT_CONDITION("DSW1", 0x0c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )         PORT_CONDITION("DSW1", 0x0c, EQUALS, 0x0c)
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* when level selection is On, press 1 to increase level */
	PORT_DIPNAME( 0x80, 0x80, "Level Selection (Cheat)")    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("VOLIN1")    /* FAKE - input port to simulate an analog stick - handled by warpwarp_vol_r */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_START("VOLIN2")    /* FAKE - input port to simulate an analog stick - handled by warpwarp_vol_r */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END

/* has High Score Initials dip switch instead of rack test */
static INPUT_PORTS_START( warpwarpr )
	PORT_INCLUDE( warpwarp )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "High Score Initials" )   PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_1k )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_2k )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 2 )
GFXDECODE_END

static GFXDECODE_START( gfx_color )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 256 )
GFXDECODE_END



void warpwarp_state::geebee(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, MASTER_CLOCK/9); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &warpwarp_state::geebee_map);
	m_maincpu->set_addrmap(AS_IO, &warpwarp_state::geebee_port_map);

	LS259(config, m_latch); // 5N
	m_latch->q_out_cb<0>().set_output("led0"); // LAMP 1
	m_latch->q_out_cb<1>().set_output("led1"); // LAMP 2
	m_latch->q_out_cb<2>().set_output("led2"); // LAMP 3
	m_latch->q_out_cb<3>().set(FUNC(warpwarp_state::counter_w));
	m_latch->q_out_cb<4>().set(FUNC(warpwarp_state::lock_out_w));
	m_latch->q_out_cb<5>().set(FUNC(warpwarp_state::geebee_bgw_w));
	m_latch->q_out_cb<6>().set(FUNC(warpwarp_state::ball_on_w));
	m_latch->q_out_cb<7>().set(FUNC(warpwarp_state::inv_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/3, 384, 0, 272, 264, 0, 224);
	screen.set_screen_update(FUNC(warpwarp_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(warpwarp_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_1k);
	PALETTE(config, m_palette, FUNC(warpwarp_state::geebee_palette), 4*2);

	MCFG_VIDEO_START_OVERRIDE(warpwarp_state,geebee)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GEEBEE_SOUND(config, m_geebee_sound, MASTER_CLOCK).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void warpwarp_state::geebeeb(machine_config &config)
{
	geebee(config);
	m_latch->q_out_cb<4>().set_nop(); // remove coin lockout
}

void warpwarp_state::navarone(machine_config &config)
{
	geebee(config);

	/* basic machine hardware */
	m_gfxdecode->set_info(gfx_2k);
	m_palette->set_entries(2*2);
	m_palette->set_init(FUNC(warpwarp_state::navarone_palette));

	MCFG_VIDEO_START_OVERRIDE(warpwarp_state,navarone)
}

void warpwarp_state::kaitei(machine_config &config)
{
	geebee(config);

	/* basic machine hardware */
	m_gfxdecode->set_info(gfx_1k);
	m_palette->set_entries(4*2+1);

	MCFG_MACHINE_RESET_OVERRIDE(warpwarp_state,kaitei)
}


void warpwarp_state::bombbee(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, MASTER_CLOCK/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &warpwarp_state::bombbee_map);

	LS259(config, m_latch); // 6L on Warp Warp
	m_latch->q_out_cb<0>().set_output("led0"); // LAMP 1
	m_latch->q_out_cb<1>().set_output("led1"); // LAMP 2
	m_latch->q_out_cb<2>().set_output("led2"); // LAMP 3
	m_latch->q_out_cb<3>().set_nop(); // n.c.
	m_latch->q_out_cb<4>().set(FUNC(warpwarp_state::lock_out_w));
	m_latch->q_out_cb<5>().set(FUNC(warpwarp_state::counter_w));
	m_latch->q_out_cb<6>().set(FUNC(warpwarp_state::ball_on_w));
	m_latch->q_out_cb<7>().set(FUNC(warpwarp_state::inv_w));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/3, 384, 0, 272, 264, 0, 224);
	screen.set_screen_update(FUNC(warpwarp_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(warpwarp_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_color);

	PALETTE(config, m_palette, FUNC(warpwarp_state::warpwarp_palette), 2*256+1);
	MCFG_VIDEO_START_OVERRIDE(warpwarp_state,warpwarp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	WARPWARP_SOUND(config, m_warpwarp_sound, MASTER_CLOCK).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void warpwarp_state::warpwarp(machine_config &config)
{
	bombbee(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &warpwarp_state::warpwarp_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( geebee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "geebee.1k",    0x0000, 0x1000, CRC(8a5577e0) SHA1(356d33e19c6b4f519816ee4b65ff9b59d6c1b565) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "geebee.3a",    0x0000, 0x0400, CRC(f257b21b) SHA1(c788fd923438f1bffbff9ff3cd4c5c8b547c0c14) )
	ROM_RELOAD(               0x0400, 0x0400 )
ROM_END

ROM_START( geebeea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "132",    0x0000, 0x0400, CRC(23252fc7) SHA1(433f0f435ff741a789942194356aaec53192608a) )
	ROM_LOAD( "133",    0x0400, 0x0400, CRC(0bc4d4ca) SHA1(46028ce1dbf46e49b921cfabec78cded914af358) )
	ROM_LOAD( "134",    0x0800, 0x0400, CRC(7899b4c1) SHA1(70f609f9873f1a4d9c8a90361c7519bdd24ad9ea) )
	ROM_LOAD( "135",    0x0c00, 0x0400, CRC(0b6e6fcb) SHA1(e7c3e8a13e3d2be6cfb6675fb57cc4a2fda6bec2) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "a_136",  0x0000, 0x0400, CRC(bd01437d) SHA1(4324c68472f762a13f978e54b3c4a7984cc7195a) )
	ROM_RELOAD(         0x0400, 0x0400 )
ROM_END

ROM_START( geebeeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.1m",    0x0000, 0x0400, CRC(23252fc7) SHA1(433f0f435ff741a789942194356aaec53192608a) )
	ROM_LOAD( "2.1p",    0x0400, 0x0400, CRC(0bc4d4ca) SHA1(46028ce1dbf46e49b921cfabec78cded914af358) )
	ROM_LOAD( "3.1s",    0x0800, 0x0400, CRC(7899b4c1) SHA1(70f609f9873f1a4d9c8a90361c7519bdd24ad9ea) )
	ROM_LOAD( "4.1t",    0x0c00, 0x0400, CRC(0b6e6fcb) SHA1(e7c3e8a13e3d2be6cfb6675fb57cc4a2fda6bec2) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "geebee.3a",    0x0000, 0x0400, CRC(f257b21b) SHA1(c788fd923438f1bffbff9ff3cd4c5c8b547c0c14) )
	ROM_RELOAD(               0x0400, 0x0400 )
ROM_END

ROM_START( geebeeg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "geebee.1k",    0x0000, 0x1000, CRC(8a5577e0) SHA1(356d33e19c6b4f519816ee4b65ff9b59d6c1b565) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "geebeeg.3a",   0x0000, 0x0400, CRC(a45932ba) SHA1(48f70742c42a9377f31fac3a1e43123751e57656) )
	ROM_RELOAD(               0x0400, 0x0400 )
ROM_END

ROM_START( navarone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "navalone.p1",  0x0000, 0x0800, CRC(5a32016b) SHA1(d856d069eba470a81341de0bf47eca2a629a69a6) )
	ROM_LOAD( "navalone.p2",  0x0800, 0x0800, CRC(b1c86fe3) SHA1(0293b742806c1517cb126443701115a3427fc60a) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "navalone.chr", 0x0000, 0x0800, CRC(b26c6170) SHA1(ae0aec2b60e1fd3b212e311afb1c588b2b286433) )
ROM_END

ROM_START( kaitein )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kaitein.p1",   0x0000, 0x0800, CRC(d88e10ae) SHA1(76d6cd46b6e59e528e7a8fff9965375a1446a91d) )
	ROM_LOAD( "kaitein.p2",   0x0800, 0x0800, CRC(aa9b5763) SHA1(64a6c8f25b0510841dcce0b57505731aa0deeda7) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "kaitein.chr",  0x0000, 0x0800, CRC(3125af4d) SHA1(9e6b161636665ee48d6bde2d5fc412fde382c687) )
ROM_END

ROM_START( kaitei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kaitei_7.1k",  0x0000, 0x0800, CRC(32f70d48) SHA1(c5ae606df1d0e513daea909f5474309a176096c1) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "kaitei_1.1m",  0x1000, 0x0400, CRC(9a7ab3b9) SHA1(94a82ba66e51c8203ec61c9320edbddbb6462d33) )
	ROM_LOAD( "kaitei_2.1p",  0x1400, 0x0400, CRC(5eeb0fff) SHA1(91cb84a9af8e4df4e6c896e7655199328b7da30b) )
	ROM_LOAD( "kaitei_3.1s",  0x1800, 0x0400, CRC(5dff4df7) SHA1(c179c93a559a0d18db3092c842634de02f3f03ea) )
	ROM_LOAD( "kaitei_4.1t",  0x1c00, 0x0400, CRC(e5f303d6) SHA1(6dd57e0b17f51d101c6c5dbfeadb7418098cc440) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "kaitei_5.bin", 0x0000, 0x0400, CRC(60fdb795) SHA1(723e635eed9937a28bee0b7978413984651ee87f) )
	ROM_LOAD( "kaitei_6.bin", 0x0400, 0x0400, CRC(21399ace) SHA1(0ad49be2c9bdab2f9dc41c7348d1d4b4b769e3c4) )
ROM_END

ROM_START( sos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sos.p1",       0x0000, 0x0800, CRC(f70bdafb) SHA1(e71d552ccc9adad48225bdb4d62c31c5741a3e95) )
	ROM_LOAD( "sos.p2",       0x0800, 0x0800, CRC(58e9c480) SHA1(0eeb5982183d0e9f9dbae04839b604a0c22b420e) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "sos.chr",      0x0000, 0x0800, CRC(66f983e4) SHA1(b3cf8bff4ac6b554d3fc06eeb8227b3b2a0dd554) )
ROM_END

ROM_START( bombbee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bombbee.1k",   0x0000, 0x2000, CRC(9f8cd7af) SHA1(0d6e1ee5519660d1498eb7a093872ed5034423f2) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "bombbee.4c",   0x0000, 0x0800, CRC(5f37d569) SHA1(d5e3fb4c5a1612a6e568c8970161b0290b88993f) )
ROM_END

ROM_START( cutieq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cutieq.1k",    0x0000, 0x2000, CRC(6486cdca) SHA1(914c36487fba2dd57c3fd1f011b2225d2baac2bf) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "cutieq.4c",    0x0000, 0x0800, CRC(0e1618c9) SHA1(456e9b3d6bae8b4af7778a38e4f40bb6736b0690) )
ROM_END

ROM_START( warpwarp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ww1_prg1.s10", 0x0000, 0x1000, CRC(f5262f38) SHA1(1c64d0282b0a209390a548ceeaaf8b7b55e50896) )
	ROM_LOAD( "ww1_prg2.s8",  0x1000, 0x1000, CRC(de8355dd) SHA1(133d137711d79aaeb45cd3ee041c0be3b73e1b2f) )
	ROM_LOAD( "ww1_prg3.s4",  0x2000, 0x1000, CRC(bdd1dec5) SHA1(bb3d9d1500e31bb271a394facaec7adc3c987e5e) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "ww1_chg1.s12", 0x0000, 0x0800, CRC(380994c8) SHA1(0cdf6a05db52c423365bff9c9df6d93ac885794e) )
ROM_END

ROM_START( warpwarpr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g-09601.2r",   0x0000, 0x1000, CRC(916ffa35) SHA1(bca2087f8b78a128cdffc55db9814854b72daab5) )
	ROM_LOAD( "g-09602.2m",   0x1000, 0x1000, CRC(398bb87b) SHA1(74373336288dc13d59e6f7e7c718aa51d857b087) )
	ROM_LOAD( "g-09603.1p",   0x2000, 0x1000, CRC(6b962fc4) SHA1(0291d0c574a1048e52121ca57e01098bff04da40) )
	ROM_LOAD( "g-09613.1t",   0x3000, 0x0800, CRC(60a67e76) SHA1(af65e7bf16a5e69fee05c0134e3b8d5bca142402) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "g-9611.4c",    0x0000, 0x0800, CRC(00e6a326) SHA1(67b7ab5b7b2c9a97d4d690d88561da48b86bc66e) )
ROM_END

ROM_START( warpwarpr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g-09601.2r",   0x0000, 0x1000, CRC(916ffa35) SHA1(bca2087f8b78a128cdffc55db9814854b72daab5) )
	ROM_LOAD( "g-09602.2m",   0x1000, 0x1000, CRC(398bb87b) SHA1(74373336288dc13d59e6f7e7c718aa51d857b087) )
	ROM_LOAD( "g-09603.1p",   0x2000, 0x1000, CRC(6b962fc4) SHA1(0291d0c574a1048e52121ca57e01098bff04da40) )
	ROM_LOAD( "g-09612.1t",   0x3000, 0x0800, CRC(b91e9e79) SHA1(378323d83c550b3acabc83dba946ab089b9195cb) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "g-9611.4c",    0x0000, 0x0800, CRC(00e6a326) SHA1(67b7ab5b7b2c9a97d4d690d88561da48b86bc66e) )
ROM_END



void warpwarp_state::init_geebee()
{
	m_handle_joystick = 0;

	m_ball_pen = 1;
	m_ball_sizex = 4;
	m_ball_sizey = 4;
}

void warpwarp_state::init_navarone()
{
	m_handle_joystick = 1;

	m_ball_pen = 1;
	m_ball_sizex = 4;
	m_ball_sizey = 4;
}

void warpwarp_state::init_kaitein()
{
	m_handle_joystick = 1;

	m_ball_pen = 8;
	m_ball_sizex = 1;
	m_ball_sizey = 16;
}

void warpwarp_state::init_kaitei()
{
	m_handle_joystick = 0;

	m_ball_pen = 8;
	m_ball_sizex = 1;
	m_ball_sizey = 16;
}

void warpwarp_state::init_sos()
{
	m_handle_joystick = 1;

	m_ball_pen = 0;
	m_ball_sizex = 4;
	m_ball_sizey = 2;
}

void warpwarp_state::init_bombbee()
{
	m_handle_joystick = 0;

	m_ball_pen = 0x200;
	m_ball_sizex = 4;
	m_ball_sizey = 4;
}

void warpwarp_state::init_warpwarp()
{
	m_handle_joystick = 1;

	m_ball_pen = 0x200;
	m_ball_sizex = 4;
	m_ball_sizey = 4;
}


/* B & W games */
GAMEL( 1978, geebee,     0,        geebee,   geebee,    warpwarp_state, init_geebee,   ROT90, "Namco", "Gee Bee (Japan)", MACHINE_SUPPORTS_SAVE, layout_geebee )
GAMEL( 1978, geebeea,    geebee,   geebeeb,  geebeeb,   warpwarp_state, init_geebee,   ROT90, "Namco (Alca license)", "Gee Bee (UK)", MACHINE_SUPPORTS_SAVE, layout_geebee )
GAMEL( 1978, geebeeb,    geebee,   geebeeb,  geebeeb,   warpwarp_state, init_geebee,   ROT90, "Namco (F.lli Bertolino license)", "Gee Bee (Europe)", MACHINE_SUPPORTS_SAVE, layout_geebee ) // Fratelli Bertolino
GAMEL( 1978, geebeeg,    geebee,   geebee,   geebee,    warpwarp_state, init_geebee,   ROT90, "Namco (Gremlin license)", "Gee Bee (US)", MACHINE_SUPPORTS_SAVE, layout_geebee )

GAMEL( 1980, navarone,   0,        navarone, navarone,  warpwarp_state, init_navarone, ROT90, "Namco", "Navarone", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_navarone )
GAME(  1980, kaitein,    kaitei,   kaitei,   kaitein,   warpwarp_state, init_kaitein,  ROT90, "K.K. Tokki (Namco license)", "Kaitei Takara Sagashi (Namco license)", MACHINE_SUPPORTS_SAVE ) // pretty sure it didn't have a color overlay
GAME(  1980, kaitei,     0,        kaitei,   kaitei,    warpwarp_state, init_kaitei,   ROT90, "K.K. Tokki", "Kaitei Takara Sagashi", MACHINE_SUPPORTS_SAVE ) // "
GAME(  1980, sos,        0,        navarone, sos,       warpwarp_state, init_sos,      ROT90, "Namco", "SOS", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // developed by Shoei? Flyer shows a Shoei logo.

/* Color games */
GAME(  1979, bombbee,    0,        bombbee,  bombbee,   warpwarp_state, init_bombbee,  ROT90, "Namco", "Bomb Bee", MACHINE_SUPPORTS_SAVE )
GAME(  1979, cutieq,     0,        bombbee,  cutieq,    warpwarp_state, init_bombbee,  ROT90, "Namco", "Cutie Q", MACHINE_SUPPORTS_SAVE )
GAME(  1981, warpwarp,   0,        warpwarp, warpwarp,  warpwarp_state, init_warpwarp, ROT90, "Namco", "Warp & Warp", MACHINE_SUPPORTS_SAVE )
GAME(  1981, warpwarpr,  warpwarp, warpwarp, warpwarpr, warpwarp_state, init_warpwarp, ROT90, "Namco (Rock-Ola license)", "Warp Warp (Rock-Ola set 1)", MACHINE_SUPPORTS_SAVE )
GAME(  1981, warpwarpr2, warpwarp, warpwarp, warpwarpr, warpwarp_state, init_warpwarp, ROT90, "Namco (Rock-Ola license)", "Warp Warp (Rock-Ola set 2)", MACHINE_SUPPORTS_SAVE )
