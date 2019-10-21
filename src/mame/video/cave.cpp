// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Cave Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing:

        X/C/V/B/Z  with  Q   shows layer 0 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  W   shows layer 1 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  E   shows layer 2 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  R   shows layer 3 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  A   shows sprites (tiles with priority 0/1/2/3/All)

        Keys can be used together!

    [ 1 Layer per chip (games use as many as 4 chips) ]

        Layer Size:             512 x 512
        Tiles:                  8 x 8 & 16 x 16.

        There are 2 tilemaps in memory, one per tiles dimension.
        A bit decides which one gets displayed.
        The tiles depth varies with games, from 16 to 256 colors.

        A per layer row-scroll / row-select effect can be enabled:

        a different scroll value is fetched (from tile RAM) for each
        scan line, and a different tilemap line for each scan line

    [ 1024 Zooming Sprites ]

        There are 2 or 4 0x4000 Sprite RAM Areas. A hardware register's
        bit selects an area to display (sprites double buffering).

        The sprites are NOT tile based: the "tile" size and start address
        is selectable for each sprite with a 16 pixel granularity.

        Also note that the zoom is of a peculiar type: pixels are never
        drawn more than once. So shrinking works as usual (some pixels are
        just not drawn) while enlarging adds some transparent pixels to
        the image, uniformly, to reach the final size.

**************************************************************************/

#include "emu.h"
#include "crsshair.h"
#include "includes/cave.h"


#define CAVE_SPRITETYPE_ZBUF        0x01
#define CAVE_SPRITETYPE_ZOOM        0x02

#define SPRITE_FLIPX_CAVE           0x01
#define SPRITE_FLIPY_CAVE           0x02
#define SPRITE_VISIBLE_CAVE         0x04


/* Sailormn: the lower 2 Megabytes of tiles banked */

void cave_state::sailormn_tilebank_w(int bank)
{
	if (m_sailormn_tilebank != bank)
	{
		m_sailormn_tilebank = bank;
		m_tilemap[2]->mark_all_dirty();
	}
}

void cave_state::sailormn_get_banked_code(bool tiledim, u32 &color, u32 &pri, u32 &code)
{
	if (!tiledim)
	{
		if ((code < 0x10000) && (m_sailormn_tilebank))
			code += 0x40000;
	}
}


/***************************************************************************

                            Video Init Routines

    Depending on the game, there can be from 1 to 4 layers and the
    tile sizes can be 8x8 or 16x16.

***************************************************************************/

void cave_state::vh_start(u16 sprcol_base, u16 sprcol_granularity)
{
	m_sprite_base_pal = sprcol_base;
	m_sprite_granularity = sprcol_granularity;

	sprite_init();

	m_layers_offs_x = 0x13;
	m_layers_offs_y = -0x12;

	m_row_effect_offs_n = -1;
	m_row_effect_offs_f = 1;

	m_background_pen[0] = m_gfxdecode[0]->gfx(0)->colorbase() +
							(m_gfxdecode[0]->gfx(0)->colors() - 1) *
							m_gfxdecode[0]->gfx(0)->granularity();

	switch (m_kludge)
	{
		case 1: /* sailormn */
			m_row_effect_offs_n = -1;
			m_row_effect_offs_f = -1;
			break;
		case 2: /* uopoko dfeveron */
			m_background_pen[0] = m_gfxdecode[0]->gfx(0)->colorbase() - m_gfxdecode[0]->gfx(0)->granularity();
			break;
		case 4: /* pwrinst2 */
			m_background_pen[0] = m_gfxdecode[0]->gfx(0)->colorbase() - m_gfxdecode[0]->gfx(0)->granularity();
			m_layers_offs_y++;
			break;
	}
}

// 4 bit sprite granularity
VIDEO_START_MEMBER(cave_state,spr_4bpp)
{
	vh_start(0, 16);
}

// 8 bit sprite granularity
VIDEO_START_MEMBER(cave_state,spr_8bpp)
{
	vh_start(0, 256);
}

// korokoro (different sprite base palette)
VIDEO_START_MEMBER(cave_state,korokoro)
{
	vh_start(0x3c00, 16);
}

// ppsatan (3 screen)
VIDEO_START_MEMBER(cave_state,ppsatan)
{
	vh_start(0x3c00, 16);
	for (int chip = 1; chip < 3; chip++)
	{
		m_background_pen[chip] = m_gfxdecode[chip]->gfx(0)->colorbase() +
							(m_gfxdecode[chip]->gfx(0)->colors() - 1) *
							m_gfxdecode[chip]->gfx(0)->granularity();

		switch (m_kludge)
		{
			case 2: /* uopoko dfeveron */
			case 4: /* pwrinst2 */
				m_background_pen[chip] = m_gfxdecode[chip]->gfx(0)->colorbase() - m_gfxdecode[chip]->gfx(0)->granularity();
				break;
		}
	}
}

/***************************************************************************

                                Sprites Drawing

    Offset:     Bits:                   Value:

    00.w        fedc ba98 76-- ----     X Position
                ---- ---- --54 3210

    02.w        fedc ba98 76-- ----     Y Position
                ---- ---- --54 3210

    04.w        fe-- ---- ---- ----
                --dc ba98 ---- ----     Color
                ---- ---- 76-- ----
                ---- ---- --54 ----     Priority
                ---- ---- ---- 3---     Flip X
                ---- ---- ---- -2--     Flip Y
                ---- ---- ---- --10     Code High Bit(s?)

    06.w                                Code Low Bits

    08/0A.w                             Zoom X / Y

    0C.w        fedc ba98 ---- ----     Tile Size X
                ---- ---- 7654 3210     Tile Size Y

    0E.w                                Unused


***************************************************************************/

