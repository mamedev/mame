// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Couriersud, Robbbert
/***************************************************************************

    Galaxian-derived video hardware

****************************************************************************

    Video timing:

        The master clock is an 18.432MHz crystal. It is divided by 3 by
        a pair of J/K flip-flops to 6.144MHz. This 6MHz signal is used to
        drive most of the video logic. Note that due to the way the
        divide-by-3 circuit is implemented, the duty cycle of the 6MHz
        signal is 66% (i.e., it is high for 2 18MHz clocks and low for 1).
        This is important for accurate stars rendering.


    Horizontal timing:

        H counts from 010000000 (128) to 111111111 (511), giving 384
        total H clocks per scanline

        However, the top bit is inverted to become 256H, so when reading
        the schematics it's really counting from:
            110000000 -> 111111111 (blanking period)
        and then from:
            000000000 -> 011111111 (main portion of screen = 256 pixels)

        HBLANK is a flip-flop clocked by 2H:
          * It is held clear when 256H = 0 (main portion of screen)
          * The D input is connected to !(64H & 32H & 16H & 8H)
          * It is clocked to 1 when H=130
          * It is clocked to 0 when H=250
          * This gives 264 total non-blanked pixels:
              6 additional pixels on the left (H=250..255)
              256 main area pixels (H=256..511)
              2 additional pixels on the right (H=128..129)

        HSYNC is a flip-flop clocked by 16H:
          * It is held clear when 256H = 0 (main portion of screen)
          * The D input is connected to !(!64H & 32H)
          * HSYNC is the /Q output
          * It is clocked to 1 when H=176
          * It is clocked to 0 when H=208


    Vertical timing:

        V counts from 011111000 (248) to 111111111 (511), giving 264
        total V clocks per frame

        IMPORTANT: the V sync chain is clocked by HSYNC. This means
        that for the first 48 H clocks of the blanking period, the
        V counter is one behind. This is important to take into account
        for sprite and missile positioning.

        VBLANK is a flip-flop clocked by 16V:
          * The D input is connected to !(128V & 64V & 32V)
          * It is clocked to 1 when V=496
          * It is clocked to 0 when V=272
          * This gives 224 total non-blanked pixels

        VSYNC is set to !256V:
          * It is set to 1 when V=248
          * It is cleared to 0 when V=256


    Sprites and missiles:

        During the HBLANK period, sprites and missiles are processed.
        Sprites are rendered into the line buffer, which was cleared
        during the visible portion of the previous scanline.

        It takes 8 H clocks to set up a sprite, and 16 to render it
        to the line buffer. The setup clocks are overlapped with the
        rendering clocks. In theory this would result in enough time
        to render 128/16 = 8 sprites. However, the setup does not
        begin until after HBLANK, so there is only enough time to
        render the first 7 1/2 entries.

        Interleaved with the setup for sprites is setup for the
        shell and missile rendering. Shells and missiles are rendered
        realtime during the visible portion of the frame, and are
        effectively color-ORed directly into the final RGB output.
        During the HBLANK setup period, each shell/missile entry is
        compared against the current V position; if an exact match
        is found, the H position is loaded into a dedicated 8-bit
        counter. The counter clocks each pixel during the active video
        period; when it reaches $FC it enables the output until it
        hits zero, at which point it shuts itself off. Because there
        is only one counter for shells and one for missiles, only one
        shell and one missile can be specified per scanline. The last
        matching entry found will always win.

        The difference between shell and missile is that shells
        populate the first 7 entries and are rendered as white,
        whereas missiles populate the final entry and are rendered
        as yellow.

        Here is the detailed sequence of events for sprite and
        missile/shell rendering during the first 24 H clocks of
        HBLANK:

            H=080: HPOSI=objram[40], /VPL latches V+objram[40]
            H=081: HPOSI=objram[40]
            H=082: HPOSI=objram[41], /OBJ DATA L latches picture number, H/V flip
            H=083: HPOSI=objram[41]
            H=084: HPOSI=objram[42], /COL L latches low 3 bits as color
            H=085: HPOSI=objram[42]
            H=086: HPOSI=objram[43]
            H=087: HPOSI=objram[43], /CNTR LD latches X position
            <sprite 0 begins rendering>
            H=088: HPOSI=objram[40], /VPL latches V+objram[40]
            H=089: HPOSI=objram[40]
            H=08A: HPOSI=objram[61]
            H=08B: HPOSI=objram[61], MSLD is latched if Y position matches shell
            H=08C: HPOSI=objram[42]
            H=08D: HPOSI=objram[42]
            H=08E: HPOSI=objram[63]
            H=08F: HPOSI=objram[63], /SLD fires to latch down shell counter value
            H=090: HPOSI=objram[44], /VPL latches V+objram[44]
            H=091: HPOSI=objram[44]
            H=092: HPOSI=objram[45], /OBJ DATA L latches picture number, H/V flip
            H=093: HPOSI=objram[45]
            H=094: HPOSI=objram[46], /COL L latches low 3 bits as color
            H=095: HPOSI=objram[46]
            H=096: HPOSI=objram[47]
            H=097: HPOSI=objram[47], /CNTR LD latches X position
            <sprite 1 begins rendering>

        From this, you can see the object RAM layout looks like:

            objram[40] = vertical position of sprite 0
            objram[41] = picture number and H/V flip of sprite 0
            objram[42] = color of sprite 0
            objram[43] = horizontal position of sprite 0

            objram[61] = vertical position of shell 0
            objram[63] = horizontal count until shell 0 starts rendering

        A vertical match for a sprite is true if ((V + vpos) & 0xf0) == 0xf0.
        A vertical match for a shell/missile is if ((V + vpos) & 0xff) == 0xff.

        Overall, the process for sprites and missiles during HBLANK looks
        like this:

            H=080: begin setup sprite 0
            H=082: begin HBLANK
            H=088: begin render sprite 0; begin setup shell 0
            H=090: begin setup sprite 1
            H=098: begin render sprite 1; begin setup shell 1
            H=0A0: begin setup sprite 2
            H=0A8: begin render sprite 2; begin setup shell 2
            H=0B0: VSYNC increments V counter; subsequent sprites match V+1
            H=0B0: begin setup sprite 3
            H=0B8: begin render sprite 3; begin setup shell 3
            H=0C0: begin setup sprite 4
            H=0C8: begin render sprite 4; begin setup shell 4
            H=0D0: begin setup sprite 5
            H=0D8: begin render sprite 5; begin setup shell 5
            H=0E0: begin setup sprite 6
            H=0E8: begin render sprite 6; begin setup shell 6
            H=0F0: begin setup sprite 7
            H=0F8: begin render sprite 7; begin setup missile
            H=0FA: end HBLANK
            H=100: finish render sprite 7 (only 1/2 way through)



       /VPL: H=xxxxxx000 -> latches sum of V+HPOSI for vertical positioning
     /COL L: H=xxxxxx100 -> latches HPOSI into color register (low 3 bits)
        /LD: H=xxxxxx111 -> shift register load from ROM
  /CNTR CLR: H=0xxxx0111 -> resets line buffer counter to 0
/OBJ DATA L: H=1xxxx0010 -> latches HPOSI into picture number latch
   /CNTR LD: H=1xxxx0111 -> latches HPOSI into line buffer counter (sprite X position)
       /SLD: H=1xxxx1111 -> latches down counter until shell (except when /MLD)
       /MLD: H=1x1111111 -> latches down counter until missile



VRAM addresses:
         addr  video
  VRA7 =  A9   SUM7
  VRA8 =  A8   SUM6
  VRA6 =  A7   SUM5
  VRA5 =  A6   SUM4
  VRA4 =  A5   SUM3
  VRA3 =  A4   128HB
  VRA0 =  A3    64HB
  VRA1 =  A2    32HB
  VRA2 =  A1    16HB
  VRA9 =  A0     8HB

OBJRAM addresses:
         addr 256H=0 256H=1
   RA4 =  A0    4H     2H
   RA7 =  A1    8HB    4H
   RA1 =  A2   16HB   16HB
   RA0 =  A3   32HB   32HB
   RA5 =  A4   64HB   64HB
   RA6 =  A5  128HB  (2H & 8HB)
   RA3 =  A6  256H   256H
   RA2 =  A7    0      0

H=80: 00,00,01,01,02,02,03,03 00,00,21,21,02,02,23,23
H=90: 04,04,05,05,06,06,07,07 04,04,25,25,06,06,27,27
H=A0: 08,08,09,09,0A,0A,0B,0B 08,08,29,29,0A,0A,2B,2B
H=B0: 0C,0C,0D,0D,0E,0E,0F,0F 0C,0C,2D,2D,0E,0E,2F,2F



***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "galaxian.h"




/*************************************
 *
 *  Constants
 *
 *************************************/

