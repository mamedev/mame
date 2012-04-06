/******************************************************************************

  Ganbare Ginkun  (Japan)  (c)1995 TECMO
  Final StarForce (US)     (c)1992 TECMO
  Riot            (Japan)  (c)1992 NMK

  Based on sprite drivers from video/wc90.c by Ernesto Corvi (ernesto@imagina.com)

******************************************************************************/

#include "emu.h"
#include "includes/tecmo16.h"


/******************************************************************************/

static TILE_GET_INFO( fg_get_tile_info )
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();
	int tile = state->m_videoram[tile_index] & 0x1fff;
	int color = state->m_colorram[tile_index] & 0x0f;

	/* bit 4 controls blending */
	tileinfo.category = (state->m_colorram[tile_index] & 0x10) >> 4;

	SET_TILE_INFO(
			1,
			tile,
			color | (tileinfo.category ? 0x70 : 0x00),
			0);
}

static TILE_GET_INFO( bg_get_tile_info )
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();
	int tile = state->m_videoram2[tile_index] & 0x1fff;
	int color = (state->m_colorram2[tile_index] & 0x0f)+0x10;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( tx_get_tile_info )
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();
	int tile = state->m_charram[tile_index];
	SET_TILE_INFO(
			0,
			tile & 0x0fff,
			tile >> 12,
			0);
}

/******************************************************************************/

VIDEO_START( fstarfrc )
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();

	/* set up tile layers */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);

	/* set up sprites */
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);

	state->m_fg_tilemap = tilemap_create(machine, fg_get_tile_info,tilemap_scan_rows,16,16,32,32);
	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,16,16,32,32);
	state->m_tx_tilemap = tilemap_create(machine, tx_get_tile_info,tilemap_scan_rows, 8, 8,64,32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_transparent_pen(0);

	state->m_tx_tilemap->set_scrolly(0,-16);
	state->m_flipscreen = 0;
	state->m_game_is_riot = 0;
}

VIDEO_START( ginkun )
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();

	/* set up tile layers */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);

	/* set up sprites */
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);

	state->m_fg_tilemap = tilemap_create(machine, fg_get_tile_info,tilemap_scan_rows,16,16,64,32);
	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,16,16,64,32);
	state->m_tx_tilemap = tilemap_create(machine, tx_get_tile_info,tilemap_scan_rows, 8, 8,64,32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_flipscreen = 0;
	state->m_game_is_riot = 0;
}

VIDEO_START( riot )
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();

	/* set up tile layers */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);

	/* set up sprites */
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);

	state->m_fg_tilemap = tilemap_create(machine, fg_get_tile_info,tilemap_scan_rows,16,16,64,32);
	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,16,16,64,32);
	state->m_tx_tilemap = tilemap_create(machine, tx_get_tile_info,tilemap_scan_rows, 8, 8,64,32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_scrolldy(-16,-16);
	state->m_flipscreen = 0;
	state->m_game_is_riot = 1;
}

/******************************************************************************/

