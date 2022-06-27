// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*

Driver for a PDP1 emulator.

    Digital Equipment Corporation
    Brian Silverman (original Java Source)
    Vadim Gerasimov (original Java Source)
    Chris Salomon (MESS driver)
    Raphael Nabet (MESS driver)

Initially, this was a conversion of a JAVA emulator
(although code has been edited extensively ever since).
I have tried contacting the author, but heard as yet nothing of him,
so I don't know if it all right with him, but after all -> he did
release the source, so hopefully everything will be fine (no his
name is not Marat).

Note: naturally I have no PDP1, I have never seen one, nor have I any
programs for it.

The first supported program was:

SPACEWAR!

The first Videogame EVER!

When I saw the java emulator, running that game I was quite intrigued to
include a driver for MESS.
I think the historical value of SPACEWAR! is enormous.

Two other programs are supported: Munching squares and LISP.

Added Debugging and Disassembler...


Also:
ftp://minnie.cs.adfa.oz.au/pub/PDP-11/Sims/Supnik_2.3/software/lispswre.tar.gz
Is a packet which includes the original LISP as source and
binary form plus a macro assembler for PDP1 programs.

For more documentation look at the source for the driver,
and the cpu/pdp1/pdp1.c file (information about the whereabouts of information
and the java source).


To load and play a game:
- Load a .rim file into the first tape reader
- Hold down Left Control, and press Enter. Let go.
- The lights will flash while the paper tape is being read.
- At the end, the game will start.



*/

#include "emu.h"
#include "includes/pdp1.h"

#include "cpu/pdp1/pdp1.h"
#include "video/crt.h"
#include "screen.h"
#include "softlist_dev.h"


/*
 *
 * The loading storing OS... is not emulated (I haven't a clue where to
 * get programs for the machine)
 *
 */


void pdp1_state::pdp1_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}

