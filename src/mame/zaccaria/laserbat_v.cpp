// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Laser Battle / Lazarian (c) 1981 Zaccaria
    Cat and Mouse           (c) 1982 Zaccaria

    video emulation by Vas Crabb

    This is an absolutely insane arrangement of three Signetics S2623
    PVIs and custom TTL logic.  The PVIs can each render up to four
    (potentially duplicated) sprites.  The TTL logic renders a single
    32x32 pixel 4-colour sprite and an 8-colour background tilemap.
    There are also two symmetrical area effects where the game only
    needs to program the horizontal distance from the screen edge for
    each line, and a per-line single-pixel shell effect.  The shell
    effect replaces one of the area effects if used.

    In order to get 30% more horizontal resolution than Signetics
    intended, this board divides the master clock by 4 to drive the
    S2621 sync generator, but has a separate set of filp-flops to do
    a symmetric divide by 3 to drive the rest of the video hardware.
    There's some extra logic to align the first pixel to the end of the
    horizontal blanking period period because the line isn't a whole
    number of pixels.  The visible portion isn't a whole number of
    pixels for that matter, either.

    There's some fancy logic to stretch the vertical blanking period for
    eight additional lines after the USG deasserts VRST and then to
    start the next vertical blanking period after 247 visible lines.

    The first visible line is line 8 from the point of view of the PVIs,
    background generator and sprite generator.

    The first visible column of the display is pixel 8 from the point of
    of view of the sprite and background generation hardware, but it's
    pixel 0 from the point of view of the PVIs.

    The hardware has 8-bit RRRGGGBBB output converted to analog levels
    with a simple resistor network driven by open-collector gates.
    However video is actually generated in a 16-bit internal colour
    space and mapped onto the 8-bit output colour space using a PLA.

    The equations in the PLA for Laser Battle/Lazarian give the
    following graphics priorities, from highest to lowest:
    * TTL-generated sprite
    * PVIs (colours ORed, object/score output ignored)
    * Shell/area effect 2
    * Background tilemap
    * Area effect 1

    The Cat and Mouse PLA program gives completely different priorities,
    once again from highest to lowest:
    * Background tilemap
    * PVIs (colours ORed, object/score output ignored)
    * TTL-generated sprite
    * Shell

    Cat and Mouse uses some signals completely differently.  LUM affects
    the background palette rather than the sprite palette, area effect 1
    affects the background palette, and area effect 2 is completely
    unused.

    The game board has no logic for flipping the screen in cocktail
    mode.  It just provides an active-low open collector out with pull-
    up indicating when player 2 is playing.  In a cocktail cabinet this
    goes to an "image commutation board".  It's not connected to
    anything in an upright cabinet.  The "image commutation board" must
    flip the image somehow, presumably by reversing the deflection coil
    connections.

    There are still issues with horizontal alignment between layers.  I
    have the schematic, yet I really can't understand where these issues
    are coming from.  I'm pretty sure alignment between TTL background
    and sprites is right, judging from gameplay.  I'm not sure about
    alignment with the effect layers.

    There are definitely alignment problems with the PVI objects, but
    that may be a bug in the S2636 implementation.  I need to check it
    in more detail.
*/

#include "emu.h"
#include "laserbat.h"


void laserbat_state_base::videoram_w(offs_t offset, uint8_t data)
{
	if (!m_mpx_bkeff)
		m_bg_ram[offset] = data;
	else
		m_eff_ram[offset & 0x1ff] = data; // A9 is not connected, only half the chip is used
}

void laserbat_state_base::wcoh_w(uint8_t data)
{
	// sprite horizontal offset
	m_wcoh = data;
}

void laserbat_state_base::wcov_w(uint8_t data)
{
	// sprite vertical offset
	m_wcov = data;
}

