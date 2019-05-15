// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail,Stephane Humbert
/***************************************************************************

   Alpha 68k video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "includes/alpha68k.h"


void alpha68k_state::flipscreen_w(int flip)
{
	m_flipscreen = flip;
}

void alpha68k_state::video_bank_w(u8 data)
{
	if ((m_bank_base ^ data) & 0xf)
	{
		m_bank_base = data & 0xf;
		m_fix_tilemap->mark_all_dirty();
	}
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(alpha68k_state::get_tile_info)
{
	const u32 tile = m_videoram[2 * tile_index] & 0xff;
	const u32 color = m_videoram[2 * tile_index + 1] & 0x0f;

	SET_TILE_INFO_MEMBER(0, tile | (m_bank_base << 8), color, 0);
}

void alpha68k_state::videoram_w(offs_t offset, u16 data)
{
	/* 8 bit RAM, upper & lower byte writes end up in the same place due to m68k byte smearing */
	m_videoram[offset] = data & 0xff;

	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

VIDEO_START_MEMBER(alpha68k_state,alpha68k)
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(alpha68k_state::get_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/

//AT
void alpha68k_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e)
{
	for (int offs = s; offs < e; offs += 0x40)
	{
		int my = m_spriteram[offs + 3 + (j << 1)];
		int mx = m_spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
		my = -my & 0x1ff;
		mx = ((mx + 0x100) & 0x1ff) - 0x100;
		if (j == 0 && s == 0x7c0)
			my++;
//ZT
		if (m_flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		for (int i = 0; i < 0x40; i += 2)
		{
			u16 tile        = m_spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			const u16 color = m_spriteram[offs + i + (0x800 * j) + 0x800] & 0x7f;

			int fy = tile & 0x8000;
			int fx = tile & 0x4000;
			tile &= 0x3fff;

			if (m_flipscreen)
			{
				if (fx) fx = 0; else fx = 1;
				if (fy) fy = 0; else fy = 1;
			}

			if (color)
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					tile,
					color,
					fx,fy,
					mx,my,0);

			if (m_flipscreen)
				my = (my - 16) & 0x1ff;
			else
				my = (my + 16) & 0x1ff;
		}
	}
}

/******************************************************************************/

