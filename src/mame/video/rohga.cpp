// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Rohga Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/rohga.h"
#include "screen.h"


WRITE16_MEMBER(rohga_state::rohga_buffer_spriteram16_w)
{
	// Spriteram seems to be triple buffered (no sprite lag on real pcb, but there
	// is on driver with only double buffering)
	m_spriteram[0]->copy();
}

/******************************************************************************/

uint32_t rohga_state::screen_update_rohga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t priority = m_decocomn->priority_r();

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(!BIT(flip, 7));

	/* Update playfields */
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields */
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(768), cliprect);

	switch (priority & 3)
	{
	case 0:
		if (priority & 4)
		{
			// Draw as 1 8BPP layer
			m_deco_tilegen[1]->tilemap_12_combine_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 3);
		}
		else
		{
			// Draw as 2 4BPP layers
			m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
			m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		}
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
		break;
	case 1:
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		break;
	case 2:
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0/*TILEMAP_DRAW_OPAQUE*/, 1);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		break;
	}

	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



VIDEO_START_MEMBER(rohga_state,wizdfire)
{
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
}

// not amazingly efficient, called multiple times to pull a layer out of the sprite bitmaps, but keeps correct sprite<->sprite priorities
void rohga_state::mixwizdfirelayer(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask)
{
	int y, x;
	const pen_t *paldata = m_palette->pens();
	bitmap_ind16* sprite_bitmap;
	int penbase;

	sprite_bitmap = &m_sprgen[1]->get_sprite_temp_bitmap();
	penbase = 0x600;

	uint16_t* srcline;
	uint32_t* dstline;


	for (y=cliprect.top();y<=cliprect.bottom();y++)
	{
		srcline=&sprite_bitmap->pix16(y,0);
		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.left();x<=cliprect.right();x++)
		{
			uint16_t pix = srcline[x];

			if ((pix & primask) != pri)
				continue;

			if (pix&0xf)
			{
				uint16_t pen = pix&0x1ff;

				if (pen&0x100)
				{
					uint32_t base = dstline[x];
					pen &=0xff;
					dstline[x] = alpha_blend_r32(base, paldata[pen+penbase], 0x80);
				}
				else
				{
					dstline[x] = paldata[pen+penbase];
				}
			}
		}
	}
}

uint32_t rohga_state::screen_update_wizdfire(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t priority = m_decocomn->priority_r();

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(!BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(!BIT(flip, 7));

	/* draw sprite gfx to temp bitmaps */
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);

	/* Update playfields */
	m_deco_tilegen[0]->pf_update(nullptr, nullptr);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(m_palette->pen(512), cliprect);

	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0600, 0x0600, 0x400, 0x1ff);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0400, 0x0600, 0x400, 0x1ff);

	if ((priority & 0x1f) == 0x1f) /* Wizdfire has bit 0x40 always set, Dark Seal 2 doesn't?! */
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 0);
	else
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0400, 0x400, 0x1ff); // 0x000 and 0x200 of 0x600

	mixwizdfirelayer(bitmap, cliprect, 0x000, 0x000);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void rohga_state::mixnitroballlayer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y, x;
	const pen_t *paldata = &m_palette->pen(0);
	bitmap_ind16 *sprite_bitmap1, *sprite_bitmap2;
	bitmap_ind8* priority_bitmap;

	uint16_t priority = m_decocomn->priority_r();

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
			 pix1 - raw pixel / colour / priority data from first 1st chip
			 pix2 - raw pixel / colour / priority data from first 2nd chip
			*/

			int pri1, pri2;

			// pix1 sprite vs playfield
			switch (priority) // TODO : Verify this from real pcb
			{
				case 0x00:
				default:
					{
						switch (pix1 & 0xe00)
						{
							case 0x000:
							default:
								pri1 = 0x200;
								break;
							case 0x200:
								pri1 = 0x020;
								break;
							case 0x400:
								pri1 = 0x008;
								break;
							case 0x600:
								pri1 = 0x002;
								break;
							case 0x800:
								pri1 = 0x100;
								break;
							case 0xa00:
								pri1 = 0x040;
								break;
							case 0xc00:
								pri1 = 0x004;
								break;
							case 0xe00:
								pri1 = 0x001;
								break;
						}
					}
					break;
				case 0x20:
					{
						switch (pix1 & 0xe00)
						{
							case 0x000:
							default:
								pri1 = 0x080;
								break;
							case 0x200:
								pri1 = 0x004;
								break;
							case 0x400:
								pri1 = 0x002;
								break;
							case 0x600:
								pri1 = 0x001;
								break;
							case 0x800:
								pri1 = 0x100;
								break;
							case 0xa00:
								pri1 = 0x020;
								break;
							case 0xc00:
								pri1 = 0x008;
								break;
							case 0xe00:
								pri1 = 0x200;
								break;
						}
					}
					break;
			}

			// pix2 sprite vs pix1 sprite
			pri2 = 0x080;
			switch (priority)
			{
				case 0x00:
				default:
					pri2 = 0x080;
					break;
				case 0x20:
					pri2 = 0x010;
					break;
			}

			uint8_t bgpri = srcpriline[x];
			/* once we get here we have

			pri1 - 0x001/0x002/0x004/0x008/0x010/0x020/0x040/0x080/0x100/0x200 (sprite chip 1 pixel priority relative to bg)
			pri2 - 0x080/0x010 (sprite chip 2 pixel priority relative to bg)

			bgpri - 0x008/0x040 (from drawing tilemaps earlier, to compare above pri1/pri2 priorities against)
			pix1 - same as before (ready to extract just colour data from)
			pix2 - same as before  ^^
			*/

			int drawnpixe1 = 0;
			if (pix1 & 0xf)
			{
				if (pri1 > bgpri)
				{
					dstline[x] = paldata[(pix1&0x1ff)+0x400];
					drawnpixe1 = 1;
				}
			}

			if (pix2 & 0xf)
			{
				if (pri2 > bgpri)
				{
					if ((!drawnpixe1) || (pri2 > pri1))
					{
						if (pix2 & 0x100)
						{
							uint32_t base = dstline[x];
							dstline[x] = alpha_blend_r32(base, paldata[(pix2&0xff)+0x600], 0x80);
						}
						else
						{
							dstline[x] = paldata[(pix2&0xff)+0x600];
						}
					}
				}
			}
		}
	}
}

uint32_t rohga_state::screen_update_nitrobal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t priority = m_decocomn->priority_r();

	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(BIT(flip, 7));

	/* draw sprite gfx to temp bitmaps */
	m_sprgen[0]->set_alt_format(true);
	m_sprgen[1]->set_alt_format(true);
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);

	/* Update playfields */
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(m_palette->pen(512), cliprect);
	screen.priority().fill(0);

	/* pf3 and pf4 are combined into a single 8bpp bitmap */
	m_deco_tilegen[1]->tilemap_12_combine_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	switch (priority)
	{
		case 0:
		default:
			m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0x008);
			break;
		case 0x20:
			m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0x040);
			break;
	}

	/* TODO verify priorities + mixing / alpha */
	mixnitroballlayer(screen,bitmap,cliprect);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
