/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  Convention: "sl" stands for "searchlight"

***************************************************************************/

#include "driver.h"


UINT8 *dday_bgvideoram;
UINT8 *dday_fgvideoram;
UINT8 *dday_textvideoram;
UINT8 *dday_colorram;

static tilemap *fg_tilemap, *bg_tilemap, *text_tilemap, *sl_tilemap;
static mame_bitmap *main_bitmap;
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
	{
		timer_value = 99;
	}
}

static void start_countdown_timer(void)
{
	timer_value = 0;

	timer_pulse(ATTOTIME_IN_SEC(1), NULL, 0, countdown_timer_callback);
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( dday )
{
	int i;


	palette_set_shadow_factor(machine, 1.0/8);	/* this matches the previous version of the driver (>>3) */

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}


	/* HACK!!! This table is handgenerated, but it matches the screenshot.
       I have no clue how it really works */

	colortable[0*8+0] = 0;
	colortable[0*8+1] = 1;
	colortable[0*8+2] = 21;
	colortable[0*8+3] = 2;
	colortable[0*8+4+0] = 0;
	colortable[0*8+4+1] = 1;
	colortable[0*8+4+2] = 21;
	colortable[0*8+4+3] = 2;

	colortable[1*8+0] = 4;
	colortable[1*8+1] = 5;
	colortable[1*8+2] = 3;
	colortable[1*8+3] = 7;
	colortable[1*8+4+0] = 4;
	colortable[1*8+4+1] = 5;
	colortable[1*8+4+2] = 3;
	colortable[1*8+4+3] = 7;

	colortable[2*8+0] = 8;
	colortable[2*8+1] = 21;
	colortable[2*8+2] = 10;
	colortable[2*8+3] = 3;
	colortable[2*8+4+0] = 8;
	colortable[2*8+4+1] = 21;
	colortable[2*8+4+2] = 10;
	colortable[2*8+4+3] = 3;

	colortable[3*8+0] = 8;
	colortable[3*8+1] = 21;
	colortable[3*8+2] = 10;
	colortable[3*8+3] = 3;
	colortable[3*8+4+0] = 8;
	colortable[3*8+4+1] = 21;
	colortable[3*8+4+2] = 10;
	colortable[3*8+4+3] = 3;

	colortable[4*8+0] = 16;
	colortable[4*8+1] = 17;
	colortable[4*8+2] = 18;
	colortable[4*8+3] = 7;
	colortable[4*8+4+0] = 16;
	colortable[4*8+4+1] = 17;
	colortable[4*8+4+2] = 18;
	colortable[4*8+4+3] = 7;

	colortable[5*8+0] = 29;
	colortable[5*8+1] = 21;
	colortable[5*8+2] = 22;
	colortable[5*8+3] = 27;
	colortable[5*8+4+0] = 29;
	colortable[5*8+4+1] = 21;
	colortable[5*8+4+2] = 22;
	colortable[5*8+4+3] = 27;

	colortable[6*8+0] = 29;
	colortable[6*8+1] = 21;
	colortable[6*8+2] = 26;
	colortable[6*8+3] = 27;
	colortable[6*8+4+0] = 29;
	colortable[6*8+4+1] = 21;
	colortable[6*8+4+2] = 26;
	colortable[6*8+4+3] = 27;

	colortable[7*8+0] = 29;
	colortable[7*8+1] = 2;
	colortable[7*8+2] = 4;
	colortable[7*8+3] = 27;
	colortable[7*8+4+0] = 29;
	colortable[7*8+4+1] = 2;
	colortable[7*8+4+2] = 4;
	colortable[7*8+4+3] = 27;
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

	sl_map = &memory_region(REGION_USER1)[(sl_image & 0x07) * 0x0200];

	flipx = (tile_index >> 4) & 0x01;
	sl_flipx = (sl_image >> 3) & 0x01;

	/* bit 4 is really a flip indicator.  Need to shift bits 5-9 to the right by 1 */
	tile_index = ((tile_index & 0x03e0) >> 1) | (tile_index & 0x0f);

	code = sl_map[flipx ? tile_index ^ 0x0f : tile_index];

	if (sl_flipx != flipx)
	{
		if (code & 0x80)
		{
			/* no mirroring, draw dark spot */
			code = 1;
		}
	}

	SET_TILE_INFO(3, code & 0x3f, 0, flipx ? TILE_FLIPX : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( dday )
{
	bg_tilemap   = tilemap_create(get_bg_tile_info,  tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	fg_tilemap   = tilemap_create(get_fg_tile_info,  tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	text_tilemap = tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	sl_tilemap   = tilemap_create(get_sl_tile_info,  tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	main_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	tilemap_set_transmask(bg_tilemap,0,0x00f0,0xff0f); /* pens 0-3 have priority over the foreground layer */

	tilemap_set_transparent_pen(fg_tilemap, 0);

	tilemap_set_transparent_pen(text_tilemap, 0);

	control = 0;
	sl_enable = 0;
	sl_image = 0;

	start_countdown_timer();
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
    {
		tilemap_mark_tile_dirty(fg_tilemap, offset + i);
	}
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
		sndti_reset(SOUND_AY8910, 0);

	sound_global_enable(data & 0x10);

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

		mame_bitmap *sl_bitmap = tilemap_get_pixmap(sl_tilemap);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			for (y = cliprect->min_y; y <= cliprect->max_y; y++)
			{
				UINT16 src_pixel = *BITMAP_ADDR16(main_bitmap, y, x);

				if (*BITMAP_ADDR16(sl_bitmap, y, x) == 0xff)
					src_pixel += machine->drv->total_colors;

				*BITMAP_ADDR16(bitmap, y, x) = src_pixel;
			}
	}
	else
		copybitmap(bitmap,main_bitmap,0,0,0,0,cliprect);

	return 0;
}
