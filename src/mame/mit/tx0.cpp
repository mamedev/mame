// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    TX-0

    Raphael Nabet, 2004
*/

#include "emu.h"
#include "tx0.h"

#include "imagedev/magtape.h"
#include "imagedev/papertape.h"

#include "screen.h"
#include "softlist_dev.h"


/*

TODO:
- due to no known software, it is unknown if this system is capable of running anything.

*/

void tx0_state::init_tx0()
{
	static const unsigned char fontdata6x8[tx0_fontdata_size] =
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
	};

	/* set up our font */
	uint8_t *dst = memregion("gfx1")->base();

	memcpy(dst, fontdata6x8, tx0_fontdata_size);
}


void tx0_state::tx0_64kw_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}


void tx0_state::tx0_8kw_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
}


static INPUT_PORTS_START( tx0 )

	PORT_START("CSW")       /* 0: various tx0 operator control panel switches */
	PORT_BIT(tx0_control, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(tx0_stop_cyc0, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("stop at cycle 0") PORT_CODE(KEYCODE_Q)
	PORT_BIT(tx0_stop_cyc1, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("stop at cycle 1") PORT_CODE(KEYCODE_W)
	PORT_BIT(tx0_gbl_cm_sel, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CM select") PORT_CODE(KEYCODE_E)
	PORT_BIT(tx0_stop, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("stop") PORT_CODE(KEYCODE_P)
	PORT_BIT(tx0_restart, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("restart") PORT_CODE(KEYCODE_O)
	PORT_BIT(tx0_read_in, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("read in") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(tx0_toggle_dn, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("edit next toggle switch register") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(tx0_toggle_up, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("edit previous toggle switch register") PORT_CODE(KEYCODE_UP)
	PORT_BIT(tx0_cm_sel, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TSS CM switch") PORT_CODE(KEYCODE_A)
	PORT_BIT(tx0_lr_sel, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TSS LR switch") PORT_CODE(KEYCODE_SLASH)

	PORT_START("MSW")       /* 1: operator control panel toggle switch register switches MS */
	PORT_BIT(    0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 0") PORT_CODE(KEYCODE_S)
	PORT_BIT(    0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 1") PORT_CODE(KEYCODE_D)

	PORT_START("LSW")       /* 2: operator control panel toggle switch register switches LS */
	PORT_BIT( 0100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 2") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 3") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 4") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 5") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 6") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 7") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 8") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 9") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 10") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 11") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 12") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 13") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 14") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 15") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 16") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Toggle Switch Register Switch 17") PORT_CODE(KEYCODE_STOP)

	PORT_START("TWR.0")      /* 3: typewriter codes 00-17 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("| _") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(Space)") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= :") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ /") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)

	PORT_START("TWR.1")      /* 4: typewriter codes 20-37 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". )") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". (") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- +") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)

	PORT_START("TWR.2")      /* 5: typewriter codes 40-57 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab Key") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)

	PORT_START("TWR.3")      /* 6: typewriter codes 60-77 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Upper case") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lower Case") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)

INPUT_PORTS_END


static const gfx_layout fontlayout =
{
	6, 8,           /* 6*8 characters */
	tx0_charnum,    /* 96+xx characters */
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
static const uint8_t tx0_colors[] =
{
	0x00,0x00,0x00, /* black */
	0xFF,0xFF,0xFF, /* white */
	0x00,0xFF,0x00, /* green */
	0x00,0x40,0x00, /* dark green */
	0xFF,0x00,0x00, /* red */
	0x80,0x80,0x80  /* light gray */
};

static const uint8_t tx0_pens[] =
{
	pen_panel_bg, pen_panel_caption,    /* captions */
	pen_typewriter_bg, pen_black,       /* black typing in typewriter */
	pen_typewriter_bg, pen_red      /* red typing in typewriter */
};

static const uint8_t total_colors_needed = pen_crt_num_levels + sizeof(tx0_colors) / 3;

static GFXDECODE_START( gfx_tx0 )
	GFXDECODE_ENTRY( "gfx1", 0, fontlayout, pen_crt_num_levels + sizeof(tx0_colors) / 3, 3 )
GFXDECODE_END

/* Initialise the palette */
void tx0_state::tx0_palette(palette_device &palette) const
{
	/* rgb components for the two color emissions */
	const double r1 = .1, g1 = .1, b1 = .924, r2 = .7, g2 = .7, b2 = .076;
	/* half period in seconds for the two color emissions */
	const double half_period_1 = .05, half_period_2 = .20;
	/* refresh period in seconds */
	const double update_period = 1./refresh_rate;
	double decay_1, decay_2;
	double cur_level_1, cur_level_2;
#if 0
#ifdef MAME_DEBUG
	/* level at which we stop emulating the decay and say the pixel is black */
	double cut_level = .02;
#endif
#endif
	uint8_t i, r, g, b;

	/* initialize CRT palette */

	/* compute the decay factor per refresh frame */
	decay_1 = pow(.5, update_period / half_period_1);
	decay_2 = pow(.5, update_period / half_period_2);

	cur_level_1 = cur_level_2 = 255.;   /* start with maximum level */

	for (i=pen_crt_max_intensity; i>0; i--)
	{
		/* compute the current color */
		r = (int) ((r1*cur_level_1 + r2*cur_level_2) + .5);
		g = (int) ((g1*cur_level_1 + g2*cur_level_2) + .5);
		b = (int) ((b1*cur_level_1 + b2*cur_level_2) + .5);
		/* write color in palette */
		palette.set_indirect_color(i, rgb_t(r, g, b));
		/* apply decay for next iteration */
		cur_level_1 *= decay_1;
		cur_level_2 *= decay_2;
	}
#if 0
#ifdef MAME_DEBUG
	{
		int recommended_pen_crt_num_levels;
		if (decay_1 > decay_2)
			recommended_pen_crt_num_levels = ceil(log(cut_level)/log(decay_1))+1;
		else
			recommended_pen_crt_num_levels = ceil(log(cut_level)/log(decay_2))+1;
		if (recommended_pen_crt_num_levels != pen_crt_num_levels)
			osd_printf_debug("File %s line %d: recommended value for pen_crt_num_levels is %d\n", __FILE__, __LINE__, recommended_pen_crt_num_levels);
	}
	/*if ((cur_level_1 > 255.*cut_level) || (cur_level_2 > 255.*cut_level))
	    osd_printf_debug("File %s line %d: Please take higher value for pen_crt_num_levels or smaller value for decay\n", __FILE__, __LINE__);*/
#endif
#endif
	palette.set_indirect_color(0, rgb_t(0, 0, 0));

	/* load static palette */
	for ( i = 0; i < 6; i++ )
	{
		r = tx0_colors[i*3]; g = tx0_colors[i*3+1]; b = tx0_colors[i*3+2];
		palette.set_indirect_color(pen_crt_num_levels + i, rgb_t(r, g, b));
	}

	/* copy colortable to palette */
	for( i = 0; i < total_colors_needed; i++ )
		palette.set_pen_indirect(i, i);

	/* set up palette for text */
	for( i = 0; i < 6; i++ )
		palette.set_pen_indirect(total_colors_needed + i, tx0_pens[i]);
}



/*
    TX-0
*
    Raphael Nabet, 2004
*/



/* crt display timer */



enum
{
	PF_RWC = 040,
	PF_EOR = 020,
	PF_PC  = 010,
	PF_EOT = 004
};


void tx0_state::machine_reset()
{
	/* reset device state */
	m_tape_reader.rcl = m_tape_reader.rc = 0;
}


void tx0_state::tx0_machine_stop()
{
	/* the core will take care of freeing the timers, BUT we must set the variables
	to nullptr if we don't want to risk confusing the tape image init function */
	m_tape_reader.timer = m_tape_puncher.timer = m_typewriter.prt_timer = m_dis_timer = nullptr;
}


void tx0_state::machine_start()
{
	m_tape_reader.timer = timer_alloc(FUNC(tx0_state::reader_callback), this);
	m_tape_puncher.timer = timer_alloc(FUNC(tx0_state::puncher_callback), this);
	m_typewriter.prt_timer = timer_alloc(FUNC(tx0_state::prt_callback), this);
	m_dis_timer = timer_alloc(FUNC(tx0_state::dis_callback), this);

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&tx0_state::tx0_machine_stop,this));
}


/*
    perforated tape handling
*/

class tx0_readtape_image_device : public paper_tape_reader_device
{
public:
	// construction/destruction
	tx0_readtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual const char *file_extensions() const noexcept override { return "tap,rim"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual const char *image_interface() const noexcept override { return "tx0_ptp"; }

protected:
	// device_t implementation
	virtual void device_start() override { }

private:
	required_device<tx0_state> m_tx0;
};

DEFINE_DEVICE_TYPE(TX0_READTAPE, tx0_readtape_image_device, "tx0_readtape_image", "TX0 Tape Reader")

tx0_readtape_image_device::tx0_readtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: paper_tape_reader_device(mconfig, TX0_READTAPE, tag, owner, clock)
	, m_tx0(*this, DEVICE_SELF_OWNER)
{
}

class tx0_punchtape_image_device : public paper_tape_punch_device
{
public:
	// construction/destruction
	tx0_punchtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual const char *file_extensions() const noexcept override { return "tap,rim"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

protected:
	// device_t implementation
	virtual void device_start() override { }

private:
	required_device<tx0_state> m_tx0;
};

DEFINE_DEVICE_TYPE(TX0_PUNCHTAPE, tx0_punchtape_image_device, "tx0_punchtape_image", "TX0 Tape Puncher")

tx0_punchtape_image_device::tx0_punchtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: paper_tape_punch_device(mconfig, TX0_PUNCHTAPE, tag, owner, clock)
	, m_tx0(*this, DEVICE_SELF_OWNER)
{
}


class tx0_printer_image_device :    public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	tx0_printer_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual bool is_readable()  const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "typ"; }
	virtual const char *image_type_name() const noexcept override { return "printout"; }
	virtual const char *image_brief_type_name() const noexcept override { return "prin"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

protected:
	// device_t implementation
	virtual void device_start() override { }

private:
	required_device<tx0_state> m_tx0;
};

DEFINE_DEVICE_TYPE(TX0_PRINTER, tx0_printer_image_device, "tx0_printer_image", "TX0 Typewriter")

tx0_printer_image_device::tx0_printer_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TX0_PRINTER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_tx0(*this, DEVICE_SELF_OWNER)
{
}

class tx0_magtape_image_device : public magtape_image_device
{
public:
	// construction/destruction
	tx0_magtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual const char *file_extensions() const noexcept override { return "tap"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_device<tx0_state> m_tx0;
};

DEFINE_DEVICE_TYPE(TX0_MAGTAPE, tx0_magtape_image_device, "tx0_magtape_image", "TX0 Magnetic Tape")

tx0_magtape_image_device::tx0_magtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: magtape_image_device(mconfig, TX0_MAGTAPE, tag, owner, clock)
	, m_tx0(*this, DEVICE_SELF_OWNER)
{
}

/*
    Open a perforated tape image

    unit 0 is reader (read-only), unit 1 is puncher (write-only)
*/
std::pair<std::error_condition, std::string> tx0_readtape_image_device::call_load()
{
	if (m_tx0)
	{
		/* reader unit */
		m_tx0->m_tape_reader.fd = this;

		/* start motor */
		m_tx0->m_tape_reader.motor_on = 1;

		/* restart reader IO when necessary */
		/* note that this function may be called before tx0_init_machine, therefore
		before tape_reader.timer is allocated.  It does not matter, as the clutch is never
		down at power-up, but we must not call timer_enable with a nullptr parameter! */

		if (m_tx0->m_tape_reader.timer)
		{
			if (m_tx0->m_tape_reader.motor_on && m_tx0->m_tape_reader.rcl)
			{
				/* delay is approximately 1/400s */
				m_tx0->m_tape_reader.timer->adjust(attotime::from_usec(2500));
			}
			else
			{
				m_tx0->m_tape_reader.timer->enable(0);
			}
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void tx0_readtape_image_device::call_unload()
{
	/* reader unit */
	if (m_tx0)
	{
		m_tx0->m_tape_reader.fd = nullptr;

		/* stop motor */
		m_tx0->m_tape_reader.motor_on = 0;

		if (m_tx0->m_tape_reader.timer)
			m_tx0->m_tape_reader.timer->enable(0);
	}
}

/*
    Read a byte from perforated tape
*/
int tx0_state::tape_read(uint8_t *reply)
{
	if (m_tape_reader.fd && (m_tape_reader.fd->fread(reply, 1) == 1))
		return 0;   /* unit OK */
	else
		return 1;   /* unit not ready */
}

/*
    Write a byte to perforated tape
*/
void tx0_state::tape_write(uint8_t data)
{
	if (m_tape_puncher.fd)
	{
		data |= 0200;
		m_tape_puncher.fd->fwrite(& data, 1);
	}
}

/*
    common code for tape read commands (R1C, R3C, and read-in mode)
*/
void tx0_state::begin_tape_read(int binary)
{
	m_tape_reader.rcl = 1;
	m_tape_reader.rc = (binary) ? 1 : 3;

	/* set up delay if tape is advancing */
	if (m_tape_reader.motor_on && m_tape_reader.rcl)
	{
		/* delay is approximately 1/400s */
		m_tape_reader.timer->adjust(attotime::from_usec(2500));
	}
	else
	{
		m_tape_reader.timer->enable(0);
	}
}


/*
    timer callback to simulate reader IO
*/
TIMER_CALLBACK_MEMBER(tx0_state::reader_callback)
{
	int not_ready;
	uint8_t data;
	int ac;

	if (m_tape_reader.rc)
	{
		not_ready = tape_read( & data);
		if (not_ready)
		{
			m_tape_reader.motor_on = 0; /* let us stop the motor */
		}
		else
		{
			if (data & 0100)
			{
				/* read current AC */
				ac = m_maincpu->state_int(TX0_AC);
				/* cycle right */
				ac = (ac >> 1) | ((ac & 1) << 17);
				/* shuffle and insert data into AC */
				ac = (ac /*& 0333333*/) | ((data & 001) << 17) | ((data & 002) << 13) | ((data & 004) << 9) | ((data & 010) << 5) | ((data & 020) << 1) | ((data & 040) >> 3);
				/* write modified AC */
				m_maincpu->set_state_int(TX0_AC, ac);

				m_tape_reader.rc = (m_tape_reader.rc+1) & 3;

				if (m_tape_reader.rc == 0)
				{   /* IO complete */
					m_tape_reader.rcl = 0;
					m_maincpu->set_state_int(TX0_IOS,1);
				}
			}
		}
	}

	if (m_tape_reader.motor_on && m_tape_reader.rcl)
		/* delay is approximately 1/400s */
		m_tape_reader.timer->adjust(attotime::from_usec(2500));
	else
		m_tape_reader.timer->enable(0);
}

/*
    timer callback to generate punch completion pulse
*/
std::pair<std::error_condition, std::string> tx0_punchtape_image_device::call_load()
{
	/* punch unit */
	if (m_tx0)
		m_tx0->m_tape_puncher.fd = this;

	fseek(0, SEEK_END);

	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> tx0_punchtape_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	return call_load();
}

void tx0_punchtape_image_device::call_unload()
{
	/* punch unit */
	if (m_tx0)
		m_tx0->m_tape_puncher.fd = nullptr;
}

TIMER_CALLBACK_MEMBER(tx0_state::puncher_callback)
{
	m_maincpu->set_state_int(TX0_IOS,1);
}

/*
    Initiate read of a 6-bit word from tape
*/
void tx0_state::tx0_io_r1l(int state)
{
	begin_tape_read( 0);
}

/*
    Initiate read of a 18-bit word from tape (used in read-in mode)
*/
void tx0_state::tx0_io_r3l(int state)
{
	begin_tape_read(1);
}

/*
    Write a 7-bit word to tape (7th bit clear)
*/
void tx0_state::tx0_io_p6h(int state)
{
	int ac;

	/* read current AC */
	ac = m_maincpu->state_int(TX0_AC);
	/* shuffle and punch 6-bit word */
	tape_write(((ac & 0100000) >> 15) | ((ac & 0010000) >> 11) | ((ac & 0001000) >> 7) | ((ac & 0000100) >> 3) | ((ac & 0000010) << 1) | ((ac & 0000001) << 5));

	m_tape_puncher.timer->adjust(attotime::from_usec(15800));
}

/*
    Write a 7-bit word to tape (7th bit set)
*/
void tx0_state::tx0_io_p7h(int state)
{
	int ac;

	/* read current AC */
	ac = m_maincpu->state_int(TX0_AC);
	/* shuffle and punch 6-bit word */
	tape_write(((ac & 0100000) >> 15) | ((ac & 0010000) >> 11) | ((ac & 0001000) >> 7) | ((ac & 0000100) >> 3) | ((ac & 0000010) << 1) | ((ac & 0000001) << 5) | 0100);

	m_tape_puncher.timer->adjust(attotime::from_usec(15800));
}


/*
    Typewriter handling

    The alphanumeric on-line typewriter is a standard device on tx-0: it can
    both handle keyboard input and print output text.
*/

/*
    Open a file for typewriter output
*/
std::pair<std::error_condition, std::string> tx0_printer_image_device::call_load()
{
	/* open file */
	if (m_tx0)
		m_tx0->m_typewriter.fd = this;

	fseek(0, SEEK_END);

	return std::make_pair(std::error_condition(), std::string());
}

void tx0_printer_image_device::call_unload()
{
	if (m_tx0)
		m_tx0->m_typewriter.fd = nullptr;
}

/*
    Write a character to typewriter
*/
void tx0_state::typewriter_out(uint8_t data)
{
	tx0_typewriter_drawchar(data);
	if (m_typewriter.fd)
		m_typewriter.fd->fwrite(&data, 1);
}

/*
    timer callback to generate typewriter completion pulse
*/
TIMER_CALLBACK_MEMBER(tx0_state::prt_callback)
{
	m_maincpu->io_complete();
}

/*
    prt io callback
*/
void tx0_state::tx0_io_prt(int state)
{
	int ac;
	int ch;

	/* read current AC */
	ac = m_maincpu->state_int(TX0_AC);
	/* shuffle and print 6-bit word */
	ch = bitswap<6>(ac, 15, 12, 9, 6, 3, 0);
	typewriter_out(ch);

	m_typewriter.prt_timer->adjust(attotime::from_msec(100));
}


/*
    timer callback to generate crt completion pulse
*/
TIMER_CALLBACK_MEMBER(tx0_state::dis_callback)
{
	m_maincpu->io_complete();
}

/*
    Plot one point on crt
*/
void tx0_state::tx0_io_dis(int state)
{
	int ac;
	int x;
	int y;

	ac = m_maincpu->state_int(TX0_AC);
	x = ac >> 9;
	y = ac & 0777;
	tx0_plot(x, y);

	m_dis_timer->adjust(attotime::from_usec(50));
}


/*
    Magtape support

    Magtape format:

    7-track tape, 6-bit data, 1-bit parity


*/

void tx0_state::schedule_select()
{
	attotime delay = attotime::zero;

	switch (m_magtape.command)
	{
	case 0: /* backspace */
		delay = attotime::from_usec(4600);
		break;
	case 1: /* read */
		delay = attotime::from_usec(8600);
		break;
	case 2: /* rewind */
		delay = attotime::from_usec(12000);
		break;
	case 3: /* write */
		delay = attotime::from_usec(4600);
		break;
	}
	m_magtape.timer->adjust(delay);
}

void tx0_state::schedule_unselect()
{
	attotime delay = attotime::zero;

	switch (m_magtape.command)
	{
	case 0: /* backspace */
		delay = attotime::from_usec(5750);
		break;
	case 1: /* read */
		delay = attotime::from_usec(1750);
		break;
	case 2: /* rewind */
		delay = attotime::from_usec(0);
		break;
	case 3: /* write */
		delay = attotime::from_usec(5750);
		break;
	}
	m_magtape.timer->adjust(delay);
}

void tx0_magtape_image_device::device_start()
{
	m_tx0->m_magtape.img = this;
}

/*
    Open a magnetic tape image
*/
std::pair<std::error_condition, std::string> tx0_magtape_image_device::call_load()
{
	if (m_tx0)
	{
		m_tx0->m_magtape.img = this;

		m_tx0->m_magtape.irg_pos = tx0_state::MTIRGP_END;

		/* restart IO when necessary */
		/* note that this function may be called before tx0_init_machine, therefore
		before magtape.timer is allocated.  We must not call timer_enable with a
		nullptr parameter! */
		if (m_tx0->m_magtape.timer)
		{
			if (m_tx0->m_magtape.state == tx0_state::MTS_SELECTING)
				m_tx0->schedule_select();
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void tx0_magtape_image_device::call_unload()
{
	if (m_tx0)
	{
		m_tx0->m_magtape.img = nullptr;

		if (m_tx0->m_magtape.timer)
		{
			if (m_tx0->m_magtape.state == tx0_state::MTS_SELECTING)
				/* I/O has not actually started, we can cancel the selection */
				m_tx0->m_tape_reader.timer->enable(0);
			if ((m_tx0->m_magtape.state == tx0_state::MTS_SELECTED) || ((m_tx0->m_magtape.state == tx0_state::MTS_SELECTING) && (m_tx0->m_magtape.command == 2)))
			{   /* unit has become unavailable */
				m_tx0->m_magtape.state = tx0_state::MTS_UNSELECTING;
				m_tx0->m_maincpu->set_state_int(TX0_PF, m_tx0->m_maincpu->state_int(TX0_PF) | PF_RWC);
				m_tx0->schedule_unselect();
			}
		}
	}
}

void tx0_state::magtape_callback()
{
	uint8_t buf = 0;
	int lr;

	switch (m_magtape.state)
	{
	case MTS_UNSELECTING:
		m_magtape.state = MTS_UNSELECTED;
		[[fallthrough]];
	case MTS_UNSELECTED:
		if (m_magtape.sel_pending)
		{
			int mar;

			mar = m_maincpu->state_int(TX0_MAR);

			if ((mar & 03) != 1)
			{   /* unimplemented device: remain in unselected state and set rwc
			    flag? */
				m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_RWC);
			}
			else
			{
				m_magtape.state = MTS_SELECTING;

				m_magtape.command = (mar & 014 >> 2);

				m_magtape.binary_flag = (mar & 020 >> 4);

				if (m_magtape.img)
					schedule_select();
			}

			m_magtape.sel_pending = false;
			m_maincpu->io_complete();
		}
		break;

	case MTS_SELECTING:
		m_magtape.state = MTS_SELECTED;
		switch (m_magtape.command)
		{
		case 0: /* backspace */
			m_magtape.long_parity = 0177;
			m_magtape.u.backspace_state = MTBSS_STATE0;
			break;
		case 1: /* read */
			m_magtape.long_parity = 0177;
			m_magtape.u.read.state = MTRDS_STATE0;
			break;
		case 2: /* rewind */
			break;
		case 3: /* write */
			m_magtape.long_parity = 0177;
			m_magtape.u.write.state = MTWTS_STATE0;
			switch (m_magtape.irg_pos)
			{
			case MTIRGP_START:
				m_magtape.u.write.counter = 150;
				break;
			case MTIRGP_ENDMINUS1:
				m_magtape.u.write.counter = 1;
				break;
			case MTIRGP_END:
				m_magtape.u.write.counter = 0;
				break;
			}
			break;
		}
		[[fallthrough]];
	case MTS_SELECTED:
		switch (m_magtape.command)
		{
		case 0: /* backspace */
			if (m_magtape.img->ftell() == 0)
			{   /* tape at ldp */
				m_magtape.state = MTS_UNSELECTING;
				m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_RWC);
				schedule_unselect();
			}
			else if (m_magtape.img->fseek( -1, SEEK_CUR))
			{   /* eject tape */
				m_magtape.img->unload();
			}
			else if (m_magtape.img->fread(&buf, 1) != 1)
			{   /* eject tape */
				m_magtape.img->unload();
			}
			else if (m_magtape.img->fseek( -1, SEEK_CUR))
			{   /* eject tape */
				m_magtape.img->unload();
			}
			else
			{
				buf &= 0x7f;    /* 7-bit tape, ignore 8th bit */
				m_magtape.long_parity ^= buf;
				switch (m_magtape.u.backspace_state)
				{
				case MTBSS_STATE0:
					/* STATE0 -> initial interrecord gap, longitudinal parity;
					if longitudinal parity was all 0s, gap between longitudinal
					parity and data, first byte of data */
					if (buf != 0)
						m_magtape.u.backspace_state = MTBSS_STATE1;
					break;
				case MTBSS_STATE1:
					/* STATE1 -> first byte of gap between longitudinal parity and
					data, second byte of data */
					if (buf == 0)
						m_magtape.u.backspace_state = MTBSS_STATE2;
					else
						m_magtape.u.backspace_state = MTBSS_STATE5;
					break;
				case MTBSS_STATE2:
					/* STATE2 -> second byte of gap between longitudinal parity and
					data */
					if (buf == 0)
						m_magtape.u.backspace_state = MTBSS_STATE3;
					else
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					break;
				case MTBSS_STATE3:
					/* STATE3 -> third byte of gap between longitudinal parity and
					data */
					if (buf == 0)
						m_magtape.u.backspace_state = MTBSS_STATE4;
					else
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					break;
				case MTBSS_STATE4:
					/* STATE4 -> first byte of data word, first byte of
					interrecord gap after data */
					if (buf == 0)
					{
						if (m_magtape.long_parity)
							logerror("invalid longitudinal parity\n");
						/* set EOR and unselect... */
						m_magtape.state = MTS_UNSELECTING;
						m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_EOR);
						schedule_unselect();
						m_magtape.irg_pos = MTIRGP_ENDMINUS1;
					}
					else
						m_magtape.u.backspace_state = MTBSS_STATE5;
					break;
				case MTBSS_STATE5:
					/* STATE5 -> second byte of data word */
					if (buf == 0)
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					else
						m_magtape.u.backspace_state = MTBSS_STATE6;
					break;
				case MTBSS_STATE6:
					/* STATE6 -> third byte of data word */
					if (buf == 0)
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					else
						m_magtape.u.backspace_state = MTBSS_STATE6;
					break;
				}
				if (m_magtape.state != MTS_UNSELECTING)
					m_magtape.timer->adjust(attotime::from_usec(66));
			}
			break;

		case 1: /* read */
			if (m_magtape.img->fread(&buf, 1) != 1)
			{   /* I/O error or EOF? */
				/* The MAME fileio layer makes it very hard to make the
				difference...  MAME seems to assume that I/O errors never
				happen, whereas it is really easy to cause one by
				deconnecting an external drive the image is located on!!! */
				uint64_t offs;
				offs = m_magtape.img->ftell();
				if (m_magtape.img->fseek( 0, SEEK_END) || (offs != m_magtape.img->ftell()))
				{   /* I/O error */
					/* eject tape */
					m_magtape.img->unload();
				}
				else
				{   /* end of tape -> ??? */
					/* maybe we run past end of tape, so that tape is ejected from
					upper reel and unit becomes unavailable??? */
					/*m_magtape.img->unload();*/
					/* Or do we stop at EOT mark??? */
					m_magtape.state = MTS_UNSELECTING;
					m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_EOT);
					schedule_unselect();
				}
			}
			else
			{
				buf &= 0x7f;    /* 7-bit tape, ignore 8th bit */
				m_magtape.long_parity ^= buf;
				switch (m_magtape.u.read.state)
				{
				case MTRDS_STATE0:
					/* STATE0 -> interrecord blank or first byte of data */
					if (buf != 0)
					{
						if (m_magtape.cpy_pending)
						{   /* read command */
							m_magtape.u.read.space_flag = false;
							m_maincpu->set_state_int(TX0_IOS,1);
							m_maincpu->set_state_int(TX0_LR, ((m_maincpu->state_int(TX0_LR) >> 1) & 0333333)
														| ((buf & 040) << 12) | ((buf & 020) << 10) | ((buf & 010) << 8) | ((buf & 004) << 6) | ((buf & 002) << 4) | ((buf & 001) << 2));
							/* check parity */
							if (! (((buf ^ (buf >> 1) ^ (buf >> 2) ^ (buf >> 3) ^ (buf >> 4) ^ (buf >> 5) ^ (buf >> 6) ^ (buf >> 7)) & 1) ^ m_magtape.binary_flag))
								m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_PC);
						}
						else
						{   /* space command */
							m_magtape.u.read.space_flag = true;
						}
						m_magtape.u.read.state = MTRDS_STATE1;
					}
					break;
				case MTRDS_STATE1:
					/* STATE1 -> second byte of data word */
					if (buf == 0)
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					if (!m_magtape.u.read.space_flag)
					{
						m_maincpu->set_state_int(TX0_LR, ((m_maincpu->state_int(TX0_LR) >> 1) & 0333333)
													| ((buf & 040) << 12) | ((buf & 020) << 10) | ((buf & 010) << 8) | ((buf & 004) << 6) | ((buf & 002) << 4) | ((buf & 001) << 2));
						/* check parity */
						if (! (((buf ^ (buf >> 1) ^ (buf >> 2) ^ (buf >> 3) ^ (buf >> 4) ^ (buf >> 5) ^ (buf >> 6) ^ (buf >> 7)) & 1) ^ m_magtape.binary_flag))
							m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_PC);
					}
					m_magtape.u.read.state = MTRDS_STATE2;
					break;
				case MTRDS_STATE2:
					/* STATE2 -> third byte of data word */
					if (buf == 0)
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					if (!m_magtape.u.read.space_flag)
					{
						m_maincpu->set_state_int(TX0_LR, ((m_maincpu->state_int(TX0_LR) >> 1) & 0333333)
													| ((buf & 040) << 12) | ((buf & 020) << 10) | ((buf & 010) << 8) | ((buf & 004) << 6) | ((buf & 002) << 4) | ((buf & 001) << 2));
						/* check parity */
						if (! (((buf ^ (buf >> 1) ^ (buf >> 2) ^ (buf >> 3) ^ (buf >> 4) ^ (buf >> 5) ^ (buf >> 6) ^ (buf >> 7)) & 1) ^ m_magtape.binary_flag))
							m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_PC);
						/* synchronize with cpy instruction */
						if (m_magtape.cpy_pending)
							m_maincpu->set_state_int(TX0_IOS,1);
						else
							m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_RWC);
					}
					m_magtape.u.read.state = MTRDS_STATE3;
					break;
				case MTRDS_STATE3:
					/* STATE3 -> first byte of new word of data, or first byte
					of gap between data and longitudinal parity */
					if (buf != 0)
					{
						m_magtape.u.read.state = MTRDS_STATE1;
						if (!m_magtape.u.read.space_flag)
						{
							m_maincpu->set_state_int(TX0_LR, ((m_maincpu->state_int(TX0_LR) >> 1) & 0333333)
														| ((buf & 040) << 12) | ((buf & 020) << 10) | ((buf & 010) << 8) | ((buf & 004) << 6) | ((buf & 002) << 4) | ((buf & 001) << 2));
							/* check parity */
							if (! (((buf ^ (buf >> 1) ^ (buf >> 2) ^ (buf >> 3) ^ (buf >> 4) ^ (buf >> 5) ^ (buf >> 6) ^ (buf >> 7)) & 1) ^ m_magtape.binary_flag))
								m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_PC);
						}
					}
					else
						m_magtape.u.read.state = MTRDS_STATE4;
					break;
				case MTRDS_STATE4:
					/* STATE4 -> second byte of gap between data and
					longitudinal parity */
					if (buf != 0)
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					else
						m_magtape.u.read.state = MTRDS_STATE5;
					break;

				case MTRDS_STATE5:
					/* STATE5 -> third byte of gap between data and
					longitudinal parity */
					if (buf != 0)
					{
						logerror("tape seems to be corrupt\n");
						/* eject tape */
						m_magtape.img->unload();
					}
					else
						m_magtape.u.read.state = MTRDS_STATE6;
					break;

				case MTRDS_STATE6:
					/* STATE6 -> longitudinal parity */
					/* check parity */
					if (m_magtape.long_parity)
					{
						logerror("invalid longitudinal parity\n");
						/* no idea if the original tx-0 magtape controller
						checks parity, but can't harm if we do */
						m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_PC);
					}
					/* set EOR and unselect... */
					m_magtape.state = MTS_UNSELECTING;
					m_maincpu->set_state_int(TX0_PF, m_maincpu->state_int(TX0_PF) | PF_EOR);
					schedule_unselect();
					m_magtape.irg_pos = MTIRGP_START;
					break;
				}
				if (m_magtape.state != MTS_UNSELECTING)
					m_magtape.timer->adjust(attotime::from_usec(66));
			}
			break;

		case 2: /* rewind */
			m_magtape.state = MTS_UNSELECTING;
			/* we rewind at 10*read speed (I don't know the real value) */
			m_magtape.timer->adjust((attotime::from_nsec(6600) * m_magtape.img->ftell()));
			//schedule_unselect(state);
			m_magtape.img->fseek( 0, SEEK_END);
			m_magtape.irg_pos = MTIRGP_END;
			break;

		case 3: /* write */
			switch (m_magtape.u.write.state)
			{
			case MTWTS_STATE0:
				if (m_magtape.u.write.counter != 0)
				{
					m_magtape.u.write.counter--;
					buf = 0;
					break;
				}
				else
				{
					m_magtape.u.write.state = MTWTS_STATE1;
				}
				[[fallthrough]];
			case MTWTS_STATE1:
				if (m_magtape.u.write.counter)
				{
					m_magtape.u.write.counter--;
					lr = m_maincpu->state_int(TX0_LR);
					buf = ((lr >> 10) & 040) | ((lr >> 8) & 020) | ((lr >> 6) & 010) | ((lr >> 4) & 004) | ((lr >> 2) & 002) | (lr & 001);
					buf |= ((buf << 1) ^ (buf << 2) ^ (buf << 3) ^ (buf << 4) ^ (buf << 5) ^ (buf << 6) ^ ((!m_magtape.binary_flag) << 6)) & 0100;
					m_maincpu->set_state_int(TX0_LR, lr >> 1);
				}
				else
				{
					if (m_magtape.cpy_pending)
					{
						m_maincpu->set_state_int(TX0_IOS,1);
						lr = m_maincpu->state_int(TX0_LR);
						buf = ((lr >> 10) & 040) | ((lr >> 8) & 020) | ((lr >> 6) & 010) | ((lr >> 4) & 004) | ((lr >> 2) & 002) | (lr & 001);
						buf |= ((buf << 1) ^ (buf << 2) ^ (buf << 3) ^ (buf << 4) ^ (buf << 5) ^ (buf << 6) ^ ((!m_magtape.binary_flag) << 6)) & 0100;
						m_maincpu->set_state_int(TX0_LR, lr >> 1);
						m_magtape.u.write.counter = 2;
						break;
					}
					else
					{
						m_magtape.u.write.state = MTWTS_STATE2;
						m_magtape.u.write.counter = 3;
					}
				}
				[[fallthrough]];
			case MTWTS_STATE2:
				if (m_magtape.u.write.counter != 0)
				{
					m_magtape.u.write.counter--;
					buf = 0;
					break;
				}
				else
				{
					buf = m_magtape.long_parity;
					m_magtape.state = (state_t)MTWTS_STATE3;
					m_magtape.u.write.counter = 150;
				}
				break;

			case MTWTS_STATE3:
				if (m_magtape.u.write.counter != 0)
				{
					m_magtape.u.write.counter--;
					buf = 0;
					break;
				}
				else
				{
					m_magtape.state = MTS_UNSELECTING;
					schedule_unselect();
					m_magtape.irg_pos = MTIRGP_END;
				}
				break;
			}
			if (m_magtape.state != MTS_UNSELECTING)
			{   /* write data word */
				m_magtape.long_parity ^= buf;
				if (m_magtape.img->fwrite(&buf, 1) != 1)
				{   /* I/O error */
					/* eject tape */
					m_magtape.img->unload();
				}
				else
					m_magtape.timer->adjust(attotime::from_usec(66));
			}
			break;
		}
		break;
	}
}

void tx0_state::tx0_sel(int state)
{
	m_magtape.sel_pending = true;

	if (m_magtape.state == MTS_UNSELECTED)
	{
		if (0)
			magtape_callback();
		m_magtape.timer->adjust(attotime::zero);
	}
}

void tx0_state::tx0_io_cpy(int state)
{
	switch (m_magtape.state)
	{
	case MTS_UNSELECTED:
	case MTS_UNSELECTING:
		/* ignore instruction and set rwc flag? */
		m_maincpu->io_complete();
		break;

	case MTS_SELECTING:
	case MTS_SELECTED:
		switch (m_magtape.command)
		{
		case 0: /* backspace */
		case 2: /* rewind */
			/* ignore instruction and set rwc flag? */
			m_maincpu->io_complete();
			break;
		case 1: /* read */
		case 3: /* write */
			m_magtape.cpy_pending = true;
			break;
		}
		break;
	}
}


/*
    callback which is called when reset line is pulsed

    IO devices should reset
*/
void tx0_state::tx0_io_reset_callback(int state)
{
	m_tape_reader.rcl = m_tape_reader.rc = 0;
	if (m_tape_reader.timer)
		m_tape_reader.timer->enable(0);

	if (m_tape_puncher.timer)
		m_tape_puncher.timer->enable(0);

	if (m_typewriter.prt_timer)
		m_typewriter.prt_timer->enable(0);

	if (m_dis_timer)
		m_dis_timer->enable(0);
}


/*
    typewriter keyboard handler
*/
void tx0_state::tx0_keyboard()
{
	int i;
	int j;

	int typewriter_keys[4];

	int typewriter_transitions;
	int charcode, lr;

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
			charcode = (i << 4) + j;
			/* shuffle and insert data into LR */
			lr = (1 << 17) | (charcode << 11) | m_maincpu->state_int(TX0_LR);
			/* write modified LR */
			m_maincpu->set_state_int(TX0_LR, lr);
			tx0_typewriter_drawchar(charcode); /* we want to echo input */
			break;
		}
	}

	for (i=0; i<4; i++)
		m_old_typewriter_keys[i] = typewriter_keys[i];
}

