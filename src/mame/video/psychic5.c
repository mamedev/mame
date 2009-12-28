/***************************************************************************
  Psychic 5
  Bombs Away

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "jalblend.h"

#define	BG_SCROLLX_LSB		0x308
#define	BG_SCROLLX_MSB		0x309
#define	BG_SCROLLY_LSB		0x30a
#define	BG_SCROLLY_MSB		0x30b
#define	BG_SCREEN_MODE		0x30c
#define	BG_PAL_INTENSITY_RG	0x1fe
#define	BG_PAL_INTENSITY_BU	0x1ff

static UINT8 ps5_vram_page;
static UINT8 bg_clip_mode;
static UINT8 title_screen;
static UINT8 psychic5_bg_status;

static UINT8 *ps5_pagedram[2];

/* Paged RAM 0 */
static UINT8 *psychic5_bg_videoram;
static UINT8 *ps5_dummy_bg_ram;

/* Paged RAM 1 */
static UINT8 *ps5_io_ram;
static UINT8 *ps5_palette_ram;
static UINT8 *psychic5_fg_videoram;

static tilemap_t *bg_tilemap, *fg_tilemap;

static int bg_palette_ram_base;
static int bg_palette_base;

static UINT16 palette_intensity;

static UINT8 bombsa_unknown;


/***************************************************************************
  Palette color
***************************************************************************/

static void psychic5_change_palette(running_machine *machine, int color, int offset)
{
	UINT8 lo = ps5_palette_ram[offset & ~1];
	UINT8 hi = ps5_palette_ram[offset | 1];
	if (jal_blend_table != NULL) jal_blend_table[color] = hi & 0x0f;
	palette_set_color_rgb(machine, color, pal4bit(lo >> 4), pal4bit(lo), pal4bit(hi >> 4));
}

static void psychic5_change_bg_palette(running_machine *machine, int color, int lo_offs, int hi_offs)
{
	UINT8 r,g,b,lo,hi,ir,ig,ib,ix;
	rgb_t irgb;

	/* red,green,blue intensities */
	ir = pal4bit(palette_intensity >> 12);
	ig = pal4bit(palette_intensity >>  8);
	ib = pal4bit(palette_intensity >>  4);
	ix = palette_intensity & 0x0f;

	irgb = MAKE_RGB(ir,ig,ib);

	lo = ps5_palette_ram[lo_offs];
	hi = ps5_palette_ram[hi_offs];

	/* red,green,blue component */
	r = pal4bit(lo >> 4);
	g = pal4bit(lo);
	b = pal4bit(hi >> 4);

	/* Grey background enable */
	if (psychic5_bg_status & 2)
	{
		UINT8 val = (r + g + b) / 3;		/* Grey */
		/* Just leave plain grey */
		palette_set_color(machine,color,jal_blend_func(MAKE_RGB(val,val,val),irgb,ix));
	}
	else
	{
		/* Seems fishy, but the title screen would be black otherwise... */
		if (!(title_screen & 1))
		{
			/* Leave the world as-is */
			palette_set_color(machine,color,jal_blend_func(MAKE_RGB(r,g,b),irgb,ix));
		}
	}
}

static void set_background_palette_intensity(running_machine *machine)
{
	int i;
	palette_intensity = ps5_palette_ram[BG_PAL_INTENSITY_BU] |
						(ps5_palette_ram[BG_PAL_INTENSITY_RG]<<8);

	/* for all of the background palette */
	for (i = 0; i < 0x100; i++)
		psychic5_change_bg_palette(machine,bg_palette_base+i,bg_palette_ram_base+i*2,bg_palette_ram_base+i*2+1);
}


/***************************************************************************
  Memory handler
***************************************************************************/

READ8_HANDLER( psychic5_vram_page_select_r )
{
	return ps5_vram_page;
}

WRITE8_HANDLER( psychic5_vram_page_select_w )
{
	ps5_vram_page = data & 1;
}

WRITE8_HANDLER( psychic5_title_screen_w )
{
	title_screen = data;
}

READ8_HANDLER( psychic5_paged_ram_r )
{
	if (ps5_vram_page == 1)
	{
		switch (offset)
		{
			case 0x00: return input_port_read(space->machine, "SYSTEM");
			case 0x01: return input_port_read(space->machine, "P1");
			case 0x02: return input_port_read(space->machine, "P2");
			case 0x03: return input_port_read(space->machine, "DSW1");
			case 0x04: return input_port_read(space->machine, "DSW2");
		}
	}

	return ps5_pagedram[ps5_vram_page][offset];
}

