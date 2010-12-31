/* video/macrossp.c */

#include "emu.h"
#include "includes/macrossp.h"


/*** SCR A LAYER ***/

WRITE32_HANDLER( macrossp_scra_videoram_w )
{
	macrossp_state *state = space->machine->driver_data<macrossp_state>();

	COMBINE_DATA(&state->scra_videoram[offset]);

	tilemap_mark_tile_dirty(state->scra_tilemap, offset);
}


static TILE_GET_INFO( get_macrossp_scra_tile_info )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();
	UINT32 attr, tileno, color;

	attr = state->scra_videoram[tile_index];
	tileno = attr & 0x0000ffff;

	switch (state->scra_videoregs[0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine->rand() & 7;
			break;
	}

	SET_TILE_INFO(1, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** SCR B LAYER ***/

WRITE32_HANDLER( macrossp_scrb_videoram_w )
{
	macrossp_state *state = space->machine->driver_data<macrossp_state>();

	COMBINE_DATA(&state->scrb_videoram[offset]);

	tilemap_mark_tile_dirty(state->scrb_tilemap, offset);
}


static TILE_GET_INFO( get_macrossp_scrb_tile_info )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();
	UINT32 attr, tileno, color;

	attr = state->scrb_videoram[tile_index];
	tileno = attr & 0x0000ffff;

	switch (state->scrb_videoregs[0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine->rand() & 7;
			break;
	}

	SET_TILE_INFO(2, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** SCR C LAYER ***/

WRITE32_HANDLER( macrossp_scrc_videoram_w )
{
	macrossp_state *state = space->machine->driver_data<macrossp_state>();

	COMBINE_DATA(&state->scrc_videoram[offset]);

	tilemap_mark_tile_dirty(state->scrc_tilemap, offset);
}


static TILE_GET_INFO( get_macrossp_scrc_tile_info )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();
	UINT32 attr, tileno, color;

	attr = state->scrc_videoram[tile_index];
	tileno = attr & 0x0000ffff;

	switch (state->scrc_videoregs[0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine->rand() & 7;
			break;
	}

	SET_TILE_INFO(3, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** TEXT LAYER ***/

WRITE32_HANDLER( macrossp_text_videoram_w )
{
	macrossp_state *state = space->machine->driver_data<macrossp_state>();

	COMBINE_DATA(&state->text_videoram[offset]);

	tilemap_mark_tile_dirty(state->text_tilemap, offset);
}


static TILE_GET_INFO( get_macrossp_text_tile_info )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();
	UINT32 tileno, colour;

	tileno = state->text_videoram[tile_index] & 0x0000ffff;
	colour = (state->text_videoram[tile_index] & 0x00fe0000) >> 17;

	SET_TILE_INFO(4, tileno, colour, 0);
}



/*** VIDEO START / UPDATE ***/

VIDEO_START( macrossp )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();

	state->spriteram_old = auto_alloc_array_clear(machine, UINT32, state->spriteram_size / 4);
	state->spriteram_old2 = auto_alloc_array_clear(machine, UINT32, state->spriteram_size / 4);

	state->text_tilemap = tilemap_create(machine, get_macrossp_text_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->scra_tilemap = tilemap_create(machine, get_macrossp_scra_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->scrb_tilemap = tilemap_create(machine, get_macrossp_scrb_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->scrc_tilemap = tilemap_create(machine, get_macrossp_scrc_tile_info, tilemap_scan_rows, 16, 16, 64, 64);

	tilemap_set_transparent_pen(state->text_tilemap, 0);
	tilemap_set_transparent_pen(state->scra_tilemap, 0);
	tilemap_set_transparent_pen(state->scrb_tilemap, 0);
	tilemap_set_transparent_pen(state->scrc_tilemap, 0);

	machine->gfx[0]->color_granularity = 64;
	machine->gfx[1]->color_granularity = 64;
	machine->gfx[2]->color_granularity = 64;
	machine->gfx[3]->color_granularity = 64;

	state_save_register_global_pointer(machine, state->spriteram_old, state->spriteram_size / 4);
	state_save_register_global_pointer(machine, state->spriteram_old2, state->spriteram_size / 4);
}



static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();
	const gfx_element *gfx = machine->gfx[0];
	//  UINT32 *source = state->spriteram;
	UINT32 *source = state->spriteram_old2; /* buffers by two frames */
	UINT32 *finish = source + state->spriteram_size / 4;

	while (source < finish)
	{

		/*

         --hh hhyy yyyy yyyy   CCww wwxx xxxx xxxx

         ---- --zz zzzz zzzz   ---- --ZZ ZZZZ ZZZZ

         fFa- pp-- cccc c---   tttt tttt tttt tttt

         */


		int wide = (source[0] & 0x00003c00) >> 10;
		int high = (source[0] & 0x3c000000) >> 26;

		int xpos = (source[0] & 0x000003ff) >> 0;
		int ypos = (source[0] & 0x03ff0000) >> 16;

		int xzoom = (source[1] & 0x000003ff) >> 0; /* 0x100 is zoom factor of 1.0 */
		int yzoom = (source[1] & 0x03ff0000) >> 16;

		int col;
		int tileno = (source[2] & 0x0000ffff) >> 0;

		int flipx = (source[2] & 0x40000000) >> 30;
		int flipy = (source[2] & 0x80000000) >> 31;

		int alpha = (source[2] & 0x20000000)?0x80:0xff; /* alpha blending enable? */

		int loopno = 0;

		int xcnt, ycnt;
		int xoffset, yoffset;

		int pri = (source[2] & 0x0c000000) >> 26;

		if (pri == priority)
		{
			switch (source[0] & 0x0000c000)
			{
				case 0x00008000:
					col = (source[2] & 0x00380000) >> 17;
					break;

				case 0x00004000:
					col = (source[2] & 0x00f80000) >> 19;
					break;

				default:
					col = machine->rand();
					break;
			}


			if (xpos > 0x1ff) xpos -=0x400;
			if (ypos > 0x1ff) ypos -=0x400;

			if (!flipx)
			{
				if (!flipy)
				{
					/* noxflip, noyflip */
					yoffset = 0; /* I'm doing this so rounding errors are cumulative, still looks a touch crappy when multiple sprites used together */
					for (ycnt = 0; ycnt <= high; ycnt++)
					{
						xoffset = 0;
						for (xcnt = 0; xcnt <= wide; xcnt++)
						{
							drawgfxzoom_alpha(bitmap,cliprect,gfx,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset += ((xzoom*16 + (1<<7)) >> 8);
							loopno++;
						}
						yoffset += ((yzoom*16 + (1<<7)) >> 8);
					}
				}
				else
				{
					/* noxflip, flipy */
					yoffset = ((high * yzoom * 16) >> 8);
					for (ycnt = high; ycnt >= 0; ycnt--)
					{
						xoffset = 0;
						for (xcnt = 0; xcnt <= wide; xcnt++)
						{
							drawgfxzoom_alpha(bitmap,cliprect,gfx,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset += ((xzoom * 16 + (1 << 7)) >> 8);
							loopno++;
						}
						yoffset -= ((yzoom * 16 + (1 << 7)) >> 8);
					}
				}
			}
			else
			{
				if (!flipy)
				{
					/* xflip, noyflip */
					yoffset = 0;
					for (ycnt = 0; ycnt <= high; ycnt++)
					{
						xoffset = ((wide*xzoom*16) >> 8);
						for (xcnt = wide; xcnt >= 0; xcnt--)
						{
							drawgfxzoom_alpha(bitmap,cliprect,gfx,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset -= ((xzoom * 16 + (1 << 7)) >> 8);
							loopno++;
						}
						yoffset += ((yzoom * 16 + (1 << 7)) >> 8);
					}
				}
				else
				{
					/* xflip, yflip */
					yoffset = ((high * yzoom * 16) >> 8);
					for (ycnt = high; ycnt >= 0; ycnt--)
					{
						xoffset = ((wide * xzoom * 16) >> 8);
						for (xcnt = wide; xcnt >=0 ; xcnt--)
						{
							drawgfxzoom_alpha(bitmap,cliprect,gfx,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset -= ((xzoom * 16 + (1 << 7)) >> 8);
							loopno++;
						}
						yoffset -= ((yzoom * 16 + (1 << 7)) >> 8);
					}
				}
			}
		}
		source += 3;
	}
}


static void draw_layer( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int layer )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();
	tilemap_t *tm;
	UINT32 *vr;

	switch (layer)
	{
		case 0:
		default:
			tm = state->scra_tilemap;
			vr = state->scra_videoregs;
			break;

		case 1:
			tm = state->scrb_tilemap;
			vr = state->scrb_videoregs;
			break;

		case 2:
			tm = state->scrc_tilemap;
			vr = state->scrc_videoregs;
			break;
	}

	if ((vr[2] & 0xf0000000) == 0xe0000000)	/* zoom enable (guess, surely wrong) */
	{
		int startx, starty, inc;

		startx = (vr[1] & 0x0000ffff) << 16;
		starty = (vr[1] & 0xffff0000) >> 0;
		inc = (vr[2] & 0x00ff0000) >> 6,

		/* WRONG! */
		/* scroll register contain position relative to the center of the screen, so adjust */
		startx -= (368/2) * inc;
		starty -= (240/2) * inc;

		tilemap_draw_roz(bitmap,cliprect,tm,
				startx,starty,inc,0,0,inc,
				1,	/* wraparound */
				0,0);
	}
	else
	{
		tilemap_set_scrollx( tm, 0, ((vr[0] & 0x000003ff) >> 0 ) );
		tilemap_set_scrolly( tm, 0, ((vr[0] & 0x03ff0000) >> 16) );
		tilemap_draw(bitmap, cliprect, tm, 0, 0);
	}
}

/* useful function to sort the three tile layers by priority order */
static void sortlayers(int *layer,int *pri)
{
#define SWAP(a,b) \
	if (pri[a] >= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

VIDEO_UPDATE( macrossp )
{
	macrossp_state *state = screen->machine->driver_data<macrossp_state>();
	int layers[3],layerpri[3];

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	layers[0] = 0;
	layerpri[0] = (state->scra_videoregs[0] & 0x0000c000) >> 14;
	layers[1] = 1;
	layerpri[1] = (state->scrb_videoregs[0] & 0x0000c000) >> 14;
	layers[2] = 2;
	layerpri[2] = (state->scrc_videoregs[0] & 0x0000c000) >> 14;

	sortlayers(layers, layerpri);

	draw_layer(screen->machine, bitmap, cliprect, layers[0]);
	draw_sprites(screen->machine, bitmap, cliprect, 0);
	draw_layer(screen->machine, bitmap, cliprect, layers[1]);
	draw_sprites(screen->machine, bitmap, cliprect, 1);
	draw_layer(screen->machine, bitmap, cliprect, layers[2]);
	draw_sprites(screen->machine, bitmap, cliprect, 2);
	draw_sprites(screen->machine, bitmap, cliprect, 3);
	tilemap_draw(bitmap, cliprect, state->text_tilemap, 0, 0);

#if 0
popmessage	("scra - %08x %08x %08x\nscrb - %08x %08x %08x\nscrc - %08x %08x %08x",
state->scra_videoregs[0]&0xffff33ff, // yyyyxxxx
state->scra_videoregs[1], // ??? more scrolling?
state->scra_videoregs[2], // 08 - 0b

state->scrb_videoregs[0]&0xffff33ff, // 00 - 03
state->scrb_videoregs[1], // 04 - 07
state->scrb_videoregs[2], // 08 - 0b

state->scrc_videoregs[0]&0xffff33ff, // 00 - 03
state->scrc_videoregs[1], // 04 - 07
state->scrc_videoregs[2]);// 08 - 0b
#endif
	return 0;
}

VIDEO_EOF( macrossp )
{
	macrossp_state *state = machine->driver_data<macrossp_state>();

	/* looks like sprites are *two* frames ahead, like nmk16 */
	memcpy(state->spriteram_old2, state->spriteram_old, state->spriteram_size);
	memcpy(state->spriteram_old, state->spriteram, state->spriteram_size);
}
