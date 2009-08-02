#include "driver.h"
#include "includes/dooyong.h"


UINT8 *dooyong_txvideoram;

static UINT8 sprites_disabled;		/* Used by lastday/lastdaya */
static UINT8 flytiger_pri;			/* Used by flytiger */
static UINT8 tx_pri;				/* Used by sadari/gundl94/primella */
static UINT16 rshark_pri;			/* Used by rshark/superx/popbingo */

/* Up to four ROM-based and one RAM-based tilemap */
static tilemap *bg_tilemap, *bg2_tilemap, *fg_tilemap, *fg2_tilemap;
static tilemap *tx_tilemap;

/* Tilemap control registers */
static UINT8 bgscroll8[0x10] = {0}, bg2scroll8[0x10] = {0}, fgscroll8[0x10] = {0}, fg2scroll8[0x10] = {0};

/* These are set at startup to configure the tilemap callbacks */
static UINT8 *bg_tilerom, *bg2_tilerom, *fg_tilerom, *fg2_tilerom;
static UINT8 *bg_tilerom2, *bg2_tilerom2, *fg_tilerom2, *fg2_tilerom2; /* For rshark/superx */
static int bg_gfx, bg2_gfx, fg_gfx, fg2_gfx;
static int tx_tilemap_mode;	/* 0 = lastday/gulfstrm/pollux/flytiger; 1 = bluehawk/primella */


/* All the dooyong games have the same tilemap control registers */

INLINE void dooyong_scroll8_w(offs_t offset, UINT8 data, UINT8 *scroll, tilemap *map)
{
	UINT8 old = scroll[offset];
	if (old != data)
	{
		scroll[offset] = data;
		if (map != NULL) switch (offset)
		{
		case 0:	/* Low byte of x scroll - scroll tilemap */
			tilemap_set_scrollx(map, 0, data);
			break;
		case 1:	/* High byte of x scroll - mark tilemap dirty so new tile gfx will be loaded */
			tilemap_mark_all_tiles_dirty(map);
			break;
		case 3:	/* Low byte of y scroll */
		case 4:	/* High byte of y scroll */
			tilemap_set_scrolly(map, 0, (int)scroll[3] | ((int)scroll[4] << 8));
			break;
		case 6:	/* Tilemap enable and mode control */
			tilemap_set_enable(map, !(data & 0x10));
			if ((data & 0x20) != (old & 0x20))	// This sets the tilemap data format
				tilemap_mark_all_tiles_dirty(map);
			break;
		default:	/* Other addresses are used but function is unknown */
			/* 0x05 and 0x07 are initialised on startup */
			/* 0x02 is initialised on startup by some games and written to continuously by others */
			/*{
                const char *name;
                if (scroll == bgscroll8)        name = "bg";
                else if (scroll == bg2scroll8)  name = "bg2";
                else if (scroll == fgscroll8)   name = "fg";
                else if (scroll == fg2scroll8)  name = "fg2";
                else                            name = "unknown";
                printf("Unknown %s tilemap control: 0x%02x = 0x%02x\n", name, (unsigned)offset, (unsigned)data);
            }*/
			break;
		}
	}
}


/* These handle writes to the tilemap scroll registers in 8-bit machines.
   There is one per tilemap, wrapping the above function that does the work. */

WRITE8_HANDLER( dooyong_bgscroll8_w )
{
	dooyong_scroll8_w(offset, data, bgscroll8, bg_tilemap);
}

static WRITE8_HANDLER( dooyong_bg2scroll8_w )
{
	dooyong_scroll8_w(offset, data, bg2scroll8, bg2_tilemap);
}

WRITE8_HANDLER( dooyong_fgscroll8_w )
{
	dooyong_scroll8_w(offset, data, fgscroll8, fg_tilemap);
}

WRITE8_HANDLER( dooyong_fg2scroll8_w )
{
	dooyong_scroll8_w(offset, data, fg2scroll8, fg2_tilemap);
}


/* These handle writes to the tilemap scroll registers in 16-bit machines.
   This is just an 8-bit peripheral in a 16-bit machine. */

WRITE16_HANDLER( dooyong_bgscroll16_w )
{
	if (ACCESSING_BITS_0_7) dooyong_bgscroll8_w(space, offset, data & 0x00ff);
}