WRITE8_HANDLER( psychic5_paged_ram_w )
{
	ps5_pagedram[ps5_vram_page][offset] = data;

	if (ps5_vram_page == 0)
	{
		if (offset <= 0xfff)
			tilemap_mark_tile_dirty(bg_tilemap, offset >> 1);
	}
	else
	{
		if (offset == BG_SCROLLX_LSB || offset == BG_SCROLLX_MSB)
		{
			UINT16 bg_scrollx = ps5_io_ram[BG_SCROLLX_LSB] | (ps5_io_ram[BG_SCROLLX_MSB] << 8);
			tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
		}
		else if (offset == BG_SCROLLY_LSB || offset == BG_SCROLLY_MSB)
		{
			UINT16 bg_scrolly = ps5_io_ram[BG_SCROLLY_LSB] | (ps5_io_ram[BG_SCROLLY_MSB] << 8);
			tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);
		}
		else if (offset == BG_SCREEN_MODE)
		{
			psychic5_bg_status = ps5_io_ram[BG_SCREEN_MODE];
		}
		else if (offset >= 0x400 && offset <= 0x5ff)	/* Sprite color */
			psychic5_change_palette(space->machine,((offset >> 1) & 0xff)+0x000,offset-0x400);
		else if (offset >= 0x800 && offset <= 0x9ff)	/* BG color */
			psychic5_change_palette(space->machine,((offset >> 1) & 0xff)+0x100,offset-0x400);
		else if (offset >= 0xa00 && offset <= 0xbff)	/* Text color */
			psychic5_change_palette(space->machine,((offset >> 1) & 0xff)+0x200,offset-0x400);
		else if (offset >= 0x1000)
			tilemap_mark_tile_dirty(fg_tilemap, (offset-0x1000) >> 1);
	}
}

WRITE8_HANDLER( bombsa_paged_ram_w )
{
	ps5_pagedram[ps5_vram_page][offset] = data;

	if (ps5_vram_page == 0)
	{
		tilemap_mark_tile_dirty(bg_tilemap, offset >> 1);
	}
	else
	{
		if (offset == BG_SCROLLX_LSB || offset == BG_SCROLLX_MSB)
		{
			UINT16 bg_scrollx = ps5_io_ram[BG_SCROLLX_LSB] | (ps5_io_ram[BG_SCROLLX_MSB] << 8);
			tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
		}
		else if (offset == BG_SCROLLY_LSB || offset == BG_SCROLLY_MSB)
		{
			UINT16 bg_scrolly = ps5_io_ram[BG_SCROLLY_LSB] | (ps5_io_ram[BG_SCROLLY_MSB] << 8);
			tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);
		}
		else if (offset == BG_SCREEN_MODE)
		{
			psychic5_bg_status = ps5_io_ram[BG_SCREEN_MODE];
		}
		else if (offset >= 0x0800 && offset <= 0x0fff)
			tilemap_mark_tile_dirty(fg_tilemap, (offset & 0x7ff) >> 1);
		else if (offset >= 0x1000 && offset <= 0x15ff)
			psychic5_change_palette(space->machine, (offset >> 1) & 0x3ff, offset-0x1000);
	}
}

WRITE8_HANDLER( bombsa_unknown_w )
{
	bombsa_unknown = data;
}


/***************************************************************************
  Callbacks for the tilemap code
***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index << 1;
	int attr = psychic5_bg_videoram[offs + 1];
	int code = psychic5_bg_videoram[offs] | ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int offs = tile_index << 1;
	int attr = psychic5_fg_videoram[offs + 1];
	int code = psychic5_fg_videoram[offs] | ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(2, code, color, flags);
}


/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

VIDEO_START( psychic5 )
{
	/*                          info              offset             w   h  col  row */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,  8,  8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 15);

	ps5_pagedram[0] = auto_alloc_array(machine, UINT8, 0x2000);
	ps5_pagedram[1] = auto_alloc_array(machine, UINT8, 0x2000);

	psychic5_bg_videoram  = &ps5_pagedram[0][0x0000];
	ps5_dummy_bg_ram      = &ps5_pagedram[0][0x1000];
	ps5_io_ram            = &ps5_pagedram[1][0x0000];
	ps5_palette_ram       = &ps5_pagedram[1][0x0400];
	psychic5_fg_videoram  = &ps5_pagedram[1][0x1000];

	jal_blend_table = auto_alloc_array(machine, UINT8, 0xc00);

	bg_palette_ram_base = 0x400;
	bg_palette_base = 0x100;
}

VIDEO_START( bombsa )
{
	/*                          info              offset             w   h   col  row */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 128, 32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,  8,  8,  32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 15);

	ps5_pagedram[0] = auto_alloc_array(machine, UINT8, 0x2000);
	ps5_pagedram[1] = auto_alloc_array(machine, UINT8, 0x2000);

	psychic5_bg_videoram  = &ps5_pagedram[0][0x0000];
	ps5_dummy_bg_ram      = &ps5_pagedram[0][0x1000];
	ps5_io_ram            = &ps5_pagedram[1][0x0000];
	psychic5_fg_videoram  = &ps5_pagedram[1][0x0800];
	ps5_palette_ram       = &ps5_pagedram[1][0x1000];

	//jal_blend_table = auto_alloc_array(machine, UINT8, 0xc00);
	jal_blend_table = NULL;

	bg_palette_ram_base = 0x000;
	bg_palette_base = 0x000;
}

