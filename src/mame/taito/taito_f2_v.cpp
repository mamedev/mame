// license:BSD-3-Clause
// copyright-holders:David Graves, Bryan McPhail, Brad Oliver, Andrew Prime, Brian Troha, Nicola Salmoria
#include "emu.h"
#include "taito_f2.h"
#include "screen.h"

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

TC0100SCN_CB_MEMBER(mjnquest_state::tmap_cb)
{
	*code = (*code & 0x7fff) | (m_gfxbank << 15);
}

void mjnquest_state::gfxbank_w(u8 data)
{
	if (BIT(data ^ m_gfxbank, 0))
	{
		m_gfxbank = BIT(data, 0);
		m_tc0100scn[0]->tilemap_set_dirty();
	}
}

void taitof2_state::core_vh_start(int sprite_type, int hide, int flip_hide)
{
	m_sprite_type = sprite_type;
	m_hide_pixels = hide;
	m_flip_hide_pixels = flip_hide;

	m_spriteram_delayed = make_unique_clear<u16[]>(m_spriteram.bytes() / 2);
	m_spriteram_buffered = make_unique_clear<u16[]>(m_spriteram.bytes() / 2);
	m_spritelist = std::make_unique<f2_tempsprite[]>(0x400);

	for (int i = 0; i < 8; i ++)
	{
		m_spritebank_buffered[i] = 0x400 * i;
		m_spritebank[i] = m_spritebank_buffered[i];
	}

	m_sprites_disabled = true;
	m_sprites_active_area = 0;
	m_sprites_flipscreen = false;

	m_sprites_master_scrollx = 0;
	m_sprites_master_scrolly = 0;

	m_spriteblendmode = 0;
	m_prepare_sprites = false;

	m_game = 0;  /* means NOT footchmp */

	save_item(NAME(m_spritebank));
	save_item(NAME(m_spritebank_buffered));
	save_item(NAME(m_sprites_disabled));
	save_item(NAME(m_sprites_active_area));
	save_item(NAME(m_sprites_flipscreen));
	save_item(NAME(m_sprites_master_scrollx));
	save_item(NAME(m_sprites_master_scrolly));
	save_item(NAME(m_tilepri));
	save_item(NAME(m_spritepri));
	save_item(NAME(m_spriteblendmode));
	save_item(NAME(m_prepare_sprites));
	save_pointer(NAME(m_spriteram_delayed), m_spriteram.bytes() / 2);
	save_pointer(NAME(m_spriteram_buffered), m_spriteram.bytes() / 2);
}

/**************************************************************************************/
/*    (spritetype, hide, hideflip, xoffs, yoffs, flipx, flipy, textflipx, textflipy) */
/**************************************************************************************/

void taitof2_state::video_start()
{
	core_vh_start(0, 0, 0);
}

