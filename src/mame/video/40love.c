/*
*   Video Driver for Forty-Love
*/

#include "driver.h"

/*
*   variables
*/

UINT8 *fortyl_video_ctrl;

static UINT8 fortyl_flipscreen,fortyl_pix_redraw;
static const UINT8 fortyl_xoffset = 128;

static UINT8 *fortyl_pixram1;
static UINT8 *fortyl_pixram2;

static bitmap_t *pixel_bitmap1;
static bitmap_t *pixel_bitmap2;

static tilemap *background;

int fortyl_pix_color[4];

static int pixram_sel;

/*
*   color prom decoding
*/

PALETTE_INIT( fortyl )
{
	int i;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->config->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[2*machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->config->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
colorram format (2 bytes per one tilemap character line, 8 pixels height):

    offset 0    x... ....   x scroll (1 MSB bit)
    offset 0    .xxx x...   tile bank (see code below for banking formula)
    offset 0    .... .xxx   tiles color (one color code per whole tilemap line)

    offset 1    xxxx xxxx   x scroll (8 LSB bits)
*/

static TILE_GET_INFO( get_bg_tile_info )
{
	int tile_number = videoram[tile_index];
	int tile_attrib = colorram[(tile_index/64)*2];
	int tile_h_bank = (tile_attrib&0x40)<<3;	/* 0x40->0x200 */
	int tile_l_bank = (tile_attrib&0x18)<<3;	/* 0x10->0x80, 0x08->0x40 */

	int code = tile_number;
	if ((tile_attrib & 0x20) && (code >= 0xc0))
		code = (code & 0x3f) | tile_l_bank | 0x100;
	code |= tile_h_bank;

	SET_TILE_INFO(	0,
			code,
			tile_attrib & 0x07,
			0);
}

/***************************************************************************

  State-related callbacks

***************************************************************************/

static STATE_POSTLOAD( redraw_pixels )
{
    fortyl_pix_redraw = 1;
    tilemap_mark_all_tiles_dirty(background);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( fortyl )
{
	fortyl_pixram1 = auto_alloc_array_clear(machine, UINT8, 0x4000);
	fortyl_pixram2 = auto_alloc_array_clear(machine, UINT8, 0x4000);

	pixel_bitmap1 = auto_bitmap_alloc(machine,256,256,video_screen_get_format(machine->primary_screen));
	pixel_bitmap2 = auto_bitmap_alloc(machine,256,256,video_screen_get_format(machine->primary_screen));

	background  = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8,8,64,32);

	tilemap_set_scroll_rows(background,32);
	tilemap_set_transparent_pen(background,0);

    state_save_register_global(machine, fortyl_flipscreen);
    state_save_register_global_array(machine, fortyl_pix_color);
    state_save_register_global_pointer(machine, fortyl_pixram1, 0x4000);
    state_save_register_global_pointer(machine, fortyl_pixram2, 0x4000);
    state_save_register_global_bitmap(machine, pixel_bitmap1);
    state_save_register_global_bitmap(machine, pixel_bitmap2);
    state_save_register_global(machine, pixram_sel);
    state_save_register_postload(machine, redraw_pixels, NULL);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

static void fortyl_set_scroll_x(int offset)
{
	int	i = offset & ~1;
	int x = ((colorram[i] & 0x80) << 1) | colorram[i+1];	/* 9 bits signed */

	if (fortyl_flipscreen)
		x += 0x51;
	else
		x -= 0x50;

	x &= 0x1ff;
	if (x&0x100) x -= 0x200;				/* sign extend */

	tilemap_set_scrollx(background, offset/2, x);
}

WRITE8_HANDLER( fortyl_pixram_sel_w )
{
	int offs;
	int f = data & 0x01;

	pixram_sel = (data & 0x04) >> 2;

	if (fortyl_flipscreen != f)
	{
		fortyl_flipscreen = f;
		flip_screen_set(space->machine, fortyl_flipscreen);
		fortyl_pix_redraw = 1;

		for (offs=0;offs<32;offs++)
			fortyl_set_scroll_x(offs*2);
	}
}

READ8_HANDLER( fortyl_pixram_r )
{
	if (pixram_sel)
		return fortyl_pixram2[offset];
	else
		return fortyl_pixram1[offset];
}

static void fortyl_plot_pix(int offset)
{
	int x,y,i,c,d1,d2;


	x = (offset & 0x1f)*8;
	y = (offset >> 5) & 0xff;

	if (pixram_sel)
	{
		d1 = fortyl_pixram2[offset];
		d2 = fortyl_pixram2[offset + 0x2000];
	}
	else
	{
		d1 = fortyl_pixram1[offset];
		d2 = fortyl_pixram1[offset + 0x2000];
	}

	for (i=0;i<8;i++)
	{
		c = ((d2>>i)&1) + ((d1>>i)&1)*2;
		if (pixram_sel)
			*BITMAP_ADDR16(pixel_bitmap2, y, x+i) = fortyl_pix_color[c];
		else
			*BITMAP_ADDR16(pixel_bitmap1, y, x+i) = fortyl_pix_color[c];
	}
}

WRITE8_HANDLER( fortyl_pixram_w )
{
	if (pixram_sel)
		fortyl_pixram2[offset] = data;
	else
		fortyl_pixram1[offset] = data;

	fortyl_plot_pix(offset & 0x1fff);
}


WRITE8_HANDLER( fortyl_bg_videoram_w )
{
	videoram[offset]=data;
	tilemap_mark_tile_dirty(background,offset);
}
READ8_HANDLER( fortyl_bg_videoram_r )
{
	return videoram[offset];
}

WRITE8_HANDLER( fortyl_bg_colorram_w )
{
	if( colorram[offset]!=data )
	{
		int i;

		colorram[offset] = data;
		for (i=(offset/2)*64; i<(offset/2)*64+64; i++)
			tilemap_mark_tile_dirty(background,i);

		fortyl_set_scroll_x(offset);
	}
}
READ8_HANDLER( fortyl_bg_colorram_r )
{
	return colorram[offset];
}

/***************************************************************************

  Display refresh

***************************************************************************/
/*
spriteram format (4 bytes per sprite):

    offset  0   xxxxxxxx    y position

    offset  1   x.......    flip Y
    offset  1   .x......    flip X
    offset  1   ..xxxxxx    gfx code (6 LSB bits)

    offset  2   ...xx...    gfx code (2 MSB bits)
    offset  2   .....xxx    color code
    offset  2   ???.....    ??? (not used, always 0)

    offset  3   xxxxxxxx    x position
*/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs;

	/* spriteram #1 */
	for (offs = 0; offs < spriteram_size; offs += 4)
	{
		int code,color,sx,sy,flipx,flipy;

		sx = spriteram[offs+3];
		sy = spriteram[offs+0] +1;

		if (fortyl_flipscreen)
			sx = 240 - sx;
		else
			sy = 242 - sy;

		code = (spriteram[offs+1] & 0x3f) + ((spriteram[offs+2] & 0x18) << 3);
		flipx = ((spriteram[offs+1] & 0x40) >> 6) ^ fortyl_flipscreen;
		flipy = ((spriteram[offs+1] & 0x80) >> 7) ^ fortyl_flipscreen;
		color = (spriteram[offs+2] & 0x07) + 0x08;

		if (spriteram[offs+2] & 0xe0)
			color = mame_rand(machine)&0xf;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx+fortyl_xoffset,sy,0);
	}

	/* spriteram #2 */
	for (offs = 0; offs < spriteram_2_size; offs += 4)
	{
		int code,color,sx,sy,flipx,flipy;

		sx = spriteram_2[offs+3];
		sy = spriteram_2[offs+0] +1;

		if (fortyl_flipscreen)
			sx = 240 - sx;
		else
			sy = 242 - sy;

		code = (spriteram_2[offs+1] & 0x3f) + ((spriteram_2[offs+2] & 0x18) << 3);
		flipx = ((spriteram_2[offs+1] & 0x40) >> 6) ^ fortyl_flipscreen;
		flipy = ((spriteram_2[offs+1] & 0x80) >> 7) ^ fortyl_flipscreen;
		color = (spriteram_2[offs+2] & 0x07) + 0x08;

		if (spriteram_2[offs+2] & 0xe0)
			color = mame_rand(machine)&0xf;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx+fortyl_xoffset,sy,0);
	}
}

static void draw_pixram( bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs;
	int f = fortyl_flipscreen ^ 1;

	if (fortyl_pix_redraw)
	{
		fortyl_pix_redraw = 0;

		for (offs=0; offs<0x2000; offs++)
			fortyl_plot_pix(offs);
	}

	if (pixram_sel)
		copybitmap(bitmap,pixel_bitmap1,f,f,fortyl_xoffset,0,cliprect);
	else
		copybitmap(bitmap,pixel_bitmap2,f,f,fortyl_xoffset,0,cliprect);
}

VIDEO_UPDATE( fortyl )
{
	draw_pixram(bitmap,cliprect);

	tilemap_set_scrolldy(background,-fortyl_video_ctrl[1]+1,-fortyl_video_ctrl[1]-1 );
	tilemap_draw(bitmap,cliprect,background,0,0);

	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