void cave_state::get_sprite_info_cave(int chip)
{
	chip %= 4;
	const u8 *base_gfx = m_spriteregion[chip]->base();
	const int code_max = m_spriteregion[chip]->bytes() / (16*16);

	if (m_sprite[chip] == nullptr)
		return;

	sprite_cave *sprite = m_sprite[chip].get();

	const int glob_flipx = m_videoregs[chip][0] & 0x8000;
	const int glob_flipy = m_videoregs[chip][1] & 0x8000;

	const int max_x = m_screen[chip]->width();
	const int max_y = m_screen[chip]->height();

	const u16 *source = m_spriteram[chip] + (0x4000 / 2) * m_spriteram_bank[chip];
	const u16 *finish = source + (0x4000 / 2);

	for (; source < finish; source += 8)
	{
		int x, y;
		int total_width_f, total_height_f;

		if (m_spritetype[0] == 2)    /* Hot Dog Storm */
		{
			x = (source[0] & 0x3ff) << 8;
			y = (source[1] & 0x3ff) << 8;
		}
		else                        /* all others */
		{
			x = source[0] << 2;
			y = source[1] << 2;
		}
		const u16 attr  = source[2];
		u32 code        = source[3] + ((attr & 3) << 16);
		int zoomx       = source[4];
		int zoomy       = source[5];
		const u16 size  = source[6];

		sprite->tile_width  = ((size >> 8) & 0x1f) * 16;
		sprite->tile_height = ((size >> 0) & 0x1f) * 16;

		if (!sprite->tile_width || !sprite->tile_height)
			continue;

		/* Bound checking */
		code %= code_max;
		sprite->pen_data = base_gfx + (16 * 16) * code;

		int flipx = attr & 0x0008;
		int flipy = attr & 0x0004;

		sprite->total_width  = (total_width_f  = sprite->tile_width  * zoomx) / 0x100;
		sprite->total_height = (total_height_f = sprite->tile_height * zoomy) / 0x100;

		if (sprite->total_width <= 1)
		{
			sprite->total_width = 1;
			sprite->zoomx_re = sprite->tile_width << 16;
			sprite->xcount0 = sprite->zoomx_re / 2;
			x -= 0x80;
		}
		else
		{
			sprite->zoomx_re = 0x1000000 / zoomx;
			sprite->xcount0 = sprite->zoomx_re - 1;
		}

		if (sprite->total_height <= 1)
		{
			sprite->total_height = 1;
			sprite->zoomy_re = sprite->tile_height << 16;
			sprite->ycount0 = sprite->zoomy_re / 2;
			y -= 0x80;
		}
		else
		{
			sprite->zoomy_re = 0x1000000 / zoomy;
			sprite->ycount0 = sprite->zoomy_re - 1;
		}

		if (m_spritetype[0] == 2)
		{
			x >>= 8;
			y >>= 8;
			if (flipx && (zoomx != 0x100)) x += sprite->tile_width - sprite->total_width;
			if (flipy && (zoomy != 0x100)) y += sprite->tile_height - sprite->total_height;
		}
		else
		{
			if (flipx && (zoomx != 0x100)) x += (sprite->tile_width << 8) - total_width_f - 0x80;
			if (flipy && (zoomy != 0x100)) y += (sprite->tile_height << 8) - total_height_f - 0x80;
			x >>= 8;
			y >>= 8;
		}

		if (x > 0x1ff)  x -= 0x400;
		if (y > 0x1ff)  y -= 0x400;

		if (x + sprite->total_width <= 0 || x >= max_x || y + sprite->total_height <= 0 || y >= max_y)
		{continue;}

		sprite->priority    = (attr & 0x0030) >> 4;
		sprite->flags       = SPRITE_VISIBLE_CAVE;
		sprite->line_offset = sprite->tile_width;
		sprite->base_pen    = m_sprite_base_pal + (((attr & 0x3f00) >> 8) * m_sprite_granularity);   // first 0x4000 colors

		if (glob_flipx) { x = max_x - x - sprite->total_width;  flipx = !flipx; }
		if (glob_flipy) { y = max_y - y - sprite->total_height; flipy = !flipy; }

		sprite->x = x;
		sprite->y = y;

		if (flipx)  sprite->flags |= SPRITE_FLIPX_CAVE;
		if (flipy)  sprite->flags |= SPRITE_FLIPY_CAVE;

		sprite++;
	}
	m_num_sprites[chip] = sprite - m_sprite[chip].get();
}