VIDEO_START_MEMBER(taitof2_state,megab)/* Megab, Liquidk */
{
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,quiz)/* Quiz Crayons, Quiz Jinsei */
{
	core_vh_start(3, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,finalb)
{
	core_vh_start(0, 1, 1);
}

VIDEO_START_MEMBER(taitof2_state,ssi)
{
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,growl)
{
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,ninjak)
{
	core_vh_start(0, 0, 0);
}

VIDEO_START_MEMBER(taitof2_state,qzchikyu)
{
	core_vh_start(0, 0, 4);
}

VIDEO_START_MEMBER(taitof2_state,solfigtr)
{
	core_vh_start(0, 3, -3);
}

VIDEO_START_MEMBER(taitof2_state,koshien)
{
	core_vh_start(0, 1,  - 1);
}

VIDEO_START_MEMBER(taitof2_state,gunfront)
{
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,thundfox)
{
	core_vh_start(0, 3, -3);
}

void mjnquest_state::video_start()
{
	core_vh_start(0, 0, 0);
	save_item(NAME(m_gfxbank));
}

VIDEO_START_MEMBER(footchmp_state,footchmp)
{
	core_vh_start(0, 3, 3);

	m_game = FOOTCHMP;
}

VIDEO_START_MEMBER(footchmp_state,deadconx)/* deadconx, metalb */
{
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,yuyugogo)
{
	core_vh_start(1, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,yesnoj)
{
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(taitof2_state,dinorex)
{
	core_vh_start(3, 3, 3);
}

VIDEO_START_MEMBER(dondokod_state,dondokod)/* dondokod, cameltry */
{
	m_pivot_xdisp = -16;
	m_pivot_ydisp = 0;
	core_vh_start(0, 3, 3);
}

VIDEO_START_MEMBER(dondokod_state,pulirula)
{
	m_pivot_xdisp = -10;    /* alignment seems correct (see level 2, falling */
	m_pivot_ydisp = 16; /* block of ice after armour man) */
	core_vh_start(2, 3, 3);
}

VIDEO_START_MEMBER(dondokod_state,driftout)
{
	m_pivot_xdisp = -16;
	m_pivot_ydisp = 16;
	core_vh_start(0, 3, 3);
}

/********************************************************
          SPRITE READ AND WRITE HANDLERS

The spritebank buffering is currently not needed.

If we wanted to buffer sprites by an extra frame, it
might be for Footchmp. That seems to be the only game
altering spritebanks of sprites while they're on screen.
********************************************************/

void taitof2_state::sprite_extension_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* areas above 0x1000 cleared in some games, but not used */

	if (offset < 0x800)
	{
		COMBINE_DATA(&m_sprite_extension[offset]);
	}
}


void taitof2_state::spritebank_w(offs_t offset, u16 data)
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

void taitof2_state::koshien_spritebank_w(u16 data)
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

void taitof2_state::taito_f2_tc360_spritemixdraw(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		u32 code, u32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, u64 primask, bool use_mixer)
{
	const u16 pal_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const u8 *source_base = gfx->get_data(code % gfx->elements());
	bitmap_ind8 &priority_bitmap = screen.priority();
	const int sprite_screen_height = (scaley * gfx->height() + 0x8000) >> 16;
	const int sprite_screen_width = (scalex * gfx->width() + 0x8000) >> 16;

	if (!scalex || !scaley)
		return;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width() << 16) / sprite_screen_width;
		int dy = (gfx->height() << 16) / sprite_screen_height;

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
			int pixels = ex - clip.max_x - 1;
			ex -= pixels;
		}
		if (ey > clip.max_y + 1)
		{ /* clip bottom */
			int pixels = ey - clip.max_y - 1;
			ey -= pixels;
		}

		if (ex > sx)
		{
			/* skip if inner loop doesn't draw anything */
			if (use_mixer)
			{
				for (int y = sy; y < ey; y++)
				{
					u8 const *const source = source_base + (y_index >> 16) * gfx->rowbytes();
					u16 *const dest = &dest_bmp.pix(y);
					u8 *const pri = &priority_bitmap.pix(y);

					int x_index = x_index_base;
					for (int x = sx; x < ex; x++)
					{
						const u8 c = source[x_index >> 16];
						if (c && BIT(~pri[x], 7))
						{
							u8 tilemap_priority = 0;

							// Get tilemap priority (0 - 0xf) for this destination pixel
							if (BIT(pri[x], 4)) tilemap_priority = m_tilepri[4];
							else if (BIT(pri[x], 3)) tilemap_priority = m_tilepri[3];
							else if (BIT(pri[x], 2)) tilemap_priority = m_tilepri[2];
							else if (BIT(pri[x], 1)) tilemap_priority = m_tilepri[1];
							else if (BIT(pri[x], 0)) tilemap_priority = m_tilepri[0];

							// Get sprite priority (0 - 0xf) for this source pixel
							const u8 sprite_priority = m_spritepri[primask & 0x03];

							// Blend mode 1 - Sprite under tilemap, use sprite palette with tilemap data
							if ((m_spriteblendmode & 0xc0) == 0xc0 && sprite_priority == (tilemap_priority - 1))
							{
								dest[x] = ((pal_base + c) & 0xfff0) | (dest[x] & 0xf);
							}
							// Blend mode 1 - Sprite over tilemap, use sprite data with tilemap palette
							else if ((m_spriteblendmode & 0xc0) == 0xc0 && sprite_priority == (tilemap_priority + 1))
							{
								if (dest[x] & 0xf)
									dest[x] = (dest[x] & 0xfff0) | ((pal_base + c) & 0xf);
								else
									dest[x] = pal_base + c;
							}
							// Blend mode 2 - Sprite under tilemap, use sprite data with tilemap palette
							else if ((m_spriteblendmode & 0xc0) == 0x80 && sprite_priority == (tilemap_priority - 1))
							{
								dest[x] = (dest[x] & 0xffef);
							}
							// Blend mode 2 - Sprite over tilemap, alternate sprite palette, confirmed in Pulirula level 2
							else if ((m_spriteblendmode & 0xc0) == 0x80 && sprite_priority == (tilemap_priority + 1))
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
			else // without blending
			{
				for (int y = sy; y < ey; y++)
				{
					u8 const *const source = source_base + (y_index >> 16) * gfx->rowbytes();
					u16 *const dest = &dest_bmp.pix(y);
					u8 *const pri = &priority_bitmap.pix(y);

					int x_index = x_index_base;
					for (int x = sx; x < ex; x++)
					{
						const u8 c = source[x_index >> 16];
						if (c && BIT(~pri[x], 7))
						{
							if (((1 << (pri[x] & 0x3f)) & primask) == 0)
								dest[x] = pal_base + c;
							pri[x] |= 0x80;
						}
						x_index += dx;
					}
					y_index += dy;
				}
			}
		}
	}
}

void taitof2_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u64 *primasks, int uses_tc360_mixer)
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

	    000c - 000f : unused

	DG comment: the sprite zoom code grafted on from Jarek's TaitoB
	may mean I have pointlessly duplicated x,y latches in the zoom &
	non zoom parts.

	*/
	bool big_sprite = false, last_continuation_tile = false;
	int y_no = 0, x_no = 0, xlatch = 0, ylatch = 0;   /* for zooms */
	u32 zoomword, zoomx, zoomy, zx = 0, zy = 0, zoomxlatch = 0, zoomylatch = 0;   /* for zooms */
	int scrollx = 0, scrolly = 0;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	f2_tempsprite *sprite_ptr = m_spritelist.get();

	/* must remember enable status from last frame because driftout fails to
	   reactivate them from a certain point onwards. */
	bool disabled = m_sprites_disabled;

	/* must remember master scroll from previous frame because driftout
	   sometimes doesn't set it. */
	int master_scrollx = m_sprites_master_scrollx;
	int master_scrolly = m_sprites_master_scrolly;

	/* must also remember the sprite bank from previous frame. */
	int area = m_sprites_active_area;

	int scroll1x = 0;
	int scroll1y = 0;
	int x = 0, y = 0;
	int xcurrent = 0, ycurrent = 0;
	u32 color = 0;

	int f2_x_offset = m_hide_pixels;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
	if (m_sprites_flipscreen)
		f2_x_offset = -m_flip_hide_pixels;       // was -f2_x_offset

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (area == 0x8000 && m_spriteram_buffered[(0x8000 + 6) / 2] == 0 && m_spriteram_buffered[(0x8000 + 10) / 2] == 0)
		area = 0;

	for (int off = 0; off < 0x4000; off += 16)
	{
		/* sprites_active_area may change during processing */
		const int offs = off + area;

		if (BIT(m_spriteram_buffered[(offs + 6) / 2], 15))
		{
			disabled = BIT(m_spriteram_buffered[(offs + 10) / 2], 12);
			m_sprites_flipscreen = BIT(m_spriteram_buffered[(offs + 10) / 2], 13);

			/* Get rid of 0-3 unwanted pixels on edge of screen. */
			f2_x_offset = m_hide_pixels;
			if (m_sprites_flipscreen)
				f2_x_offset = -m_flip_hide_pixels;       // was -f2_x_offset

			if (m_game == FOOTCHMP)
				area = 0x8000 * (m_spriteram_buffered[(offs + 6) / 2] & 0x0001);
			else
				area = 0x8000 * (m_spriteram_buffered[(offs + 10) / 2] & 0x0001);
			continue;
		}

//popmessage("%04x",area);

		/* check for extra scroll offset */
		if ((m_spriteram_buffered[(offs + 4) / 2] & 0xf000) == 0xa000)
		{
			master_scrollx = util::sext(m_spriteram_buffered[(offs + 4) / 2], 12);
			master_scrolly = util::sext(m_spriteram_buffered[(offs + 6) / 2], 12);
		}

		if ((m_spriteram_buffered[(offs + 4) / 2] & 0xf000) == 0x5000)
		{
			scroll1x = util::sext(m_spriteram_buffered[(offs + 4) / 2], 12);
			scroll1y = util::sext(m_spriteram_buffered[(offs + 6) / 2], 12);
		}

		if (disabled)
			continue;

		const u16 spritedata = m_spriteram_buffered[(offs + 8) / 2];

		const u16 spritecont = (spritedata & 0xff00) >> 8;

		if (BIT(spritecont, 3))   /* sprite continuation flag set */
		{
			if (!big_sprite)   /* are we starting a big sprite ? */
			{
				xlatch = m_spriteram_buffered[(offs + 4) / 2] & 0xfff;
				ylatch = m_spriteram_buffered[(offs + 6) / 2] & 0xfff;
				x_no = 0;
				y_no = 0;
				zoomword = m_spriteram_buffered[(offs + 2) / 2];
				zoomylatch = (zoomword >> 8) & 0xff;
				zoomxlatch = (zoomword >> 0) & 0xff;
				big_sprite = true;   /* we have started a new big sprite */
			}
		}
		else if (big_sprite)
		{
			last_continuation_tile = true;   /* don't clear big_sprite until last tile done */
		}


		if (BIT(~spritecont, 2))
			color = spritedata & 0xff;


// The bigsprite == 0 check fixes "tied-up" little sprites in Thunderfox
// which (mostly?) have spritecont = 0x20 when they are not continuations
// of anything.
		if (!big_sprite || !(spritecont & 0xf0))
		{
			x = m_spriteram_buffered[(offs + 4) / 2];

// Some absolute x values deduced here are 1 too high (scenes when you get
// home run in Koshien, and may also relate to BG layer woods and stuff as you
// journey in MjnQuest). You will see they are 1 pixel too far to the right.
// Where is this extra pixel offset coming from??

			if (BIT(x, 15))   /* absolute (koshien) */
			{
				scrollx = - f2_x_offset - 0x60;
				scrolly = 0;
			}
			else if (BIT(x, 14))   /* ignore extra scroll */
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
			y = m_spriteram_buffered[(offs + 6) / 2] & 0xfff;

			xcurrent = x;
			ycurrent = y;
		}
		else
		{
			if (BIT(~spritecont, 4))
				y = ycurrent;
			else if (BIT(spritecont, 5))
			{
				y += 16;
				y_no++;   /* keep track of y tile for zooms */
			}
			if (BIT(~spritecont, 6))
				x = xcurrent;
			else if (BIT(spritecont, 7))
			{
				x += 16;
				y_no = 0;
				x_no++;   /* keep track of x tile for zooms */
			}
		}

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;
			zx = 0x10;  /* default, no zoom: 16 pixels across */
			zy = 0x10;  /* default, no zoom: 16 pixels vertical */

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
			zoomword = m_spriteram_buffered[(offs + 2) / 2];
			zoomy = (zoomword >> 8) & 0xff;
			zoomx = (zoomword >> 0) & 0xff;
			zx = (0x100 - zoomx) / 16;
			zy = (0x100 - zoomy) / 16;
		}

		if (last_continuation_tile)
		{
			big_sprite = false;
			last_continuation_tile = false;
		}

		u32 code = 0;
		int extoffs = offs;
		/* spriteram[0x4000-7fff] has no corresponding extension area */
		if (extoffs >= 0x8000) extoffs -= 0x4000;

		if (m_sprite_type == 0)
		{
			code = m_spriteram_buffered[offs / 2] & 0x1fff;
			const u32 i = (code & 0x1c00) >> 10;
			code = m_spritebank[i] + (code & 0x3ff);
		}

		if (m_sprite_type == 1)   /* Yuyugogo */
		{
			code = m_spriteram_buffered[offs / 2] & 0x3ff;
			code |= (m_sprite_extension[(extoffs >> 4)] & 0x3f) << 10;
		}

		if (m_sprite_type == 2)   /* Pulirula */
		{
			code = m_spriteram_buffered[offs / 2] & 0xff;
			code |= (m_sprite_extension[(extoffs >> 4)] & 0xff00);
		}

		if (m_sprite_type == 3)   /* Dinorex and a few quizzes */
		{
			code = m_spriteram_buffered[offs / 2] & 0xff;
			code |= (m_sprite_extension[(extoffs >> 4)] & 0xff) << 8;
		}

		if (code == 0) continue;

		int flipx = BIT(spritecont, 0);
		int flipy = BIT(spritecont, 1);

		int curx = util::sext(x + scrollx, 12);
		int cury = util::sext(y + scrolly, 12);

		if (m_sprites_flipscreen)
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
			if (m_gfxdecode->gfx(0)->granularity() == 64)    /* Final Blow is 6-bit deep */
				sprite_ptr->color /= 4;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks || uses_tc360_mixer)
			{
				if (uses_tc360_mixer)
					sprite_ptr->primask = (color & 0xc0) >> 6;
				else if (primasks)
					sprite_ptr->primask = primasks[(color & 0xc0) >> 6];

				sprite_ptr++;
			}
			else
			{
				m_gfxdecode->gfx(0)->zoom_transpen(bitmap, cliprect,
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx, sprite_ptr->flipy,
						sprite_ptr->x, sprite_ptr->y,
						sprite_ptr->zoomx, sprite_ptr->zoomy,
						0);
			}
		}
	}


	/* this happens only if primsks != nullptr */
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		taito_f2_tc360_spritemixdraw(screen, bitmap, cliprect,
				m_gfxdecode->gfx(0),
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx, sprite_ptr->flipy,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->zoomx, sprite_ptr->zoomy,
				sprite_ptr->primask,
				uses_tc360_mixer);
	}
}


