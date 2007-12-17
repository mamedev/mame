/* Space Bugger - Video Hardware */

#include "driver.h"

extern UINT8* sbugger_videoram, *sbugger_videoram_attr;

static tilemap *sbugger_tilemap;

static TILE_GET_INFO( get_sbugger_tile_info )
{
	int tileno, color;

	tileno = sbugger_videoram[tile_index];
	color = sbugger_videoram_attr[tile_index];

	SET_TILE_INFO(0,tileno,color,0);
}

WRITE8_HANDLER( sbugger_videoram_w )
{
	sbugger_videoram[offset] = data;
	tilemap_mark_tile_dirty(sbugger_tilemap,offset);
}

WRITE8_HANDLER( sbugger_videoram_attr_w )
{
	sbugger_videoram_attr[offset] = data;
	tilemap_mark_tile_dirty(sbugger_tilemap,offset);
}

VIDEO_START(sbugger)
{
	sbugger_tilemap = tilemap_create(get_sbugger_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 16,64,16);
}

VIDEO_UPDATE(sbugger)
{
	tilemap_draw(bitmap,cliprect,sbugger_tilemap,0,0);
	return 0;
}

/* not right but so we can see things ok */
PALETTE_INIT(sbugger)
{
	/* just some random colours for now */
	int i;

	for (i = 0;i < 256;i++)
	{
		int r = mame_rand(machine)|0x80;
		int g = mame_rand(machine)|0x80;
		int b = mame_rand(machine)|0x80;
		if (i == 0) r = g = b = 0;

		palette_set_color(machine,i*2+1,MAKE_RGB(r,g,b));
		palette_set_color(machine,i*2,MAKE_RGB(0,0,0));

	}

}
