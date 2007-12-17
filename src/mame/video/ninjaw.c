#include "driver.h"
#include "video/taitoic.h"

#define TC0100SCN_GFX_NUM 1

struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};
static struct tempsprite *spritelist;

static int taito_hide_pixels;



/**********************************************************/

static VIDEO_START( ninjaw_core )
{
	int chips;

	spritelist = auto_malloc(0x1000 * sizeof(*spritelist));

	chips = number_of_TC0100SCN();

	assert_always(chips > 0, "we have an erroneous TC0100SCN configuration");

	TC0100SCN_vh_start(machine,chips,TC0100SCN_GFX_NUM,taito_hide_pixels,0,0,0,0,0,0);

	if (has_TC0110PCR())
		TC0110PCR_vh_start();

	if (has_second_TC0110PCR())
		TC0110PCR_1_vh_start();

	if (has_third_TC0110PCR())
		TC0110PCR_2_vh_start();

	/* Ensure palette from correct TC0110PCR used for each screen */
	TC0100SCN_set_chip_colbanks(0x0,0x100,0x200);
}

VIDEO_START( ninjaw )
{
	taito_hide_pixels = 22;
	video_start_ninjaw_core(machine);
}

/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int primask,int x_offs,int y_offs)
{
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, invis, curx, cury;
	int code;

#ifdef MAME_DEBUG
	int unknown=0;
#endif

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
       while processing sprite ram and then draw them all at the end */
	struct tempsprite *sprite_ptr = spritelist;

	for (offs = (spriteram_size/2)-4;offs >=0;offs -= 4)
	{
		data = spriteram16[offs+2];
		tilenum = data & 0x7fff;

		if (!tilenum) continue;

		data = spriteram16[offs+0];
//      x = (data - 8) & 0x3ff;
		x = (data - 32) & 0x3ff;	/* aligns sprites on rock outcrops and sewer hole */

		data = spriteram16[offs+1];
		y = (data - 0) & 0x1ff;

		/* don't know meaning of "invis" bit (Darius: explosions of your bomb shots) */
		data = spriteram16[offs+3];
		flipx    = (data & 0x1);
		flipy    = (data & 0x2) >> 1;
		priority = (data & 0x4) >> 2; // 1 = low
		if (priority != primask) continue;
		invis    = (data & 0x8) >> 3;
		color    = (data & 0x7f00) >> 8;

		/* Ninjaw: this stops your player flickering black and cutting into tank sprites */
		if (invis) continue;

#ifdef MAME_DEBUG
		if (data & 0x80f0)   unknown |= (data &0x80f0);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x>0x3c0) x -= 0x400;
		if (y>0x180) y -= 0x200;

		curx = x;
		cury = y;
		code = tilenum;

		sprite_ptr->code = code;
		sprite_ptr->color = color;
		sprite_ptr->flipx = flipx;
		sprite_ptr->flipy = flipy;
		sprite_ptr->x = curx;
		sprite_ptr->y = cury;

		drawgfx(bitmap,machine->gfx[0],
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				cliprect,TRANSPARENCY_PEN,0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( ninjaw )
{
	int xoffs = 36*8*screen;

	UINT8 layer[3], nodraw;

	TC0100SCN_tilemap_update(machine);

	layer[0] = TC0100SCN_bottomlayer(0);
	layer[1] = layer[0]^1;
	layer[2] = 2;

	/* chip 0 does tilemaps on the left, chip 1 center, chip 2 the right */
	// draw bottom layer
	nodraw  = TC0100SCN_tilemap_draw(machine,bitmap,cliprect,screen,layer[0],TILEMAP_DRAW_OPAQUE,0);	/* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw) fillbitmap(bitmap, get_black_pen(machine), cliprect);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(machine,bitmap,cliprect,1,xoffs,8); // draw sprites with priority 1 which are under the mid layer

	// draw middle layer
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,screen,layer[1],0,0);

	draw_sprites(machine,bitmap,cliprect,0,xoffs,8); // draw sprites with priority 0 which are over the mid layer

	// draw top(text) layer
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,screen,layer[2],0,0);
	return 0;
}
