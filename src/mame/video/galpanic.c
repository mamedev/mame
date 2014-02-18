#include "emu.h"
#include "kan_pand.h"
#include "includes/galpanic.h"


VIDEO_START_MEMBER(galpanic_state,galpanic)
{
	m_screen->register_screen_bitmap(m_bitmap);
	m_screen->register_screen_bitmap(m_sprites_bitmap);
}

PALETTE_INIT_MEMBER(galpanic_state,galpanic)
{
	int i;

	/* first 1024 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0;i < 32768;i++)
		palette.set_pen_color(i+1024,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}



WRITE16_MEMBER(galpanic_state::galpanic_bgvideoram_w)
{
	int sx,sy;


	data = COMBINE_DATA(&m_bgvideoram[offset]);

	sy = offset / 256;
	sx = offset % 256;

	m_bitmap.pix16(sy, sx) = 1024 + (data >> 1);
}

WRITE16_MEMBER(galpanic_state::galpanic_paletteram_w)
{
	data = COMBINE_DATA(&m_generic_paletteram_16[offset]);
	/* bit 0 seems to be a transparency flag for the front bitmap */
	m_palette->set_pen_color(offset,pal5bit(data >> 6),pal5bit(data >> 11),pal5bit(data >> 1));
}


void galpanic_state::comad_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *spriteram16 = m_spriteram;
	int offs;
	int sx=0, sy=0;

	for (offs = 0;offs < m_spriteram.bytes()/2;offs += 4)
	{
		int code,color,flipx,flipy;

		code = spriteram16[offs + 1] & 0x1fff;
		color = (spriteram16[offs] & 0x003c) >> 2;
		flipx = spriteram16[offs] & 0x0002;
		flipy = spriteram16[offs] & 0x0001;

		if((spriteram16[offs] & 0x6000) == 0x6000) /* Link bits */
		{
			sx += spriteram16[offs + 2] >> 6;
			sy += spriteram16[offs + 3] >> 6;
		}
		else
		{
			sx = spriteram16[offs + 2] >> 6;
			sy = spriteram16[offs + 3] >> 6;
		}

		sx = (sx&0x1ff) - (sx&0x200);
		sy = (sy&0x1ff) - (sy&0x200);

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

void galpanic_state::draw_fgbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = 0;offs < m_fgvideoram.bytes()/2;offs++)
	{
		int sx,sy,color;

		sx = offs % 256;
		sy = offs / 256;
		color = m_fgvideoram[offs];
		if (color)
			bitmap.pix16(sy, sx) = color;
	}
}

UINT32 galpanic_state::screen_update_galpanic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,m_bitmap,0,0,0,0,cliprect);

	draw_fgbitmap(bitmap, cliprect);

	m_pandora->update(bitmap, cliprect);

	return 0;
}

UINT32 galpanic_state::screen_update_comad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,m_bitmap,0,0,0,0,cliprect);

	draw_fgbitmap(bitmap, cliprect);


//  if(galpanic_clear_sprites)
	{
		m_sprites_bitmap.fill(0, cliprect);
		comad_draw_sprites(bitmap,cliprect);
	}
//  else
//  {
//      /* keep sprites on the bitmap without clearing them */
//      comad_draw_sprites(machine(),m_sprites_bitmap,0);
//      copybitmap_trans(bitmap,m_sprites_bitmap,0,0,0,0,cliprect,0);
//  }
	return 0;
}
