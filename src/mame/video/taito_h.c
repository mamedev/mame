/***************************************************************************

Functions to emulate the video hardware of the machine.

TODO
====

Harmonise draw_sprites code and offload it into taitoic.c, leaving this
as short as possible (TC0080VCO has sprites as well as tilemaps in its
address space).

Maybe wait until the Taito Air system is done - also uses TC0080VCO.

Why does syvalion draw_sprites ignore the zoomy value
(using zoomx instead) ???


Sprite ram notes
----------------

BG / chain RAM
-----------------------------------------
[0]  +0         +1
     -xxx xxxx  xxxx xxxx = tile number

[1]  +0         +1
     ---- ----  x--- ---- = flip Y
     ---- ----  -x-- ---- = flip X
     ---- ----  --xx xxxx = color


sprite RAM
--------------------------------------------------------------
 +0         +1         +2         +3
 ---x ----  ---- ----  ---- ----  ---- ---- = unknown
 ---- xx--  ---- ----  ---- ----  ---- ---- = chain y size
 ---- --xx  xxxx xxxx  ---- ----  ---- ---- = sprite x coords
 ---- ----  ---- ----  ---- --xx  xxxx xxxx = sprite y coords

 +4         +5         +6         +7
 --xx xxxx  ---- ----  ---- ----  ---- ---- = zoom x
 ---- ----  --xx xxxx  ---- ----  ---- ---- = zoom y
 ---- ----  ---- ----  ---x xxxx  xxxx xxxx = tile information offset


***************************************************************************/

#include "driver.h"
#include "taitoic.h"


extern UINT16	*TC0080VCO_chain_ram_0;
extern UINT16	*TC0080VCO_chain_ram_1;
extern UINT16	*TC0080VCO_spriteram;
extern UINT16	*TC0080VCO_scroll_ram;
extern int		TC0080VCO_flipscreen;

/* Needed in the sprite palette color marking */
extern int TC0080VCO_has_tx;

/* These are hand-tuned values */
static int zoomy_conv_table[] =
{
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,	/* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,	/* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};



/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

VIDEO_START( syvalion )
{
	TC0080VCO_vh_start(machine,0,1,1,1,-2);
}

VIDEO_START( recordbr )
{
	TC0080VCO_vh_start(machine,0,0,1,1,-2);
}

VIDEO_START( dleague )
{
	TC0080VCO_vh_start(machine,0,0,1,1,-2);
}


/***************************************************************************
  Screen refresh
***************************************************************************/

static void syvalion_draw_sprites(mame_bitmap *bitmap,const rectangle *cliprect)
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
       is always 64 pixels. */

	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, ex, zx;
	int ysize;
	int j, k;
	int offs;					/* sprite RAM offset */
	int tile_offs;				/* sprite chain offset */
	int zoomx;					/* zoomx value */

	for (offs = 0x03f8 / 2 ; offs >= 0 ; offs -= 0x008 / 2)
	{
		x0        =  TC0080VCO_spriteram[offs + 1] & 0x3ff;
		y0        =  TC0080VCO_spriteram[offs + 0] & 0x3ff;
		zoomx     = (TC0080VCO_spriteram[offs + 2] & 0x7f00) >> 8;
		tile_offs = (TC0080VCO_spriteram[offs + 3] & 0x1fff) << 2;
		ysize     = size[ ( TC0080VCO_spriteram[ offs ] & 0x0c00 ) >> 10 ];

		if (tile_offs)
		{
			/* The increasing ratio of expansion is different whether zoom value */
			/* is less or more than 63.                                          */
			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (TC0080VCO_flipscreen)
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			y = y0;
			for ( j = 0 ; j < ysize ; j ++ )
			{
				x = x0;
				for (k = 0 ; k < 4 ; k ++ )
				{
					if (tile_offs >= 0x1000)
					{
						int tile, color, flipx, flipy;

						tile  = TC0080VCO_chain_ram_0[tile_offs] & 0x7fff;
						color = TC0080VCO_chain_ram_1[tile_offs] & 0x001f;
						flipx = TC0080VCO_chain_ram_1[tile_offs] & 0x0040;
						flipy = TC0080VCO_chain_ram_1[tile_offs] & 0x0080;

						if (TC0080VCO_flipscreen)
						{
							flipx ^= 0x0040;
							flipy ^= 0x0080;
						}

						drawgfxzoom( bitmap, Machine -> gfx[0],
								 tile,
								 color,
								 flipx, flipy,
								 x, y,
								 cliprect,
								 TRANSPARENCY_PEN, 0,
								 zx, zx
						);
					}
					tile_offs ++;
					x += dx;
				}
				y += dx;
			}
		}
	}
}

