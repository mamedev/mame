// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "emu.h"
#include "includes/deco32.h"

/******************************************************************************/

void deco32_state::pri_w(u32 data)
{
	m_pri = data;
}

void dragngun_state::sprite_control_w(u32 data)
{
	m_sprite_ctrl = data;
}

void dragngun_state::spriteram_dma_w(u32 data)
{
	/* DMA spriteram to private sprite chip area, and clear cpu ram */
	m_spriteram->copy();
	memset(m_spriteram->live(),0,0x2000);
}

/******************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

void deco32_state::buffered_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	m_dirty_palette[offset] = 1;
}

void deco32_state::palette_dma_w(u32 data)
{
	for (int i = 0; i < m_palette->entries(); i++)
	{
		if (m_dirty_palette[i])
		{
			m_dirty_palette[i] = 0;

			const u8 b = (m_paletteram[i] >> 16) & 0xff;
			const u8 g = (m_paletteram[i] >>  8) & 0xff;
			const u8 r = (m_paletteram[i] >>  0) & 0xff;

			m_palette->set_pen_color(i, rgb_t(r, g, b));
		}
	}
}

/******************************************************************************/

void deco32_state::video_start()
{
	save_item(NAME(m_pri));
}

void deco32_state::allocate_spriteram(int chip)
{
	m_spriteram16[chip] = std::make_unique<u16[]>(0x2000/4);
	m_spriteram16_buffered[chip] = std::make_unique<u16[]>(0x2000/4);
	save_pointer(NAME(m_spriteram16[chip]), 0x2000/4, chip);
	save_pointer(NAME(m_spriteram16_buffered[chip]), 0x2000/4, chip);
}

void deco32_state::allocate_buffered_palette()
{
	m_dirty_palette = make_unique_clear<u8[]>(2048);
	save_pointer(NAME(m_dirty_palette), 2048);
}

void deco32_state::allocate_rowscroll(int size1, int size2, int size3, int size4)
{
	m_pf_rowscroll[0] = make_unique_clear<u16[]>(size1);
	m_pf_rowscroll[1] = make_unique_clear<u16[]>(size2);
	m_pf_rowscroll[2] = make_unique_clear<u16[]>(size3);
	m_pf_rowscroll[3] = make_unique_clear<u16[]>(size4);
	save_pointer(NAME(m_pf_rowscroll[0]), size1);
	save_pointer(NAME(m_pf_rowscroll[1]), size2);
	save_pointer(NAME(m_pf_rowscroll[2]), size3);
	save_pointer(NAME(m_pf_rowscroll[3]), size4);
}

void captaven_state::video_start()
{
	m_deco_tilegen[1]->set_pf1_8bpp_mode(1);
	deco32_state::allocate_spriteram(0);
	deco32_state::allocate_rowscroll(0x4000/4, 0x2000/4, 0x4000/4, 0x2000/4);
	deco32_state::video_start();
}

void fghthist_state::video_start()
{
	m_sprgen[0]->alloc_sprite_bitmap();
	deco32_state::allocate_spriteram(0);
	deco32_state::allocate_rowscroll(0x2000/4, 0x2000/4, 0x2000/4, 0x2000/4);
	deco32_state::allocate_buffered_palette();
	deco32_state::video_start();
}

void nslasher_state::video_start()
{
	const int width = m_screen->width();
	const int height = m_screen->height();
	m_tilemap_alpha_bitmap = std::make_unique<bitmap_ind16>(width, height);
	for (int chip = 0; chip < 2; chip++)
	{
		m_sprgen[chip]->alloc_sprite_bitmap();
		deco32_state::allocate_spriteram(chip);
	}
	deco32_state::allocate_rowscroll(0x2000/4, 0x2000/4, 0x2000/4, 0x2000/4);
	deco32_state::video_start();
}

void dragngun_state::video_start()
{
	//m_deco_tilegen[0]->set_pf1_8bpp_mode(1); // despite being 8bpp this doesn't require the same shifting as captaven, why not?
	m_screen->register_screen_bitmap(m_temp_render_bitmap);
	deco32_state::allocate_rowscroll(0x4000/4, 0x2000/4, 0x4000/4, 0x2000/4);
	deco32_state::allocate_buffered_palette();
	save_item(NAME(m_sprite_ctrl));
}
/******************************************************************************/


/******************************************************************************/

u32 captaven_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 flip = m_deco_tilegen[0]->pf_control_r(0);
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x000), cliprect); // Palette index not confirmed

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0].get(), m_pf_rowscroll[1].get());
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2].get(), m_pf_rowscroll[3].get());

	// pf4 not used (because pf3 is in 8bpp mode)

	if ((m_pri & 1) == 0)
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	}
	else
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	}

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);

	m_sprgen[0]->set_alt_format(true);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram16_buffered[0].get(), 0x400); // only low half of sprite ram is used?

	return 0;
}

