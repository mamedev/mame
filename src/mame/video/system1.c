/*************************************************************************

  System1 / System 2.   By Jarek Parchanski & Mirko Buffoni.

  Many thanks to Roberto Ventura, for precious information about
  System 1 hardware.

*************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "system1.h"

UINT8 *system1_scroll_y;
UINT8 *system1_scroll_x;
UINT8 *system1_videoram;
UINT8 *system1_backgroundram;
UINT8 *system1_sprites_collisionram;
UINT8 *system1_background_collisionram;
UINT8 *system1_scrollx_ram;
size_t system1_videoram_size;
size_t system1_backgroundram_size;

static UINT8 *sprite_onscreen_map;
static int background_scrollx=0,background_scrolly=0;
static UINT8 *bg_dirtybuffer;

static int scrollx_row[32];
static bitmap_t *tmp_bitmap;

//static int system1_pixel_mode = 0
static UINT8 system1_background_memory,system1_video_mode=0;

static const UINT8 *system1_color_prom;

static UINT8 *wbml_paged_videoram;
static UINT8 wbml_videoram_bank=0,wbml_videoram_bank_latch=0;

static int blockgal_kludgeoffset;
static int system1_sprite_xoffset = 0;

/***************************************************************************

  There are two kind of color handling: in the System 1 games, values in the
  palette RAM are directly mapped to colors with the usual BBGGGRRR format;
  in the System 2 ones (Choplifter, WBML, etc.), the value in the palette RAM
  is a lookup offset for three palette PROMs in RRRRGGGGBBBB format.

  It's hard to tell for sure because they use resistor packs, but here's
  what I think the values are from measurment with a volt meter:

  Blue: .250K ohms
  Blue: .495K ohms
  Green:.250K ohms
  Green:.495K ohms
  Green:.995K ohms
  Red:  .495K ohms
  Red:  .250K ohms
  Red:  .995K ohms

  accurate to +/- .003K ohms.

***************************************************************************/
PALETTE_INIT( system1 )
{
	system1_color_prom = color_prom;
}

