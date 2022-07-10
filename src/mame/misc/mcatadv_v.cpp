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
#include "mcatadv.h"
#include "screen.h"

#include <algorithm>


void mcatadv_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	u16 *source = (m_spriteram_old.get() + (m_spriteram.bytes() / 2) /2);
	source -= 4;
	u16 *finish = m_spriteram_old.get();
	int const global_x = m_vidregs[0] - 0x184;
	int const global_y = m_vidregs[1] - 0x1f1;

	u32 const sprmask = m_sprdata.bytes()-1;

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
		u32 const pen = (source[0] & 0x3f00) >> 8;
		u32 const tileno = source[1] & 0xffff;
		u8 pri = (source[0] & 0xc000) >> 14;

		pri |= 0x8;

		int x = source[2] & 0x3ff;
		int y = source[3] & 0x3ff;
		int flipy = source[0] & 0x0040;
		int flipx = source[0] & 0x0080;

		int const height = ((source[3] & 0xf000) >> 12) * 16;
		int const width = ((source[2] & 0xf000) >> 12) * 16;
		u32 offset = tileno * 256;

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

			for (int ycnt = ystart; ycnt != yend; ycnt += yinc)
			{
				const int drawypos = y + ycnt - global_y;

				if ((drawypos >= cliprect.min_y) && (drawypos <= cliprect.max_y))
				{
					u16 *const destline = &bitmap.pix(drawypos);
					u8 *const priline = &screen.priority().pix(drawypos);

					for (int xcnt = xstart; xcnt != xend; xcnt += xinc)
					{
						const int drawxpos = x + xcnt - global_x;

						if ((drawxpos >= cliprect.min_x) && (drawxpos <= cliprect.max_x))
						{
							const int pridata = priline[drawxpos];

							if (!(pridata & 0x10)) // if we haven't already drawn a sprite pixel here (sprite masking)
							{
								u8 pix = m_sprdata[(offset / 2)&sprmask];

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

void mcatadv_state::mcatadv_draw_tilemap_part( screen_device &screen, int layer, int i, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if (!m_tilemap[layer]->enable())
		return;

	rectangle clip;

	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;

	for (u32 drawline = cliprect.min_y; drawline <= cliprect.max_y; drawline++)
	{
		clip.min_y = drawline;
		clip.max_y = drawline;

		int scrollx = (m_tilemap[layer]->scrollx() & 0x1ff) - 0x194;
		int scrolly = (m_tilemap[layer]->scrolly() & 0x1ff) - 0x1df;

		if (m_tilemap[layer]->rowselect_en())
		{
			const int rowselect = m_tilemap[layer]->rowselect(drawline + scrolly);
			scrolly = rowselect - drawline;
		}

		if (m_tilemap[layer]->rowscroll_en())
		{
			const int rowscroll = m_tilemap[layer]->rowscroll(drawline + scrolly);
			scrollx += rowscroll;
		}

		/* Global Flip */
		if (m_tilemap[layer]->flipx()) scrollx -= 0x19;
		if (m_tilemap[layer]->flipy()) scrolly -= 0x141;
		int flip = (m_tilemap[layer]->flipx() ? TILEMAP_FLIPX : 0) | (m_tilemap[layer]->flipy() ? TILEMAP_FLIPY : 0);

		m_tilemap[layer]->set_scrollx(0, scrollx);
		m_tilemap[layer]->set_scrolly(0, scrolly);
		m_tilemap[layer]->set_flip(flip);

		m_tilemap[layer]->draw(screen, bitmap, clip, i, i);
	}
}

u32 mcatadv_state::screen_update_mcatadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x3f0, cliprect);
	screen.priority().fill(0, cliprect);

	for (int i = 0; i < 2; i++)
	{
		m_tilemap[i]->prepare();
		if (m_tilemap[i]->external() != m_palette_bank[i])
		{
			m_palette_bank[i] = m_tilemap[i]->external()&0xf;
			m_tilemap[i]->mark_all_dirty();
		}
	}

/*
    popmessage("%02x %02x %02x %02x",
        m_tilemap[0]->rowscroll_en(),
        m_tilemap[0]->rowselect_en(),
        m_tilemap[1]->rowscroll_en(),
        m_tilemap[1]->rowselect_en());
*/

	for (int i = 0; i <= 3; i++)
	{
	#ifdef MAME_DEBUG
			if (!machine().input().code_pressed(KEYCODE_Q))
	#endif
				mcatadv_draw_tilemap_part(screen, 0, i|0x8, bitmap, cliprect);

	#ifdef MAME_DEBUG
			if (!machine().input().code_pressed(KEYCODE_W))
	#endif
				mcatadv_draw_tilemap_part(screen, 1, i|0x8, bitmap, cliprect);
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
	m_spriteram_old = make_unique_clear<u16[]>(m_spriteram.bytes() / 2);
	m_vidregs_old = std::make_unique<u16[]>(m_vidregs.bytes() / 2);

	m_palette_bank[0] = m_palette_bank[1] = 0;

	save_pointer(NAME(m_spriteram_old), m_spriteram.bytes() / 2);
	save_pointer(NAME(m_vidregs_old), m_vidregs.bytes() / 2);
}

WRITE_LINE_MEMBER(mcatadv_state::screen_vblank_mcatadv)
{
	// rising edge
	if (state)
	{
		std::copy(&m_spriteram[0], &m_spriteram[m_spriteram.bytes() / 2], &m_spriteram_old[0]);
		std::copy(&m_vidregs[0], &m_vidregs[m_vidregs.bytes() / 2], &m_vidregs_old[0]);
	}
}