void laserbat_state_base::cnt_eff_w(uint8_t data)
{
	/*
	    +-----+-------------+-----------------------------------------------+
	    | bit |    name     | description                                   |
	    +-----+-------------+-----------------------------------------------+
	    |  0  | /ABEFF1     | effect 1 enable                               |
	    |  1  | /ABEFF2     | effect 2/shell enable                         |
	    |  2  | MPX EFF2 SH | select SHELL point or EFF2 area for effect 2  |
	    |  3  | COLEFF 0    | area effect colour bit 0                      |
	    |  4  | COLEFF 1    | area effect colour bit 1                      |
	    |  5  | /NEG 1      | select inside/outside area for effect 1       |
	    |  6  | /NEG 2      | select inside/outside area for effect 2       |
	    |  7  | MPX P_1/2   | selects input row 2                           |
	    +-----+-------------+-----------------------------------------------+
	*/

	m_abeff1 = !bool(data & 0x01);
	m_abeff2 = !bool(data & 0x02);
	m_mpx_eff2_sh = bool(data & 0x04);
	m_coleff = (data >> 3) & 0x03;
	m_neg1 = !bool(data & 0x20);
	m_neg2 = !bool(data & 0x40);
	m_mpx_p_1_2 = bool(data & 0x80);

//  popmessage("effect: 0x%02X", data);
}

void laserbat_state_base::cnt_nav_w(uint8_t data)
{
	/*
	    +-----+-----------+--------------------------------------+
	    | bit |   name    | description                          |
	    +-----+-----------+--------------------------------------+
	    |  0  | /NAVE     | sprite enable                        |
	    |  1  | CLR0      | sprite colour bit 0                  |
	    |  2  | CLR1      | sprite colour bit 1                  |
	    |  3  | LUM       | sprite luminance                     |
	    |  4  | MPX BKEFF | access background RAM or effect RAM  |
	    |  5  | SHPA      | sprite select bit 0                  |
	    |  6  | SHPB      | sprite select bit 1                  |
	    |  7  | SHPC      | sprite select bit 2                  |
	    +-----+-----------+--------------------------------------+
	*/

	m_nave = !bool(data & 0x01);
	m_clr_lum = (data >> 1) & 0x07;
	m_mpx_bkeff = bool(data & 0x10);
	m_shp = (data >> 5) & 0x07;

//  popmessage("nav: 0x%02X", data);
}


uint32_t laserbat_state_base::screen_update_laserbat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool const flip_y = flip_screen_y(), flip_x = flip_screen_x();
	int const offs_y = m_screen->visible_area().max_y + m_screen->visible_area().min_y;
	int const offs_x = m_screen->visible_area().max_x + m_screen->visible_area().min_x;

	for (int y = cliprect.min_y; cliprect.max_y >= y; y++)
	{
		uint16_t const *const src = &m_bitmap.pix(flip_y ? (offs_y - y) : y);
		uint16_t *dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; cliprect.max_x >= x; x++)
		{
			dst[x] = uint16_t(m_gfxmix->read(src[flip_x ? (offs_x - x) : x]));
		}
	}

	return 0;
}