void taitof2_state::update_spritebanks()
{
#if 1
	for (int i = 0; i < 8; i ++)
	{
		m_spritebank[i] = m_spritebank_buffered[i];
	}
#else
	/* this makes footchmp blobbing worse! */
	for (i = 0; i < 8; i ++)
	{
		m_spritebank[i] = m_spritebank_eof[i];
		m_spritebank_eof[i] = m_spritebank_buffered[i];
	}
#endif
}

void taitof2_state::handle_sprite_buffering()
{
	if (m_prepare_sprites)   /* no buffering */
	{
		memcpy(m_spriteram_buffered.get(), m_spriteram, m_spriteram.bytes());
		m_prepare_sprites = false;
	}
}

void taitof2_state::update_sprites_active_area()
{
	update_spritebanks();

	/* if the frame was skipped, we'll have to do the buffering now */
	handle_sprite_buffering();

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (m_sprites_active_area == 0x8000 &&
			m_spriteram_buffered[(0x8000 + 6) / 2] == 0 &&
			m_spriteram_buffered[(0x8000 + 10) / 2] == 0)
		m_sprites_active_area = 0;

	for (int off = 0; off < 0x4000; off += 16)
	{
		/* sprites_active_area may change during processing */
		const int offs = off + m_sprites_active_area;

		if (BIT(m_spriteram_buffered[(offs + 6) / 2], 15))
		{
			m_sprites_disabled = BIT(m_spriteram_buffered[(offs + 10) / 2], 12);
			if (m_game == FOOTCHMP)
				m_sprites_active_area = 0x8000 * (m_spriteram_buffered[(offs + 6) / 2] & 0x0001);
			else
				m_sprites_active_area = 0x8000 * (m_spriteram_buffered[(offs + 10) / 2] & 0x0001);
			continue;
		}

		/* check for extra scroll offset */
		if ((m_spriteram_buffered[(offs + 4) / 2] & 0xf000) == 0xa000)
		{
			m_sprites_master_scrollx = util::sext(m_spriteram_buffered[(offs + 4) / 2], 12);
			m_sprites_master_scrolly = util::sext(m_spriteram_buffered[(offs + 6) / 2], 12);
		}
	}
}


