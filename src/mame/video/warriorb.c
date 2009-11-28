#include "driver.h"
#include "video/taitoic.h"

#define TC0100SCN_GFX_NUM 1

/**********************************************************/

static void warriorb_core_vh_start(running_machine *machine, int x_offs,int multiscrn_xoffs)
{
	int chips;
	int mask;

	chips = TC0100SCN_count(machine);

	assert_always(chips > 0, "erroneous TC0100SCN configuration");

	TC0100SCN_vh_start(machine,chips,TC0100SCN_GFX_NUM,x_offs,0,0,0,0,0,multiscrn_xoffs);

	mask = TC0110PCR_mask(machine);
	if (mask & 1)
		TC0110PCR_vh_start(machine);

	if (mask & 2)
		TC0110PCR_1_vh_start(machine);

	/* Ensure palette from correct TC0110PCR used for each screen */
	TC0100SCN_set_chip_colbanks(0,0x100,0x0);
}

VIDEO_START( darius2d )
{
	warriorb_core_vh_start(machine, 4,0);
}

VIDEO_START( warriorb )
{
	warriorb_core_vh_start(machine, 4,1);
}


/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int x_offs,int y_offs)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs, data, data2, tilenum, color, flipx, flipy;
	int x, y, priority, pri_mask;

#ifdef MAME_DEBUG
	int unknown=0;
#endif

	/* pdrawgfx() needs us to draw sprites front to back */
	for (offs = 0;offs < machine->generic.spriteram_size/2;offs += 4)
	{
		data = spriteram16[offs+1];
		tilenum = data & 0x7fff;

		data = spriteram16[offs+0];
		y = (-(data &0x1ff) - 24) & 0x1ff;	/* (inverted y adjusted for vis area) */
		flipy = (data & 0x200) >> 9;

		data2 = spriteram16[offs+2];
		/* 8,4 also seen in msbyte */
		priority = (data2 & 0x0100) >> 8; // 1 = low

		if(priority)
			pri_mask = 0xfffe;
		else
			pri_mask = 0;

		color    = (data2 & 0x7f);

		data = spriteram16[offs+3];
		x = (data & 0x3ff);
		flipx = (data & 0x400) >> 10;


#ifdef MAME_DEBUG
		if (data2 & 0xf280)   unknown |= (data2 &0xf280);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x>0x3c0) x -= 0x400;
		if (y>0x180) y -= 0x200;

		pdrawgfx_transpen(bitmap,cliprect,machine->gfx[0],
		  		 tilenum,
				 color,
				 flipx,flipy,
				 x,y,
				 machine->priority_bitmap,pri_mask,0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( warriorb )
{
	int xoffs = 0, screen_number = -1;
	UINT8 layer[3], nodraw;

	const device_config *left_screen   = devtag_get_device(screen->machine, "lscreen");
	const device_config *right_screen  = devtag_get_device(screen->machine, "rscreen");

	if (screen == left_screen)
	{
		xoffs = 40*8*0;
		screen_number = 0;
	}
	else if (screen == right_screen)
	{
		xoffs = 40*8*1;
		screen_number = 1;
	}

	TC0100SCN_tilemap_update(screen->machine);

	layer[0] = TC0100SCN_bottomlayer(0);
	layer[1] = layer[0]^1;
	layer[2] = 2;

	/* Clear priority bitmap */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	/* chip 0 does tilemaps on the left, chip 1 does the ones on the right */
	// draw bottom layer
	nodraw  = TC0100SCN_tilemap_draw(screen->machine,bitmap,cliprect,screen_number,layer[0],TILEMAP_DRAW_OPAQUE,0);	/* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if(nodraw) bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	// draw middle layer
	TC0100SCN_tilemap_draw(screen->machine,bitmap,cliprect,screen_number,layer[1],0,1);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen->machine, bitmap,cliprect,xoffs,8); // draw sprites

	// draw top(text) layer
	TC0100SCN_tilemap_draw(screen->machine,bitmap,cliprect,screen_number,layer[2],0,0);
	return 0;
}
