// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*

  Malzak

  Video functions

  SAA 5050 -- Character display
  S2636 (x2) -- Sprites, Sprite->Sprite collisions
  Playfield graphics generator
      (TODO: probably best to switch this to tilemaps one day, figure out banking)

*/


#include "emu.h"
#include "video/saa5050.h"
#include "includes/malzak.h"

UINT32 malzak_state::screen_update_malzak(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int sx, sy;
	int x,y;

	bitmap.fill(rgb_t::black);

	m_trom->screen_update(screen, bitmap, cliprect);

	// playfield - not sure exactly how this works...
	for (x = 0; x < 16; x++)
		for (y = 0; y < 16; y++)
		{
			sx = ((x * 16 - 48) - m_malzak_x) * 2;
			sy = ((y * 16) - m_malzak_y) * 2;

			if (sx < -271*2)
				sx += 512*2;
			if (sx < -15*2)
				sx += 256*2;

			m_gfxdecode->gfx(0)->zoom_transpen(bitmap,cliprect, m_playfield_code[x * 16 + y], 2, 0, 0, sx, sy, 0x20000, 0x20000, 0);
		}

	/* update the S2636 chips */
	bitmap_ind16 const &s2636_0_bitmap = m_s2636_0->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636_1->update(cliprect);

	/* copy the S2636 images into the main bitmap */
	{
		int y;

		for (y = cliprect.min_y; y <= cliprect.max_y / 2; y++)
		{
			int x;

			for (x = cliprect.min_x; x <= cliprect.max_x / 2; x++)
			{
				int pixel0 = s2636_0_bitmap.pix16(y, x);
				int pixel1 = s2636_1_bitmap.pix16(y, x);

				if (S2636_IS_PIXEL_DRAWN(pixel0)) {
					bitmap.pix32(y*2, x*2) = palette[S2636_PIXEL_COLOR(pixel0)];
					bitmap.pix32(y*2+1, x*2) = palette[S2636_PIXEL_COLOR(pixel0)];
					bitmap.pix32(y*2, x*2+1) = palette[S2636_PIXEL_COLOR(pixel0)];
					bitmap.pix32(y*2+1, x*2+1) = palette[S2636_PIXEL_COLOR(pixel0)];
				}

				if (S2636_IS_PIXEL_DRAWN(pixel1)) {
					bitmap.pix32(y*2, x*2) = palette[S2636_PIXEL_COLOR(pixel1)];
					bitmap.pix32(y*2+1, x*2) = palette[S2636_PIXEL_COLOR(pixel1)];
					bitmap.pix32(y*2, x*2+1) = palette[S2636_PIXEL_COLOR(pixel1)];
					bitmap.pix32(y*2+1, x*2+1) = palette[S2636_PIXEL_COLOR(pixel1)];
				}
			}
		}
	}

	return 0;
}

WRITE8_MEMBER(malzak_state::malzak_playfield_w)
{
	int tile = ((m_malzak_x / 16) * 16) + (offset / 16);

//  m_playfield_x[tile] = m_malzak_x / 16;
//  m_playfield_y[tile] = m_malzak_y;
	m_playfield_code[tile] = (data & 0x1f);
	logerror("GFX: 0x16%02x write 0x%02x\n", offset, data);
}