void taitof2_state::screen_vblank_no_buffer(int state)
{
	// rising edge
	if (state)
	{
		update_sprites_active_area();

		m_prepare_sprites = true;
	}
}

void taitof2_state::screen_vblank_full_buffer_delayed(int state)
{
	// rising edge
	if (state)
	{
		update_sprites_active_area();

		m_prepare_sprites = false;
		memcpy(m_spriteram_buffered.get(), m_spriteram_delayed.get(), m_spriteram.bytes());
		for (int i = 0; i < m_spriteram.bytes() / 2; i++)
			m_spriteram_buffered[i] = m_spriteram[i];
		memcpy(m_spriteram_delayed.get(), m_spriteram, m_spriteram.bytes());
	}
}

void taitof2_state::screen_vblank_partial_buffer_delayed(int state)
{
	// rising edge
	if (state)
	{
		update_sprites_active_area();

		m_prepare_sprites = false;
		memcpy(m_spriteram_buffered.get(), m_spriteram_delayed.get(), m_spriteram.bytes());
		for (int i = 0; i < m_spriteram.bytes() / 2; i += 4)
			m_spriteram_buffered[i] = m_spriteram[i];
		memcpy(m_spriteram_delayed.get(), m_spriteram, m_spriteram.bytes());
	}
}

