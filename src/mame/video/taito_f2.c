#include "emu.h"
#include "video/taitoic.h"
#include "includes/taito_f2.h"

/************************************************************
                      SPRITE BANKING

  Four sprite banking methods are used for games with more
  than $2000 sprite tiles, because the sprite ram only has
  13 bits available for tile numbers.

   0 = standard (only a limited selection of sprites are
                  available for display at a given time)
   1 = use sprite extension area lo bytes for hi 6 bits
   2 = use sprite extension area hi bytes
   3 = use sprite extension area lo bytes as hi bytes
            (sprite extension areas mean all sprite
             tiles are always accessible)
************************************************************/

enum
{
	FOOTCHMP = 1
};


/***********************************************************************************/

static void taitof2_core_vh_start (running_machine &machine, int sprite_type, int hide, int flip_hide )
{
	int i;
	taitof2_state *state = machine.driver_data<taitof2_state>();
	state->m_sprite_type = sprite_type;
	state->m_hide_pixels = hide;
	state->m_flip_hide_pixels = flip_hide;

	state->m_spriteram_delayed = auto_alloc_array(machine, UINT16, state->m_spriteram_size / 2);
	state->m_spriteram_buffered = auto_alloc_array(machine, UINT16, state->m_spriteram_size / 2);
	state->m_spritelist = auto_alloc_array(machine, struct f2_tempsprite, 0x400);

	for (i = 0; i < 8; i ++)
	{
		state->m_spritebank_buffered[i] = 0x400 * i;
		state->m_spritebank[i] = state->m_spritebank_buffered[i];
	}

	state->m_sprites_disabled = 1;
	state->m_sprites_active_area = 0;
	state->m_sprites_flipscreen = 0;

	state->m_sprites_master_scrollx = 0;
	state->m_sprites_master_scrolly = 0;

	state->m_spriteblendmode = 0;
	state->m_prepare_sprites = 0;

	state->m_game = 0;	/* means NOT footchmp */

	state->save_item(NAME(state->m_spritebank));
	state->save_item(NAME(state->m_spritebank_buffered));
	state->save_item(NAME(state->m_sprites_disabled));
	state->save_item(NAME(state->m_sprites_active_area));
	state->save_item(NAME(state->m_sprites_flipscreen));
	state->save_item(NAME(state->m_sprites_master_scrollx));
	state->save_item(NAME(state->m_sprites_master_scrolly));
	state->save_item(NAME(state->m_tilepri));
	state->save_item(NAME(state->m_spritepri));
	state->save_item(NAME(state->m_spriteblendmode));
	state->save_item(NAME(state->m_prepare_sprites));
	state->save_pointer(NAME(state->m_spriteram_delayed), state->m_spriteram_size / 2);
	state->save_pointer(NAME(state->m_spriteram_buffered), state->m_spriteram_size / 2);
}

/**************************************************************************************/
/*    ( spritetype, hide, hideflip, xoffs, yoffs, flipx, flipy, textflipx, textflipy) */
/**************************************************************************************/

VIDEO_START( taitof2_default )
{
	taitof2_core_vh_start(machine, 0, 0, 0);
}

VIDEO_START( taitof2_megab )   /* Megab, Liquidk */
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_quiz )   /* Quiz Crayons, Quiz Jinsei */
{
	taitof2_core_vh_start(machine, 3, 3, 3);
}

VIDEO_START( taitof2_finalb )
{
	taitof2_core_vh_start(machine, 0, 1, 1);
}

VIDEO_START( taitof2_ssi )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_growl )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_ninjak )
{
	taitof2_core_vh_start(machine, 0, 0, 0);
}

VIDEO_START( taitof2_qzchikyu )
{
	taitof2_core_vh_start(machine, 0, 0, 4);
}

VIDEO_START( taitof2_solfigtr )
{
	taitof2_core_vh_start(machine, 0, 3, -3);
}

VIDEO_START( taitof2_koshien )
{
	taitof2_core_vh_start(machine, 0, 1,  - 1);
}

VIDEO_START( taitof2_gunfront )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_thundfox )
{
	taitof2_core_vh_start(machine, 0, 3, -3);
}

VIDEO_START( taitof2_mjnquest )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	taitof2_core_vh_start(machine, 0, 0, 0);

	tc0100scn_set_bg_tilemask(state->m_tc0100scn, 0x7fff);
}

VIDEO_START( taitof2_footchmp )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	taitof2_core_vh_start(machine, 0, 3, 3);

	state->m_game = FOOTCHMP;
}

VIDEO_START( taitof2_hthero )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	taitof2_core_vh_start(machine, 0, 3, 3);

	state->m_game = FOOTCHMP;
}

VIDEO_START( taitof2_deadconx )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_deadconxj )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_metalb )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_yuyugogo )
{
	taitof2_core_vh_start(machine, 1, 3, 3);
}

VIDEO_START( taitof2_yesnoj )
{
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_dinorex )
{
	taitof2_core_vh_start(machine, 3, 3, 3);
}

VIDEO_START( taitof2_dondokod )	/* dondokod, cameltry */
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	state->m_pivot_xdisp = -16;
	state->m_pivot_ydisp = 0;
	taitof2_core_vh_start(machine, 0, 3, 3);
}

VIDEO_START( taitof2_pulirula )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	state->m_pivot_xdisp = -10;	/* alignment seems correct (see level 2, falling */
	state->m_pivot_ydisp = 16;	/* block of ice after armour man) */
	taitof2_core_vh_start(machine, 2, 3, 3);
}

VIDEO_START( taitof2_driftout )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	state->m_pivot_xdisp = -16;
	state->m_pivot_ydisp = 16;
	taitof2_core_vh_start(machine, 0, 3, 3);
}


/********************************************************
          SPRITE READ AND WRITE HANDLERS

The spritebank buffering is currently not needed.

If we wanted to buffer sprites by an extra frame, it
might be for Footchmp. That seems to be the only game
altering spritebanks of sprites while they're on screen.
********************************************************/