TIMER_CALLBACK_MEMBER(laserbat_state_base::video_line)
{
	/*
	    +-----+---------+----------------------------------+-------------------------------------+
	    | bit |  name   | laserbat/lazarian                | catnmous                            |
	    +-----+---------+----------------------------------+-------------------------------------+
	    |  0  | NAV0    | sprite bit 0                     | sprite bit 0                        |
	    |  1  | NAV1    | sprite bit 1                     | sprite bit 1                        |
	    |  2  | CLR0    | sprite palette bit 0             | sprite palette bit 0                |
	    |  3  | CLR1    | sprite palette bit 1             | sprite palette bit 1                |
	    |  4  | LUM     | sprite luminance                 | background tilemap palette control  |
	    |  5  | C1*     | combined PVI red (active low)    | combined PVI red (active low)       |
	    |  6  | C2*     | combined PVI green (active low)  | combined PVI green (active low)     |
	    |  7  | C3*     | combined PVI blue (active low)   | combined PVI blue (active low)      |
	    |  8  | BKR     | background tilemap red           | background tilemap bit 0            |
	    |  9  | BKG     | background tilemap green         | background tilemap bit 1            |
	    | 10  | BKB     | background tilemap blue          | background tilemap bit 2            |
	    | 11  | SHELL   | shell point                      | shell point                         |
	    | 12  | EFF1    | effect 1 area                    | background tilemap palette control  |
	    | 13  | EFF2    | effect 2 area                    | unused                              |
	    | 14  | COLEFF0 | area effect colour bit 0         | background tilemap palette control  |
	    | 15  | COLEFF1 | area effect colour bit 1         | background tilemap palette control  |
	    +-----+---------+----------------------------------+-------------------------------------+
	*/

	assert(m_bitmap.width() > m_screen->visible_area().max_x);
	assert(m_bitmap.height() > m_screen->visible_area().max_y);

	// prep some useful values
	int const y = m_screen->vpos();
	int const min_x = m_screen->visible_area().min_x;
	int const max_x = m_screen->visible_area().max_x;
	int const x_offset = min_x - (8 * 3);
	int const y_offset = m_screen->visible_area().min_y - 8;
	uint16_t *const row = &m_bitmap.pix(y);

	// wait for next scanline
	m_scanline_timer->adjust(m_screen->time_until_pos(y + 1, 0));

	// update the PVIs
	if (!y)
	{
		m_pvi[0]->start_new_frame();
		m_pvi[1]->start_new_frame();
		m_pvi[2]->start_new_frame();
	}
	m_pvi[0]->render_next_line();
	m_pvi[1]->render_next_line();
	m_pvi[2]->render_next_line();

	uint16_t const *const pvi1_row = &m_pvi[0]->bitmap().pix(y);
	uint16_t const *const pvi2_row = &m_pvi[1]->bitmap().pix(y);
	uint16_t const *const pvi3_row = &m_pvi[2]->bitmap().pix(y);

	// don't draw outside the visible area
	m_bitmap.plot_box(0, y, m_bitmap.width(), 1, 0);
	if ((m_screen->visible_area().min_y > y) || (m_screen->visible_area().max_y < y))
		return;

	// render static effect bits
	uint16_t const static_bits = ((uint16_t(m_coleff) << 14) & 0xc000) | ((uint16_t(m_clr_lum) << 2) & 0x001c);
	m_bitmap.plot_box(min_x, y, max_x - min_x + 1, 1, static_bits);

	// render the TTL-generated background tilemap
	unsigned const bg_row = (y - y_offset) & 0x07;
	uint8_t const *const bg_src = &m_bg_ram[((y - y_offset) << 2) & 0x3e0];
	for (unsigned byte = 0, px = x_offset + (9 * 3); max_x >= px; byte++)
	{
		uint16_t const tile = (uint16_t(bg_src[byte & 0x1f]) << 3) & 0x7f8;
		uint8_t red   = m_gfx1[0x0000 | tile | bg_row];
		uint8_t green = m_gfx1[0x0800 | tile | bg_row];
		uint8_t blue  = m_gfx1[0x1000 | tile | bg_row];
		for (unsigned pixel = 0; 8 > pixel; pixel++, red <<= 1, green <<= 1, blue <<= 1)
		{
			uint16_t const bg = ((red & 0x80) ? 0x0100 : 0x0000) | ((green & 0x80) ? 0x0200 : 0x0000) | ((blue & 0x80) ? 0x0400 : 0x0000);
			if ((min_x <= px) && (max_x >= px)) row[px] |= bg;
			px++;
			if ((min_x <= px) && (max_x >= px)) row[px] |= bg;
			px++;
			if ((min_x <= px) && (max_x >= px)) row[px] |= bg;
			px++;
		}
	}

	// render shell/effect graphics
	uint8_t const eff1_val = m_eff_ram[((y - y_offset) & 0xff) | 0x100];
	uint8_t const eff2_val = m_eff_ram[((y - y_offset) & 0xff) | 0x000];
	for (int x = 0, px = x_offset; max_x >= px; x++)
	{
		// calculate area effects
		bool const right_half = bool(x & 0x80);
		bool const eff1_cmp = right_half ? (uint8_t(x & 0x7f) < (eff1_val & 0x7f)) : (uint8_t(x & 0x7f) > (~eff1_val & 0x7f));
		bool const eff2_cmp = right_half ? ((uint8_t(x & 0x7f) | m_eff2_mask) < ((eff2_val & 0x7f) | m_eff2_mask)) : ((uint8_t(x & 0x7f) | m_eff2_mask) > ((~eff2_val & 0x7f) | m_eff2_mask));
		bool const eff1 = m_abeff1 && (m_neg1 ? !eff1_cmp : eff1_cmp);
		bool const eff2 = m_abeff2 && (m_neg2 ? !eff2_cmp : eff2_cmp) && m_mpx_eff2_sh;

		// calculate shell point effect
		bool const shell = m_abeff2 && ((uint8_t(x & 0xff) | m_eff2_mask) == ((eff2_val & 0xff) | m_eff2_mask)) && !m_mpx_eff2_sh;

		// set effect bits, and mix in PVI graphics while we're here
		uint16_t const effect_bits = (shell ? 0x0800 : 0x0000) | (eff1 ? 0x1000 : 0x0000) | (eff2 ? 0x2000 : 0x0000);
		uint16_t pvi_bits = ~(pvi1_row[px] | pvi2_row[px] | pvi3_row[px]);
		pvi_bits = ((pvi_bits & 0x01) << 7) | ((pvi_bits & 0x02) << 5) | ((pvi_bits & 0x04) << 3);
		if ((min_x <= px) && (max_x >= px)) row[px] |= effect_bits | pvi_bits;
		px++;
		if ((min_x <= px) && (max_x >= px)) row[px] |= effect_bits | pvi_bits;
		px++;
		if ((min_x <= px) && (max_x >= px)) row[px] |= effect_bits | pvi_bits;
		px++;
	}

	// render the TTL-generated sprite
	// magic offsets here I don't understand the source of
	if (m_nave)
	{
		int const sprite_row = y + y_offset - ((256 - m_wcov) & 0x0ff);
		if ((0 <= sprite_row) && (32 > sprite_row))
		{
			for (unsigned byte = 0, x = x_offset + (3 * ((256 - m_wcoh + 5) & 0x0ff)); 8 > byte; byte++)
			{
				uint8_t bits = m_gfx2[m_gfx2_base | ((m_shp << 8) & 0x700) | ((sprite_row << 3) & 0x0f8) | (byte & 0x07)];
				for (unsigned pixel = 0; 4 > pixel; pixel++, bits <<= 2)
				{
					if (max_x >= x) row[x++] |= (bits >> 6) & 0x03;
					if (max_x >= x) row[x++] |= (bits >> 6) & 0x03;
					if (max_x >= x) row[x++] |= (bits >> 6) & 0x03;
				}
			}
		}
	}
}


