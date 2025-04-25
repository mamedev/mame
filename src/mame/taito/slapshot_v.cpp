// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "slapshot.h"
#include "screen.h"

/**********************************************************/

void slapshot_state::video_start()
{
	m_spriteram_delayed = std::make_unique<u16[]>(m_spriteram.bytes() / 2);
	m_spriteram_buffered = std::make_unique<u16[]>(m_spriteram.bytes() / 2);
	m_spritelist = std::make_unique<slapshot_tempsprite[]>(0x400);

	m_sprites_disabled = true;
	m_sprites_active_area = 0;

	save_item(NAME(m_sprites_disabled));
	save_item(NAME(m_sprites_active_area));
	save_item(NAME(m_sprites_master_scrollx));
	save_item(NAME(m_sprites_master_scrolly));
	save_item(NAME(m_sprites_flipscreen));
	save_item(NAME(m_prepare_sprites));
	save_pointer(NAME(m_spriteram_delayed), m_spriteram.bytes() / 2);
	save_pointer(NAME(m_spriteram_buffered), m_spriteram.bytes() / 2);
}

/************************************************************
            SPRITE DRAW ROUTINES
************************************************************/

void slapshot_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 *primasks, int y_offset)
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

	*/
	bool big_sprite = false, last_continuation_tile = false;
	int y_no = 0, x_no = 0, xlatch = 0, ylatch = 0;   /* for zooms */
	u32 zoomword, zoomx, zoomy, zx = 0, zy = 0, zoomxlatch = 0, zoomylatch = 0;   /* for zooms */
	int scrollx = 0, scrolly = 0;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	slapshot_tempsprite *sprite_ptr = &m_spritelist[0];

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

	int x_offset = 3;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
	if (m_sprites_flipscreen) x_offset = -x_offset;

	/* safety check to avoid getting stuck in bank 2 for games using only one bank */
	if (area == 0x8000 &&
			m_spriteram_buffered[(0x8000 + 6) / 2] == 0 &&
			m_spriteram_buffered[(0x8000 + 10) / 2] == 0)
		area = 0;

	for (int off = 0; off < 0x4000; off += 16)
	{
		/* sprites_active_area may change during processing */
		const int offs = off + area;

		if (BIT(m_spriteram_buffered[(offs + 6) / 2], 15))
		{
			disabled = BIT(m_spriteram_buffered[(offs + 10) / 2], 12);
			m_sprites_flipscreen = BIT(m_spriteram_buffered[(offs + 10) / 2], 13);
			x_offset = 3;   /* Get rid of 0-3 unwanted pixels on edge of screen. */
			if (m_sprites_flipscreen) x_offset = -x_offset;
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

// DG: the bigsprite == 0 check fixes "tied-up" little sprites in Thunderfox
// which (mostly?) have spritecont = 0x20 when they are not continuations
// of anything.
		if (!big_sprite || !(spritecont & 0xf0))
		{
			x = m_spriteram_buffered[(offs + 4) / 2];

// DG: some absolute x values deduced here are 1 too high (scenes when you get
// home run in Koshien, and may also relate to BG layer woods and stuff as you
// journey in MjnQuest). You will see they are 1 pixel too far to the right.
// Where is this extra pixel offset coming from??

			if (BIT(x, 15))   /* absolute (koshien) */
			{
				scrollx = - x_offset - 0x60;
				scrolly = 0;
			}
			else if (BIT(x, 14))   /* ignore extra scroll */
			{
				scrollx = master_scrollx - x_offset - 0x60;
				scrolly = master_scrolly;
			}
			else   /* all scrolls applied */
			{
				scrollx = scroll1x + master_scrollx - x_offset - 0x60;
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

/* Black lines between flames in Gunfront attract before the zoom
   finishes suggest these calculations are flawed? */

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
				   coords of our chunk and diagonally adjoining one. */

				x = xlatch + x_no * (0x100 - zoomx) / 16;
				y = ylatch + y_no * (0x100 - zoomy) / 16;
				zx = xlatch + (x_no + 1) * (0x100 - zoomx) / 16 - x;
				zy = ylatch + (y_no + 1) * (0x100 - zoomy) / 16 - y;
			}
		}
		else
		{
			zoomword = m_spriteram_buffered[(offs+2)/2];
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
		if (extoffs >= 0x8000) extoffs -= 0x4000;   /* spriteram[0x4000-7fff] has no corresponding extension area */

		code = m_spriteram_buffered[offs / 2] & 0xff;
		code |= (m_spriteext[(extoffs >> 4)] & 0xff00);

		if (code == 0) continue;

		bool flipx = BIT(spritecont, 0);
		bool flipy = BIT(spritecont, 1);

		int curx = util::sext(x + scrollx, 12);
		int cury = util::sext(y + scrolly, 12);

		if (m_sprites_flipscreen)
		{
			/* -zx/y is there to fix zoomed sprite coords in screenflip.
			   drawgfxzoom does not know to draw from flip-side of sprites when
			   screen is flipped; so we must correct the coords ourselves. */

			curx = 319 - curx - zx;
			cury = 256 - cury - zy;
			flipx = !flipx;
			flipy = !flipy;
		}

		cury += y_offset;

		{
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			if (m_gfxdecode->gfx(0)->granularity() == 64)    /* Final Blow, Slapshot are 6bpp */
				sprite_ptr->color /= 4;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks)
			{
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
	while (sprite_ptr != &m_spritelist[0])
	{
		sprite_ptr--;

		m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx, sprite_ptr->flipy,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->zoomx, sprite_ptr->zoomy,
				screen.priority(), sprite_ptr->primask,
				0);
	}
}


void slapshot_state::handle_sprite_buffering()
{
	if (m_prepare_sprites)   /* no buffering */
	{
		memcpy(m_spriteram_buffered.get(), m_spriteram, m_spriteram.bytes());
		m_prepare_sprites = false;
	}
}

void slapshot_state::update_sprites_active_area()
{
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
		int offs = off + m_sprites_active_area;

		if (BIT(m_spriteram_buffered[(offs + 6) / 2], 15))
		{
			m_sprites_disabled = BIT(m_spriteram_buffered[(offs + 10) / 2], 12);
			m_sprites_active_area = 0x8000 * (m_spriteram_buffered[(offs + 10) / 2] & 0x0001);
			continue;
		}

		/* check for extra scroll offset */
		if ((m_spriteram_buffered[(offs+4)/2] & 0xf000) == 0xa000)
		{
			m_sprites_master_scrollx = util::sext(m_spriteram_buffered[(offs + 4) / 2], 12);
			m_sprites_master_scrolly = util::sext(m_spriteram_buffered[(offs + 6) / 2], 12);
		}
	}
}

void slapshot_state::screen_vblank_no_buffer(int state)
{
	// rising edge
	if (state)
	{
		update_sprites_active_area();
		m_prepare_sprites = true;
	}
}


/**************************************************************
                SCREEN REFRESH

Slapshot and Opwolf3 use in the PRI chip
---------------------------------------

+4  xxxx0000   BG1
    0000xxxx   BG0
+6  xxxx0000   BG3
    0000xxxx   BG2

Slapshot mostly keeps all sprites above the bg layers.
One exception is the "puck" in early attract which is
a bg layer given priority over some sprites.
********************************************************************/

u32 slapshot_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u8 tilepri[5];
	u8 spritepri[4];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_Z))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg0: %01x",m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_X))
	{
		m_dislayer[1] ^= 1;
		popmessage("bg1: %01x",m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_C))
	{
		m_dislayer[2] ^= 1;
		popmessage("bg2: %01x",m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[3] ^= 1;
		popmessage("bg3: %01x",m_dislayer[3]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[4] ^= 1;
		popmessage("text: %01x",m_dislayer[4]);
	}
#endif

	handle_sprite_buffering();

	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	tilepri[0] = m_tc0360pri->read(4) & 0x0f;     /* bg0 */
	tilepri[1] = m_tc0360pri->read(4) >> 4;       /* bg1 */
	tilepri[2] = m_tc0360pri->read(5) & 0x0f;     /* bg2 */
	tilepri[3] = m_tc0360pri->read(5) >> 4;       /* bg3 */

/* we actually assume text layer is on top of everything anyway, but FWIW... */
	tilepri[layer[4]] = m_tc0360pri->read(9) & 0x0f;    /* fg (text layer) */

	spritepri[0] = m_tc0360pri->read(6) & 0x0f;
	spritepri[1] = m_tc0360pri->read(6) >> 4;
	spritepri[2] = m_tc0360pri->read(7) & 0x0f;
	spritepri[3] = m_tc0360pri->read(7) >> 4;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[0]] == 0)
#endif
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[1]] == 0)
#endif
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[2]] == 0)
#endif
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[3]] == 0)
#endif
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

	u32 primasks[4] = {0,0,0,0};

	for (int i = 0; i < 4; i++)
	{
		if (spritepri[i] < tilepri[(layer[0])]) primasks[i] |= 0xaaaa;
		if (spritepri[i] < tilepri[(layer[1])]) primasks[i] |= 0xcccc;
		if (spritepri[i] < tilepri[(layer[2])]) primasks[i] |= 0xf0f0;
		if (spritepri[i] < tilepri[(layer[3])]) primasks[i] |= 0xff00;
	}

	draw_sprites(screen,bitmap,cliprect,primasks,0);

	/*
	TODO: This isn't the correct way to handle the priority. At the moment of
	writing, pdrawgfx() doesn't support 5 layers, so I have to cheat, assuming
	that the FG layer is always on top of sprites.
	*/

#ifdef MAME_DEBUG
	if (m_dislayer[layer[4]] == 0)
#endif
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);
	return 0;
}
