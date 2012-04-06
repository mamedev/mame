/***************************************************************************

    Ninja Gaiden / Tecmo Knights Video Hardware

***************************************************************************/

#include "emu.h"
#include "includes/gaiden.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();
	UINT16 *videoram1 = &state->m_videoram3[0x0800];
	UINT16 *videoram2 = state->m_videoram3;
	SET_TILE_INFO(
			1,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();
	UINT16 *videoram1 = &state->m_videoram2[0x0800];
	UINT16 *videoram2 = state->m_videoram2;
	SET_TILE_INFO(
			2,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info_raiga )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();
	UINT16 *videoram1 = &state->m_videoram2[0x0800];
	UINT16 *videoram2 = state->m_videoram2;

	/* bit 3 controls blending */
	tileinfo.category = (videoram2[tile_index] & 0x08) >> 3;

	SET_TILE_INFO(
			2,
			videoram1[tile_index] & 0x0fff,
			((videoram2[tile_index] & 0xf0) >> 4) | (tileinfo.category ? 0x80 : 0x00),
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();
	UINT16 *videoram1 = &state->m_videoram[0x0400];
	UINT16 *videoram2 = state->m_videoram;
	SET_TILE_INFO(
			0,
			videoram1[tile_index] & 0x07ff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gaiden )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();

	/* set up tile layers */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);

	state->m_background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_foreground = tilemap_create(machine, get_fg_tile_info_raiga, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_text_layer = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_background->set_transparent_pen(0);
	state->m_foreground->set_transparent_pen(0);
	state->m_text_layer->set_transparent_pen(0);

	state->m_background->set_scrolldy(0, 33);
	state->m_foreground->set_scrolldy(0, 33);
	state->m_text_layer->set_scrolldy(0, 31);

	state->m_background->set_scrolldx(0, -1);
	state->m_foreground->set_scrolldx(0, -1);
	state->m_text_layer->set_scrolldx(0, -1);

	/* set up sprites */
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);
}

VIDEO_START( mastninj )
{

	gaiden_state *state = machine.driver_data<gaiden_state>();

	/* set up tile layers */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);

	state->m_background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_foreground = tilemap_create(machine, get_fg_tile_info_raiga, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_text_layer = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

//  state->m_background->set_transparent_pen(15);
	state->m_foreground->set_transparent_pen(15);
	state->m_text_layer->set_transparent_pen(15);

	/* set up sprites */
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);

	state->m_background->set_scrolldx(-248, 248);
	state->m_foreground->set_scrolldx(-252, 252);
}

VIDEO_START( raiga )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();

	/* set up tile layers */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);

	state->m_background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_foreground = tilemap_create(machine, get_fg_tile_info_raiga, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_text_layer = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_background->set_transparent_pen(0);
	state->m_foreground->set_transparent_pen(0);
	state->m_text_layer->set_transparent_pen(0);

	/* set up sprites */
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);
}

VIDEO_START( drgnbowl )
{
	gaiden_state *state = machine.driver_data<gaiden_state>();
	/* set up tile layers */
	state->m_background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_foreground = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_text_layer = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_foreground->set_transparent_pen(15);
	state->m_text_layer->set_transparent_pen(15);

	state->m_background->set_scrolldx(-248, 248);
	state->m_foreground->set_scrolldx(-252, 252);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(gaiden_state::gaiden_flip_w)
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(machine(), data & 1);
}

WRITE16_MEMBER(gaiden_state::gaiden_txscrollx_w)
{
	COMBINE_DATA(&m_tx_scroll_x);
	m_text_layer->set_scrollx(0, m_tx_scroll_x);
}

WRITE16_MEMBER(gaiden_state::gaiden_txscrolly_w)
{
	COMBINE_DATA(&m_tx_scroll_y);
	m_text_layer->set_scrolly(0, (m_tx_scroll_y - m_tx_offset_y) & 0xffff);
}

