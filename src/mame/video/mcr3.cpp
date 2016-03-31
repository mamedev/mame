// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway MCR-III system

***************************************************************************/

#include "emu.h"
#include "includes/mcr.h"
#include "includes/mcr3.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

#ifdef UNUSED_FUNCTION
TILE_GET_INFO_MEMBER(mcr3_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int data = videoram[tile_index * 2] | (videoram[tile_index * 2 + 1] << 8);
	int code = (data & 0x3ff) | ((data >> 4) & 0x400);
	int color = (data >> 12) & 3;
	SET_TILE_INFO_MEMBER(0, code, color, TILE_FLIPYX((data >> 10) & 3));
}
#endif


TILE_GET_INFO_MEMBER(mcr3_state::mcrmono_get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int data = videoram[tile_index * 2] | (videoram[tile_index * 2 + 1] << 8);
	int code = (data & 0x3ff) | ((data >> 4) & 0x400);
	int color = ((data >> 12) & 3) ^ 3;
	SET_TILE_INFO_MEMBER(0, code, color, TILE_FLIPYX(data >> 10));
}


TILEMAP_MAPPER_MEMBER(mcr3_state::spyhunt_bg_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) | ((col & 0x3f) << 4) | ((row & 0x10) << 6);
}


TILE_GET_INFO_MEMBER(mcr3_state::spyhunt_get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int data = videoram[tile_index];
	int code = (data & 0x3f) | ((data >> 1) & 0x40);
	SET_TILE_INFO_MEMBER(0, code, 0, (data & 0x40) ? TILE_FLIPY : 0);
}


TILE_GET_INFO_MEMBER(mcr3_state::spyhunt_get_alpha_tile_info)
{
	SET_TILE_INFO_MEMBER(2, m_spyhunt_alpharam[tile_index], 0, 0);
}



/*************************************
 *
 *  Spy Hunter-specific palette init
 *
 *************************************/

PALETTE_INIT_MEMBER(mcr3_state,spyhunt)
{
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		palette.set_pen_color(i,rgb_t::black); /* black */
	}

	/* alpha colors are hard-coded */
	palette.set_pen_color(4*16+0,rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(4*16+1,rgb_t(0x00,0xff,0x00));
	palette.set_pen_color(4*16+2,rgb_t(0x00,0x00,0xff));
	palette.set_pen_color(4*16+3,rgb_t(0xff,0xff,0xff));
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

#ifdef UNUSED_FUNCTION
VIDEO_START_MEMBER(mcr3_state,mcr3)
{
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mcr3_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  16,16, 32,30);
}
#endif


VIDEO_START_MEMBER(mcr3_state,mcrmono)
{
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mcr3_state::mcrmono_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  16,16, 32,30);
}


VIDEO_START_MEMBER(mcr3_state,spyhunt)
{
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mcr3_state::spyhunt_get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(mcr3_state::spyhunt_bg_scan),this),  64,32, 64,32);

	/* initialize the text tilemap */
	m_alpha_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mcr3_state::spyhunt_get_alpha_tile_info),this), TILEMAP_SCAN_COLS,  16,16, 32,32);
	m_alpha_tilemap->set_transparent_pen(0);
	m_alpha_tilemap->set_scrollx(0, 16);

	save_item(NAME(m_spyhunt_sprite_color_mask));
	save_item(NAME(m_spyhunt_scrollx));
	save_item(NAME(m_spyhunt_scrolly));
	save_item(NAME(m_spyhunt_scroll_offset));
}





/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_MEMBER(mcr3_state::mcr3_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


WRITE8_MEMBER(mcr3_state::spyhunt_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(mcr3_state::spyhunt_alpharam_w)
{
	m_spyhunt_alpharam[offset] = data;
	m_alpha_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(mcr3_state::spyhunt_scroll_value_w)
{
	switch (offset)
	{
		case 0:
			/* low 8 bits of horizontal scroll */
			m_spyhunt_scrollx = (m_spyhunt_scrollx & ~0xff) | data;
			break;

		case 1:
			/* upper 3 bits of horizontal scroll and upper 1 bit of vertical scroll */
			m_spyhunt_scrollx = (m_spyhunt_scrollx & 0xff) | ((data & 0x07) << 8);
			m_spyhunt_scrolly = (m_spyhunt_scrolly & 0xff) | ((data & 0x80) << 1);
			break;

		case 2:
			/* low 8 bits of vertical scroll */
			m_spyhunt_scrolly = (m_spyhunt_scrolly & ~0xff) | data;
			break;
	}
}



/*************************************
 *
 *  Sprite update
 *
 *************************************/

void mcr3_state::mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	m_screen->priority().fill(1, cliprect);

	/* loop over sprite RAM */
	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code, color, flipx, flipy, sx, sy, flags;

		/* skip if zero */
		if (spriteram[offs] == 0)
			continue;

/*
    monoboard:
        flags.d0 -> ICG0~ -> PCG0~/PCG2~/PCG4~/PCG6~ -> bit 4 of linebuffer
        flags.d1 -> ICG1~ -> PCG1~/PCG3~/PCG5~/PCG7~ -> bit 5 of linebuffer
        flags.d2 -> IPPR  -> PPR0 /PPR1 /PPR2 /PPR3  -> bit 6 of linebuffer
        flags.d3 -> IRA15 ----------------------------> address line 15 of FG ROMs
        flags.d4 -> HFLIP
        flags.d5 -> VFLIP

*/

		/* extract the bits of information */
		flags = spriteram[offs + 1];
		code = spriteram[offs + 2] + 256 * ((flags >> 3) & 0x01);
		color = ~flags & color_mask;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		sx = (spriteram[offs + 3] - 3) * 2;
		sy = (241 - spriteram[offs]);

		if (interlaced == 1) sy *= 2;

		code ^= code_xor;

		sx += dx;
		sy += dy;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
		    The color 8 is used to cover over other sprites. */
		if (!mcr_cocktail_flip)
		{
			/* first draw the sprite, visible */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
					screen.priority(), 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
					screen.priority(), 0x02, 0xfeff);
		}
		else
		{
			/* first draw the sprite, visible */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					screen.priority(), 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					screen.priority(), 0x02, 0xfeff);
		}
	}
}



/*************************************
 *
 *  Generic MCR3 redraw
 *
 *************************************/

UINT32 mcr3_state::screen_update_mcr3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* update the flip state */
	m_bg_tilemap->set_flip(mcr_cocktail_flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

	/* draw the background */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	mcr3_update_sprites(screen, bitmap, cliprect, 0x03, 0, 0, 0, 1);
	return 0;
}


UINT32 mcr3_state::screen_update_spyhunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	m_bg_tilemap->set_scrollx(0, m_spyhunt_scrollx * 2 + m_spyhunt_scroll_offset);
	m_bg_tilemap->set_scrolly(0, m_spyhunt_scrolly * 2);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	mcr3_update_sprites(screen, bitmap, cliprect, m_spyhunt_sprite_color_mask, 0, -12, 0, 1);

	/* render any characters on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