VIDEO_RESET( psychic5 )
{
	bg_clip_mode = 0;
	ps5_vram_page = 0;
	psychic5_bg_status = 0;
	memset(ps5_pagedram[0],0,0x2000);
	memset(ps5_pagedram[1],0,0x2000);
	palette_intensity = 0;
	if (jal_blend_table != NULL) memset(jal_blend_table,0,0xc00);
}

VIDEO_RESET( bombsa )
{
	ps5_vram_page = 0;
	psychic5_bg_status = 0;
	title_screen = 0;
	memset(ps5_pagedram[0],0,0x2000);
	memset(ps5_pagedram[1],0,0x2000);
	palette_intensity = 0;
	if (jal_blend_table != NULL) memset(jal_blend_table,0,0xc00);
}


/***************************************************************************
  Screen refresh
***************************************************************************/

#define DRAW_SPRITE(code, sx, sy) jal_blend_drawgfx(bitmap, cliprect, machine->gfx[0], code, color, flipx, flipy, sx, sy, 15);

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	/* Draw the sprites */
	for (offs = 0; offs < machine->generic.spriteram_size; offs += 16)
	{
		int attr  = spriteram[offs + 13];
		int code  = spriteram[offs + 14] | ((attr & 0xc0) << 2);
		int color = spriteram[offs + 15] & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[offs + 12];
		int sy = spriteram[offs + 11];
		int size = (attr & 0x08) ? 32:16;

		if (attr & 0x01) sx -= 256;
		if (attr & 0x04) sy -= 256;

		if (flip_screen_get(machine))
		{
			sx = 224 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (size == 32)
		{
			int x0,x1,y0,y1;

			if (flipx) { x0 = 2; x1 = 0; }
			else { x0 = 0; x1 = 2; }

			if (flipy) { y0 = 1; y1 = 0; }
			else { y0 = 0; y1 = 1; }

			DRAW_SPRITE(code + x0 + y0, sx, sy)
			DRAW_SPRITE(code + x0 + y1, sx, sy + 16)
			DRAW_SPRITE(code + x1 + y0, sx + 16, sy)
			DRAW_SPRITE(code + x1 + y1, sx + 16, sy + 16)
		}
		else
		{
			if (flip_screen_get(machine))
				DRAW_SPRITE(code, sx + 16, sy + 16)
			else
				DRAW_SPRITE(code, sx, sy)
		}
	}
}

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	static int sx1 = 0, sy1 = 0, sy2 = 0;

	rectangle clip = *cliprect;

	set_background_palette_intensity(machine);

	if (!(title_screen & 1))
	{
		bg_clip_mode = 0;
		sx1 = sy1 = sy2 = 0;
	}
	else
	{
		int sy1_old = sy1;
		int sx1_old = sx1;
		int sy2_old = sy2;

		sy1 = spriteram[11];		/* sprite 0 */
		sx1 = spriteram[12];
		sy2 = spriteram[11+128];	/* sprite 8 */

		switch (bg_clip_mode)
		{
		case  0: case  4: if (sy1_old != sy1) bg_clip_mode++; break;
		case  2: case  6: if (sy2_old != sy2) bg_clip_mode++; break;
		case  8: case 10:
		case 12: case 14: if (sx1_old != sx1) bg_clip_mode++; break;
		case  1: case  5: if (sy1 == 0xf0) bg_clip_mode++; break;
		case  3: case  7: if (sy2 == 0xf0) bg_clip_mode++; break;
		case  9: case 11: if (sx1 == 0xf0) bg_clip_mode++; break;
		case 13: case 15: if (sx1_old == 0xf0) bg_clip_mode++;
		case 16: if (sy1 != 0x00) bg_clip_mode = 0; break;
		}

		switch (bg_clip_mode)
		{
		case  0: case  4: case  8: case 12: case 16:
			clip.min_x = clip.max_x = clip.min_y = clip.max_y = 0;
			break;
		case  1: clip.min_y = sy1; break;
		case  3: clip.max_y = sy2; break;
		case  5: clip.max_y = sy1; break;
		case  7: clip.min_y = sy2; break;
		case  9: case 15: clip.min_x = sx1; break;
		case 11: case 13: clip.max_x = sx1; break;
		}

		if (flip_screen_get(machine))
		{
			int min_x,max_x,min_y,max_y;
			min_x = 255 - clip.max_x;
			max_x = 255 - clip.min_x;
			min_y = 255 - clip.max_y;
			max_y = 255 - clip.min_y;
			clip.min_x = min_x;
			clip.max_x = max_x;
			clip.min_y = min_y;
			clip.max_y = max_y;
		}
	}

	tilemap_draw(bitmap, &clip, bg_tilemap, 0, 0);
}

VIDEO_UPDATE( psychic5 )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	if (psychic5_bg_status & 1)	/* Backgound enable */
		draw_background(screen->machine, bitmap, cliprect);
	if (!(title_screen & 1))
		draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( bombsa )
{
	if (psychic5_bg_status & 1)	/* Backgound enable */
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, screen->machine->pens[0x0ff]);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