static void recordbr_draw_sprites(mame_bitmap *bitmap, const rectangle *cliprect, int priority)
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
       is always 64 pixels. */

	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, dy, ex, ey, zx, zy;
	int ysize;
	int j, k;
	int offs;					/* sprite RAM offset */
	int tile_offs;				/* sprite chain offset */
	int zoomx, zoomy;			/* zoom value */

	for (offs = 0x03f8 / 2 ; offs >= 0 ; offs -= 0x008 / 2)
	{
		if (offs <  0x01b0 && priority == 0)	continue;
		if (offs >= 0x01b0 && priority == 1)	continue;

		x0        =  TC0080VCO_spriteram[offs + 1] & 0x3ff;
		y0        =  TC0080VCO_spriteram[offs + 0] & 0x3ff;
		zoomx     = (TC0080VCO_spriteram[offs + 2] & 0x7f00) >> 8;
		zoomy     = (TC0080VCO_spriteram[offs + 2] & 0x007f);
		tile_offs = (TC0080VCO_spriteram[offs + 3] & 0x1fff) << 2;
		ysize     = size[ ( TC0080VCO_spriteram[ offs ] & 0x0c00 ) >> 10 ];

		if (tile_offs)
		{
			/* Convert zoomy value to real value as zoomx */
			zoomy = zoomy_conv_table[zoomy];

			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (zoomy < 63)
			{
				dy = 8 + (zoomy + 2) / 8;
				ey = (zoomy + 2) % 8;
				zy = ((dy << 1) + ey) << 11;
			}
			else
			{
				dy = 16 + (zoomy - 63) / 4;
				ey = (zoomy - 63) % 4;
				zy = (dy + ey) << 12;
			}

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (TC0080VCO_flipscreen)
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
				dy = -dy;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			y = y0;
			for (j = 0 ; j < ysize ; j ++)
			{
				x = x0;
				for (k = 0 ; k < 4 ; k ++)
				{
					if (tile_offs >= 0x1000)
					{
						int tile, color, flipx, flipy;

						tile  = TC0080VCO_chain_ram_0[tile_offs] & 0x7fff;
						color = TC0080VCO_chain_ram_1[tile_offs] & 0x001f;
						flipx = TC0080VCO_chain_ram_1[tile_offs] & 0x0040;
						flipy = TC0080VCO_chain_ram_1[tile_offs] & 0x0080;

						if (TC0080VCO_flipscreen)
						{
							flipx ^= 0x0040;
							flipy ^= 0x0080;
						}

						drawgfxzoom( bitmap, Machine -> gfx[0],
								 tile,
								 color,
								 flipx, flipy,
								 x, y,
								 cliprect,
								 TRANSPARENCY_PEN, 0,
								 zx, zy
						);
					}
					tile_offs ++;
					x += dx;
				}
				y += dy;
			}
		}
	}
}

