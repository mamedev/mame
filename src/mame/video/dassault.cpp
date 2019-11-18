// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Desert Assault Video emulation - Bryan McPhail, mish@tendril.co.uk

  I'm not sure if one of the alpha blending effects is correct (mode 0x8000,
  the usual mode 0x4000 should be correct).  It may be some kind of orthogonal
  priority effect where it should cut a hole in other higher priority sprites
  to reveal a non-alpha'd hole, or alpha against a further back tilemap.
  (is this the helicopter shadow at the end of lv.1 ?)

  Also, some priorities are still a little questionable.


****************************************************************************/

#include "emu.h"
#include "includes/dassault.h"
#include "screen.h"

/******************************************************************************/

void dassault_state::video_start()
{
	m_priority = 0;
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
	save_item(NAME(m_priority));
}

void dassault_state::mixdassaultlayer(bitmap_rgb32 &bitmap, bitmap_ind16* sprite_bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask, uint16_t penbase, uint8_t alpha)
{
	int y, x;
	const pen_t *paldata = &m_palette->pen(0);

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
				if (pix & 0x800) pen += 0x200;

				if (alpha!=0xff)
				{
					if (pix&0x400) // TODO, Additive/Subtractive Blending?
					{
						uint32_t base = dstline[x];
						dstline[x] = alpha_blend_r32(base, paldata[pen+penbase], alpha);
					}
					else if (pix&0x200)
					{
						uint32_t base = dstline[x];
						dstline[x] = alpha_blend_r32(base, paldata[pen+penbase], alpha);
					}
					else
					{
						dstline[x] = paldata[pen+penbase];
					}
				}
				else
				{
					dstline[x] = paldata[pen+penbase];
				}
			}
		}
	}
}

/* are the priorities 100% correct? they're the same as they were before conversion to DECO52 sprite device, but if (for example) you walk to the side of the crates in the first part of the game you appear over them... */
uint32_t dassault_state::screen_update_dassault(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t priority = m_priority;

	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(BIT(flip, 7));

	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);
	bitmap_ind16* sprite_bitmap1 = &m_sprgen[0]->get_sprite_temp_bitmap();
	bitmap_ind16* sprite_bitmap2 = &m_sprgen[1]->get_sprite_temp_bitmap();

	/* Update tilemaps */
	m_deco_tilegen[0]->pf_update(nullptr, m_pf2_rowscroll);
	m_deco_tilegen[1]->pf_update(nullptr, m_pf4_rowscroll);

	/* Draw playfields/update priority bitmap */
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(3072), cliprect);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	/* The middle playfields can be swapped priority-wise */
	if ((priority & 3) == 0)
	{
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0600, 0x0600,  0x400, 0xff); // 1
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2); // 2
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0400, 0x0600,  0x400, 0xff); // 8
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 16); // 16
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0200, 0x0600,  0x400, 0xff); // 32
		mixdassaultlayer(bitmap, sprite_bitmap2, cliprect,  0x0000, 0x0000,  0x800, 0x80); // 64?
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0000, 0x0600,  0x400, 0xff); // 128

	}
	else if ((priority & 3) == 1)
	{
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0600, 0x0600,  0x400, 0xff); // 1
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2); // 2
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0400, 0x0600,  0x400, 0xff); // 8
		mixdassaultlayer(bitmap, sprite_bitmap2, cliprect,  0x0000, 0x0000,  0x800, 0x80); // 16?
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0200, 0x0600,  0x400, 0xff); // 32
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 64); // 64
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0000, 0x0600,  0x400, 0xff); // 128
	}
	else if ((priority & 3) == 3)
	{
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0600, 0x0600,  0x400, 0xff); // 1
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2); // 2
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0400, 0x0600,  0x400, 0xff); // 8
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 16); // 16
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0200, 0x0600,  0x400, 0xff); // 32
		mixdassaultlayer(bitmap, sprite_bitmap2, cliprect,  0x0000, 0x0000,  0x800, 0x80); // 64?
		mixdassaultlayer(bitmap, sprite_bitmap1, cliprect,  0x0000, 0x0600,  0x400, 0xff); // 128
	}
	else
	{
		/* Unused */
	}

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
