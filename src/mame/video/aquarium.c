/* Aquarium */

#include "emu.h"
#include "includes/aquarium.h"

/* gcpinbal.c modified */
static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs )
{
	aquarium_state *state = machine.driver_data<aquarium_state>();
	int offs, chain_pos;
	int x, y, curx, cury;
	UINT8 col, flipx, flipy, chain;
	UINT16 code;

	for (offs = 0; offs < state->m_spriteram_size / 2; offs += 8)
	{
		code = ((state->m_spriteram[offs + 5]) & 0xff) + (((state->m_spriteram[offs + 6]) & 0xff) << 8);
		code &= 0x3fff;

		if (!(state->m_spriteram[offs + 4] &0x80))	/* active sprite ? */
		{
			x = ((state->m_spriteram[offs + 0]) &0xff) + (((state->m_spriteram[offs + 1]) & 0xff) << 8);
			y = ((state->m_spriteram[offs + 2]) &0xff) + (((state->m_spriteram[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col = ((state->m_spriteram[offs + 7]) & 0x0f);
			chain = (state->m_spriteram[offs + 4]) & 0x07;
			flipy = (state->m_spriteram[offs + 4]) & 0x10;
			flipx = (state->m_spriteram[offs + 4]) & 0x20;

			curx = x;
			cury = y;

			if (((state->m_spriteram[offs + 4]) & 0x08) && flipy)
				cury += (chain * 16);

			if (!(((state->m_spriteram[offs + 4]) & 0x08)) && flipx)
				curx += (chain * 16);


			for (chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				drawgfx_transpen(bitmap, cliprect,machine.gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury,0);

				/* wrap around y */
				drawgfx_transpen(bitmap, cliprect,machine.gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury + 256,0);

				code++;

				if ((state->m_spriteram[offs + 4]) &0x08)	/* Y chain */
				{
					if (flipy)
						cury -= 16;
					else
						cury += 16;
				}
				else	/* X chain */
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
static TILE_GET_INFO( get_aquarium_txt_tile_info )
{
	aquarium_state *state = machine.driver_data<aquarium_state>();
	int tileno, colour;

	tileno = (state->m_txt_videoram[tile_index] & 0x0fff);
	colour = (state->m_txt_videoram[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO(2, tileno, colour, 0);
}

WRITE16_MEMBER(aquarium_state::aquarium_txt_videoram_w)
{
	m_txt_videoram[offset] = data;
	m_txt_tilemap->mark_tile_dirty(offset);
}

/* MID Layer */
static TILE_GET_INFO( get_aquarium_mid_tile_info )
{
	aquarium_state *state = machine.driver_data<aquarium_state>();
	int tileno, colour, flag;

	tileno = (state->m_mid_videoram[tile_index * 2] & 0x0fff);
	colour = (state->m_mid_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((state->m_mid_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO(1, tileno, colour, flag);

	tileinfo.category = (state->m_mid_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_MEMBER(aquarium_state::aquarium_mid_videoram_w)
{
	m_mid_videoram[offset] = data;
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}

/* BAK Layer */
static TILE_GET_INFO( get_aquarium_bak_tile_info )
{
	aquarium_state *state = machine.driver_data<aquarium_state>();
	int tileno, colour, flag;

	tileno = (state->m_bak_videoram[tile_index * 2] & 0x0fff);
	colour = (state->m_bak_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((state->m_bak_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO(3, tileno, colour, flag);

	tileinfo.category = (state->m_bak_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_MEMBER(aquarium_state::aquarium_bak_videoram_w)
{
	m_bak_videoram[offset] = data;
	m_bak_tilemap->mark_tile_dirty(offset / 2);
}

VIDEO_START(aquarium)
{
	aquarium_state *state = machine.driver_data<aquarium_state>();
	state->m_txt_tilemap = tilemap_create(machine, get_aquarium_txt_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_bak_tilemap = tilemap_create(machine, get_aquarium_bak_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_mid_tilemap = tilemap_create(machine, get_aquarium_mid_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_txt_tilemap->set_transparent_pen(0);
	state->m_mid_tilemap->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16(aquarium)
{
	aquarium_state *state = screen.machine().driver_data<aquarium_state>();
	state->m_mid_tilemap->set_scrollx(0, state->m_scroll[0]);
	state->m_mid_tilemap->set_scrolly(0, state->m_scroll[1]);
	state->m_bak_tilemap->set_scrollx(0, state->m_scroll[2]);
	state->m_bak_tilemap->set_scrolly(0, state->m_scroll[3]);
	state->m_txt_tilemap->set_scrollx(0, state->m_scroll[4]);
	state->m_txt_tilemap->set_scrolly(0, state->m_scroll[5]);

	state->m_bak_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_mid_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect, 16);

	state->m_bak_tilemap->draw(bitmap, cliprect, 1, 0);
	state->m_mid_tilemap->draw(bitmap, cliprect, 1, 0);
	state->m_txt_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
