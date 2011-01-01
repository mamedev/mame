#include "emu.h"
#include "includes/deniam.h"


static void deniam_common_init( running_machine *machine )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	int i;

	state->bg_scrollx_reg = 0x00a4/2;
	state->bg_scrolly_reg = 0x00a8/2;
	state->bg_page_reg    = 0x00ac/2;
	state->fg_scrollx_reg = 0x00a2/2;
	state->fg_scrolly_reg = 0x00a6/2;
	state->fg_page_reg    = 0x00aa/2;

	state->display_enable = 0;
	state->coinctrl = 0;

	for (i = 0; i < 4; i++)
	{
		state->bg_page[i] = 0;
		state->fg_page[i] = 0;
	}
}

DRIVER_INIT( logicpro )
{
	deniam_state *state = machine->driver_data<deniam_state>();

	deniam_common_init(machine);

	state->bg_scrollx_offs = 0x00d;
	state->bg_scrolly_offs = 0x000;
	state->fg_scrollx_offs = 0x009;
	state->fg_scrolly_offs = 0x000;
}

DRIVER_INIT( karianx )
{
	deniam_state *state = machine->driver_data<deniam_state>();

	deniam_common_init(machine);

	state->bg_scrollx_offs = 0x10d;
	state->bg_scrolly_offs = 0x080;
	state->fg_scrollx_offs = 0x109;
	state->fg_scrolly_offs = 0x080;
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( scan_pages )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x3f) + ((row & 0x1f) << 6) + ((col & 0x40) << 5) + ((row & 0x20) << 7);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	int page = tile_index >> 11;
	UINT16 attr = state->videoram[state->bg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO(
			0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	int page = tile_index >> 11;
	UINT16 attr = state->videoram[state->fg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO(
			0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	UINT16 attr = state->textram[tile_index];
	SET_TILE_INFO(
			0,
			attr & 0xf1ff,
			(attr & 0x0e00) >> 9,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( deniam )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, scan_pages, 8, 8, 128, 64);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, scan_pages, 8, 8, 128, 64);
	state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_transparent_pen(state->tx_tilemap, 0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( deniam_videoram_w )
{
	deniam_state *state = space->machine->driver_data<deniam_state>();
	int page, i;
	COMBINE_DATA(&state->videoram[offset]);

	page = offset >> 11;
	for (i = 0; i < 4; i++)
	{
		if (state->bg_page[i] == page)
			tilemap_mark_tile_dirty(state->bg_tilemap, i * 0x800 + (offset & 0x7ff));
		if (state->fg_page[i] == page)
			tilemap_mark_tile_dirty(state->fg_tilemap, i * 0x800 + (offset & 0x7ff));
	}
}


WRITE16_HANDLER( deniam_textram_w )
{
	deniam_state *state = space->machine->driver_data<deniam_state>();
	COMBINE_DATA(&state->textram[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}


WRITE16_HANDLER( deniam_palette_w )
{
	deniam_state *state = space->machine->driver_data<deniam_state>();
	int r, g, b;

	data = COMBINE_DATA(&state->paletteram[offset]);

	r = ((data << 1) & 0x1e) | ((data >> 12) & 0x01);
	g = ((data >> 3) & 0x1e) | ((data >> 13) & 0x01);
	b = ((data >> 7) & 0x1e) | ((data >> 14) & 0x01);
	palette_set_color_rgb(space->machine, offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

READ16_HANDLER( deniam_coinctrl_r )
{
	deniam_state *state = space->machine->driver_data<deniam_state>();
	return state->coinctrl;
}

WRITE16_HANDLER( deniam_coinctrl_w )
{
	deniam_state *state = space->machine->driver_data<deniam_state>();
	COMBINE_DATA(&state->coinctrl);

	/* bit 0 is coin counter */
	coin_counter_w(space->machine, 0, state->coinctrl & 0x01);

	/* bit 6 is display enable (0 freezes screen) */
	state->display_enable = state->coinctrl & 0x20;

	/* other bits unknown (unused?) */
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | --------xxxxxxxx | display y start
 *   0  | xxxxxxxx-------- | display y end
 *   2  | -------xxxxxxxxx | x position
 *   2  | ------x--------- | unknown (used in logicpr2, maybe just a bug?)
 *   2  | xxxxxx---------- | unused?
 *   4  | ---------xxxxxxx | width
 *   4  | --------x------- | is this flip y like in System 16?
 *   4  | -------x-------- | flip x
 *   4  | xxxxxxx--------- | unused?
 *   6  | xxxxxxxxxxxxxxxx | ROM address low bits
 *   8  | ----------xxxxxx | color
 *   8  | --------xx------ | priority
 *   8  | ---xxxxx-------- | ROM address high bits
 *   8  | xxx------------- | unused? (extra address bits for larger ROMs?)
 *   a  | ---------------- | zoomx like in System 16?
 *   c  | ---------------- | zoomy like in System 16?
 *   e  | ---------------- |
 */
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	int offs;
	UINT8 *gfx = machine->region("gfx2")->base();

	for (offs = state->spriteram_size / 2 - 8; offs >= 0; offs -= 8)
	{
		int sx, starty, endy, x, y, start, color, width, flipx, primask;
		UINT8 *rom = gfx;

		sx = (state->spriteram[offs + 1] & 0x01ff) + 16 * 8 - 1;
		if (sx >= 512) sx -= 512;
		starty = state->spriteram[offs + 0] & 0xff;
		endy = state->spriteram[offs + 0] >> 8;

		width = state->spriteram[offs + 2] & 0x007f;
		flipx = state->spriteram[offs + 2] & 0x0100;
		if (flipx) sx++;

		color = 0x40 + (state->spriteram[offs + 4] & 0x3f);

		primask = 8;
		switch (state->spriteram[offs + 4] & 0xc0)
		{
			case 0x00: primask |= 4 | 2 | 1; break;	/* below everything */
			case 0x40: primask |= 4 | 2;     break;	/* below fg and tx */
			case 0x80: primask |= 4;         break;	/* below tx */
			case 0xc0:                       break;	/* above everything */
		}


		start = state->spriteram[offs + 3] + ((state->spriteram[offs + 4] & 0x1f00) << 8);
		rom += 2 * start;

		for (y = starty + 1; y <= endy; y++)
		{
			int drawing = 0;
			int i = 0;

			rom += 2 * width;	/* note that the first line is skipped */
			x = 0;
			while (i < 512)	/* safety check */
			{
				if (flipx)
				{
					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = 1;
						else break;
					}
					else
					{
						if (rom[i] & 0x0f)
						{
							if (sx + x >= cliprect->min_x && sx + x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx + x) = color * 16 + (rom[i] & 0x0f);
								*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) = 8;
							}
						}
						x++;
					}

					if ((rom[i] & 0xf0) == 0xf0)
					{
						if (!drawing) drawing = 1;
						else break;
					}
					else
					{
						if (rom[i] & 0xf0)
						{
							if (sx + x >= cliprect->min_x && sx + x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx + x) = color * 16+(rom[i] >> 4);
								*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) = 8;
							}
						}
						x++;
					}

					i--;
				}
				else
				{
					if ((rom[i] & 0xf0) == 0xf0)
					{
						if (!drawing) drawing = 1;
						else break;
					}
					else
					{
						if (rom[i] & 0xf0)
						{
							if (sx + x >= cliprect->min_x && sx + x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx + x) = color * 16 + (rom[i] >> 4);
								*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) = 8;
							}
						}
						x++;
					}

					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = 1;
						else break;
					}
					else
					{
						if (rom[i] & 0x0f)
						{
							if (sx + x >= cliprect->min_x && sx + x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx + x) = color * 16 + (rom[i] & 0x0f);
								*BITMAP_ADDR8(machine->priority_bitmap, y, sx + x) = 8;
							}
						}
						x++;
					}

					i++;
				}
			}
		}
	}
}