u32 alpha68k_state::screen_update_alpha68k_II(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(2047, cliprect);
//AT
	draw_sprites(bitmap, cliprect, 0, 0x07c0, 0x0800);
	draw_sprites(bitmap, cliprect, 1, 0x0000, 0x0800);
	draw_sprites(bitmap, cliprect, 2, 0x0000, 0x0800);
	draw_sprites(bitmap, cliprect, 0, 0x0000, 0x07c0);
//ZT
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/

/*
    Video banking:

    Write to these locations in this order for correct bank:

    20 28 30 for Bank 0
    60 28 30 for Bank 1
    20 68 30 etc
    60 68 30
    20 28 70
    60 28 70
    20 68 70
    60 68 70 for Bank 7

    Actual data values written don't matter!

*/

/* Graphics flags?  Not related to fix chars anyway */
WRITE_LINE_MEMBER(alpha68k_state::video_control2_w)
{
	logerror("%s: Q2 changed to %d\n", machine().describe_context(), state);
}

WRITE_LINE_MEMBER(alpha68k_state::video_control3_w)
{
	logerror("%s: Q3 changed to %d\n", machine().describe_context(), state);
}

/******************************************************************************/

void alpha68k_state::draw_sprites_V(bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e, u16 fx_mask, u16 fy_mask, u16 sprite_mask)
{
	for (int offs = s; offs < e; offs += 0x40)
	{
//AT
		int my = m_spriteram[offs + 3 + (j << 1)];
		int mx = m_spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
		my = -my & 0x1ff;
		mx = ((mx + 0x100) & 0x1ff) - 0x100;
		if (j == 0 && s == 0x7c0)
			my++;
//ZT
		if (m_flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		for (int i = 0; i < 0x40; i += 2)
		{
			u16 tile        = m_spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			const u16 color = m_spriteram[offs + i + (0x800 * j) + 0x800] & 0xff;

			int fx = tile & fx_mask;
			int fy = tile & fy_mask;
			tile = tile & sprite_mask;
			if (tile > 0x4fff)
				continue;

			if (m_flipscreen)
			{
				if (fx) fx = 0; else fx = 1;
				if (fy) fy = 0; else fy = 1;
			}

			if (color)
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					tile,
					color,
					fx,fy,
					mx,my,0);

			if (m_flipscreen)
				my = (my - 16) & 0x1ff;
			else
				my = (my + 16) & 0x1ff;
		}
	}
}

u32 alpha68k_state::screen_update_alpha68k_V(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(4095, cliprect);

	/* This appears to be correct priority */
	if (m_microcontroller_id == 0x8814) /* Sky Adventure */
	{
		draw_sprites_V(bitmap, cliprect, 0, 0x07c0, 0x0800, 0, 0x8000, 0x7fff);
		draw_sprites_V(bitmap, cliprect, 1, 0x0000, 0x0800, 0, 0x8000, 0x7fff);
		//AT: *KLUDGE* fixes priest priority in level 1(could be a game bug)
		if (m_spriteram[0x1bde] == 0x24 && (m_spriteram[0x1bdf] >> 8) == 0x3b)
		{
			draw_sprites_V(bitmap, cliprect, 2, 0x03c0, 0x0800, 0, 0x8000, 0x7fff);
			draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x03c0, 0, 0x8000, 0x7fff);
		}
		else
			draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x0800, 0, 0x8000, 0x7fff);

		draw_sprites_V(bitmap, cliprect, 0, 0x0000, 0x07c0, 0, 0x8000, 0x7fff);
	}
	else    /* gangwars */
	{
		draw_sprites_V(bitmap, cliprect, 0, 0x07c0, 0x0800, 0x8000, 0, 0x7fff);
		draw_sprites_V(bitmap, cliprect, 1, 0x0000, 0x0800, 0x8000, 0, 0x7fff);
		draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x0800, 0x8000, 0, 0x7fff);
		draw_sprites_V(bitmap, cliprect, 0, 0x0000, 0x07c0, 0x8000, 0, 0x7fff);
	}

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

u32 alpha68k_state::screen_update_alpha68k_V_sb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(4095, cliprect);

	/* This appears to be correct priority */
	draw_sprites_V(bitmap, cliprect, 0, 0x07c0, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(bitmap, cliprect, 1, 0x0000, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(bitmap, cliprect, 0, 0x0000, 0x07c0, 0x4000, 0x8000, 0x3fff);

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/
//AT
void alpha68k_state::draw_sprites_I(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d, int yshift)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int offs = 0; offs < 0x400; offs += 0x20)
	{
		int mx = m_spriteram[offs + c];
		int my = (yshift - (mx >> 8)) & 0xff;
		mx &= 0xff;

		for (int i = 0; i < 0x20; i++)
		{
			const u16 data = m_spriteram[offs + d + i];
			const u16 tile = data & 0x3fff;
			const bool fy = data & 0x4000;
			const u8 color = m_color_proms[tile << 1 | data >> 15];

				gfx->transpen(bitmap,cliprect, tile, color, 0, fy, mx, my, 0);

			my = (my + 8) & 0xff;
		}
	}
}

u32 alpha68k_state::screen_update_alpha68k_I(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int yshift = (m_microcontroller_id == 0x890a) ? 1 : 0; // The Next Space is 1 pixel off

	bitmap.fill(m_palette->black_pen(), cliprect);

	/* This appears to be correct priority */
	draw_sprites_I(bitmap, cliprect, 2, 0x0800, yshift);
	draw_sprites_I(bitmap, cliprect, 3, 0x0c00, yshift);
	draw_sprites_I(bitmap, cliprect, 1, 0x0400, yshift);
	return 0;
}
//ZT
/******************************************************************************/