/*
    Not a real interrupt - just handle keyboard input
*/
INTERRUPT_GEN_MEMBER(tx0_state::tx0_interrupt)
{
	int control_keys;
	int tsr_keys;

	int control_transitions;
	int tsr_transitions;


	/* read new state of control keys */
	control_keys = m_csw->read();

	if (control_keys & tx0_control)
	{
		/* compute transitions */
		control_transitions = control_keys & (~ m_old_control_keys);

		if (control_transitions & tx0_stop_cyc0)
		{
			m_maincpu->set_state_int(TX0_STOP_CYC0, !m_maincpu->state_int(TX0_STOP_CYC0));
		}
		if (control_transitions & tx0_stop_cyc1)
		{
			m_maincpu->set_state_int(TX0_STOP_CYC1, !m_maincpu->state_int(TX0_STOP_CYC1));
		}
		if (control_transitions & tx0_gbl_cm_sel)
		{
			m_maincpu->set_state_int(TX0_GBL_CM_SEL, !m_maincpu->state_int(TX0_GBL_CM_SEL));
		}
		if (control_transitions & tx0_stop)
		{
			m_maincpu->set_state_int(TX0_RUN, (uint64_t)0);
			m_maincpu->set_state_int(TX0_RIM, (uint64_t)0);
		}
		if (control_transitions & tx0_restart)
		{
			m_maincpu->set_state_int(TX0_RUN, 1);
			m_maincpu->set_state_int(TX0_RIM, (uint64_t)0);
		}
		if (control_transitions & tx0_read_in)
		{   /* set cpu to read instructions from perforated tape */
			m_maincpu->pulse_reset();
			m_maincpu->set_state_int(TX0_RUN, (uint64_t)0);
			m_maincpu->set_state_int(TX0_RIM, 1);
		}
		if (control_transitions & tx0_toggle_dn)
		{
			m_tsr_index++;
			if (m_tsr_index == 18)
				m_tsr_index = 0;
		}
		if (control_transitions & tx0_toggle_up)
		{
			m_tsr_index--;
			if (m_tsr_index == -1)
				m_tsr_index = 17;
		}
		if (control_transitions & tx0_cm_sel)
		{
			if (m_tsr_index >= 2)
			{
				uint32_t cm_sel = (uint32_t) m_maincpu->state_int(TX0_CM_SEL);
				m_maincpu->set_state_int(TX0_CM_SEL, cm_sel ^ (1 << (m_tsr_index - 2)));
			}
		}
		if (control_transitions & tx0_lr_sel)
		{
			if (m_tsr_index >= 2)
			{
				uint32_t lr_sel = (uint32_t) m_maincpu->state_int(TX0_LR_SEL);
				m_maincpu->set_state_int(TX0_LR_SEL, (lr_sel ^ (1 << (m_tsr_index - 2))));
			}
		}

		/* remember new state of control keys */
		m_old_control_keys = control_keys;


		/* handle toggle switch register keys */
		tsr_keys = (ioport("MSW")->read() << 16) | ioport("LSW")->read();

		/* compute transitions */
		tsr_transitions = tsr_keys & (~ m_old_tsr_keys);

		/* update toggle switch register */
		if (tsr_transitions)
			m_maincpu->set_state_int(TX0_TBR+m_tsr_index, m_maincpu->state_int(TX0_TBR+m_tsr_index) ^ tsr_transitions);

		/* remember new state of toggle switch register keys */
		m_old_tsr_keys = tsr_keys;
	}
	else
	{
		m_old_control_keys = 0;
		m_old_tsr_keys = 0;

		tx0_keyboard();
	}
}