static void dleague_draw_sprites(mame_bitmap *bitmap, const rectangle *cliprect, int priority)
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
       is always 64 pixels. */

	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, ex, zx;
	int ysize;
	int j, k;
	int offs;					/* sprite RAM offset */
	int tile_offs;				/* sprite chain offset */
	int zoomx;					/* zoomx value */
	int pribit;

	for (offs = 0x03f8 / 2 ; offs >= 0 ; offs -= 0x008 / 2)
	{
		x0        =  TC0080VCO_spriteram[offs + 1] & 0x3ff;
		y0        =  TC0080VCO_spriteram[offs + 0] & 0x3ff;
		zoomx     = (TC0080VCO_spriteram[offs + 2] & 0x7f00) >> 8;
		tile_offs = (TC0080VCO_spriteram[offs + 3] & 0x1fff) << 2;
		pribit    = (TC0080VCO_spriteram[offs + 0] & 0x1000) >> 12;
		ysize     = size[ ( TC0080VCO_spriteram[ offs ] & 0x0c00 ) >> 10 ];

		if (tile_offs)
		{
			/* The increasing ratio of expansion is different whether zoom value */
			/* is less or more than 63.                                          */
			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
				pribit = 0;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (TC0080VCO_scroll_ram[0x0002] & 0x8000)
				pribit = 1;

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (TC0080VCO_flipscreen)
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			if ( priority == pribit )
			{
				y = y0;
				for ( j = 0 ; j < ysize ; j ++ )
				{
					x = x0;
					for (k = 0 ; k < 4 ; k ++ )
					{
						if (tile_offs >= 0x1000)	/* or pitcher gets blanked */
						{
							int tile, color, flipx, flipy;

							tile  = TC0080VCO_chain_ram_0[tile_offs] & 0x7fff;
							color = TC0080VCO_chain_ram_1[tile_offs] & 0x001f;
							flipx = TC0080VCO_chain_ram_1[tile_offs] & 0x0040;
							flipy = TC0080VCO_chain_ram_1[tile_offs] & 0x0080;

							if (TC0080VCO_flipscreen)
							{
								flipx ^= 0x0040;
								flipy ^= 0x0080;
							}

							drawgfxzoom( bitmap, Machine -> gfx[0],
									 tile,
									 color,
									 flipx, flipy,
									 x, y,
									 cliprect,
									 TRANSPARENCY_PEN, 0,
									 zx, zx
							);
						}
						tile_offs ++;
						x += dx;
					}
					y += dx;
				}
			}
		}
	}
}



#ifdef MAME_DEBUG

static void taitoh_log_vram(void)
{
	// null function: the necessary pointers are now internal to taitoic.c
	// Recreate it there if wanted (add prototype to taitoic.h)
}

#endif


/**************************************************************************/

VIDEO_UPDATE( syvalion )
{
	TC0080VCO_tilemap_update(machine);

#ifdef MAME_DEBUG
	taitoh_log_vram();
#endif

	fillbitmap(bitmap, machine->pens[0], cliprect);

	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,0,TILEMAP_DRAW_OPAQUE,0);
	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,1,0,0);
	syvalion_draw_sprites (bitmap,cliprect);
	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,2,0,0);
	return 0;
}


VIDEO_UPDATE( recordbr )
{
	TC0080VCO_tilemap_update(machine);

#ifdef MAME_DEBUG
	taitoh_log_vram();
#endif

	fillbitmap(bitmap, machine->pens[0], cliprect);

#ifdef MAME_DEBUG
	if ( !input_code_pressed(KEYCODE_A) )
		TC0080VCO_tilemap_draw(machine,bitmap,cliprect,0,TILEMAP_DRAW_OPAQUE,0);
	if ( !input_code_pressed(KEYCODE_S) )
		recordbr_draw_sprites(bitmap,cliprect,0);
	if ( !input_code_pressed(KEYCODE_D) )
		TC0080VCO_tilemap_draw(machine,bitmap,cliprect,1,0,0);
	if ( !input_code_pressed(KEYCODE_F) )
		recordbr_draw_sprites(bitmap,cliprect,1);
#else
	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,0,TILEMAP_DRAW_OPAQUE,0);
	recordbr_draw_sprites (bitmap,cliprect,0);
	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,1,0,0);
	recordbr_draw_sprites (bitmap,cliprect,1);
#endif

	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,2,0,0);
	return 0;
}


VIDEO_UPDATE( dleague )
{
	TC0080VCO_tilemap_update(machine);

#ifdef MAME_DEBUG
	taitoh_log_vram();
#endif

	fillbitmap(bitmap, machine->pens[0], cliprect);

#ifdef MAME_DEBUG
	if ( !input_code_pressed(KEYCODE_A) )
		TC0080VCO_tilemap_draw(machine,bitmap,cliprect,0,TILEMAP_DRAW_OPAQUE,0);
	if ( !input_code_pressed(KEYCODE_S) )
		dleague_draw_sprites(bitmap,cliprect,0);
	if ( !input_code_pressed(KEYCODE_D) )
		TC0080VCO_tilemap_draw(machine,bitmap,cliprect,1,0,0);
	if ( !input_code_pressed(KEYCODE_F) )
		dleague_draw_sprites(bitmap,cliprect,1);
#else
	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,0,TILEMAP_DRAW_OPAQUE,0);
	dleague_draw_sprites  (bitmap,cliprect,0);
	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,1,0,0);
	dleague_draw_sprites  (bitmap,cliprect,1);
#endif

	TC0080VCO_tilemap_draw(machine,bitmap,cliprect,2,0,0);
	return 0;
}

