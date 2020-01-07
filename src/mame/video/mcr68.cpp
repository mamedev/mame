// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway MCR-68k system

***************************************************************************/

#include "emu.h"
#include "includes/mcr68.h"


#define LOW_BYTE(x) ((x) & 0xff)


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(mcr68_state::get_bg_tile_info)
{
	uint16_t *videoram = m_videoram;
	int data = LOW_BYTE(videoram[tile_index * 2]) | (LOW_BYTE(videoram[tile_index * 2 + 1]) << 8);
	int code = (data & 0x3ff) | ((data >> 4) & 0xc00);
	int color = (~data >> 12) & 3;
	SET_TILE_INFO_MEMBER(0, code, color, TILE_FLIPYX(data >> 10));
	if (m_gfxdecode->gfx(0)->elements() < 0x1000)
		tileinfo.category = (data >> 15) & 1;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(mcr68_state,mcr68)
{
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mcr68_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_bg_tilemap->set_transparent_pen(0);
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE16_MEMBER(mcr68_state::mcr68_videoram_w)
{
	uint16_t *videoram = m_videoram;
	COMBINE_DATA(&videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



/*************************************
 *
 *  Sprite update
 *
 *************************************/

void mcr68_state::mcr68_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	rectangle sprite_clip = m_screen->visible_area();
	uint16_t *spriteram = m_spriteram;
	int offs;

	/* adjust for clipping */
	sprite_clip.min_x += m_sprite_clip;
	sprite_clip.max_x -= m_sprite_clip;
	sprite_clip &= cliprect;

	screen.priority().fill(1, sprite_clip);

	/* loop over sprite RAM */
	for (offs = m_spriteram.bytes() / 2 - 4;offs >= 0;offs -= 4)
	{
		int code, color, flipx, flipy, x, y, flags;

		flags = LOW_BYTE(spriteram[offs + 1]);
		code = LOW_BYTE(spriteram[offs + 2]) + 256 * ((flags >> 3) & 0x01) + 512 * ((flags >> 6) & 0x03);

		/* skip if zero */
		if (code == 0)
			continue;

		/* also skip if this isn't the priority we're drawing right now */
		if (((flags >> 2) & 1) != priority)
			continue;

		/* extract the bits of information */
		color = ~flags & 0x03;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		x = LOW_BYTE(spriteram[offs + 3]) * 2 + m_sprite_xoffset;
		y = (241 - LOW_BYTE(spriteram[offs])) * 2;

		/* allow sprites to clip off the left side */
		if (x > 0x1f0) x -= 0x200;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
		    The color 8 is used to cover over other sprites. */

		/* first draw the sprite, visible */
		m_gfxdecode->gfx(1)->prio_transmask(bitmap,sprite_clip, code, color, flipx, flipy, x, y,
				screen.priority(), 0x00, 0x0101);

		/* then draw the mask, behind the background but obscuring following sprites */
		m_gfxdecode->gfx(1)->prio_transmask(bitmap,sprite_clip, code, color, flipx, flipy, x, y,
				screen.priority(), 0x02, 0xfeff);
	}
}



/*************************************
 *
 *  General MCR/68k update
 *
 *************************************/

uint32_t mcr68_state::screen_update_mcr68(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw the background */
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	/* draw the low-priority sprites */
	mcr68_update_sprites(screen, bitmap, cliprect, 0);

	/* redraw tiles with priority over sprites */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	/* draw the high-priority sprites */
	mcr68_update_sprites(screen, bitmap, cliprect, 1);
	return 0;
}
