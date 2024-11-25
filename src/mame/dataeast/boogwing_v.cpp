// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "emu.h"
#include "boogwing.h"


void boogwing_state::video_start()
{
	m_priority = 0;
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
	m_screen->register_screen_bitmap(m_temp_bitmap);
	m_screen->register_screen_bitmap(m_alpha_tmap_bitmap);
	save_item(NAME(m_priority));
}

constexpr u32 sub_blend_r32(u32 d, u32 s, u8 level)
{
	// stage 1 boss for ragtime, inverts source layer for a shadow effect,
	// >> 9 instead of >> 8 is a guess
	s^=0xffffff;
	return ((((s & 0x0000ff) * level + (d & 0x0000ff) * int(256 - level)) >> 9)) |
			((((s & 0x00ff00) * level + (d & 0x00ff00) * int(256 - level)) >> 9) & 0x00ff00) |
			((((s & 0xff0000) * level + (d & 0xff0000) * int(256 - level)) >> 9) & 0xff0000);
}

/* Mix the 2 sprite planes with the already rendered tilemaps..
 note, if we implement tilemap blending etc. too we'll probably have to mix those in here as well..

 this is just a reimplementation of the old priority system used before conversion but to work with
 the bitmaps.  It could probably be simplified / improved greatly, along with the long-standing bugs
 fixed, with manual mixing you have full control.

 apparently priority is based on a PROM, that should be used if possible.

 Reference video : https://www.youtube.com/watch?v=mRdIlP_erBM (Live stream)
*/
void boogwing_state::mix_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = &m_deco_ace->pen(0);
	bitmap_ind16 *sprite_bitmap1, *sprite_bitmap2, *alpha_tmap_bitmap;
	bitmap_ind8* priority_bitmap;

	uint16_t priority = m_priority & 0x7;

	sprite_bitmap1 = &m_sprgen[0]->get_sprite_temp_bitmap();
	sprite_bitmap2 = &m_sprgen[1]->get_sprite_temp_bitmap();
	alpha_tmap_bitmap = &m_alpha_tmap_bitmap;
	priority_bitmap = &screen.priority();

	for (int y=cliprect.top();y<=cliprect.bottom();y++)
	{
		uint16_t *srcline1=&sprite_bitmap1->pix(y,0);
		uint16_t *srcline2=&sprite_bitmap2->pix(y,0);
		uint16_t *srcline3=&alpha_tmap_bitmap->pix(y,0);
		uint16_t *tmapline=&m_temp_bitmap.pix(y,0);
		uint8_t *srcpriline=&priority_bitmap->pix(y,0);

		uint32_t *dstline=&bitmap.pix(y,0);

		for (int x=cliprect.left();x<=cliprect.right();x++)
		{
			uint16_t pix1 = srcline1[x];
			uint16_t pix2 = srcline2[x];
			uint16_t pix3 = srcline3[x];
			uint16_t tmappix = tmapline[x];

			/* Here we have
			 pix1 - raw pixel / colour / priority data from first 1st chip
			 pix2 - raw pixel / colour / priority data from first 2nd chip
			 pix3 - raw pixel data from alpha blended tilemap
			*/

			int pri1, pri2, pri3 = 0;
			int spri1, spri2, alpha2, alpha3;
			alpha2 = m_deco_ace->get_alpha((pix2 >> 4) & 0xf);
			alpha3 = m_deco_ace->get_alpha(0x1f);

			// pix1 sprite vs pix2 sprite
			switch (priority)
			{
				// TODO - check only in pri mode 0/2??
				case 0x00:
					{
						if ((pix1 & 0x600) == 0x600)
							spri1 = 2;
						else if ((pix1 & 0x600) == 0x400)
							spri1 = 8;
						else
							spri1 = 32;
					}
					break;
				default:
					{
						if (pix1 & 0x400)
							spri1 = 8;
						else
							spri1 = 32;
					}
					break;
			}

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

				case 0x00:
					{
						if ((pix1 & 0x400) == 0x400)
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
			{
				if (pix2 & 0x800) // Use LUT, ex : Explosions
					alpha2 = (pix2 & 8) ? 0xff : m_deco_ace->get_alpha(0x14 + ((pix2 - 1) & 0x7));
				else
					alpha2 = m_deco_ace->get_alpha(0x10 + ((pix2 & 0x80) >> 7));
			}
			else if (pix2 & 0x800)
				alpha2 = m_deco_ace->get_alpha(0x12 + ((pix2 & 0x80) >> 7));

			// pix2 sprite vs playfield
			switch (priority)
			{
				case 0x02:
					// Sprite vs playfield
					if ((pix2 & 0x600) == 0x600)
						pri2 = 4;
					else if ((pix2 & 0x600) == 0x400)
						pri2 = 16;
					else
						pri2 = 64;
					break;

				case 0x03:
					pri3 = 32;
					[[fallthrough]];
				default:
					if ((pix2 & 0x400) == 0x400)
						pri2 = 16;
					else
						pri2 = 64;
					break;
			}

			uint8_t bgpri = srcpriline[x];
			/* once we get here we have

			pri1 - 4/16/64 (sprite chip 1 pixel priority relative to bg)
			pri2 - 4/16/64 (sprite chip 2 pixel priority relative to bg)
			spri1 - 2/8/32 (priority of sprite chip 1 relative to other sprite chip)
			spri2 - 4/16/64 (priority of sprite chip 2 relative to other sprite chip)
			alpha2 - 0x80/0xff alpha level of sprite chip 2 pixels (0x80 if enabled, 0xff if not)
			alpha3 - alpha level of alpha-blended playfield pixels

			bgpri - 0 / 8 / 32 (from drawing tilemaps earlier, to compare above pri1/pri2 priorities against)
			pix1 - same as before (ready to extract just colour data from)
			pix2 - same as before  ^^
			*/

			const u16 calculated_coloffs = (m_priority & 8) ? 0x800 : 0;
			int drawnpixe1 = 0;
			if (pix1 & 0xf)
			{
				if (pri1 > bgpri)
				{
					dstline[x] = paldata[calculated_coloffs | ((pix1&0x1ff)+0x500)];
					drawnpixe1 |= 1;
				}
			}

			if (pix2 & 0xf)
			{
				if ((drawnpixe1 == 0) && (tmappix != 0xffff))
					dstline[x] = paldata[calculated_coloffs | tmappix];

				if (pri2 > bgpri)
				{
					if ((!drawnpixe1) || (spri2 > spri1))
					{
						if (alpha2>=0xff)
						{
							dstline[x] = paldata[calculated_coloffs | ((pix2&0xff)+0x700)];
						}
						else
						{
							uint32_t base = dstline[x];
							dstline[x] = alpha_blend_r32(base, paldata[calculated_coloffs | ((pix2&0xff)+0x700)], alpha2);
						}
						drawnpixe1 |= 2;
					}
				}
			}

			if ((drawnpixe1 == 0) && (tmappix != 0xffff))
				dstline[x] = paldata[tmappix];

			// alpha blended tilemap handling
			if (pix3 & 0xf)
			{
				// TODO : sprite vs playfield priority, actual behavior of shadowing
				if (priority == 0x3)
				{
					bool bg2_drawed = (bgpri == 8) && (!drawnpixe1);
					bool sprite1_drawed = (drawnpixe1 & 1) && (pri1 <= pri3);
					bool sprite2_drawed = (drawnpixe1 & 2) && (pri2 <= pri3);
					if ((bg2_drawed) || ((sprite1_drawed && (~drawnpixe1 & 2)) || (sprite2_drawed && (~drawnpixe1 & 1)) || (sprite1_drawed && sprite2_drawed)))
					{
						if (((pix2 & 0x900) != 0x900) || ((spri2 <= spri1) && sprite1_drawed))
						{
							// TODO: make it functional, check out modes 0x21 and 0x1000.
							dstline[x] = (m_deco_ace->get_aceram(0x1f) == 0x22) ?
								sub_blend_r32(dstline[x], paldata[((drawnpixe1 & 3) ? calculated_coloffs : 0) | pix3], alpha3) :
								alpha_blend_r32(dstline[x], paldata[((drawnpixe1 & 3) ? calculated_coloffs : 0) | pix3], alpha3);
						}
					}
				}
			}
		}
	}
}

uint32_t boogwing_state::screen_update_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t const priority = m_priority;

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
	m_temp_bitmap.fill(0xffff, cliprect);
	screen.priority().fill(0);

	// bit&0x8 is definitely some kind of palette effect
	// bit&0x4 combines playfields
	if ((priority & 0x7) == 0x5)
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_12_combine_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x4)
	{
		m_deco_tilegen[1]->tilemap_12_combine_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x1 || (priority & 0x7) == 0x2)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 8);
		m_deco_tilegen[1]->tilemap_1_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x3)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 8);

		// This mode uses playfield 3 to shadow sprites & playfield 2 (instead of
		// regular alpha-blending, the destination is inverted).  Not yet implemented.
		m_alpha_tmap_bitmap.fill(0, cliprect);
		m_deco_tilegen[1]->tilemap_1_draw(screen, m_alpha_tmap_bitmap, cliprect, 0, 0); // 32
	}
	else
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_1_draw(screen, m_temp_bitmap, cliprect, 0, 8);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}

	mix_boogwing(screen,bitmap,cliprect);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
