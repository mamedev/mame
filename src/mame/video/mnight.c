#include "driver.h"

UINT8   *mnight_scrolly_ram;
UINT8   *mnight_scrollx_ram;
UINT8   *mnight_bgenable_ram;
UINT8   *mnight_spoverdraw_ram;
UINT8   *mnight_background_videoram;
size_t mnight_backgroundram_size;
UINT8   *mnight_foreground_videoram;
size_t mnight_foregroundram_size;

static mame_bitmap *bitmap_bg;
static mame_bitmap *bitmap_sp;

static UINT8     *bg_dirtybuffer;
static int       bg_enable = 1;
static int       sp_overdraw = 0;

static void mnight_mark_background_dirty(void)
{
	/* set 1024 bytes allocated in VIDEO_START( mnight ) to 'true' */
	memset(bg_dirtybuffer,1,1024);
}

VIDEO_START( mnight )
{
	bg_dirtybuffer = auto_malloc(1024);

	bitmap_bg = auto_bitmap_alloc (machine->screen[0].width*2,machine->screen[0].height*2,machine->screen[0].format);
	bitmap_sp = auto_bitmap_alloc (machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	mnight_mark_background_dirty() ;

	state_save_register_func_postload(mnight_mark_background_dirty) ;
}


WRITE8_HANDLER( mnight_bgvideoram_w )
{
	if (mnight_background_videoram[offset] != data)
	{
		bg_dirtybuffer[offset >> 1] = 1;
		mnight_background_videoram[offset] = data;
	}
}

WRITE8_HANDLER( mnight_fgvideoram_w )
{
	if (mnight_foreground_videoram[offset] != data)
		mnight_foreground_videoram[offset] = data;
}

WRITE8_HANDLER( mnight_background_enable_w )
{
	if (bg_enable!=data)
	{
		mnight_bgenable_ram[offset] = data;
		bg_enable = data;
		if (bg_enable)
			memset(bg_dirtybuffer, 1, mnight_backgroundram_size / 2);
		else
			fillbitmap(bitmap_bg, Machine->pens[0],0);
	}
}

WRITE8_HANDLER( mnight_sprite_overdraw_w )
{
	if (sp_overdraw != (data&1))
	{
		mnight_spoverdraw_ram[offset] = data;
		fillbitmap(bitmap_sp,15,&Machine->screen[0].visarea);
		sp_overdraw = data & 1;
	}
}

static void draw_foreground(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the foreground text */

	for (offs = 0 ;offs < mnight_foregroundram_size / 2; offs++)
	{
		int sx,sy,tile,palette,flipx,flipy,lo,hi;

		if (mnight_foreground_videoram[offs*2] | mnight_foreground_videoram[offs*2+1])
		{
			sx = (offs % 32) << 3;
			sy = (offs >> 5) << 3;

			lo = mnight_foreground_videoram[offs*2];
			hi = mnight_foreground_videoram[offs*2+1];
			tile = ((hi & 0xc0) << 2) | lo;
			flipx = hi & 0x10;
			flipy = hi & 0x20;
			palette = hi & 0x0f;

			drawgfx(bitmap,machine->gfx[3],
					tile,
					palette,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN, 15);
		}

	}
}


static void draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */

	for (offs = 0 ;offs < mnight_backgroundram_size / 2; offs++)
	{
		int sx,sy,tile,palette,flipy,lo,hi;

		if (bg_dirtybuffer[offs])
		{
			sx = (offs % 32) << 4;
			sy = (offs >> 5) << 4;

			bg_dirtybuffer[offs] = 0;

			lo = mnight_background_videoram[offs*2];
			hi = mnight_background_videoram[offs*2+1];
			tile = ((hi & 0x10) << 6) | ((hi & 0xc0) << 2) | lo;
			flipy = hi & 0x20;
			palette = hi & 0x0f;
			drawgfx(bitmap,machine->gfx[0],
					tile,
					palette,
					0,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_NONE,0);
		}

	}
}


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 11 ;offs < spriteram_size; offs+=16)
	{
		int sx,sy,tile,palette,flipx,flipy,big;

		if (spriteram[offs+2] & 2)
		{
			sx = spriteram[offs+1];
			sy = spriteram[offs];
			if (spriteram[offs+2] & 1) sx-=256;
			tile = spriteram[offs+3]+((spriteram[offs+2] & 0xc0)<<2) + ((spriteram[offs+2] & 0x08)<<7);
			big  = spriteram[offs+2] & 4;
			if (big) tile /= 4;
			flipx = spriteram[offs+2] & 0x10;
			flipy = spriteram[offs+2] & 0x20;
			palette = spriteram[offs+4] & 0x0f;
			drawgfx(bitmap,machine->gfx[(big)?2:1],
					tile,
					palette,
					flipx,flipy,
					sx,sy,
					cliprect,
					TRANSPARENCY_PEN, 15);

			/* kludge to clear shots */
			if (((spriteram[offs+2]==2) || (spriteram[offs+2]==0x12)) && (((tile>=0xd0) && (tile<=0xd5)) || ((tile>=0x20) && (tile<=0x25))))
				spriteram[offs+2]=0;
		}
	}
}


VIDEO_UPDATE( mnight )
{
	int scrollx,scrolly;


	if (bg_enable)
		draw_background(machine, bitmap_bg, 0);

	scrollx = -((mnight_scrollx_ram[0]+mnight_scrollx_ram[1]*256) & 0x1FF);
	scrolly = -((mnight_scrolly_ram[0]+mnight_scrolly_ram[1]*256) & 0x1FF);

	if (sp_overdraw)	/* overdraw sprite mode */
	{
		copyscrollbitmap(bitmap,bitmap_bg,1,&scrollx,1,&scrolly,cliprect,TRANSPARENCY_NONE,0);
		draw_sprites(machine, bitmap_sp, 0);
		draw_foreground(machine, bitmap_sp, 0);
		copybitmap(bitmap,bitmap_sp,0,0,0,0,cliprect,TRANSPARENCY_PEN, 15);
	}
	else			/* normal sprite mode */
	{
		copyscrollbitmap(bitmap,bitmap_bg,1,&scrollx,1,&scrolly,cliprect,TRANSPARENCY_NONE,0);
		draw_sprites(machine, bitmap, cliprect);
		draw_foreground(machine, bitmap, cliprect);
	}

	return 0;
}