WRITE16_MEMBER(taitof2_state::taitof2_sprite_extension_w)
{
	/* areas above 0x1000 cleared in some games, but not used */

	if (offset < 0x800)
	{
		COMBINE_DATA(&m_sprite_extension[offset]);
	}
}


WRITE16_MEMBER(taitof2_state::taitof2_spritebank_w)
{
	int i = 0;
	int j = 0;

	if (offset < 2)
		return;   /* irrelevant zero writes */

	if (offset < 4)   /* special bank pairs */
	{
		j = (offset & 1) << 1;   /* either set pair 0&1 or 2&3 */
		i = data << 11;
		m_spritebank_buffered[j] = i;
		m_spritebank_buffered[j + 1] = (i + 0x400);

//logerror("bank %d, set to: %04x\n", j, i);
//logerror("bank %d, paired so: %04x\n", j + 1, i + 0x400);

	}
	else   /* last 4 are individual banks */
	{
		i = data << 10;
		m_spritebank_buffered[offset] = i;

//logerror("bank %d, new value: %04x\n", offset, i);
	}

}

WRITE16_MEMBER(taitof2_state::koshien_spritebank_w)
{
	m_spritebank_buffered[0] = 0x0000;   /* never changes */
	m_spritebank_buffered[1] = 0x0400;

	m_spritebank_buffered[2] =  ((data & 0x00f) + 1) * 0x800;
	m_spritebank_buffered[4] = (((data & 0x0f0) >> 4) + 1) * 0x800;
	m_spritebank_buffered[6] = (((data & 0xf00) >> 8) + 1) * 0x800;
	m_spritebank_buffered[3] = m_spritebank_buffered[2] + 0x400;
	m_spritebank_buffered[5] = m_spritebank_buffered[4] + 0x400;
	m_spritebank_buffered[7] = m_spritebank_buffered[6] + 0x400;
}