#define STAR_RNG_PERIOD     ((1 << 17) - 1)
#define RGB_MAXIMUM         224


/*************************************
 *
 *  Palette setup
 *
 *************************************/

void galaxian_state::galaxian_palette(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();
	static const int rgb_resistances[3] = { 1000, 470, 220 };

	/*
	    Sprite/tilemap colors are mapped through a color PROM as follows:

	      bit 7 -- 220 ohm resistor  -- BLUE
	            -- 470 ohm resistor  -- BLUE
	            -- 220 ohm resistor  -- GREEN
	            -- 470 ohm resistor  -- GREEN
	            -- 1  kohm resistor  -- GREEN
	            -- 220 ohm resistor  -- RED
	            -- 470 ohm resistor  -- RED
	      bit 0 -- 1  kohm resistor  -- RED

	    Note that not all boards have this configuration. Namco PCBs may
	    have 330 ohm resistors instead of 220, but the default setup has
	    also been used by Namco.

	    In parallel with these resistors are a pair of 150 ohm and 100 ohm
	    resistors on each R,G,B component that are connected to the star
	    generator.

	    And in parallel with the whole mess are a set of 100 ohm resistors
	    on each R,G,B component that are enabled when a shell/missile is
	    enabled.

	    When computing weights, we use RGB_MAXIMUM as the maximum to give
	    headroom for stars and shells/missiles. This is not fully accurate,
	    but if we included all possible sources in parallel, the brightness
	    of the main game would be very low to allow for all the oversaturation
	    of the stars and shells/missiles.
	*/
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, RGB_MAXIMUM, -1.0,
			3, &rgb_resistances[0], rweights, 470, 0,
			3, &rgb_resistances[0], gweights, 470, 0,
			2, &rgb_resistances[1], bweights, 470, 0);

	// decode the palette first
	int const len = memregion("proms")->bytes();
	for (int i = 0; i < len; i++)
	{
		uint8_t bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	/*
	    The maximum sprite/tilemap resistance is ~130 Ohms with all RGB
	    outputs enabled (1/(1/1000 + 1/470 + 1/220)). Since we normalized
	    to RGB_MAXIMUM, this maps RGB_MAXIMUM -> 130 Ohms.

	    The stars are at 150 Ohms for the LSB, and 100 Ohms for the MSB.
	    This means the 3 potential values are:

	        150 Ohms -> RGB_MAXIMUM * 130 / 150
	        100 Ohms -> RGB_MAXIMUM * 130 / 100
	         60 Ohms -> RGB_MAXIMUM * 130 / 60

	    Since we can't saturate that high, we instead approximate this
	    by compressing the values proportionally into the 194->255 range.
	*/
	int const minval = RGB_MAXIMUM * 130 / 150;
	int const midval = RGB_MAXIMUM * 130 / 100;
	int const maxval = RGB_MAXIMUM * 130 / 60;

	// compute the values for each of 4 possible star values
	uint8_t const starmap[4]{
			0,
			minval,
			minval + (255 - minval) * (midval - minval) / (maxval - minval),
			255 };

	// generate the colors for the stars
	for (int i = 0; i < 64; i++)
	{
		uint8_t bit0, bit1;

		// bit 5 = red @ 150 Ohm, bit 4 = red @ 100 Ohm
		bit0 = BIT(i, 5);
		bit1 = BIT(i, 4);
		int const r = starmap[(bit1 << 1) | bit0];

		// bit 3 = green @ 150 Ohm, bit 2 = green @ 100 Ohm
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 2);
		int const g = starmap[(bit1 << 1) | bit0];

		// bit 1 = blue @ 150 Ohm, bit 0 = blue @ 100 Ohm
		bit0 = BIT(i, 1);
		bit1 = BIT(i, 0);
		int const b = starmap[(bit1 << 1) | bit0];

		// set the RGB color
		m_star_color[i] = rgb_t(r, g, b);
	}

	// default bullet colors are white for the first 7, and yellow for the last one
	for (int i = 0; i < 7; i++)
		m_bullet_color[i] = rgb_t(0xff, 0xff, 0xff);
	m_bullet_color[7] = rgb_t(0xff,0xff,0x00);
}