void tx0_state::tx0_64kw(machine_config &config)
{
	/* basic machine hardware */
	/* TX0 CPU @ approx. 167 kHz (no master clock, but the memory cycle time is approximately 6usec) */
	TX0_64KW(config, m_maincpu, 166667);
	m_maincpu->cpy().set(FUNC(tx0_state::tx0_io_cpy));
	m_maincpu->r1l().set(FUNC(tx0_state::tx0_io_r1l));
	m_maincpu->dis().set(FUNC(tx0_state::tx0_io_dis));
	m_maincpu->r3l().set(FUNC(tx0_state::tx0_io_r3l));
	m_maincpu->prt().set(FUNC(tx0_state::tx0_io_prt));
	m_maincpu->rsv().set_nop();
	m_maincpu->p6h().set(FUNC(tx0_state::tx0_io_p6h));
	m_maincpu->p7h().set(FUNC(tx0_state::tx0_io_p7h));
	m_maincpu->sel().set(FUNC(tx0_state::tx0_sel));
	m_maincpu->res().set(FUNC(tx0_state::tx0_io_reset_callback));
	m_maincpu->set_addrmap(AS_PROGRAM, &tx0_state::tx0_64kw_map);
	/* dummy interrupt: handles input */
	m_maincpu->set_vblank_int("screen", FUNC(tx0_state::tx0_interrupt));

	/* video hardware (includes the control panel and typewriter output) */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(refresh_rate);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(virtual_width, virtual_height);
	screen.set_visarea(0, virtual_width-1, 0, virtual_height-1);
	screen.set_screen_update(FUNC(tx0_state::screen_update_tx0));
	screen.screen_vblank().set(FUNC(tx0_state::screen_vblank_tx0));
	screen.set_palette(m_palette);

	CRT(config, m_crt, 0);
	m_crt->set_num_levels(pen_crt_num_levels);
	m_crt->set_offsets(crt_window_offset_x, crt_window_offset_y);
	m_crt->set_size(crt_window_width, crt_window_height);

	TX0_READTAPE(config, "readt", 0);
	TX0_PUNCHTAPE(config, "punch", 0);
	TX0_PRINTER(config, "typewriter", 0);
	TX0_MAGTAPE(config, "magtape", 0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tx0);
	PALETTE(config, m_palette, FUNC(tx0_state::tx0_palette), total_colors_needed + sizeof(tx0_pens), total_colors_needed);

	SOFTWARE_LIST(config, "ptp_list").set_original("tx0_ptp").set_filter("64");
}