WRITE16_MEMBER(gaiden_state::gaiden_fgscrollx_w)
{
	COMBINE_DATA(&m_fg_scroll_x);
	m_foreground->set_scrollx(0, m_fg_scroll_x);
}

WRITE16_MEMBER(gaiden_state::gaiden_fgscrolly_w)
{
	COMBINE_DATA(&m_fg_scroll_y);
	m_foreground->set_scrolly(0, (m_fg_scroll_y - m_fg_offset_y) & 0xffff);
}

WRITE16_MEMBER(gaiden_state::gaiden_bgscrollx_w)
{
	COMBINE_DATA(&m_bg_scroll_x);
	m_background->set_scrollx(0, m_bg_scroll_x);
}

WRITE16_MEMBER(gaiden_state::gaiden_bgscrolly_w)
{
	COMBINE_DATA(&m_bg_scroll_y);
	m_background->set_scrolly(0, (m_bg_scroll_y - m_bg_offset_y) & 0xffff);
}

WRITE16_MEMBER(gaiden_state::gaiden_txoffsety_w)
{

	if (ACCESSING_BITS_0_7) {
		m_tx_offset_y = data;
		m_text_layer->set_scrolly(0, (m_tx_scroll_y - m_tx_offset_y) & 0xffff);
	}
}

WRITE16_MEMBER(gaiden_state::gaiden_fgoffsety_w)
{

	if (ACCESSING_BITS_0_7) {
		m_fg_offset_y = data;
		m_foreground->set_scrolly(0, (m_fg_scroll_y - m_fg_offset_y) & 0xffff);
	}
}

WRITE16_MEMBER(gaiden_state::gaiden_bgoffsety_w)
{

	if (ACCESSING_BITS_0_7) {
		m_bg_offset_y = data;
		m_background->set_scrolly(0, (m_bg_scroll_y - m_bg_offset_y) & 0xffff);
	}
}

WRITE16_MEMBER(gaiden_state::gaiden_sproffsety_w)
{

	if (ACCESSING_BITS_0_7) {
		m_spr_offset_y = data;
		// handled in draw_sprites
	}
}


WRITE16_MEMBER(gaiden_state::gaiden_videoram3_w)
{
	COMBINE_DATA(&m_videoram3[offset]);
	m_background->mark_tile_dirty(offset & 0x07ff);
}

READ16_MEMBER(gaiden_state::gaiden_videoram3_r)
{
	return m_videoram3[offset];
}

WRITE16_MEMBER(gaiden_state::gaiden_videoram2_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_foreground->mark_tile_dirty(offset & 0x07ff);
}

READ16_MEMBER(gaiden_state::gaiden_videoram2_r)
{
	return m_videoram2[offset];
}