WRITE16_HANDLER( dooyong_bg2scroll16_w )
{
	if (ACCESSING_BITS_0_7) dooyong_bg2scroll8_w(space, offset, data & 0x00ff);
}

WRITE16_HANDLER( dooyong_fgscroll16_w )
{
	if (ACCESSING_BITS_0_7) dooyong_fgscroll8_w(space, offset, data & 0x00ff);
}

WRITE16_HANDLER( dooyong_fg2scroll16_w )
{
	if (ACCESSING_BITS_0_7) dooyong_fg2scroll8_w(space, offset, data & 0x00ff);
}


WRITE8_HANDLER( dooyong_txvideoram8_w )
{
	if (dooyong_txvideoram[offset] != data)
	{
		dooyong_txvideoram[offset] = data;
		if (tx_tilemap_mode == 0)
			tilemap_mark_tile_dirty(tx_tilemap, offset & 0x07ff);
		else
			tilemap_mark_tile_dirty(tx_tilemap, offset >> 1);
	}
}


/* Control registers seem to be different on every game */

WRITE8_HANDLER( lastday_ctrl_w )
{
	/* bits 0 and 1 are coin counters */
	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);

	/* bit 3 is used but unknown */

	/* bit 4 disables sprites */
	sprites_disabled = data & 0x10;

	/* bit 6 is flip screen */
	flip_screen_set(space->machine, data & 0x40);
}

WRITE8_HANDLER( pollux_ctrl_w )
{
	/* bit 0 is flip screen */
	flip_screen_set(space->machine, data & 0x01);

	/* bits 6 and 7 are coin counters */
	coin_counter_w(0, data & 0x80);
	coin_counter_w(1, data & 0x40);

	/* bit 1 is used but unknown */

	/* bit 2 is continuously toggled (unknown) */

	/* bit 4 is used but unknown */
}

WRITE8_HANDLER( primella_ctrl_w )
{
	/* bits 0-2 select ROM bank */
	memory_set_bank(space->machine, 1, data & 0x07);

	/* bit 3 disables tx layer */
	tx_pri = data & 0x08;

	/* bit 4 flips screen */
	flip_screen_set(space->machine, data & 0x10);

	/* bit 5 used but unknown */

//  logerror("%04x: bankswitch = %02x\n",cpu_get_pc(space->cpu),data&0xe0);
}

WRITE8_HANDLER( flytiger_ctrl_w )
{
	/* bit 0 is flip screen */
	flip_screen_set(space->machine, data & 0x01);

	/* bits 1, 2, 3 used but unknown */

	/* bit 4 changes tilemaps priority */
	flytiger_pri = data & 0x10;
}

WRITE16_HANDLER( rshark_ctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 flips screen */
		flip_screen_set(space->machine, data & 0x0001);

		/* bit 4 changes tilemaps priority */
		rshark_pri = data & 0x0010;

		/* bit 5 used but unknown */
	}
}


/* These games all have ROM-based tilemaps for the backgrounds, title
   screens and sometimes "bosses" and special attacks. There are three
   schemes for tilemap encoding.  The scheme is chosen based on the
   contents of the tilemap control variables declared above.
   Note that although the tilemaps are arbitrarily wide (hundreds of
   thousands of pixels, depending on the size of the ROM), we only
   decode a 1024 pixel wide block at a time, and invalidate the tilemap
   when the x scroll moves out of range (trying to decode the whole lot
   at once uses hundreds of megabytes of RAM). */