void moonwar_state::moonwar_palette(palette_device &palette)
{
	galaxian_palette(palette);

	// wire mod to connect the bullet blue output to the 220 ohm resistor
	m_bullet_color[7] = rgb_t(0xef, 0xef, 0x97);
}

void galaxian_state::eagle_palette(palette_device &palette)
{
	galaxian_palette(palette);

	// uses a wiring harness that swaps RGB -> GBR
	for (unsigned i = 0; palette.entries() > i; ++i)
	{
		rgb_t const c = palette.pen(i);
		palette.set_pen_color(i, c.g(), c.b(), c.r());
	}
}

void sbhoei_state::sbhoei_palette(palette_device &palette)
{
	galaxian_palette(palette);

	// uses a wiring harness that swaps RGB -> RBG
	for (unsigned i = 0; palette.entries() > i; ++i)
	{
		rgb_t const c = palette.pen(i);
		palette.set_pen_color(i, c.r(), c.b(), c.g());
	}
}

/*************************************
 *
 *  Common video init
 *
 *************************************/

void galaxian_state::video_start()
{
	/* create a tilemap for the background */
	if (!m_sfx_adjust)
	{
		/* normal galaxian hardware is row-based and individually scrolling columns */
		m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxian_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, m_x_scale*8,8, 32,32);
		m_bg_tilemap->set_scroll_cols(32);
	}
	else
	{
		/* sfx hardware is column-based and individually scrolling rows */
		m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxian_state::bg_get_tile_info)), TILEMAP_SCAN_COLS, m_x_scale*8,8, 32,32);
		m_bg_tilemap->set_scroll_rows(32);
	}
	m_bg_tilemap->set_transparent_pen(0);

	/* initialize globals */
	m_flipscreen_x = 0;
	m_flipscreen_y = 0;
	m_background_enable = 0;
	m_background_blue = 0;
	m_background_red = 0;
	m_background_green = 0;
	std::fill(std::begin(m_gfxbank), std::end(m_gfxbank), 0);

	/* initialize stars */
	stars_init();

	/* register for save states */
	state_save_register();
}


void galaxian_state::state_save_register()
{
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
	save_item(NAME(m_background_enable));
	save_item(NAME(m_background_red));
	save_item(NAME(m_background_green));
	save_item(NAME(m_background_blue));

	save_item(NAME(m_sprites_base));
	save_item(NAME(m_bullets_base));
	save_item(NAME(m_gfxbank));

	save_item(NAME(m_stars_enabled));
	save_item(NAME(m_star_rng_origin));
	save_item(NAME(m_star_rng_origin_frame));
	save_item(NAME(m_stars_blink_state));
}



/*************************************
 *
 *  Common video update
 *
 *************************************/

uint32_t galaxian_state::screen_update_galaxian(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* draw the background layer (including stars) */
	m_draw_background_ptr(bitmap, cliprect);

	/* draw the tilemap characters over top */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* render the sprites next. Some custom pcbs (eg. zigzag, fantastc) have more than one sprite generator (ideally, this should be rendered in parallel) */
	for (int i = 0; i < m_numspritegens; i++)
		sprites_draw(screen, bitmap, cliprect, &m_spriteram[m_sprites_base + i * 0x20]);

	/* if we have bullets to draw, render them following */
	if (!m_draw_bullet_ptr.isnull())
		bullets_draw(screen, bitmap, cliprect, &m_spriteram[m_bullets_base]);

	return 0;
}



/*************************************
 *
 *  Background tilemap
 *
 *************************************/

TILE_GET_INFO_MEMBER(galaxian_state::bg_get_tile_info)
{
	uint8_t *videoram = m_videoram;
	uint8_t x = tile_index & 0x1f;
	uint8_t y = tile_index >> 5;

	uint16_t code = videoram[tile_index];
	uint8_t attrib = m_spriteram[x*2+1];
	uint8_t color = attrib & 7;

	m_extend_tile_info_ptr(&code, &color, attrib, x, y);

	tileinfo.set(0, code, color, 0);
}


