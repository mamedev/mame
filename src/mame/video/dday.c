/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  Convention: "sl" stands for "searchlight"

***************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"


UINT8 *dday_bgvideoram;
UINT8 *dday_fgvideoram;
UINT8 *dday_textvideoram;
UINT8 *dday_colorram;

static tilemap *fg_tilemap, *bg_tilemap, *text_tilemap, *sl_tilemap;
static bitmap_t *main_bitmap;
static int control;
static int sl_image;
static int sl_enable;
static int timer_value;


/* Note: There seems to be no way to reset this timer via hardware.
         The game uses a difference method to reset it to 99.

  Thanks Zwaxy for the timer info. */

READ8_HANDLER( dday_countdown_timer_r )
{
	return ((timer_value / 10) << 4) | (timer_value % 10);
}

static TIMER_CALLBACK( countdown_timer_callback )
{
	timer_value--;

	if (timer_value < 0)
		timer_value = 99;
}

static void start_countdown_timer(running_machine *machine)
{
	timer_value = 0;

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
	int code;

	code = dday_bgvideoram[tile_index];
	SET_TILE_INFO(0, code, code >> 5, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, flipx;

	flipx = dday_colorram[tile_index & 0x03e0] & 0x01;
	code = dday_fgvideoram[flipx ? tile_index ^ 0x1f : tile_index];
	SET_TILE_INFO(2, code, code >> 5, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int code;

	code = dday_textvideoram[tile_index];
	SET_TILE_INFO(1, code, code >> 5, 0);
}

static TILE_GET_INFO( get_sl_tile_info )
{
	int code, sl_flipx, flipx;
	UINT8* sl_map;

	sl_map = &memory_region(machine, "user1")[(sl_image & 0x07) * 0x0200];

	flipx = (tile_index >> 4) & 0x01;
	sl_flipx = (sl_image >> 3) & 0x01;

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
	bg_tilemap   = tilemap_create(machine, get_bg_tile_info,  tilemap_scan_rows,8,8,32,32);
	fg_tilemap   = tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows,8,8,32,32);
	text_tilemap = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,32,32);
	sl_tilemap   = tilemap_create(machine, get_sl_tile_info,  tilemap_scan_rows,8,8,32,32);

	main_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);

	tilemap_set_transmask(bg_tilemap,0,0x00f0,0xff0f); /* pens 0-3 have priority over the foreground layer */

	tilemap_set_transparent_pen(fg_tilemap, 0);

	tilemap_set_transparent_pen(text_tilemap, 0);

	control = 0;
	sl_enable = 0;
	sl_image = 0;

	start_countdown_timer(machine);
}

WRITE8_HANDLER( dday_bgvideoram_w )
{
	dday_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( dday_fgvideoram_w )
{
	dday_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
	tilemap_mark_tile_dirty(fg_tilemap, offset ^ 0x1f);  /* for flipx case */
}

WRITE8_HANDLER( dday_textvideoram_w )
{
	dday_textvideoram[offset] = data;
	tilemap_mark_tile_dirty(text_tilemap, offset);
}

WRITE8_HANDLER( dday_colorram_w )
{
	int i;

	offset &= 0x03e0;

	dday_colorram[offset & 0x3e0] = data;

	for (i = 0; i < 0x20; i++)
		tilemap_mark_tile_dirty(fg_tilemap, offset + i);
}

READ8_HANDLER( dday_colorram_r )
{
	return dday_colorram[offset & 0x03e0];
}


WRITE8_HANDLER( dday_sl_control_w )
{
	if (sl_image != data)
	{
		sl_image = data;

		tilemap_mark_all_tiles_dirty(sl_tilemap);
	}
}


WRITE8_HANDLER( dday_control_w )
{
	//if (data & 0xac)  logerror("Control = %02X\n", data & 0xac);

	/* bit 0 is coin counter 1 */
	coin_counter_w(0, data & 0x01);

	/* bit 1 is coin counter 2 */
	coin_counter_w(1, data & 0x02);

	/* bit 4 is sound enable */
	if (!(data & 0x10) && (control & 0x10))
		devtag_reset(space->machine, "ay1");

	sound_global_enable(space->machine, data & 0x10);

	/* bit 6 is search light enable */
	sl_enable = data & 0x40;

	control = data;
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( dday )
{
	tilemap_draw(main_bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	tilemap_draw(main_bitmap,cliprect,fg_tilemap,0,0);
	tilemap_draw(main_bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(main_bitmap,cliprect,text_tilemap,0,0);

	if (sl_enable)
	{
		/* apply shadow */
		int x, y;

		bitmap_t *sl_bitmap = tilemap_get_pixmap(sl_tilemap);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			for (y = cliprect->min_y; y <= cliprect->max_y; y++)
			{
				UINT16 src_pixel = *BITMAP_ADDR16(main_bitmap, y, x);

				if (*BITMAP_ADDR16(sl_bitmap, y, x) == 0xff)
					src_pixel += screen->machine->config->total_colors;

				*BITMAP_ADDR16(bitmap, y, x) = src_pixel;
			}
	}
	else
		copybitmap(bitmap,main_bitmap,0,0,0,0,cliprect);

	return 0;
}
