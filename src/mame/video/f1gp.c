// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"

#include "includes/f1gp.h"


#define TOTAL_CHARS 0x800


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(f1gp_state::f1gp_get_roz_tile_info)
{
	int code = m_rozvideoram[tile_index];

	SET_TILE_INFO_MEMBER(3, code & 0x7ff, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(f1gp_state::f1gp2_get_roz_tile_info)
{
	int code = m_rozvideoram[tile_index];

	SET_TILE_INFO_MEMBER(2, (code & 0x7ff) + (m_roz_bank << 11), code >> 12, 0);
}

TILE_GET_INFO_MEMBER(f1gp_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code & 0x7fff, 0, (code & 0x8000) ? TILE_FLIPY : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/




VIDEO_START_MEMBER(f1gp_state,f1gp)
{
	m_roz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(f1gp_state::f1gp_get_roz_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(f1gp_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);

	m_zoomdata = (UINT16 *)memregion("gfx4")->base();
	m_gfxdecode->gfx(3)->set_source((UINT8 *)m_zoomdata);

	save_pointer(NAME(m_zoomdata), memregion("gfx4")->bytes()/2);
}


VIDEO_START_MEMBER(f1gp_state,f1gpb)
{
	m_roz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(f1gp_state::f1gp_get_roz_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(f1gp_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);

	m_zoomdata = (UINT16 *)memregion("gfx4")->base();
	m_gfxdecode->gfx(3)->set_source((UINT8 *)m_zoomdata);

	save_pointer(NAME(m_zoomdata), memregion("gfx4")->bytes()/2);
}

/* new hw type */
UINT32 f1gp_state::f1gp2_tile_callback( UINT32 code )
{
	return m_sprcgram[code&0x3fff];
}

/* old hw type */
UINT32 f1gp_state::f1gp_old_tile_callback( UINT32 code )
{
	return m_spr1cgram[code % (m_spr1cgram.bytes()/2)];
}

UINT32 f1gp_state::f1gp_ol2_tile_callback( UINT32 code )
{
	return m_spr2cgram[code % (m_spr2cgram.bytes()/2)];
}



VIDEO_START_MEMBER(f1gp_state,f1gp2)
{
	m_roz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(f1gp_state::f1gp2_get_roz_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(f1gp_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);
	m_roz_tilemap->set_transparent_pen(0x0f);

	m_fg_tilemap->set_scrolldx(-80, 0);
	m_fg_tilemap->set_scrolldy(-26, 0);

}


/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_MEMBER(f1gp_state::f1gp_zoomdata_r)
{
	return m_zoomdata[offset];
}

WRITE16_MEMBER(f1gp_state::f1gp_zoomdata_w)
{
	COMBINE_DATA(&m_zoomdata[offset]);
	m_gfxdecode->gfx(3)->mark_dirty(offset / 64);
}

READ16_MEMBER(f1gp_state::f1gp_rozvideoram_r)
{
	return m_rozvideoram[offset];
}

WRITE16_MEMBER(f1gp_state::f1gp_rozvideoram_w)
{
	COMBINE_DATA(&m_rozvideoram[offset]);
	m_roz_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(f1gp_state::f1gp_fgvideoram_w)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(f1gp_state::f1gp_fgscroll_w)
{
	COMBINE_DATA(&m_scroll[offset]);

	m_fg_tilemap->set_scrollx(0, m_scroll[0]);
	m_fg_tilemap->set_scrolly(0, m_scroll[1]);
}

WRITE16_MEMBER(f1gp_state::f1gp_gfxctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_flipscreen = data & 0x20;
		m_gfxctrl = data & 0xdf;
	}
}

WRITE16_MEMBER(f1gp_state::f1gp2_gfxctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_flipscreen = data & 0x20;

		/* bit 0/1 = fg/sprite/roz priority */
		/* bit 2 = blank screen */

		m_gfxctrl = data & 0xdf;
	}

	if (ACCESSING_BITS_8_15)
	{
		if (m_roz_bank != (data >> 8))
		{
			m_roz_bank = (data >> 8);
			m_roz_tilemap->mark_all_dirty();
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/


UINT32 f1gp_state::screen_update_f1gp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 1);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	/* quick kludge for "continue" screen priority */
	if (m_gfxctrl == 0x00)
	{
		m_spr_old->turbofrc_draw_sprites(m_spr1vram, m_spr1vram.bytes(),  0, bitmap, cliprect, screen.priority(), 0x02);
		m_spr_old2->turbofrc_draw_sprites(m_spr2vram, m_spr2vram.bytes(), 0, bitmap, cliprect, screen.priority(), 0x02);
	}
	else
	{
		m_spr_old->turbofrc_draw_sprites(m_spr1vram, m_spr1vram.bytes(), 0, bitmap, cliprect, screen.priority(), 0x00);
		m_spr_old2->turbofrc_draw_sprites(m_spr2vram, m_spr2vram.bytes(), 0, bitmap, cliprect, screen.priority(), 0x02);
	}
	return 0;
}


UINT32 f1gp_state::screen_update_f1gp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_gfxctrl & 4)  /* blank screen */
		bitmap.fill(m_palette->black_pen(), cliprect);
	else
	{
		switch (m_gfxctrl & 3)
		{
			case 0:
				m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				m_spr->draw_sprites(m_spritelist, 0x2000, screen, bitmap, cliprect);
				m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
				break;
			case 1:
				m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
				m_spr->draw_sprites(m_spritelist, 0x2000, screen, bitmap, cliprect);
				break;
			case 2:
				m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
				m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 1);
				m_spr->draw_sprites(m_spritelist, 0x2000, screen, bitmap, cliprect);
				break;
#ifdef MAME_DEBUG
			case 3:
				popmessage("unsupported priority 3\n");
#endif
		}
	}
	return 0;
}

/***************************************************************************

  BOOTLEG SUPPORT

***************************************************************************/

// BOOTLEG
void f1gp_state::f1gpb_draw_sprites( screen_device &screen,bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	UINT16 *spriteram = m_spriteram;
	int attr_start, start_offset = m_spriteram.bytes() / 2 - 4;

	// find the "end of list" to draw the sprites in reverse order
	for (attr_start = 4; attr_start < m_spriteram.bytes() / 2; attr_start += 4)
	{
		if (spriteram[attr_start + 3 - 4] == 0xffff) /* end of list marker */
		{
			start_offset = attr_start - 4;
			break;
		}
	}

	for (attr_start = start_offset;attr_start >= 4;attr_start -= 4)
	{
		int code, gfx;
		int x, y, flipx, flipy, color, pri;

		x = (spriteram[attr_start + 2] & 0x03ff) - 48;
		y = (256 - (spriteram[attr_start + 3 - 4] & 0x03ff)) - 15;
		flipx = spriteram[attr_start + 1] & 0x0800;
		flipy = spriteram[attr_start + 1] & 0x8000;
		color = spriteram[attr_start + 1] & 0x000f;
		code = spriteram[attr_start + 0] & 0x3fff;
		pri = 0; //?

		if((spriteram[attr_start + 1] & 0x00f0) && (spriteram[attr_start + 1] & 0x00f0) != 0xc0)
		{
			printf("attr %X\n",spriteram[attr_start + 1] & 0x00f0);
			code = machine().rand();
		}

/*
        if (spriteram[attr_start + 1] & ~0x88cf)
            printf("1 = %X\n", spriteram[attr_start + 1] & ~0x88cf);
*/
		if(code >= 0x2000)
		{
			gfx = 1;
			code -= 0x2000;
		}
		else
		{
			gfx = 0;
		}

		m_gfxdecode->gfx(1 + gfx)->prio_transpen(bitmap,cliprect,
			code,
			color,
			flipx,flipy,
			x,y,
			screen.priority(),
			pri ? 0 : 0x2,15);

		// wrap around x
		m_gfxdecode->gfx(1 + gfx)->prio_transpen(bitmap,cliprect,
			code,
			color,
			flipx,flipy,
			x - 512,y,
			screen.priority(),
			pri ? 0 : 0x2,15);
	}
}

// BOOTLEG
UINT32 f1gp_state::screen_update_f1gpb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	incxy = (INT16)m_rozregs[1];
	incyx = -incxy;
	incxx = incyy = (INT16)m_rozregs[3];
	startx = m_rozregs[0] + 328;
	starty = m_rozregs[2];

	m_fg_tilemap->set_scrolly(0, m_fgregs[0] + 8);

	screen.priority().fill(0, cliprect);

	m_roz_tilemap->draw_roz(screen, bitmap, cliprect,
		startx << 13, starty << 13,
		incxx << 5, incxy << 5, incyx << 5, incyy << 5,
		1, 0, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	f1gpb_draw_sprites(screen, bitmap, cliprect);

	return 0;
}
