// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/nycaptor.h"

#define NYCAPTOR_DEBUG  0

/*
 298 (e298) - spot (0-3) , 299 (e299) - lives
 spot number isn't set to 0 in main menu ; lives - yes
 sprites in main menu req priority 'type' 0
*/
int nycaptor_state::nycaptor_spot(  )
{
	if (m_gametype == 0 || m_gametype == 2)
		return m_sharedram[0x299] ? m_sharedram[0x298] : 0;
	else
		return 0;
}

TILE_GET_INFO_MEMBER(nycaptor_state::get_tile_info)
{
	int pal = m_videoram[tile_index * 2 + 1] & 0x0f;
	tileinfo.category = (m_videoram[tile_index * 2 + 1] & 0x30) >> 4;

	tileinfo.group = 0;

	if ((!nycaptor_spot()) && (pal == 6))
		tileinfo.group = 1;

	if (((nycaptor_spot() == 3) && (pal == 8)) || ((nycaptor_spot() == 1) && (pal == 0xc)))
		tileinfo.group = 2;

	if ((nycaptor_spot() == 1) && (tileinfo.category == 2))
		tileinfo.group = 3;

#if NYCAPTOR_DEBUG
	if (m_mask & (1 << tileinfo.category))
	{
		if (nycaptor_spot())
			pal = 0xe;
		else
			pal = 4;
	}
#endif

	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index * 2] + ((m_videoram[tile_index * 2 + 1] & 0xc0) << 2) + 0x400 * m_char_bank,
			pal, 0
			);
}


void nycaptor_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nycaptor_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32 );

	m_bg_tilemap->set_transmask(0, 0xf800, 0x7ff); //split 0
	m_bg_tilemap->set_transmask(1, 0xfe00, 0x01ff);//split 1
	m_bg_tilemap->set_transmask(2, 0xfffc, 0x0003);//split 2
	m_bg_tilemap->set_transmask(3, 0xfff0, 0x000f);//split 3
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram.resize(m_palette->entries());
	m_paletteram_ext.resize(m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext, ENDIANNESS_LITTLE, 1);

	save_item(NAME(m_paletteram));
	save_item(NAME(m_paletteram_ext));
}

WRITE8_MEMBER(nycaptor_state::nycaptor_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(nycaptor_state::nycaptor_palette_w)
{
	if (m_gametype == 2) //colt
		return;

	if (offset & 0x100)
		m_palette->write_ext(space, (offset & 0xff) + (m_palette_bank << 8), data);
	else
		m_palette->write(space, (offset & 0xff) + (m_palette_bank << 8), data);
}

READ8_MEMBER(nycaptor_state::nycaptor_palette_r)
{
	if (offset & 0x100)
		return m_paletteram_ext[(offset & 0xff) + (m_palette_bank << 8)];
	else
		return m_paletteram[(offset & 0xff) + (m_palette_bank << 8)];
}

WRITE8_MEMBER(nycaptor_state::nycaptor_gfxctrl_w)
{
	m_gfxctrl = data;

	if (m_char_bank != ((data & 0x18) >> 3))
	{
		m_char_bank = ((data & 0x18) >> 3);
		m_bg_tilemap->mark_all_dirty();
	}

	m_palette_bank = BIT(data, 5);

}

READ8_MEMBER(nycaptor_state::nycaptor_gfxctrl_r)
{
	return m_gfxctrl;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_scrlram_w)
{
	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}

void nycaptor_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = m_spriteram[0x9f - i];
		int offs = (pr & 0x1f) * 4;
		int code, sx, sy, flipx, flipy, pal, priori;

		code = m_spriteram[offs + 2] + ((m_spriteram[offs + 1] & 0x10) << 4);//1 bit wolny = 0x20
		pal  = m_spriteram[offs + 1] & 0x0f;
		sx   = m_spriteram[offs + 3];
		sy   = 240 - m_spriteram[offs + 0];
		priori = (pr & 0xe0) >> 5;

		if (priori == pri)
		{
#if NYCAPTOR_DEBUG
			if (m_mask & (1 << (pri + 4))) pal = 0xd;
#endif
			flipx = BIT(m_spriteram[offs + 1], 6);
			flipy = BIT(m_spriteram[offs + 1], 7);

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if (m_spriteram[offs + 3] > 240)
			{
				sx = (m_spriteram[offs + 3] - 256);
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					pal,
					flipx,flipy,
					sx,sy,15);
			}
		}
	}
}





#if NYCAPTOR_DEBUG
/*
 Keys :
   q/w/e/r - bg priority display select
   a/s/d/f/g/h/j/k - sprite priority display select
   z - clear
   x - no bg/sprite pri.
*/

#define mKEY_MASK(x,y) if (machine().input().code_pressed_once(x)) { m_mask |= y; m_bg_tilemap->mark_all_dirty(); }

void nycaptor_state::nycaptor_setmask(  )
{
	mKEY_MASK(KEYCODE_Q, 1); /* bg */
	mKEY_MASK(KEYCODE_W, 2);
	mKEY_MASK(KEYCODE_E, 4);
	mKEY_MASK(KEYCODE_R, 8);

	mKEY_MASK(KEYCODE_A, 0x10); /* sprites */
	mKEY_MASK(KEYCODE_S, 0x20);
	mKEY_MASK(KEYCODE_D, 0x40);
	mKEY_MASK(KEYCODE_F, 0x80);
	mKEY_MASK(KEYCODE_G, 0x100);
	mKEY_MASK(KEYCODE_H, 0x200);
	mKEY_MASK(KEYCODE_J, 0x400);
	mKEY_MASK(KEYCODE_K, 0x800);

	if (machine().input().code_pressed_once(KEYCODE_Z)){m_mask = 0; m_bg_tilemap->mark_all_dirty();} /* disable */
	if (machine().input().code_pressed_once(KEYCODE_X)){m_mask |= 0x1000; m_bg_tilemap->mark_all_dirty();} /* no layers */
}
#endif

UINT32 nycaptor_state::screen_update_nycaptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if NYCAPTOR_DEBUG
	nycaptor_setmask();
	if (m_mask & 0x1000)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		draw_sprites(bitmap, cliprect, 0);
		draw_sprites(bitmap, cliprect, 1);
		draw_sprites(bitmap, cliprect, 2);
		draw_sprites(bitmap, cliprect, 3);
		draw_sprites(bitmap, cliprect, 4);
		draw_sprites(bitmap, cliprect, 5);
		draw_sprites(bitmap, cliprect, 6);
		draw_sprites(bitmap, cliprect, 7);
	}
	else
#endif
	switch (nycaptor_spot() & 3)
	{
	case 0:
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		draw_sprites(bitmap, cliprect, 6);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(bitmap, cliprect, 3);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		draw_sprites(bitmap, cliprect, 0);
		draw_sprites(bitmap, cliprect, 2);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		draw_sprites(bitmap, cliprect, 1);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 1:
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		draw_sprites(bitmap, cliprect, 3);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		draw_sprites(bitmap, cliprect, 2);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(bitmap, cliprect, 1);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 2:
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(bitmap, cliprect, 1);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 3:
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(bitmap, cliprect, 1);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;
	}

	return 0;
}