INLINE void lastday_get_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index,
		const UINT8 *tilerom, UINT8 *scroll, int graphics)
{
	int offs = (tile_index + ((int)scroll[1] << 6)) * 2;
	int attr = tilerom[offs];
	int code, color, flags;
	if (scroll[6] & 0x20)
	{	/* lastday/gulfstrm/pollux/flytiger */
		/* Tiles take two bytes in ROM:
                         MSB   LSB
           [offs + 0x00] cCCC CYXc    (bit 9 of gfx code, bits 3-0 of color, Y flip, X flip, bit 8 of gfx code)
           [offs + 0x01] cccc cccc    (bits 7-0 of gfx code)
           c = gfx code
           C = color code
           X = x flip
           Y = y flip */
		code = tilerom[offs + 1] | ((attr & 0x01) << 8) | ((attr & 0x80) << 2);
		color = (attr & 0x78) >> 3;
		flags = ((attr & 0x02) ? TILE_FLIPX : 0) | ((attr & 0x04) ? TILE_FLIPY : 0);
	}
	else
	{	/* primella/popbingo */
		/* Tiles take two bytes in ROM:
                         MSB   LSB
           [offs + 0x00] YXCC CCcc    (Y flip, X flip, bits 3-0 of color code, bits 9-8 of gfx code)
           [offs + 0x01] cccc cccc    (bits 7-0 of gfx code)
           c = gfx code
           C = color code
           X = x flip
           Y = y flip */
		code = tilerom[offs + 1] | ((attr & 0x03) << 8);
		color = (attr & 0x3c) >> 2;
		flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);
	}

	SET_TILE_INFO(graphics, code, color, flags);
}

INLINE void rshark_get_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index,
		const UINT8 *tilerom1, const UINT8 *tilerom2, UINT8 *scroll, int graphics)
{
		/* Tiles take two bytes in tile ROM 1:
                         MSB   LSB
           [offs + 0x00] YX?c cccc    (Y flip, X flip, bits 12-8 of gfx code)
           [offs + 0x01] cccc cccc    (bits 7-0 of gfx code)
           ? = unused/unknown
           c = gfx code
           X = x flip
           Y = y flip */
	int offs = tile_index + ((int)scroll[1] << 9);
	int attr = tilerom1[offs * 2];
	int code = tilerom1[(offs * 2) + 1] | ((attr & 0x1f) << 8);
	int color = tilerom2[offs] & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(graphics, code, color, flags);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	if (bg_tilerom2 != NULL)
		rshark_get_tile_info(machine, tileinfo, tile_index, bg_tilerom, bg_tilerom2, bgscroll8, bg_gfx);
	else
		lastday_get_tile_info(machine, tileinfo, tile_index, bg_tilerom, bgscroll8, bg_gfx);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	if (bg2_tilerom2 != NULL)
		rshark_get_tile_info(machine, tileinfo, tile_index, bg2_tilerom, bg2_tilerom2, bg2scroll8, bg2_gfx);
	else
		lastday_get_tile_info(machine, tileinfo, tile_index, bg2_tilerom, bg2scroll8, bg2_gfx);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	if (fg_tilerom2 != NULL)
		rshark_get_tile_info(machine, tileinfo, tile_index, fg_tilerom, fg_tilerom2, fgscroll8, fg_gfx);
	else
		lastday_get_tile_info(machine, tileinfo, tile_index, fg_tilerom, fgscroll8, fg_gfx);
}

static TILE_GET_INFO( get_fg2_tile_info )
{
	if (fg2_tilerom2 != NULL)
		rshark_get_tile_info(machine, tileinfo, tile_index, fg2_tilerom, fg2_tilerom2, fg2scroll8, fg2_gfx);
	else
		lastday_get_tile_info(machine, tileinfo, tile_index, fg2_tilerom, fg2scroll8, fg2_gfx);
}

/* flytiger uses some palette banking technique or something, but we
   don't know what it is.  For now, this is the same as the code used
   for the other layers (hence the really strange colour). */