void taitof2_state::screen_vblank_partial_buffer_delayed_thundfox(int state)
{
	// rising edge
	if (state)
	{
		update_sprites_active_area();

		m_prepare_sprites = false;
		memcpy(m_spriteram_buffered.get(), m_spriteram_delayed.get(), m_spriteram.bytes());
		for (int i = 0; i < m_spriteram.bytes() / 2; i += 8)
		{
			m_spriteram_buffered[i]     = m_spriteram[i];
			m_spriteram_buffered[i + 1] = m_spriteram[i + 1];
			m_spriteram_buffered[i + 4] = m_spriteram[i + 4];
		}
		memcpy(m_spriteram_delayed.get(), m_spriteram, m_spriteram.bytes());
	}
}

void taitof2_state::screen_vblank_partial_buffer_delayed_qzchikyu(int state)
{
	// rising edge
	if (state)
	{
		/* spriteram[2] and [3] are 1 frame behind...
		   probably thundfox_eof_callback would work fine */

		update_sprites_active_area();

		m_prepare_sprites = false;
		memcpy(m_spriteram_buffered.get(), m_spriteram_delayed.get(), m_spriteram.bytes());
		for (int i = 0; i < m_spriteram.bytes() / 2; i += 8)
		{
			m_spriteram_buffered[i]     = m_spriteram[i];
			m_spriteram_buffered[i + 1] = m_spriteram[i + 1];
			m_spriteram_buffered[i + 4] = m_spriteram[i + 4];
			m_spriteram_buffered[i + 5] = m_spriteram[i + 5]; // not needed?
			m_spriteram_buffered[i + 6] = m_spriteram[i + 6]; // not needed?
			m_spriteram_buffered[i + 7] = m_spriteram[i + 7]; // not needed?
		}
		memcpy(m_spriteram_delayed.get(), m_spriteram, m_spriteram.bytes());
	}
}


