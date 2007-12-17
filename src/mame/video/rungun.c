/*
   Run and Gun
   (c) 1993 Konami

   Video hardware emulation.

   Driver by R. Belmont
*/

#include "driver.h"
#include "video/konamiic.h"

static int ttl_gfx_index;
static tilemap *ttl_tilemap, *rng_936_tilemap;
static UINT16 ttl_vram[0x1000];

static int sprite_colorbase;
extern UINT16 *rng_936_videoram;

/* TTL text plane stuff */

static TILE_GET_INFO( ttl_get_tile_info )
{
	UINT32 *lvram = (UINT32 *)ttl_vram;
	int attr, code;

	code = (lvram[tile_index]>>16)&0xffff;
	code |= (lvram[tile_index]&0x0f)<<8;	/* tile "bank" */

	attr = ((lvram[tile_index]&0xf0)>>4);	/* palette */

	SET_TILE_INFO(ttl_gfx_index, code, attr, 0);
}

static void rng_sprite_callback(int *code, int *color, int *priority_mask)
{
	*color = sprite_colorbase | (*color & 0x001f);
}

READ16_HANDLER( ttl_ram_r )
{
	return(ttl_vram[offset]);
}

WRITE16_HANDLER( ttl_ram_w )
{
	COMBINE_DATA(&ttl_vram[offset]);
}

/* 53936 (PSAC2) rotation/zoom plane */

WRITE16_HANDLER(rng_936_videoram_w)
{
	COMBINE_DATA(&rng_936_videoram[offset]);
	tilemap_mark_tile_dirty(rng_936_tilemap, offset/2);
}

static TILE_GET_INFO( get_rng_936_tile_info )
{
	int tileno, colour, flipx;

	tileno = rng_936_videoram[tile_index*2+1] & 0x3fff;
	flipx =  (rng_936_videoram[tile_index*2+1] & 0xc000) >> 14;

	colour = 0x10 + (rng_936_videoram[tile_index*2] & 0x000f);

	SET_TILE_INFO(0, tileno, colour, TILE_FLIPYX(flipx));
}


VIDEO_START(rng)
{
	static const gfx_layout charlayout =
	{
		8, 8,	// 8x8
		4096,	// # of tiles
		4,		// 4bpp
		{ 0, 1, 2, 3 },	// plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },	// X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	// Y offsets
		8*8*4
	};

	K055673_vh_start(machine, REGION_GFX2, 1, -8, 15, rng_sprite_callback);

	K053936_wraparound_enable(0, 0);
	K053936_set_offset(0, 34, 9);

	rng_936_tilemap = tilemap_create(get_rng_936_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 128, 128);
	tilemap_set_transparent_pen(rng_936_tilemap, 0);

	/* find first empty slot to decode gfx */
	for (ttl_gfx_index = 0; ttl_gfx_index < MAX_GFX_ELEMENTS; ttl_gfx_index++)
		if (machine->gfx[ttl_gfx_index] == 0)
			break;

	assert(ttl_gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	machine->gfx[ttl_gfx_index] = allocgfx(&charlayout);
	decodegfx(machine->gfx[ttl_gfx_index], memory_region(REGION_GFX3), 0, machine->gfx[ttl_gfx_index]->total_elements);

	if (machine->drv->color_table_len)
	        machine->gfx[ttl_gfx_index]->total_colors = machine->drv->color_table_len / 16;
	else
	        machine->gfx[ttl_gfx_index]->total_colors = machine->drv->total_colors / 16;

	// create the tilemap
	ttl_tilemap = tilemap_create(ttl_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	tilemap_set_transparent_pen(ttl_tilemap, 0);

	state_save_register_global_array(ttl_vram);

	sprite_colorbase = 0x20;
}

VIDEO_UPDATE(rng)
{
	fillbitmap(bitmap, get_black_pen(machine), cliprect);
	fillbitmap(priority_bitmap, 0, cliprect);

	K053936_0_zoom_draw(bitmap, cliprect, rng_936_tilemap, 0, 0);

	K053247_sprites_draw(machine, bitmap, cliprect);

	tilemap_mark_all_tiles_dirty(ttl_tilemap);
	tilemap_draw(bitmap, cliprect, ttl_tilemap, 0, 0);
	return 0;
}
