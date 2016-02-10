// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood
/* Magical Cat Adventure / Nostradamus Video Hardware */

/*
Notes:
Tilemap drawing is a killer on the first level of Nost due to the whole tilemap being dirty every frame.
Sprite drawing is quite fast (See USER1 in the profiler)

ToDo: Fix Sprites & Rowscroll/Select for Cocktail
*/

#include "emu.h"
#include "includes/mcatadv.h"

TILE_GET_INFO_MEMBER(mcatadv_state::get_mcatadv_tile_info1)
{
	int tileno = m_videoram1[tile_index * 2 + 1];
	int colour = (m_videoram1[tile_index * 2] & 0x3f00) >> 8;
	int pri = (m_videoram1[tile_index * 2] & 0xc000) >> 14;

	pri |= 0x8;

	SET_TILE_INFO_MEMBER(0,tileno,colour + m_palette_bank1 * 0x40, 0);
	tileinfo.category = pri;
}

WRITE16_MEMBER(mcatadv_state::mcatadv_videoram1_w)
{
	COMBINE_DATA(&m_videoram1[offset]);
	m_tilemap1->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(mcatadv_state::get_mcatadv_tile_info2)
{
	int tileno = m_videoram2[tile_index * 2 + 1];
	int colour = (m_videoram2[tile_index * 2] & 0x3f00) >> 8;
	int pri = (m_videoram2[tile_index * 2] & 0xc000) >> 14;

	pri |= 0x8;

	SET_TILE_INFO_MEMBER(1, tileno, colour + m_palette_bank2 * 0x40, 0);
	tileinfo.category = pri;
}

WRITE16_MEMBER(mcatadv_state::mcatadv_videoram2_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_tilemap2->mark_tile_dirty(offset / 2);
}


void mcatadv_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 *source = (m_spriteram_old.get() + (m_spriteram.bytes() / 2) /2);
	source -= 4;
	UINT16 *finish = m_spriteram_old.get();
	int global_x = m_vidregs[0] - 0x184;
	int global_y = m_vidregs[1] - 0x1f1;

	UINT16 *destline;
	UINT8 *priline;
	UINT8 *sprdata = memregion("gfx1")->base();
	int sprmask = memregion("gfx1")->bytes()-1;

	int xstart, xend, xinc;
	int ystart, yend, yinc;

	if (m_vidregs_old[2] == 0x0001) /* Double Buffered */
	{
		source += (m_spriteram.bytes() / 2) / 2;
		finish += (m_spriteram.bytes() / 2) / 2;
	}
	else if (m_vidregs_old[2]) /* I suppose it's possible that there is 4 banks, haven't seen it used though */
	{
		logerror("Spritebank != 0/1\n");
	}

	while (source >= finish)
	{
		int pen = (source[0] & 0x3f00) >> 8;
		int tileno = source[1] & 0xffff;
		int pri = (source[0] & 0xc000) >> 14;

		pri |= 0x8;

		int x = source[2] & 0x3ff;
		int y = source[3] & 0x3ff;
		int flipy = source[0] & 0x0040;
		int flipx = source[0] & 0x0080;

		int height = ((source[3] & 0xf000) >> 12) * 16;
		int width = ((source[2] & 0xf000) >> 12) * 16;
		int offset = tileno * 256;

		int drawxpos, drawypos;
		int xcnt, ycnt;
		int pix;

		if (x & 0x200) x-=0x400;
		if (y & 0x200) y-=0x400;

#if 0 // For Flipscreen/Cocktail
		if(m_vidregs[0] & 0x8000)
		{
			flipx = !flipx;
		}
		if(m_vidregs[1] & 0x8000)
		{
			flipy = !flipy;
		}
#endif

		if (source[3] != source[0]) // 'hack' don't draw sprites while its testing the ram!
		{
			if(!flipx) { xstart = 0;        xend = width;  xinc = 1; }
			else       { xstart = width-1;  xend = -1;     xinc = -1; }
			if(!flipy) { ystart = 0;        yend = height; yinc = 1; }
			else       { ystart = height-1; yend = -1;     yinc = -1; }

			for (ycnt = ystart; ycnt != yend; ycnt += yinc)
			{
				drawypos = y + ycnt - global_y;

				if ((drawypos >= cliprect.min_y) && (drawypos <= cliprect.max_y))
				{
					destline = &bitmap.pix16(drawypos);
					priline = &screen.priority().pix8(drawypos);

					for (xcnt = xstart; xcnt != xend; xcnt += xinc)
					{
						drawxpos = x + xcnt - global_x;

						if ((drawxpos >= cliprect.min_x) && (drawxpos <= cliprect.max_x))
						{
							int pridata = priline[drawxpos];


							if (!(pridata & 0x10)) // if we haven't already drawn a sprite pixel here (sprite masking)
							{
								pix = sprdata[(offset / 2)&sprmask];

								if (offset & 1)
									pix = pix >> 4;
								pix &= 0x0f;

								if (pix)
								{
									if ((priline[drawxpos] < pri))
										destline[drawxpos] = (pix + (pen << 4));

									priline[drawxpos] |= 0x10;
								}

							}
						}

						offset++;
					}
				}
				else
				{
					offset += width;
				}
			}
		}
		source -= 4;
	}
}