WRITE16_MEMBER(gaiden_state::gaiden_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_text_layer->mark_tile_dirty(offset & 0x03ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* mix & blend the paletted 16-bit tile and sprite bitmaps into an RGB 32-bit bitmap */

/* the source data is 3 16-bit indexed bitmaps, we use them to determine which 2 colours
   to blend into the final 32-bit rgb bitmaps, this is currently broken (due to zsolt's core
   changes?) it appears that the sprite drawing is no longer putting the correct raw data
   in the bitmaps? */
static void blendbitmaps(running_machine &machine,
		bitmap_rgb32 &dest,bitmap_ind16 &src1,bitmap_ind16 &src2,bitmap_ind16 &src3,
		int sx,int sy,const rectangle &cliprect)
{
	int y,x;
	const pen_t *paldata = machine.pens;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dd  = &dest.pix32(y);
		UINT16 *sd1 = &src1.pix16(y);
		UINT16 *sd2 = &src2.pix16(y);
		UINT16 *sd3 = &src3.pix16(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (sd3[x])
			{
				if (sd2[x])
					dd[x] = paldata[sd2[x] | 0x0400] | paldata[sd3[x]];
				else
					dd[x] = paldata[sd1[x] | 0x0400] | paldata[sd3[x]];
			}
			else
			{
				if (sd2[x])
				{
					if (sd2[x] & 0x800)
						dd[x] = paldata[sd1[x] | 0x0400] | paldata[sd2[x]];
					else
						dd[x] = paldata[sd2[x]];
				}
				else
					dd[x] = paldata[sd1[x]];
			}
		}
	}
}

/* sprite format:
 *
 *  word        bit                 usage
 * --------+-fedcba9876543210-+----------------
 *    0    | ---------------x | flip x
 *         | --------------x- | flip y
 *         | -------------x-- | enable
 *         | ----------x----- | blend
 *         | --------xx------ | sprite-tile priority
 *    1    | xxxxxxxxxxxxxxxx | number
 *    2    | --------xxxx---- | palette
 *         | --------------xx | size: 8x8, 16x16, 32x32, 64x64
 *    3    | xxxxxxxxxxxxxxxx | y position
 *    4    | xxxxxxxxxxxxxxxx | x position
 *    5,6,7|                  | unused
 */

#define NUM_SPRITES 256

static void gaiden_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect )
{
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

	gaiden_state *state = machine.driver_data<gaiden_state>();
	const gfx_element *gfx = machine.gfx[3];
	const UINT16 *source = (NUM_SPRITES - 1) * 8 + state->m_spriteram;
	int count = NUM_SPRITES;

	/* draw all sprites from front to back */
	while (count--)
	{
		UINT32 attributes = source[0];
		UINT32 priority_mask;
		int col,row;

		if (attributes & 0x04)
		{
			UINT32 priority = (attributes >> 6) & 3;
			UINT32 flipx = (attributes & 1);
			UINT32 flipy = (attributes & 2);

			UINT32 color = source[2];
			UINT32 sizex = 1 << ((color >> 0) & 3);						/* 1,2,4,8 */
			UINT32 sizey = 1 << ((color >> state->m_sprite_sizey) & 3);	/* 1,2,4,8 */

			/* raiga needs something like this */
			UINT32 number = (source[1] & (sizex > 2 ? 0x7ff8 : 0x7ffc));

			int ypos = (source[3] + state->m_spr_offset_y) & 0x01ff;
			int xpos = source[4] & 0x01ff;

			color = (color >> 4) & 0x0f;

			/* wraparound */
			if (xpos >= 256)
				xpos -= 512;
			if (ypos >= 256)
				ypos -= 512;

			if (flip_screen_get(machine))
			{
				flipx = !flipx;
				flipy = !flipy;

				xpos = 256 - (8 * sizex) - xpos;
				ypos = 256 - (8 * sizey) - ypos;

				if (xpos <= -256)
					xpos += 512;
				if (ypos <= -256)
					ypos += 512;
			}

			/* bg: 1; fg:2; text: 4 */
			switch( priority )
			{
				default:
				case 0x0: priority_mask = 0;					break;
				case 0x1: priority_mask = 0xf0;					break;	/* obscured by text layer */
				case 0x2: priority_mask = 0xf0 | 0xcc;			break;	/* obscured by foreground */
				case 0x3: priority_mask = 0xf0 | 0xcc | 0xaa;	break;	/* obscured by bg and fg  */
			}


			/* blending */
			if (attributes & 0x20)
			{
				color |= 0x80;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						pdrawgfx_transpen_raw(bitmap_sp, cliprect, gfx,
							number + layout[row][col],
							gfx->color_base + color * gfx->color_granularity,
							flipx, flipy,
							sx, sy,
							machine.priority_bitmap, priority_mask, 0);
					}
				}
			}
			else
			{
				bitmap_ind16 &bitmap = (priority >= 2) ? bitmap_bg : bitmap_fg;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						pdrawgfx_transpen_raw(bitmap, cliprect, gfx,
							number + layout[row][col],
							gfx->color_base + color * gfx->color_granularity,
							flipx, flipy,
							sx, sy,
							machine.priority_bitmap, priority_mask, 0);
					}
				}
			}
		}
		source -= 8;
	}
}