static void taito_f2_tc360_spritemixdraw( running_machine &machine, bitmap_ind16 &dest_bmp, const rectangle &clip, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();
	int pal_base = gfx->color_base + gfx->color_granularity * (color % gfx->total_colors);
	const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
	bitmap_ind8 &priority_bitmap = gfx->machine().priority_bitmap;
	int sprite_screen_height = (scaley * gfx->height + 0x8000) >> 16;
	int sprite_screen_width = (scalex * gfx->width + 0x8000) >> 16;

	if (!scalex || !scaley)
		return;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width << 16) / sprite_screen_width;
		int dy = (gfx->height << 16) / sprite_screen_height;

		int ex = sx + sprite_screen_width;
		int ey = sy + sprite_screen_height;

		int x_index_base;
		int y_index;

		if (flipx)
		{
			x_index_base = (sprite_screen_width - 1) * dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if (flipy)
		{
			y_index = (sprite_screen_height - 1) * dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if (sx < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x - sx;
			sx += pixels;
			x_index_base += pixels * dx;
		}
		if (sy < clip.min_y)
		{ /* clip top */
			int pixels = clip.min_y - sy;
			sy += pixels;
			y_index += pixels * dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if (ex > clip.max_x + 1)
		{ /* clip right */
			int pixels = ex-clip.max_x - 1;
			ex -= pixels;
		}
		if (ey > clip.max_y + 1)
		{ /* clip bottom */
			int pixels = ey-clip.max_y - 1;
			ey -= pixels;
		}

		if (ex > sx)
		{
			/* skip if inner loop doesn't draw anything */
			int y;

			for (y = sy; y < ey; y++)
			{
				const UINT8 *source = source_base + (y_index >> 16) * gfx->line_modulo;
				UINT16 *dest = &dest_bmp.pix16(y);
				UINT8 *pri = &priority_bitmap.pix8(y);

				int x, x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					int c = source[x_index >> 16];
					if (c && (pri[x] & 0x80) == 0)
					{
						UINT8 tilemap_priority = 0, sprite_priority = 0;

						// Get tilemap priority (0 - 0xf) for this destination pixel
						if (pri[x] & 0x10) tilemap_priority = state->m_tilepri[4];
						else if (pri[x] & 0x8) tilemap_priority = state->m_tilepri[3];
						else if (pri[x] & 0x4) tilemap_priority = state->m_tilepri[2];
						else if (pri[x] & 0x2) tilemap_priority = state->m_tilepri[1];
						else if (pri[x] & 0x1) tilemap_priority = state->m_tilepri[0];

						// Get sprite priority (0 - 0xf) for this source pixel
						if ((color & 0xc0) == 0xc0)
							sprite_priority = state->m_spritepri[3];
						else if ((color & 0xc0) == 0x80)
							sprite_priority = state->m_spritepri[2];
						else if ((color & 0xc0) == 0x40)
							sprite_priority = state->m_spritepri[1];
						else if ((color & 0xc0) == 0x00)
							sprite_priority = state->m_spritepri[0];

						// Blend mode 1 - Sprite under tilemap, use sprite palette with tilemap data
						if ((state->m_spriteblendmode & 0xc0) == 0xc0 && sprite_priority == (tilemap_priority - 1))
						{
							dest[x] = ((pal_base + c) & 0xfff0) | (dest[x] & 0xf);
						}
						// Blend mode 1 - Sprite over tilemap, use sprite data with tilemap palette
						else if ((state->m_spriteblendmode & 0xc0) == 0xc0 && sprite_priority == (tilemap_priority + 1))
						{
							if (dest[x] & 0xf)
								dest[x] = (dest[x] & 0xfff0) | ((pal_base + c) & 0xf);
							else
								dest[x] = pal_base + c;
						}
						// Blend mode 2 - Sprite under tilemap, use sprite data with tilemap palette
						else if ((state->m_spriteblendmode & 0xc0) == 0x80 && sprite_priority == (tilemap_priority - 1))
						{
							dest[x] = (dest[x] & 0xffef);
						}
						// Blend mode 2 - Sprite over tilemap, alternate sprite palette, confirmed in Pulirula level 2
						else if ((state->m_spriteblendmode & 0xc0) == 0x80 && sprite_priority == (tilemap_priority + 1))
						{
							dest[x] = ((pal_base + c) & 0xffef); // Pulirula level 2, Liquid Kids attract mode
						}
						// No blending
						else
						{
							if (sprite_priority > tilemap_priority) // Ninja Kids confirms tilemap takes priority in equal value case
								dest[x] = pal_base + c;
						}
						pri[x] |= 0x80;
					}

					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int *primasks, int uses_tc360_mixer )
{
	/*
        Sprite format:
        0000: ---xxxxxxxxxxxxx tile code (0x0000 - 0x1fff)
        0002: xxxxxxxx-------- sprite y-zoom level
              --------xxxxxxxx sprite x-zoom level

              0x00 - non scaled = 100%
              0x80 - scaled to 50%
              0xc0 - scaled to 25%
              0xe0 - scaled to 12.5%
              0xff - scaled to zero pixels size (off)

        [this zoom scale may not be 100% correct, see Gunfront flame screen]

        0004: ----xxxxxxxxxxxx x-coordinate (-0x800 to 0x07ff)
              ---x------------ latch extra scroll
              --x------------- latch master scroll
              -x-------------- don't use extra scroll compensation
              x--------------- absolute screen coordinates (ignore all sprite scrolls)
              xxxx------------ the typical use of the above is therefore
                               1010 = set master scroll
                               0101 = set extra scroll
        0006: ----xxxxxxxxxxxx y-coordinate (-0x800 to 0x07ff)
              x--------------- marks special control commands (used in conjunction with 00a)
                               If the special command flag is set:
              ---------------x related to sprite ram bank
              ---x------------ unknown (deadconx, maybe others)
              --x------------- unknown, some games (growl, gunfront) set it to 1 when
                               screen is flipped
        0008: --------xxxxxxxx color (0x00 - 0xff)
              -------x-------- flipx
              ------x--------- flipy
              -----x---------- if set, use latched color, else use & latch specified one
              ----x----------- if set, next sprite entry is part of sequence
              ---x------------ if clear, use latched y coordinate, else use current y
              --x------------- if set, y += 16
              -x-------------- if clear, use latched x coordinate, else use current x
              x--------------- if set, x += 16
        000a: only valid when the special command bit in 006 is set
              ---------------x related to sprite ram bank. I think this is the one causing
                               the bank switch, implementing it this way all games seem
                               to properly bank switch except for footchmp which uses the
                               bit in byte 006 instead.
              ------------x--- unknown; some games toggle it before updating sprite ram.
              ------xx-------- unknown (finalb)
              -----x---------- unknown (mjnquest)
              ---x------------ disable the following sprites until another marker with
                               this bit clear is found
              --x------------- flip screen

        000b - 000f : unused

    DG comment: the sprite zoom code grafted on from Jarek's TaitoB
    may mean I have pointlessly duplicated x,y latches in the zoom &
    non zoom parts.

    */
	taitof2_state *state = machine.driver_data<taitof2_state>();
	int i, x, y, off, extoffs;
	int code, color, spritedata, spritecont, flipx, flipy;
	int xcurrent, ycurrent, big_sprite = 0;
	int y_no = 0, x_no = 0, xlatch = 0, ylatch = 0, last_continuation_tile = 0;   /* for zooms */
	UINT32 zoomword, zoomx, zoomy, zx = 0, zy = 0, zoomxlatch = 0, zoomylatch = 0;   /* for zooms */
	int scroll1x, scroll1y;
	int scrollx = 0, scrolly = 0;
	int curx, cury;
	int f2_x_offset;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
       while processing sprite ram and then draw them all at the end */
	struct f2_tempsprite *sprite_ptr = state->m_spritelist;

	/* must remember enable status from last frame because driftout fails to
       reactivate them from a certain point onwards. */
	int disabled = state->m_sprites_disabled;

	/* must remember master scroll from previous frame because driftout
       sometimes doesn't set it. */
	int master_scrollx = state->m_sprites_master_scrollx;
	int master_scrolly = state->m_sprites_master_scrolly;

	/* must also remember the sprite bank from previous frame. */
	int area = state->m_sprites_active_area;

	scroll1x = 0;
	scroll1y = 0;
	x = y = 0;
	xcurrent = ycurrent = 0;
	color = 0;

	f2_x_offset = state->m_hide_pixels;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
	if (state->m_sprites_flipscreen)
		f2_x_offset = -state->m_flip_hide_pixels;		// was -f2_x_offset

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (area == 0x8000 && state->m_spriteram_buffered[(0x8000 + 6) / 2] == 0 && state->m_spriteram_buffered[(0x8000 + 10) / 2] == 0)
		area = 0;

	for (off = 0; off < 0x4000; off += 16)
	{
		/* sprites_active_area may change during processing */
		int offs = off + area;

		if (state->m_spriteram_buffered[(offs + 6) / 2] & 0x8000)
		{
			disabled = state->m_spriteram_buffered[(offs + 10) / 2] & 0x1000;
			state->m_sprites_flipscreen = state->m_spriteram_buffered[(offs + 10) / 2] & 0x2000;

			/* Get rid of 0-3 unwanted pixels on edge of screen. */
			f2_x_offset = state->m_hide_pixels;
			if (state->m_sprites_flipscreen)
				f2_x_offset = -state->m_flip_hide_pixels;		// was -f2_x_offset

			if (state->m_game == FOOTCHMP)
				area = 0x8000 * (state->m_spriteram_buffered[(offs + 6) / 2] & 0x0001);
			else
				area = 0x8000 * (state->m_spriteram_buffered[(offs + 10) / 2] & 0x0001);
			continue;
		}

//popmessage("%04x",area);

		/* check for extra scroll offset */
		if ((state->m_spriteram_buffered[(offs + 4) / 2] & 0xf000) == 0xa000)
		{
			master_scrollx = state->m_spriteram_buffered[(offs + 4) / 2] & 0xfff;
			if (master_scrollx >= 0x800)
				master_scrollx -= 0x1000;   /* signed value */

			master_scrolly = state->m_spriteram_buffered[(offs + 6) / 2] & 0xfff;
			if (master_scrolly >= 0x800)
				master_scrolly -= 0x1000;   /* signed value */
		}

		if ((state->m_spriteram_buffered[(offs + 4) / 2] & 0xf000) == 0x5000)
		{
			scroll1x = state->m_spriteram_buffered[(offs + 4) / 2] & 0xfff;
			if (scroll1x >= 0x800)
				scroll1x -= 0x1000;   /* signed value */

			scroll1y = state->m_spriteram_buffered[(offs + 6) / 2] & 0xfff;
			if (scroll1y >= 0x800)
				scroll1y -= 0x1000;   /* signed value */
		}

		if (disabled)
			continue;

		spritedata = state->m_spriteram_buffered[(offs + 8) / 2];

		spritecont = (spritedata & 0xff00) >> 8;

		if ((spritecont & 0x08) != 0)   /* sprite continuation flag set */
		{
			if (big_sprite == 0)   /* are we starting a big sprite ? */
			{
				xlatch = state->m_spriteram_buffered[(offs + 4) / 2] & 0xfff;
				ylatch = state->m_spriteram_buffered[(offs + 6) / 2] & 0xfff;
				x_no = 0;
				y_no = 0;
				zoomword = state->m_spriteram_buffered[(offs + 2) / 2];
				zoomylatch = (zoomword >> 8) & 0xff;
				zoomxlatch = (zoomword >> 0) & 0xff;
				big_sprite = 1;   /* we have started a new big sprite */
			}
		}
		else if (big_sprite)
		{
			last_continuation_tile = 1;   /* don't clear big_sprite until last tile done */
		}


		if ((spritecont & 0x04) == 0)
			color = spritedata & 0xff;


// The bigsprite == 0 check fixes "tied-up" little sprites in Thunderfox
// which (mostly?) have spritecont = 0x20 when they are not continuations
// of anything.
		if (big_sprite == 0 || (spritecont & 0xf0) == 0)
		{
			x = state->m_spriteram_buffered[(offs + 4) / 2];

// Some absolute x values deduced here are 1 too high (scenes when you get
// home run in Koshien, and may also relate to BG layer woods and stuff as you
// journey in MjnQuest). You will see they are 1 pixel too far to the right.
// Where is this extra pixel offset coming from??

			if (x & 0x8000)   /* absolute (koshien) */
			{
				scrollx = - f2_x_offset - 0x60;
				scrolly = 0;
			}
			else if (x & 0x4000)   /* ignore extra scroll */
			{
				scrollx = master_scrollx - f2_x_offset - 0x60;
				scrolly = master_scrolly;
			}
			else   /* all scrolls applied */
			{
				scrollx = scroll1x + master_scrollx - f2_x_offset - 0x60;
				scrolly = scroll1y + master_scrolly;
			}
			x &= 0xfff;
			y = state->m_spriteram_buffered[(offs + 6) / 2] & 0xfff;

			xcurrent = x;
			ycurrent = y;
		}
		else
		{
			if ((spritecont & 0x10) == 0)
				y = ycurrent;
			else if ((spritecont & 0x20) != 0)
			{
				y += 16;
				y_no++;   /* keep track of y tile for zooms */
			}
			if ((spritecont & 0x40) == 0)
				x = xcurrent;
			else if ((spritecont & 0x80) != 0)
			{
				x += 16;
				y_no=0;
				x_no++;   /* keep track of x tile for zooms */
			}
		}

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;
			zx = 0x10;	/* default, no zoom: 16 pixels across */
			zy = 0x10;	/* default, no zoom: 16 pixels vertical */

			if (zoomx || zoomy)
			{
				/* "Zoom" zx&y is pixel size horizontally and vertically
                   of our sprite chunk. So it is difference in x and y
                   coords of our chunk and diagonally adjoining one.

                   Ideally, big_sprite should be zoomed as a whole, and not
                   into chunks. The current implementation practically only has
                   16 zoom levels instead of the 256 the hardware supports,
                   resulting in a shaky and distorted zoom.
                   (example: planet earth zooming in, in gunfront intro)
                */

				x = xlatch + (x_no * (0xff - zoomx) + 15) / 16;
				y = ylatch + (y_no * (0xff - zoomy) + 15) / 16;
				zx = xlatch + ((x_no + 1) * (0xff - zoomx) + 15) / 16 - x;
				zy = ylatch + ((y_no + 1) * (0xff - zoomy) + 15) / 16 - y;
			}
		}
		else
		{
			zoomword = state->m_spriteram_buffered[(offs + 2) / 2];
			zoomy = (zoomword >> 8) & 0xff;
			zoomx = (zoomword >> 0) & 0xff;
			zx = (0x100 - zoomx) / 16;
			zy = (0x100 - zoomy) / 16;
		}

		if (last_continuation_tile)
		{
			big_sprite=0;
			last_continuation_tile=0;
		}

		code = 0;
		extoffs = offs;
		/* spriteram[0x4000-7fff] has no corresponding extension area */
		if (extoffs >= 0x8000) extoffs -= 0x4000;

		if (state->m_sprite_type == 0)
		{
			code = state->m_spriteram_buffered[(offs) / 2] & 0x1fff;
			i = (code & 0x1c00) >> 10;
			code = state->m_spritebank[i] + (code & 0x3ff);
		}

		if (state->m_sprite_type == 1)   /* Yuyugogo */
		{
			code = state->m_spriteram_buffered[(offs) / 2] & 0x3ff;
			i = (state->m_sprite_extension[(extoffs >> 4)] & 0x3f ) << 10;
			code = (i | code);
		}

		if (state->m_sprite_type == 2)   /* Pulirula */
		{
			code = state->m_spriteram_buffered[(offs) / 2] & 0xff;
			i = (state->m_sprite_extension[(extoffs >> 4)] & 0xff00 );
			code = (i | code);
		}

		if (state->m_sprite_type == 3)   /* Dinorex and a few quizzes */
		{
			code = state->m_spriteram_buffered[(offs) / 2] & 0xff;
			i = (state->m_sprite_extension[(extoffs >> 4)] & 0xff ) << 8;
			code = (i | code);
		}

		if (code == 0) continue;

		flipx = spritecont & 0x01;
		flipy = spritecont & 0x02;

		curx = (x + scrollx) & 0xfff;
		if (curx >= 0x800)	curx -= 0x1000;   /* treat it as signed */

		cury = (y + scrolly) & 0xfff;
		if (cury >= 0x800)	cury -= 0x1000;   /* treat it as signed */

		if (state->m_sprites_flipscreen)
		{
			/* -zx/y is there to fix zoomed sprite coords in screenflip.
               drawgfxzoom does not know to draw from flip-side of sprites when
               screen is flipped; so we must correct the coords ourselves. */

			curx = 320 - curx - zx;
			cury = 256 - cury - zy;
			flipx = !flipx;
			flipy = !flipy;
		}

		{
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			if (machine.gfx[0]->color_granularity == 64)	/* Final Blow is 6-bit deep */
				sprite_ptr->color /= 4;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks || uses_tc360_mixer)
			{
				if (primasks)
					sprite_ptr->primask = primasks[(color & 0xc0) >> 6];

				sprite_ptr++;
			}
			else
			{
				drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[0],
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						sprite_ptr->zoomx,sprite_ptr->zoomy,0);
			}
		}
	}


	/* this happens only if primsks != NULL */
	while (sprite_ptr != state->m_spritelist)
	{
		sprite_ptr--;

		if (!uses_tc360_mixer)
			pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[0],
					sprite_ptr->code,
					sprite_ptr->color,
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,
					sprite_ptr->zoomx,sprite_ptr->zoomy,
					machine.priority_bitmap,sprite_ptr->primask,0);
		else
			taito_f2_tc360_spritemixdraw(machine, bitmap,cliprect,machine.gfx[0],
					sprite_ptr->code,
					sprite_ptr->color,
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,
					sprite_ptr->zoomx,sprite_ptr->zoomy);
	}
}


