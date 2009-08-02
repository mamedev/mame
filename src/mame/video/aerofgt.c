#include "driver.h"
#include "includes/aerofgt.h"


UINT16 *aerofgt_rasterram;
UINT16 *aerofgt_bg1videoram,*aerofgt_bg2videoram;
UINT16 *aerofgt_spriteram1,*aerofgt_spriteram2,*aerofgt_spriteram3;
UINT16 *wbbc97_bitmapram;
size_t aerofgt_spriteram1_size,aerofgt_spriteram2_size,aerofgt_spriteram3_size;
UINT16 *spikes91_tx_tilemap_ram;

static UINT8 gfxbank[8];
static UINT16 bg1scrollx,bg1scrolly,bg2scrollx,bg2scrolly,wbbc97_bitmap_enable;

static int charpalettebank,spritepalettebank;

static tilemap *bg1_tilemap,*bg2_tilemap;
static int sprite_gfx;
static int spikes91_lookup;

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pspikes_tile_info )
{
	UINT16 code = aerofgt_bg1videoram[tile_index];
	int bank = (code & 0x1000) >> 12;
	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + 8 * charpalettebank,
			0);
}

static TILE_GET_INFO( karatblz_bg1_tile_info )
{
	UINT16 code = aerofgt_bg1videoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x1fff) + (gfxbank[0] << 13),
			(code & 0xe000) >> 13,
			0);
}

/* also spinlbrk */
static TILE_GET_INFO( karatblz_bg2_tile_info )
{
	UINT16 code = aerofgt_bg2videoram[tile_index];
	SET_TILE_INFO(
			1,
			(code & 0x1fff) + (gfxbank[1] << 13),
			(code & 0xe000) >> 13,
			0);
}