static TILE_GET_INFO( flytiger_get_fg_tile_info )
{
	const UINT8 *tilerom = fg_tilerom;

	int offs = (tile_index + (fgscroll8[1] << 6)) * 2;
	int attr = tilerom[offs];
	int code = tilerom[offs + 1] | ((attr & 0x01) << 8) | ((attr & 0x80) << 2);
	int color = (attr & 0x78) >> 3; //TODO: missing 4th bit or palette bank
	int flags = ((attr & 0x02) ? TILE_FLIPX : 0) | ((attr & 0x04) ? TILE_FLIPY : 0);

	SET_TILE_INFO(fg_gfx, code, color, flags);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	/* Each tile takes two bytes of memory:
                     MSB   LSB
       [offs + 0x00] cccc cccc    (bits 7-0 of gfx code)
       [offs + 0x01] CCCC cccc    (bits 3-0 of color code, bits 11-8 of gfx code)
       c = gfx code
       C = color code */
	int offs, attr, code, color;
	if (tx_tilemap_mode == 0)
	{	/* lastday/gulfstrm/pollux/flytiger */
		offs = tile_index;
		attr = dooyong_txvideoram[offs | 0x0800];
	}
	else
	{	/* bluehawk/primella */
		offs = tile_index * 2;
		attr = dooyong_txvideoram[offs + 1];
	}
	code = dooyong_txvideoram[offs] | ((attr & 0x0f) << 8);
	color = (attr & 0xf0) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pollux_extensions)
{
	/* Sprites take 32 bytes each in memory:
                     MSB   LSB
       [offs + 0x00] cccc cccc    (bits 7-0 of gfx code)
       [offs + 0x01] cccx CCCC    (bits 10-8 of gfx code, bit 8 of x position, bits 3-0 of color code)
       [offs + 0x02] yyyy yyyy
       [offs + 0x03] xxxx xxxx    (bits 7-0 of x offset)
       ...
       [offs + 0x1c] ?hhh XY*c    (bits 2-0 of height, x flip, y flip, * see note, bit 11 of gfx code)
       ...
       ? = unused/unknown
       E = enable
       c = gfx code
       x = x position
       y = y position
       C = color code
       w = width
       X = x flip
       Y = y flip
       * = alters y position in pollux and flytiger - see code below
       bit 11 of gfx code only used by gulfstrm, pollux, bluehawk and flytiger
       height only used by pollux, bluehawk and flytiger
       x flip and y flip only used by pollux and flytiger */

	int offs;

	for (offs = 0; offs < spriteram_size; offs += 32)
	{
		int sx, sy, code, color, pri;
		int flipx = 0, flipy = 0, height = 0, y;

		sx = buffered_spriteram[offs+3] | ((buffered_spriteram[offs+1] & 0x10) << 4);
		sy = buffered_spriteram[offs+2];
		code = buffered_spriteram[offs] | ((buffered_spriteram[offs+1] & 0xe0) << 3);
		color = buffered_spriteram[offs+1] & 0x0f;
		//TODO: This priority mechanism works for known games, but seems a bit strange.
		//Are we missing something?  (The obvious spare palette bit isn't it.)
		pri = (((color == 0x00) || (color == 0x0f)) ? 0xfc : 0xf0);

		if (pollux_extensions)
		{
			/* gulfstrm, pollux, bluehawk, flytiger */
			code |= ((buffered_spriteram[offs+0x1c] & 0x01) << 11);

			if (pollux_extensions >= 2)
			{
				/* pollux, bluehawk, flytiger */
				height = (buffered_spriteram[offs+0x1c] & 0x70) >> 4;
				code &= ~height;

				if (pollux_extensions == 3)
				{
					/* bluehawk */
					sy += 6 - ((~buffered_spriteram[offs+0x1c] & 0x02) << 7);
					flipx = buffered_spriteram[offs+0x1c] & 0x08;
					flipy = buffered_spriteram[offs+0x1c] & 0x04;
				}

				if (pollux_extensions == 4)
				{
					/* flytiger */
					sy -=(buffered_spriteram[offs+0x1c] & 0x02) << 7;
					flipx = buffered_spriteram[offs+0x1c] & 0x08;
					flipy = buffered_spriteram[offs+0x1c] & 0x04;
				}
			}
		}

		if (flip_screen_get(machine))
		{
			sx = 498 - sx;
			sy = 240 - (16 * height) - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (y = 0; y <= height; y++)
		{
			pdrawgfx_transpen(bitmap, cliprect, machine->gfx[1],
					code + y,
					color,
					flipx, flipy,
					sx, sy + (16 * (flipy ? (height - y) : y)),
					machine->priority_bitmap,
					pri, 15);
		}
	}
}

static void rshark_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* Sprites take 8 16-bit words each in memory:
                  MSB             LSB
       [offs + 0] ???? ???? ???? ???E
       [offs + 1] ???? ???? hhhh wwww
       [offs + 2] ???? ???? ???? ????
       [offs + 3] cccc cccc cccc cccc
       [offs + 4] ???? ???x xxxx xxxx
       [offs + 5] ???? ???? ???? ????
       [offs + 6] ???? ???y yyyy yyyy
       [offs + 7] ???? ???? ???? CCCC
       ? = unused/unknown
       E = enable
       c = gfx code
       x = x offset
       y = y offset (signed)
       C = color code
       w = width
       h = height */

	int offs;

	for (offs = (spriteram_size / 2) - 8; offs >= 0; offs -= 8)
	{
		if (buffered_spriteram16[offs] & 0x0001)	/* enable */
		{
			int sx, sy, code, color, pri;
			int flipx = 0, flipy = 0, width, height, x, y;

			sx = buffered_spriteram16[offs+4] & 0x01ff;
			sy = (INT16)buffered_spriteram16[offs+6] & 0x01ff;
			if (sy & 0x0100) sy |= ~(int)0x01ff;	// Correctly sign-extend 9-bit number
			code = buffered_spriteram16[offs+3];
			color = buffered_spriteram16[offs+7] & 0x000f;
			//TODO: This priority mechanism works for known games, but seems a bit strange.
			//Are we missing something?  (The obvious spare palette bit isn't it.)
			pri = (((color == 0x00) || (color == 0x0f)) ? 0xfc : 0xf0);
			width = buffered_spriteram16[offs+1] & 0x000f;
			height = (buffered_spriteram16[offs+1] & 0x00f0) >> 4;

			if (flip_screen_get(machine))
			{
				sx = 498 - (16 * width) - sx;
				sy = 240 - (16 * height) - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			for (y = 0; y <= height; y++)
			{
				int _y = sy + (16 * (flipy ? (height - y) : y));
				for (x = 0; x <= width; x++)
				{
					int _x = sx + (16 * (flipx ? (width - x) : x));
					pdrawgfx_transpen(bitmap, cliprect, machine->gfx[0],
							code,
							color,
							flipx, flipy,
							_x, _y,
							machine->priority_bitmap,
							pri, 15);
					code++;
				}
			}
		}
	}
}


VIDEO_UPDATE( lastday )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 4);

	if (!sprites_disabled)
		draw_sprites(screen->machine, bitmap, cliprect, 0);
	return 0;
}