void mcatadv_state::mcatadv_draw_tilemap_part( screen_device &screen, UINT16* current_scroll, UINT16* current_videoram1, int i, tilemap_t* current_tilemap, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int flip;
	UINT32 drawline;
	rectangle clip;

	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;

	for (drawline = cliprect.min_y; drawline <= cliprect.max_y; drawline++)
	{
		int scrollx, scrolly;

		clip.min_y = drawline;
		clip.max_y = drawline;

		scrollx = (current_scroll[0] & 0x1ff) - 0x194;
		scrolly = (current_scroll[1] & 0x1ff) - 0x1df;

		if ((current_scroll[1] & 0x4000) == 0x4000)
		{
			int rowselect = current_videoram1[0x1000 / 2 + (((drawline + scrolly) & 0x1ff) * 2) + 1];
			scrolly = rowselect - drawline;
		}

		if ((current_scroll[0] & 0x4000) == 0x4000)
		{
			int rowscroll = current_videoram1[0x1000 / 2 + (((drawline + scrolly) & 0x1ff) * 2) + 0];
			scrollx += rowscroll;
		}

		/* Global Flip */
		if (!(current_scroll[0] & 0x8000)) scrollx -= 0x19;
		if (!(current_scroll[1] & 0x8000)) scrolly -= 0x141;
		flip = ((current_scroll[0] & 0x8000) ? 0 : TILEMAP_FLIPX) | ((current_scroll[1] & 0x8000) ? 0 : TILEMAP_FLIPY);

		current_tilemap->set_scrollx(0, scrollx);
		current_tilemap->set_scrolly(0, scrolly);
		current_tilemap->set_flip(flip);

		current_tilemap->draw(screen, bitmap, clip, i, i);
	}
}

UINT32 mcatadv_state::screen_update_mcatadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	bitmap.fill(0x3f0, cliprect);
	screen.priority().fill(0, cliprect);

	if (m_scroll1[2] != m_palette_bank1)
	{
		m_palette_bank1 = m_scroll1[2]&0xf;
		m_tilemap1->mark_all_dirty();
	}

	if (m_scroll2[2] != m_palette_bank2)
	{
		m_palette_bank2 = m_scroll2[2]&0xf;
		m_tilemap2->mark_all_dirty();
	}

/*
    popmessage("%02x %02x %02x %02x",
        (mcatadv_scroll1[0]  & 0x4000) >> 8,
        (mcatadv_scroll1[1]  & 0x4000) >> 8,
        (mcatadv_scroll2[0] & 0x4000) >> 8,
        (mcatadv_scroll2[1] & 0x4000) >> 8);
*/

	for (i = 0; i <= 3; i++)
	{
	#ifdef MAME_DEBUG
			if (!machine().input().code_pressed(KEYCODE_Q))
	#endif
			if (!(m_scroll1[2]&0x10))
				mcatadv_draw_tilemap_part(screen, m_scroll1,  m_videoram1, i|0x8, m_tilemap1, bitmap, cliprect);

	#ifdef MAME_DEBUG
			if (!machine().input().code_pressed(KEYCODE_W))
	#endif
			if (!(m_scroll2[2]&0x10)) // tilemap flicker effect on large shadow, nost level 7
				mcatadv_draw_tilemap_part(screen, m_scroll2, m_videoram2, i|0x8, m_tilemap2, bitmap, cliprect);
	}

	g_profiler.start(PROFILER_USER1);
#ifdef MAME_DEBUG
	if (!machine().input().code_pressed(KEYCODE_E))
#endif
		draw_sprites (screen, bitmap, cliprect);
	g_profiler.stop();
	return 0;
}

void mcatadv_state::video_start()
{
	m_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mcatadv_state::get_mcatadv_tile_info1),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap1->set_transparent_pen(0);

	m_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mcatadv_state::get_mcatadv_tile_info2),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap2->set_transparent_pen(0);

	m_spriteram_old = make_unique_clear<UINT16[]>(m_spriteram.bytes() / 2);
	m_vidregs_old = std::make_unique<UINT16[]>((0x0f + 1) / 2);

	m_palette_bank1 = 0;
	m_palette_bank2 = 0;

	save_pointer(NAME(m_spriteram_old.get()), m_spriteram.bytes() / 2);
	save_pointer(NAME(m_vidregs_old.get()), (0x0f + 1) / 2);
}

void mcatadv_state::screen_eof_mcatadv(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_spriteram_old.get(), m_spriteram, m_spriteram.bytes());
		memcpy(m_vidregs_old.get(), m_vidregs, 0xf);
	}
}