/* SSI */
u32 taitof2_state::screen_update_ssi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_sprite_buffering();

	/* SSI only uses sprites, the tilemap registers are not even initialized.
	   (they are in Majestic 12, but the tilemaps are not used anyway) */
	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);
	draw_sprites(screen, bitmap, cliprect, nullptr, 0);
	return 0;
}


u32 taitof2_state::screen_update_yesnoj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_sprite_buffering();

	m_tc0100scn[0]->tilemap_update();

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */
	draw_sprites(screen, bitmap, cliprect, nullptr, 0);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, m_tc0100scn[0]->bottomlayer(), 0, 0);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, m_tc0100scn[0]->bottomlayer() ^ 1, 0, 0);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	return 0;
}


u32 taitof2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_sprite_buffering();

	m_tc0100scn[0]->tilemap_update();

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, m_tc0100scn[0]->bottomlayer(), 0, 0);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, m_tc0100scn[0]->bottomlayer() ^ 1, 0, 0);
	draw_sprites(screen, bitmap, cliprect, nullptr, 0);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	return 0;
}


u32 taitof2_state::screen_update_pri(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3];

	handle_sprite_buffering();

	m_tc0100scn[0]->tilemap_update();

	layer[0] = m_tc0100scn[0]->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;
	m_tilepri[layer[0]] = m_tc0360pri->read(5) & 0x0f;
	m_tilepri[layer[1]] = m_tc0360pri->read(5) >> 4;
	m_tilepri[layer[2]] = m_tc0360pri->read(4) >> 4;

	m_spritepri[0] = m_tc0360pri->read(6) & 0x0f;
	m_spritepri[1] = m_tc0360pri->read(6) >> 4;
	m_spritepri[2] = m_tc0360pri->read(7) & 0x0f;
	m_spritepri[3] = m_tc0360pri->read(7) >> 4;

	m_spriteblendmode = m_tc0360pri->read(0) & 0xc0;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */

	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	draw_sprites(screen, bitmap, cliprect, nullptr, 1);
	return 0;
}



void dondokod_state::draw_roz_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 priority, u8 priority_mask)
{
	if (m_tc0280grd != nullptr)
		m_tc0280grd->zoom_draw(screen, bitmap, cliprect, m_pivot_xdisp, m_pivot_ydisp, priority, priority_mask);

	if (m_tc0430grw != nullptr)
		m_tc0430grw->zoom_draw(screen, bitmap, cliprect, m_pivot_xdisp, m_pivot_ydisp, priority, priority_mask);
}

