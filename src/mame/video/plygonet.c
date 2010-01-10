/*
    Polygonet Commanders (Konami, 1993)

    Video hardware emulation

    Currently contains: TTL text plane (probably not complete, needs banking? colors?)
    Needs: PSAC2 roz plane, polygons
*/

#include "emu.h"
#include "video/konicdev.h"

/* TTL text plane stuff */

static int ttl_gfx_index;
static tilemap_t *ttl_tilemap, *roz_tilemap;
static UINT16 ttl_vram[0x800], roz_vram[0x800];

/* TTL text plane */

static TILE_GET_INFO( ttl_get_tile_info )
{
	int attr, code;

	code = ttl_vram[tile_index]&0xfff;

	attr = ttl_vram[tile_index]>>12;	/* palette in all 4 bits? */

	SET_TILE_INFO(ttl_gfx_index, code, attr, 0);
}

static TILE_GET_INFO( roz_get_tile_info )
{
	int attr, code;

	attr = (roz_vram[tile_index] >> 12) + 16;	/* roz base palette is palette 16 */
	code = roz_vram[tile_index] & 0x3ff;

	SET_TILE_INFO(0, code, attr, 0);
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

READ32_HANDLER( polygonet_roz_ram_r )
{
	UINT32 *vram = (UINT32 *)roz_vram;

	return(vram[offset]);
}

WRITE32_HANDLER( polygonet_roz_ram_w )
{
	UINT32 *vram = (UINT32 *)roz_vram;

	COMBINE_DATA(&vram[offset]);

	tilemap_mark_tile_dirty(roz_tilemap, offset*2);
	tilemap_mark_tile_dirty(roz_tilemap, offset*2+1);
}

static TILEMAP_MAPPER( plygonet_scan )
{
	return row * num_cols + (col^1);
}

static TILEMAP_MAPPER( plygonet_scan_cols )
{
	return col * num_rows + (row^1);
}

VIDEO_START( polygonet )
{
	static const gfx_layout charlayout =
	{
		8, 8,	/* 8x8 */
		4096,	/* # of tiles */
		4,		/* 4bpp */
		{ 0, 1, 2, 3 },	/* plane offsets */
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },	/* X offsets */
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	/* Y offsets */
		8*8*4
	};

	/* find first empty slot to decode gfx */
	for (ttl_gfx_index = 0; ttl_gfx_index < MAX_GFX_ELEMENTS; ttl_gfx_index++)
		if (machine->gfx[ttl_gfx_index] == 0)
			break;

	assert(ttl_gfx_index != MAX_GFX_ELEMENTS);

	/* decode the ttl layer's gfx */
	machine->gfx[ttl_gfx_index] = gfx_element_alloc(machine, &charlayout, memory_region(machine, "gfx1"), machine->config->total_colors / 16, 0);

	/* create the tilemap */
	ttl_tilemap = tilemap_create(machine, ttl_get_tile_info, plygonet_scan,  8, 8, 64, 32);

	tilemap_set_transparent_pen(ttl_tilemap, 0);

	/* set up the roz t-map too */
	roz_tilemap = tilemap_create(machine, roz_get_tile_info, plygonet_scan_cols, 16, 16, 32, 64);
	tilemap_set_transparent_pen(roz_tilemap, 0);

	/* save states */
	state_save_register_global(machine, ttl_gfx_index);
	state_save_register_global_array(machine, ttl_vram);
	state_save_register_global_array(machine, roz_vram);
}

VIDEO_UPDATE( polygonet )
{
	const device_config *k053936 = devtag_get_device(screen->machine, "k053936");
	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	k053936_zoom_draw(k053936, bitmap, cliprect, roz_tilemap, 0, 0, 0);

	tilemap_draw(bitmap, cliprect, ttl_tilemap, 0, 1<<0);
	return 0;
}

