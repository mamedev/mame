#include "driver.h"
#include "video/konamiic.h"


#define TOTAL_CHARS 0x1000
#define TOTAL_SPRITES 0x4000

UINT16 *gradius3_gfxram;
int gradius3_priority;
static int layer_colorbase[3],sprite_colorbase;



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void gradius3_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}



/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void gradius3_sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	#define L0 0xaa
	#define L1 0xcc
	#define L2 0xf0
	static const int primask[2][4] =
	{
		{ L0|L2, L0, L0|L2, L0|L1|L2 },
		{ L1|L2, L2, 0,     L0|L1|L2 }
	};
	int pri = ((*color & 0x60) >> 5);
	if (gradius3_priority == 0) *priority_mask = primask[0][pri];
	else *priority_mask = primask[1][pri];

	*code |= (*color & 0x01) << 13;
	*color = sprite_colorbase + ((*color & 0x1e) >> 1);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gradius3 )
{
	int i;

	layer_colorbase[0] = 0;
	layer_colorbase[1] = 32;
	layer_colorbase[2] = 48;
	sprite_colorbase = 16;
	K052109_vh_start(machine,"gfx1",GRADIUS3_PLANE_ORDER,gradius3_tile_callback);
	K051960_vh_start(machine,"gfx2",GRADIUS3_PLANE_ORDER,gradius3_sprite_callback);

	K052109_set_layer_offsets(2, -2, 0);
	K051960_set_sprite_offsets(2, 0);

	/* re-decode the sprites because the ROMs are connected to the custom IC differently
       from how they are connected to the CPU. */
	for (i = 0;i < TOTAL_SPRITES;i++)
		gfx_element_mark_dirty(machine->gfx[1],i);

	gfx_element_set_source(machine->gfx[0], (UINT8 *)gradius3_gfxram);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_HANDLER( gradius3_gfxrom_r )
{
	UINT8 *gfxdata = memory_region(space->machine, "gfx2");

	return (gfxdata[2*offset+1] << 8) | gfxdata[2*offset];
}

WRITE16_HANDLER( gradius3_gfxram_w )
{
	int oldword = gradius3_gfxram[offset];
	COMBINE_DATA(&gradius3_gfxram[offset]);
	if (oldword != gradius3_gfxram[offset])
		gfx_element_mark_dirty(space->machine->gfx[0], offset / 16);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( gradius3 )
{
	const address_space *space = cputag_get_address_space(screen->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	/* TODO: this kludge enforces the char banks. For some reason, they don't work otherwise. */
	K052109_w(space,0x1d80,0x10);
	K052109_w(space,0x1f00,0x32);

	K052109_tilemap_update();

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	if (gradius3_priority == 0)
	{
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,2);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,4);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,1);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,K052109_tilemap[0],TILEMAP_DRAW_OPAQUE,1);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,2);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,4);
	}

	K051960_sprites_draw(screen->machine,bitmap,cliprect,-1,-1);
	return 0;
}