u32 dondokod_state::screen_update_pri_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int tilepri[3];
	int layer[3];
	const u16 roz_base_color = (m_tc0360pri->read(1) & 0x3f) << 2;

	handle_sprite_buffering();

	if (m_tc0280grd != nullptr)
		m_tc0280grd->tilemap_update(roz_base_color);

	if (m_tc0430grw != nullptr)
		m_tc0430grw->tilemap_update(roz_base_color);

	m_tc0100scn[0]->tilemap_update();

	u8 rozpri = (m_tc0360pri->read(1) & 0xc0) >> 6;
	rozpri = (m_tc0360pri->read(8 + rozpri / 2) >> 4 * (rozpri & 1)) & 0x0f;

	layer[0] = m_tc0100scn[0]->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	tilepri[layer[0]] = m_tc0360pri->read(5) & 0x0f;
	tilepri[layer[1]] = m_tc0360pri->read(5) >> 4;
	tilepri[layer[2]] = m_tc0360pri->read(4) >> 4;

	m_spritepri[0] = m_tc0360pri->read(6) & 0x0f;
	m_spritepri[1] = m_tc0360pri->read(6) >> 4;
	m_spritepri[2] = m_tc0360pri->read(7) & 0x0f;
	m_spritepri[3] = m_tc0360pri->read(7) >> 4;

	m_spriteblendmode = m_tc0360pri->read(0) & 0xc0;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */

	int drawn = 0;
	for (int i = 0; i < 16; i++)
	{
		if (rozpri == i)
		{
			draw_roz_layer(screen, bitmap, cliprect, 1 << drawn);
			m_tilepri[drawn] = i;
			drawn++;
		}

		for (int j = 0; j < 3; j++)
		{
			if (tilepri[layer[j]] == i)
			{
				m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, layer[j], 0, 1 << drawn);
				m_tilepri[drawn] = i;
				drawn++;
			}
		}
	}

	draw_sprites(screen, bitmap, cliprect, nullptr, 1);
	return 0;
}


/* Thunderfox */
u32 taitof2_state::screen_update_thundfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int tilepri[2][3];
	int spritepri[4];
	int layer[2][3];
	int drawn[2];

	handle_sprite_buffering();

	m_tc0100scn[0]->tilemap_update();
	m_tc0100scn[1]->tilemap_update();

	layer[0][0] = m_tc0100scn[0]->bottomlayer();
	layer[0][1] = layer[0][0] ^ 1;
	layer[0][2] = 2;
	tilepri[0][layer[0][0]] = m_tc0360pri->read(5) & 0x0f;
	tilepri[0][layer[0][1]] = m_tc0360pri->read(5) >> 4;
	tilepri[0][layer[0][2]] = m_tc0360pri->read(4) >> 4;

	layer[1][0] = m_tc0100scn[1]->bottomlayer();
	layer[1][1] = layer[1][0] ^ 1;
	layer[1][2] = 2;
	tilepri[1][layer[1][0]] = m_tc0360pri->read(9) & 0x0f;
	tilepri[1][layer[1][1]] = m_tc0360pri->read(9) >> 4;
	tilepri[1][layer[1][2]] = m_tc0360pri->read(8) >> 4;

	spritepri[0] = m_tc0360pri->read(6) & 0x0f;
	spritepri[1] = m_tc0360pri->read(6) >> 4;
	spritepri[2] = m_tc0360pri->read(7) & 0x0f;
	spritepri[3] = m_tc0360pri->read(7) >> 4;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */

	drawn[0] = drawn[1] = 0;
	while (drawn[0] < 3 && drawn[1] < 3)
	{
		int pick;

		if (tilepri[0][drawn[0]] < tilepri[1][drawn[1]])
		{
			pick = 0;
		}
		else
		{
			pick = 1;
		}

		m_tc0100scn[pick]->tilemap_draw(screen, bitmap, cliprect, layer[pick][drawn[pick]], 0, 1 << (drawn[pick] + 3 * pick));
		drawn[pick]++;
	}
	while (drawn[0] < 3)
	{
		m_tc0100scn[0]->tilemap_draw(screen, bitmap, cliprect, layer[0][drawn[0]], 0, 1 << drawn[0]);
		drawn[0]++;
	}
	while (drawn[1] < 3)
	{
		m_tc0100scn[1]->tilemap_draw(screen, bitmap, cliprect, layer[1][drawn[1]], 0, 1 << (drawn[1] + 3));
		drawn[1]++;
	}

	u64 primasks[4] = {0,0,0,0};

	for (int i = 0; i < 4; i++)
	{
		if (spritepri[i] < tilepri[0][0]) primasks[i] |= 0xaaaaaaaaaaaaaaaaU;
		if (spritepri[i] < tilepri[0][1]) primasks[i] |= 0xccccccccccccccccU;
		if (spritepri[i] < tilepri[0][2]) primasks[i] |= 0xf0f0f0f0f0f0f0f0U;
		if (spritepri[i] < tilepri[1][0]) primasks[i] |= 0xff00ff00ff00ff00U;
		if (spritepri[i] < tilepri[1][1]) primasks[i] |= 0xffff0000ffff0000U;
		if (spritepri[i] < tilepri[1][2]) primasks[i] |= 0xffffffff00000000U;
	}

	draw_sprites(screen, bitmap, cliprect, primasks, 0);
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

