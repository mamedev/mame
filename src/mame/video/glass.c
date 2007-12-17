/***************************************************************************

  Glass Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "driver.h"

UINT16 *glass_spriteram;
UINT16 *glass_vregs;
UINT16 *glass_videoram;

static tilemap *pant[2];
static mame_bitmap *screen_bitmap;

static int glass_blitter_serial_buffer[5];
static int current_command = 0;
int glass_current_bit = 0;

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | xxxxxxxx xxxxxxxx | code
      1  | -------- ---xxxxx | color (uses colors 0x200-0x3ff)
      1  | -------- --x----- | not used?
      1  | -------- -x------ | flip x
      1  | -------- x------- | flip y
      1  | xxxxxxxx -------- | not used
*/

static TILE_GET_INFO( get_tile_info_glass_screen0 )
{
	int data = glass_videoram[tile_index << 1];
	int data2 = glass_videoram[(tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	SET_TILE_INFO(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}


static TILE_GET_INFO( get_tile_info_glass_screen1 )
{
	int data = glass_videoram[(0x1000/2) + (tile_index << 1)];
	int data2 = glass_videoram[(0x1000/2) + (tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	SET_TILE_INFO(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}

/***************************************************************************

    Blitter

***************************************************************************/

/*
    The blitter is accessed writting 5 consecutive bits. The stream is: P0 P1 B2 B1 B0

    if P0 is set, the hardware selects the first half of ROM H9 (girls)
    if P1 is set, the hardware selects the second half of ROM H9 (boys)

    B2B1B0 selects the picture (there are 8 pictures in each half of the ROM)
*/

WRITE16_HANDLER( glass_blitter_w )
{
	glass_blitter_serial_buffer[glass_current_bit] = data & 0x01;
	glass_current_bit++;

	if (glass_current_bit == 5){
		current_command = (glass_blitter_serial_buffer[0] << 4) |
							(glass_blitter_serial_buffer[1] << 3) |
							(glass_blitter_serial_buffer[2] << 2) |
							(glass_blitter_serial_buffer[3] << 1) |
							(glass_blitter_serial_buffer[4] << 0);
		glass_current_bit = 0;

		/* fill the screen bitmap with the current picture */
		{
			int i, j;
			UINT8 *gfx = (UINT8 *)memory_region(REGION_GFX3);

			gfx = gfx + (current_command & 0x07)*0x10000 + (current_command & 0x08)*0x10000 + 0x140;

			if ((current_command & 0x18) != 0){
				for (j = 0; j < 200; j++){
					for (i = 0; i < 320; i++){
						int color = *gfx;
						gfx++;
						*BITMAP_ADDR16(screen_bitmap, j, i) = Machine->pens[color & 0xff];
					}
				}
			} else {
				fillbitmap(screen_bitmap, Machine->pens[0], 0);
			}
		}
	}
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_HANDLER( glass_vram_w )
{
	COMBINE_DATA(&glass_videoram[offset]);
	tilemap_mark_tile_dirty(pant[offset >> 11],((offset << 1) & 0x0fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( glass )
{
	pant[0] = tilemap_create(get_tile_info_glass_screen0,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	pant[1] = tilemap_create(get_tile_info_glass_screen1,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	screen_bitmap = auto_bitmap_alloc (320, 200, machine->screen[0].format);

	tilemap_set_transparent_pen(pant[0],0);
	tilemap_set_transparent_pen(pant[1],0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | -------x xxxxxxxx | x position
      2  | ---xxxx- -------- | sprite color (uses colors 0x100-0x1ff)
      2  | xx------ -------- | not used?
      3  | xxxxxxxx xxxxxxxx | sprite code
*/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int i;
	const gfx_element *gfx = machine->gfx[0];

	for (i = 3; i < (0x1000 - 6)/2; i += 4){
		int sx = glass_spriteram[i+2] & 0x01ff;
		int sy = (240 - (glass_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = glass_spriteram[i+3];
		int color = (glass_spriteram[i+2] & 0x1e00) >> 9;
		int attr = (glass_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;

		number = ((number & 0x03) << 14) | ((number & 0x0fffc) >> 2);

		drawgfx(bitmap,gfx,number,
				0x10 + (color & 0x0f),xflip,yflip,
				sx-0x0f,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
}

/***************************************************************************

    Display Refresh

****************************************************************************/

VIDEO_UPDATE( glass )
{
	/* set scroll registers */
	tilemap_set_scrolly(pant[0], 0, glass_vregs[0]);
	tilemap_set_scrollx(pant[0], 0, glass_vregs[1] + 0x04);
	tilemap_set_scrolly(pant[1], 0, glass_vregs[2]);
	tilemap_set_scrollx(pant[1], 0, glass_vregs[3]);

	/* draw layers + sprites */
	fillbitmap(bitmap, get_black_pen(machine), cliprect);
	copybitmap(bitmap,screen_bitmap,0,0,0x18,0x24,cliprect,TRANSPARENCY_NONE,0);
	tilemap_draw(bitmap,cliprect,pant[1],0,0);
	tilemap_draw(bitmap,cliprect,pant[0],0,0);
	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