WRITE16_MEMBER(tecmo16_state::tecmo16_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_colorram_w)
{
	COMBINE_DATA(&m_colorram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_videoram2_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_colorram2_w)
{
	COMBINE_DATA(&m_colorram2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(tecmo16_state::tecmo16_charram_w)
{
	COMBINE_DATA(&m_charram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_flipscreen_w)
{
	m_flipscreen = data & 0x01;
	flip_screen_set(machine(), m_flipscreen);
}

/******************************************************************************/

WRITE16_MEMBER(tecmo16_state::tecmo16_scroll_x_w)
{
	COMBINE_DATA(&m_scroll_x_w);
	m_fg_tilemap->set_scrollx(0,m_scroll_x_w);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_scroll_y_w)
{
	COMBINE_DATA(&m_scroll_y_w);
	m_fg_tilemap->set_scrolly(0,m_scroll_y_w);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_scroll2_x_w)
{
	COMBINE_DATA(&m_scroll2_x_w);
	m_bg_tilemap->set_scrollx(0,m_scroll2_x_w);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_scroll2_y_w)
{
	COMBINE_DATA(&m_scroll2_y_w);
	m_bg_tilemap->set_scrolly(0,m_scroll2_y_w);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_scroll_char_x_w)
{
	COMBINE_DATA(&m_scroll_char_x_w);
	m_tx_tilemap->set_scrollx(0,m_scroll_char_x_w);
}

WRITE16_MEMBER(tecmo16_state::tecmo16_scroll_char_y_w)
{
	COMBINE_DATA(&m_scroll_char_y_w);
	m_tx_tilemap->set_scrolly(0,m_scroll_char_y_w-16);
}

/******************************************************************************/

/* mix & blend the paletted 16-bit tile and sprite bitmaps into an RGB 32-bit bitmap */
static void blendbitmaps(running_machine &machine,
		bitmap_rgb32 &dest,bitmap_ind16 &src1,bitmap_ind16 &src2,bitmap_ind16 &src3,
		int sx,int sy,const rectangle &clip)
{
	int ox;
	int oy;
	int ex;
	int ey;

	/* check bounds */
	ox = sx;
	oy = sy;

	ex = sx + src1.width() - 1;
	if (sx < 0) sx = 0;
	if (sx < clip.min_x) sx = clip.min_x;
	if (ex >= dest.width()) ex = dest.width() - 1;
	if (ex > clip.max_x) ex = clip.max_x;
	if (sx > ex) return;

	ey = sy + src1.height() - 1;
	if (sy < 0) sy = 0;
	if (sy < clip.min_y) sy = clip.min_y;
	if (ey >= dest.height()) ey = dest.height() - 1;
	if (ey > clip.max_y) ey = clip.max_y;
	if (sy > ey) return;

	{
		const pen_t *paldata = machine.pens;
		UINT32 *end;

		UINT16 *sd1 = &src1.pix16(0);
		UINT16 *sd2 = &src2.pix16(0);
		UINT16 *sd3 = &src3.pix16(0);

		int sw = ex-sx+1;														/* source width  */
		int sh = ey-sy+1;														/* source height */
		int sm = src1.rowpixels();												/* source modulo */

		UINT32 *dd = &dest.pix32(sy, sx);								/* dest data     */
		int dm = dest.rowpixels();												/* dest modulo   */

		sd1 += (sx-ox);
		sd1 += sm * (sy-oy);
		sd2 += (sx-ox);
		sd2 += sm * (sy-oy);
		sd3 += (sx-ox);
		sd3 += sm * (sy-oy);

		sm -= sw;
		dm -= sw;

		while (sh)
		{

#define BLENDPIXEL(x)	if (sd3[x]) {														\
							if (sd2[x]) {													\
								dd[x] = paldata[sd2[x] | 0x0400] + paldata[sd3[x]];			\
							} else {														\
								dd[x] = paldata[sd1[x] | 0x0400] + paldata[sd3[x]];			\
							}																\
						} else {															\
							if (sd2[x]) {													\
								if (sd2[x] & 0x0800) {										\
									dd[x] = paldata[sd1[x] | 0x0400] + paldata[sd2[x]];		\
								} else {													\
									dd[x] = paldata[sd2[x]];								\
								}															\
							} else {														\
								dd[x] = paldata[sd1[x]];									\
							}																\
						}

			end = dd + sw;
			while (dd <= end - 8)
			{
				BLENDPIXEL(0);
				BLENDPIXEL(1);
				BLENDPIXEL(2);
				BLENDPIXEL(3);
				BLENDPIXEL(4);
				BLENDPIXEL(5);
				BLENDPIXEL(6);
				BLENDPIXEL(7);
				dd += 8;
				sd1 += 8;
				sd2 += 8;
				sd3 += 8;
			}
			while (dd < end)
			{
				BLENDPIXEL(0);
				dd++;
				sd1++;
				sd2++;
				sd3++;
			}
			dd += dm;
			sd1 += sm;
			sd2 += sm;
			sd3 += sm;
			sh--;

#undef BLENDPIXEL

		}
	}
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect)
{
	tecmo16_state *state = machine.driver_data<tecmo16_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs;
	static const UINT8 layout[8][8] =
	{
		{ 0, 1, 4, 5,16,17,20,21},
		{ 2, 3, 6, 7,18,19,22,23},
		{ 8, 9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	bitmap_ind16 &bitmap = bitmap_bg;

	for (offs = state->m_spriteram_size/2 - 8;offs >= 0;offs -= 8)
	{
		if (spriteram16[offs] & 0x04)	/* enable */
		{
			int code,color,sizex,sizey,flipx,flipy,xpos,ypos;
			int x,y,priority,priority_mask;

			code = spriteram16[offs+1];
			color = (spriteram16[offs+2] & 0xf0) >> 4;
			sizex = 1 << ((spriteram16[offs+2] & 0x03) >> 0);

			if(state->m_game_is_riot)
				sizey = sizex;
			else
				sizey = 1 << ((spriteram16[offs+2] & 0x0c) >> 2);

			if (sizex >= 2) code &= ~0x01;
			if (sizey >= 2) code &= ~0x02;
			if (sizex >= 4) code &= ~0x04;
			if (sizey >= 4) code &= ~0x08;
			if (sizex >= 8) code &= ~0x10;
			if (sizey >= 8) code &= ~0x20;
			flipx = spriteram16[offs] & 0x01;
			flipy = spriteram16[offs] & 0x02;
			xpos = spriteram16[offs+4];
			if (xpos >= 0x8000) xpos -= 0x10000;
			ypos = spriteram16[offs+3];
			if (ypos >= 0x8000) ypos -= 0x10000;
			priority = (spriteram16[offs] & 0xc0) >> 6;

			/* bg: 1; fg:2; text: 4 */
			switch (priority)
			{
				default:
				case 0x0: priority_mask = 0; break;
				case 0x1: priority_mask = 0xf0; break; /* obscured by text layer */
				case 0x2: priority_mask = 0xf0|0xcc; break;	/* obscured by foreground */
				case 0x3: priority_mask = 0xf0|0xcc|0xaa; break; /* obscured by bg and fg */
			}

			if (state->m_flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}

			/* blending */
			if (spriteram16[offs] & 0x20)
			{
				color |= 0x80;

				for (y = 0;y < sizey;y++)
				{
					for (x = 0;x < sizex;x++)
					{
						int sx,sy;

						if (!state->m_flipscreen)
						{
							sx = xpos + 8*(flipx?(sizex-1-x):x);
							sy = ypos + 8*(flipy?(sizey-1-y):y);
						} else {
							sx = 256 - (xpos + 8*(!flipx?(sizex-1-x):x) + 8);
							sy = 256 - (ypos + 8*(!flipy?(sizey-1-y):y) + 8);
						}
						pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[2],
								code + layout[y][x],
								machine.gfx[2]->color_base + color * machine.gfx[2]->color_granularity,
								flipx,flipy,
								sx,sy,
								machine.priority_bitmap, priority_mask,0);

						/* wrap around x */
						pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[2],
								code + layout[y][x],
								machine.gfx[2]->color_base + color * machine.gfx[2]->color_granularity,
								flipx,flipy,
								sx-512,sy,
								machine.priority_bitmap, priority_mask,0);

						/* wrap around x */
						pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[2],
								code + layout[y][x],
								machine.gfx[2]->color_base + color * machine.gfx[2]->color_granularity,
								flipx,flipy,
								sx+512,sy,
								machine.priority_bitmap, priority_mask,0);
					}
				}
			}
			else
			{
				bitmap_ind16 &bitmap = (priority >= 2) ? bitmap_bg : bitmap_fg;

				for (y = 0;y < sizey;y++)
				{
					for (x = 0;x < sizex;x++)
					{
						int sx,sy;

						if (!state->m_flipscreen)
						{
							sx = xpos + 8*(flipx?(sizex-1-x):x);
							sy = ypos + 8*(flipy?(sizey-1-y):y);
						} else {
							sx = 256 - (xpos + 8*(!flipx?(sizex-1-x):x) + 8);
							sy = 256 - (ypos + 8*(!flipy?(sizey-1-y):y) + 8);
						}
						pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[2],
								code + layout[y][x],
								machine.gfx[2]->color_base + color * machine.gfx[2]->color_granularity,
								flipx,flipy,
								sx,sy,
								machine.priority_bitmap, priority_mask,0);

						/* wrap around x */
						pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[2],
								code + layout[y][x],
								machine.gfx[2]->color_base + color * machine.gfx[2]->color_granularity,
								flipx,flipy,
								sx-512,sy,
								machine.priority_bitmap, priority_mask,0);

						/* wrap around x */
						pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[2],
								code + layout[y][x],
								machine.gfx[2]->color_base + color * machine.gfx[2]->color_granularity,
								flipx,flipy,
								sx+512,sy,
								machine.priority_bitmap, priority_mask,0);
					}
				}
			}
		}
	}
}

/******************************************************************************/

SCREEN_UPDATE_RGB32( tecmo16 )
{
	tecmo16_state *state = screen.machine().driver_data<tecmo16_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_tile_bitmap_bg.fill(0x300, cliprect);
	state->m_tile_bitmap_fg.fill(0, cliprect);
	state->m_sprite_bitmap.fill(0, cliprect);

	/* draw tilemaps into a 16-bit bitmap */
	state->m_bg_tilemap->draw(state->m_tile_bitmap_bg, cliprect, 0, 1);
	state->m_fg_tilemap->draw(state->m_tile_bitmap_fg, cliprect, 0, 2);
	/* draw the blended tiles at a lower priority
       so sprites covered by them will still be drawn */
	state->m_fg_tilemap->draw(state->m_tile_bitmap_fg, cliprect, 1, 0);
	state->m_tx_tilemap->draw(state->m_tile_bitmap_fg, cliprect, 0, 4);

	/* draw sprites into a 16-bit bitmap */
	draw_sprites(screen.machine(), state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, state->m_sprite_bitmap, cliprect);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(screen.machine(), bitmap, state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, state->m_sprite_bitmap, 0, 0, cliprect);
	return 0;
}