void galaxian_state::galaxian_videoram_w(offs_t offset, uint8_t data)
{
	// update any video up to the current scanline
	m_screen->update_partial(m_screen->vpos());

	// store the data and mark the corresponding tile dirty
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void galaxian_state::galaxian_objram_w(offs_t offset, uint8_t data)
{
	// update any video up to the current scanline
	m_screen->update_partial(m_screen->vpos());

	// store the data
	m_spriteram[offset] = data;

	// the first $40 bytes affect the tilemap
	if (offset < 0x40)
	{
		// even entries control the scroll position
		if ((offset & 0x01) == 0)
		{
			// Frogger: top and bottom 4 bits swapped entering the adder
			if (m_frogger_adjust)
				data = (data >> 4) | (data << 4);
			if (!m_sfx_adjust)
				m_bg_tilemap->set_scrolly(offset >> 1, data);
			else
				m_bg_tilemap->set_scrollx(offset >> 1, m_x_scale*data);
		}

		// odd entries control the color base for the row
		else
		{
			for (offset >>= 1; offset < 0x0400; offset += 32)
				m_bg_tilemap->mark_tile_dirty(offset);
		}
	}
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void galaxian_state::sprites_clip(screen_device &screen, rectangle &cliprect)
{
	// 16 of the 256 pixels of the sprites are hard-clipped at the line buffer.
	// According to the schematics, it should be the first 16 pixels.
	// See sprites_draw for an explanation of the +1.
	rectangle clip = screen.visible_area();
	if (m_flipscreen_x)
		clip.max_x = (256 - (16 + 1)) * m_x_scale - 1;
	else
		clip.min_x = ((16 + 1) * m_x_scale);

	cliprect &= clip;
}

void galaxian_state::sprites_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *spritebase)
{
	rectangle clip = cliprect;
	sprites_clip(screen, clip);

	// The line buffer is only written if it contains a '0' currently;
	// it is cleared during the visible area, and populated during HBLANK
	// To simulate this, we render backwards so that lower numbered sprites
	// have priority over higher numbered sprites.
	for (int sprnum = 7; sprnum >= 0; sprnum--)
	{
		const uint8_t *base = &spritebase[sprnum * 4];

		// Frogger: top and bottom 4 bits swapped entering the adder
		uint8_t base0 = m_frogger_adjust ? ((base[0] >> 4) | (base[0] << 4)) : base[0];

		// the first three sprites match against y-1 (seems other way around for sfx/monsterz)
		uint8_t sy = 240 - (base0 - (m_sfx_adjust ? (sprnum >= 3) : (sprnum < 3)));

		uint16_t code = base[1] & 0x3f;
		uint8_t flipx = base[1] & 0x40;
		uint8_t flipy = base[1] & 0x80;
		uint8_t color = base[2] & 7;

		// the existence of +1 (sprite vs tile layer) is supported by a LOT of games
		const int hoffset = 1;
		uint8_t sx = base[3] + hoffset;

		// extend the sprite information
		m_extend_sprite_info_ptr(base, &sx, &sy, &flipx, &flipy, &code, &color);

		// apply flipscreen in X direction
		if (m_flipscreen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		// apply flipscreen in Y direction
		if (m_flipscreen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		// draw
		m_gfxdecode->gfx(1)->transpen(bitmap,clip,
				code, color,
				flipx, flipy,
				m_h0_start + m_x_scale * sx, sy, 0);
	}
}



/*************************************
 *
 *  Bullets rendering
 *
 *************************************/

void galaxian_state::bullets_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *base)
{
	// iterate over scanlines
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint8_t shell = 0xff, missile = 0xff;
		uint8_t effy;

		// the first 3 entries match Y-1
		effy = m_flipscreen_y ? ((y - 1) ^ 255) : (y - 1);
		for (int which = 0; which < 3; which++)
			if (uint8_t(base[which*4+1] + effy) == 0xff)
				shell = which;

		// remaining entries match Y
		effy = m_flipscreen_y ? (y ^ 255) : y;
		for (int which = 3; which < 8; which++)
			if (uint8_t(base[which*4+1] + effy) == 0xff)
			{
				if (which != 7)
					shell = which;
				else
					missile = which;
			}

		// draw the shell
		if (shell != 0xff)
			m_draw_bullet_ptr(bitmap, cliprect, shell, 255 - base[shell*4+3], y);
		if (missile != 0xff)
			m_draw_bullet_ptr(bitmap, cliprect, missile, 255 - base[missile*4+3], y);
	}
}



/*************************************
 *
 *  Screen orientation
 *
 *************************************/

void galaxian_state::galaxian_flip_screen_x_w(uint8_t data)
{
	if (m_flipscreen_x != (data & 0x01))
	{
		m_screen->update_partial(m_screen->vpos());

		/* when the direction changes, we count a different number of clocks */
		/* per frame, so we need to reset the origin of the stars to the current */
		/* frame before we flip */
		stars_update_origin();

		m_flipscreen_x = data & 0x01;
		m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}

void galaxian_state::galaxian_flip_screen_y_w(uint8_t data)
{
	if (m_flipscreen_y != (data & 0x01))
	{
		m_screen->update_partial(m_screen->vpos());

		m_flipscreen_y = data & 0x01;
		m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}

void galaxian_state::galaxian_flip_screen_xy_w(uint8_t data)
{
	galaxian_flip_screen_x_w(data);
	galaxian_flip_screen_y_w(data);
}



/*************************************
 *
 *  Background controls
 *
 *************************************/

void galaxian_state::galaxian_stars_enable_w(uint8_t data)
{
	if ((m_stars_enabled ^ data) & 0x01)
		m_screen->update_partial(m_screen->vpos());

	if (!m_stars_enabled && (data & 0x01))
	{
		/* on the rising edge of this, the CLR on the shift registers is released */
		/* this resets the "origin" of this frame to 0 minus the number of clocks */
		/* we have counted so far */
		m_star_rng_origin = STAR_RNG_PERIOD - (m_screen->vpos() * 512 + m_screen->hpos());
		m_star_rng_origin_frame = m_screen->frame_number();
	}
	m_stars_enabled = data & 0x01;
}


void galaxian_state::scramble_background_enable_w(uint8_t data)
{
	if ((m_background_enable ^ data) & 0x01)
		m_screen->update_partial(m_screen->vpos());

	m_background_enable = data & 0x01;
}


void galaxian_state::scramble_background_red_w(uint8_t data)
{
	if ((m_background_red ^ data) & 0x01)
		m_screen->update_partial(m_screen->vpos());

	m_background_red = data & 0x01;
}


void galaxian_state::scramble_background_green_w(uint8_t data)
{
	if ((m_background_green ^ data) & 0x01)
		m_screen->update_partial(m_screen->vpos());

	m_background_green = data & 0x01;
}


void galaxian_state::scramble_background_blue_w(uint8_t data)
{
	if ((m_background_blue ^ data) & 0x01)
		m_screen->update_partial(m_screen->vpos());

	m_background_blue = data & 0x01;
}



/*************************************
 *
 *  Graphics banking
 *
 *************************************/

void galaxian_state::galaxian_gfxbank_w(offs_t offset, uint8_t data)
{
	if (m_gfxbank[offset] != data)
	{
		m_screen->update_partial(m_screen->vpos());
		m_gfxbank[offset] = data;
		m_bg_tilemap->mark_all_dirty();
	}
}



/*************************************
 *
 *  Star initialization
 *
 *************************************/

void galaxian_state::stars_init()
{
	/* reset the blink and enabled states */
	m_stars_enabled = false;
	m_stars_blink_state = 0;

	/* precalculate the RNG */
	m_stars = std::make_unique<uint8_t[]>(STAR_RNG_PERIOD);
	uint32_t shiftreg = 0;
	for (int i = 0; i < STAR_RNG_PERIOD; i++)
	{
		/* stars are enabled if the upper 8 bits are 1 and the low bit is 0 */
		int enabled = ((shiftreg & 0x1fe01) == 0x1fe00);

		/* color comes from the 6 bits below the top 8 bits */
		int color = (~shiftreg & 0x1f8) >> 3;

		/* store the color value in the low 6 bits and the enable in the upper bit */
		m_stars[i] = color | (enabled << 7);

		/* the LFSR is fed based on the XOR of bit 12 and the inverse of bit 0 */
		shiftreg = (shiftreg >> 1) | ((((shiftreg >> 12) ^ ~shiftreg) & 1) << 16);
	}
}



/*************************************
 *
 *  Adjust the origin of stars
 *
 *************************************/

void galaxian_state::stars_update_origin()
{
	int curframe = m_screen->frame_number();

	/* only update on a different frame */
	if (curframe != m_star_rng_origin_frame)
	{
		/* The RNG period is 2^17-1; each frame, the shift register is clocked */
		/* 512*256 = 2^17 times. This means that we clock one extra time each */
		/* frame. However, if we are NOT flipped, there is a pair of D flip-flops */
		/* at 6B which delay the count so that we count 512*256-2 = 2^17-2 times. */
		/* In this case, we only one time less than the period each frame. Both */
		/* of these off-by-one countings produce the horizontal star scrolling. */
		int per_frame_delta = m_flipscreen_x ? 1 : -1;
		int total_delta = per_frame_delta * (curframe - m_star_rng_origin_frame);

		/* we can't just use % here because mod of a negative number is undefined */
		while (total_delta < 0)
			total_delta += STAR_RNG_PERIOD;

		/* now that everything is positive, do the mod */
		m_star_rng_origin = (m_star_rng_origin + total_delta) % STAR_RNG_PERIOD;
		m_star_rng_origin_frame = curframe;
	}
}



/*************************************
 *
 *  Star blinking
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(galaxian_state::scramble_stars_blink_timer)
{
	m_stars_blink_state++;
}



/*************************************
 *
 *  Draw a row of stars
 *
 *************************************/

void galaxian_state::stars_draw_row(bitmap_rgb32 &bitmap, int maxx, int y, uint32_t star_offs, uint8_t starmask)
{
	/* ensure our star offset is valid */
	star_offs %= STAR_RNG_PERIOD;

	/* iterate over the specified number of 6MHz pixels */
	for (int x = 0; x < maxx; x++)
	{
		/* stars are suppressed unless V1 ^ H8 == 1 */
		int enable_star = (y ^ (x >> 3)) & 1;
		uint8_t star;

		/*
		    The RNG clock is the master clock (18MHz) ANDed with the pixel clock (6MHz).
		    The divide-by-3 circuit that produces the pixel clock generates a square wave
		    with a 2/3 duty cycle, so the result of the AND generates a clock like this:
		                _   _   _   _   _   _   _   _
		      MASTER: _| |_| |_| |_| |_| |_| |_| |_| |
		                _______     _______     ______
		      PIXEL:  _|       |___|       |___|
		                _   _       _   _       _   _
		      RNG:    _| |_| |_____| |_| |_____| |_| |

		    Thus for each pixel, there are 3 master clocks and 2 RNG clocks, and the RNG
		    is clocked asymmetrically. To simulate this, we expand the horizontal screen
		    size by 3 and handle the first RNG clock with one pixel and the second RNG
		    clock with two pixels.
		*/

		/* first RNG clock: one pixel */
		star = m_stars[star_offs++];
		if (star_offs >= STAR_RNG_PERIOD)
			star_offs = 0;
		if (enable_star && (star & 0x80) != 0 && (star & starmask) != 0)
			bitmap.pix(y, m_x_scale*x + 0) = m_star_color[star & 0x3f];

		/* second RNG clock: two pixels */
		star = m_stars[star_offs++];
		if (star_offs >= STAR_RNG_PERIOD)
			star_offs = 0;
		if (enable_star && (star & 0x80) != 0 && (star & starmask) != 0)
		{
			bitmap.pix(y, m_x_scale*x + 1) = m_star_color[star & 0x3f];
			bitmap.pix(y, m_x_scale*x + 2) = m_star_color[star & 0x3f];
		}
	}
}



/*************************************
 *
 *  Background rendering
 *
 *************************************/

void galaxian_state::null_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* erase the background to black */
	bitmap.fill(rgb_t::black(), cliprect);
}


void galaxian_state::galaxian_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx)
{
	/* update the star origin to the current frame */
	stars_update_origin();

	/* render stars if enabled */
	if (m_stars_enabled)
	{
		/* iterate over scanlines */
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			uint32_t star_offs = m_star_rng_origin + y * 512;
			stars_draw_row(bitmap, maxx, y, star_offs, 0xff);
		}
	}
}