static void update_spritebanks( running_machine &machine )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();
	int i;
#if 1
	for (i = 0; i < 8; i ++)
	{
		state->m_spritebank[i] = state->m_spritebank_buffered[i];
	}
#else
	/* this makes footchmp blobbing worse! */
	for (i = 0; i < 8; i ++)
	{
		state->m_spritebank[i] = state->m_spritebank_eof[i];
		state->m_spritebank_eof[i] = state->m_spritebank_buffered[i];
	}
#endif
}

static void taitof2_handle_sprite_buffering( running_machine &machine )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	if (state->m_prepare_sprites)	/* no buffering */
	{
		memcpy(state->m_spriteram_buffered, state->m_spriteram, state->m_spriteram_size);
		state->m_prepare_sprites = 0;
	}
}

static void taitof2_update_sprites_active_area( running_machine &machine )
{
	taitof2_state *state = machine.driver_data<taitof2_state>();
	int off;

	update_spritebanks(machine);

	/* if the frame was skipped, we'll have to do the buffering now */
	taitof2_handle_sprite_buffering(machine);

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (state->m_sprites_active_area == 0x8000 &&
			state->m_spriteram_buffered[(0x8000 + 6) / 2] == 0 &&
			state->m_spriteram_buffered[(0x8000 + 10) / 2] == 0)
		state->m_sprites_active_area = 0;

	for (off = 0; off < 0x4000; off += 16)
	{
		/* sprites_active_area may change during processing */
		int offs = off + state->m_sprites_active_area;

		if (state->m_spriteram_buffered[(offs + 6) / 2] & 0x8000)
		{
			state->m_sprites_disabled = state->m_spriteram_buffered[(offs + 10) / 2] & 0x1000;
			if (state->m_game == FOOTCHMP)
				state->m_sprites_active_area = 0x8000 * (state->m_spriteram_buffered[(offs + 6) / 2] & 0x0001);
			else
				state->m_sprites_active_area = 0x8000 * (state->m_spriteram_buffered[(offs + 10) / 2] & 0x0001);
			continue;
		}

		/* check for extra scroll offset */
		if ((state->m_spriteram_buffered[(offs + 4) / 2] & 0xf000) == 0xa000)
		{
			state->m_sprites_master_scrollx = state->m_spriteram_buffered[(offs + 4) / 2] & 0xfff;
			if (state->m_sprites_master_scrollx >= 0x800)
				state->m_sprites_master_scrollx -= 0x1000;   /* signed value */

			state->m_sprites_master_scrolly = state->m_spriteram_buffered[(offs + 6) / 2] & 0xfff;
			if (state->m_sprites_master_scrolly >= 0x800)
				state->m_sprites_master_scrolly -= 0x1000;   /* signed value */
		}
	}
}