void laserbat_state::laserbat_palette(palette_device &palette) const
{
	/*
	    Uses GRBGRBGR pixel format.  The two topmost bist are the LSBs
	    for red and green.  LSB for blue is always effectively 1.  The
	    middle group is the MSB.  Yet another crazy thing they did.

	    Each colour channel has an emitter follower buffer amplifier
	    biased with a 1k resistor to +5V and a 3k3 resistor to ground.
	    Output is adjusted by connecting additional resistors across the
	    leg to ground using an open collector buffer - 270R, 820R and
	    1k0 for unset MSB to LSB, respectively (blue has no LSB so it
	    has no 1k0 resistor).

	    Assuming 0.7V drop across the emitter follower and no drop
	    across the open collector buffer, these are the approximate
	    output voltages:

	    0.0000, 0.1031, 0.1324, 0.2987, 0.7194, 1.2821, 1.4711, 3.1372

	    The game never sets the colour to any value above 4, effectively
	    treating it as 5-level red and green, and 3-level blue, for a
	    total of 75 usable colours.

	    From the fact that there's no DC offset on red and green, and
	    the highest value used is just over 0.7V, I'm guessing the game
	    expects to drive a standard 0.7V RGB monitor, and higher colour
	    values would simply saturate the input.  To make it not look
	    like the inside of a coal mine, I've applied gamma decoding at
	    2.2

	    However there's that nasty DC offset on the blue caused by the
	    fact that it has no LSB, but it's eliminated at the AC-coupling
	    of the input and output of the buffer amplifier on the monitor
	    interface board.  I'm treating it as though it has the same gain
	    as the other channels.  After gamma adjustment, medium red and
	    medium blue as used by the game have almost the same intensity.
	*/

	int const weights[] = { 0, 107, 120, 173, 255, 255, 255, 255 };
	int const blue_weights[] = { 0, 0, 60, 121, 241, 255, 255, 255 };
	for (int entry = 0; palette.entries() > entry; entry++)
	{
		uint8_t const bits(entry & 0xff);
		uint8_t const r(((bits & 0x01) << 1) | ((bits & 0x08) >> 1) | ((bits & 0x40) >> 6));
		uint8_t const g(((bits & 0x02) >> 0) | ((bits & 0x10) >> 2) | ((bits & 0x80) >> 7));
		uint8_t const b(((bits & 0x04) >> 1) | ((bits & 0x20) >> 3) | 0x01);
		palette.set_pen_color(entry, rgb_t(weights[r], weights[g], blue_weights[b]));
	}
}


