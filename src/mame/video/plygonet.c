/*
    Polygonet Commanders (Konami, 1993)

    Video hardware emulation

    Currently contains: TTL text plane (probably not complete, needs banking? colors?)
    Needs: PSAC2 roz plane, polygons
*/

#include "driver.h"
#include "video/konamiic.h"

/* TTL text plane stuff */

static int ttl_gfx_index;
static tilemap *ttl_tilemap;
static UINT16 ttl_vram[0x800];

/* TTL text plane */

static TILE_GET_INFO( ttl_get_tile_info )
{
	int attr, code;

	code = ttl_vram[tile_index]&0xff;
	attr = 0;

	SET_TILE_INFO(ttl_gfx_index, code, attr, 0);
}

READ32_HANDLER( polygonet_ttl_ram_r )
{
	UINT32 *vram = (UINT32 *)ttl_vram;

	return(vram[offset]);
}

WRITE32_HANDLER( polygonet_ttl_ram_w )
{
	UINT32 *vram = (UINT32 *)ttl_vram;

	COMBINE_DATA(&vram[offset]);

	tilemap_mark_tile_dirty(ttl_tilemap, offset*2);
	tilemap_mark_tile_dirty(ttl_tilemap, offset*2+1);
}

VIDEO_START( polygonet )
{
	static const gfx_layout charlayout =
	{
		8, 8,		// 8x8
		4096,		// # of tiles
		4,	   	// 4bpp
		{ 0, 1, 2, 3 },	// plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },		// X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	// Y offsets
		8*8*4
	};

	/* find first empty slot to decode gfx */
	for (ttl_gfx_index = 0; ttl_gfx_index < MAX_GFX_ELEMENTS; ttl_gfx_index++)
		if (machine->gfx[ttl_gfx_index] == 0)
			break;

	assert(ttl_gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	machine->gfx[ttl_gfx_index] = allocgfx(&charlayout);
	decodegfx(machine->gfx[ttl_gfx_index], memory_region(REGION_GFX1), 0, machine->gfx[ttl_gfx_index]->total_elements);

	if (machine->drv->color_table_len)
	        machine->gfx[ttl_gfx_index]->total_colors = machine->drv->color_table_len / 16;
	else
	        machine->gfx[ttl_gfx_index]->total_colors = machine->drv->total_colors / 16;

	// create the tilemap
	ttl_tilemap = tilemap_create(ttl_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	tilemap_set_transparent_pen(ttl_tilemap, 0);

	state_save_register_global_array(ttl_vram);
}

VIDEO_UPDATE( polygonet )
{
	fillbitmap(priority_bitmap, 0, NULL);
	fillbitmap(bitmap, get_black_pen(machine), &machine->screen[0].visarea);

	tilemap_draw(bitmap, cliprect, ttl_tilemap, 0, 1<<0);
	return 0;
}

