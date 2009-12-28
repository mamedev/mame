/* notes...

 drawing sprites in a single pass with pdrawgfx breaks Thunder Dragon 2,
  which seems to expect the sprite priority values to affect sprite-sprite
  priority.  Thunder Dragon 2 also breaks if you support sprite flipping,
  the collectable point score / power up names appear flipped..

*/

#include "driver.h"
#include "includes/nmk16.h"

// the larger tilemaps on macross2, rapid hero and thunder dragon 2 appear to act like 4 'banks'
// of the smaller tilemaps, rather than being able to scroll into each other (not verified on real hw,
// but see raphero intro / 1st level cases)
UINT16 *nmk_bgvideoram0;
UINT16 *nmk_bgvideoram1;
UINT16 *nmk_bgvideoram2;
UINT16 *nmk_bgvideoram3;

static int nmk16_simple_scroll;

UINT16 *nmk_fgvideoram,*nmk_txvideoram;
UINT16 *gunnail_scrollram, *gunnail_scrollramy;
UINT16 *afega_scroll_0;
UINT16 *afega_scroll_1;


static int redraw_bitmap;

static UINT16 *spriteram_old,*spriteram_old2;
static int bgbank;
static int videoshift;
static int bioship_background_bank;
static UINT8 bioship_scroll[4];

static tilemap_t *bg_tilemap0;
static tilemap_t *bg_tilemap1;
static tilemap_t *bg_tilemap2;
static tilemap_t *bg_tilemap3;

static tilemap_t *tx_tilemap;

static tilemap_t *fg_tilemap; // strahl
static bitmap_t *background_bitmap; // bioship

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

#define TILES_PER_PAGE_X	(0x10)
#define TILES_PER_PAGE_Y	(0x10)
#define PAGES_PER_TMAP_X	(0x10)
#define PAGES_PER_TMAP_Y	(0x02)

static TILEMAP_MAPPER( afega_tilemap_scan_pages )
{
	return	(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

static TILE_GET_INFO( macross_get_bg0_tile_info ) { int code = nmk_bgvideoram0[tile_index]; SET_TILE_INFO(1,(code & 0xfff) + (bgbank << 12),code >> 12,0);}
static TILE_GET_INFO( macross_get_bg1_tile_info ) { int code = nmk_bgvideoram1[tile_index]; SET_TILE_INFO(1,(code & 0xfff) + (bgbank << 12),code >> 12,0);}
static TILE_GET_INFO( macross_get_bg2_tile_info ) { int code = nmk_bgvideoram2[tile_index]; SET_TILE_INFO(1,(code & 0xfff) + (bgbank << 12),code >> 12,0);}
static TILE_GET_INFO( macross_get_bg3_tile_info ) { int code = nmk_bgvideoram3[tile_index]; SET_TILE_INFO(1,(code & 0xfff) + (bgbank << 12),code >> 12,0);}


static TILE_GET_INFO( strahl_get_fg_tile_info )
{
	int code = nmk_fgvideoram[tile_index];
	SET_TILE_INFO(
			3,
			(code & 0xfff),
			code >> 12,
			0);
}

static TILE_GET_INFO( macross_get_tx_tile_info )
{
	int code = nmk_txvideoram[tile_index];
	SET_TILE_INFO(
			0,
			code & 0xfff,
			code >> 12,
			0);
}

static TILE_GET_INFO( bjtwin_get_bg_tile_info )
{
	int code = nmk_bgvideoram0[tile_index];
	int bank = (code & 0x800) ? 1 : 0;
	SET_TILE_INFO(
			bank,
			(code & 0x7ff) + ((bank) ? (bgbank << 11) : 0),
			code >> 12,
			0);
}

static TILE_GET_INFO( get_tile_info_0_8bit )
{
	UINT16 code = nmk_bgvideoram0[tile_index];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void nmk16_video_init(running_machine *machine)
{
	spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);

	videoshift = 0;		/* 256x224 screen, no shift */
	background_bitmap = NULL;
	nmk16_simple_scroll = 1;
}


VIDEO_START( bioship )
{
	bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap0,15);
	tilemap_set_transparent_pen(tx_tilemap,15);

	nmk16_video_init(machine);
	background_bitmap = auto_bitmap_alloc(machine,8192,512,video_screen_get_format(machine->primary_screen));
	bioship_background_bank=0;
	redraw_bitmap = 1;

}

VIDEO_START( strahl )
{
	bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	fg_tilemap = tilemap_create(machine, strahl_get_fg_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,15);
	tilemap_set_transparent_pen(tx_tilemap,15);

	nmk16_video_init(machine);
}

VIDEO_START( macross )
{
	bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,32,32);

	tilemap_set_transparent_pen(tx_tilemap,15);

	nmk16_video_init(machine);
}