u32 footchmp_state::screen_update_metalb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5], invlayer[4];

	handle_sprite_buffering();

	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	invlayer[layer[0]] = 0;
	invlayer[layer[1]] = 1;
	invlayer[layer[2]] = 2;
	invlayer[layer[3]] = 3;

	m_tilepri[invlayer[0]] = m_tc0360pri->read(4) & 0x0f; /* bg0 */
	m_tilepri[invlayer[1]] = m_tc0360pri->read(4) >> 4;   /* bg1 */
	m_tilepri[invlayer[2]] = m_tc0360pri->read(5) & 0x0f; /* bg2 */
	m_tilepri[invlayer[3]] = m_tc0360pri->read(5) >> 4;   /* bg3 */
	m_tilepri[4] = m_tc0360pri->read(9) & 0x0f;           /* fg (text layer) */

	m_spritepri[0] = m_tc0360pri->read(6) & 0x0f;
	m_spritepri[1] = m_tc0360pri->read(6) >> 4;
	m_spritepri[2] = m_tc0360pri->read(7) & 0x0f;
	m_spritepri[3] = m_tc0360pri->read(7) >> 4;

	m_spriteblendmode = m_tc0360pri->read(0) & 0xc0;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 16);

	draw_sprites(screen, bitmap, cliprect, nullptr, 1);
	return 0;
}


/* Deadconx, Footchmp */
u32 footchmp_state::screen_update_deadconx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u8 tilepri[5];
	u8 spritepri[4];

	handle_sprite_buffering();

	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	tilepri[0] = m_tc0360pri->read(4) >> 4;      /* bg0 */
	tilepri[1] = m_tc0360pri->read(5) & 0x0f;    /* bg1 */
	tilepri[2] = m_tc0360pri->read(5) >> 4;      /* bg2 */
	tilepri[3] = m_tc0360pri->read(4) & 0x0f;    /* bg3 */
	tilepri[layer[4]] = m_tc0360pri->read(9) >> 4;    /* fg (text layer) */

	spritepri[0] = m_tc0360pri->read(6) & 0x0f;
	spritepri[1] = m_tc0360pri->read(6) >> 4;
	spritepri[2] = m_tc0360pri->read(7) & 0x0f;
	spritepri[3] = m_tc0360pri->read(7) >> 4;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 16);

	u64 primasks[4] = {0,0,0,0};

	for (int i = 0; i < 4; i++)
	{
		if (spritepri[i] < tilepri[(layer[0])]) primasks[i] |= 0xaaaaaaaaaaaaaaaaU;
		if (spritepri[i] < tilepri[(layer[1])]) primasks[i] |= 0xccccccccccccccccU;
		if (spritepri[i] < tilepri[(layer[2])]) primasks[i] |= 0xf0f0f0f0f0f0f0f0U;
		if (spritepri[i] < tilepri[(layer[3])]) primasks[i] |= 0xff00ff00ff00ff00U;
		if (spritepri[i] < tilepri[(layer[4])]) primasks[i] |= 0xffff0000ffff0000U;
	}

	draw_sprites(screen, bitmap, cliprect, primasks, 0);
	return 0;
}
