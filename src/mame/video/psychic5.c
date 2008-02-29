/***************************************************************************
  Psychic 5

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "jalblend.c"

#define	BG_SCROLLX_LSB		0x308
#define	BG_SCROLLX_MSB		0x309
#define	BG_SCROLLY_LSB		0x30a
#define	BG_SCROLLY_MSB		0x30b
#define	BG_SCREEN_MODE		0x30c
#define	BG_PAL_INTENSITY_RG	0x1fe
#define	BG_PAL_INTENSITY_BU	0x1ff

static int ps5_vram_page;
static int bg_clip_mode;
static int title_screen;

/* Paged RAM 0 */
static UINT8 *psychic5_bg_videoram;
static UINT8 *ps5_dummy_bg_ram;

/* Paged RAM 1 */
static UINT8 *ps5_io_ram;
static UINT8 *ps5_palette_ram;
static UINT8 *psychic5_fg_videoram;

static tilemap *bg_tilemap, *fg_tilemap;

MACHINE_RESET( psychic5 )
{
	bg_clip_mode = -10;
	flip_screen_set(0);
}

WRITE8_HANDLER( psychic5_vram_page_select_w )
{
	ps5_vram_page = data;
}

READ8_HANDLER( psychic5_vram_page_select_r )
{
	return ps5_vram_page;
}

WRITE8_HANDLER( psychic5_title_screen_w )
{
	title_screen = data & 0x01;
}

static void psychic5_paletteram_w(int color_offs, int offset, int data)
{
	int r,g,b,a,val;

	ps5_palette_ram[offset] = data;

	/* red component */
	r  = (ps5_palette_ram[offset & ~1] >> 4) & 0x0f ;

	/* green component */
	g  = (ps5_palette_ram[offset & ~1]) & 0x0f ;

	/* blue component */
	b  = (ps5_palette_ram[offset | 1] >> 4) & 0x0f ;

	/* "alpha" component */
	val  = (ps5_palette_ram[offset | 1] & 0x0f) & 0x0f ;
	a = (val << 4) | val ;

	jal_blend_table[(offset / 2)-color_offs] = a ;

	palette_set_color_rgb(Machine,(offset / 2)-color_offs,pal4bit(r),pal4bit(g),pal4bit(b));
}

static void set_background_palette_intensity(running_machine *machine)
{
	int i,r,g,b,val,lo,hi,ir,ig,ib,ix;

	/* red,green,blue intensites */
	ir = ps5_palette_ram[BG_PAL_INTENSITY_RG] >> 4 ;  ir = (ir << 4) | ir ;
	ig = ps5_palette_ram[BG_PAL_INTENSITY_RG] & 15 ;  ig = (ig << 4) | ig ;
	ib = ps5_palette_ram[BG_PAL_INTENSITY_BU] >> 4 ;  ib = (ib << 4) | ib ;
	ix = ps5_palette_ram[0x1ff] & 0x0f ;

	/* for all of the background palette */
	for (i = 0; i < 0x100; i++)
	{
		lo = ps5_palette_ram[0x400+i*2];
		hi = ps5_palette_ram[0x400+i*2+1];

		/* red component */
		val  = (lo  >> 4) & 0x0f;
		r = (val << 4) | val ;

		/* green component */
		val  = (lo  & 15) & 0x0f;
		g = (val << 4) | val ;

		/* blue component */
		val =  (hi  >> 4) & 0x0f;
		b = (val << 4) | val ;

		/* Grey background enable */
		if (ps5_io_ram[BG_SCREEN_MODE] & 0x02)
		{
			val = (UINT8)((r + g + b) / 3);		/* Grey */

			if (ix != 0x0)						/* Tint the grey */
			{
				UINT32 result = jal_blend_func(MAKE_RGB(val,val,val), MAKE_RGB(ir, ig, ib), jal_blend_table[0xff]) ;
				palette_set_color(machine, 0x100+i, result) ;
			}
			else								/* Just leave plain grey */
			{
				palette_set_color(machine,0x100+i,MAKE_RGB(val,val,val));
			}
		}
		else
		{
			/* Seems fishy, but the title screen would be black otherwise... */
			if (!title_screen)
			{
				if (ix != 0x0)		/* Tint the world */
				{
					UINT32 result = jal_blend_func(MAKE_RGB(r, g, b), MAKE_RGB(ir, ig, ib), jal_blend_table[0xff]) ;
					palette_set_color(machine, 0x100+i, result) ;
				}
				else				/* Leave the world as-is */
				{
					palette_set_color(machine,0x100+i,MAKE_RGB(r,g,b)) ;
				}
			}
		}
	}
}