void cave_state::get_sprite_info_donpachi(int chip)
{
	chip %= 4;
	const u8 *base_gfx = m_spriteregion[chip]->base();
	const int code_max = m_spriteregion[chip]->bytes() / (16*16);

	if (m_sprite[chip] == nullptr)
		return;

	sprite_cave *sprite = m_sprite[chip].get();

	const int glob_flipx = m_videoregs[chip][0] & 0x8000;
	const int glob_flipy = m_videoregs[chip][1] & 0x8000;

	const int max_x = m_screen[chip]->width();
	const int max_y = m_screen[chip]->height();

	const u16 *source = m_spriteram[chip] + (0x4000 / 2) * m_spriteram_bank[chip];
	const u16 *finish = source + (0x4000 / 2);

	for (; source < finish; source += 8)
	{
		int y;

		const u16 attr = source[0];
		u32 code       = source[1] + ((attr & 3) << 16);
		int x          = source[2] & 0x3ff;

		if (m_spritetype[0] == 3)    /* pwrinst2 */
			y = (source[3] + 1) & 0x3ff;
		else
			y = source[3] & 0x3ff;

		const u16 size = source[4];

		sprite->tile_width  = sprite->total_width  = ((size >> 8) & 0x1f) * 16;
		sprite->tile_height = sprite->total_height = ((size >> 0) & 0x1f) * 16;

		/* Bound checking */
		code %= code_max;
		sprite->pen_data = base_gfx + (16*16) * code;

		if (x > 0x1ff)  x -= 0x400;
		if (y > 0x1ff)  y -= 0x400;

		if (!sprite->tile_width || !sprite->tile_height ||
			x + sprite->total_width <= 0 || x >= max_x || y + sprite->total_height <= 0 || y >= max_y)
		{continue;}

		int flipx    = attr & 0x0008;
		int flipy    = attr & 0x0004;

		if (m_spritetype[0] == 3)    /* pwrinst2 */
		{
			sprite->priority = ((attr & 0x0010) >> 4) + 2;
			sprite->base_pen = m_sprite_base_pal + ((((attr & 0x3f00) >> 8) + ((attr & 0x0020) << 1)) * m_sprite_granularity);
		}
		else
		{
			sprite->priority = (attr & 0x0030) >> 4;
			sprite->base_pen = m_sprite_base_pal + (((attr & 0x3f00) >> 8) * m_sprite_granularity);  // first 0x4000 colors
		}

		sprite->flags = SPRITE_VISIBLE_CAVE;
		sprite->line_offset = sprite->tile_width;

		if (glob_flipx) { x = max_x - x - sprite->total_width;  flipx = !flipx; }
		if (glob_flipy) { y = max_y - y - sprite->total_height; flipy = !flipy; }

		sprite->x = x;
		sprite->y = y;

		if (flipx)  sprite->flags |= SPRITE_FLIPX_CAVE;
		if (flipy)  sprite->flags |= SPRITE_FLIPY_CAVE;

		sprite++;
	}
	m_num_sprites[chip] = sprite - m_sprite[chip].get();
}


void cave_state::sprite_init()
{
	if (m_spritetype[0] == 0 || m_spritetype[0] == 2) // most of the games
	{
		m_get_sprite_info = &cave_state::get_sprite_info_cave;
		m_spritetype[1] = CAVE_SPRITETYPE_ZOOM;
	}
	else                        // donpachi ddonpach
	{
		m_get_sprite_info = &cave_state::get_sprite_info_donpachi;
		m_spritetype[1] = 0;
	}

	m_sprite_zbuf_baseval = 0x10000 - MAX_SPRITE_NUM;
	for (int screen = 0; screen < 4; screen++)
	{
		if (m_screen[screen])
		{
			m_screen[screen]->register_screen_bitmap(m_sprite_zbuf[screen]);

			save_item(NAME(m_sprite_zbuf[screen]), screen);
		}
	}

	for (int chip = 0; chip < 4; chip++)
	{
		if (m_videoregs[chip])
		{
			m_num_sprites[chip] = m_spriteram[chip].bytes() / 0x10 / 2;
			m_sprite[chip] = std::make_unique<sprite_cave []>(m_num_sprites[chip]);
		}
		else
		{
			m_num_sprites[chip] = 0;
			m_sprite[chip] = nullptr;
		}
		for (auto &prio : m_sprite_table[chip])
			for (sprite_cave *&spr : prio)
				spr = nullptr;
		m_spriteram_bank[chip] = m_spriteram_bank_delay[chip] = 0;
	}

	m_sprite_draw = &cave_state::sprite_draw_donpachi;

	save_item(NAME(m_sprite_zbuf_baseval));
	save_item(NAME(m_num_sprites));
	save_item(NAME(m_spriteram_bank));
	save_item(NAME(m_spriteram_bank_delay));

	save_item(NAME(m_blit.clip_left));
	save_item(NAME(m_blit.clip_right));
	save_item(NAME(m_blit.clip_top));
	save_item(NAME(m_blit.clip_bottom));
}

