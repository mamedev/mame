#include "emu.h"

UINT8 *usgames_videoram,*usgames_charram;


static tilemap_t *usgames_tilemap;



PALETTE_INIT(usgames)
{
	int j;

	for (j = 0; j < 0x200; j++)
	{
		int data;
		int r, g, b, i;

		if (j & 0x01)
			data = (j >> 5) & 0x0f;
		else
			data = (j >> 1) & 0x0f;

		r = (data & 1) >> 0;
		g = (data & 2) >> 1;
		b = (data & 4) >> 2;
		i = (data & 8) >> 3;

		r = 0xff * r;
		g = 0x7f * g * (i + 1);
		b = 0x7f * b * (i + 1);

		palette_set_color(machine,j,MAKE_RGB(r, g, b));
	}
}



static TILE_GET_INFO( get_usgames_tile_info )
{
	int tileno, colour;

	tileno = usgames_videoram[tile_index*2];
	colour = usgames_videoram[tile_index*2+1];

	SET_TILE_INFO(0,tileno,colour,0);
}

VIDEO_START(usgames)
{
	usgames_tilemap = tilemap_create(machine, get_usgames_tile_info,tilemap_scan_rows, 8, 8,64,32);
	gfx_element_set_source(machine->gfx[0], usgames_charram);
}


WRITE8_HANDLER( usgames_videoram_w )
{
	usgames_videoram[offset] = data;
	tilemap_mark_tile_dirty(usgames_tilemap,offset/2);
}

WRITE8_HANDLER( usgames_charram_w )
{
	usgames_charram[offset] = data;
	gfx_element_mark_dirty(space->machine->gfx[0], offset/8);
}


VIDEO_UPDATE(usgames)
{
	tilemap_draw(bitmap,cliprect,usgames_tilemap,0,0);
	return 0;
}