static void raiga_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect )
{
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

	gaiden_state *state = machine.driver_data<gaiden_state>();
	const gfx_element *gfx = machine.gfx[3];
	const UINT16 *source = (NUM_SPRITES - 1) * 8 + state->m_spriteram;
	int count = NUM_SPRITES;

	/* draw all sprites from front to back */
	while (count--)
	{
		UINT32 attributes = source[0];
		UINT32 priority_mask;
		int col,row;

		if (attributes & 0x04)
		{
			UINT32 priority = (attributes >> 6) & 3;
			UINT32 flipx = (attributes & 1);
			UINT32 flipy = (attributes & 2);

			UINT32 color = source[2];
			UINT32 sizex = 1 << ((color >> 0) & 3);						/* 1,2,4,8 */
			UINT32 sizey = 1 << ((color >> state->m_sprite_sizey) & 3);	/* 1,2,4,8 */

			/* raiga needs something like this */
			UINT32 number = (source[1] & (sizex > 2 ? 0x7ff8 : 0x7ffc));

			int ypos = (source[3] + state->m_spr_offset_y) & 0x01ff;
			int xpos = source[4] & 0x01ff;

			color = (color >> 4) & 0x0f;

			/* wraparound */
			if (xpos >= 256)
				xpos -= 512;
			if (ypos >= 256)
				ypos -= 512;

			if (flip_screen_get(machine))
			{
				flipx = !flipx;
				flipy = !flipy;

				xpos = 256 - (8 * sizex) - xpos;
				ypos = 256 - (8 * sizey) - ypos;

				if (xpos <= -256)
					xpos += 512;
				if (ypos <= -256)
					ypos += 512;
			}

			/* bg: 1; fg:2; text: 4 */
			switch( priority )
			{
				default:
				case 0x0: priority_mask = 0;					break;
				case 0x1: priority_mask = 0xf0;					break;	/* obscured by text layer */
				case 0x2: priority_mask = 0xf0 | 0xcc;			break;	/* obscured by foreground */
				case 0x3: priority_mask = 0xf0 | 0xcc | 0xaa;	break;	/* obscured by bg and fg  */
			}

			/* blending */
			if (attributes & 0x20)
			{
				color |= 0x80;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						pdrawgfx_transpen_raw(bitmap_sp, cliprect, gfx,
							number + layout[row][col],
							gfx->color_base + color * gfx->color_granularity,
							flipx, flipy,
							sx, sy,
							machine.priority_bitmap, priority_mask, 0);
					}
				}
			}
			else
			{
				bitmap_ind16 &bitmap = (priority >= 2) ? bitmap_bg : bitmap_fg;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						pdrawgfx_transpen_raw(bitmap, cliprect, gfx,
							number + layout[row][col],
							gfx->color_base + color * gfx->color_granularity,
							flipx, flipy,
							sx, sy,
							machine.priority_bitmap, priority_mask, 0);
					}
				}
			}
		}

		source -= 8;
	}
}


/* sprite format:
 *
 *  word        bit                 usage
 * --------+-fedcba9876543210-+----------------
 *    0    | --------xxxxxxxx | sprite code (lower bits)
 *         | ---xxxxx-------- | unused ?
 *    1    | --------xxxxxxxx | y position
 *         | ------x--------- | unused ?
 *    2    | --------xxxxxxxx | x position
 *         | -------x-------- | unused ?
 *    3    | -----------xxxxx | sprite code (upper bits)
 *         | ----------x----- | sprite-tile priority
 *         | ---------x------ | flip x
 *         | --------x------- | flip y
 * 0x400   |-------------xxxx | color
 *         |---------x------- | x position (high bit)
 */