u32 dragngun_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x400), cliprect); // Palette index not confirmed

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0].get(), m_pf_rowscroll[1].get());
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2].get(), m_pf_rowscroll[3].get());

	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 1); // it uses pf3 in 8bpp mode instead, like captaven
	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 8);

	// zooming sprite draw is very slow, and sprites are buffered.. however, one of the levels attempts to use
	// partial updates for every line, which causes things to be very slow... the sprites appear to support
	// multiple layers of alpha, so rendering to a buffer for layer isn't easy (maybe there are multiple sprite
	// chips at work?)
	//
	// really, it needs optimizing ..
	// so for now we only draw these 2 layers on the last update call
	if (cliprect.bottom() == 247)
	{
		rectangle clip(cliprect.left(), cliprect.right(), 8, 247);

		m_sprgenzoom->dragngun_draw_sprites(bitmap,clip,m_spriteram->buffer(), m_sprite_layout_ram[0], m_sprite_layout_ram[1], m_sprite_lookup_ram[0], m_sprite_lookup_ram[1], m_sprite_ctrl, screen.priority(), m_temp_render_bitmap);

	}

	return 0;
}


u32 fghthist_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x300), cliprect); // Palette index not confirmed

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0].get(), m_pf_rowscroll[1].get());
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2].get(), m_pf_rowscroll[3].get());

	// sprites are flipped relative to tilemaps
	m_sprgen[0]->set_flip_screen(true);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram16_buffered[0].get(), 0x800);

	/* Draw screen */
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);

	if (m_pri & 1)
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 1024, 0x1ff);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 1024, 0x1ff);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}

	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 1024, 0x1ff);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/*
    This function mimics the priority PROM/circuit on the pcb.  It takes
    the tilemaps & sprite bitmaps as inputs, and outputs a final pixel
    based on alpha & priority values.  Rendering sprites to temporary
    bitmaps is the only reasonable way to implement proper priority &
    blending support - it can't be done in-place on the final framebuffer
    without a lot of support bitmaps.
*/
void nslasher_state::mixDualAlphaSprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap)
{
	const pen_t *pens = m_deco_ace->pens();
	const pen_t *pal0 = &pens[gfx0->colorbase()];
	const pen_t *pal1 = &pens[gfx1->colorbase()];
	const pen_t *pal2 = &pens[m_gfxdecode->gfx((m_pri & 1) ? 1 : 2)->colorbase()];
	bitmap_ind16& sprite0_mix_bitmap = m_sprgen[0]->get_sprite_temp_bitmap();
	bitmap_ind16& sprite1_mix_bitmap = m_sprgen[1]->get_sprite_temp_bitmap();

	/* Mix sprites into main bitmap, based on priority & alpha */
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const u16* sprite0 = &sprite0_mix_bitmap.pix16(y);
		const u16* sprite1 = &sprite1_mix_bitmap.pix16(y);
		const u16* alphaTilemap = &m_tilemap_alpha_bitmap->pix16(y);
		const u8* tilemapPri = &screen.priority().pix8(y);
		u32* destLine = &bitmap.pix32(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const u16 priColAlphaPal0 = sprite0[x];
			const u16 priColAlphaPal1 = sprite1[x];
			const u16 pri0 = (priColAlphaPal0 & 0x6000) >> 13;
			const u16 pri1 = (priColAlphaPal1 & 0x6000) >> 13;
			const u16 col0 = (((priColAlphaPal0 & 0x1f00) >> 8) % gfx0->colors()) * gfx0->granularity();
			const u16 col1 = (((priColAlphaPal1 & 0x0f00) >> 8) % gfx1->colors()) * gfx1->granularity();
			const u16 alpha1 = priColAlphaPal1 & 0x8000;

			// Apply sprite bitmap 0 according to priority rules
			if ((priColAlphaPal0 & 0xff) != 0)
			{
				/*
				    Sprite 0 priority rules:

				    0 = Sprite above all layers
				    1 = Sprite under top playfield
				    2 = Sprite under top two playfields
				    3 = Sprite under all playfields
				*/
				if ((pri0 & 0x3) == 0 || (pri0 & 0x3) == 1 || ((pri0 & 0x3) == 2 && mixAlphaTilemap)) // Spri0 on top of everything, or under alpha playfield
				{
					destLine[x] = pal0[(priColAlphaPal0 & 0xff) + col0];
				}
				else if ((pri0 & 0x3) == 2) // Spri0 under top playfield
				{
					if (tilemapPri[x] < 4)
						destLine[x] = pal0[(priColAlphaPal0 & 0xff) + col0];
				}
				else // Spri0 under top & middle playfields
				{
					if (tilemapPri[x] < 2)
						destLine[x] = pal0[(priColAlphaPal0 & 0xff) + col0];
				}
			}

			// Apply sprite bitmap 1 according to priority rules
			if ((priColAlphaPal1 & 0xff) != 0)
			{
				// Apply alpha for this pixel based on Ace setting
				if (alpha1)
				{
					/*
					    Alpha rules:

					    Pri 0 - Over all tilemaps, but under sprite 0 pri 0, pri 1, pri 2
					    Pri 1 -
					    Pri 2 -
					    Pri 3 -
					*/

					/* Alpha values are tied to ACE ram... */
					//int alpha = m_deco_ace->get_alpha(((priColAlphaPal1 & 0xf0) >> 4) / 2);
					//if (alpha < 0)
					//  alpha = 0;

					/* I don't really understand how object ACE ram is really hooked up,
					    the only obvious place in Night Slashers is the stagecoach in level 2 */

					if (pri1 == 0 && (((priColAlphaPal0 & 0xff) == 0 || ((pri0 & 0x3) != 0 && (pri0 & 0x3) != 1 && (pri0 & 0x3) != 2))))
					{
						if ((m_pri & 1) == 0 || ((m_pri & 1) == 1 && tilemapPri[x] < 4) || ((m_pri & 1) == 1 && mixAlphaTilemap))
							destLine[x] = alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1 & 0xff) + col1], 0x80);
					}
					else if (pri1 == 1 && ((priColAlphaPal0 & 0xff) == 0 || ((pri0 & 0x3) != 0 && (pri0 & 0x3) != 1 && (pri0 & 0x3) != 2)))
						destLine[x] = alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1 & 0xff) + col1], 0x80);
					else if (pri1 == 2) // TOdo
						destLine[x] = alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1 & 0xff) + col1], 0x80);
					else if (pri1 == 3) // TOdo
						destLine[x] = alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1 & 0xff) + col1], 0x80);
				}
				else
				{
					/*
					    Non alpha rules:

					    Pri 0 - Under sprite 0 pri 0, over all tilemaps
					*/
					if (pri1 == 0 && ((priColAlphaPal0 & 0xff) == 0 || ((pri0 & 0x3) != 0)))
						destLine[x] = pal1[(priColAlphaPal1 & 0xff) + col1];
					else if (pri1 == 1) // todo
						destLine[x] = pal1[(priColAlphaPal1 & 0xff) + col1];
					else if (pri1 == 2) // todo
						destLine[x] = pal1[(priColAlphaPal1 & 0xff) + col1];
					else if (pri1 == 3) // todo
						destLine[x] = pal1[(priColAlphaPal1 & 0xff) + col1];
				}
			}

			/* Optionally mix in alpha tilemap */
			if (mixAlphaTilemap)
			{
				const u16 p = alphaTilemap[x];
				if (p & 0xf)
				{
					/* Alpha tilemap under top two sprite 0 priorities */
					if (((priColAlphaPal0 & 0xff) == 0 || (pri0 & 0x3) == 2 || (pri0 & 0x3) == 3)
						&& ((priColAlphaPal1 & 0xff) == 0 || (pri1 & 0x3) == 2 || (pri1 & 0x3) == 3 || alpha1))
					{
						/* Alpha values are tied to ACE ram */
						int alpha = m_deco_ace->get_alpha(0x17 + (((p & 0xf0) >> 4) / 2));
						if (alpha < 0)
							alpha = 0;

						destLine[x] = alpha_blend_r32(destLine[x], pal2[p], alpha);
					}
				}
			}
		}
	}
}