VIDEO_UPDATE( gulfstrm )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect, 1);
	return 0;
}

VIDEO_UPDATE( pollux )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect, 2);
	return 0;
}

VIDEO_UPDATE( flytiger )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (flytiger_pri)
	{
		tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 1);
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 2);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);
		tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	}
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect, 4);
	return 0;
}


VIDEO_UPDATE( bluehawk )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	tilemap_draw(bitmap, cliprect, fg2_tilemap, 0, 4);
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect, 3);
	return 0;
}

VIDEO_UPDATE( primella )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	if (tx_pri) tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	if (!tx_pri) tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( rshark )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, bg2_tilemap, 0, (rshark_pri ? 2 : 1));
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	tilemap_draw(bitmap, cliprect, fg2_tilemap, 0, 2);

	rshark_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( popbingo )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 1);

	rshark_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}


VIDEO_START( lastday )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx5");
	fg_tilerom = memory_region(machine, "gfx6");
	bg_tilerom2 = NULL;
	fg_tilerom2 = NULL;
	bg_gfx = 2;
	fg_gfx = 3;
	tx_tilemap_mode = 0;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols,
		 8, 8, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(tx_tilemap, 15);

	/* Text layer is offset on this machine */
	tilemap_set_scrolly(tx_tilemap, 0, 8);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, fgscroll8);
	state_save_register_global(machine, sprites_disabled);
}

VIDEO_START( gulfstrm )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx5");
	fg_tilerom = memory_region(machine, "gfx6");
	bg_tilerom2 = NULL;
	fg_tilerom2 = NULL;
	bg_gfx = 2;
	fg_gfx = 3;
	tx_tilemap_mode = 0;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols,
		 8, 8, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(tx_tilemap, 15);

	/* Text layer is offset on this machine */
	tilemap_set_scrolly(tx_tilemap, 0, 8);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, fgscroll8);
}

VIDEO_START( pollux )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx5");
	fg_tilerom = memory_region(machine, "gfx6");
	bg_tilerom2 = NULL;
	fg_tilerom2 = NULL;
	bg_gfx = 2;
	fg_gfx = 3;
	tx_tilemap_mode = 0;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols,
		 8, 8, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(tx_tilemap, 15);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, fgscroll8);
}