SCREEN_VBLANK( taitof2_no_buffer )
{
	// rising edge
	if (vblank_on)
	{
		taitof2_state *state = screen.machine().driver_data<taitof2_state>();

		taitof2_update_sprites_active_area(screen.machine());

		state->m_prepare_sprites = 1;
	}
}

SCREEN_VBLANK( taitof2_full_buffer_delayed )
{
	// rising edge
	if (vblank_on)
	{
		taitof2_state *state = screen.machine().driver_data<taitof2_state>();
		UINT16 *spriteram = state->m_spriteram;
		int i;

		taitof2_update_sprites_active_area(screen.machine());

		state->m_prepare_sprites = 0;
		memcpy(state->m_spriteram_buffered, state->m_spriteram_delayed, state->m_spriteram_size);
		for (i = 0; i < state->m_spriteram_size / 2; i++)
			state->m_spriteram_buffered[i] = spriteram[i];
		memcpy(state->m_spriteram_delayed, spriteram, state->m_spriteram_size);
	}
}

SCREEN_VBLANK( taitof2_partial_buffer_delayed )
{
	// rising edge
	if (vblank_on)
	{
		taitof2_state *state = screen.machine().driver_data<taitof2_state>();
		UINT16 *spriteram = state->m_spriteram;
		int i;

		taitof2_update_sprites_active_area(screen.machine());

		state->m_prepare_sprites = 0;
		memcpy(state->m_spriteram_buffered, state->m_spriteram_delayed, state->m_spriteram_size);
		for (i = 0;i < state->m_spriteram_size / 2; i += 4)
			state->m_spriteram_buffered[i] = spriteram[i];
		memcpy(state->m_spriteram_delayed, spriteram, state->m_spriteram_size);
	}
}