void alpha68k_state::kyros_palette(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	for (int i = 0; i < 0x100; i++)
	{
		u8 const ctabentry = ((color_prom[i] & 0x0f) << 4) | (color_prom[i + 0x100] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}

void alpha68k_state::paddlem_palette(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	for (int i = 0; i < 0x400; i++)
	{
		u8 const ctabentry = ((color_prom[i + 0x400] & 0x0f) << 4) | (color_prom[i] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}

void alpha68k_state::kyros_video_banking(u8 *bank, int data)
{
	*bank = (data >> 13 & 4) | (data >> 10 & 3);
}

void alpha68k_state::jongbou_video_banking(u8 *bank, int data)
{
	*bank = (data >> 11 & 4) | (data >> 10 & 3);
}

void alpha68k_state::kyros_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d)
{
//AT
	for (int offs = 0; offs < 0x400; offs += 0x20)
	{
		int mx = m_spriteram[offs + c];
		int my = -(mx >> 8) & 0xff;
		mx &= 0xff;

		if (m_flipscreen)
			my = 249 - my;

		for (int i = 0; i < 0x20; i++)
		{
			const u16 data = m_spriteram[offs + d + i];
			if (data != 0x20)
			{
				const u8 color = m_color_proms[(data >> 1 & 0x1000) | (data & 0xffc) | (data >> 14 & 3)];
				if (color != 0xff)
				{
					int fy = data & 0x1000;
					int fx = 0;

					if (m_flipscreen)
					{
						if (fy) fy = 0; else fy = 1;
						fx = 1;
					}

					u8 bank = 0;
					const u32 tile = (data >> 3 & 0x400) | (data & 0x3ff);
					if (m_game_id == ALPHA68K_KYROS)
						kyros_video_banking(&bank, data);
					else
						jongbou_video_banking(&bank, data);

					m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect, tile, color, fx, fy, mx, my, 0);
				}
			}
//ZT
			if (m_flipscreen)
				my = (my - 8) & 0xff;
			else
				my = (my + 8) & 0xff;
		}
	}
}

u32 alpha68k_state::screen_update_kyros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_indirect(0x100, *m_videoram & 0xff);
	bitmap.fill(0x100, cliprect); //AT

	kyros_draw_sprites(bitmap, cliprect, 2, 0x0800);
	kyros_draw_sprites(bitmap, cliprect, 3, 0x0c00);
	kyros_draw_sprites(bitmap, cliprect, 1, 0x0400);
	return 0;
}

/******************************************************************************/

void alpha68k_state::sstingry_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d)
{
//AT
	for (int offs = 0; offs < 0x400; offs += 0x20)
	{
		int mx = m_spriteram[offs + c];
		int my = -(mx >> 8) & 0xff;
		mx &= 0xff;
		if (mx > 0xf8)
			mx -= 0x100;

		if (m_flipscreen)
			my = 249 - my;

		for (int i = 0; i < 0x20; i++)
		{
			const u16 data = m_spriteram[offs + d + i];
			if (data != 0x40)
			{
				int fy = data & 0x1000;
				int fx = 0;

				if (m_flipscreen)
				{
					if (fy) fy = 0; else fy = 1;
					fx = 1;
				}

				const u16 color = (data >> 7 & 0x18) | (data >> 13 & 7);
				const u16 tile = data & 0x3ff;
				const u8 bank = data >> 10 & 3;
				m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect, tile, color, fx, fy, mx, my, 0);
			}
//ZT
			if (m_flipscreen)
				my = (my - 8) & 0xff;
			else
				my = (my + 8) & 0xff;
		}
	}
}

u32 alpha68k_state::screen_update_sstingry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_indirect(0x100, *m_videoram & 0xff);
	bitmap.fill(0x100, cliprect); //AT

	sstingry_draw_sprites(bitmap, cliprect, 2, 0x0800);
	sstingry_draw_sprites(bitmap, cliprect, 3, 0x0c00);
	sstingry_draw_sprites(bitmap, cliprect, 1, 0x0400);
	return 0;
}