void cave_state::sprite_check(int chip, int screen_no, screen_device &screen, const rectangle &clip)
{
	if (m_sprite[chip] == nullptr)
		return;

	{   /* set clip */
		const int left = clip.min_x;
		const int top = clip.min_y;
		const int right = clip.max_x + 1;
		const int bottom = clip.max_y + 1;

		m_blit.clip_left = left;
		m_blit.clip_top = top;
		m_blit.clip_right = right;
		m_blit.clip_bottom = bottom;
	}

	{   /* check priority & sprite type */
		sprite_cave *sprite = m_sprite[chip].get();
		const sprite_cave *const finish = &sprite[m_num_sprites[chip]];
		int i[4] = {0,0,0,0};
		int priority_check = 0;
		int spritetype = m_spritetype[1];
		const rectangle &visarea = screen.visible_area();

		while (sprite < finish)
		{
			if (sprite->x + sprite->total_width  > m_blit.clip_left && sprite->x < m_blit.clip_right  &&
				sprite->y + sprite->total_height > m_blit.clip_top  && sprite->y < m_blit.clip_bottom   )
			{
				m_sprite_table[chip][sprite->priority][i[sprite->priority]++] = sprite;

				if (!(spritetype & CAVE_SPRITETYPE_ZBUF))
				{
					if (priority_check > sprite->priority)
						spritetype |= CAVE_SPRITETYPE_ZBUF;
					else
						priority_check = sprite->priority;
				}
			}
			sprite++;
		}

		m_sprite_table[chip][0][i[0]] = nullptr;
		m_sprite_table[chip][1][i[1]] = nullptr;
		m_sprite_table[chip][2][i[2]] = nullptr;
		m_sprite_table[chip][3][i[3]] = nullptr;

		switch (spritetype)
		{
			case CAVE_SPRITETYPE_ZOOM:
				m_sprite_draw = &cave_state::sprite_draw_cave;
				break;

			case CAVE_SPRITETYPE_ZOOM | CAVE_SPRITETYPE_ZBUF:
				m_sprite_draw = &cave_state::sprite_draw_cave_zbuf;
				if (clip.min_y == visarea.min_y)
				{
					if (!(m_sprite_zbuf_baseval += MAX_SPRITE_NUM))
						m_sprite_zbuf[screen_no].fill(0, visarea);
				}
				break;

			case CAVE_SPRITETYPE_ZBUF:
				m_sprite_draw = &cave_state::sprite_draw_donpachi_zbuf;
				if (clip.min_y == visarea.min_y)
				{
					if (!(m_sprite_zbuf_baseval += MAX_SPRITE_NUM))
						m_sprite_zbuf[screen_no].fill(0, visarea);
				}
				break;

			default:
			case 0:
				m_sprite_draw = &cave_state::sprite_draw_donpachi;
		}
	}
}

void cave_state::do_blit_zoom32(int chip, const sprite_cave *sprite)
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0x10000 + sprite->xcount0, ycount0 = 0x10000 + sprite->ycount0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < m_blit.clip_left)
			x2 = m_blit.clip_left;

		if (x1 > m_blit.clip_right)
		{
			xcount0 += (x1 - m_blit.clip_right) * sprite->zoomx_re;
			x1 = m_blit.clip_right;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1--;
			}
		}

		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < m_blit.clip_left)
		{
			xcount0 += (m_blit.clip_left - x1) * sprite->zoomx_re;
			x1 = m_blit.clip_left;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1++;
			}
		}
		if (x2 > m_blit.clip_right)
			x2 = m_blit.clip_right;
		if (x1 >= x2)
			return;
	}

	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < m_blit.clip_top)
			y2 = m_blit.clip_top;
		if (y1 > m_blit.clip_bottom)
		{
			ycount0 += (y1 - m_blit.clip_bottom) * sprite->zoomy_re;
			y1 = m_blit.clip_bottom;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1--;
			}
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < m_blit.clip_top)
		{
			ycount0 += (m_blit.clip_top - y1) * sprite->zoomy_re;
			y1 = m_blit.clip_top;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1++;
			}
		}
		if (y2 > m_blit.clip_bottom)
			y2 = m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const u8 *pen_data = sprite->pen_data -1 -sprite->line_offset;
		const pen_t base_pen = sprite->base_pen;
		const int pitch = m_blit.line_offset * dy / 4;
		u32 *dest = (u32 *)(m_blit.baseaddr + m_blit.line_offset * y1);
		int ycount = ycount0;

		for (int y = y1; y != y2; y += dy)
		{
			if (ycount & 0xffff0000)
			{
				int xcount = xcount0;
				pen_data += sprite->line_offset * (ycount >> 16);
				ycount &= 0xffff;
				const u8 *source = pen_data;
				for (int x = x1; x != x2; x += dx)
				{
					if (xcount & 0xffff0000)
					{
						source += xcount >> 16;
						xcount &= 0xffff;
						const u8 pen = *source;
						if (pen)
							dest[x] = m_palette[chip]->pen_color(base_pen + pen);
					}
					xcount += sprite->zoomx_re;
				}
			}
			ycount += sprite->zoomy_re;
			dest += pitch;
		}
	}
}