static WRITE8_HANDLER( psychic5_bg_videoram_w )
{
	psychic5_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

static WRITE8_HANDLER( psychic5_fg_videoram_w )
{
	psychic5_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset / 2);
}

READ8_HANDLER( psychic5_paged_ram_r )
{
	int val;

	if (!ps5_vram_page)
	{
		if (offset < 0x1000)
			return psychic5_bg_videoram[offset];
		else
			return ps5_dummy_bg_ram[offset & 0xfff];
	}
	else
	{
		if (offset < 0x400)
		{
			val = 0;
			switch(offset)
			{
				case 0x00:
					val = input_port_0_r(0);
					break;
				case 0x01:
					val = input_port_1_r(0);
					break;
				case 0x02:
					val = input_port_2_r(0);
					break;
				case 0x03:
					val = input_port_3_r(0);
					break;
				case 0x04:
					val = input_port_4_r(0);
					break;
				default:
					val = ps5_io_ram[offset];
			}
			return (val);
		}
		else if (offset < 0x1000)
		{
			return ps5_palette_ram[offset-0x400];
		}
		else
		{
			return psychic5_fg_videoram[offset & 0xfff];
		}
	}
	return 0;
}

WRITE8_HANDLER( psychic5_paged_ram_w )
{
	if (!ps5_vram_page)
	{
		if (offset < 0x1000)
			psychic5_bg_videoram_w(offset,data);
		else
			ps5_dummy_bg_ram[offset & 0xfff] = data;
	}
	else
	{
		if (offset < 0x400)
		{
			ps5_io_ram[offset] = data;
		}
		else if (offset < 0x600)
		{
			psychic5_paletteram_w(000, offset-0x400, data);
		}
		else if (offset > 0x5ff && offset< 0x800)
		{
			ps5_palette_ram[offset-0x400] = data;
		}
		else if (offset > 0x7ff && offset < 0xa00)
		{
			psychic5_paletteram_w(256, offset-0x400, data);
		}
		else if (offset > 0x9ff && offset < 0xc00)
		{
			psychic5_paletteram_w(256, offset-0x400, data);
		}
		else if (offset < 0x1000)
		{
			ps5_palette_ram[offset-0x400] = data;
		}
		else
		{
			psychic5_fg_videoram_w(offset & 0xfff, data);
		}
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = psychic5_bg_videoram[offs + 1];
	int code = psychic5_bg_videoram[offs] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) + ((attr & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int offs = tile_index * 2;
	int attr = psychic5_fg_videoram[offs + 1];
	int code = psychic5_fg_videoram[offs] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) + ((attr & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO(2, code, color, flags);
}

VIDEO_START( psychic5 )
{
	psychic5_bg_videoram = auto_malloc(0x1000);
	psychic5_fg_videoram = auto_malloc(0x1000);
	ps5_dummy_bg_ram = auto_malloc(0x1000);
	ps5_io_ram = auto_malloc(0x400);
	ps5_palette_ram = auto_malloc(0xc00);
	jal_blend_table = auto_malloc(0xc00);

	memset(psychic5_bg_videoram, 0,0x1000);
	memset(psychic5_fg_videoram,0,0x1000);
	memset(ps5_dummy_bg_ram,0,0x1000);
	memset(ps5_io_ram,0,0x400);
	memset(ps5_palette_ram,0,0xc00);
	memset(jal_blend_table,0,0xc00) ;

	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_cols,
		 16, 16, 64, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_cols,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 15);
}

#define DRAW_SPRITE(code, sx, sy) jal_blend_drawgfx(bitmap, machine->gfx[0], code, color, flipx, flipy, sx, sy, cliprect, TRANSPARENCY_PEN, 15);
/* #define DRAW_SPRITE(code, sx, sy) drawgfx(bitmap, machine->gfx[0], code, color, flipx, flipy, sx, sy, cliprect, TRANSPARENCY_PEN, 15); */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs;

	if (title_screen)
		return;
	else
		bg_clip_mode = -10;

	for (offs = 11; offs < spriteram_size; offs += 16)
	{
		int tileofs0, tileofs1, tileofs2, tileofs3, temp1, temp2;
		int attr = spriteram[offs + 2];
		int code = spriteram[offs + 3] + ((attr & 0xc0) << 2);
		int color = spriteram[offs + 4] & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[offs + 1];
		int sy = spriteram[offs];
		int size32 = attr & 0x08;

		if (attr & 0x01) sx -= 256;
		if (attr & 0x04) sy -= 256;

		if (flip_screen_get())
		{
			sx = 224 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipy)
		{
			tileofs0 = 1; tileofs1 = 0;	tileofs2 = 3; tileofs3 = 2;
		}
		else
		{
			tileofs0 = 0; tileofs1 = 1;	tileofs2 = 2; tileofs3 = 3;
		}

		if (flipx)
		{
			temp1 = tileofs0;
			temp2 = tileofs1;
			tileofs0 = tileofs2;
			tileofs1 = tileofs3;
			tileofs2 = temp1;
			tileofs3 = temp2;
		}

		if (size32)
		{
			DRAW_SPRITE(code + tileofs0, sx, sy)
			DRAW_SPRITE(code + tileofs1, sx, sy + 16)
			DRAW_SPRITE(code + tileofs2, sx + 16, sy)
			DRAW_SPRITE(code + tileofs3, sx + 16, sy + 16)
		}
		else
		{
			if (flip_screen_get())
				DRAW_SPRITE(code, sx + 16, sy + 16)
			else
				DRAW_SPRITE(code, sx, sy)
		}
	}
}

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int bg_scrollx = (ps5_io_ram[BG_SCROLLX_LSB] + ((ps5_io_ram[BG_SCROLLX_MSB] & 0x03) << 8)) & 0x3ff;
	int bg_scrolly = (ps5_io_ram[BG_SCROLLY_LSB] + ((ps5_io_ram[BG_SCROLLY_MSB] & 0x01) << 8)) & 0x1ff;

	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	set_background_palette_intensity(machine);

	if (ps5_io_ram[BG_SCREEN_MODE] & 0x01)  /* background enable */
	{
		if (title_screen)
		{
			rectangle clip = *cliprect;

			int sx1 = spriteram[12];		/* sprite 0 */
			int sy1 = spriteram[11];
			int tile = spriteram[14];
			int sy2 = spriteram[11+128];	/* sprite 8 */

			if (bg_clip_mode >=0 && bg_clip_mode < 3 && sy1==240) bg_clip_mode = 0;
			if (bg_clip_mode > 2 && bg_clip_mode < 5 && sy2==240) bg_clip_mode = -10;
			if (bg_clip_mode > 4 && bg_clip_mode < 7 && sx1==240) bg_clip_mode = 0;
			if (bg_clip_mode > 6 && bg_clip_mode < 9 && (sx1==240 || sx1==0)) bg_clip_mode = -10;

			if (sy1!=240 && sy1!=0 && bg_clip_mode<=0)
			{
				if (sy1 > 128)
					bg_clip_mode = 1;
				else
					bg_clip_mode = 2;
			}

			if (sy2!=240 && sy2!=0 && bg_clip_mode<=0)
			{
				if (sy2 > 128)
					bg_clip_mode = 3;
				else
					bg_clip_mode = 4;
			}

			if (sx1!=240 && sx1!=0 && bg_clip_mode<=0 && tile==0x3c)
			{
				if (sx1 > 128)
					bg_clip_mode = 5;
				else
					bg_clip_mode = 6;
			}

			if (sx1!=240 && sx1!=0 && bg_clip_mode<=0 && tile==0x1c)
			{
				if (sx1 > 128)
					bg_clip_mode = 7;
				else
					bg_clip_mode = 8;
			}

			if (bg_clip_mode)
			{
				if (bg_clip_mode == 1)
					clip.min_y = sy1;
				else if (bg_clip_mode == 2)
 					clip.max_y = sy1;
				else if (bg_clip_mode == 3)
					clip.max_y = sy2;
				else if (bg_clip_mode == 4)
					clip.min_y = sy2;
				else if (bg_clip_mode == 5)
					clip.min_x = sx1;
				else if (bg_clip_mode == 6)
 					clip.max_x = sx1;
				else if (bg_clip_mode == 7)
					clip.max_x = sx1;
				else if (bg_clip_mode == 8)
					clip.min_x = sx1;
				else if (bg_clip_mode == -10)
				{
					clip.min_x = 0;
					clip.min_y = 0;
					clip.max_x = 0;
					clip.max_y = 0;
				}

				fillbitmap(bitmap, get_black_pen(machine), cliprect);
				tilemap_draw(bitmap, &clip, bg_tilemap, 0, 0);
			}
			else
				tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
		}
		else
			tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	}
	else
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
}

VIDEO_UPDATE( psychic5 )
{
	draw_background(machine, bitmap, cliprect);
	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