SCREEN_VBLANK( taitof2_partial_buffer_delayed_thundfox )
{
	// rising edge
	if (vblank_on)
	{
		taitof2_state *state = screen.machine().driver_data<taitof2_state>();
		UINT16 *spriteram = state->m_spriteram;
		int i;

		taitof2_update_sprites_active_area(screen.machine());

		state->m_prepare_sprites = 0;
		memcpy(state->m_spriteram_buffered, state->m_spriteram_delayed, state->m_spriteram_size);
		for (i = 0; i < state->m_spriteram_size / 2; i += 8)
		{
			state->m_spriteram_buffered[i]     = spriteram[i];
			state->m_spriteram_buffered[i + 1] = spriteram[i + 1];
			state->m_spriteram_buffered[i + 4] = spriteram[i + 4];
		}
		memcpy(state->m_spriteram_delayed, spriteram, state->m_spriteram_size);
	}
}

SCREEN_VBLANK( taitof2_partial_buffer_delayed_qzchikyu )
{
	// rising edge
	if (vblank_on)
	{
		/* spriteram[2] and [3] are 1 frame behind...
           probably thundfox_eof_callback would work fine */

		taitof2_state *state = screen.machine().driver_data<taitof2_state>();
		UINT16 *spriteram = state->m_spriteram;
		int i;

		taitof2_update_sprites_active_area(screen.machine());

		state->m_prepare_sprites = 0;
		memcpy(state->m_spriteram_buffered, state->m_spriteram_delayed, state->m_spriteram_size);
		for (i = 0; i < state->m_spriteram_size / 2; i += 8)
		{
			state->m_spriteram_buffered[i]     = spriteram[i];
			state->m_spriteram_buffered[i + 1] = spriteram[i + 1];
			state->m_spriteram_buffered[i + 4] = spriteram[i + 4];
			state->m_spriteram_buffered[i + 5] = spriteram[i + 5];	// not needed?
			state->m_spriteram_buffered[i + 6] = spriteram[i + 6];	// not needed?
			state->m_spriteram_buffered[i + 7] = spriteram[i + 7];	// not needed?
		}
		memcpy(state->m_spriteram_delayed, spriteram, state->m_spriteram_size);
	}
}


/* SSI */
SCREEN_UPDATE_IND16( taitof2_ssi )
{
	taitof2_handle_sprite_buffering(screen.machine());

	/* SSI only uses sprites, the tilemap registers are not even initialized.
       (they are in Majestic 12, but the tilemaps are not used anyway) */
	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, NULL, 0);
	return 0;
}


SCREEN_UPDATE_IND16( taitof2_yesnoj )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();

	taitof2_handle_sprite_buffering(screen.machine());

	tc0100scn_tilemap_update(state->m_tc0100scn);

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);	/* wrong color? */
	draw_sprites(screen.machine(), bitmap, cliprect, NULL, 0);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, tc0100scn_bottomlayer(state->m_tc0100scn), 0, 0);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, tc0100scn_bottomlayer(state->m_tc0100scn) ^ 1, 0, 0);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, 2, 0, 0);
	return 0;
}


SCREEN_UPDATE_IND16( taitof2 )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();

	taitof2_handle_sprite_buffering(screen.machine());

	tc0100scn_tilemap_update(state->m_tc0100scn);

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);	/* wrong color? */
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, tc0100scn_bottomlayer(state->m_tc0100scn), 0, 0);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, tc0100scn_bottomlayer(state->m_tc0100scn) ^ 1, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, NULL, 0);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, 2, 0, 0);
	return 0;
}


SCREEN_UPDATE_IND16( taitof2_pri )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();
	int layer[3];

	taitof2_handle_sprite_buffering(screen.machine());

	tc0100scn_tilemap_update(state->m_tc0100scn);

	layer[0] = tc0100scn_bottomlayer(state->m_tc0100scn);
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;
	state->m_tilepri[layer[0]] = tc0360pri_r(state->m_tc0360pri, 5) & 0x0f;
	state->m_tilepri[layer[1]] = tc0360pri_r(state->m_tc0360pri, 5) >> 4;
	state->m_tilepri[layer[2]] = tc0360pri_r(state->m_tc0360pri, 4) >> 4;

	state->m_spritepri[0] = tc0360pri_r(state->m_tc0360pri, 6) & 0x0f;
	state->m_spritepri[1] = tc0360pri_r(state->m_tc0360pri, 6) >> 4;
	state->m_spritepri[2] = tc0360pri_r(state->m_tc0360pri, 7) & 0x0f;
	state->m_spritepri[3] = tc0360pri_r(state->m_tc0360pri, 7) >> 4;

	state->m_spriteblendmode = tc0360pri_r(state->m_tc0360pri, 0) & 0xc0;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);	/* wrong color? */

	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, layer[0], 0, 1);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, layer[1], 0, 2);
	tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, layer[2], 0, 4);

	draw_sprites(screen.machine(), bitmap, cliprect, NULL, 1);
	return 0;
}



static void draw_roz_layer( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32 priority)
{
	taitof2_state *state = machine.driver_data<taitof2_state>();

	if (state->m_tc0280grd != NULL)
		tc0280grd_zoom_draw(state->m_tc0280grd, bitmap, cliprect, state->m_pivot_xdisp, state->m_pivot_ydisp, priority);

	if (state->m_tc0430grw != NULL)
		tc0430grw_zoom_draw(state->m_tc0430grw, bitmap, cliprect, state->m_pivot_xdisp, state->m_pivot_ydisp, priority);
}

