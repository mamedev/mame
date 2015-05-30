// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail
/***************************************************************************

   Alpha 68k video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "includes/alpha68k.h"


void alpha68k_state::alpha68k_flipscreen_w( int flip )
{
	m_flipscreen = flip;
}

void alpha68k_state::alpha68k_V_video_bank_w( int bank )
{
	m_bank_base = bank & 0xf;
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(alpha68k_state::get_tile_info)
{
	int tile = m_videoram[2 * tile_index] & 0xff;
	int color = m_videoram[2 * tile_index + 1] & 0x0f;

	tile = tile | (m_bank_base << 8);

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

WRITE16_MEMBER(alpha68k_state::alpha68k_videoram_w)
{
	/* 8 bit RAM, upper & lower byte writes end up in the same place due to m68k byte smearing */
	m_videoram[offset] = data & 0xff;

	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

VIDEO_START_MEMBER(alpha68k_state,alpha68k)
{
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(alpha68k_state::get_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/

//AT
void alpha68k_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e )
{
	UINT16 *spriteram = m_spriteram;
	int offs, mx, my, color, tile, fx, fy, i;

	for (offs = s; offs < e; offs += 0x40)
	{
		my = spriteram[offs + 3 + (j << 1)];
		mx = spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
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

		for (i = 0; i < 0x40; i += 2)
		{
			tile = spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			color = spriteram[offs + i + (0x800 * j) + 0x800] & 0x7f;

			fy = tile & 0x8000;
			fx = tile & 0x4000;
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

UINT32 alpha68k_state::screen_update_alpha68k_II(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_last_bank != m_bank_base)
		machine().tilemap().mark_all_dirty();

	m_last_bank = m_bank_base;
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

WRITE16_MEMBER(alpha68k_state::alpha68k_II_video_bank_w)
{
	switch (offset)
	{
		case 0x10: /* Reset */
			m_bank_base = m_buffer_28 = m_buffer_60 = m_buffer_68 = 0;
			return;
		case 0x14:
			if (m_buffer_60) m_bank_base=1; else m_bank_base=0;
			m_buffer_28 = 1;
			return;
		case 0x18:
			if (m_buffer_68) {if (m_buffer_60) m_bank_base = 3; else m_bank_base = 2; }
			if (m_buffer_28) {if (m_buffer_60) m_bank_base = 1; else m_bank_base = 0; }
			return;
		case 0x30:
			m_buffer_28 = m_buffer_68 = 0; m_bank_base = 1;
			m_buffer_60 = 1;
			return;
		case 0x34:
			if (m_buffer_60) m_bank_base = 3; else m_bank_base = 2;
			m_buffer_68 = 1;
			return;
		case 0x38:
			if (m_buffer_68) {if (m_buffer_60) m_bank_base = 7; else m_bank_base = 6; }
			if (m_buffer_28) {if (m_buffer_60) m_bank_base = 5; else m_bank_base = 4; }
			return;
		case 0x08: /* Graphics flags?  Not related to fix chars anyway */
		case 0x0c:
		case 0x28:
		case 0x2c:
			return;
	}

	logerror("%04x \n",offset);
}

/******************************************************************************/

WRITE16_MEMBER(alpha68k_state::alpha68k_V_video_control_w)
{
	switch (offset)
	{
		case 0x08: /* Graphics flags?  Not related to fix chars anyway */
		case 0x0c:
		case 0x28:
		case 0x2c:
			return;
	}
}

void alpha68k_state::draw_sprites_V( bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e, int fx_mask, int fy_mask, int sprite_mask )
{
	UINT16 *spriteram = m_spriteram;
	int offs, mx, my, color, tile, fx, fy, i;

	for (offs = s; offs < e; offs += 0x40)
	{
//AT
		my = spriteram[offs + 3 + (j << 1)];
		mx = spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
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

		for (i = 0; i < 0x40; i += 2)
		{
			tile = spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			color = spriteram[offs + i + (0x800 * j) + 0x800] & 0xff;

			fx = tile & fx_mask;
			fy = tile & fy_mask;
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

UINT32 alpha68k_state::screen_update_alpha68k_V(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *spriteram = m_spriteram;

	if (m_last_bank != m_bank_base)
		machine().tilemap().mark_all_dirty();

	m_last_bank = m_bank_base;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(4095, cliprect);

	/* This appears to be correct priority */
	if (m_microcontroller_id == 0x8814) /* Sky Adventure */
	{
		draw_sprites_V(bitmap, cliprect, 0, 0x07c0, 0x0800, 0, 0x8000, 0x7fff);
		draw_sprites_V(bitmap, cliprect, 1, 0x0000, 0x0800, 0, 0x8000, 0x7fff);
		//AT: *KLUDGE* fixes priest priority in level 1(could be a game bug)
		if (spriteram[0x1bde] == 0x24 && (spriteram[0x1bdf] >> 8) == 0x3b)
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

UINT32 alpha68k_state::screen_update_alpha68k_V_sb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_last_bank != m_bank_base)
		machine().tilemap().mark_all_dirty();

	m_last_bank = m_bank_base;
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
void alpha68k_state::draw_sprites_I( bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d, int yshift )
{
	UINT16 *spriteram = m_spriteram;
	int data, offs, mx, my, tile, color, fy, i;
	UINT8 *color_prom = memregion("user1")->base();
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (offs = 0; offs < 0x400; offs += 0x20)
	{
		mx = spriteram[offs + c];
		my = (yshift - (mx >> 8)) & 0xff;
		mx &= 0xff;

		for (i = 0; i < 0x20; i++)
		{
			data = spriteram[offs + d + i];
			tile = data & 0x3fff;
			fy = data & 0x4000;
			color = color_prom[tile << 1 | data >> 15];

				gfx->transpen(bitmap,cliprect, tile, color, 0, fy, mx, my, 0);

			my = (my + 8) & 0xff;
		}
	}
}

UINT32 alpha68k_state::screen_update_alpha68k_I(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

PALETTE_INIT_MEMBER(alpha68k_state,kyros)
{
	const UINT8 *color_prom = memregion("proms")->base();
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

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = ((color_prom[i] & 0x0f) << 4) | (color_prom[i + 0x100] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}

PALETTE_INIT_MEMBER(alpha68k_state,paddlem)
{
	const UINT8 *color_prom = memregion("proms")->base();
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

	for (i = 0; i < 0x400; i++)
	{
		UINT8 ctabentry = ((color_prom[i + 0x400] & 0x0f) << 4) | (color_prom[i] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}

void alpha68k_state::kyros_video_banking(int *bank, int data)
{
	*bank = (data >> 13 & 4) | (data >> 10 & 3);
}

void alpha68k_state::jongbou_video_banking(int *bank, int data)
{
	*bank = (data >> 11 & 4) | (data >> 10 & 3);
}

void alpha68k_state::kyros_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d )
{
	UINT16 *spriteram = m_spriteram;
	int offs, mx, my, color, tile, i, bank, fy, fx;
	int data;
	UINT8 *color_prom = memregion("user1")->base();

//AT
	for (offs = 0; offs < 0x400; offs += 0x20)
	{
		mx = spriteram[offs + c];
		my = -(mx >> 8) & 0xff;
		mx &= 0xff;

		if (m_flipscreen)
			my = 249 - my;

		for (i = 0; i < 0x20; i++)
		{
			data = spriteram[offs + d + i];
			if (data!=0x20)
			{
				color = color_prom[(data >> 1 & 0x1000) | (data & 0xffc) | (data >> 14 & 3)];
				if (color != 0xff)
				{
					fy = data & 0x1000;
					fx = 0;

					if(m_flipscreen)
					{
						if (fy) fy = 0; else fy = 1;
						fx = 1;
					}

					tile = (data >> 3 & 0x400) | (data & 0x3ff);
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

UINT32 alpha68k_state::screen_update_kyros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_indirect(0x100, *m_videoram & 0xff);
	bitmap.fill(0x100, cliprect); //AT

	kyros_draw_sprites(bitmap, cliprect, 2, 0x0800);
	kyros_draw_sprites(bitmap, cliprect, 3, 0x0c00);
	kyros_draw_sprites(bitmap, cliprect, 1, 0x0400);
	return 0;
}

/******************************************************************************/

void alpha68k_state::sstingry_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d )
{
//AT
	UINT16 *spriteram = m_spriteram;
	int data, offs, mx, my, color, tile, i, bank, fy, fx;

	for (offs = 0; offs < 0x400; offs += 0x20)
	{
		mx = spriteram[offs + c];
		my = -(mx >> 8) & 0xff;
		mx &= 0xff;
		if (mx > 0xf8)
			mx -= 0x100;

		if (m_flipscreen)
			my = 249 - my;

		for (i = 0; i < 0x20; i++)
		{
			data = spriteram[offs + d + i];
			if (data != 0x40)
			{
				fy = data & 0x1000;
				fx = 0;

				if(m_flipscreen)
				{
					if (fy) fy = 0; else fy = 1;
					fx = 1;
				}

				color = (data >> 7 & 0x18) | (data >> 13 & 7);
				tile = data & 0x3ff;
				bank = data >> 10 & 3;
				m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect, tile, color, fx, fy, mx, my, 0);
			}
//ZT
			if(m_flipscreen)
				my = (my - 8) & 0xff;
			else
				my = (my + 8) & 0xff;
		}
	}
}

UINT32 alpha68k_state::screen_update_sstingry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_indirect(0x100, *m_videoram & 0xff);
	bitmap.fill(0x100, cliprect); //AT

	sstingry_draw_sprites(bitmap, cliprect, 2, 0x0800);
	sstingry_draw_sprites(bitmap, cliprect, 3, 0x0c00);
	sstingry_draw_sprites(bitmap, cliprect, 1, 0x0400);
	return 0;
}