void galaxian_state::galaxian_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* erase the background to black first */
	bitmap.fill(rgb_t::black(), cliprect);

	galaxian_draw_stars(bitmap, cliprect, 256);
}


void galaxian_state::background_draw_colorsplit(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t color, int split, int split_flipped)
{
	/* horizontal bgcolor split */
	if (m_flipscreen_x)
	{
		rectangle draw = cliprect;
		draw.max_x = std::min(draw.max_x, split_flipped * m_x_scale - 1);
		if (draw.min_x <= draw.max_x)
			bitmap.fill(rgb_t::black(), draw);

		draw = cliprect;
		draw.min_x = std::max(draw.min_x, split_flipped * m_x_scale);
		if (draw.min_x <= draw.max_x)
			bitmap.fill(color, draw);
	}
	else
	{
		rectangle draw = cliprect;
		draw.max_x = std::min(draw.max_x, split * m_x_scale - 1);
		if (draw.min_x <= draw.max_x)
			bitmap.fill(color, draw);

		draw = cliprect;
		draw.min_x = std::max(draw.min_x, split * m_x_scale);
		if (draw.min_x <= draw.max_x)
			bitmap.fill(rgb_t::black(), draw);
	}
}


void galaxian_state::scramble_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx)
{
	/* update the star origin to the current frame */
	stars_update_origin();

	/* render stars if enabled */
	if (m_stars_enabled)
	{
		int blink_state = m_stars_blink_state & 3;

		/* iterate over scanlines */
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			/* blink state 2 suppressed stars when 2V == 0 */
			if (blink_state != 2 || (y & 2) != 0)
			{
				/* blink states 0 and 1 suppress stars when certain bits of the color == 0 */
				static const uint8_t colormask_table[4] = { 0x20, 0x08, 0xff, 0xff };
				stars_draw_row(bitmap, maxx, y, y * 512, colormask_table[blink_state]);
			}
		}
	}
}