VIDEO_START( gunnail )
{
	bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,64,32);

	tilemap_set_transparent_pen(tx_tilemap,15);
	tilemap_set_scroll_rows(bg_tilemap0,512);

	nmk16_video_init(machine);
	videoshift = 64;	/* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
	nmk16_simple_scroll = 0;
}

VIDEO_START( macross2 )
{
	bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	bg_tilemap1 = tilemap_create(machine, macross_get_bg1_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	bg_tilemap2 = tilemap_create(machine, macross_get_bg2_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	bg_tilemap3 = tilemap_create(machine, macross_get_bg3_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,64,32);

	tilemap_set_transparent_pen(tx_tilemap,15);

	nmk16_video_init(machine);
	videoshift = 64;	/* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}

VIDEO_START( raphero )
{
	VIDEO_START_CALL( macross2 );
	nmk16_simple_scroll = 0;
}

VIDEO_START( bjtwin )
{
	bg_tilemap0 = tilemap_create(machine, bjtwin_get_bg_tile_info,tilemap_scan_cols,8,8,64,32);

	nmk16_video_init(machine);
	videoshift = 64;	/* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( nmk_bgvideoram0_w ) { COMBINE_DATA(&nmk_bgvideoram0[offset]); tilemap_mark_tile_dirty(bg_tilemap0,offset); }
WRITE16_HANDLER( nmk_bgvideoram1_w ) { COMBINE_DATA(&nmk_bgvideoram1[offset]); tilemap_mark_tile_dirty(bg_tilemap1,offset); }
WRITE16_HANDLER( nmk_bgvideoram2_w ) { COMBINE_DATA(&nmk_bgvideoram2[offset]); tilemap_mark_tile_dirty(bg_tilemap2,offset); }
WRITE16_HANDLER( nmk_bgvideoram3_w ) { COMBINE_DATA(&nmk_bgvideoram3[offset]); tilemap_mark_tile_dirty(bg_tilemap3,offset); }


WRITE16_HANDLER( nmk_fgvideoram_w )
{
	COMBINE_DATA(&nmk_fgvideoram[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE16_HANDLER( nmk_txvideoram_w )
{
	COMBINE_DATA(&nmk_txvideoram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

static int mustang_bg_xscroll;

WRITE16_HANDLER( mustang_scroll_w )
{
//  mame_printf_debug("mustang %04x %04x %04x\n",offset,data,mem_mask);

	switch (data & 0xff00)
	{
		case 0x0000:
			mustang_bg_xscroll = (mustang_bg_xscroll & 0x00ff) | ((data & 0x00ff)<<8);
			break;

		case 0x0100:
			mustang_bg_xscroll = (mustang_bg_xscroll & 0xff00) | (data & 0x00ff);
			break;

		case 0x0200:
			break;

		case 0x0300:
			break;

		default:
			break;
	}

	tilemap_set_scrollx(bg_tilemap0,0,mustang_bg_xscroll - videoshift);
}

WRITE16_HANDLER( bioshipbg_scroll_w )
{
	static UINT8 scroll[4];

	if (ACCESSING_BITS_8_15)
	{
		scroll[offset] = (data >> 8) & 0xff;

		if (offset & 2)
			tilemap_set_scrolly(bg_tilemap0,0,scroll[2] * 256 + scroll[3]);
		else
			tilemap_set_scrollx(bg_tilemap0,0,scroll[0] * 256 + scroll[1] - videoshift);
	}
}

WRITE16_HANDLER( nmk_scroll_w )
{
	if (ACCESSING_BITS_0_7)
	{
		static UINT8 scroll[4];

		scroll[offset] = data & 0xff;

		if (offset & 2)
			tilemap_set_scrolly(bg_tilemap0,0,scroll[2] * 256 + scroll[3]);
		else
			tilemap_set_scrollx(bg_tilemap0,0,scroll[0] * 256 + scroll[1] - videoshift);
	}
}

WRITE16_HANDLER( nmk_scroll_2_w )
{
	if (ACCESSING_BITS_0_7)
	{
		static UINT8 scroll[4];

		scroll[offset] = data & 0xff;

		if (offset & 2)
			tilemap_set_scrolly(fg_tilemap,0,scroll[2] * 256 + scroll[3]);
		else
			tilemap_set_scrollx(fg_tilemap,0,scroll[0] * 256 + scroll[1] - videoshift);
	}
}

WRITE16_HANDLER( vandyke_scroll_w )
{
	static UINT16 scroll[4];

	scroll[offset] = data;

	tilemap_set_scrollx(bg_tilemap0,0,scroll[0] * 256 + (scroll[1] >> 8));
	tilemap_set_scrolly(bg_tilemap0,0,scroll[2] * 256 + (scroll[3] >> 8));
}

WRITE16_HANDLER( vandykeb_scroll_w )
{
	static UINT16 scroll[8];

	COMBINE_DATA(&scroll[offset]);

	tilemap_set_scrollx(bg_tilemap0,0,scroll[6] * 256 + (scroll[5] >> 8));
	tilemap_set_scrolly(bg_tilemap0,0,scroll[1] * 256 + (scroll[0] >> 8));
}

WRITE16_HANDLER( manybloc_scroll_w )
{
	COMBINE_DATA(&gunnail_scrollram[offset]);

	tilemap_set_scrollx(bg_tilemap0,0,gunnail_scrollram[0x82/2]-videoshift);
	tilemap_set_scrolly(bg_tilemap0,0,gunnail_scrollram[0xc2/2]);
}

WRITE16_HANDLER( nmk_flipscreen_w )
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(space->machine, data & 0x01);
}

WRITE16_HANDLER( nmk_tilebank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (bgbank != (data & 0xff))
		{
			bgbank = data & 0xff;
			tilemap_mark_all_tiles_dirty(bg_tilemap0);
		}
	}
}

WRITE16_HANDLER( bioship_scroll_w )
{
	if (ACCESSING_BITS_8_15)
		bioship_scroll[offset]=data>>8;
}

WRITE16_HANDLER( bioship_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (bioship_background_bank != data)
		{
			bioship_background_bank = data;
			redraw_bitmap=1;
		}
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/



// manybloc uses extra flip bits on the sprites, but these break other games

static void nmk16_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int offs;

	for (offs = 0;offs < 0x1000/2;offs += 8)
	{
		if ((spriteram_old2[offs] & 0x0001))
		{
			int sx = (spriteram_old2[offs+4] & 0x1ff) + videoshift;
			int sy = (spriteram_old2[offs+6] & 0x1ff);
			int code = spriteram_old2[offs+3];
			int color = spriteram_old2[offs+7];
			int w = (spriteram_old2[offs+1] & 0x0f);
			int h = ((spriteram_old2[offs+1] & 0xf0) >> 4);
			int pri = (spriteram_old2[offs] & 0xc0) >> 6;
			int xx,yy,x;
			int delta = 16;

			if(pri != priority)
				continue;

			if (flip_screen_get(machine))
			{
				sx = 368 - sx;
				sy = 240 - sy;
				delta = -16;
			}

			yy = h;
			do
			{
				x = sx;
				xx = w;
				do
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							flip_screen_get(machine), flip_screen_get(machine),
							((x + 16) & 0x1ff) - 16,sy & 0x1ff,15);
					code++;
					x += delta;
				} while (--xx >= 0);

				sy += delta;
			} while (--yy >= 0);
		}
	}
}

static void nmk16_draw_sprites_flipsupported(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int offs;

	for (offs = 0;offs < 0x1000/2;offs += 8)
	{
		if (spriteram_old2[offs] & 0x0001)
		{
			int sx = (spriteram_old2[offs+4] & 0x1ff) + videoshift;
			int sy = (spriteram_old2[offs+6] & 0x1ff);
			int code = spriteram_old2[offs+3];
			int color = spriteram_old2[offs+7];
			int w = (spriteram_old2[offs+1] & 0x0f);
			int h = ((spriteram_old2[offs+1] & 0xf0) >> 4);
			int pri = (spriteram_old2[offs] & 0xc0) >> 6;
			int flipy= ((spriteram_old2[offs+1] & 0x0200) >> 9);
			int flipx = ((spriteram_old2[offs+1] & 0x0100) >> 8);

			int xx,yy,x;
			int delta = 16;

			if(pri != priority)
				continue;

			flipx ^= flip_screen_get(machine);
			flipy ^= flip_screen_get(machine);

			if (flip_screen_get(machine))
			{
				sx = 368 - sx;
				sy = 240 - sy;
				delta = -16;
			}

			yy = h;
			sy+=flipy?(delta*h):0;
			do
			{
				x = sx+(flipx?(delta*w):0);


				xx = w;
				do
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							flipx, flipy,
							((x + 16) & 0x1ff) - 16,sy & 0x1ff,15);

					code++;
					x +=delta * ( flipx?-1:1 );


				} while (--xx >= 0);
				sy += delta * ( flipy?-1:1);

			} while (--yy >= 0);
		}
	}
}


VIDEO_UPDATE( macross )
{
	tilemap_set_scrollx(tx_tilemap,0,-videoshift);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( tdragon )
{
//  mcu_run(screen->machine, 1);

	tilemap_set_scrollx(tx_tilemap,0,-videoshift);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( hachamf )
{
//  mcu_run(screen->machine, 0);

	tilemap_set_scrollx(tx_tilemap,0,-videoshift);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( manybloc )
{
	tilemap_set_scrollx(tx_tilemap,0,-videoshift);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( tharrier )
{
	/* I think the protection device probably copies this to the regs... */
	UINT16 tharrier_scroll = nmk16_mainram[0x9f00/2];

	tilemap_set_scrollx(tx_tilemap,0,-videoshift);
	tilemap_set_scrollx(bg_tilemap0,0,tharrier_scroll);
	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( gunnail )
{
	int y1;
	int i=16;
	rectangle bgclip = *cliprect;

	// the hardware supports per-scanline X *and* Y scroll which isn't
	// supported by tilemaps so we have to draw the tilemap one line at a time
	y1 = cliprect->min_y;

	if (!nmk16_simple_scroll)
	{
		while (y1 <= cliprect->max_y)
		{
			int const yscroll = gunnail_scrollramy[0] + gunnail_scrollramy[y1];
			int tilemap_bank_select;
			tilemap_t* bg_tilemap = bg_tilemap0;

			bgclip.min_y = y1;
			bgclip.max_y = y1;


			tilemap_bank_select = (gunnail_scrollram[0]&0x3000)>>12;
			switch (tilemap_bank_select)
			{
				case 0: if (bg_tilemap0) bg_tilemap = bg_tilemap0; break;
				case 1: if (bg_tilemap1) bg_tilemap = bg_tilemap1; break;
				case 2: if (bg_tilemap2) bg_tilemap = bg_tilemap2; break;
				case 3: if (bg_tilemap3) bg_tilemap = bg_tilemap3; break;
			}

			tilemap_set_scroll_rows(bg_tilemap,512);

			tilemap_set_scrolly(bg_tilemap, 0, yscroll);
			tilemap_set_scrollx(bg_tilemap,(i + yscroll) & 0x1ff, gunnail_scrollram[0] + gunnail_scrollram[i] - videoshift);

			tilemap_draw(bitmap,&bgclip,bg_tilemap,0,0);

			y1++;
			i++;
		}
	}
	else
	{
		UINT16 yscroll = ((gunnail_scrollram[2]&0xff)<<8) | ((gunnail_scrollram[3]&0xff)<<0);
		UINT16 xscroll = ((gunnail_scrollram[0]&0xff)<<8) | ((gunnail_scrollram[1]&0xff)<<0);
		int tilemap_bank_select;
		tilemap_t* bg_tilemap = bg_tilemap0;

		//popmessage( "scroll %04x, %04x", yscroll,xscroll);

		tilemap_bank_select = (xscroll&0x3000)>>12;
		switch (tilemap_bank_select)
		{
			case 0: if (bg_tilemap0) bg_tilemap = bg_tilemap0; break;
			case 1: if (bg_tilemap1) bg_tilemap = bg_tilemap1; break;
			case 2: if (bg_tilemap2) bg_tilemap = bg_tilemap2; break;
			case 3: if (bg_tilemap3) bg_tilemap = bg_tilemap3; break;
		}

		tilemap_set_scroll_rows(bg_tilemap,1);

		tilemap_set_scrolly(bg_tilemap, 0, yscroll);
		tilemap_set_scrollx(bg_tilemap, 0, xscroll - videoshift);

		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	}

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	tilemap_set_scrollx(tx_tilemap,0,-videoshift);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( bioship )
{
	UINT16 *tilerom = (UINT16 *)memory_region(screen->machine, "gfx5");
	int scrollx=-(bioship_scroll[1] + bioship_scroll[0]*256);
	int scrolly=-(bioship_scroll[3] + bioship_scroll[2]*256);

	tilemap_set_scrollx(tx_tilemap,0,-videoshift);

	if (redraw_bitmap)
	{
		int bank = bioship_background_bank * 0x2000;
		int sx=0, sy=0, offs;
		redraw_bitmap=0;

		/* Draw background from tile rom */
		for (offs = 0;offs <0x1000;offs++) {
				UINT16 data = tilerom[offs+bank];
				int numtile = data&0xfff;
				int color = (data&0xf000)>>12;

				drawgfx_opaque(background_bitmap,0,screen->machine->gfx[3],
						numtile,
						color,
						0,0,   /* no flip */
						16*sx,16*sy);

				data = tilerom[offs+0x1000+bank];
				numtile = data&0xfff;
				color = (data&0xf000)>>12;
				drawgfx_opaque(background_bitmap,0,screen->machine->gfx[3],
						numtile,
						color,
						0,0,   /* no flip */
						16*sx,(16*sy)+256);

				sy++;
				if (sy==16) {sy=0; sx++;}
		}
	}

	copyscrollbitmap(bitmap,background_bitmap,1,&scrollx,1,&scrolly,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( strahl )
{
	tilemap_set_scrollx(tx_tilemap,0,-videoshift);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( bjtwin )
{
	tilemap_set_scrollx(bg_tilemap0,0,-videoshift);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites(screen->machine, bitmap,cliprect,0);

	return 0;
}

VIDEO_EOF( nmk )
{
	/* sprites are DMA'd from Main RAM to a private buffer automatically
       (or at least this is how I interpret the datasheet) */

	/* -- I actually see little evidence to support this, sprite lag
          in some games should be checked on real boards */

//  memcpy(spriteram_old2,spriteram_old,0x1000);
	memcpy(spriteram_old2,nmk16_mainram+0x8000/2,0x1000);
}

VIDEO_EOF( strahl )
{
	/* sprites are DMA'd from Main RAM to a private buffer automatically
       (or at least this is how I interpret the datasheet) */

	/* -- I actually see little evidence to support this, sprite lag
          in some games should be checked on real boards */

	/* strahl sprites are allocated in memory range FF000-FFFFF */

	memcpy(spriteram_old2,nmk16_mainram+0xF000/2,0x1000);
}



/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START( afega )
{
	spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);

	bg_tilemap0 = tilemap_create(	machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,

								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	tx_tilemap = tilemap_create(	machine, macross_get_tx_tile_info, tilemap_scan_cols,

								8,8,
								32,32);

	tilemap_set_transparent_pen(tx_tilemap,0xf);
}


VIDEO_START( grdnstrm )
{
	spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);


	bg_tilemap0 = tilemap_create(	machine, get_tile_info_0_8bit, afega_tilemap_scan_pages,

								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	tx_tilemap = tilemap_create(	machine, macross_get_tx_tile_info, tilemap_scan_cols,

								8,8,
								32,32);

	tilemap_set_transparent_pen(tx_tilemap,0xf);
}


VIDEO_START( firehawk )
{
	spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);


	bg_tilemap0 = tilemap_create(	machine, get_tile_info_0_8bit, afega_tilemap_scan_pages,

								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	tx_tilemap = tilemap_create(	machine, macross_get_tx_tile_info, tilemap_scan_cols,

								8,8,
								32,32);

	tilemap_set_transparent_pen(tx_tilemap,0xf);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

static void video_update(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
	int dsw_flipscreen,			// 1 = Horizontal and vertical screen flip are hardwired to 2 dip switches
	int xoffset, int yoffset,	// bg_tilemap0 offsets
	int attr_mask				// "sprite active" mask
	)
{



	if (dsw_flipscreen)
	{

		flip_screen_x_set(machine, ~input_port_read(machine, "DSW1") & 0x0100);
		flip_screen_y_set(machine, ~input_port_read(machine, "DSW1") & 0x0200);
	}


	tilemap_set_scrollx(bg_tilemap0, 0, afega_scroll_0[1] + xoffset);
	tilemap_set_scrolly(bg_tilemap0, 0, afega_scroll_0[0] + yoffset);

	tilemap_set_scrollx(tx_tilemap, 0, afega_scroll_1[1]);
	tilemap_set_scrolly(tx_tilemap, 0, afega_scroll_1[0]);


	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
}

static void redhawki_video_update(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect	)
{


	tilemap_set_scrollx(bg_tilemap0, 0, afega_scroll_1[0]&0xff);
	tilemap_set_scrolly(bg_tilemap0, 0, afega_scroll_1[1]&0xff);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,0);
}

VIDEO_UPDATE( afega )		{	video_update(screen->machine,bitmap,cliprect, 1, -0x100,+0x000, 0x0001);	return 0; }
VIDEO_UPDATE( bubl2000 )	{	video_update(screen->machine,bitmap,cliprect, 0, -0x100,+0x000, 0x0001);	return 0; }	// no flipscreen support, I really would confirmation from the schematics
VIDEO_UPDATE( redhawkb )	{	video_update(screen->machine,bitmap,cliprect, 0, +0x000,+0x100, 0x0001);	return 0; }
VIDEO_UPDATE( redhawki )	{	redhawki_video_update(screen->machine,bitmap,cliprect); return 0;} // strange scroll regs

VIDEO_UPDATE( firehawk )
{
	tilemap_set_scrolly(bg_tilemap0, 0, afega_scroll_1[1] + 0x100);
	tilemap_set_scrollx(bg_tilemap0, 0, afega_scroll_1[0]);

	tilemap_draw(bitmap,cliprect,bg_tilemap0,0,0);

	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(screen->machine, bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

