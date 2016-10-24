// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Andrea Mazzoleni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/retofinv.h"


void retofinv_state::palette_init_retofinv(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;


	/* fg chars (1bpp) */
	for (i = 0; i < 0x200; i++)
	{
		uint8_t ctabentry;

		if (i & 0x01)
			ctabentry = i >> 1;
		else
			ctabentry = 0;

		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites and bg tiles */
	for (i = 0; i < 0x800; i++)
	{
		uint8_t ctabentry = BITSWAP8(color_prom[i],4,5,6,7,3,2,1,0);
		palette.set_pen_indirect(i + 0x200, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
tilemap_memory_index retofinv_state::tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows)
{
	row += 2;
	col -= 2;
	if (col  & 0x20)
		return ((col & 0x1f) << 5) + row;
	else
		return (row << 5) + col;
}

void retofinv_state::bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	SET_TILE_INFO_MEMBER(2,
			m_bg_videoram[tile_index] + 256 * m_bg_bank,
			m_bg_videoram[0x400 + tile_index] & 0x3f,
			0);
}

void retofinv_state::fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int color = m_fg_videoram[0x400 + tile_index];

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(0,
			m_fg_videoram[tile_index] + 256 * m_fg_bank,
			color,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void retofinv_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(retofinv_state::bg_get_tile_info),this),tilemap_mapper_delegate(FUNC(retofinv_state::tilemap_scan),this),8,8,36,28);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(retofinv_state::fg_get_tile_info),this),tilemap_mapper_delegate(FUNC(retofinv_state::tilemap_scan),this),8,8,36,28);

	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0);

	save_item(NAME(m_fg_bank));
	save_item(NAME(m_bg_bank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void retofinv_state::bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void retofinv_state::fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void retofinv_state::gfx_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch (offset)
	{
		case 0:
			flip_screen_set(data & 1);
			break;

		case 1:
			if (m_fg_bank != (data & 1))
			{
				m_fg_bank = data & 1;
				m_fg_tilemap->mark_all_dirty();
			}
			break;

		case 2:
			if (m_bg_bank != (data & 1))
			{
				m_bg_bank = data & 1;
				m_bg_tilemap->mark_all_dirty();
			}
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

void retofinv_state::draw_sprites(bitmap_ind16 &bitmap)
{
	uint8_t *spriteram = m_sharedram + 0x0780;
	uint8_t *spriteram_2 = m_sharedram + 0x0f80;
	uint8_t *spriteram_3 = m_sharedram + 0x1780;
	int offs;
	const rectangle spritevisiblearea(2*8, 34*8-1, 0*8, 28*8-1);

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs];
		int color = spriteram[offs+1] & 0x3f;
		int sx = ((spriteram_2[offs+1] << 1) + ((spriteram_3[offs+1] & 0x80) >> 7)) - 39;
		int sy = 256 - ((spriteram_2[offs] << 1) + ((spriteram_3[offs] & 0x80) >> 7)) + 1;
		/* not sure about the flipping, it's hardly ever used (mostly for shots) */
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizey = (spriteram_3[offs] & 0x04) >> 2;
		int sizex = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;  // fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				m_gfxdecode->gfx(1)->transmask(bitmap,spritevisiblearea,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x,sy + 16*y,
					m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0xff));
			}
		}
	}
}



uint32_t retofinv_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