void galaxian_state::scramble_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* blue background - 390 ohm resistor */
	bitmap.fill(m_background_enable ? rgb_t(0,0,0x56) : rgb_t::black(), cliprect);

	scramble_draw_stars(bitmap, cliprect, 256);
}


void galaxian_state::anteater_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* blue background, horizontal split as seen on flyer and real cabinet */
	background_draw_colorsplit(bitmap, cliprect, m_background_enable ? rgb_t(0,0,0x56) : rgb_t::black(), 56, 256-56);

	scramble_draw_stars(bitmap, cliprect, 256);
}


void galaxian_state::jumpbug_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* blue background - 390 ohm resistor */
	bitmap.fill(m_background_enable ? rgb_t(0,0,0x56) : rgb_t::black(), cliprect);

	/* render stars same as galaxian but nothing in the status area */

	/* update the star origin to the current frame */
	stars_update_origin();

	/* render stars if enabled */
	if (m_stars_enabled)
	{
		/* iterate over scanlines */
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			uint32_t star_offs = m_star_rng_origin + y * 512;
			stars_draw_row(bitmap, 232, y, star_offs, 0xff); // verified on a real PCB
		}
	}
}


void galaxian_state::turtles_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/*
	    The background color generator is connected this way:

	        RED   - 390 ohm resistor
	        GREEN - 470 ohm resistor
	        BLUE  - 390 ohm resistor
	*/
	bitmap.fill(rgb_t(m_background_red * 0x55, m_background_green * 0x47, m_background_blue * 0x55), cliprect);
}


void nihon_sfx_state::sfx_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// current schematics are unreadable, assuming like Turtles
	bitmap.fill(rgb_t(m_background_red * 0x55, m_background_green * 0x47, m_background_blue * 0x55), cliprect);
	scramble_draw_stars(bitmap, cliprect, 256);
}


void galaxian_state::frogger_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* according to schematics it is at 128+8; but it has been verified different on real machine.
	Video proof: http://www.youtube.com/watch?v=ssr69mQf224 */
	background_draw_colorsplit(bitmap, cliprect, rgb_t(0,0,0x47), 128, 128);
}


#ifdef UNUSED_FUNCTION
int galaxian_state::flip_and_clip(rectangle &draw, int xstart, int xend, const rectangle &cliprect)
{
	draw = cliprect;
	if (!m_flipscreen_x)
	{
		draw.min_x = xstart * m_x_scale;
		draw.max_x = xend * m_x_scale + (m_x_scale - 1);
	}
	else
	{
		draw.min_x = (xend ^ 255) * m_x_scale;
		draw.max_x = (xstart ^ 255) * m_x_scale + (m_x_scale - 1);
	}
	draw &= cliprect;
	return (draw.min_x <= draw.max_x);
}

void galaxian_state::amidar_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint8_t *prom = memregion("user1")->base();
	rectangle draw;

	for (int x = 0; x < 32; x++)
		if (flip_and_clip(&draw, x * 8, x * 8 + 7, cliprect))
		{
			/*
			    The background PROM is connected the following way:

			       bit 0 = 0 enables the blue gun if BCB is asserted
			       bit 1 = 0 enables the red gun if BCR is asserted and
			                 the green gun if BCG is asserted
			       bits 2-7 are unconnected

			    The background color generator is connected this way:

			        RED   - 270 ohm resistor
			        GREEN - 560 ohm resistor
			        BLUE  - 470 ohm resistor
			*/
			uint8_t red = ((~prom[x] & 0x02) && m_background_red) ? 0x7c : 0x00;
			uint8_t green = ((~prom[x] & 0x02) && m_background_green) ? 0x3c : 0x00;
			uint8_t blue = ((~prom[x] & 0x01) && m_background_blue) ? 0x47 : 0x00;
			bitmap.fill(rgb_t(red, green, blue, draw));
		}
}
#endif



/*************************************
 *
 *  Bullet rendering
 *
 *************************************/

inline void galaxian_state::galaxian_draw_pixel(bitmap_rgb32 &bitmap, const rectangle &cliprect, int y, int x, rgb_t color)
{
	if (y >= cliprect.min_y && y <= cliprect.max_y)
	{
		x *= m_x_scale;
		x += m_h0_start;
		if (x >= cliprect.min_x && x <= cliprect.max_x)
			bitmap.pix(y, x) = color;

		x++;
		if (x >= cliprect.min_x && x <= cliprect.max_x)
			bitmap.pix(y, x) = color;

		x++;
		if (x >= cliprect.min_x && x <= cliprect.max_x)
			bitmap.pix(y, x) = color;
	}
}


