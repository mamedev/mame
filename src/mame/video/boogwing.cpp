// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "emu.h"
#include "includes/boogwing.h"
#include "screen.h"


void boogwing_state::video_start()
{
	m_priority = 0;
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
	save_item(NAME(m_priority));
}


/* Mix the 2 sprite planes with the already rendered tilemaps..
 note, if we implement tilemap blending etc. too we'll probably have to mix those in here as well..

 this is just a reimplementation of the old priority system used before conversion but to work with
 the bitmaps.  It could probably be simplified / improved greatly, along with the long-standing bugs
 fixed, with manual mixing you have full control.

 apparently priority is based on a PROM, that should be used if possible.
*/
void boogwing_state::mix_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y, x;
	const pen_t *paldata = &m_deco_ace->pen(0);
	bitmap_ind16 *sprite_bitmap1, *sprite_bitmap2;
	bitmap_ind8* priority_bitmap;

	uint16_t priority = m_priority;

	sprite_bitmap1 = &m_sprgen[0]->get_sprite_temp_bitmap();
	sprite_bitmap2 = &m_sprgen[1]->get_sprite_temp_bitmap();
	priority_bitmap = &screen.priority();

	uint32_t* dstline;
	uint16_t *srcline1, *srcline2;
	uint8_t *srcpriline;

	for (y=cliprect.top();y<=cliprect.bottom();y++)
	{
		srcline1=&sprite_bitmap1->pix16(y,0);
		srcline2=&sprite_bitmap2->pix16(y,0);
		srcpriline=&priority_bitmap->pix8(y,0);

		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.left();x<=cliprect.right();x++)
		{
			uint16_t pix1 = srcline1[x];
			uint16_t pix2 = srcline2[x];

			/* Here we have
			 pix1 - raw pixel / colour / priority data from first 1sdt chip
			 pix2 - raw pixel / colour / priority data from first 2nd chip
			*/

			int pri1, pri2;
			int spri1, spri2, alpha2;
			alpha2 = m_deco_ace->get_alpha((pix2 >> 4) & 0xf);

			// pix1 sprite vs pix2 sprite
			if (pix1 & 0x400)       // TODO - check only in pri mode 2??
				spri1 = 8;
			else
				spri1 = 32;

			// pix1 sprite vs playfield
			switch (priority)
			{
				case 0x01:
					{
						if ((pix1 & 0x600))
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;

				default:
					{
						if ((pix1 & 0x600) == 0x600)
							pri1 = 4;
						else if ((pix1 & 0x600) == 0x400)
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;
			}

			// pix2 sprite vs pix1 sprite
			if ((pix2 & 0x600) == 0x600)
				spri2 = 4;
			else if ((pix2 & 0x600))
				spri2 = 16;
			else
				spri2 = 64;

			// Transparency
			if (pix2 & 0x100)
				alpha2 = 0x80; // TODO : Uses 0x10-0x14 of Aceram?
				// alpha2 = m_deco_ace->get_alpha(0x14 + ((pix & 0xf0) > 0x50) ? 0x5 : ((pix2 >> 4) & 0x7)); This fixes explosion effects, but this is HACK.

			// pix2 sprite vs playfield
			switch (priority)
			{
				case 0x02:
					{
						// Additional sprite alpha in this mode
						if (pix2 & 0x400)
							alpha2 = 0x80; // TODO

						// Sprite vs playfield
						if ((pix2 & 0x600) == 0x600)
							pri2 = 4;
						else if ((pix2 & 0x600) == 0x400)
							pri2 = 16;
						else
							pri2 = 64;
					}
					break;

				default:
					{
						if ((pix2 & 0x400) == 0x400)
							pri2 = 16;
						else
							pri2 = 64;
					}
					break;
			}

			uint8_t bgpri = srcpriline[x];
			/* once we get here we have

			pri1 - 4/16/64 (sprite chip 1 pixel priority relative to bg)
			pri2 - 4/16/64 (sprite chip 2 pixel priority relative to bg)
			spri1 - 8/32 (priority of sprite chip 1 relative to other sprite chip)
			spri2 - 4/16/64 (priority of sprite chip 2 relative to other sprite chip)
			alpha2 - 0x80/0xff alpha level of sprite chip 2 pixels (0x80 if enabled, 0xff if not)

			bgpri - 0 / 8 / 32 (from drawing tilemaps earlier, to compare above pri1/pri2 priorities against)
			pix1 - same as before (ready to extract just colour data from)
			pix2 - same as before  ^^
			*/

			int drawnpixe1 = 0;
			if (pix1 & 0xf)
			{
				if (pri1 > bgpri)
				{
					dstline[x] = paldata[(pix1&0x1ff)+0x500];
					drawnpixe1 = 1;
				}
			}

			if (pix2 & 0xf)
			{
				if (pri2 > bgpri)
				{
					if ((!drawnpixe1) || (spri2 > spri1))
					{
						if (alpha2>=0xff)
						{
							dstline[x] = paldata[(pix2&0xff)+0x700];
						}
						else
						{
							uint32_t base = dstline[x];
							dstline[x] = alpha_blend_r32(base, paldata[(pix2&0xff)+0x700], alpha2);
						}
					}
				}
			}
		}
	}
}

uint32_t boogwing_state::screen_update_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t priority = m_priority;

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(!BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(!BIT(flip, 7));

	/* Draw sprite planes to bitmaps for later mixing */
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields */
	bitmap.fill(m_deco_ace->pen(0x400), cliprect); /* pen not confirmed */
	screen.priority().fill(0);

	// bit&0x8 is definitely some kind of palette effect
	// bit&0x4 combines playfields
	if ((priority & 0x7) == 0x5)
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x1 || (priority & 0x7) == 0x2)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 8);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x3)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 8);

		// This mode uses playfield 3 to shadow sprites & playfield 2 (instead of
		// regular alpha-blending, the destination is inverted).  Not yet implemented.
		// m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 32);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 8);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 32);
	}

	mix_boogwing(screen,bitmap,cliprect);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