SCREEN_UPDATE_IND16( taitof2_pri_roz )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();
	int tilepri[3];
	int rozpri;
	int layer[3];
	int drawn;
	int i,j;
	int roz_base_color = (tc0360pri_r(state->m_tc0360pri, 1) & 0x3f) << 2;

	taitof2_handle_sprite_buffering(screen.machine());

	if (state->m_tc0280grd != NULL)
		tc0280grd_tilemap_update(state->m_tc0280grd, roz_base_color);

	if (state->m_tc0430grw != NULL)
		tc0430grw_tilemap_update(state->m_tc0430grw, roz_base_color);

	tc0100scn_tilemap_update(state->m_tc0100scn);

	rozpri = (tc0360pri_r(state->m_tc0360pri, 1) & 0xc0) >> 6;
	rozpri = (tc0360pri_r(state->m_tc0360pri, 8 + rozpri / 2) >> 4 * (rozpri & 1)) & 0x0f;

	layer[0] = tc0100scn_bottomlayer(state->m_tc0100scn);
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	tilepri[layer[0]] = tc0360pri_r(state->m_tc0360pri, 5) & 0x0f;
	tilepri[layer[1]] = tc0360pri_r(state->m_tc0360pri, 5) >> 4;
	tilepri[layer[2]] = tc0360pri_r(state->m_tc0360pri, 4) >> 4;

	state->m_spritepri[0] = tc0360pri_r(state->m_tc0360pri, 6) & 0x0f;
	state->m_spritepri[1] = tc0360pri_r(state->m_tc0360pri, 6) >> 4;
	state->m_spritepri[2] = tc0360pri_r(state->m_tc0360pri, 7) & 0x0f;
	state->m_spritepri[3] = tc0360pri_r(state->m_tc0360pri, 7) >> 4;

	state->m_spriteblendmode = tc0360pri_r(state->m_tc0360pri, 0) & 0xc0;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);	/* wrong color? */

	drawn = 0;
	for (i = 0; i < 16; i++)
	{
		if (rozpri == i)
		{
			draw_roz_layer(screen.machine(), bitmap, cliprect, 1 << drawn);
			state->m_tilepri[drawn] = i;
			drawn++;
		}

		for (j = 0; j < 3; j++)
		{
			if (tilepri[layer[j]] == i)
			{
				tc0100scn_tilemap_draw(state->m_tc0100scn, bitmap, cliprect, layer[j], 0, 1 << drawn);
				state->m_tilepri[drawn] = i;
				drawn++;
			}
		}
	}

	draw_sprites(screen.machine(), bitmap, cliprect, NULL, 1);
	return 0;
}



/* Thunderfox */
SCREEN_UPDATE_IND16( taitof2_thundfox )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();
	int tilepri[2][3];
	int spritepri[4];
	int layer[2][3];
	int drawn[2];

	taitof2_handle_sprite_buffering(screen.machine());

	tc0100scn_tilemap_update(state->m_tc0100scn_1);
	tc0100scn_tilemap_update(state->m_tc0100scn_2);

	layer[0][0] = tc0100scn_bottomlayer(state->m_tc0100scn_1);
	layer[0][1] = layer[0][0] ^ 1;
	layer[0][2] = 2;
	tilepri[0][layer[0][0]] = tc0360pri_r(state->m_tc0360pri, 5) & 0x0f;
	tilepri[0][layer[0][1]] = tc0360pri_r(state->m_tc0360pri, 5) >> 4;
	tilepri[0][layer[0][2]] = tc0360pri_r(state->m_tc0360pri, 4) >> 4;

	layer[1][0] = tc0100scn_bottomlayer(state->m_tc0100scn_2);
	layer[1][1] = layer[1][0] ^ 1;
	layer[1][2] = 2;
	tilepri[1][layer[1][0]] = tc0360pri_r(state->m_tc0360pri, 9) & 0x0f;
	tilepri[1][layer[1][1]] = tc0360pri_r(state->m_tc0360pri, 9) >> 4;
	tilepri[1][layer[1][2]] = tc0360pri_r(state->m_tc0360pri, 8) >> 4;

	spritepri[0] = tc0360pri_r(state->m_tc0360pri, 6) & 0x0f;
	spritepri[1] = tc0360pri_r(state->m_tc0360pri, 6) >> 4;
	spritepri[2] = tc0360pri_r(state->m_tc0360pri, 7) & 0x0f;
	spritepri[3] = tc0360pri_r(state->m_tc0360pri, 7) >> 4;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);	/* wrong color? */

	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 6 layers, so I have to cheat, assuming
    that the two FG layers are always on top of sprites.
    */

	drawn[0] = drawn[1] = 0;
	while (drawn[0] < 2 && drawn[1] < 2)
	{
		int pick;
		device_t *tc0100scn;

		if (tilepri[0][drawn[0]] < tilepri[1][drawn[1]])
		{
			pick = 0;
			tc0100scn = state->m_tc0100scn_1;
		}
		else
		{
			pick = 1;
			tc0100scn = state->m_tc0100scn_2;
		}

		tc0100scn_tilemap_draw(tc0100scn , bitmap, cliprect, layer[pick][drawn[pick]], 0, 1 << (drawn[pick] + 2 * pick));
		drawn[pick]++;
	}
	while (drawn[0] < 2)
	{
		tc0100scn_tilemap_draw(state->m_tc0100scn_1, bitmap, cliprect, layer[0][drawn[0]], 0, 1 << drawn[0]);
		drawn[0]++;
	}
	while (drawn[1] < 2)
	{
		tc0100scn_tilemap_draw(state->m_tc0100scn_2, bitmap, cliprect, layer[1][drawn[1]], 0, 1 << (drawn[1] + 2));
		drawn[1]++;
	}

	{
		int primasks[4] = {0,0,0,0};
		int i;

		for (i = 0;i < 4;i++)
		{
			if (spritepri[i] < tilepri[0][0]) primasks[i] |= 0xaaaa;
			if (spritepri[i] < tilepri[0][1]) primasks[i] |= 0xcccc;
			if (spritepri[i] < tilepri[1][0]) primasks[i] |= 0xf0f0;
			if (spritepri[i] < tilepri[1][1]) primasks[i] |= 0xff00;
		}

		draw_sprites(screen.machine(), bitmap,cliprect,primasks,0);
	}


	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 6 layers, so I have to cheat, assuming
    that the two FG layers are always on top of sprites.
    */

	if (tilepri[0][2] < tilepri[1][2])
	{
		tc0100scn_tilemap_draw(state->m_tc0100scn_1, bitmap, cliprect, layer[0][2], 0, 0);
		tc0100scn_tilemap_draw(state->m_tc0100scn_2, bitmap, cliprect, layer[1][2], 0, 0);
	}
	else
	{
		tc0100scn_tilemap_draw(state->m_tc0100scn_2, bitmap, cliprect, layer[1][2], 0, 0);
		tc0100scn_tilemap_draw(state->m_tc0100scn_1, bitmap, cliprect, layer[0][2], 0, 0);
	}
	return 0;
}