static void set_bg_page( running_machine *machine, int page, int value )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	int tile_index;

	if (state->bg_page[page] != value)
	{
		state->bg_page[page] = value;
		for (tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			tilemap_mark_tile_dirty(state->bg_tilemap, tile_index);
	}
}

static void set_fg_page( running_machine *machine, int page, int value )
{
	deniam_state *state = machine->driver_data<deniam_state>();
	int tile_index;

	if (state->fg_page[page] != value)
	{
		state->fg_page[page] = value;
		for (tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			tilemap_mark_tile_dirty(state->fg_tilemap, tile_index);
	}
}

VIDEO_UPDATE( deniam )
{
	deniam_state *state = screen->machine->driver_data<deniam_state>();
	int bg_scrollx, bg_scrolly, fg_scrollx, fg_scrolly;
	int page;

	if (!state->display_enable)
		return 0;	/* don't update (freeze display) */

	bg_scrollx = state->textram[state->bg_scrollx_reg] - state->bg_scrollx_offs;
	bg_scrolly = (state->textram[state->bg_scrolly_reg] & 0xff) - state->bg_scrolly_offs;
	page = state->textram[state->bg_page_reg];
	set_bg_page(screen->machine, 3, (page >>12) & 0x0f);
	set_bg_page(screen->machine, 2, (page >> 8) & 0x0f);
	set_bg_page(screen->machine, 1, (page >> 4) & 0x0f);
	set_bg_page(screen->machine, 0, (page >> 0) & 0x0f);

	fg_scrollx = state->textram[state->fg_scrollx_reg] - state->fg_scrollx_offs;
	fg_scrolly = (state->textram[state->fg_scrolly_reg] & 0xff) - state->fg_scrolly_offs;
	page = state->textram[state->fg_page_reg];
	set_fg_page(screen->machine, 3, (page >>12) & 0x0f);
	set_fg_page(screen->machine, 2, (page >> 8) & 0x0f);
	set_fg_page(screen->machine, 1, (page >> 4) & 0x0f);
	set_fg_page(screen->machine, 0, (page >> 0) & 0x0f);

	tilemap_set_scrollx(state->bg_tilemap, 0, bg_scrollx & 0x1ff);
	tilemap_set_scrolly(state->bg_tilemap, 0, bg_scrolly & 0x0ff);
	tilemap_set_scrollx(state->fg_tilemap, 0, fg_scrollx & 0x1ff);
	tilemap_set_scrolly(state->fg_tilemap, 0, fg_scrolly & 0x0ff);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 2);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