static TILE_GET_INFO( spinlbrk_bg1_tile_info )
{
	UINT16 code = aerofgt_bg1videoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (gfxbank[0] << 12),
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	UINT16 code = aerofgt_bg1videoram[tile_index];
	int bank = (code & 0x1800) >> 11;
	SET_TILE_INFO(
			0,
			(code & 0x07ff) + (gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	UINT16 code = aerofgt_bg2videoram[tile_index];
	int bank = 4 + ((code & 0x1800) >> 11);
	SET_TILE_INFO(
			1,
			(code & 0x07ff) + (gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void aerofgt_register_state_globals(running_machine *machine)
{
    state_save_register_global_array(machine, gfxbank);
    state_save_register_global(machine, bg1scrollx);
    state_save_register_global(machine, bg1scrolly);
    state_save_register_global(machine, bg2scrollx);
    state_save_register_global(machine, bg2scrolly);
    state_save_register_global(machine, charpalettebank);
    state_save_register_global(machine, spritepalettebank);
}

VIDEO_START( pspikes )
{
	bg1_tilemap = tilemap_create(machine, get_pspikes_tile_info,tilemap_scan_rows,8,8,64,32);
	/* no bg2 in this game */

	sprite_gfx = 1;

    aerofgt_register_state_globals(machine);
    state_save_register_global(machine, spikes91_lookup);
}


VIDEO_START( karatblz )
{
	bg1_tilemap = tilemap_create(machine, karatblz_bg1_tile_info,tilemap_scan_rows,     8,8,64,64);
	bg2_tilemap = tilemap_create(machine, karatblz_bg2_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(bg2_tilemap,15);

	spritepalettebank = 0;

	sprite_gfx = 2;

    aerofgt_register_state_globals(machine);
}

VIDEO_START( spinlbrk )
{
	int i;

	bg1_tilemap = tilemap_create(machine, spinlbrk_bg1_tile_info,tilemap_scan_rows,     8,8,64,64);
	bg2_tilemap = tilemap_create(machine, karatblz_bg2_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(bg2_tilemap,15);

	spritepalettebank = 0;

	sprite_gfx = 2;


	/* sprite maps are hardcoded in this game */

	/* enemy sprites use ROM instead of RAM */
	aerofgt_spriteram2 = (UINT16 *)memory_region(machine, "gfx5");
	aerofgt_spriteram2_size = 0x20000;

	/* front sprites are direct maps */
	aerofgt_spriteram1 = aerofgt_spriteram2 + aerofgt_spriteram2_size/2;
	aerofgt_spriteram1_size = 0x4000;
	for (i = 0;i < aerofgt_spriteram1_size/2;i++)
    {
		aerofgt_spriteram1[i] = i;
    }

    aerofgt_register_state_globals(machine);
}

VIDEO_START( turbofrc )
{
	bg1_tilemap = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_rows,     8,8,64,64);
	bg2_tilemap = tilemap_create(machine, get_bg2_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(bg2_tilemap,15);

	spritepalettebank = 0;

	sprite_gfx = 2;

    aerofgt_register_state_globals(machine);
}

VIDEO_START( wbbc97 )
{
	bg1_tilemap = tilemap_create(machine, get_pspikes_tile_info,tilemap_scan_rows,8,8,64,32);
	/* no bg2 in this game */

	tilemap_set_transparent_pen(bg1_tilemap,15);

	sprite_gfx = 1;

    aerofgt_register_state_globals(machine);

    state_save_register_global(machine, wbbc97_bitmap_enable);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( aerofgt_bg1videoram_w )
{
	COMBINE_DATA(&aerofgt_bg1videoram[offset]);
	tilemap_mark_tile_dirty(bg1_tilemap,offset);
}

WRITE16_HANDLER( aerofgt_bg2videoram_w )
{
	COMBINE_DATA(&aerofgt_bg2videoram[offset]);
	tilemap_mark_tile_dirty(bg2_tilemap,offset);
}


static void setbank(tilemap *tmap,int num,int bank)
{
	if (gfxbank[num] != bank)
	{
		gfxbank[num] = bank;
		tilemap_mark_all_tiles_dirty(tmap);
	}
}

WRITE16_HANDLER( pspikes_gfxbank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		setbank(bg1_tilemap,0,(data & 0xf0) >> 4);
		setbank(bg1_tilemap,1,data & 0x0f);
	}
}

WRITE16_HANDLER( pspikesb_gfxbank_w )
{
	COMBINE_DATA(&aerofgt_rasterram[0x200/2]);

	setbank(bg1_tilemap,0,(data & 0xf000) >> 12);
	setbank(bg1_tilemap,1,(data & 0x0f00) >> 8);
}

WRITE16_HANDLER( spikes91_lookup_w )
{
	spikes91_lookup = data & 1;
}

WRITE16_HANDLER( karatblz_gfxbank_w )
{
	if (ACCESSING_BITS_8_15)
	{
		setbank(bg1_tilemap,0,(data & 0x0100) >> 8);
		setbank(bg2_tilemap,1,(data & 0x0800) >> 11);
	}
}

WRITE16_HANDLER( spinlbrk_gfxbank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		setbank(bg1_tilemap,0,(data & 0x07));
		setbank(bg2_tilemap,1,(data & 0x38) >> 3);
	}
}

WRITE16_HANDLER( turbofrc_gfxbank_w )
{
	static UINT16 bank[2];
	tilemap *tmap = (offset == 0) ? bg1_tilemap : bg2_tilemap;

	data = COMBINE_DATA(&bank[offset]);

	setbank(tmap,4*offset + 0,(data >>  0) & 0x0f);
	setbank(tmap,4*offset + 1,(data >>  4) & 0x0f);
	setbank(tmap,4*offset + 2,(data >>  8) & 0x0f);
	setbank(tmap,4*offset + 3,(data >> 12) & 0x0f);
}

WRITE16_HANDLER( aerofgt_gfxbank_w )
{
	static UINT16 bank[4];
	tilemap *tmap = (offset < 2) ? bg1_tilemap : bg2_tilemap;

	data = COMBINE_DATA(&bank[offset]);

	setbank(tmap,2*offset + 0,(data >> 8) & 0xff);
	setbank(tmap,2*offset + 1,(data >> 0) & 0xff);
}

WRITE16_HANDLER( aerofgt_bg1scrollx_w )
{
	COMBINE_DATA(&bg1scrollx);
}

WRITE16_HANDLER( aerofgt_bg1scrolly_w )
{
	COMBINE_DATA(&bg1scrolly);
}

WRITE16_HANDLER( aerofgt_bg2scrollx_w )
{
	COMBINE_DATA(&bg2scrollx);
}

WRITE16_HANDLER( aerofgt_bg2scrolly_w )
{
	COMBINE_DATA(&bg2scrolly);
}

WRITE16_HANDLER( pspikes_palette_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		spritepalettebank = data & 0x03;
		if (charpalettebank != (data & 0x1c) >> 2)
		{
			charpalettebank = (data & 0x1c) >> 2;
			tilemap_mark_all_tiles_dirty(bg1_tilemap);
		}
	}
}

WRITE16_HANDLER( wbbc97_bitmap_enable_w )
{
	COMBINE_DATA(&wbbc97_bitmap_enable);
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void aerofgt_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int priority)
{
	int offs;


	priority <<= 12;

	offs = 0;
	while (offs < 0x0400 && (aerofgt_spriteram3[offs] & 0x8000) == 0)
	{
		int attr_start;

		attr_start = 4 * (aerofgt_spriteram3[offs] & 0x03ff);

		/* is the way I handle priority correct? Or should I just check bit 13? */
		if ((aerofgt_spriteram3[attr_start + 2] & 0x3000) == priority)
		{
			int map_start;
			int ox,oy,x,y,xsize,ysize,zoomx,zoomy,flipx,flipy,color;

			ox = aerofgt_spriteram3[attr_start + 1] & 0x01ff;
			xsize = (aerofgt_spriteram3[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (aerofgt_spriteram3[attr_start + 1] & 0xf000) >> 12;
			oy = aerofgt_spriteram3[attr_start + 0] & 0x01ff;
			ysize = (aerofgt_spriteram3[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (aerofgt_spriteram3[attr_start + 0] & 0xf000) >> 12;
			flipx = aerofgt_spriteram3[attr_start + 2] & 0x4000;
			flipy = aerofgt_spriteram3[attr_start + 2] & 0x8000;
			color = (aerofgt_spriteram3[attr_start + 2] & 0x0f00) >> 8;
			map_start = aerofgt_spriteram3[attr_start + 3] & 0x3fff;

			ox += (xsize*zoomx+2)/4;
			oy += (ysize*zoomy+2)/4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0;y <= ysize;y++)
			{
				int sx,sy;

				if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
				else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

				for (x = 0;x <= xsize;x++)
				{
					int code;

					if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
					else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

					if (map_start < 0x2000)
						code = aerofgt_spriteram1[map_start & 0x1fff] & 0x1fff;
					else
						code = aerofgt_spriteram2[map_start & 0x1fff] & 0x1fff;

					drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_gfx + (map_start >= 0x2000 ? 1 : 0)],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11, zoomy << 11,15);
					map_start++;
				}
			}
		}

		offs++;
	}
}

static void turbofrc_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int chip,int chip_disabled_pri)
{
	int attr_start,base,first;

	base = chip * 0x0200;
	first = 4 * aerofgt_spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200-8;attr_start >= first + base;attr_start -= 4)
	{
		int map_start;
		int ox,oy,x,y,xsize,ysize,zoomx,zoomy,flipx,flipy,color,pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(aerofgt_spriteram3[attr_start + 2] & 0x0080)) continue;
		pri = aerofgt_spriteram3[attr_start + 2] & 0x0010;
		if ( chip_disabled_pri & !pri) continue;
		if ((!chip_disabled_pri) & (pri>>4)) continue;
		ox = aerofgt_spriteram3[attr_start + 1] & 0x01ff;
		xsize = (aerofgt_spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (aerofgt_spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = aerofgt_spriteram3[attr_start + 0] & 0x01ff;
		ysize = (aerofgt_spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (aerofgt_spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = aerofgt_spriteram3[attr_start + 2] & 0x0800;
		flipy = aerofgt_spriteram3[attr_start + 2] & 0x8000;
		color = (aerofgt_spriteram3[attr_start + 2] & 0x000f) + 16 * spritepalettebank;

		map_start = aerofgt_spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0;y <= ysize;y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0;x <= xsize;x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = aerofgt_spriteram1[map_start % (aerofgt_spriteram1_size/2)];
				else
					code = aerofgt_spriteram2[map_start % (aerofgt_spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine->priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void spinlbrk_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int chip,int chip_disabled_pri)
{
	int attr_start,base,first;

	base = chip * 0x0200;
	first = 4 * aerofgt_spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200-8;attr_start >= first + base;attr_start -= 4)
	{
		int map_start;
		int ox,oy,x,y,xsize,ysize,zoomx,zoomy,flipx,flipy,color,pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(aerofgt_spriteram3[attr_start + 2] & 0x0080)) continue;
		pri = aerofgt_spriteram3[attr_start + 2] & 0x0010;
		if ( chip_disabled_pri & !pri) continue;
		if ((!chip_disabled_pri) & (pri>>4)) continue;
		ox = aerofgt_spriteram3[attr_start + 1] & 0x01ff;
		xsize = (aerofgt_spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (aerofgt_spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = aerofgt_spriteram3[attr_start + 0] & 0x01ff;
		ysize = (aerofgt_spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (aerofgt_spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = aerofgt_spriteram3[attr_start + 2] & 0x0800;
		flipy = aerofgt_spriteram3[attr_start + 2] & 0x8000;
		color = (aerofgt_spriteram3[attr_start + 2] & 0x000f) + 16 * spritepalettebank;

		map_start = aerofgt_spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0;y <= ysize;y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0;x <= xsize;x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = aerofgt_spriteram1[map_start % (aerofgt_spriteram1_size/2)];
				else
					code = aerofgt_spriteram2[map_start % (aerofgt_spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine->priority_bitmap,pri ? 2 : 0,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void aerfboo2_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int chip,int chip_disabled_pri)
{
	int attr_start,base,first;

	base = chip * 0x0200;
//  first = 4 * aerofgt_spriteram3[0x1fe + base];
	first = 0;

	for (attr_start = base + 0x0200-4;attr_start >= first + base;attr_start -= 4)
	{
		int map_start;
		int ox,oy,x,y,xsize,ysize,zoomx,zoomy,flipx,flipy,color,pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(aerofgt_spriteram3[attr_start + 2] & 0x0080)) continue;
		pri = aerofgt_spriteram3[attr_start + 2] & 0x0010;
		if ( chip_disabled_pri & !pri) continue;
		if ((!chip_disabled_pri) & (pri>>4)) continue;
		ox = aerofgt_spriteram3[attr_start + 1] & 0x01ff;
		xsize = (aerofgt_spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (aerofgt_spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = aerofgt_spriteram3[attr_start + 0] & 0x01ff;
		ysize = (aerofgt_spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (aerofgt_spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = aerofgt_spriteram3[attr_start + 2] & 0x0800;
		flipy = aerofgt_spriteram3[attr_start + 2] & 0x8000;
		color = (aerofgt_spriteram3[attr_start + 2] & 0x000f) + 16 * spritepalettebank;

		map_start = aerofgt_spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0;y <= ysize;y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0;x <= xsize;x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = aerofgt_spriteram1[map_start % (aerofgt_spriteram1_size/2)];
				else
					code = aerofgt_spriteram2[map_start % (aerofgt_spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine->priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void pspikesb_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int i;

	for (i = 4;i < aerofgt_spriteram3_size/2;i += 4)
	{
		int xpos,ypos,color,flipx,flipy,code;

		if (aerofgt_spriteram3[i + 3 - 4] & 0x8000) break;

		xpos = (aerofgt_spriteram3[i + 2] & 0x1ff) - 34;
		ypos = 256 - (aerofgt_spriteram3[i + 3 - 4] & 0x1ff) - 33;
		code = aerofgt_spriteram3[i + 0] & 0x1fff;
		flipy = 0;
		flipx = aerofgt_spriteram3[i + 1] & 0x0800;
		color = aerofgt_spriteram3[i + 1] & 0x000f;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[sprite_gfx],
				code,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		drawgfx_transpen(bitmap,cliprect,machine->gfx[sprite_gfx],
				code,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);

	}
}

static void spikes91_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int i;
	UINT8 *lookup;
	lookup = memory_region(machine, "user1");
	spritepalettebank = 1;

	for (i = aerofgt_spriteram3_size/2 - 4 ; i >= 4 ; i -= 4)
	{
		int xpos,ypos,color,flipx,flipy,code,realcode;

		code = aerofgt_spriteram3[i + 0] & 0x1fff;

		if (!code) continue;

		xpos = (aerofgt_spriteram3[i + 2] & 0x01ff) - 16;
		ypos = 256 - (aerofgt_spriteram3[i + 1] & 0x00ff) - 26;
		flipy = 0;
		flipx = aerofgt_spriteram3[i + 3] & 0x8000;
		color = ((aerofgt_spriteram3[i + 3] & 0x00f0) >> 4);

		code |= spikes91_lookup * 0x2000;

		realcode = (lookup[code] << 8) + lookup[0x10000 + code];

		drawgfx_transpen(bitmap,cliprect,machine->gfx[sprite_gfx],
				realcode,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		drawgfx_transpen(bitmap,cliprect,machine->gfx[sprite_gfx],
				realcode,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);
	}
}

static void aerfboot_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int attr_start,last;

	last = ((aerofgt_rasterram[0x404/2] << 5) - 0x8000) / 2;

	for (attr_start = aerofgt_spriteram3_size / 2 - 4 ; attr_start >= last ; attr_start -= 4)
	{
		int code;
		int ox,oy,sx,sy,zoomx,zoomy,flipx,flipy,color,pri;

		ox = aerofgt_spriteram3[attr_start + 1] & 0x01ff;
		oy = aerofgt_spriteram3[attr_start + 0] & 0x01ff;
		flipx = aerofgt_spriteram3[attr_start + 2] & 0x0800;
		flipy = aerofgt_spriteram3[attr_start + 2] & 0x8000;
		color = aerofgt_spriteram3[attr_start + 2] & 0x000f;

		zoomx = (aerofgt_spriteram3[attr_start + 1] & 0xf000) >> 12;
		zoomy = (aerofgt_spriteram3[attr_start + 0] & 0xf000) >> 12;
		pri = aerofgt_spriteram3[attr_start + 2] & 0x0010;
		code = aerofgt_spriteram3[attr_start + 3] & 0x1fff;

		if (!(aerofgt_spriteram3[attr_start + 2] & 0x0040)) code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_gfx + (code >= 0x1000 ? 0 : 1)],
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				machine->priority_bitmap,pri ? 0 : 2,15);

	}

	last = ((aerofgt_rasterram[0x402/2] << 5) - 0x8000) / 2;

	for (attr_start = ((aerofgt_spriteram3_size / 2) / 2) - 4 ; attr_start >= last ; attr_start -= 4)
	{
		int code;
		int ox,oy,sx,sy,zoomx,zoomy,flipx,flipy,color,pri;

		ox = aerofgt_spriteram3[attr_start + 1] & 0x01ff;
		oy = aerofgt_spriteram3[attr_start + 0] & 0x01ff;
		flipx = aerofgt_spriteram3[attr_start + 2] & 0x0800;
		flipy = aerofgt_spriteram3[attr_start + 2] & 0x8000;
		color = aerofgt_spriteram3[attr_start + 2] & 0x000f;

		zoomx = (aerofgt_spriteram3[attr_start + 1] & 0xf000) >> 12;
		zoomy = (aerofgt_spriteram3[attr_start + 0] & 0xf000) >> 12;
		pri = aerofgt_spriteram3[attr_start + 2] & 0x0010;
		code = aerofgt_spriteram3[attr_start + 3] & 0x1fff;

		if (!(aerofgt_spriteram3[attr_start + 2] & 0x0040)) code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[sprite_gfx + (code >= 0x1000 ? 0 : 1)],
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				machine->priority_bitmap,pri ? 0 : 2,15);

	}
}

static void wbbc97_draw_bitmap(bitmap_t *bitmap)
{
	int x,y,count;

	count = 16; // weird, the bitmap doesn't start at 0?
	for (y=0;y<256;y++)
		for (x=0;x<512;x++)
		{
			int color = wbbc97_bitmapram[count] >> 1;

			/* data is GRB; convert to RGB */
			rgb_t pen = MAKE_RGB(pal5bit((color & 0x3e0) >> 5), pal5bit((color & 0x7c00) >> 10), pal5bit(color & 0x1f));
			*BITMAP_ADDR32(bitmap, y, (10+x-aerofgt_rasterram[(y & 0x7f)])&0x1ff) = pen;

			count++;
			count &= 0x1ffff;
		}
}


VIDEO_UPDATE( pspikes )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,256);
	scrolly = bg1scrolly;
	for (i = 0;i < 256;i++)
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0xff,aerofgt_rasterram[i]);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0,-1);
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0, 0);
	return 0;
}

VIDEO_UPDATE( pspikesb )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,256);
	scrolly = bg1scrolly;
	for (i = 0;i < 256;i++)
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0xff,aerofgt_rasterram[i]+22);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	pspikesb_draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( spikes91 )
{
	int i,scrolly;
	int y,x;
	int count;
	const gfx_element *gfx = screen->machine->gfx[0];

	tilemap_set_scroll_rows(bg1_tilemap,256);
	scrolly = bg1scrolly;

	for (i = 0;i < 256;i++)
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0xff,aerofgt_rasterram[i+0x01f0/2]+0x96+0x16);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	spikes91_draw_sprites(screen->machine,bitmap,cliprect);

	/* we could use a tilemap, but it's easier to just do it here */
	count = 0;
	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			UINT16 tileno = spikes91_tx_tilemap_ram[count]&0x1fff;
			UINT16 colour = spikes91_tx_tilemap_ram[count]&0xe000;
			drawgfx_transpen(bitmap,cliprect,gfx,
					tileno,
					colour>>13,
					0,0,
					(x*8)+24,(y*8)+8,15);

			count++;

		}

	}

	return 0;
}

VIDEO_UPDATE( karatblz )
{
	tilemap_set_scrollx(bg1_tilemap,0,bg1scrollx-8);
	tilemap_set_scrolly(bg1_tilemap,0,bg1scrolly);
	tilemap_set_scrollx(bg2_tilemap,0,bg2scrollx-4);
	tilemap_set_scrolly(bg2_tilemap,0,bg2scrolly);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,0);

	/* we use the priority buffer so sprites are drawn front to back */
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,1,-1);
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,1, 0);
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0,-1);
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0, 0);
	return 0;
}

VIDEO_UPDATE( spinlbrk )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,512);
	scrolly = 0;
	for (i = 0;i < 256;i++)
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0x1ff,aerofgt_rasterram[i]-8);
//  tilemap_set_scrolly(bg1_tilemap,0,bg1scrolly);
	tilemap_set_scrollx(bg2_tilemap,0,bg2scrollx-4);
