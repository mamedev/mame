// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Funny Bubble Video hardware

todo - convert to tilemap

 */


#include "emu.h"
#include "includes/funybubl.h"


WRITE8_MEMBER(funybubl_state::funybubl_paldatawrite)
{
	int colchanged ;
	UINT32 coldat;

	m_paletteram[offset] = data;
	colchanged = offset >> 2;
	coldat = m_paletteram[colchanged * 4] | (m_paletteram[colchanged * 4 + 1] << 8) |
			(m_paletteram[colchanged * 4 + 2] << 16) | (m_paletteram[colchanged * 4 + 3] << 24);

	m_palette->set_pen_color(colchanged, pal6bit(coldat >> 12), pal6bit(coldat >> 0), pal6bit(coldat >> 6));
}


void funybubl_state::video_start()
{
}

void funybubl_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *source = &m_banked_vram[0x2000 - 0x20];
	UINT8 *finish = source - 0x1000;

	while (source > finish)
	{
		int xpos, ypos, tile;

		/* the sprites are in the sprite list twice
		 the first format (in comments) appears to be a buffer, if you use
		 this list you get garbage sprites in 2 player mode
		 the second format (used) seems correct

		 */
/*
        ypos = 0xff - source[1 + 0x10];
        xpos = source[2 + 0x10];
        tile = source[0 + 0x10] | ( (source[3 + 0x10] & 0x0f) <<8);
        if (source[3 + 0x10] & 0x80) tile += 0x1000;
        if (source[3 + 0x10] & 0x20) xpos += 0x100;
        // bits 0x40 (not used?) and 0x10 (just set during transition period of x co-ord 0xff and 0x00) ...
        xpos -= 8;
        ypos -= 14;

*/
		ypos = source[2];
		xpos = source[3];
		tile = source[0] | ( (source[1] & 0x0f) << 8);
		if (source[1] & 0x80) tile += 0x1000;
		if (source[1] & 0x20)
		{
			if (xpos < 0xe0)
				xpos += 0x100;
		}

		// bits 0x40 and 0x10 not used?...

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, tile, 0, 0, 0, xpos, ypos, 255);
		source -= 0x20;
	}
}


UINT32 funybubl_state::screen_update_funybubl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, offs;
	offs = 0;

	bitmap.fill(m_palette->black_pen(), cliprect);

	/* tilemap .. convert it .. banking makes it slightly more annoying but still easy */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x< 64; x++)
		{
			int data;

			data = m_banked_vram[offs] | (m_banked_vram[offs + 1] << 8);
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, data & 0x7fff, (data & 0x8000) ? 2 : 1, 0, 0, x*8, y*8, 0);
			offs += 2;
		}
	}

	draw_sprites(bitmap, cliprect);

#if 0
	if ( machine().input().code_pressed_once(KEYCODE_W) )
	{
		FILE *fp;

		fp = fopen("funnybubsprites", "w+b");
		if (fp)
		{
			fwrite(&m_banked_vram[0x1000], 0x1000, 1, fp);
			fclose(fp);
		}
	}
#endif
	return 0;
}
