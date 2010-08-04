// remaining gfx glitches

// layer priority register not fully understood

#include "emu.h"
#include "includes/drgnmst.h"


static TILE_GET_INFO( get_drgnmst_fg_tile_info )
{
	drgnmst_state *state = machine->driver_data<drgnmst_state>();
	int tileno, colour, flipyx;
	tileno = state->fg_videoram[tile_index * 2] & 0xfff;
	colour = state->fg_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (state->fg_videoram[tile_index * 2 + 1] & 0x60)>>5;

	SET_TILE_INFO(1, tileno, colour, TILE_FLIPYX(flipyx));
}

WRITE16_HANDLER( drgnmst_fg_videoram_w )
{
	drgnmst_state *state = space->machine->driver_data<drgnmst_state>();
	COMBINE_DATA(&state->fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_drgnmst_bg_tile_info )
{
	drgnmst_state *state = machine->driver_data<drgnmst_state>();
	int tileno, colour, flipyx;
	tileno = (state->bg_videoram[tile_index * 2]& 0x1fff) + 0x800;
	colour = state->bg_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (state->bg_videoram[tile_index * 2 + 1] & 0x60) >> 5;

	SET_TILE_INFO(3, tileno, colour, TILE_FLIPYX(flipyx));
}

WRITE16_HANDLER( drgnmst_bg_videoram_w )
{
	drgnmst_state *state = space->machine->driver_data<drgnmst_state>();
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_drgnmst_md_tile_info )
{
	drgnmst_state *state = machine->driver_data<drgnmst_state>();
	int tileno, colour, flipyx;
	tileno = (state->md_videoram[tile_index * 2] & 0x7fff) - 0x2000;
	colour = state->md_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (state->md_videoram[tile_index * 2 + 1] & 0x60) >> 5;

	SET_TILE_INFO(2, tileno, colour, TILE_FLIPYX(flipyx));
}

WRITE16_HANDLER( drgnmst_md_videoram_w )
{
	drgnmst_state *state = space->machine->driver_data<drgnmst_state>();
	COMBINE_DATA(&state->md_videoram[offset]);
	tilemap_mark_tile_dirty(state->md_tilemap, offset / 2);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	drgnmst_state *state = machine->driver_data<drgnmst_state>();
	const gfx_element *gfx = machine->gfx[0];
	UINT16 *source = state->spriteram;
	UINT16 *finish = source + 0x800 / 2;

	while (source < finish)
	{
		int xpos, ypos, number, flipx, flipy, wide, high;
		int x, y;
		int incx, incy;
		int colr;

		number = source[2];
		xpos = source[0];
		ypos = source[1];
		flipx = source[3] & 0x0020;
		flipy = source[3] & 0x0040;
		wide = (source[3] & 0x0f00) >> 8;
		high = (source[3] & 0xf000) >> 12;
		colr = (source[3] & 0x001f);

		if ((source[3] & 0xff00) == 0xff00) break;


		if (!flipx) { incx = 16;} else { incx = -16; xpos += 16 * wide; }
		if (!flipy) { incy = 16;} else { incy = -16; ypos += 16 * high; }


		for (y = 0; y <= high; y++)
		{
			for (x = 0; x <= wide; x++)
			{
				int realx, realy, realnumber;

				realx = xpos + incx * x;
				realy = ypos + incy * y;
				realnumber = number + x + y * 16;

				drawgfx_transpen(bitmap, cliprect, gfx, realnumber, colr, flipx, flipy, realx, realy, 15);
			}
		}
		source += 4;
	}
}


static TILEMAP_MAPPER( drgnmst_fg_tilemap_scan_cols )
{
	return (col * 32) + (row & 0x1f) + ((row & 0xe0) >> 5) * 2048;
}

static TILEMAP_MAPPER( drgnmst_md_tilemap_scan_cols )
{
	return (col * 16) + (row & 0x0f) + ((row & 0xf0) >> 4) * 1024;
}

static TILEMAP_MAPPER( drgnmst_bg_tilemap_scan_cols )
{
	return (col * 8) + (row & 0x07) + ((row & 0xf8) >> 3) * 512;
}

VIDEO_START(drgnmst)
{
	drgnmst_state *state = machine->driver_data<drgnmst_state>();
	state->fg_tilemap = tilemap_create(machine, get_drgnmst_fg_tile_info, drgnmst_fg_tilemap_scan_cols, 8, 8, 64,64);
	tilemap_set_transparent_pen(state->fg_tilemap, 15);

	state->md_tilemap = tilemap_create(machine, get_drgnmst_md_tile_info, drgnmst_md_tilemap_scan_cols, 16, 16, 64,64);
	tilemap_set_transparent_pen(state->md_tilemap, 15);

	state->bg_tilemap = tilemap_create(machine, get_drgnmst_bg_tile_info, drgnmst_bg_tilemap_scan_cols, 32, 32, 64,64);
	tilemap_set_transparent_pen(state->bg_tilemap, 15);

	// do the other tilemaps have rowscroll too? probably not ..
	tilemap_set_scroll_rows(state->md_tilemap, 1024);
}

VIDEO_UPDATE(drgnmst)
{
	drgnmst_state *state = screen->machine->driver_data<drgnmst_state>();
	int y, rowscroll_bank;

	tilemap_set_scrollx(state->bg_tilemap, 0, state->vidregs[10] - 18); // verify
	tilemap_set_scrolly(state->bg_tilemap, 0, state->vidregs[11]); // verify

//  tilemap_set_scrollx(state->md_tilemap,0, state->vidregs[8] - 16); // rowscrolled
	tilemap_set_scrolly(state->md_tilemap,0, state->vidregs[9]); // verify

	tilemap_set_scrollx(state->fg_tilemap,0, state->vidregs[6] - 18); // verify (test mode colour test needs it)
	tilemap_set_scrolly(state->fg_tilemap,0, state->vidregs[7]); // verify

	rowscroll_bank = (state->vidregs[4] & 0x30) >> 4;

	for (y = 0; y < 1024; y++)
		tilemap_set_scrollx(state->md_tilemap, y, state->vidregs[8] - 16 + state->rowscrollram[y + 0x800 * rowscroll_bank]);

	// todo: figure out which bits relate to the order
	switch (state->vidregs2[0])
	{
		case 0x2451: // fg unsure
		case 0x2d9a: // fg unsure
		case 0x2440: // all ok
		case 0x245a: // fg unsure, title screen
			tilemap_draw(bitmap, cliprect, state->fg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
			tilemap_draw(bitmap, cliprect, state->md_tilemap, 0, 0);
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
			break;
		case 0x23c0: // all ok
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
			tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
			tilemap_draw(bitmap, cliprect, state->md_tilemap, 0, 0);
			break;
		case 0x38da: // fg unsure
		case 0x215a: // fg unsure
		case 0x2140: // all ok
			tilemap_draw(bitmap, cliprect, state->fg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
			tilemap_draw(bitmap, cliprect, state->md_tilemap, 0, 0);
			break;
		case 0x2d80: // all ok
			tilemap_draw(bitmap, cliprect, state->md_tilemap, TILEMAP_DRAW_OPAQUE, 0);
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
			tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
			break;
		default:
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
			tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
			tilemap_draw(bitmap, cliprect, state->md_tilemap, 0, 0);
			logerror ("unknown video priority regs %04x\n", state->vidregs2[0]);

	}

	draw_sprites(screen->machine,bitmap,cliprect);

//  popmessage ("x %04x x %04x x %04x x %04x x %04x", state->vidregs2[0], state->vidregs[12], state->vidregs[13], state->vidregs[14], state->vidregs[15]);
//  popmessage ("x %04x x %04x y %04x y %04x z %04x z %04x",state->vidregs[0],state->vidregs[1],state->vidregs[2],state->vidregs[3],state->vidregs[4],state->vidregs[5]);

	return 0;
}