void tx0_state::tx0_8kwo(machine_config &config)
{
	tx0_64kw(config);

	/* basic machine hardware */
	/* TX0 CPU @ approx. 167 kHz (no master clock, but the memory cycle time is
	approximately 6usec) */
	TX0_8KW_OLD(config.replace(), m_maincpu, 166667);
	m_maincpu->cpy().set(FUNC(tx0_state::tx0_io_cpy));
	m_maincpu->r1l().set(FUNC(tx0_state::tx0_io_r1l));
	m_maincpu->dis().set(FUNC(tx0_state::tx0_io_dis));
	m_maincpu->r3l().set(FUNC(tx0_state::tx0_io_r3l));
	m_maincpu->prt().set(FUNC(tx0_state::tx0_io_prt));
	m_maincpu->rsv().set_nop();
	m_maincpu->p6h().set(FUNC(tx0_state::tx0_io_p6h));
	m_maincpu->p7h().set(FUNC(tx0_state::tx0_io_p7h));
	m_maincpu->sel().set(FUNC(tx0_state::tx0_sel));
	m_maincpu->res().set(FUNC(tx0_state::tx0_io_reset_callback));
	m_maincpu->set_addrmap(AS_PROGRAM, &tx0_state::tx0_8kw_map);
	/* dummy interrupt: handles input */
	m_maincpu->set_vblank_int("screen", FUNC(tx0_state::tx0_interrupt));

	SOFTWARE_LIST(config.replace(), "ptp_list").set_original("tx0_ptp").set_filter("8O");
}