WRITE8_HANDLER( system1_paletteram_w )
{
	int val,r,g,b;


	paletteram[offset] = data;

	if (system1_color_prom)
	{
		int bit0,bit1,bit2,bit3;

		val = system1_color_prom[data+0*256];
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		bit3 = (val >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		val = system1_color_prom[data+1*256];
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		bit3 = (val >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		val = system1_color_prom[data+2*256];
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		bit3 = (val >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
	}
	else
	{
		r = pal3bit(data >> 0);
		g = pal3bit(data >> 3);
		b = pal2bit(data >> 6);
	}

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}

static void system1_postload(void)
{
	memset(sprite_onscreen_map, 255, 256*256);
	memset(bg_dirtybuffer, 1, 1024);
}

VIDEO_START( system1 )
{
	sprite_onscreen_map = auto_malloc(256*256);
	memset(sprite_onscreen_map,255,256*256);

	bg_dirtybuffer = auto_malloc(1024);
	memset(bg_dirtybuffer,1,1024);

	tmp_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	state_save_register_func_postload(system1_postload);
	state_save_register_global(system1_background_memory);
	state_save_register_global(system1_video_mode);

	system1_sprite_xoffset = 1;
}

VIDEO_START( wbml )
{
	sprite_onscreen_map = auto_malloc(256*256);
	memset(sprite_onscreen_map,255,256*256);

	bg_dirtybuffer = auto_malloc(1024);
	memset(bg_dirtybuffer,1,1024);
	wbml_paged_videoram = auto_malloc(0x4000);	/* Allocate 16k for background banked ram */
	memset(wbml_paged_videoram,0,0x4000);

	tmp_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	system1_sprite_xoffset = 1+7*2;

	state_save_register_func_postload(system1_postload);
	state_save_register_global(system1_background_memory);
	state_save_register_global(system1_video_mode);

	state_save_register_global_pointer(wbml_paged_videoram, 0x4000);
	state_save_register_global(wbml_videoram_bank);
	state_save_register_global(wbml_videoram_bank_latch);
}

WRITE8_HANDLER( system1_videomode_w )
{
if (data & 0x6e) logerror("videomode = %02x\n",data);

	/* bit 0 is coin counter */
	coin_counter_w(0, data & 1);

	/* bit 4 is screen blank */
	system1_video_mode = data;

	/* bit 7 is flip screen */
	flip_screen_set(data & 0x80);
}

READ8_HANDLER( system1_videomode_r )
{
	return system1_video_mode;
}

void system1_define_background_memory(int mode)
{
	system1_background_memory = mode;
}


INLINE int get_sprite_bottom_y(int spr_number)
{
	return  spriteram[0x10 * spr_number + SPR_Y_BOTTOM];
}

INLINE void draw_pixel(bitmap_t *bitmap,
				  	   int x,int y,int x_flipped,int y_flipped,
				  	   int spr_number,int color)
{
	int xr,yr;
	int sprite_onscreen;


	if (x < 0 || x >= Machine->screen[0].width ||
		y < 0 || y >= Machine->screen[0].height)
		return;

	if (sprite_onscreen_map[256*y+x] != 255)
	{
		sprite_onscreen = sprite_onscreen_map[256*y+x];
		system1_sprites_collisionram[sprite_onscreen + 32 * spr_number] = 0xff;
	}

	sprite_onscreen_map[256*y+x] = spr_number;

	if (x_flipped >= Machine->screen[0].visarea.min_x ||
		x_flipped <= Machine->screen[0].visarea.max_x ||
		y_flipped >= Machine->screen[0].visarea.min_y ||
		y_flipped <= Machine->screen[0].visarea.max_y)
	{
		*BITMAP_ADDR16(bitmap, y_flipped, x_flipped) = color;
	}

	xr = ((x - background_scrollx) & 0xff) / 8;
	yr = ((y - background_scrolly) & 0xff) / 8;

	/* TODO: bits 5 and 6 of backgroundram are also used (e.g. Pitfall2, Mr. Viking) */
	/* what's the difference? Bit 7 is used in Choplifter/WBML for extra char bank */
	/* selection, but it is also set in Pitfall2 */

	if (system1_background_memory == system1_BACKGROUND_MEMORY_SINGLE)
	{
		if (system1_backgroundram[2 * (32 * yr + xr) + 1] & 0x10)
			system1_background_collisionram[0x20 + spr_number] = 0xff;
	}
	else
	{
		/* TODO: I should handle the paged background memory here. */
		/* maybe collision detection is not used by the paged games */
		/* (wbml and tokisens), though tokisens doesn't play very well */
		/* (you can't seem to fit in gaps where you should fit) */
	}

	/* TODO: collision should probably be checked with the foreground as well */
	/* (TeddyBoy Blues, head of the tiger in girl bonus round) */
}

WRITE8_HANDLER( system1_background_collisionram_w )
{
	/* to do the RAM check, Mister Viking writes 0xff and immediately */
	/* reads it back, expecting bit 0 to be NOT set. */
	system1_background_collisionram[offset] = 0x7e;
}

WRITE8_HANDLER( system1_sprites_collisionram_w )
{
	/* to do the RAM check, Mister Viking write 0xff and immediately */
	/* reads it back, expecting bit 0 to be NOT set. */
	/* Up'n Down expects to find 0x7e at f800 before doing the whole */
	/* collision test */
	system1_sprites_collisionram[offset] = 0x7e;
}

static void draw_sprite(bitmap_t *bitmap,int spr_number)
{
	int sy,row,height,src,bank;
	UINT8 *sprite_base;
	int sprite_palette_base;
	INT16 skip;	/* bytes to skip before drawing each row (can be negative) */
	UINT8 *gfx;


	sprite_base	= spriteram + 0x10 * spr_number;

	src = sprite_base[SPR_GFXOFS_LO] + (sprite_base[SPR_GFXOFS_HI] << 8);
	bank = 0x8000 * (((sprite_base[SPR_X_HI] & 0x80) >> 7) + ((sprite_base[SPR_X_HI] & 0x40) >> 5));
	bank &= (memory_region_length(REGION_GFX2)-1);	/* limit to the range of available ROMs */
	skip = sprite_base[SPR_SKIP_LO] + (sprite_base[SPR_SKIP_HI] << 8);

	height = sprite_base[SPR_Y_BOTTOM] - sprite_base[SPR_Y_TOP];
	sprite_palette_base = 0x10 * spr_number;

	sy = sprite_base[SPR_Y_TOP] + 1;

	/* graphics region #2 contains the packed sprite data */
	gfx = &memory_region(REGION_GFX2)[bank];

	for (row = 0;row < height;row++)
	{
		int x,x_flipped;
		int y,y_flipped;
		int src2;

		src = src2 = src + skip;

		x = sprite_base[SPR_X_LO] + ((sprite_base[SPR_X_HI] & 0x01) << 8) + system1_sprite_xoffset;
		x_flipped = x;
		y = y_flipped = sy+row;

		if (flip_screen_get())
		{
			y_flipped = 258 - sy - height + row;
			x_flipped = (252*2) - x;
		}

		x /= 2;	/* the hardware has sub-pixel placement, it seems */
		x_flipped /= 2;

		while (1)
		{
			int color1,color2,data;

			data = gfx[src2 & 0x7fff];

			if (src & 0x8000)
			{
				src2--;

				color1 = data & 0x0f;
				color2 = data >> 4;
			}
			else
			{
				src2++;

				color1 = data >> 4;
				color2 = data & 0x0f;
			}

			if (color1 == 15) break;
			if (color1)
				draw_pixel(bitmap,x,y,x_flipped,y_flipped,spr_number,sprite_palette_base+color1);
			x++;
			x_flipped += flip_screen_get() ? -1 : 1;

			if (color2 == 15) break;
			if (color2)
				draw_pixel(bitmap,x,y,x_flipped,y_flipped,spr_number,sprite_palette_base+color2);
			x++;
			x_flipped += flip_screen_get() ? -1 : 1;
		}
	}
}


static void draw_sprites(bitmap_t *bitmap)
{
	int spr_number,sprite_bottom_y,sprite_top_y;
	UINT8 *sprite_base;


	memset(sprite_onscreen_map,255,256*256);

	for (spr_number = 0;spr_number < 32;spr_number++)
	{
		sprite_base = spriteram + 0x10 * spr_number;
		sprite_top_y = sprite_base[SPR_Y_TOP];
		sprite_bottom_y = sprite_base[SPR_Y_BOTTOM];
		if (sprite_bottom_y && (sprite_bottom_y-sprite_top_y > 0))
			draw_sprite(bitmap,spr_number);
	}
}



WRITE8_HANDLER( system1_backgroundram_w )
{
	system1_backgroundram[offset] = data;
	bg_dirtybuffer[offset>>1] = 1;
}


static int system1_draw_fg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int sx,sy,offs;
	int drawn = 0;


	priority <<= 3;

	for (offs = 0;offs < system1_videoram_size;offs += 2)
	{
		if ((system1_videoram[offs+1] & 0x08) == priority)
		{
			int code,color;


			code = (system1_videoram[offs] | (system1_videoram[offs+1] << 8));
			code = ((code >> 4) & 0x800) | (code & 0x7ff);	/* Heavy Metal only */
			color = ((code >> 5) & 0x3f);
			sx = (offs/2) % 32;
			sy = (offs/2) / 32;

			if (flip_screen_get())
			{
				sx = 31 - sx;
				sy = 31 - sy;
			}

			code %= machine->gfx[0]->total_elements;
			if (machine->gfx[0]->pen_usage[code] & ~1)
			{
				drawn = 1;

				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						8*sx + blockgal_kludgeoffset,8*sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}

	return drawn;
}

static void system1_draw_bg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int sx,sy,offs;
	int background_scrollx_flip, background_scrolly_flip;


	background_scrollx = ((system1_scroll_x[0] >> 1) + ((system1_scroll_x[1] & 1) << 7) + 14 + 2*blockgal_kludgeoffset) & 0xff;
	background_scrolly = (-*system1_scroll_y) & 0xff;

	background_scrollx_flip = (275 - background_scrollx) & 0xff;
	background_scrolly_flip = (256 - background_scrolly) & 0xff;

	if (priority == -1)
	{
		/* optimized far background */

		/* for every character in the background video RAM, check if it has
         * been modified since last time and update it accordingly.
         */

		for (offs = 0;offs < system1_backgroundram_size;offs += 2)
		{
			if (bg_dirtybuffer[offs / 2])
			{
				int code,color;


				bg_dirtybuffer[offs / 2] = 0;

				code = (system1_backgroundram[offs] | (system1_backgroundram[offs+1] << 8));
				code = ((code >> 4) & 0x800) | (code & 0x7ff);	/* Heavy Metal only */
				color = ((code >> 5) & 0x3f) + 0x40;
				sx = (offs/2) % 32;
				sy = (offs/2) / 32;

				if (flip_screen_get())
				{
					sx = 31 - sx;
					sy = 31 - sy;
				}

				drawgfx(tmp_bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						8*sx,8*sy,
						0,TRANSPARENCY_NONE,0);
			}
		}

		/* copy the temporary bitmap to the screen */
		if (flip_screen_get())
			copyscrollbitmap(bitmap,tmp_bitmap,1,&background_scrollx_flip,1,&background_scrolly_flip,cliprect);
		else
			copyscrollbitmap(bitmap,tmp_bitmap,1,&background_scrollx,1,&background_scrolly,cliprect);
	}
	else
	{
		priority <<= 3;

		for (offs = 0;offs < system1_backgroundram_size;offs += 2)
		{
			if ((system1_backgroundram[offs+1] & 0x08) == priority)
			{
				int code,color;


				code = (system1_backgroundram[offs] | (system1_backgroundram[offs+1] << 8));
				code = ((code >> 4) & 0x800) | (code & 0x7ff);	/* Heavy Metal only */
				color = ((code >> 5) & 0x3f) + 0x40;
				sx = (offs/2) % 32;
				sy = (offs/2) / 32;

				if (flip_screen_get())
				{
					sx = 8*(31-sx) + background_scrollx_flip;
					sy = 8*(31-sy) + background_scrolly_flip;
				}
				else
				{
					sx = 8*sx + background_scrollx;
					sy = 8*sy + background_scrolly;
				}

				/* draw it 4 times because of possible wrap around */
				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						sx,sy,
						cliprect,TRANSPARENCY_PEN,0);
				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						sx-256,sy,
						cliprect,TRANSPARENCY_PEN,0);
				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						sx,sy-256,
						cliprect,TRANSPARENCY_PEN,0);
				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						sx-256,sy-256,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}

VIDEO_UPDATE( system1 )
{
	int drawn;


	system1_draw_bg(machine, bitmap, cliprect, -1);
	drawn = system1_draw_fg(machine, bitmap, cliprect, 0);
	/* redraw low priority bg tiles if necessary */
	if (drawn) system1_draw_bg(machine, bitmap, cliprect, 0);
	draw_sprites(bitmap);
	system1_draw_bg(machine, bitmap, cliprect, 1);
	system1_draw_fg(machine, bitmap, cliprect, 1);

	/* even if screen is off, sprites must still be drawn to update the collision table */
	if (system1_video_mode & 0x10)  /* screen off */
		fillbitmap(bitmap,0,cliprect);
	return 0;
}









WRITE8_HANDLER( choplifter_scroll_x_w )
{
	system1_scrollx_ram[offset] = data;

	scrollx_row[offset/2] = (system1_scrollx_ram[offset & ~1] >> 1) + ((system1_scrollx_ram[offset | 1] & 1) << 7);
}

static void chplft_draw_bg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int sx,sy,offs;
	int choplifter_scroll_x_on = (system1_scrollx_ram[0] == 0xe5 && system1_scrollx_ram[1] == 0xff) ? 0 : 1;


	if (priority == -1)
	{
		/* optimized far background */

		/* for every character in the background video RAM, check if it has
         * been modified since last time and update it accordingly.
         */

		for (offs = 0;offs < system1_backgroundram_size;offs += 2)
		{
			if (bg_dirtybuffer[offs / 2])
			{
				int code,color;


				bg_dirtybuffer[offs / 2] = 0;

				code = (system1_backgroundram[offs] | (system1_backgroundram[offs+1] << 8));
				code = ((code >> 4) & 0x800) | (code & 0x7ff);	/* Heavy Metal only */
				color = ((code >> 5) & 0x3f) + 0x40;
				sx = (offs/2) % 32;
				sy = (offs/2) / 32;

				if (flip_screen_get())
				{
					sx = 31 - sx;
					sy = 31 - sy;
				}

				drawgfx(tmp_bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						8*sx,8*sy,
						0,TRANSPARENCY_NONE,0);
			}
		}

		/* copy the temporary bitmap to the screen */
		if (choplifter_scroll_x_on)
		{
			if (flip_screen_get())
			{
				int scrollx_row_flip[32],i;

				for (i = 0; i < 32; i++)
					scrollx_row_flip[31-i] = (256-scrollx_row[i]) & 0xff;

				copyscrollbitmap(bitmap,tmp_bitmap,32,scrollx_row_flip,0,0,cliprect);
			}
			else
				copyscrollbitmap(bitmap,tmp_bitmap,32,scrollx_row,0,0,cliprect);
		}
		else
			copybitmap(bitmap,tmp_bitmap,0,0,0,0,cliprect);
	}
	else
	{
		priority <<= 3;

		for (offs = 0;offs < system1_backgroundram_size;offs += 2)
		{
			if ((system1_backgroundram[offs+1] & 0x08) == priority)
			{
				int code,color;


				code = (system1_backgroundram[offs] | (system1_backgroundram[offs+1] << 8));
				code = ((code >> 4) & 0x800) | (code & 0x7ff);	/* Heavy Metal only */
				color = ((code >> 5) & 0x3f) + 0x40;
				sx = (offs/2) % 32;
				sy = (offs/2) / 32;

				if (flip_screen_get())
				{
					sx = 8*(31-sx);

					if (choplifter_scroll_x_on)
						sx = (sx - scrollx_row[sy]) & 0xff;

					sy = 31 - sy;
				}
				else
				{
					sx = 8*sx;

					if (choplifter_scroll_x_on)
						sx = (sx + scrollx_row[sy]) & 0xff;
				}

				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen_get(),flip_screen_get(),
						sx,8*sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}

VIDEO_UPDATE( choplifter )
{
	int drawn;


	chplft_draw_bg(machine, bitmap, cliprect, -1);
	drawn = system1_draw_fg(machine, bitmap, cliprect, 0);
	/* redraw low priority bg tiles if necessary */
	if (drawn) chplft_draw_bg(machine, bitmap, cliprect, 0);
	draw_sprites(bitmap);
	chplft_draw_bg(machine, bitmap, cliprect, 1);
	system1_draw_fg(machine, bitmap, cliprect, 1);

	/* even if screen is off, sprites must still be drawn to update the collision table */
	if (system1_video_mode & 0x10)  /* screen off */
		fillbitmap(bitmap,0,cliprect);


#ifdef MAME_DEBUG
	if (input_code_pressed(KEYCODE_SPACE))		// goto next level
	{
		memory_region(REGION_CPU1)[0xC085]=33;
	}
#endif
	return 0;
}



READ8_HANDLER( wbml_videoram_bank_latch_r )
{
	return wbml_videoram_bank_latch;
}

WRITE8_HANDLER( wbml_videoram_bank_latch_w )
{
	wbml_videoram_bank_latch = data;
	wbml_videoram_bank = (data >> 1) & 0x03;	/* Select 4 banks of 4k, bit 2,1 */
}

READ8_HANDLER( wbml_paged_videoram_r )
{
	return wbml_paged_videoram[0x1000*wbml_videoram_bank + offset];
}

WRITE8_HANDLER( wbml_paged_videoram_w )
{
	wbml_paged_videoram[0x1000*wbml_videoram_bank + offset] = data;
}

static void wbml_draw_bg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int trasp)
{
	int page;

	int xscroll = (wbml_paged_videoram[0x7c0] >> 1) + ((wbml_paged_videoram[0x7c1] & 1) << 7) - 256 + 5;
	int yscroll = -wbml_paged_videoram[0x7ba];

	for (page=0; page < 4; page++)
	{
		const UINT8 *source = wbml_paged_videoram + (wbml_paged_videoram[0x0740 + page*2] & 0x07)*0x800;
		int startx = (page&1)*256+xscroll;
		int starty = (page>>1)*256+yscroll;
		int row,col;


		for( row=0; row<32*8; row+=8 )
		{
			for( col=0; col<32*8; col+=8 )
			{
				int code,priority;
				int x = (startx+col) & 0x1ff;
				int y = (starty+row) & 0x1ff;
				if (x > 256) x -= 512;
				if (y > 224) y -= 512;

				if (flip_screen_get())
				{
					x = 248 - x;
					y = 248 - y;
				}

				code = source[0] + (source[1] << 8);
				priority = code & 0x800;
				code = ((code >> 4) & 0x800) | (code & 0x7ff);

				if (!trasp)
					drawgfx(bitmap,machine->gfx[0],
							code,
							((code >> 5) & 0x3f) + 64,
							flip_screen_get(),flip_screen_get(),
							x,y,
							cliprect, TRANSPARENCY_NONE, 0);
				else if (priority)
					drawgfx(bitmap,machine->gfx[0],
							code,
							((code >> 5) & 0x3f) + 64,
							flip_screen_get(),flip_screen_get(),
							x,y,
							cliprect, TRANSPARENCY_PEN, 0);

				source+=2;
			}
		}
	} /* next page */
}

static void wbml_draw_fg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;


	for (offs = 0;offs < 0x700;offs += 2)
	{
		int sx,sy,code;


		sx = (offs/2) % 32;
		sy = (offs/2) / 32;
		code = wbml_paged_videoram[offs] | (wbml_paged_videoram[offs+1] << 8);
		code = ((code >> 4) & 0x800) | (code & 0x7ff);

		if (flip_screen_get())
		{
			sx = 31 - sx;
			sy = 31 - sy;
		}

		drawgfx(bitmap,machine->gfx[0],
				code,
				(code >> 5) & 0x3f,
				flip_screen_get(),flip_screen_get(),
				8*sx,8*sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( wbml )
{
	wbml_draw_bg(machine, bitmap, cliprect, 0);
	draw_sprites(bitmap);
	wbml_draw_bg(machine, bitmap, cliprect, 1);
	wbml_draw_fg(machine, bitmap, cliprect);

	/* even if screen is off, sprites must still be drawn to update the collision table */
	if (system1_video_mode & 0x10)  /* screen off */
		fillbitmap(bitmap,0,cliprect);
	return 0;
}

/* same as wbml but with rows scroll */
static void ufosensi_draw_bg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int trasp)
{
	int page;

	int yscroll = -wbml_paged_videoram[0x7ba];

	for (page=0; page < 4; page++)
	{
		const UINT8 *source = wbml_paged_videoram + (wbml_paged_videoram[0x0740 + page*2] & 0x07)*0x800;
		int starty = (page>>1)*256+yscroll;
		int row,col;


		for( row=0; row<32*8; row+=8 )
		{
			for( col=0; col<32*8; col+=8 )
			{
				int code,priority;
				int xscroll = (wbml_paged_videoram[0x7c0 + row/4] >> 1) + ((wbml_paged_videoram[0x7c1 + row/4] & 1) << 7) - 256 + 5;
				int startx = (page&1)*256+xscroll;
				int x = (startx+col) & 0x1ff;
				int y = (starty+row) & 0x1ff;
				if (x > 256) x -= 512;
				if (y > 224) y -= 512;

				if (flip_screen_get())
				{
					x = 248 - x;
					y = 248 - y;
				}

				code = source[0] + (source[1] << 8);
				priority = code & 0x800;
				code = ((code >> 4) & 0x800) | (code & 0x7ff);

				if (!trasp)
					drawgfx(bitmap,machine->gfx[0],
							code,
							((code >> 5) & 0x3f) + 64,
							flip_screen_get(),flip_screen_get(),
							x,y,
							cliprect, TRANSPARENCY_NONE, 0);
				else if (priority)
					drawgfx(bitmap,machine->gfx[0],
							code,
							((code >> 5) & 0x3f) + 64,
							flip_screen_get(),flip_screen_get(),
							x,y,
							cliprect, TRANSPARENCY_PEN, 0);

				source+=2;
			}
		}
	} /* next page */
}

VIDEO_UPDATE( ufosensi )
{
	ufosensi_draw_bg(machine, bitmap, cliprect, 0);
	draw_sprites(bitmap);
	ufosensi_draw_bg(machine, bitmap, cliprect, 1);
	wbml_draw_fg(machine, bitmap, cliprect);

	/* even if screen is off, sprites must still be drawn to update the collision table */
	if (system1_video_mode & 0x10)  /* screen off */
		fillbitmap(bitmap,0,cliprect);
	return 0;
}

VIDEO_UPDATE( blockgal )
{
	int drawn;


	blockgal_kludgeoffset = -8;

	system1_draw_bg(machine, bitmap, cliprect, -1);
	drawn = system1_draw_fg(machine, bitmap, cliprect, 0);
	/* redraw low priority bg tiles if necessary */
	if (drawn) system1_draw_bg(machine, bitmap, cliprect, 0);
	draw_sprites(bitmap);
	system1_draw_bg(machine, bitmap, cliprect, 1);
	system1_draw_fg(machine, bitmap, cliprect, 1);

	/* even if screen is off, sprites must still be drawn to update the collision table */
	if (system1_video_mode & 0x10)  /* screen off */
		fillbitmap(bitmap,0,cliprect);

	blockgal_kludgeoffset = 0;
	return 0;
}
