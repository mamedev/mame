/******************************************************************************

  Ganbare Ginkun  (Japan)  (c)1995 TECMO
  Final StarForce (US)     (c)1992 TECMO
  Riot            (Japan)  (c)1992 NMK

  Based on sprite drivers from video/wc90.c by Ernesto Corvi (ernesto@imagina.com)

******************************************************************************/

#include "emu.h"
#include "includes/tecmo16.h"


/******************************************************************************/

TILE_GET_INFO_MEMBER(tecmo16_state::fg_get_tile_info)
{
	int tile = m_videoram[tile_index] & 0x1fff;
	int color = m_colorram[tile_index] & 0x0f;

	/* bit 4 controls blending */
	tileinfo.category = (m_colorram[tile_index] & 0x10) >> 4;

	SET_TILE_INFO_MEMBER(1,
			tile,
			color | (tileinfo.category ? 0x70 : 0x00),
			0);
}

TILE_GET_INFO_MEMBER(tecmo16_state::bg_get_tile_info)
{
	int tile = m_videoram2[tile_index] & 0x1fff;
	int color = (m_colorram2[tile_index] & 0x0f)+0x10;

	SET_TILE_INFO_MEMBER(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(tecmo16_state::tx_get_tile_info)
{
	int tile = m_charram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			tile & 0x0fff,
			tile >> 12,
			0);
}

/******************************************************************************/

void tecmo16_state::video_start()
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::fg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolly(0,-16);
	m_flipscreen = 0;
	m_game_is_riot = 0;
}

VIDEO_START_MEMBER(tecmo16_state,ginkun)
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::fg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
	m_flipscreen = 0;
	m_game_is_riot = 0;
}

VIDEO_START_MEMBER(tecmo16_state,riot)
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::fg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_scrolldy(-16,-16);
	m_flipscreen = 0;
	m_game_is_riot = 1;
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
	flip_screen_set(m_flipscreen);
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
static void blendbitmaps(palette_device &palette,
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
		const pen_t *paldata = palette.pens();
		UINT32 *end;

		UINT16 *sd1 = &src1.pix16(0);
		UINT16 *sd2 = &src2.pix16(0);
		UINT16 *sd3 = &src3.pix16(0);

		int sw = ex-sx+1;                                                       /* source width  */
		int sh = ey-sy+1;                                                       /* source height */
		int sm = src1.rowpixels();                                              /* source modulo */

		UINT32 *dd = &dest.pix32(sy, sx);                               /* dest data     */
		int dm = dest.rowpixels();                                              /* dest modulo   */

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
#define BLENDPIXEL(x)   if (sd3[x]) {                                                       \
							if (sd2[x]) {                                                   \
								dd[x] = paldata[sd2[x] | 0x0400] + paldata[sd3[x]];         \
							} else {                                                        \
								dd[x] = paldata[sd1[x] | 0x0400] + paldata[sd3[x]];         \
							}                                                               \
						} else {                                                            \
							if (sd2[x]) {                                                   \
								if (sd2[x] & 0x0800) {                                      \
									dd[x] = paldata[sd1[x] | 0x0400] + paldata[sd2[x]];     \
								} else {                                                    \
									dd[x] = paldata[sd2[x]];                                \
								}                                                           \
							} else {                                                        \
								dd[x] = paldata[sd1[x]];                                    \
							}                                                               \
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

/******************************************************************************/

UINT32 tecmo16_state::screen_update_tecmo16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_tile_bitmap_bg.fill(0x300, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);

	/* draw tilemaps into a 16-bit bitmap */
	m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 2);
	/* draw the blended tiles at a lower priority
	   so sprites covered by them will still be drawn */
	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 1, 0);
	m_tx_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 4);

	/* draw sprites into a 16-bit bitmap */
	tecmo16_draw_sprites(screen, m_gfxdecode, m_tile_bitmap_bg, m_tile_bitmap_fg, m_sprite_bitmap, cliprect, m_spriteram, m_spriteram.bytes(), m_game_is_riot, m_flipscreen);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(m_palette, bitmap, m_tile_bitmap_bg, m_tile_bitmap_fg, m_sprite_bitmap, 0, 0, cliprect);
	return 0;
}