static INPUT_PORTS_START( pdp1 )
	PORT_START("SPACEWAR")      /* 0: spacewar controllers */
	PORT_BIT( ROTATE_LEFT_PLAYER1, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_NAME("Spin Left Player 1") PORT_CODE(KEYCODE_A) PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT( ROTATE_RIGHT_PLAYER1, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Spin Right Player 1") PORT_CODE(KEYCODE_S) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT( THRUST_PLAYER1, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Thrust Player 1") PORT_CODE(KEYCODE_D) PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( FIRE_PLAYER1, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Fire Player 1") PORT_CODE(KEYCODE_F) PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT( ROTATE_LEFT_PLAYER2, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_NAME("Spin Left Player 2") PORT_CODE(KEYCODE_LEFT) PORT_CODE(JOYCODE_X_LEFT_SWITCH ) PORT_PLAYER(2)
	PORT_BIT( ROTATE_RIGHT_PLAYER2, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Spin Right Player 2") PORT_CODE(KEYCODE_RIGHT) PORT_CODE(JOYCODE_X_RIGHT_SWITCH ) PORT_PLAYER(2)
	PORT_BIT( THRUST_PLAYER2, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Thrust Player 2") PORT_CODE(KEYCODE_UP) PORT_CODE(JOYCODE_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( FIRE_PLAYER2, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Fire Player 2") PORT_CODE(KEYCODE_DOWN) PORT_CODE(JOYCODE_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( HSPACE_PLAYER1, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hyperspace Player 1") PORT_CODE(KEYCODE_Z) PORT_CODE(JOYCODE_BUTTON3)
	PORT_BIT( HSPACE_PLAYER2, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hyperspace Player 2") PORT_CODE(KEYCODE_SLASH) PORT_CODE(JOYCODE_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("CSW")       /* 1: various pdp1 operator control panel switches */
	PORT_BIT(pdp1_control, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(pdp1_extend, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("extend") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(pdp1_start_nobrk, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("start (sequence break disabled)") PORT_CODE(KEYCODE_U)
	PORT_BIT(pdp1_start_brk, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("start (sequence break enabled)") PORT_CODE(KEYCODE_I)
	PORT_BIT(pdp1_stop, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("stop") PORT_CODE(KEYCODE_O)
	PORT_BIT(pdp1_continue, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("continue") PORT_CODE(KEYCODE_P)
	PORT_BIT(pdp1_examine, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("examine") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(pdp1_deposit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("deposit") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(pdp1_read_in, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("read in") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(pdp1_reader, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("reader")
	PORT_BIT(pdp1_tape_feed, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("tape feed")
	PORT_BIT(pdp1_single_step, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("single step") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(pdp1_single_inst, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("single inst") PORT_CODE(KEYCODE_SLASH)

	PORT_START("SENSE")     /* 2: operator control panel sense switches */
	PORT_DIPNAME(     040, 000, "Sense Switch 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_DIPSETTING(    000, DEF_STR( Off ) )
	PORT_DIPSETTING(    040, DEF_STR( On ) )
	PORT_DIPNAME(     020, 000, "Sense Switch 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_DIPSETTING(    000, DEF_STR( Off ) )
	PORT_DIPSETTING(    020, DEF_STR( On ) )
	PORT_DIPNAME(     010, 000, "Sense Switch 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_DIPSETTING(    000, DEF_STR( Off ) )
	PORT_DIPSETTING(    010, DEF_STR( On ) )
	PORT_DIPNAME(     004, 000, "Sense Switch 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_DIPSETTING(    000, DEF_STR( Off ) )
	PORT_DIPSETTING(    004, DEF_STR( On ) )
	PORT_DIPNAME(     002, 002, "Sense Switch 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_DIPSETTING(    000, DEF_STR( Off ) )
	PORT_DIPSETTING(    002, DEF_STR( On ) )
	PORT_DIPNAME(     001, 000, "Sense Switch 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_DIPSETTING(    000, DEF_STR( Off ) )
	PORT_DIPSETTING(    001, DEF_STR( On ) )

	PORT_START("TSTADD")        /* 3: operator control panel test address switches */
	PORT_BIT( 0100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Extension Test Address Switch 3") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Extension Test Address Switch 4") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Extension Test Address Switch 5") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Extension Test Address Switch 6") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 7") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 8") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 9") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 10") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 11") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 12") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 13") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 14") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 15") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 16") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 17") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Address Switch 18") PORT_CODE(KEYCODE_R)

	PORT_START("TWDMSB")        /* 4: operator control panel test word switches MSB */
	PORT_BIT(    0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 1") PORT_CODE(KEYCODE_A)
	PORT_BIT(    0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 2") PORT_CODE(KEYCODE_S)

	PORT_START("TWDLSB")        /* 5: operator control panel test word switches LSB */
	PORT_BIT( 0100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 8") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 9") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 10") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 11") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 12") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 13") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 14") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 15") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 16") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 17") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Test Word Switch 18") PORT_CODE(KEYCODE_N)

	/*
	    Note that I can see 2 additional keys whose purpose is unknown to me.
	    The caps look like "MAR REL" for the leftmost one and "MAR SET" for
	    rightmost one: maybe they were used to set the margin (I don't have the
	    manual for the typewriter). */

	PORT_START("TWR.0")      /* 6: typewriter codes 00-17 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(Space)") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 \"") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 '") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 ~") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 (implies)") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 (or)") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 (and)") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 <") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 >") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (up arrow)") PORT_CODE(KEYCODE_9)

	PORT_START("TWR.1")      /* 7: typewriter codes 20-37 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 (right arrow)") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", =") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab Key") PORT_CODE(KEYCODE_TAB)

	PORT_START("TWR.2")      /* 8: typewriter codes 40-57 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(non-spacing middle dot) _") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- +") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(") ]") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(non-spacing overstrike) |") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("( [") PORT_CODE(KEYCODE_MINUS)

	PORT_START("TWR.3")      /* 9: typewriter codes 60-77 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lower Case") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". (multiply)") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Upper case") PORT_CODE(KEYCODE_RSHIFT)
	/* hack to support my macintosh which does not differentiate the  Right Shift key */
	/* PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Upper case") PORT_CODE(KEYCODE_CAPSLOCK) */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)

	PORT_START("CFG")       /* 10: pseudo-input port with config */
	PORT_DIPNAME( 0x0003, 0x0002, "RAM size")
	PORT_DIPSETTING(   0x0000, "4kw" )
	PORT_DIPSETTING(   0x0001, "32kw")
	PORT_DIPSETTING(   0x0002, "64kw")
	PORT_DIPNAME( 0x0004, 0x0000, "Hardware multiply")
	PORT_DIPSETTING(   0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Hardware divide")
	PORT_DIPSETTING(   0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Type 20 sequence break system")
	PORT_DIPSETTING(   0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Type 32 light pen") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_DIPSETTING(   0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0020, DEF_STR( On ) )

	PORT_START("LIGHTPEN")  /* 11: pseudo-input port with lightpen status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("select larger light pen tip") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("select smaller light pen tip") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("light pen down")

	PORT_START("LIGHTX") /* 12: lightpen - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_RESET

	PORT_START("LIGHTY") /* 13: lightpen - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_RESET
INPUT_PORTS_END


static const gfx_layout fontlayout =
{
	6, 8,           /* 6*8 characters */
	pdp1_charnum,   /* 96+4 characters */
	1,              /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


/*
    The static palette only includes the pens for the control panel and
    the typewriter, as the CRT palette is generated dynamically.

    The CRT palette defines various levels of intensity between white and
    black.  Grey levels follow an exponential law, so that decrementing the
    color index periodically will simulate the remanence of a cathode ray tube.
*/
static constexpr rgb_t pdp1_colors[] =
{
	{0x00, 0x00, 0x00 }, // black
	{0xff, 0xff, 0xff }, // white
	{0x00, 0xff, 0x00 }, // green
	{0x00, 0x40, 0x00 }, // dark green
	{0xff, 0x00, 0x00 }, // red
	{0x80, 0x80, 0x80 }  // light gray
};

static constexpr uint8_t pdp1_pens[] =
{
	pen_panel_bg, pen_panel_caption,
	pen_typewriter_bg, pen_black,
	pen_typewriter_bg, pen_red
};

static const uint8_t total_colors_needed = pen_crt_num_levels + sizeof(pdp1_colors) / 3;

static GFXDECODE_START( gfx_pdp1 )
	GFXDECODE_ENTRY( "gfx1", 0, fontlayout, pen_crt_num_levels + sizeof(pdp1_colors) / 3, 3 )
GFXDECODE_END

/* Initialise the palette */
void pdp1_state::pdp1_palette(palette_device &palette) const
{
	// rgb components for the two color emissions
	constexpr double r1 = .1, g1 = .1, b1 = .924, r2 = .7, g2 = .7, b2 = .076;
	// half period in seconds for the two color emissions
	constexpr double half_period_1 = .05, half_period_2 = .20;
	// refresh period in seconds
	constexpr double update_period = 1./refresh_rate;
	double decay_1, decay_2;
	double cur_level_1, cur_level_2;

	// initialize CRT palette

	// compute the decay factor per refresh frame
	decay_1 = pow(.5, update_period / half_period_1);
	decay_2 = pow(.5, update_period / half_period_2);

	cur_level_1 = cur_level_2 = 255.;   // start with maximum level

	for (int i = pen_crt_max_intensity; i>0; i--)
	{
		// compute the current color
		int const r = int((r1*cur_level_1 + r2*cur_level_2) + .5);
		int const g = int((g1*cur_level_1 + g2*cur_level_2) + .5);
		int const b = int((b1*cur_level_1 + b2*cur_level_2) + .5);
		// write color in palette
		palette.set_indirect_color(i, rgb_t(r, g, b));
		// apply decay for next iteration
		cur_level_1 *= decay_1;
		cur_level_2 *= decay_2;
	}

	palette.set_indirect_color(0, rgb_t(0, 0, 0));

	// load static palette
	for (int i = 0; i < 6; i++)
		palette.set_indirect_color(pen_crt_num_levels + i, pdp1_colors[i]);

	// copy colortable to palette
	for (int i = 0; i < total_colors_needed; i++)
		palette.set_pen_indirect(i, i);

	// set up palette for text
	for (int i = 0; i < 6; i++)
		palette.set_pen_indirect(total_colors_needed + i, pdp1_pens[i]);
}


/*
    pdp1 machine code

    includes emulation for I/O devices (with the IOT opcode) and control panel functions

    TODO:
    * typewriter out should overwrite the typewriter buffer
    * improve emulation of the internals of tape reader?
    * improve puncher timing?
*/


/* This flag makes the emulated pdp-1 trigger a sequence break request when a character has been
typed on the typewriter keyboard.  It is useful in order to test sequence break, but we need
to emulate a connection box in which we can connect each wire to any interrupt line.  Also,
we need to determine the exact relationship between the status register and the sequence break
system (both standard and type 20). */
#define USE_SBS 0

#define LOG_IOT 0
#define LOG_IOT_EXTRA 0

/* IOT completion may take much more than the 5us instruction time.  A possible programming
error would be to execute an IOT before the last IOT of the same kind is over: such a thing
would confuse the targeted IO device, which might ignore the latter IOT, execute either
or both IOT incompletely, or screw up completely.  I insist that such an error can be caused
by a pdp-1 programming error, even if there is no emulator error. */
#define LOG_IOT_OVERLAP 0


/*
    devices which are known to generate a completion pulse (source: maintenance manual 9-??,
    and 9-20, 9-21):
    emulated:
    * perforated tape reader
    * perforated tape punch
    * typewriter output
    * CRT display
    unemulated:
    * card punch (pac: 43)
    * line printer (lpr, lsp, but NOT lfb)

    This list should probably include additional optional devices (card reader comes to mind).
*/

/* IO status word */

/* defines for io_status bits */
enum
{
	io_st_pen = 0400000,    /* light pen: light has hit the pen */
	io_st_ptr = 0200000,    /* perforated tape reader: reader buffer full */
	io_st_tyo = 0100000,    /* typewriter out: device ready */
	io_st_tyi = 0040000,    /* typewriter in: new character in buffer */
	io_st_ptp = 0020000     /* perforated tape punch: device ready */
};







/* crt display timer */

/* light pen config */



#define PARALLEL_DRUM_WORD_TIME attotime::from_nsec(8500)
#define PARALLEL_DRUM_ROTATION_TIME attotime::from_nsec(8500*4096)


static pdp1_reset_param_t pdp1_reset_param =
{
	0,  /* extend mode support defined in input ports and pdp1_init_machine */
	0,  /* hardware multiply/divide support defined in input ports and pdp1_init_machine */
	0   /* type 20 sequence break system support defined in input ports and pdp1_init_machine */
};

void pdp1_state::machine_reset()
{
	int cfg = m_cfg->read();

	pdp1_reset_param.extend_support = (cfg >> pdp1_config_extend_bit) & pdp1_config_extend_mask;
	pdp1_reset_param.hw_mul_div = (cfg >> pdp1_config_hw_mul_div_bit) & pdp1_config_hw_mul_div_mask;
	pdp1_reset_param.type_20_sbs = (cfg >> pdp1_config_type_20_sbs_bit) & pdp1_config_type_20_sbs_mask;

	/* reset device state */
	m_tape_reader->m_rcl = m_tape_reader->m_rc = 0;
	m_io_status = io_st_tyo | io_st_ptp;
	m_lightpen.active = m_lightpen.down = 0;
	m_lightpen.x = m_lightpen.y = 0;
	m_lightpen.radius = 10; /* ??? */
	pdp1_update_lightpen_state(&m_lightpen);
}


void pdp1_state::pdp1_machine_stop()
{
	/* the core will take care of freeing the timers, BUT we must set the variables
	to NULL if we don't want to risk confusing the tape image init function */
	m_tape_reader->m_timer = m_tape_puncher->m_timer = m_typewriter->m_tyo_timer = m_dpy_timer = nullptr;
}


/*
    driver init function

    Set up the pdp1_memory pointer, and generate font data.
*/
void pdp1_state::machine_start()
{
	uint8_t *dst;

	static const unsigned char fontdata6x8[pdp1_fontdata_size] =
	{   /* ASCII characters */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,
		0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xf8,0x50,0xf8,0x50,0x00,0x00,
		0x20,0x70,0xc0,0x70,0x18,0xf0,0x20,0x00,0x40,0xa4,0x48,0x10,0x20,0x48,0x94,0x08,
		0x60,0x90,0xa0,0x40,0xa8,0x90,0x68,0x00,0x10,0x20,0x40,0x00,0x00,0x00,0x00,0x00,
		0x20,0x40,0x40,0x40,0x40,0x40,0x20,0x00,0x10,0x08,0x08,0x08,0x08,0x08,0x10,0x00,
		0x20,0xa8,0x70,0xf8,0x70,0xa8,0x20,0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,
		0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x00,
		0x70,0x88,0x08,0x10,0x20,0x40,0xf8,0x00,0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00,
		0x10,0x30,0x50,0x90,0xf8,0x10,0x10,0x00,0xf8,0x80,0xf0,0x08,0x08,0x88,0x70,0x00,
		0x70,0x80,0xf0,0x88,0x88,0x88,0x70,0x00,0xf8,0x08,0x08,0x10,0x20,0x20,0x20,0x00,
		0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x70,0x88,0x88,0x88,0x78,0x08,0x70,0x00,
		0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x60,
		0x10,0x20,0x40,0x80,0x40,0x20,0x10,0x00,0x00,0x00,0xf8,0x00,0xf8,0x00,0x00,0x00,
		0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,
		0x70,0x88,0xb8,0xa8,0xb8,0x80,0x70,0x00,0x70,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,
		0xf0,0x88,0x88,0xf0,0x88,0x88,0xf0,0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,
		0xf0,0x88,0x88,0x88,0x88,0x88,0xf0,0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,
		0xf8,0x80,0x80,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x80,0x98,0x88,0x88,0x70,0x00,
		0x88,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
		0x08,0x08,0x08,0x08,0x88,0x88,0x70,0x00,0x88,0x90,0xa0,0xc0,0xa0,0x90,0x88,0x00,
		0x80,0x80,0x80,0x80,0x80,0x80,0xf8,0x00,0x88,0xd8,0xa8,0x88,0x88,0x88,0x88,0x00,
		0x88,0xc8,0xa8,0x98,0x88,0x88,0x88,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x08,
		0xf0,0x88,0x88,0xf0,0x88,0x88,0x88,0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,
		0xf8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0x88,0x88,0x88,0x88,0x88,0x50,0x20,0x00,0x88,0x88,0x88,0x88,0xa8,0xd8,0x88,0x00,
		0x88,0x50,0x20,0x20,0x20,0x50,0x88,0x00,0x88,0x88,0x88,0x50,0x20,0x20,0x20,0x00,
		0xf8,0x08,0x10,0x20,0x40,0x80,0xf8,0x00,0x30,0x20,0x20,0x20,0x20,0x20,0x30,0x00,
		0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x30,0x10,0x10,0x10,0x10,0x10,0x30,0x00,
		0x20,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,
		0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00,
		0x80,0x80,0xf0,0x88,0x88,0x88,0xf0,0x00,0x00,0x00,0x70,0x88,0x80,0x80,0x78,0x00,
		0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00,0x00,0x00,0x70,0x88,0xf8,0x80,0x78,0x00,
		0x18,0x20,0x70,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x70,
		0x80,0x80,0xf0,0x88,0x88,0x88,0x88,0x00,0x20,0x00,0x20,0x20,0x20,0x20,0x20,0x00,
		0x20,0x00,0x20,0x20,0x20,0x20,0x20,0xc0,0x80,0x80,0x90,0xa0,0xe0,0x90,0x88,0x00,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xf0,0xa8,0xa8,0xa8,0xa8,0x00,
		0x00,0x00,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,
		0x00,0x00,0xf0,0x88,0x88,0xf0,0x80,0x80,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x08,
		0x00,0x00,0xb0,0xc8,0x80,0x80,0x80,0x00,0x00,0x00,0x78,0x80,0x70,0x08,0xf0,0x00,
		0x20,0x20,0x70,0x20,0x20,0x20,0x18,0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,
		0x00,0x00,0x88,0x88,0x88,0x50,0x20,0x00,0x00,0x00,0xa8,0xa8,0xa8,0xa8,0x50,0x00,
		0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,0x00,0x88,0x88,0x88,0x78,0x08,0x70,
		0x00,0x00,0xf8,0x10,0x20,0x40,0xf8,0x00,0x08,0x10,0x10,0x20,0x10,0x10,0x08,0x00,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x40,0x20,0x20,0x10,0x20,0x20,0x40,0x00,
		0x00,0x68,0xb0,0x00,0x00,0x00,0x00,0x00,0x20,0x50,0x20,0x50,0xa8,0x50,0x00,0x00,

		/* non-spacing middle dot */
		0x00,
		0x00,
		0x00,
		0x20,
		0x00,
		0x00,
		0x00,
		0x00,
		/* non-spacing overstrike */
		0x00,
		0x00,
		0x00,
		0x00,
		0xfc,
		0x00,
		0x00,
		0x00,
		/* implies */
		0x00,
		0x00,
		0x70,
		0x08,
		0x08,
		0x08,
		0x70,
		0x00,
		/* or */
		0x88,
		0x88,
		0x88,
		0x50,
		0x50,
		0x50,
		0x20,
		0x00,
		/* and */
		0x20,
		0x50,
		0x50,
		0x50,
		0x88,
		0x88,
		0x88,
		0x00,
		/* up arrow */
		0x20,
		0x70,
		0xa8,
		0x20,
		0x20,
		0x20,
		0x20,
		0x00,
		/* right arrow */
		0x00,
		0x20,
		0x10,
		0xf8,
		0x10,
		0x20,
		0x00,
		0x00,
		/* multiply */
		0x00,
		0x88,
		0x50,
		0x20,
		0x50,
		0x88,
		0x00,
		0x00,
	};

	/* set up our font */
	dst = memregion("gfx1")->base();
	memcpy(dst, fontdata6x8, pdp1_fontdata_size);

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&pdp1_state::pdp1_machine_stop,this));

	m_dpy_timer = timer_alloc(FUNC(pdp1_state::dpy_callback), this);
}


/*
    perforated tape handling
*/

DEFINE_DEVICE_TYPE(PDP1_READTAPE, pdp1_readtape_image_device, "pdp1_readtape_image", "PDP-1 Tape Reader")

pdp1_readtape_image_device::pdp1_readtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: paper_tape_reader_device(mconfig, PDP1_READTAPE, tag, owner, clock)
	, m_maincpu(*this, "^maincpu")
	, m_st_ptr(*this)
	, m_timer(nullptr)
{
}

void pdp1_readtape_image_device::device_resolve_objects()
{
	m_st_ptr.resolve_safe();
}

void pdp1_readtape_image_device::device_start()
{
	m_timer = timer_alloc(FUNC(pdp1_readtape_image_device::reader_callback), this);
	m_timer->adjust(attotime::zero, 0, attotime::from_hz(2500));
	m_timer->enable(0);
}


DEFINE_DEVICE_TYPE(PDP1_PUNCHTAPE, pdp1_punchtape_image_device, "pdp1_punchtape_image_device", "PDP-1 Tape Puncher")

pdp1_punchtape_image_device::pdp1_punchtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: paper_tape_punch_device(mconfig, PDP1_PUNCHTAPE, tag, owner, clock)
	, m_maincpu(*this, "^maincpu")
	, m_st_ptp(*this)
	, m_timer(nullptr)
{
}

void pdp1_punchtape_image_device::device_resolve_objects()
{
	m_st_ptp.resolve_safe();
}

void pdp1_punchtape_image_device::device_start()
{
	m_timer = timer_alloc(FUNC(pdp1_punchtape_image_device::puncher_callback), this);
}


DEFINE_DEVICE_TYPE(PDP1_TYPEWRITER, pdp1_typewriter_device, "pdp1_typewriter_image", "PDP-1 Typewriter")

pdp1_typewriter_device::pdp1_typewriter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PDP1_TYPEWRITER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_maincpu(*this, "^maincpu")
	, m_driver_state(*this, DEVICE_SELF_OWNER)
	, m_twr(*this, "^TWR.%u", 0)
	, m_st_tyo(*this)
	, m_st_tyi(*this)
	, m_tyo_timer(nullptr)
{
}

void pdp1_typewriter_device::device_resolve_objects()
{
	m_st_tyo.resolve_safe();
	m_st_tyi.resolve_safe();
}

void pdp1_typewriter_device::device_start()
{
	m_tyo_timer = timer_alloc(FUNC(pdp1_typewriter_device::tyo_callback), this);

	m_color = m_pos = m_case_shift = 0;
}


DEFINE_DEVICE_TYPE(PDP1_CYLINDER, pdp1_cylinder_image_device, "pdp1_cylinder_image", "PDP-1 Cylinder")

pdp1_cylinder_image_device::pdp1_cylinder_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PDP1_CYLINDER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_maincpu(*this, "^maincpu")
{
}

void pdp1_cylinder_image_device::device_start()
{
}

/*
    Open a perforated tape image
*/
image_init_result pdp1_readtape_image_device::call_load()
{
	// start motor
	m_motor_on = 1;

		/* restart reader IO when necessary */
		/* note that this function may be called before pdp1_init_machine, therefore
		before m_timer is allocated.  It does not matter, as the clutch is never
		down at power-up, but we must not call timer_enable with a nullptr parameter! */

	if (m_timer)
	{
		if (m_motor_on && m_rcl)
		{
			m_timer->enable(1);
		}
		else
		{
			m_timer->enable(0);
		}
	}

	return image_init_result::PASS;
}

void pdp1_readtape_image_device::call_unload()
{
	// stop motor
	m_motor_on = 0;

	if (m_timer)
		m_timer->enable(0);
}

/*
    Read a byte from perforated tape
*/
int pdp1_readtape_image_device::tape_read(uint8_t *reply)
{
	if (is_loaded() && (fread(reply, 1) == 1))
		return 0;   /* unit OK */
	else
		return 1;   /* unit not ready */
}


/*
    common code for tape read commands (RPA, RPB, and read-in mode)
*/
void pdp1_readtape_image_device::begin_tape_read(int binary, int nac)
{
	m_rb = 0;
	m_rcl = 1;
	m_rc = (binary) ? 1 : 3;
	m_rby = (binary) ? 1 : 0;
	m_rcp = nac;

	if (LOG_IOT_OVERLAP)
	{
		if (m_timer->enable(0))
			logerror("Error: overlapped perforated tape reads (Read-in mode, RPA/RPB instruction)\n");
	}
	/* set up delay if tape is advancing */
	if (m_motor_on && m_rcl)
	{
		/* delay is approximately 1/400s */
		m_timer->enable(1);
	}
	else
	{
		m_timer->enable(0);
	}
}

/*
    timer callback to simulate reader IO
*/
TIMER_CALLBACK_MEMBER(pdp1_readtape_image_device::reader_callback)
{
	if (m_rc)
	{
		uint8_t data;
		int not_ready = tape_read(&data);
		if (not_ready)
		{
			m_motor_on = 0; // let us stop the motor
		}
		else
		{
			if ((!m_rby) || (data & 0200))
			{
				m_rb |= (m_rby) ? (data & 077) : data;

				if (m_rc != 3)
				{
					m_rb <<= 6;
				}

				m_rc = (m_rc+1) & 3;

				if (m_rc == 0)
				{   // IO complete
					m_rcl = 0;
					if (m_rcp)
					{
						m_maincpu->set_state_int(PDP1_IO, m_rb);  // transfer reader buffer to IO
						m_maincpu->set_state_int(PDP1_IOS,1);
					}
					else
						m_st_ptr(1);
				}
			}
		}
	}

	if (m_motor_on && m_rcl)
	{
		m_timer->enable(1);
	}
	else
	{
		m_timer->enable(0);
	}
}

/*
    perforated tape reader iot callbacks
*/

/*
    rpa iot callback

    op2: iot command operation (equivalent to mb & 077)
    nac: nead a completion pulse
    mb: contents of the MB register
    io: pointer on the IO register
    ac: contents of the AC register
*/
/*
 * Read Perforated Tape, Alphanumeric
 * rpa Address 0001
 *
 * This instruction reads one line of tape (all eight Channels) and transfers the resulting 8-bit code to
 * the Reader Buffer. If bits 5 and 6 of the rpa function are both zero (720001), the contents of the
 * Reader Buffer must be transferred to the IO Register by executing the rrb instruction. When the Reader
 * Buffer has information ready to be transferred to the IO Register, Status Register Bit 1 is set to
 * one. If bits 5 and 6 are different (730001 or 724001) the 8-bit code read from tape is automatically
 * transferred to the IO Register via the Reader Buffer and appears as follows:
 *
 * IO Bits        10 11 12 13 14 15 16 17
 * TAPE CHANNELS  8  7  6  5  4  3  2  1
 */
void pdp1_readtape_image_device::iot_rpa(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("Warning, RPA instruction not fully emulated: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	begin_tape_read(0, nac);
}

/*
    rpb iot callback
*/
/*
 * Read Perforated Tape, Binary rpb
 * Address 0002
 *
 * The instruction reads three lines of tape (six Channels per line) and assembles
 * the resulting 18-bit word in the Reader Buffer.  For a line to be recognized by
 * this instruction Channel 8 must be punched (lines with Channel 8 not punched will
 * be skipped over).  Channel 7 is ignored.  The instruction sub 5137, for example,
 * appears on tape and is assembled by rpb as follows:
 *
 * Channel 8 7 6 5 4 | 3 2 1
 * Line 1  X   X     |   X
 * Line 2  X   X   X |     X
 * Line 3  X     X X | X X X
 * Reader Buffer 100 010 101 001 011 111
 *
 * (Vertical dashed line indicates sprocket holes and the symbols "X" indicate holes
 * punched in tape).
 *
 * If bits 5 and 6 of the rpb instruction are both zero (720002), the contents of
 * the Reader Buffer must be transferred to the IO Register by executing
 * a rrb instruction.  When the Reader Buffer has information ready to be transferred
 * to the IO Register, Status Register Bit 1 is set to one.  If bits 5 and 6 are
 * different (730002 or 724002) the 18-bit word read from tape is automatically
 * transferred to the IO Register via the Reader Buffer.
 *
 * The rpb command pulse is also asserted for each word transfer in read-in mode.
 */
void pdp1_readtape_image_device::iot_rpb(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("Warning, RPB instruction not fully emulated: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	begin_tape_read(1, nac);
}

/*
    rrb iot callback
*/
void pdp1_readtape_image_device::iot_rrb(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("RRB instruction: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	io = m_rb;
	m_st_ptr(0);
}


image_init_result pdp1_punchtape_image_device::call_load()
{
	return image_init_result::PASS;
}

void pdp1_punchtape_image_device::call_unload()
{
}

/*
    Write a byte to perforated tape
*/
void pdp1_punchtape_image_device::tape_write(uint8_t data)
{
	if (is_loaded())
		fwrite(&data, 1);
}

/*
    timer callback to generate punch completion pulse
*/
TIMER_CALLBACK_MEMBER(pdp1_punchtape_image_device::puncher_callback)
{
	int nac = param;
	m_st_ptp(1);
	if (nac)
	{
		m_maincpu->set_state_int(PDP1_IOS,1);
	}
}

/*
    perforated tape punch iot callbacks
*/

/*
    ppa iot callback
*/
/*
 * Punch Perforated Tape, Alphanumeric
 * ppa Address 0005
 *
 * For each In-Out Transfer instruction one line of tape is punched. In-Out Register
 * Bit 17 conditions Hole 1. Bit 16 conditions Hole 2, etc. Bit 10 conditions Hole 8
 */
void pdp1_punchtape_image_device::iot_ppa(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("PPA instruction: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	tape_write(io & 0377);
	m_st_ptp(0);
	// delay is approximately 1/63.3 second
	if (LOG_IOT_OVERLAP)
	{
		if (m_timer->enable(0))
			logerror("Error: overlapped PPA/PPB instructions: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());
	}

	m_timer->adjust(attotime::from_usec(15800), nac);
}

/*
    ppb iot callback
*/
/*
 * Punch Perforated Tape, Binary
 * ppb Addres 0006
 *
 * For each In-Out Transfer instruction one line of tape is punched. In-Out Register
 * Bit 5 conditions Hole 1. Bit 4 conditions Hole 2, etc. Bit 0 conditions Hole 6.
 * Hole 7 is left blank. Hole 8 is always punched in this mode.
 */
void pdp1_punchtape_image_device::iot_ppb(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("PPB instruction: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	tape_write((io >> 12) | 0200);
	m_st_ptp(0);
	/* delay is approximately 1/63.3 second */
	if (LOG_IOT_OVERLAP)
	{
		if (m_timer->enable(0))
			logerror("Error: overlapped PPA/PPB instructions: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());
	}
	m_timer->adjust(attotime::from_usec(15800), nac);
}


/*
    Typewriter handling

    The alphanumeric on-line typewriter is a standard device on pdp-1: it
    can both handle keyboard input and print output text.
*/

/*
    Open a file for typewriter output
*/
image_init_result pdp1_typewriter_device::call_load()
{
	m_st_tyo(1);

	return image_init_result::PASS;
}

void pdp1_typewriter_device::call_unload()
{
}

/*
    Write a character to typewriter
*/
void pdp1_typewriter_device::typewriter_out(uint8_t data)
{
	if (LOG_IOT_EXTRA)
		logerror("typewriter output %o\n", data);

	drawchar(data);
	if (is_loaded())
#if 1
		fwrite(& data, 1);
#else
	{
		static const char ascii_table[2][64] =
		{   /* n-s = non-spacing */
			{   /* lower case */
				' ',                '1',                '2',                '3',
				'4',                '5',                '6',                '7',
				'8',                '9',                '*',                '*',
				'*',                '*',                '*',                '*',
				'0',                '/',                's',                't',
				'u',                'v',                'w',                'x',
				'y',                'z',                '*',                ',',
				'*',/*black*/       '*',/*red*/         '*',/*Tab*/         '*',
				'\200',/*n-s middle dot*/'j',           'k',                'l',
				'm',                'n',                'o',                'p',
				'q',                'r',                '*',                '*',
				'-',                ')',                '\201',/*n-s overstrike*/'(',
				'*',                'a',                'b',                'c',
				'd',                'e',                'f',                'g',
				'h',                'i',                '*',/*Lower Case*/  '.',
				'*',/*Upper Case*/  '*',/*Backspace*/   '*',                '*'/*Carriage Return*/
			},
			{   /* upper case */
				' ',                '"',                '\'',               '~',
				'\202',/*implies*/  '\203',/*or*/       '\204',/*and*/      '<',
				'>',                '\205',/*up arrow*/ '*',                '*',
				'*',                '*',                '*',                '*',
				'\206',/*right arrow*/'?',              'S',                'T',
				'U',                'V',                'W',                'X',
				'Y',                'Z',                '*',                '=',
				'*',/*black*/       '*',/*red*/         '\t',/*Tab*/        '*',
				'_',/*n-s???*/      'J',                'K',                'L',
				'M',                'N',                'O',                'P',
				'Q',                'R',                '*',                '*',
				'+',                ']',                '|',/*n-s???*/      '[',
				'*',                'A',                'B',                'C',
				'D',                'E',                'F',                'G',
				'H',                'I',                '*',/*Lower Case*/  '\207',/*multiply*/
				'*',/*Upper Case*/  '\b',/*Backspace*/  '*',                '*'/*Carriage Return*/
			}
		};

		data &= 0x3f;

		switch (data)
		{
		case 034:
			// Black: ignore
			//color = color_typewriter_black;
			{
				static const char black[5] = { '\033', '[', '3', '0', 'm' };
				fwrite(black, sizeof(black));
			}
			break;

		case 035:
			// Red: ignore
			//color = color_typewriter_red;
			{
				static const char red[5] = { '\033', '[', '3', '1', 'm' };
				fwrite(red, sizeof(red));
			}
			break;

		case 072:
			// Lower case
			m_case_shift = 0;
			break;

		case 074:
			// Upper case
			m_case_shift = 1;
			break;

		case 077:
			// Carriage Return
			{
				static const char line_end[2] = { '\r', '\n' };
				fwrite(line_end, sizeof(line_end));
			}
			break;

		default:
			// Any printable character...

			if ((data != 040) && (data != 056)) // 040 and 056 are non-spacing characters: don't try to print right now
				/* print character (lookup ASCII equivalent in table) */
				fwrite(&ascii_table[m_case_shift][data], 1);

			break;
		}
	}
#endif
}

/*
    timer callback to generate typewriter completion pulse
*/
TIMER_CALLBACK_MEMBER(pdp1_typewriter_device::tyo_callback)
{
	int nac = param;
	m_st_tyo(1);
	if (nac)
	{
		m_maincpu->set_state_int(PDP1_IOS,1);
	}
}

/*
    tyo iot callback
*/
void pdp1_typewriter_device::iot_tyo(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("Warning, TYO instruction not fully emulated: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	int ch = io & 077;

	typewriter_out(ch);
	m_st_tyo(0);

	/* compute completion delay (source: maintenance manual 9-12, 9-13 and 9-14) */
	int delay;
	switch (ch)
	{
	case 072:   /* lower-case */
	case 074:   /* upper-case */
	case 034:   /* black */
	case 035:   /* red */
		delay = 175;    /* approximately 175ms (?) */
		break;

	case 077:   /* carriage return */
		delay = 205;    /* approximately 205ms (?) */
		break;

	default:
		delay = 105;    /* approximately 105ms */
		break;
	}
	if (LOG_IOT_OVERLAP)
	{
		if (m_tyo_timer->enable(0))
			logerror("Error: overlapped TYO instruction: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());
	}

	m_tyo_timer->adjust(attotime::from_msec(delay), nac);
}

/*
    tyi iot callback
*/
/* When a typewriter key is struck, the code for the struck key is placed in the
 * typewriter buffer, Program Flag 1 is set, and the type-in status bit is set to
 * one. A program designed to accept typed-in data would periodically check
 * Program Flag 1, and if found to be set, an In-Out Transfer Instruction with
 * address 4 could be executed for the information to be transferred to the
 * In-Out Register. This In-Out Transfer should not use the optional in-out wait.
 * The information contained in the typewriter buffer is then transferred to the
 * right six bits of the In-Out Register. The tyi instruction automatically
 * clears the In-Out Register before transferring the information and also clears
 * the type-in status bit.
 */
void pdp1_typewriter_device::iot_tyi(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("Warning, TYI instruction not fully emulated: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	io = m_tb;
	if (! (m_driver_state->m_io_status & io_st_tyi))
		io |= 0100;
	else
		m_st_tyi(0);
}


/*
 * PRECISION CRT DISPLAY (TYPE 30)
 *
 * This sixteen-inch cathode ray tube display is intended to be used as an on-line output device for the
 * PDP-1. It is useful for high speed presentation of graphs, diagrams, drawings, and alphanumerical
 * information. The unit is solid state, self-powered and completely buffered. It has magnetic focus and
 * deflection.
 *
 * Display characteristics are as follows:
 *
 *     Random point plotting
 *     Accuracy of points +/-3 per cent of raster size
 *     Raster size 9.25 by 9.25 inches
 *     1024 by 1024 addressable locations
 *     Fixed origin at center of CRT
 *     Ones complement binary arithmetic
 *     Plots 20,000 points per second
 *
 * Resolution is such that 512 points along each axis are discernable on the face of the tube.
 *
 * One instruction is added to the PDP-1 with the installation of this display:
 *
 * Display One Point On CRT
 * dpy Address 0007
 *
 * This instruction clears the light pen status bit and displays one point using bits 0 through 9 of the
 * AC to represent the (signed) X coordinate of the point and bits 0 through 9 of the IO as the (signed)
 * Y coordinate.
 *
 * Many variations of the Type 30 Display are available. Some of these include hardware for line and
 * curve generation.
 */


/*
    timer callback to generate crt completion pulse
*/
TIMER_CALLBACK_MEMBER(pdp1_state::dpy_callback)
{
	m_maincpu->set_state_int(PDP1_IOS,1);
}


/*
    dpy iot callback

    Light on one pixel on CRT
*/
void pdp1_state::iot_dpy(int op2, int nac, int mb, int &io, int ac)
{
	int x = ((ac+0400000) & 0777777) >> 8;
	int y = ((io+0400000) & 0777777) >> 8;
	pdp1_plot(x, y);

	/* light pen 32 support */
	m_io_status &= ~io_st_pen;

	if (m_lightpen.down && ((x-m_lightpen.x)*(x-m_lightpen.x)+(y-m_lightpen.y)*(y-m_lightpen.y) < m_lightpen.radius*m_lightpen.radius))
	{
		m_io_status |= io_st_pen;

		m_maincpu->set_state_int(PDP1_PF3, 1);
	}

	if (nac)
	{
		/* 50us delay */
		if (LOG_IOT_OVERLAP)
		{
			/* note that overlap detection is incomplete: it will only work if both DPY
			instructions require a completion pulse */
			if (m_dpy_timer->enable(0))
				logerror("Error: overlapped DPY instruction: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());
		}
		m_dpy_timer->adjust(attotime::from_usec(50));
	}
}



/*
    MIT parallel drum (variant of type 23)
*/

void pdp1_cylinder_image_device::set_il(int il)
{
	m_il = il;

	attotime il_phase = ((PARALLEL_DRUM_WORD_TIME * il) - m_rotation_timer->elapsed());
	if (il_phase < attotime::zero)
		il_phase = il_phase + PARALLEL_DRUM_ROTATION_TIME;
	m_il_timer->adjust(il_phase, 0, PARALLEL_DRUM_ROTATION_TIME);
}

TIMER_CALLBACK_MEMBER(pdp1_cylinder_image_device::rotation_timer_callback)
{
}

TIMER_CALLBACK_MEMBER(pdp1_cylinder_image_device::il_timer_callback)
{
	if (m_dba)
	{
		/* set break request and status bit 5 */
		/* ... */
	}
}

void pdp1_cylinder_image_device::parallel_drum_init()
{
	m_rotation_timer = timer_alloc(FUNC(pdp1_cylinder_image_device::rotation_timer_callback), this);
	m_rotation_timer->adjust(PARALLEL_DRUM_ROTATION_TIME, 0, PARALLEL_DRUM_ROTATION_TIME);

	m_il_timer = timer_alloc(FUNC(pdp1_cylinder_image_device::il_timer_callback), this);
	set_il(0);
}

/*
    Open a file for drum
*/
image_init_result pdp1_cylinder_image_device::call_load()
{
	return image_init_result::PASS;
}

void pdp1_cylinder_image_device::call_unload()
{
}

void pdp1_cylinder_image_device::iot_dia(int op2, int nac, int mb, int &io, int ac)
{
	m_wfb = (io & 0370000) >> 12;
	set_il(io & 0007777);

	m_dba = 0; /* right? */
}

void pdp1_cylinder_image_device::iot_dba(int op2, int nac, int mb, int &io, int ac)
{
	m_wfb = (io & 0370000) >> 12;
	set_il(io & 0007777);

	m_dba = 1;
}

/*
    Read a word from drum
*/
uint32_t pdp1_cylinder_image_device::drum_read(int field, int position)
{
	int offset = (field*4096+position)*3;
	uint8_t buf[3];

	if (is_loaded() && (!fseek(offset, SEEK_SET)) && (fread(buf, 3) == 3))
		return ((buf[0] << 16) | (buf[1] << 8) | buf[2]) & 0777777;

	return 0;
}

/*
    Write a word to drum
*/
void pdp1_cylinder_image_device::drum_write(int field, int position, uint32_t data)
{
	int offset = (field*4096+position)*3;
	uint8_t buf[3];

	if (is_loaded())
	{
		buf[0] = data >> 16;
		buf[1] = data >> 8;
		buf[2] = data;

		fseek(offset, SEEK_SET);
		fwrite(buf, 3);
	}
}

void pdp1_cylinder_image_device::iot_dcc(int op2, int nac, int mb, int &io, int ac)
{
	m_rfb = (io & 0370000) >> 12;
	m_wc = - (io & 0007777);

	m_wcl = ac & 0177777/*0007777???*/;

	m_dba = 0; /* right? */
	/* clear status bit 5... */

	/* do transfer */
	attotime delay = m_il_timer->remaining();
	int dc = m_il;
	do
	{
		if ((m_wfb >= 1) && (m_wfb <= 22))
		{
			drum_write(m_wfb-1, dc, (signed)m_maincpu->space(AS_PROGRAM).read_dword(m_wcl));
		}

		if ((m_rfb >= 1) && (m_rfb <= 22))
		{
			m_maincpu->space(AS_PROGRAM).write_dword(m_wcl, drum_read(m_rfb-1, dc));
		}

		m_wc = (m_wc+1) & 07777;
		m_wcl = (m_wcl+1) & 0177777/*0007777???*/;
		dc = (dc+1) & 07777;
		if (m_wc)
			delay = delay + PARALLEL_DRUM_WORD_TIME;
	} while (m_wc);
	m_maincpu->adjust_icount(-m_maincpu->attotime_to_cycles(delay));
	/* if no error, skip */
	m_maincpu->set_state_int(PDP1_PC, m_maincpu->state_int(PDP1_PC)+1);
}

void pdp1_cylinder_image_device::iot_dra(int op2, int nac, int mb, int &io, int ac)
{
	io = (m_rotation_timer->elapsed() *
		  (ATTOSECONDS_PER_SECOND / (PARALLEL_DRUM_WORD_TIME.as_attoseconds()))).seconds() & 0007777;

	/* set parity error and timing error... */
}



/*
    iot 11 callback

    Read state of Spacewar! controllers

    Not documented, except a few comments in the Spacewar! source code:
        it should leave the control word in the io as follows.
        high order 4 bits, rotate ccw, rotate cw, (both mean hyperspace)
        fire rocket, and fire torpedo. low order 4 bits, same for
        other ship. routine is entered by jsp cwg.
*/
void pdp1_state::iot_011(int op2, int nac, int mb, int &io, int ac)
{
	int key_state = m_spacewar->read();
	int reply;


	reply = 0;

	if (key_state & ROTATE_RIGHT_PLAYER2)
		reply |= 0400000;
	if (key_state & ROTATE_LEFT_PLAYER2)
		reply |= 0200000;
	if (key_state & THRUST_PLAYER2)
		reply |= 0100000;
	if (key_state & FIRE_PLAYER2)
		reply |= 0040000;
	if (key_state & HSPACE_PLAYER2)
		reply |= 0600000;

	if (key_state & ROTATE_RIGHT_PLAYER1)
		reply |= 010;
	if (key_state & ROTATE_LEFT_PLAYER1)
		reply |= 004;
	if (key_state & THRUST_PLAYER1)
		reply |= 002;
	if (key_state & FIRE_PLAYER1)
		reply |= 001;
	if (key_state & HSPACE_PLAYER1)
		reply |= 014;

	io = reply;
}


/*
    cks iot callback

    check IO status
*/
void pdp1_state::iot_cks(int op2, int nac, int mb, int &io, int ac)
{
	if (LOG_IOT_EXTRA)
		logerror("CKS instruction: mb=0%06o, (%s)\n", (unsigned) mb, machine().describe_context());

	io = m_io_status;
}

template <int Mask>
WRITE_LINE_MEMBER(pdp1_state::io_status_w)
{
	if (state)
		m_io_status |= Mask;
	else
		m_io_status &= ~Mask;

	if (USE_SBS && Mask == io_st_tyi)
		m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE); /* PDP1 - interrupt it, baby */
}


/*
    callback which is called when Start Clear is pulsed

    IO devices should reset
*/
void pdp1_state::io_start_clear()
{
	m_tape_reader->m_rcl = m_tape_reader->m_rc = 0;
	if (m_tape_reader->m_timer)
		m_tape_reader->m_timer->enable(0);

	if (m_tape_puncher->m_timer)
		m_tape_puncher->m_timer->enable(0);

	if (m_typewriter->m_tyo_timer)
		m_typewriter->m_tyo_timer->enable(0);

	if (m_dpy_timer)
		m_dpy_timer->enable(0);

	m_io_status = io_st_tyo | io_st_ptp;
}


/*
    typewriter keyboard handler
*/
void pdp1_typewriter_device::pdp1_keyboard()
{
	int i;
	int j;

	int typewriter_keys[4];

	int typewriter_transitions;


	for (i=0; i<4; i++)
	{
		typewriter_keys[i] = m_twr[i]->read();
	}

	for (i=0; i<4; i++)
	{
		typewriter_transitions = typewriter_keys[i] & (~ m_old_typewriter_keys[i]);
		if (typewriter_transitions)
		{
			for (j=0; (((typewriter_transitions >> j) & 1) == 0) /*&& (j<16)*/; j++)
				;
			m_tb = (i << 4) + j;
			m_st_tyi(1);
			m_maincpu->set_state_int(PDP1_PF1, 1);
			drawchar(m_tb);  /* we want to echo input */
			break;
		}
	}

	for (i=0; i<4; i++)
		m_old_typewriter_keys[i] = typewriter_keys[i];
}

void pdp1_state::pdp1_lightpen()
{
	int x_delta, y_delta;
	int current_state = m_io_lightpen->read();

	m_lightpen.active = (m_cfg->read() >> pdp1_config_lightpen_bit) & pdp1_config_lightpen_mask;

	/* update pen down state */
	m_lightpen.down = m_lightpen.active && (current_state & pdp1_lightpen_down);

	/* update size of pen tip hole */
	if ((current_state & ~m_old_lightpen) & pdp1_lightpen_smaller)
	{
		m_lightpen.radius --;
		if (m_lightpen.radius < 0)
			m_lightpen.radius = 0;
	}
	if ((current_state & ~m_old_lightpen) & pdp1_lightpen_larger)
	{
		m_lightpen.radius ++;
		if (m_lightpen.radius > 32)
			m_lightpen.radius = 32;
	}

	m_old_lightpen = current_state;

	/* update pen position */
	x_delta = m_lightx->read();
	y_delta = m_lighty->read();

	if (x_delta >= 0x80)
		x_delta -= 0x100;
	if (y_delta >= 0x80)
		y_delta -= 256;

	m_lightpen.x += x_delta;
	m_lightpen.y += y_delta;

	if (m_lightpen.x < 0)
		m_lightpen.x = 0;
	if (m_lightpen.x > 1023)
		m_lightpen.x = 1023;
	if (m_lightpen.y < 0)
		m_lightpen.y = 0;
	if (m_lightpen.y > 1023)
		m_lightpen.y = 1023;

	pdp1_update_lightpen_state(&m_lightpen);
}

/*
    Not a real interrupt - just handle keyboard input
*/
INTERRUPT_GEN_MEMBER(pdp1_state::pdp1_interrupt)
{
	int control_keys;
	int tw_keys;
	int ta_keys;

	int control_transitions;
	int tw_transitions;
	int ta_transitions;


	m_maincpu->set_state_int(PDP1_SS, m_sense->read());

	/* read new state of control keys */
	control_keys = m_csw->read();

	if (control_keys & pdp1_control)
	{
		/* compute transitions */
		control_transitions = control_keys & (~ m_old_control_keys);

		if (control_transitions & pdp1_extend)
		{
			m_maincpu->set_state_int(PDP1_EXTEND_SW, ! m_maincpu->state_int(PDP1_EXTEND_SW));
		}
		if (control_transitions & pdp1_start_nobrk)
		{
			m_maincpu->pulse_start_clear();    /* pulse Start Clear line */
			m_maincpu->set_state_int(PDP1_EXD, m_maincpu->state_int(PDP1_EXTEND_SW));
			m_maincpu->set_state_int(PDP1_SBM, (uint64_t)0);
			m_maincpu->set_state_int(PDP1_OV, (uint64_t)0);
			m_maincpu->set_state_int(PDP1_PC, m_maincpu->state_int(PDP1_TA));
			m_maincpu->set_state_int(PDP1_RUN, 1);
		}
		if (control_transitions & pdp1_start_brk)
		{
			m_maincpu->pulse_start_clear();    /* pulse Start Clear line */
			m_maincpu->set_state_int(PDP1_EXD, m_maincpu->state_int(PDP1_EXTEND_SW));
			m_maincpu->set_state_int(PDP1_SBM, 1);
			m_maincpu->set_state_int(PDP1_OV, (uint64_t)0);
			m_maincpu->set_state_int(PDP1_PC, m_maincpu->state_int(PDP1_TA));
			m_maincpu->set_state_int(PDP1_RUN, 1);
		}
		if (control_transitions & pdp1_stop)
		{
			m_maincpu->set_state_int(PDP1_RUN, (uint64_t)0);
			m_maincpu->set_state_int(PDP1_RIM, (uint64_t)0);  /* bug : we stop after reading an even-numbered word
			                                (i.e. data), whereas a real pdp-1 stops after reading
			                                an odd-numbered word (i.e. dio instruciton) */
		}
		if (control_transitions & pdp1_continue)
		{
			m_maincpu->set_state_int(PDP1_RUN, 1);
		}
		if (control_transitions & pdp1_examine)
		{
			m_maincpu->pulse_start_clear();    /* pulse Start Clear line */
			m_maincpu->set_state_int(PDP1_PC, m_maincpu->state_int(PDP1_TA));
			m_maincpu->set_state_int(PDP1_MA, m_maincpu->state_int(PDP1_PC));
			m_maincpu->set_state_int(PDP1_IR, pdp1_device::LAC); /* this instruction is actually executed */

			m_maincpu->set_state_int(PDP1_MB, (signed)m_maincpu->space(AS_PROGRAM).read_dword(PDP1_MA));
			m_maincpu->set_state_int(PDP1_AC, m_maincpu->state_int(PDP1_MB));
		}
		if (control_transitions & pdp1_deposit)
		{
			m_maincpu->pulse_start_clear();    /* pulse Start Clear line */
			m_maincpu->set_state_int(PDP1_PC, m_maincpu->state_int(PDP1_TA));
			m_maincpu->set_state_int(PDP1_MA, m_maincpu->state_int(PDP1_PC));
			m_maincpu->set_state_int(PDP1_AC, m_maincpu->state_int(PDP1_TW));
			m_maincpu->set_state_int(PDP1_IR, pdp1_device::DAC); /* this instruction is actually executed */

			m_maincpu->set_state_int(PDP1_MB, m_maincpu->state_int(PDP1_AC));
			m_maincpu->space(AS_PROGRAM).write_dword(m_maincpu->state_int(PDP1_MA), m_maincpu->state_int(PDP1_MB));
		}
		if (control_transitions & pdp1_read_in)
		{   /* set cpu to read instructions from perforated tape */
			m_maincpu->pulse_start_clear();    /* pulse Start Clear line */
			m_maincpu->set_state_int(PDP1_PC, (  m_maincpu->state_int(PDP1_TA) & 0170000)
										|  (m_maincpu->state_int(PDP1_PC) & 0007777));  /* transfer ETA to EPC */
			/*m_maincpu->set_state_int(PDP1_MA, m_maincpu->state_int(PDP1_PC));*/
			m_maincpu->set_state_int(PDP1_EXD, m_maincpu->state_int(PDP1_EXTEND_SW));
			m_maincpu->set_state_int(PDP1_OV, (uint64_t)0);       /* right??? */
			m_maincpu->set_state_int(PDP1_RUN, (uint64_t)0);
			m_maincpu->set_state_int(PDP1_RIM, 1);
		}
		if (control_transitions & pdp1_reader)
		{
		}
		if (control_transitions & pdp1_tape_feed)
		{
		}
		if (control_transitions & pdp1_single_step)
		{
			m_maincpu->set_state_int(PDP1_SNGL_STEP, ! m_maincpu->state_int(PDP1_SNGL_STEP));
		}
		if (control_transitions & pdp1_single_inst)
		{
			m_maincpu->set_state_int(PDP1_SNGL_INST, ! m_maincpu->state_int(PDP1_SNGL_INST));
		}

		/* remember new state of control keys */
		m_old_control_keys = control_keys;


		/* handle test word keys */
		tw_keys = (m_twdmsb->read() << 16) | m_twdlsb->read();

		/* compute transitions */
		tw_transitions = tw_keys & (~ m_old_tw_keys);

		if (tw_transitions)
			m_maincpu->set_state_int(PDP1_TW, m_maincpu->state_int(PDP1_TW) ^ tw_transitions);

		/* remember new state of test word keys */
		m_old_tw_keys = tw_keys;


		/* handle address keys */
		ta_keys = m_tstadd->read();

		/* compute transitions */
		ta_transitions = ta_keys & (~ m_old_ta_keys);

		if (ta_transitions)
			m_maincpu->set_state_int(PDP1_TA, m_maincpu->state_int(PDP1_TA) ^ ta_transitions);

		/* remember new state of test word keys */
		m_old_ta_keys = ta_keys;

	}
	else
	{
		m_old_control_keys = 0;
		m_old_tw_keys = 0;
		m_old_ta_keys = 0;

		m_typewriter->pdp1_keyboard();
	}

	pdp1_lightpen();
}


void pdp1_state::pdp1(machine_config &config)
{
	/* basic machine hardware */
	/* PDP1 CPU @ 200 kHz (no master clock, but the instruction and memory rate is 200 kHz) */
	PDP1(config, m_maincpu, 1000000); /* the CPU core uses microsecond counts */
	m_maincpu->set_reset_param(&pdp1_reset_param);
	m_maincpu->set_addrmap(AS_PROGRAM, &pdp1_state::pdp1_map);
	m_maincpu->set_vblank_int("screen", FUNC(pdp1_state::pdp1_interrupt));   /* dummy interrupt: handles input */
	/* external iot handlers.  00, 50 to 56 and 74 are internal to the cpu core. */
	/* I put a ? when the source is the handbook, since a) I have used the maintenance manual
	as the primary source (as it goes more into details) b) the handbook and the maintenance
	manual occasionally contradict each other. */
	/*  (iot)       rpa         rpb         tyo         tyi         ppa         ppb         dpy */
	/*              spacewar                                                                 */
	/*                          lag                                             glf?/jsp?   gpl?/gpr?/gcf? */
	/*  rrb         rcb?        rcc?        cks         mcs         mes         mel          */
	/*  cad?        rac?        rbc?        pac                     lpr/lfb/lsp swc/sci/sdf?/shr?   scv? */
	/*  (dsc)       (asc)       (isb)       (cac)       (lsm)       (esm)       (cbs)        */
	/*  icv?        dia         dba         dcc         dra                     mri|rlc?    mrf/inr?/ccr? */
	/*  mcb|dur?    mwc|mtf?    mrc|sfc?... msm|cgo?    (eem/lem)   mic         muf          */
	m_maincpu->set_iot_callback<001>(m_tape_reader, FUNC(pdp1_readtape_image_device::iot_rpa));
	m_maincpu->set_iot_callback<002>(m_tape_reader, FUNC(pdp1_readtape_image_device::iot_rpb));
	m_maincpu->set_iot_callback<003>(m_typewriter, FUNC(pdp1_typewriter_device::iot_tyo));
	m_maincpu->set_iot_callback<004>(m_typewriter, FUNC(pdp1_typewriter_device::iot_tyi));
	m_maincpu->set_iot_callback<005>(m_tape_puncher, FUNC(pdp1_punchtape_image_device::iot_ppa));
	m_maincpu->set_iot_callback<006>(m_tape_puncher, FUNC(pdp1_punchtape_image_device::iot_ppb));
	m_maincpu->set_iot_callback<007>(FUNC(pdp1_state::iot_dpy));
	m_maincpu->set_iot_callback<011>(FUNC(pdp1_state::iot_011));
	m_maincpu->set_iot_callback<030>(m_tape_reader, FUNC(pdp1_readtape_image_device::iot_rrb));
	m_maincpu->set_iot_callback<033>(FUNC(pdp1_state::iot_cks));
	/* dia, dba, dcc, dra are documented in MIT PDP-1 COMPUTER MODIFICATION
	BULLETIN no. 2 (drumInstrWriteup.bin/drumInstrWriteup.txt), and are
	similar to IOT documented in Parallel Drum Type 23 Instruction Manual. */
	m_maincpu->set_iot_callback<061>(m_parallel_drum, FUNC(pdp1_cylinder_image_device::iot_dia));
	m_maincpu->set_iot_callback<062>(m_parallel_drum, FUNC(pdp1_cylinder_image_device::iot_dba));
	m_maincpu->set_iot_callback<063>(m_parallel_drum, FUNC(pdp1_cylinder_image_device::iot_dcc));
	m_maincpu->set_iot_callback<064>(m_parallel_drum, FUNC(pdp1_cylinder_image_device::iot_dra));
	m_maincpu->set_io_sc_callback(FUNC(pdp1_state::io_start_clear));

	/* video hardware (includes the control panel and typewriter output) */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(refresh_rate);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(virtual_width, virtual_height);
	screen.set_visarea(0, virtual_width-1, 0, virtual_height-1);
	screen.set_screen_update(FUNC(pdp1_state::screen_update_pdp1));
	screen.screen_vblank().set(FUNC(pdp1_state::screen_vblank_pdp1));
	screen.set_palette(m_palette);

	CRT(config, m_crt, 0);
	m_crt->set_num_levels(pen_crt_num_levels);
	m_crt->set_offsets(crt_window_offset_x, crt_window_offset_y);
	m_crt->set_size(crt_window_width, crt_window_height);

	PDP1_READTAPE(config, m_tape_reader);
	m_tape_reader->st_ptr().set(FUNC(pdp1_state::io_status_w<io_st_ptr>));

	PDP1_PUNCHTAPE(config, m_tape_puncher);
	m_tape_puncher->st_ptp().set(FUNC(pdp1_state::io_status_w<io_st_ptp>));

	PDP1_TYPEWRITER(config, m_typewriter);
	m_typewriter->st_tyo().set(FUNC(pdp1_state::io_status_w<io_st_tyo>));
	m_typewriter->st_tyi().set(FUNC(pdp1_state::io_status_w<io_st_tyi>));

	PDP1_CYLINDER(config, m_parallel_drum);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pdp1);
	PALETTE(config, m_palette, FUNC(pdp1_state::pdp1_palette), total_colors_needed + std::size(pdp1_pens), total_colors_needed);

	SOFTWARE_LIST(config, "ptp_list").set_original("pdp1_ptp");
}

/*
    pdp1 can address up to 65336 18 bit words when extended (4096 otherwise).
*/
ROM_START(pdp1)
	ROM_REGION(pdp1_fontdata_size, "gfx1", ROMREGION_ERASEFF)
		/* space filled with our font */
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT CLASS       INIT        COMPANY                           FULLNAME  FLAGS
COMP( 1961, pdp1, 0,      0,      pdp1,    pdp1, pdp1_state, empty_init, "Digital Equipment Corporation",  "PDP-1",  MACHINE_NO_SOUND_HW )