void cave_state::do_blit_zoom32_zb(int chip, const sprite_cave *sprite)
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0x10000 + sprite->xcount0, ycount0 = 0x10000 + sprite->ycount0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < m_blit.clip_left)
			x2 = m_blit.clip_left;
		if (x1 > m_blit.clip_right)
		{
			xcount0 += (x1 - m_blit.clip_right) * sprite->zoomx_re;
			x1 = m_blit.clip_right;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1--;
			}
		}
		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < m_blit.clip_left)
		{
			xcount0 += (m_blit.clip_left - x1) * sprite->zoomx_re;
			x1 = m_blit.clip_left;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1++;
			}
		}
		if (x2 > m_blit.clip_right)
			x2 = m_blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < m_blit.clip_top)
			y2 = m_blit.clip_top;
		if (y1 > m_blit.clip_bottom)
		{
			ycount0 += (y1 - m_blit.clip_bottom) * sprite->zoomy_re;
			y1 = m_blit.clip_bottom;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1--;
			}
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < m_blit.clip_top)
		{
			ycount0 += (m_blit.clip_top - y1) * sprite->zoomy_re;
			y1 = m_blit.clip_top;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1++;
			}
		}
		if (y2 > m_blit.clip_bottom)
			y2 = m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const u8 *pen_data = sprite->pen_data - 1 - sprite->line_offset;
		const pen_t base_pen = sprite->base_pen;
		const int pitch = m_blit.line_offset * dy / 4;
		u32 *dest = (u32 *)(m_blit.baseaddr + m_blit.line_offset * y1);
		const int pitchz = m_blit.line_offset_zbuf * dy / 2;
		u16 *zbf = (u16 *)(m_blit.baseaddr_zbuf + m_blit.line_offset_zbuf * y1);
		const u16 pri_sp = (u16)(sprite - m_sprite[chip].get()) + m_sprite_zbuf_baseval;
		int ycount = ycount0;

		for (int y = y1; y != y2; y += dy)
		{
			if (ycount & 0xffff0000)
			{
				int xcount = xcount0;
				pen_data += sprite->line_offset * (ycount >> 16);
				ycount &= 0xffff;
				const u8 *source = pen_data;
				for (int x = x1; x != x2; x += dx)
				{
					if (xcount & 0xffff0000)
					{
						source += xcount >> 16;
						xcount &= 0xffff;
						const u8 pen = *source;
						if (pen && (zbf[x] <= pri_sp))
						{
							dest[x] = m_palette[chip]->pen_color(base_pen + pen);
							zbf[x] = pri_sp;
						}
					}
					xcount += sprite->zoomx_re;
				}
			}
			ycount += sprite->zoomy_re;
			dest += pitch;
			zbf += pitchz;
		}
	}
}

void cave_state::do_blit_32(int chip, const sprite_cave *sprite)
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0, ycount0 = 0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < m_blit.clip_left)
			x2 = m_blit.clip_left;
		if (x1 > m_blit.clip_right)
		{
			xcount0 = x1 - m_blit.clip_right;
			x1 = m_blit.clip_right;
		}
		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < m_blit.clip_left)
		{
			xcount0 = m_blit.clip_left - x1;
			x1 = m_blit.clip_left;
		}
		if (x2 > m_blit.clip_right)
			x2 = m_blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < m_blit.clip_top)
			y2 = m_blit.clip_top;
		if (y1 > m_blit.clip_bottom)
		{
			ycount0 = y1 - m_blit.clip_bottom;
			y1 = m_blit.clip_bottom;
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else {
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < m_blit.clip_top)
		{
			ycount0 = m_blit.clip_top - y1;
			y1 = m_blit.clip_top;
		}
		if (y2 > m_blit.clip_bottom)
			y2 = m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const u8 *pen_data = sprite->pen_data;
		const pen_t base_pen = sprite->base_pen;
		const int pitch = m_blit.line_offset * dy / 4;
		u32 *dest = (u32 *)(m_blit.baseaddr + m_blit.line_offset * y1);

		pen_data += sprite->line_offset * ycount0 + xcount0;
		for (int y = y1; y != y2; y += dy)
		{
			const u8 *source = pen_data;
			for (int x = x1; x != x2; x += dx)
			{
				const u8 pen = *source;
				if (pen)
					dest[x] = m_palette[chip]->pen_color(base_pen + pen);
				source++;
			}
			pen_data += sprite->line_offset;
			dest += pitch;
		}
	}
}


void cave_state::do_blit_32_zb(int chip, const sprite_cave *sprite)
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0, ycount0 = 0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < m_blit.clip_left)
			x2 = m_blit.clip_left;
		if (x1 > m_blit.clip_right)
		{
			xcount0 = x1 - m_blit.clip_right;
			x1 = m_blit.clip_right;
		}
		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < m_blit.clip_left)
		{
			xcount0 = m_blit.clip_left - x1;
			x1 = m_blit.clip_left;
		}
		if (x2 > m_blit.clip_right)
			x2 = m_blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < m_blit.clip_top)
			y2 = m_blit.clip_top;
		if (y1 > m_blit.clip_bottom)
		{
			ycount0 = y1 - m_blit.clip_bottom;
			y1 = m_blit.clip_bottom;
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < m_blit.clip_top)
		{
			ycount0 = m_blit.clip_top - y1;
			y1 = m_blit.clip_top;
		}
		if (y2 > m_blit.clip_bottom)
			y2 = m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const u8 *pen_data = sprite->pen_data;
		const pen_t base_pen = sprite->base_pen;
		const int pitch = m_blit.line_offset * dy / 4;
		u32 *dest = (u32 *)(m_blit.baseaddr + m_blit.line_offset * y1);
		const int pitchz = m_blit.line_offset_zbuf * dy / 2;
		u16 *zbf = (u16 *)(m_blit.baseaddr_zbuf + m_blit.line_offset_zbuf * y1);
		const u16 pri_sp = (u16)(sprite - m_sprite[chip].get()) + m_sprite_zbuf_baseval;

		pen_data += sprite->line_offset * ycount0 + xcount0;
		for (int y = y1; y != y2; y += dy)
		{
			const u8 *source;
			source = pen_data;
			for (int x = x1; x != x2; x += dx)
			{
				const u8 pen = *source;
				if (pen && (zbf[x] <= pri_sp))
				{
					dest[x] = m_palette[chip]->pen_color(base_pen + pen);
					zbf[x] = pri_sp;
				}
				source++;
			}
			pen_data += sprite->line_offset;
			dest += pitch;
			zbf += pitchz;
		}
	}
}