//  tilemap_set_scrolly(bg2_tilemap,0,bg2scrolly);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,1);

	/* we use the priority buffer so sprites are drawn front to back */
	spinlbrk_draw_sprites(screen->machine,bitmap,cliprect,0, 0);
	spinlbrk_draw_sprites(screen->machine,bitmap,cliprect,0,-1);
	spinlbrk_draw_sprites(screen->machine,bitmap,cliprect,1, 0);
	spinlbrk_draw_sprites(screen->machine,bitmap,cliprect,1,-1);
	return 0;
}

VIDEO_UPDATE( turbofrc )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,512);
	scrolly = bg1scrolly+2;
	for (i = 0;i < 256;i++)
//      tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0x1ff,aerofgt_rasterram[i]-11);
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0x1ff,aerofgt_rasterram[7]-11);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);
	tilemap_set_scrollx(bg2_tilemap,0,bg2scrollx-7);
	tilemap_set_scrolly(bg2_tilemap,0,bg2scrolly+2);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,1);

	/* we use the priority buffer so sprites are drawn front to back */
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,1,-1); //ship
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,1, 0); //intro
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0,-1); //enemy
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0, 0); //enemy
	return 0;
}

VIDEO_UPDATE( aerofgt )
{
	tilemap_set_scrollx(bg1_tilemap,0,aerofgt_rasterram[0x0000]-18);
	tilemap_set_scrolly(bg1_tilemap,0,bg1scrolly);
	tilemap_set_scrollx(bg2_tilemap,0,aerofgt_rasterram[0x0200]-20);
	tilemap_set_scrolly(bg2_tilemap,0,bg2scrolly);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);

	aerofgt_draw_sprites(screen->machine,bitmap,cliprect,0);
	aerofgt_draw_sprites(screen->machine,bitmap,cliprect,1);

	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,0);

	aerofgt_draw_sprites(screen->machine,bitmap,cliprect,2);
	aerofgt_draw_sprites(screen->machine,bitmap,cliprect,3);
	return 0;
}