VIDEO_START( bluehawk )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx3") + 0x78000;
	fg_tilerom = memory_region(machine, "gfx4") + 0x78000;
	fg2_tilerom = memory_region(machine, "gfx5") + 0x38000;
	bg_tilerom2 = NULL;
	fg_tilerom2 = NULL;
	fg2_tilerom2 = NULL;
	bg_gfx = 2;
	fg_gfx = 3;
	fg2_gfx = 4;
	tx_tilemap_mode = 1;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg2_tilemap = tilemap_create(machine, get_fg2_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols,
		 8, 8, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(fg2_tilemap, 15);
	tilemap_set_transparent_pen(tx_tilemap, 15);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, fgscroll8);
	state_save_register_global_array(machine, fg2scroll8);
}

VIDEO_START( flytiger )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx3") + 0x78000;
	fg_tilerom = memory_region(machine, "gfx4") + 0x78000;
	bg_tilerom2 = NULL;
	fg_tilerom2 = NULL;
	bg_gfx = 2;
	fg_gfx = 3;
	tx_tilemap_mode = 0;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg_tilemap = tilemap_create(machine, flytiger_get_fg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols,
		 8, 8, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(bg_tilemap, 15);
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(tx_tilemap, 15);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, fgscroll8);
	state_save_register_global(machine, flytiger_pri);
}

VIDEO_START( primella )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx2") + memory_region_length(machine, "gfx2") - 0x8000;
	fg_tilerom = memory_region(machine, "gfx3") + memory_region_length(machine, "gfx3") - 0x8000;
	bg_tilerom2 = NULL;
	fg_tilerom2 = NULL;
	bg_gfx = 1;
	fg_gfx = 2;
	tx_tilemap_mode = 1;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols,
		 8, 8, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(tx_tilemap, 15);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, fgscroll8);
	state_save_register_global(machine, tx_pri);
}

VIDEO_START( rshark )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx5");
	bg2_tilerom = memory_region(machine, "gfx4");
	fg_tilerom = memory_region(machine, "gfx3");
	fg2_tilerom = memory_region(machine, "gfx2");
	bg_tilerom2 = memory_region(machine, "gfx6") + 0x60000;
	bg2_tilerom2 = memory_region(machine, "gfx6") + 0x40000;
	fg_tilerom2 = memory_region(machine, "gfx6") + 0x20000;
	fg2_tilerom2 = memory_region(machine, "gfx6") + 0x00000;
	bg_gfx = 4;
	bg2_gfx = 3;
	fg_gfx = 2;
	fg2_gfx = 1;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 16, 16, 64, 32);
	bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_cols,
		 16, 16, 64, 32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,
		 16, 16, 64, 32);
	fg2_tilemap = tilemap_create(machine, get_fg2_tile_info, tilemap_scan_cols,
		 16, 16, 64, 32);

	/* Configure tilemap transparency */
	tilemap_set_transparent_pen(bg2_tilemap, 15);
	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_transparent_pen(fg2_tilemap, 15);

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, bg2scroll8);
	state_save_register_global_array(machine, fgscroll8);
	state_save_register_global_array(machine, fg2scroll8);
	state_save_register_global(machine, rshark_pri);
}

VIDEO_START( popbingo )
{
	/* Configure tilemap callbacks */
	bg_tilerom = memory_region(machine, "gfx2");
	bg_gfx = 1;

	/* Create tilemaps */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 32, 8);
	bg2_tilemap = fg_tilemap = fg2_tilemap = NULL;	/* Stop scroll handler from crashing on these */

	memset(bgscroll8, 0, 0x10);
	memset(bg2scroll8, 0, 0x10);
	memset(fgscroll8, 0, 0x10);
	memset(fg2scroll8, 0, 0x10);

	/* Register for save/restore */
	state_save_register_global_array(machine, bgscroll8);
	state_save_register_global_array(machine, bg2scroll8);	// Not used atm
	state_save_register_global_array(machine, fgscroll8);	// Not used atm
	state_save_register_global_array(machine, fg2scroll8);	// Not used atm
	state_save_register_global(machine, rshark_pri);
}


VIDEO_EOF( dooyong )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}

VIDEO_EOF( rshark )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
}