void cave_state::sprite_draw_cave(int chip, int priority)
{
	if (m_sprite[chip] == nullptr)
		return;

	int i = 0;
	while (m_sprite_table[chip][priority][i])
	{
		const sprite_cave *sprite = m_sprite_table[chip][priority][i++];
		if ((sprite->tile_width == sprite->total_width) && (sprite->tile_height == sprite->total_height))
			do_blit_32(chip, sprite);
		else
			do_blit_zoom32(chip, sprite);
	}
}

void cave_state::sprite_draw_cave_zbuf(int chip, int priority)
{
	if (m_sprite[chip] == nullptr)
		return;

	int i = 0;
	while (m_sprite_table[chip][priority][i])
	{
		const sprite_cave *sprite = m_sprite_table[chip][priority][i++];
		if ((sprite->tile_width == sprite->total_width) && (sprite->tile_height == sprite->total_height))
			do_blit_32_zb(chip, sprite);
		else
			do_blit_zoom32_zb(chip, sprite);
	}
}

void cave_state::sprite_draw_donpachi(int chip, int priority)
{
	if (m_sprite[chip] == nullptr)
		return;

	int i = 0;
	while (m_sprite_table[chip][priority][i])
		do_blit_32(chip, m_sprite_table[chip][priority][i++]);
}

void cave_state::sprite_draw_donpachi_zbuf(int chip, int priority)
{
	if (m_sprite[chip] == nullptr)
		return;

	int i = 0;
	while (m_sprite_table[chip][priority][i])
		do_blit_32_zb(chip, m_sprite_table[chip][priority][i++]);
}


/***************************************************************************

                                Screen Drawing


                Layers Control Registers (cave_vctrl_0..2)


        Offset:     Bits:                   Value:

        0.w         f--- ---- ---- ----     0 = Layer Flip X
                    -e-- ---- ---- ----     Activate Row-scroll
                    --d- ---- ---- ----
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll X

        2.w         f--- ---- ---- ----     0 = Layer Flip Y
                    -e-- ---- ---- ----     Activate Row-select
                    --d- ---- ---- ----     0 = 8x8 tiles, 1 = 16x16 tiles
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll Y

        4.w         fedc ba98 765- ----
                    ---- ---- ---4 ----     Layer Disable
                    ---- ---- ---- 32--
                    ---- ---- ---- --10     Layer Priority (decides the order
                                            of the layers for tiles with the
                                            same tile priority)


        Row-scroll / row-select data is fetched from tile RAM + $1000.

        Row-select:     a tilemap line is specified for each scan line.
        Row-scroll:     a different scroll value is specified for each scan line.


                    Sprites Registers (cave_videoregs)


    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Sprites Flip X
                -edc ba98 7654 3210     Sprites Offset X

        2.w     f--- ---- ---- ----     Sprites Flip Y
                -edc ba98 7654 3210     Sprites Offset Y

        ..

        8.w     fedc ba98 7654 32--
                ---- ---- ---- --10     Sprite RAM Bank

        There are more!

***************************************************************************/

inline void cave_state::tilemap_draw(int chip,
	screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
	u32 flags, u32 priority, u32 priority2, int GFX)
{
	tilemap038_device *TILEMAP = m_tilemap[GFX];

	/* Bail out if ... */

	if (!TILEMAP)                                              /* no tilemap; */
		return;

	if (((TILEMAP->external() & 0x0003) != priority2) ||       /* tilemap's global priority not the requested one; */
			((!TILEMAP->enable())))                            /* tilemap's disabled. */
		return;

	const bool flipx = TILEMAP->flipx();
	const bool flipy = TILEMAP->flipy();
	TILEMAP->set_flip((flipx ? TILEMAP_FLIPX : 0) | (flipy ? TILEMAP_FLIPY : 0));

	int offs_x         = m_layers_offs_x;
	const int offs_y   = m_layers_offs_y;

	const int offs_row = flipy ? m_row_effect_offs_f : m_row_effect_offs_n;

	/* An additional 8 pixel offset for layers with 8x8 tiles. Plus
	   Layer 0 is displaced by 1 pixel wrt Layer 1, so is Layer 2 wrt
	   Layer 1 */
	if      (TILEMAP == m_tilemap[0])    offs_x -= (TILEMAP->tiledim() ? 1 : (1 + 8));
	else if (TILEMAP == m_tilemap[1])    offs_x -= (TILEMAP->tiledim() ? 2 : (2 + 8));
	else if (TILEMAP == m_tilemap[2])    offs_x -= (TILEMAP->tiledim() ? 3 : (3 + 8));
	else if (TILEMAP == m_tilemap[3])    offs_x -= (TILEMAP->tiledim() ? 4 : (4 + 8));

	const int sx = TILEMAP->scrollx() - m_videoregs[chip][0] + (flipx ? (offs_x + 2) : -offs_x);
	const int sy = TILEMAP->scrolly() - m_videoregs[chip][1] + (flipy ? (offs_y + 2) : -offs_y);

	if (TILEMAP->rowselect_en())  // row-select
	{
		rectangle clip;
		int endline, vramdata0, vramdata1;

		/*
		    Row-select:

		    A tilemap line is specified for each scan line. This is handled
		    using many horizontal clipping regions (slices) and calling
		    tilemap_draw multiple times.
		*/

		clip.min_x = cliprect.min_x;
		clip.max_x = cliprect.max_x;

		for (int startline = cliprect.min_y; startline <= cliprect.max_y;)
		{
			/* Find the largest slice */
			vramdata0 = (vramdata1 = TILEMAP->rowselect(sy + offs_row + startline));
			for (endline = startline + 1; endline <= cliprect.max_y; endline++)
				if ((++vramdata1) != TILEMAP->rowselect(sy + offs_row + endline)) break;

			TILEMAP->set_scrolly(0, vramdata0 - startline);

			if (TILEMAP->rowscroll_en())  // row-scroll, row-select
			{
				/*
				    Row-scroll:

				    A different scroll value is specified for each scan line.
				    This is handled using tilemap->set_scroll_rows and calling
				    tilemap->draw just once.
				*/

				TILEMAP->set_scroll_rows(512);
				for (int line = startline; line < endline; line++)
					TILEMAP->set_scrollx((vramdata0 - startline + line) & 511,
										sx + TILEMAP->rowscroll(sy + offs_row + line));
			}
			else                    // no row-scroll, row-select
			{
				TILEMAP->set_scroll_rows(1);
				TILEMAP->set_scrollx(0, sx);
			}

			if (flipy)
			{
				clip.min_y = cliprect.max_y - (endline - 1 - cliprect.min_y);
				clip.max_y = cliprect.max_y - (startline - cliprect.min_y);
			}
			else
			{
				clip.min_y = startline;
				clip.max_y = endline - 1;
			}

			TILEMAP->draw(screen, bitmap, clip, flags, priority);

			startline = endline;
		}
	}
	else if (TILEMAP->rowscroll_en()) // row-scroll, no row-select
	{
		TILEMAP->set_scroll_rows(512);
		for (int line = cliprect.min_y; line <= cliprect.max_y; line++)
			TILEMAP->set_scrollx((line + sy) & 511,
							sx + TILEMAP->rowscroll(sy + offs_row + line));
		TILEMAP->set_scrolly(0, sy);
		TILEMAP->draw(screen, bitmap, cliprect, flags, priority);
	}
	else
	{
		/* Normal scrolling */
		TILEMAP->set_scroll_rows(1);
		TILEMAP->set_scroll_cols(1);
		TILEMAP->set_scrollx(0, sx);
		TILEMAP->set_scrolly(0, sy);
		TILEMAP->draw(screen, bitmap, cliprect, flags, priority);
	}
}