void galaxian_state::galaxian_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	/*
	    Both "shells" and "missiles" begin displaying when the horizontal counter
	    reaches $FC, and they stop displaying when it reaches $00, resulting in
	    4-pixel-long shots. The first 7 entries are called "shells" and render as
	    white; the final entry is called a "missile" and renders as yellow.
	*/
	x -= 4;
	galaxian_draw_pixel(bitmap, cliprect, y, x++, m_bullet_color[offs]);
	galaxian_draw_pixel(bitmap, cliprect, y, x++, m_bullet_color[offs]);
	galaxian_draw_pixel(bitmap, cliprect, y, x++, m_bullet_color[offs]);
	galaxian_draw_pixel(bitmap, cliprect, y, x++, m_bullet_color[offs]);
}


void galaxian_state::mshuttle_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	/* verified by schematics:
	    * both "W" and "Y" bullets are 4 pixels long
	    * "W" bullets are enabled when H6 == 0, and are always purple
	    * "Y" bullets are enabled when H6 == 1, and vary in color based on H4,H3,H2
	*/
	static const rgb_t colors[8] =
	{
		rgb_t(0xff,0xff,0xff),
		rgb_t(0xff,0xff,0x00),
		rgb_t(0x00,0xff,0xff),
		rgb_t(0x00,0xff,0x00),
		rgb_t(0xff,0x00,0xff),
		rgb_t(0xff,0x00,0x00),
		rgb_t(0x00,0x00,0xff),
		rgb_t(0x00,0x00,0x00)
	};
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : rgb_t(0xff,0x00,0xff));
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : rgb_t(0xff,0x00,0xff));
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : rgb_t(0xff,0x00,0xff));
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : rgb_t(0xff,0x00,0xff));
}


void galaxian_state::scramble_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	/*
	    Scramble only has "shells", which begin displaying when the counter
	    reaches $FA, and stop displaying two pixel clock layers, as verified
	    on real hardware.

	    All shells are rendered as yellow.
	*/
	x -= 4;
	galaxian_draw_pixel(bitmap, cliprect, y, --x, rgb_t(0xff, 0xff, 0x00));
	galaxian_draw_pixel(bitmap, cliprect, y, --x, rgb_t(0xff, 0xff, 0x00));
}


void galaxian_state::theend_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	/* Same as galaxian except blue/green are swapped */
	x -= 4;
	galaxian_draw_pixel(bitmap, cliprect, y, x++, rgb_t(m_bullet_color[offs].r(), m_bullet_color[offs].b(), m_bullet_color[offs].g()));
	galaxian_draw_pixel(bitmap, cliprect, y, x++, rgb_t(m_bullet_color[offs].r(), m_bullet_color[offs].b(), m_bullet_color[offs].g()));
	galaxian_draw_pixel(bitmap, cliprect, y, x++, rgb_t(m_bullet_color[offs].r(), m_bullet_color[offs].b(), m_bullet_color[offs].g()));
	galaxian_draw_pixel(bitmap, cliprect, y, x++, rgb_t(m_bullet_color[offs].r(), m_bullet_color[offs].b(), m_bullet_color[offs].g()));
}


void nihon_sfx_state::sfx_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	scramble_draw_bullet(bitmap, cliprect, offs, x + 16, y);
}



/*************************************
 *
 *  Video extensions
 *
 *************************************/

/*** generic ***/
void galaxian_state::empty_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
}

void galaxian_state::empty_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
}

void galaxian_state::upper_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	/* tiles are in the upper half of a larger ROM */
	*code += 0x100;
}

void galaxian_state::upper_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	/* sprites are in the upper half of a larger ROM */
	*code += 0x40;
}


/*** Jetsoft BMX Stunts ***/
void bmxstunts_state::bmxstunts_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	if (BIT(base[2], 4))
		*code += 0x40;
}


/*** Frogger ***/
void galaxian_state::frogger_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*color = ((*color >> 1) & 0x03) | ((*color << 2) & 0x04);
}

void galaxian_state::frogger_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*color = ((*color >> 1) & 0x03) | ((*color << 2) & 0x04);
}


/*** Bagman on Moon Cresta hardware, Ghostmuncher Galaxian ***/
void bagmanmc_state::bagmanmc_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*code |= m_gfxbank[0] << 9;
	*color |= m_gfxbank[0] << 3;
}

void bagmanmc_state::bagmanmc_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*code |= (m_gfxbank[0] << 7) | 0x40;
	*color |= m_gfxbank[0] << 3;
}


/*** Pisces ***/
void pisces_state::pisces_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*code |= m_gfxbank[0] << 8;
}

void pisces_state::pisces_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*code |= m_gfxbank[0] << 6;
}


/*** Mighty Monkey ***/
void galaxian_state::mimonkey_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*code |= m_gfxbank[0] << 8 | m_gfxbank[2] << 9;
}

void galaxian_state::mimonkey_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*code |= m_gfxbank[0] << 6 | m_gfxbank[2] << 7;
}


/*** Batman Part 2 ***/
void galaxian_state::batman2_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	if (*code & 0x80)
		*code |= m_gfxbank[0] << 8;
}


/*** Moon Cresta ***/
void galaxian_state::mooncrst_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	if (m_gfxbank[2] && (*code & 0xc0) == 0x80)
		*code = (*code & 0x3f) | (m_gfxbank[0] << 6) | (m_gfxbank[1] << 7) | 0x0100;
}

void galaxian_state::mooncrst_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	if (m_gfxbank[2] && (*code & 0x30) == 0x20)
		*code = (*code & 0x0f) | (m_gfxbank[0] << 4) | (m_gfxbank[1] << 5) | 0x40;
}


/*** Moon Quasar ***/
void galaxian_state::moonqsr_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*code |= (attrib & 0x20) << 3;
}

void galaxian_state::moonqsr_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*code |= (base[2] & 0x20) << 1;
}


/*** Moon Shuttle ***/
void galaxian_state::mshuttle_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*code |= (attrib & 0x30) << 4;
}

void galaxian_state::mshuttle_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*code |= (base[2] & 0x30) << 2;
}