VIDEO_UPDATE( aerfboot )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,512);
	scrolly = bg1scrolly+2;
	for (i = 0;i < 256;i++)
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0x1ff,aerofgt_rasterram[7]+174);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);
	tilemap_set_scrollx(bg2_tilemap,0,bg2scrollx+172);
	tilemap_set_scrolly(bg2_tilemap,0,bg2scrolly+2);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,1);

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboot_draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( aerfboo2 )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,512);
	scrolly = bg1scrolly+2;
	for (i = 0;i < 256;i++)
//      tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0x1ff,aerofgt_rasterram[i]-11);
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0x1ff,aerofgt_rasterram[7]-11);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);
	tilemap_set_scrollx(bg2_tilemap,0,bg2scrollx-7);
	tilemap_set_scrolly(bg2_tilemap,0,bg2scrolly+2);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,1);

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboo2_draw_sprites(screen->machine,bitmap,cliprect,1,-1); //ship
	aerfboo2_draw_sprites(screen->machine,bitmap,cliprect,1, 0); //intro
	aerfboo2_draw_sprites(screen->machine,bitmap,cliprect,0,-1); //enemy
	aerfboo2_draw_sprites(screen->machine,bitmap,cliprect,0, 0); //enemy
	return 0;
}

VIDEO_UPDATE( wbbc97 )
{
	int i,scrolly;

	tilemap_set_scroll_rows(bg1_tilemap,256);
	scrolly = bg1scrolly;
	for (i = 0;i < 256;i++)
		tilemap_set_scrollx(bg1_tilemap,(i + scrolly) & 0xff,aerofgt_rasterram[i]);
	tilemap_set_scrolly(bg1_tilemap,0,scrolly);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	if(wbbc97_bitmap_enable)
	{
		wbbc97_draw_bitmap(bitmap);
		tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,bg1_tilemap,TILEMAP_DRAW_OPAQUE,0);
	}

	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0,-1);
	turbofrc_draw_sprites(screen->machine,bitmap,cliprect,0, 0);
	return 0;
}