u32 cave_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	m_blit.baseaddr = reinterpret_cast<u8 *>(bitmap.raw_pixptr(0));
	m_blit.line_offset = bitmap.rowbytes();
	m_blit.baseaddr_zbuf = reinterpret_cast<u8 *>(m_sprite_zbuf[0].raw_pixptr(0));
	m_blit.line_offset_zbuf = m_sprite_zbuf[0].rowbytes();

	for (int GFX = 0; GFX < 4; GFX++)
	{
		if (m_tilemap[GFX])
			m_tilemap[GFX]->prepare();
	}

#ifdef MAME_DEBUG
{
	if (machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) || machine().input().code_pressed(KEYCODE_C) ||
			machine().input().code_pressed(KEYCODE_V) || machine().input().code_pressed(KEYCODE_B))
	{
		int msk = 0, val = 0;

		if (machine().input().code_pressed(KEYCODE_X))  val = 1;    // priority 0 only
		if (machine().input().code_pressed(KEYCODE_C))  val = 2;    // ""       1
		if (machine().input().code_pressed(KEYCODE_V))  val = 4;    // ""       2
		if (machine().input().code_pressed(KEYCODE_B))  val = 8;    // ""       3
		if (machine().input().code_pressed(KEYCODE_Z))  val = 1|2|4|8;  // All of the above priorities

		if (machine().input().code_pressed(KEYCODE_Q))  msk |= val <<  0;   // for layer 0
		if (machine().input().code_pressed(KEYCODE_W))  msk |= val <<  4;   // for layer 1
		if (machine().input().code_pressed(KEYCODE_E))  msk |= val <<  8;   // for layer 2
		if (machine().input().code_pressed(KEYCODE_R))  msk |= val << 12;   // for layer 3
		if (machine().input().code_pressed(KEYCODE_A))  msk |= val << 16;   // for sprites
		if (msk != 0) layers_ctrl &= msk;

#if 1
		/* Show the video registers (cave_videoregs) */
		popmessage("%04X %04X %04X %04X %04X %04X %04X %04X",
			m_videoregs[0][0], m_videoregs[0][1], m_videoregs[0][2], m_videoregs[0][3],
			m_videoregs[0][4], m_videoregs[0][5], m_videoregs[0][6], m_videoregs[0][7]);
#endif
		/* Show the scroll / flags registers of the selected layer */
		if ((m_tilemap[0]) && (msk & 0x000f))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[0]->vregs(0),m_tilemap[0]->vregs(1),m_tilemap[0]->vregs(2));
		if ((m_tilemap[1]) && (msk & 0x00f0))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[1]->vregs(0),m_tilemap[1]->vregs(1),m_tilemap[1]->vregs(2));
		if ((m_tilemap[2]) && (msk & 0x0f00))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[2]->vregs(0),m_tilemap[2]->vregs(1),m_tilemap[2]->vregs(2));
		if ((m_tilemap[3]) && (msk & 0xf000))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[3]->vregs(0),m_tilemap[3]->vregs(1),m_tilemap[3]->vregs(2));
	}

	/* Show the row / "column" scroll enable flags, when they change state */
	m_rasflag = 0;
	for (int GFX = 0; GFX < 4; GFX++)
	{
		if (m_tilemap[GFX])
		{
			m_rasflag |= (m_tilemap[GFX]->vregs(0) & 0x4000) ? 0x0001 << (4*GFX) : 0;
			m_rasflag |= (m_tilemap[GFX]->vregs(1) & 0x4000) ? 0x0002 << (4*GFX) : 0;
		}
	}

	if (m_rasflag != m_old_rasflag)
	{
		popmessage("Line Effect: 0:%c%c 1:%c%c 2:%c%c 3:%c%c",
			(m_rasflag & 0x0001) ? 'x' : ' ', (m_rasflag & 0x0002) ? 'y' : ' ',
			(m_rasflag & 0x0010) ? 'x' : ' ', (m_rasflag & 0x0020) ? 'y' : ' ',
			(m_rasflag & 0x0100) ? 'x' : ' ', (m_rasflag & 0x0200) ? 'y' : ' ',
			(m_rasflag & 0x1000) ? 'x' : ' ', (m_rasflag & 0x2000) ? 'y' : ' ');
		m_old_rasflag = m_rasflag;
	}
}
#endif

	sprite_check(0, 0, screen, cliprect);

	bitmap.fill(m_palette[0]->pen_color(m_background_pen[0]), cliprect);

	/*
	    Tiles and sprites are ordered by priority (0 back, 3 front) with
	    sprites going below tiles of their same priority.

	    Sprites with the same priority are ordered by their place in
	    sprite RAM (last sprite is the frontmost).

	    Tiles with the same priority are ordered by the priority of their layer.

	    Tiles with the same priority *and* the same priority of their layer
	    are ordered by layer (0 back, 2 front)
	*/
	for (int pri = 0; pri <= 3; pri++)  // tile / sprite priority
	{
		if (layers_ctrl & (1 << (pri + 16)))    (this->*m_sprite_draw)(0, pri);

		for (int pri2 = 0; pri2 <= 3; pri2++)   // priority of the whole layer
		{
			if (layers_ctrl & (1 << (pri +  0)))    tilemap_draw(0, screen, bitmap, cliprect, pri, 0, pri2, 0);
			if (layers_ctrl & (1 << (pri +  4)))    tilemap_draw(0, screen, bitmap, cliprect, pri, 0, pri2, 1);
			if (layers_ctrl & (1 << (pri +  8)))    tilemap_draw(0, screen, bitmap, cliprect, pri, 0, pri2, 2);
			if (layers_ctrl & (1 << (pri + 12)))    tilemap_draw(0, screen, bitmap, cliprect, pri, 0, pri2, 3);
		}
	}
	return 0;
}



