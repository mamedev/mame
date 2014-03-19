/* Aquarium */

#include "emu.h"
#include "includes/aquarium.h"

/* gcpinbal.c modified */
void aquarium_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs )
{
	int offs, chain_pos;
	int x, y, curx, cury;
	UINT8 col, flipx, flipy, chain;
	UINT16 code;

	for (offs = 0; offs < m_spriteram.bytes() / 2; offs += 8)
	{
		code = ((m_spriteram[offs + 5]) & 0xff) + (((m_spriteram[offs + 6]) & 0xff) << 8);
		code &= 0x3fff;

		if (!(m_spriteram[offs + 4] &0x80))  /* active sprite ? */
		{
			x = ((m_spriteram[offs + 0]) &0xff) + (((m_spriteram[offs + 1]) & 0xff) << 8);
			y = ((m_spriteram[offs + 2]) &0xff) + (((m_spriteram[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col = ((m_spriteram[offs + 7]) & 0x0f);
			chain = (m_spriteram[offs + 4]) & 0x07;
			flipy = (m_spriteram[offs + 4]) & 0x10;
			flipx = (m_spriteram[offs + 4]) & 0x20;

			curx = x;
			cury = y;

			if (((m_spriteram[offs + 4]) & 0x08) && flipy)
				cury += (chain * 16);

			if (!(((m_spriteram[offs + 4]) & 0x08)) && flipx)
				curx += (chain * 16);


			for (chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						col,
						flipx, flipy,
						curx,cury,0);

				/* wrap around y */
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						col,
						flipx, flipy,
						curx,cury + 256,0);

				code++;

				if ((m_spriteram[offs + 4]) &0x08)   /* Y chain */
				{
					if (flipy)
						cury -= 16;
					else
						cury += 16;
				}
				else    /* X chain */
				{
					if (flipx)
						curx -= 16;
					else
						curx += 16;
				}
			}
		}
	}
#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf, "sprite rotate offs %04x ?", rotate);
		popmessage(buf);
	}
#endif
}

/* TXT Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_aquarium_txt_tile_info)
{
	int tileno, colour;

	tileno = (m_txt_videoram[tile_index] & 0x0fff);
	colour = (m_txt_videoram[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO_MEMBER(2, tileno, colour, 0);
}

WRITE16_MEMBER(aquarium_state::aquarium_txt_videoram_w)
{
	m_txt_videoram[offset] = data;
	m_txt_tilemap->mark_tile_dirty(offset);
}

/* MID Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_aquarium_mid_tile_info)
{
	int tileno, colour, flag;

	tileno = (m_mid_videoram[tile_index * 2] & 0x0fff);
	colour = (m_mid_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((m_mid_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO_MEMBER(1, tileno, colour, flag);

	tileinfo.category = (m_mid_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_MEMBER(aquarium_state::aquarium_mid_videoram_w)
{
	m_mid_videoram[offset] = data;
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}

/* BAK Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_aquarium_bak_tile_info)
{
	int tileno, colour, flag;

	tileno = (m_bak_videoram[tile_index * 2] & 0x0fff);
	colour = (m_bak_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((m_bak_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO_MEMBER(3, tileno, colour, flag);

	tileinfo.category = (m_bak_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_MEMBER(aquarium_state::aquarium_bak_videoram_w)
{
	m_bak_videoram[offset] = data;
	m_bak_tilemap->mark_tile_dirty(offset / 2);
}

void aquarium_state::video_start()
{
	m_txt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarium_state::get_aquarium_txt_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_bak_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarium_state::get_aquarium_bak_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_mid_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarium_state::get_aquarium_mid_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_txt_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
}

UINT32 aquarium_state::screen_update_aquarium(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mid_tilemap->set_scrollx(0, m_scroll[0]);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);
	m_bak_tilemap->set_scrollx(0, m_scroll[2]);
	m_bak_tilemap->set_scrolly(0, m_scroll[3]);
	m_txt_tilemap->set_scrollx(0, m_scroll[4]);
	m_txt_tilemap->set_scrolly(0, m_scroll[5]);

	m_bak_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect, 16);

	m_bak_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