/*********************************************************************

Deadconx and Footchmp use in the PRI chip
-----------------------------------------

+4  xxxx0000   BG0
    0000xxxx   BG3
+6  xxxx0000   BG2
    0000xxxx   BG1

Deadconx = 0x7db9 (bg0-3) 0x8eca (sprites)
So it has bg0 [back] / s / bg1 / s / bg2 / s / bg3 / s

Footchmp = 0x8db9 (bg0-3) 0xe5ac (sprites)
So it has s / bg0 [grass] / bg1 [crowd] / s / bg2 [goal] / s / bg3 [messages] / s [scan dots]

Metalb uses in the PRI chip
---------------------------

+4  xxxx0000   BG1
    0000xxxx   BG0
+6  xxxx0000   BG3
    0000xxxx   BG2

and it changes these (and the sprite pri settings) a lot.

********************************************************************/

SCREEN_UPDATE_IND16( taitof2_metalb )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();
	UINT8 layer[5], invlayer[4];
	UINT16 priority;

	taitof2_handle_sprite_buffering(screen.machine());

	tc0480scp_tilemap_update(state->m_tc0480scp);

	priority = tc0480scp_get_bg_priority(state->m_tc0480scp);

	layer[0] = (priority & 0xf000) >> 12;	/* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;	/* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	invlayer[layer[0]] = 0;
	invlayer[layer[1]] = 1;
	invlayer[layer[2]] = 2;
	invlayer[layer[3]] = 3;

	state->m_tilepri[invlayer[0]] = tc0360pri_r(state->m_tc0360pri, 4) & 0x0f;	/* bg0 */
	state->m_tilepri[invlayer[1]] = tc0360pri_r(state->m_tc0360pri, 4) >> 4;	/* bg1 */
	state->m_tilepri[invlayer[2]] = tc0360pri_r(state->m_tc0360pri, 5) & 0x0f;	/* bg2 */
	state->m_tilepri[invlayer[3]] = tc0360pri_r(state->m_tc0360pri, 5) >> 4;	/* bg3 */
	state->m_tilepri[4] = tc0360pri_r(state->m_tc0360pri, 9) & 0x0f;			/* fg (text layer) */

	state->m_spritepri[0] = tc0360pri_r(state->m_tc0360pri, 6) & 0x0f;
	state->m_spritepri[1] = tc0360pri_r(state->m_tc0360pri, 6) >> 4;
	state->m_spritepri[2] = tc0360pri_r(state->m_tc0360pri, 7) & 0x0f;
	state->m_spritepri[3] = tc0360pri_r(state->m_tc0360pri, 7) >> 4;

	state->m_spriteblendmode = tc0360pri_r(state->m_tc0360pri, 0) & 0xc0;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[0], 0 ,1);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[1], 0, 2);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[2], 0, 4);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[3], 0, 8);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[4], 0, 16);

	draw_sprites(screen.machine(), bitmap, cliprect, NULL, 1);
	return 0;
}


/* Deadconx, Footchmp */
SCREEN_UPDATE_IND16( taitof2_deadconx )
{
	taitof2_state *state = screen.machine().driver_data<taitof2_state>();
	UINT8 layer[5];
	UINT8 tilepri[5];
	UINT8 spritepri[4];
	UINT16 priority;

	taitof2_handle_sprite_buffering(screen.machine());

	tc0480scp_tilemap_update(state->m_tc0480scp);

	priority = tc0480scp_get_bg_priority(state->m_tc0480scp);

	layer[0] = (priority & 0xf000) >> 12;	/* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;	/* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	tilepri[0] = tc0360pri_r(state->m_tc0360pri, 4) >> 4;      /* bg0 */
	tilepri[1] = tc0360pri_r(state->m_tc0360pri, 5) & 0x0f;    /* bg1 */
	tilepri[2] = tc0360pri_r(state->m_tc0360pri, 5) >> 4;      /* bg2 */
	tilepri[3] = tc0360pri_r(state->m_tc0360pri, 4) & 0x0f;    /* bg3 */

/* we actually assume text layer is on top of everything anyway, but FWIW... */
	tilepri[layer[4]] = tc0360pri_r(state->m_tc0360pri, 7) >> 4;    /* fg (text layer) */

	spritepri[0] = tc0360pri_r(state->m_tc0360pri, 6) & 0x0f;
	spritepri[1] = tc0360pri_r(state->m_tc0360pri, 6) >> 4;
	spritepri[2] = tc0360pri_r(state->m_tc0360pri, 7) & 0x0f;
	spritepri[3] = tc0360pri_r(state->m_tc0360pri, 7) >> 4;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[0], 0 ,1);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[1], 0, 2);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[2], 0, 4);
	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[3], 0, 8);

	{
		int primasks[4] = {0,0,0,0};
		int i;

		for (i = 0;i < 4;i++)
		{
			if (spritepri[i] < tilepri[(layer[0])]) primasks[i] |= 0xaaaa;
			if (spritepri[i] < tilepri[(layer[1])]) primasks[i] |= 0xcccc;
			if (spritepri[i] < tilepri[(layer[2])]) primasks[i] |= 0xf0f0;
			if (spritepri[i] < tilepri[(layer[3])]) primasks[i] |= 0xff00;
		}

		draw_sprites(screen.machine(), bitmap, cliprect, primasks, 0);
	}

	/*
    TODO: This isn't the correct way to handle the priority. At the moment of
    writing, pdrawgfx() doesn't support 5 layers, so I have to cheat, assuming
    that the FG layer is always on top of sprites.
    */

	tc0480scp_tilemap_draw(state->m_tc0480scp, bitmap, cliprect, layer[4], 0, 0);
	return 0;
}
