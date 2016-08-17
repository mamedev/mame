// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

  video/balsente.c

  Functions to emulate the video hardware of the machine.

****************************************************************************/

#include "emu.h"
#include "includes/balsente.h"


/*************************************
 *
 *  Video system start
 *
 *************************************/

void balsente_state::video_start()
{
	/* reset the system */
	m_palettebank_vis = 0;
	m_sprite_bank[0] = memregion("gfx1")->base();
	m_sprite_bank[1] = memregion("gfx1")->base() + 0x10000;

	/* determine sprite size */
	m_sprite_data = memregion("gfx1")->base();
	m_sprite_mask = memregion("gfx1")->bytes() - 1;

	/* register for saving */
	save_item(NAME(m_expanded_videoram));
	save_item(NAME(m_palettebank_vis));
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_videoram_w)
{
	/* expand the two pixel values into two bytes */
	m_videoram[offset] = data;

	m_expanded_videoram[offset * 2 + 0] = data >> 4;
	m_expanded_videoram[offset * 2 + 1] = data & 15;
}



/*************************************
 *
 *  Palette banking
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_palette_select_w)
{
	/* only update if changed */
	if (m_palettebank_vis != (data & 3))
	{
		/* update the scanline palette */
		m_screen->update_partial(m_screen->vpos() - 1 + BALSENTE_VBEND);
		m_palettebank_vis = data & 3;
	}

	logerror("balsente_palette_select_w(%d) scanline=%d\n", data & 3, m_screen->vpos());
}



/*************************************
 *
 *  Palette RAM write
 *
 *************************************/

WRITE8_MEMBER(balsente_state::balsente_paletteram_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data & 0x0f;

	r = m_generic_paletteram_8[(offset & ~3) + 0];
	g = m_generic_paletteram_8[(offset & ~3) + 1];
	b = m_generic_paletteram_8[(offset & ~3) + 2];

	m_palette->set_pen_color(offset / 4, pal4bit(r), pal4bit(g), pal4bit(b));
}



/*************************************
 *
 *  Sprite banking
 *
 *************************************/

WRITE8_MEMBER(balsente_state::shrike_sprite_select_w)
{
	if( m_sprite_data != m_sprite_bank[(data & 0x80 >> 7) ^ 1 ])
	{
		logerror( "shrike_sprite_select_w( 0x%02x )\n", data );
		m_screen->update_partial(m_screen->vpos() - 1 + BALSENTE_VBEND);
		m_sprite_data = m_sprite_bank[(data & 0x80 >> 7) ^ 1];
	}

	shrike_shared_6809_w( space, 1, data );
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void balsente_state::draw_one_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *sprite)
{
	int flags = sprite[0];
	int image = sprite[1] | ((flags & 7) << 8);
	int ypos = sprite[2] + 17 + BALSENTE_VBEND;
	int xpos = sprite[3];
	UINT8 *src;
	int x, y;

	/* get a pointer to the source image */
	src = &m_sprite_data[(64 * image) & m_sprite_mask];
	if (flags & 0x80) src += 4 * 15;

	/* loop over y */
	for (y = 0; y < 16; y++, ypos = (ypos + 1) & 255)
	{
		if (ypos >= (16 + BALSENTE_VBEND) && ypos >= cliprect.min_y && ypos <= cliprect.max_y)
		{
			const pen_t *pens = &m_palette->pen(m_palettebank_vis * 256);
			UINT8 *old = &m_expanded_videoram[(ypos - BALSENTE_VBEND) * 256 + xpos];
			int currx = xpos;

			/* standard case */
			if (!(flags & 0x40))
			{
				/* loop over x */
				for (x = 0; x < 4; x++, old += 2)
				{
					int ipixel = *src++;
					int left = ipixel & 0xf0;
					int right = (ipixel << 4) & 0xf0;

					/* left pixel, combine with the background */
					if (left && currx >= 0 && currx < 256)
						bitmap.pix16(ypos, currx) = pens[left | old[0]];
					currx++;

					/* right pixel, combine with the background */
					if (right && currx >= 0 && currx < 256)
						bitmap.pix16(ypos, currx) = pens[right | old[1]];
					currx++;
				}
			}

			/* hflip case */
			else
			{
				src += 4;

				/* loop over x */
				for (x = 0; x < 4; x++, old += 2)
				{
					int ipixel = *--src;
					int left = (ipixel << 4) & 0xf0;
					int right = ipixel & 0xf0;

					/* left pixel, combine with the background */
					if (left && currx >= 0 && currx < 256)
						bitmap.pix16(ypos, currx) = pens[left | old[0]];
					currx++;

					/* right pixel, combine with the background */
					if (right && currx >= 0 && currx < 256)
						bitmap.pix16(ypos, currx) = pens[right | old[1]];
					currx++;
				}
				src += 4;
			}
		}
		else
			src += 4;
		if (flags & 0x80) src -= 2 * 4;
	}
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

UINT32 balsente_state::screen_update_balsente(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = &m_palette->pen(m_palettebank_vis * 256);
	int y, i;

	/* draw scanlines from the VRAM directly */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		draw_scanline8(bitmap, 0, y, 256, &m_expanded_videoram[(y - BALSENTE_VBEND) * 256], pens);

	/* draw the sprite images */
	for (i = 0; i < 40; i++)
		draw_one_sprite(bitmap, cliprect, &m_spriteram[(0xe0 + i * 4) & 0xff]);

	return 0;
}
