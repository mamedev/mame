#include "driver.h"


UINT16 *deniam_videoram,*deniam_textram;
static int display_enable;
static int bg_scrollx_offs,bg_scrolly_offs,fg_scrollx_offs,fg_scrolly_offs;
static int bg_scrollx_reg,bg_scrolly_reg,bg_page_reg;
static int fg_scrollx_reg,fg_scrolly_reg,fg_page_reg;
static int bg_page[4],fg_page[4];

static tilemap *bg_tilemap,*fg_tilemap,*tx_tilemap;



DRIVER_INIT( logicpro )
{
	bg_scrollx_reg = 0x00a4/2;
	bg_scrolly_reg = 0x00a8/2;
	bg_page_reg    = 0x00ac/2;
	fg_scrollx_reg = 0x00a2/2;
	fg_scrolly_reg = 0x00a6/2;
	fg_page_reg    = 0x00aa/2;

	bg_scrollx_offs = 0x00d;
	bg_scrolly_offs = 0x000;
	fg_scrollx_offs = 0x009;
	fg_scrolly_offs = 0x000;
}
DRIVER_INIT( karianx )
{
	bg_scrollx_reg = 0x00a4/2;
	bg_scrolly_reg = 0x00a8/2;
	bg_page_reg    = 0x00ac/2;
	fg_scrollx_reg = 0x00a2/2;
	fg_scrolly_reg = 0x00a6/2;
	fg_page_reg    = 0x00aa/2;

	bg_scrollx_offs = 0x10d;
	bg_scrolly_offs = 0x080;
	fg_scrollx_offs = 0x109;
	fg_scrolly_offs = 0x080;
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
	int page = tile_index >> 11;
	UINT16 attr = deniam_videoram[bg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO(
			0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int page = tile_index >> 11;
	UINT16 attr = deniam_videoram[fg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	SET_TILE_INFO(
			0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	UINT16 attr = deniam_textram[tile_index];
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
	bg_tilemap = tilemap_create(get_bg_tile_info,scan_pages,       TILEMAP_TYPE_PEN,     8,8,128,64);
	fg_tilemap = tilemap_create(get_fg_tile_info,scan_pages,       TILEMAP_TYPE_PEN,8,8,128,64);
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8, 64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(tx_tilemap,0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( deniam_videoram_w )
{
	int page,i;
	COMBINE_DATA(&deniam_videoram[offset]);

	page = offset >> 11;
	for (i = 0;i < 4;i++)
	{
		if (bg_page[i] == page)
			tilemap_mark_tile_dirty(bg_tilemap,i * 0x800 + (offset & 0x7ff));
		if (fg_page[i] == page)
			tilemap_mark_tile_dirty(fg_tilemap,i * 0x800 + (offset & 0x7ff));
	}
}


WRITE16_HANDLER( deniam_textram_w )
{
	COMBINE_DATA(&deniam_textram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}


WRITE16_HANDLER( deniam_palette_w )
{
	int r,g,b;

	data = COMBINE_DATA(&paletteram16[offset]);

	r = ((data << 1) & 0x1e) | ((data >> 12) & 0x01);
	g = ((data >> 3) & 0x1e) | ((data >> 13) & 0x01);
	b = ((data >> 7) & 0x1e) | ((data >> 14) & 0x01);
	palette_set_color_rgb(Machine,offset,pal5bit(r),pal5bit(g),pal5bit(b));
}


static UINT16 coinctrl;

READ16_HANDLER( deniam_coinctrl_r )
{
	return coinctrl;
}

WRITE16_HANDLER( deniam_coinctrl_w )
{
	COMBINE_DATA(&coinctrl);

	/* bit 0 is coin counter */
	coin_counter_w(0,coinctrl & 0x01);

	/* bit 6 is display enable (0 freezes screen) */
	display_enable = coinctrl & 0x20;

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
static void draw_sprites(running_machine* machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size/2-8;offs >= 0;offs -= 8)
	{
		int sx,starty,endy,x,y,start,color,width,flipx,primask;
		UINT8 *rom = memory_region(REGION_GFX2);


		sx = (spriteram16[offs+1] & 0x01ff) + 16*8 - 1;
		if (sx >= 512) sx -= 512;
		starty = spriteram16[offs+0] & 0xff;
		endy = spriteram16[offs+0] >> 8;

		width = spriteram16[offs+2] & 0x007f;
		flipx = spriteram16[offs+2] & 0x0100;
		if (flipx) sx++;

		color = 0x40 + (spriteram16[offs+4] & 0x3f);

		primask = 8;
		switch (spriteram16[offs+4] & 0xc0)
		{
			case 0x00: primask |= 4|2|1; break;	/* below everything */
			case 0x40: primask |= 4|2;   break;	/* below fg and tx */
			case 0x80: primask |= 4;     break;	/* below tx */
			case 0xc0:                   break;	/* above everything */
		}


		start = spriteram16[offs+3] + ((spriteram16[offs+4] & 0x1f00) << 8);
		rom += 2*start;

		for (y = starty+1;y <= endy;y++)
		{
			int drawing = 0;
			int i=0;

			rom += 2*width;	/* note that the first line is skipped */
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
							if (sx+x >= cliprect->min_x && sx+x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(priority_bitmap, y, sx+x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx+x) = machine->pens[color*16+(rom[i]&0x0f)];
								*BITMAP_ADDR8(priority_bitmap, y, sx+x) = 8;
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
							if (sx+x >= cliprect->min_x && sx+x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(priority_bitmap, y, sx+x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx+x) = machine->pens[color*16+(rom[i]>>4)];
								*BITMAP_ADDR8(priority_bitmap, y, sx+x) = 8;
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
							if (sx+x >= cliprect->min_x && sx+x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(priority_bitmap, y, sx+x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx+x) = machine->pens[color*16+(rom[i]>>4)];
								*BITMAP_ADDR8(priority_bitmap, y, sx+x) = 8;
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
							if (sx+x >= cliprect->min_x && sx+x <= cliprect->max_x &&
								y >= cliprect->min_y && y <= cliprect->max_y)
							{
								if ((*BITMAP_ADDR8(priority_bitmap, y, sx+x) & primask) == 0)
									*BITMAP_ADDR16(bitmap, y, sx+x) = machine->pens[color*16+(rom[i]&0x0f)];
								*BITMAP_ADDR8(priority_bitmap, y, sx+x) = 8;
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

static void set_bg_page(int page,int value)
{
	int tile_index;

	if (bg_page[page] != value)
	{
		bg_page[page] = value;
		for (tile_index = page * 0x800;tile_index < (page+1)*0x800;tile_index++)
			tilemap_mark_tile_dirty(bg_tilemap,tile_index);
	}
}

static void set_fg_page(int page,int value)
{
	int tile_index;

	if (fg_page[page] != value)
	{
		fg_page[page] = value;
		for (tile_index = page * 0x800;tile_index < (page+1)*0x800;tile_index++)
			tilemap_mark_tile_dirty(fg_tilemap,tile_index);
	}
}

VIDEO_UPDATE( deniam )
{
	int bg_scrollx,bg_scrolly,fg_scrollx,fg_scrolly;
	int page;


	if (!display_enable) return 0;	/* don't update (freeze display) */

	bg_scrollx = deniam_textram[bg_scrollx_reg] - bg_scrollx_offs;
	bg_scrolly = (deniam_textram[bg_scrolly_reg] & 0xff) - bg_scrolly_offs;
	page = deniam_textram[bg_page_reg];
	set_bg_page(3,(page >>12) & 0x0f);
	set_bg_page(2,(page >> 8) & 0x0f);
	set_bg_page(1,(page >> 4) & 0x0f);
	set_bg_page(0,(page >> 0) & 0x0f);

	fg_scrollx = deniam_textram[fg_scrollx_reg] - fg_scrollx_offs;
	fg_scrolly = (deniam_textram[fg_scrolly_reg] & 0xff) - fg_scrolly_offs;
	page = deniam_textram[fg_page_reg];
	set_fg_page(3,(page >>12) & 0x0f);
	set_fg_page(2,(page >> 8) & 0x0f);
	set_fg_page(1,(page >> 4) & 0x0f);
	set_fg_page(0,(page >> 0) & 0x0f);

	tilemap_set_scrollx(bg_tilemap,0,bg_scrollx & 0x1ff);
	tilemap_set_scrolly(bg_tilemap,0,bg_scrolly & 0x0ff);
	tilemap_set_scrollx(fg_tilemap,0,fg_scrollx & 0x1ff);
	tilemap_set_scrolly(fg_tilemap,0,fg_scrolly & 0x0ff);

	fillbitmap(priority_bitmap,0,cliprect);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,1);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,2);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,4);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