void tx0_state::tx0_8kw(machine_config &config)
{
	tx0_64kw(config);

	/* basic machine hardware */
	/* TX0 CPU @ approx. 167 kHz (no master clock, but the memory cycle time is
	approximately 6usec) */
	TX0_8KW(config.replace(), m_maincpu, 166667);
	m_maincpu->cpy().set(FUNC(tx0_state::tx0_io_cpy));
	m_maincpu->r1l().set(FUNC(tx0_state::tx0_io_r1l));
	m_maincpu->dis().set(FUNC(tx0_state::tx0_io_dis));
	m_maincpu->r3l().set(FUNC(tx0_state::tx0_io_r3l));
	m_maincpu->prt().set(FUNC(tx0_state::tx0_io_prt));
	m_maincpu->rsv().set_nop();
	m_maincpu->p6h().set(FUNC(tx0_state::tx0_io_p6h));
	m_maincpu->p7h().set(FUNC(tx0_state::tx0_io_p7h));
	m_maincpu->sel().set(FUNC(tx0_state::tx0_sel));
	m_maincpu->res().set(FUNC(tx0_state::tx0_io_reset_callback));
	m_maincpu->set_addrmap(AS_PROGRAM, &tx0_state::tx0_8kw_map);
	/* dummy interrupt: handles input */
	m_maincpu->set_vblank_int("screen", FUNC(tx0_state::tx0_interrupt));

	SOFTWARE_LIST(config.replace(), "ptp_list").set_original("tx0_ptp").set_filter("8N");
}

ROM_START(tx0_64kw)
	ROM_REGION(tx0_fontdata_size, "gfx1", ROMREGION_ERASEFF)
		/* space filled with our font */
ROM_END

ROM_START(tx0_8kwo)
	ROM_REGION(tx0_fontdata_size, "gfx1", ROMREGION_ERASEFF)
		/* space filled with our font */
ROM_END

ROM_START(tx0_8kw)
	ROM_REGION(tx0_fontdata_size, "gfx1", ROMREGION_ERASEFF)
		/* space filled with our font */
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT  STATE      INIT      COMPANY  FULLNAME                                                  FLAGS
COMP( 1956, tx0_64kw, tx0_8kw,  0,      tx0_64kw, tx0,   tx0_state, init_tx0, "MIT",   "TX-0 original demonstrator (64 kWords of RAM)",          MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
COMP( 1959, tx0_8kwo, tx0_8kw,  0,      tx0_8kwo, tx0,   tx0_state, init_tx0, "MIT",   "TX-0 upgraded system (8 kWords of RAM, old order code)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
COMP( 1962, tx0_8kw,  0,        0,      tx0_8kw,  tx0,   tx0_state, init_tx0, "MIT",   "TX-0 upgraded system (8 kWords of RAM, new order code)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