void catnmous_state::catnmous_palette(palette_device &palette) const
{
	/*
	    Uses GRBGRBGR pixel format.  The two topmost bist are the LSBs
	    for red and green.  The middle group is the MSB.  Yet another
	    crazy thing they did.

	    Each colour channel has an emitter follower buffer amlpifier
	    biased with a 1k resistor to +5V and a 3k3 resistor to ground.
	    Output is adjusted by connecting additional resistors across the
	    leg to ground using an open collector buffer.  Red and green use
	    560R, 820R and 1k0 for unset MSB to LSB, respectively.  Blue
	    uses 47R and 820R on the PCB we have a photo of, although the
	    47R resistor looks like it could be a bad repair (opposite
	    orientation and burn marks on PCB).

	    Assuming 0.7V drop across the emitter follower and no drop
	    across the open collector buffer, these are the approximate
	    output voltages for red and green:

	    0.2419, 0.4606, 0.5229, 0.7194, 0.9188, 1.2821, 1.4711, 3.1372

	    The game uses all colour values except 4.  The DC offset will be
	    eliminated by the AC coupling on the monitor interface board.
	    The differences steps aren't very linear, they vary from 0.06V
	    to 0.36V with no particular order.  The input would be expected
	    to saturate somewhere inside the big jump to the highest level.

	    Let's assume the 47R resistor is a bad repair and it's supposed
	    to be 470R.  That gives us these output voltages for blue:

	    0.3752, 0.7574, 1.2821, 3.1372

	    To make life easier, I'll assume the monitor is expected to have
	    half the gain of a standard monitor and no gamma decoding is
	    necessary.
	*/

	int const weights[] = { 0, 40, 51, 87, 123, 189, 224, 255 };
	int const blue_weights[] = { 0, 70, 165, 255 };
	for (int entry = 0; palette.entries() > entry; entry++)
	{
		uint8_t const bits(entry & 0xff);
		uint8_t const r(((bits & 0x01) << 1) | ((bits & 0x08) >> 1) | ((bits & 0x40) >> 6));
		uint8_t const g(((bits & 0x02) >> 0) | ((bits & 0x10) >> 2) | ((bits & 0x80) >> 7));
		uint8_t const b(((bits & 0x04) >> 2) | ((bits & 0x20) >> 4));
		palette.set_pen_color(entry, rgb_t(weights[r], weights[g], blue_weights[b]));
	}
}