/**************************************************************/

void cave_state::get_sprite_info(int chip)
{
	if (m_videoregs[chip] == nullptr)
		return;

	const int spriteram_bankmax = m_spriteram[chip].bytes() / 0x4000;

	if (m_kludge == 3)   /* mazinger metmqstr */
	{
		if (machine().video().skip_this_frame() == 0)
		{
			m_spriteram_bank[chip] = m_spriteram_bank_delay[chip];
			(this->*m_get_sprite_info)(chip);
		}
		m_spriteram_bank_delay[chip] = (m_videoregs[chip][4] & 3) % spriteram_bankmax;
	}
	else
	{
		if (machine().video().skip_this_frame() == 0)
		{
			m_spriteram_bank[chip] = (m_videoregs[chip][4] & 3) % spriteram_bankmax;
			(this->*m_get_sprite_info)(chip);
		}
	}
}
void cave_state::device_post_load()
{
	for (int chip = 0; chip < 4; chip++)
		get_sprite_info(chip);
}


// Poka Poka Satan: 3 screens * (1 Sprite chip + 1 Tilemap chip)

u32 cave_state::screen_update_ppsatan_core(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip)
{
	m_blit.baseaddr = reinterpret_cast<u8 *>(bitmap.raw_pixptr(0));
	m_blit.line_offset = bitmap.rowbytes();
	m_blit.baseaddr_zbuf = reinterpret_cast<u8 *>(m_sprite_zbuf[chip].raw_pixptr(0));
	m_blit.line_offset_zbuf = m_sprite_zbuf[chip].rowbytes();

	m_tilemap[chip]->prepare();

	sprite_check(chip, chip, screen, cliprect);

	bitmap.fill(m_palette[chip]->pen_color(m_background_pen[chip]), cliprect);

	for (int pri = 0; pri <= 3; pri++)  // tile / sprite priority
	{
		(this->*m_sprite_draw)(chip, pri);

		for (int pri2 = 0; pri2 <= 3; pri2++)   // priority of the whole layer
			tilemap_draw(chip, screen, bitmap, cliprect, pri, 0, pri2, chip);
	}

	return 0;
}

u32 cave_state::screen_update_ppsatan_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return screen_update_ppsatan_core(screen, bitmap, cliprect, 0);
}
u32 cave_state::screen_update_ppsatan_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	machine().crosshair().get_crosshair(1).set_screen(&screen);
	return screen_update_ppsatan_core(screen, bitmap, cliprect, 1);
}
u32 cave_state::screen_update_ppsatan_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	machine().crosshair().get_crosshair(0).set_screen(&screen);
	return screen_update_ppsatan_core(screen, bitmap, cliprect, 2);
}