/*** Calipso ***/
void galaxian_state::calipso_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	/* same as the others, but no sprite flipping, but instead the bits are used
	   as extra sprite code bits, giving 256 sprite images */
	/* No flips */
	*code = base[1];
	*flipx = 0;
	*flipy = 0;
}


/*** Jumpbug ***/
void galaxian_state::jumpbug_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	if ((*code & 0xc0) == 0x80 && (m_gfxbank[2] & 0x01))
		*code += 128 + (( m_gfxbank[0] & 0x01) << 6) +
						(( m_gfxbank[1] & 0x01) << 7) +
						((~m_gfxbank[4] & 0x01) << 8);
}

void galaxian_state::jumpbug_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	if ((*code & 0x30) == 0x20 && (m_gfxbank[2] & 0x01) != 0)
	{
		*code += 32 + (( m_gfxbank[0] & 0x01) << 4) +
						(( m_gfxbank[1] & 0x01) << 5) +
						((~m_gfxbank[4] & 0x01) << 6);
	}
}


/*** Namennayo ***/
void namenayo_state::namenayo_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	if ((attrib & 0x01) == 0x00) // main game display
	{
		int attr = m_exattrram[y & 0x1f];
		*code += ((attr & 0x38) >> 3) * 0x100;
		*color = attr & 0x07;
	}
	else // status bar
	{
		// course map
		if ((attrib & 0xfe) == 0x20)
			*code += 0x400;
	}
}

void namenayo_state::namenayo_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	if (base[2] & 0x08)
		*code += 0x40;

	// sprite colours are from the 2nd PROM
	*color += 8;
}

void namenayo_state::namenayo_unk_d800_w(uint8_t data)
{
	// seems to be connected to scroll position?
	// popmessage("namenayo_unk_d800_w %02x", data);
}

void namenayo_state::namenayo_extattr_w(offs_t offset, uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	m_exattrram[offset] = data;
	m_bg_tilemap->mark_all_dirty();
}

void namenayo_state::namenayo_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	rectangle draw;

	if (m_flipscreen_x)
	{
		draw = cliprect;
		draw.min_x = std::max(draw.min_x, 72 * m_x_scale);
	}
	else
	{
		draw = cliprect;
		draw.max_x = std::min(draw.max_x, 184 * m_x_scale - 1);
	}

	// draw an opaque copy of the tilemap in part of the screen to ensure some of the graphics are correct
	// this can't extend past the playfield or you get unwanted fill near the status bar
	// this might not be 100% how the hardware works.
	m_bg_tilemap->draw(*m_screen, bitmap, draw, TILEMAP_DRAW_OPAQUE, 0);
}


/*** Guttang Gottong bootleg ***/
void galaxian_state::guttangt_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	// is this configurable or a hardwired mod?
	*code |= 0x40;
}


/*** Hoei Space Battle ***/
void sbhoei_state::sbhoei_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	if (((m_gfxbank[2] & 1) && (*code & 0xc0) == 0x80) || ((m_gfxbank[3] & 1) && (*code & 0xc0) == 0xc0))
		*code = (*code & 0x3f) | ((m_gfxbank[0] & 1) << 6) | ((m_gfxbank[1] & 1) << 7) | (0x0100 << (m_gfxbank[4] & 1));
}

void sbhoei_state::sbhoei_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	if (((m_gfxbank[2] & 1) && (*code & 0x30) == 0x20) || ((m_gfxbank[3] & 1) && (*code & 0x30) == 0x30))
		*code = (*code & 0x0f) | ((m_gfxbank[0] & 1) << 4) | ((m_gfxbank[1] & 1) << 5) | (0x40 << (m_gfxbank[4] & 1));
}


/*** Four Play ***/
void fourplay_state::fourplay_rombank_w(offs_t offset, uint8_t data)
{
	// gfxbank[4] is used as a cpu bank number, and gfxbank[0] for graphics banking
	m_gfxbank[4] = (m_gfxbank[4] & (2 - offset)) | (data << offset);
	m_gfxbank[0] = (m_gfxbank[4] == 3); // 1 = true, 0 = false

	m_rombank->set_entry(m_gfxbank[4]);
}


/*** Video Eight ***/
void videight_state::videight_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)
{
	*code |= (m_gfxbank[0] << 8);
	*color |= (m_gfxbank[4] << 3);
}

void videight_state::videight_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	*code |= (m_gfxbank[0] << 6);
	*color |= (m_gfxbank[4] << 3);
}

// This handles the main bankswitching for code and one-bank gfx games
void videight_state::videight_rombank_w(offs_t offset, uint8_t data)
{
	static constexpr uint8_t gfxbanks[] = { 0, 10, 2, 8, 1, 9, 4, 11 };
	if (offset == 2)
		galaxian_stars_enable_w(data);
	else
	{
		m_gfxbank[4] = (m_gfxbank[4] & (6 - offset)) | (data << ((offset + 1) >> 1));
		galaxian_gfxbank_w (0, gfxbanks[m_gfxbank[4]]);
		m_rombank->set_entry(m_gfxbank[4]);
	}
}

// This handles those games with multiple gfx banks
void videight_state::videight_gfxbank_w(offs_t offset, uint8_t data)
{
	// Moon Cresta (mooncrgx)
	if ((data < 2) && (m_gfxbank[4] == 3))
	{
		static constexpr uint8_t gfxbanks[] = { 8, 12, 8, 14, 8, 13, 8, 15 };
		if (!offset) m_gfxbank[3] = (m_gfxbank[3] & 6) | data;
		if (offset == 1) m_gfxbank[3] = (m_gfxbank[3] & 5) | (data << 1);
		if (offset == 2) m_gfxbank[3] = (m_gfxbank[3] & 3) | (data << 2);
		galaxian_gfxbank_w(0, gfxbanks[m_gfxbank[3]]);
	}

	// Uniwar S
	if ((data < 2) && (m_gfxbank[4] == 2) && (offset == 2))
		galaxian_gfxbank_w(0, data + 2);

	// Pisces (piscesb)
	if ((data < 2) && (m_gfxbank[4] == 6) && (offset == 2))
		galaxian_gfxbank_w(0, data + 4);
}
