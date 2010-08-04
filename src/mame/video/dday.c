/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  Convention: "sl" stands for "searchlight"

***************************************************************************/

#include "emu.h"
#include "sound/ay8910.h"
#include "includes/dday.h"


/* Note: There seems to be no way to reset this timer via hardware.
         The game uses a difference method to reset it to 99.

  Thanks Zwaxy for the timer info. */

READ8_HANDLER( dday_countdown_timer_r )
{
	dday_state *state = space->machine->driver_data<dday_state>();
	return ((state->timer_value / 10) << 4) | (state->timer_value % 10);
}

static TIMER_CALLBACK( countdown_timer_callback )
{
	dday_state *state = machine->driver_data<dday_state>();
	state->timer_value--;

	if (state->timer_value < 0)
		state->timer_value = 99;
}

static void start_countdown_timer(running_machine *machine)
{
	dday_state *state = machine->driver_data<dday_state>();

	state->timer_value = 0;

	timer_pulse(machine, ATTOTIME_IN_SEC(1), NULL, 0, countdown_timer_callback);
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( dday )
{
	int i;

	palette_set_shadow_factor(machine, 1.0 / 8);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* HACK!!! This table is handgenerated, but it matches the screenshot.
       I have no clue how it really works */
	colortable_entry_set_value(machine->colortable, 0*8+0+0, 0x00);
	colortable_entry_set_value(machine->colortable, 0*8+0+1, 0x01);
	colortable_entry_set_value(machine->colortable, 0*8+0+2, 0x15);
	colortable_entry_set_value(machine->colortable, 0*8+0+3, 0x02);
	colortable_entry_set_value(machine->colortable, 0*8+4+0, 0x00);
	colortable_entry_set_value(machine->colortable, 0*8+4+1, 0x01);
	colortable_entry_set_value(machine->colortable, 0*8+4+2, 0x15);
	colortable_entry_set_value(machine->colortable, 0*8+4+3, 0x02);

	colortable_entry_set_value(machine->colortable, 1*8+0+0, 0x04);
	colortable_entry_set_value(machine->colortable, 1*8+0+1, 0x05);
	colortable_entry_set_value(machine->colortable, 1*8+0+2, 0x03);
	colortable_entry_set_value(machine->colortable, 1*8+0+3, 0x07);
	colortable_entry_set_value(machine->colortable, 1*8+4+0, 0x04);
	colortable_entry_set_value(machine->colortable, 1*8+4+1, 0x05);
	colortable_entry_set_value(machine->colortable, 1*8+4+2, 0x03);
	colortable_entry_set_value(machine->colortable, 1*8+4+3, 0x07);

	colortable_entry_set_value(machine->colortable, 2*8+0+0, 0x08);
	colortable_entry_set_value(machine->colortable, 2*8+0+1, 0x15);
	colortable_entry_set_value(machine->colortable, 2*8+0+2, 0x0a);
	colortable_entry_set_value(machine->colortable, 2*8+0+3, 0x03);
	colortable_entry_set_value(machine->colortable, 2*8+4+0, 0x08);
	colortable_entry_set_value(machine->colortable, 2*8+4+1, 0x15);
	colortable_entry_set_value(machine->colortable, 2*8+4+2, 0x0a);
	colortable_entry_set_value(machine->colortable, 2*8+4+3, 0x03);

	colortable_entry_set_value(machine->colortable, 3*8+0+0, 0x08);
	colortable_entry_set_value(machine->colortable, 3*8+0+1, 0x15);
	colortable_entry_set_value(machine->colortable, 3*8+0+2, 0x0a);
	colortable_entry_set_value(machine->colortable, 3*8+0+3, 0x03);
	colortable_entry_set_value(machine->colortable, 3*8+4+0, 0x08);
	colortable_entry_set_value(machine->colortable, 3*8+4+1, 0x15);
	colortable_entry_set_value(machine->colortable, 3*8+4+2, 0x0a);
	colortable_entry_set_value(machine->colortable, 3*8+4+3, 0x03);

	colortable_entry_set_value(machine->colortable, 4*8+0+0, 0x10);
	colortable_entry_set_value(machine->colortable, 4*8+0+1, 0x11);
	colortable_entry_set_value(machine->colortable, 4*8+0+2, 0x12);
	colortable_entry_set_value(machine->colortable, 4*8+0+3, 0x07);
	colortable_entry_set_value(machine->colortable, 4*8+4+0, 0x10);
	colortable_entry_set_value(machine->colortable, 4*8+4+1, 0x11);
	colortable_entry_set_value(machine->colortable, 4*8+4+2, 0x12);
	colortable_entry_set_value(machine->colortable, 4*8+4+3, 0x07);

	colortable_entry_set_value(machine->colortable, 5*8+0+0, 0x1d);
	colortable_entry_set_value(machine->colortable, 5*8+0+1, 0x15);
	colortable_entry_set_value(machine->colortable, 5*8+0+2, 0x16);
	colortable_entry_set_value(machine->colortable, 5*8+0+3, 0x1b);
	colortable_entry_set_value(machine->colortable, 5*8+4+0, 0x1d);
	colortable_entry_set_value(machine->colortable, 5*8+4+1, 0x15);
	colortable_entry_set_value(machine->colortable, 5*8+4+2, 0x16);
	colortable_entry_set_value(machine->colortable, 5*8+4+3, 0x1b);

	colortable_entry_set_value(machine->colortable, 6*8+0+0, 0x1d);
	colortable_entry_set_value(machine->colortable, 6*8+0+1, 0x15);
	colortable_entry_set_value(machine->colortable, 6*8+0+2, 0x1a);
	colortable_entry_set_value(machine->colortable, 6*8+0+3, 0x1b);
	colortable_entry_set_value(machine->colortable, 6*8+4+0, 0x1d);
	colortable_entry_set_value(machine->colortable, 6*8+4+1, 0x15);
	colortable_entry_set_value(machine->colortable, 6*8+4+2, 0x1a);
	colortable_entry_set_value(machine->colortable, 6*8+4+3, 0x1b);

	colortable_entry_set_value(machine->colortable, 7*8+0+0, 0x1d);
	colortable_entry_set_value(machine->colortable, 7*8+0+1, 0x02);
	colortable_entry_set_value(machine->colortable, 7*8+0+2, 0x04);
	colortable_entry_set_value(machine->colortable, 7*8+0+3, 0x1b);
	colortable_entry_set_value(machine->colortable, 7*8+4+0, 0x1d);
	colortable_entry_set_value(machine->colortable, 7*8+4+1, 0x02);
	colortable_entry_set_value(machine->colortable, 7*8+4+2, 0x04);
	colortable_entry_set_value(machine->colortable, 7*8+4+3, 0x1b);
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	dday_state *state = machine->driver_data<dday_state>();
	int code;

	code = state->bgvideoram[tile_index];
	SET_TILE_INFO(0, code, code >> 5, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	dday_state *state = machine->driver_data<dday_state>();
	int code, flipx;

	flipx = state->colorram[tile_index & 0x03e0] & 0x01;
	code = state->fgvideoram[flipx ? tile_index ^ 0x1f : tile_index];
	SET_TILE_INFO(2, code, code >> 5, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	dday_state *state = machine->driver_data<dday_state>();
	int code;

	code = state->textvideoram[tile_index];
	SET_TILE_INFO(1, code, code >> 5, 0);
}

static TILE_GET_INFO( get_sl_tile_info )
{
	dday_state *state = machine->driver_data<dday_state>();
	int code, sl_flipx, flipx;
	UINT8* sl_map;

	sl_map = &memory_region(machine, "user1")[(state->sl_image & 0x07) * 0x0200];

	flipx = (tile_index >> 4) & 0x01;
	sl_flipx = (state->sl_image >> 3) & 0x01;

	/* bit 4 is really a flip indicator.  Need to shift bits 5-9 to the right by 1 */
	tile_index = ((tile_index & 0x03e0) >> 1) | (tile_index & 0x0f);

	code = sl_map[flipx ? tile_index ^ 0x0f : tile_index];

	if ((sl_flipx != flipx) && (code & 0x80))
		/* no mirroring, draw dark spot */
		code = 1;

	SET_TILE_INFO(3, code & 0x3f, 0, flipx ? TILE_FLIPX : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( dday )
{
	dday_state *state = machine->driver_data<dday_state>();
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->text_tilemap = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->sl_tilemap = tilemap_create(machine, get_sl_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->main_bitmap = machine->primary_screen->alloc_compatible_bitmap();

	tilemap_set_transmask(state->bg_tilemap, 0, 0x00f0, 0xff0f); /* pens 0-3 have priority over the foreground layer */
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_transparent_pen(state->text_tilemap, 0);

	start_countdown_timer(machine);
}

WRITE8_HANDLER( dday_bgvideoram_w )
{
	dday_state *state = space->machine->driver_data<dday_state>();

	state->bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( dday_fgvideoram_w )
{
	dday_state *state = space->machine->driver_data<dday_state>();

	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset ^ 0x1f);  /* for flipx case */
}

WRITE8_HANDLER( dday_textvideoram_w )
{
	dday_state *state = space->machine->driver_data<dday_state>();

	state->textvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->text_tilemap, offset);
}

WRITE8_HANDLER( dday_colorram_w )
{
	dday_state *state = space->machine->driver_data<dday_state>();
	int i;

	offset &= 0x03e0;

	state->colorram[offset & 0x3e0] = data;

	for (i = 0; i < 0x20; i++)
		tilemap_mark_tile_dirty(state->fg_tilemap, offset + i);
}

READ8_HANDLER( dday_colorram_r )
{
	dday_state *state = space->machine->driver_data<dday_state>();
	return state->colorram[offset & 0x03e0];
}


WRITE8_HANDLER( dday_sl_control_w )
{
	dday_state *state = space->machine->driver_data<dday_state>();

	if (state->sl_image != data)
	{
		state->sl_image = data;
		tilemap_mark_all_tiles_dirty(state->sl_tilemap);
	}
}


WRITE8_HANDLER( dday_control_w )
{
	dday_state *state = space->machine->driver_data<dday_state>();

	//if (data & 0xac)  logerror("Control = %02X\n", data & 0xac);

	/* bit 0 is coin counter 1 */
	coin_counter_w(space->machine, 0, data & 0x01);

	/* bit 1 is coin counter 2 */
	coin_counter_w(space->machine, 1, data & 0x02);

	/* bit 4 is sound enable */
	if (!(data & 0x10) && (state->control & 0x10))
		state->ay1->reset();

	sound_global_enable(space->machine, data & 0x10);

	/* bit 6 is search light enable */
	state->sl_enable = data & 0x40;

	state->control = data;
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( dday )
{
	dday_state *state = screen->machine->driver_data<dday_state>();

	tilemap_draw(state->main_bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(state->main_bitmap, cliprect, state->fg_tilemap, 0, 0);
	tilemap_draw(state->main_bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0, 0);
	tilemap_draw(state->main_bitmap, cliprect, state->text_tilemap, 0, 0);

	if (state->sl_enable)
	{
		/* apply shadow */
		int x, y;

		bitmap_t *sl_bitmap = tilemap_get_pixmap(state->sl_tilemap);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			for (y = cliprect->min_y; y <= cliprect->max_y; y++)
			{
				UINT16 src_pixel = *BITMAP_ADDR16(state->main_bitmap, y, x);

				if (*BITMAP_ADDR16(sl_bitmap, y, x) == 0xff)
					src_pixel += screen->machine->total_colors();

				*BITMAP_ADDR16(bitmap, y, x) = src_pixel;
			}
	}
	else
		copybitmap(bitmap, state->main_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