static void drgnbowl_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gaiden_state *state = machine.driver_data<gaiden_state>();
	UINT16 *spriteram = state->m_spriteram;
	int i, code, color, x, y, flipx, flipy, priority_mask;

	for( i = 0; i < 0x800/2; i += 4 )
	{
		code = (spriteram[i + 0] & 0xff) | ((spriteram[i + 3] & 0x1f) << 8);
		y = 256 - (spriteram[i + 1] & 0xff) - 12;
		x = spriteram[i + 2] & 0xff;
		color = (spriteram[(0x800/2) + i] & 0x0f);
		flipx = spriteram[i + 3] & 0x40;
		flipy = spriteram[i + 3] & 0x80;

		if(spriteram[(0x800/2) + i] & 0x80)
			x -= 256;

		x += 256;

		if(spriteram[i + 3] & 0x20)
			priority_mask = 0xf0 | 0xcc; /* obscured by foreground */
		else
			priority_mask = 0;

		pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[3],
				code,
				machine.gfx[3]->color_base + color * machine.gfx[3]->color_granularity,
				flipx,flipy,x,y,
				machine.priority_bitmap, priority_mask,15);

		/* wrap x*/
		pdrawgfx_transpen_raw(bitmap,cliprect,machine.gfx[3],
				code,
				machine.gfx[3]->color_base + color * machine.gfx[3]->color_granularity,
				flipx,flipy,x-512,y,
				machine.priority_bitmap, priority_mask,15);

	}
}

SCREEN_UPDATE_RGB32( gaiden )
{
	gaiden_state *state = screen.machine().driver_data<gaiden_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_tile_bitmap_bg.fill(0x200, cliprect);
	state->m_tile_bitmap_fg.fill(0, cliprect);
	state->m_sprite_bitmap.fill(0, cliprect);

	/* draw tilemaps into a 16-bit bitmap */
	state->m_background->draw(state->m_tile_bitmap_bg, cliprect, 0, 1);
	state->m_foreground->draw(state->m_tile_bitmap_fg, cliprect, 0, 2);
	/* draw the blended tiles at a lower priority
       so sprites covered by them will still be drawn */
	state->m_foreground->draw(state->m_tile_bitmap_fg, cliprect, 1, 0);
	state->m_text_layer->draw(state->m_tile_bitmap_fg, cliprect, 0, 4);

	/* draw sprites into a 16-bit bitmap */
	gaiden_draw_sprites(screen.machine(), state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, state->m_sprite_bitmap, cliprect);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(screen.machine(), bitmap, state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, state->m_sprite_bitmap, 0, 0, cliprect);
	return 0;

}

SCREEN_UPDATE_RGB32( raiga )
{
	gaiden_state *state = screen.machine().driver_data<gaiden_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_tile_bitmap_bg.fill(0x200, cliprect);
	state->m_tile_bitmap_fg.fill(0, cliprect);
	state->m_sprite_bitmap.fill(0, cliprect);

	/* draw tilemaps into a 16-bit bitmap */
	state->m_background->draw(state->m_tile_bitmap_bg, cliprect, 0, 1);
	state->m_foreground->draw(state->m_tile_bitmap_fg, cliprect, 0, 2);
	/* draw the blended tiles at a lower priority
       so sprites covered by them will still be drawn */
	state->m_foreground->draw(state->m_tile_bitmap_fg, cliprect, 1, 0);
	state->m_text_layer->draw(state->m_tile_bitmap_fg, cliprect, 0, 4);

	/* draw sprites into a 16-bit bitmap */
	raiga_draw_sprites(screen.machine(), state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, state->m_sprite_bitmap, cliprect);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(screen.machine(), bitmap, state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, state->m_sprite_bitmap, 0, 0, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( drgnbowl )
{
	gaiden_state *state = screen.machine().driver_data<gaiden_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_background->draw(bitmap, cliprect, 0, 1);
	state->m_foreground->draw(bitmap, cliprect, 0, 2);
	state->m_text_layer->draw(bitmap, cliprect, 0, 4);
	drgnbowl_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