u32 nslasher_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool alphaTilemap = false;
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0].get(), m_pf_rowscroll[1].get());
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2].get(), m_pf_rowscroll[3].get());

	/* This is not a conclusive test for deciding if tilemap needs alpha blending */
	if (m_deco_ace->get_aceram(0x17) != 0x0 && m_pri)
		alphaTilemap = true;

	screen.priority().fill(0, cliprect);

	bitmap.fill(m_deco_ace->pen(0x200), cliprect);

	/* Draw sprites to temporary bitmaps, saving alpha & priority info for later mixing */
	m_sprgen[0]->set_pix_raw_shift(8);
	m_sprgen[1]->set_pix_raw_shift(8);

	// sprites are flipped relative to tilemaps
	m_sprgen[0]->set_flip_screen(true);
	m_sprgen[1]->set_flip_screen(true);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram16_buffered[0].get(), 0x800);
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram16_buffered[1].get(), 0x800);

	/* Render alpha-blended tilemap to separate buffer for proper mixing */
	m_tilemap_alpha_bitmap->fill(0, cliprect);

	/* Draw playfields & sprites */
	if (m_pri & 2)
	{
		m_deco_tilegen[1]->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 1, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);
		if (m_pri & 1)
		{
			m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
			if (alphaTilemap)
				m_deco_tilegen[1]->tilemap_1_draw(screen, *m_tilemap_alpha_bitmap, cliprect, 0, 4);
			else
				m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		}
		else
		{
			m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
			if (alphaTilemap)
				m_deco_tilegen[0]->tilemap_2_draw(screen, *m_tilemap_alpha_bitmap, cliprect, 0, 4);
			else
				m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
		}
	}

	mixDualAlphaSprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_gfxdecode->gfx(4), alphaTilemap);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
