#include "driver.h"
#include "snk.h"

/*******************************************************************************
 Shadow Handling Notes
********************************************************************************
 previously shadows were handled by toggling them on and off with a
 shadows_visible flag.

 Games Not Using Shadows?

 those using gwar_vh_screenrefresh (gwar, bermudat, psychos, chopper1)
    (0-15 , 15 is transparent)

 Games Using Shadows?

 those using tnk3_vh_screenrefresh (tnk3, athena, fitegolf) sgladiat is similar
    (0-7  , 6  is shadow, 7  is transparent) * these are using aso colour prom convert *
 those using tdfever_vh_screenrefresh (tdfever)
    (0-15 , 14(13 for tdfeverj) is shadow, 15 is transparent)
 those using ftsoccer_vh_screenrefresh (ftsoccer)
    (0-15 , 14 is shadow/highlight, 15 is transparent)
 those using ikari_vh_screenrefresh (ikari, victroad)
    (0-7  , 6  is shadow, 7  is transparent)

*******************************************************************************/


int snk_blink_parity = 0;

#define MAX_VRAM_SIZE (64*64*2) /* 0x2000 */




PALETTE_INIT( snk_3bpp_shadow )
{
	int i;
	palette_init_RRRR_GGGG_BBBB(machine, colortable, color_prom);

	if(!(machine->drv->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 5; i++) gfx_drawmode_table[i] = DRAWMODE_SOURCE;

	gfx_drawmode_table[6] = DRAWMODE_SHADOW;
	gfx_drawmode_table[7] = DRAWMODE_NONE;
}

PALETTE_INIT( snk_4bpp_shadow )
{
	int i;
	palette_init_RRRR_GGGG_BBBB(machine, colortable, color_prom);

	if(!(machine->drv->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 13; i++) gfx_drawmode_table[i] = DRAWMODE_SOURCE;

	gfx_drawmode_table[14] = DRAWMODE_SHADOW;
	gfx_drawmode_table[15] = DRAWMODE_NONE;
}

VIDEO_START( snk )
{
	snk_blink_parity = 0;

	dirtybuffer = auto_malloc( MAX_VRAM_SIZE );

	tmpbitmap = auto_bitmap_alloc( 512, 512, machine->screen[0].format );

	memset( dirtybuffer, 0xff, MAX_VRAM_SIZE );
}

/**************************************************************************************/

static void tnk3_draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly,
					int x_size, int y_size, int bg_type )
{
	const gfx_element *gfx = machine->gfx[1];

	int tile_number, attributes, color, sx, sy;
	int offs, x, y;

	/* to be moved to memmap */
	for(x=0; x<x_size; x++) for(y=0; y<y_size; y++)
	{
		offs = (x*y_size + y) << 1;
		tile_number = videoram[offs];
		attributes  = videoram[offs+1];

		if(tile_number != dirtybuffer[offs] || attributes != dirtybuffer[offs+1])
		{
			dirtybuffer[offs]   = tile_number;
			dirtybuffer[offs+1] = attributes;

			if(bg_type == 0)
			{
					/* type tnk3 */
					tile_number |= (attributes & 0x30) << 4;
					color = (attributes & 0xf) ^ 8;
			}
			else
			{
					/* type ikari */
					tile_number |= (attributes & 0x03) << 8;
					color = attributes >> 4;
			}

			sx = x * 512 / x_size;
			sy = y * 512 / y_size;

			drawgfx(tmpbitmap,gfx,tile_number,color,0,0,sx,sy,0,TRANSPARENCY_NONE,0);
		}
	}
	copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,1,&scrolly,cliprect,TRANSPARENCY_NONE,0);
}

void tnk3_draw_text(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int bank, UINT8 *source )
{
	const gfx_element *gfx = machine->gfx[0];

	int tile_number, color, sx, sy;
	int x, y;

	for(x=0; x<32; x++) for(y=0; y<32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(tile_number == 0x20 || tile_number == 0xff) continue;

		if(bank == -1) color = 8;
		else
		{
			color = tile_number >> 5;
			tile_number |= bank << 8;
		}
		sx = (x+2) << 3;
		sy = (y+1) << 3;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN,15);
	}
}

static void tnk3_draw_status_main(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int bank, UINT8 *source, int start )
{
	const gfx_element *gfx = machine->gfx[0];

	int tile_number, color, sx, sy;
	int x, y;

	for(x = start; x < start+2; x++) for(y = 0; y < 32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(bank == -1) color = 8;
		else
		{
 			color = tile_number >> 5;
			tile_number |= (bank << 8);
		}
		sx = ((x+34)&0x3f) << 3;
		sy = (y+1) << 3;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_NONE,0);
	}
}

void tnk3_draw_status(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int bank, UINT8 *source )
{
	tnk3_draw_status_main(machine,bitmap,cliprect,bank,source, 0);
	tnk3_draw_status_main(machine,bitmap,cliprect,bank,source,30);
}

static void tnk3_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int xscroll, int yscroll )
{
	const gfx_element *gfx = machine->gfx[2];

	int tile_number, attributes, color, sx, sy;
	int offs;

	for(offs = 0; offs < 50*4; offs+=4)
	{
		if(*(UINT32*)(spriteram+offs) == 0 || *(UINT32*)(spriteram+offs) == -1) continue;

		tile_number = spriteram[offs+1];
		attributes  = spriteram[offs+3]; /* YBBX.CCCC */
		if(attributes & 0x40) tile_number |= 256;
		if(attributes & 0x20) tile_number |= 512;

		color = attributes & 0xf;
		sx =  xscroll - spriteram[offs+2];
		if(!(attributes & 0x80)) sx += 256;
		sy = -yscroll + spriteram[offs];
		if(attributes & 0x10) sy += 256;
		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-16) sx -= 512;
		if (sy > 512-16) sy -= 512;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN_TABLE,7);
	}
}

VIDEO_UPDATE( tnk3 )
{
	UINT8 *ram = snk_rambase - 0xd000;
	int attributes = ram[0xc800];
	/*
        X-------
        -X------    character bank (for text layer)
        --X-----
        ---X----    scrolly MSB (background)
        ----X---    scrolly MSB (sprites)
        -----X--
        ------X-    scrollx MSB (background)
        -------X    scrollx MSB (sprites)
    */

	/* to be moved to memmap */
	spriteram = &ram[0xd000];

	{
		int bg_scrollx = -ram[0xcc00] + 15;
		int bg_scrolly = -ram[0xcb00] + 8;
		if(attributes & 0x02) bg_scrollx += 256;
		if(attributes & 0x10) bg_scrolly += 256;
		tnk3_draw_background( machine, bitmap, cliprect, bg_scrollx, bg_scrolly, 64, 64, 0 );
	}

	{
		int sp_scrollx = ram[0xca00] + 29;
		int sp_scrolly = ram[0xc900] + 9;
		if(attributes & 0x01) sp_scrollx += 256;
		if(attributes & 0x08) sp_scrolly += 256;
		tnk3_draw_sprites( machine, bitmap, cliprect, sp_scrollx, sp_scrolly );
	}

	{
		int bank = (attributes & 0x40) ? 1:0;

		tnk3_draw_text( machine, bitmap, cliprect, bank, &ram[0xf800] );
		tnk3_draw_status( machine, bitmap, cliprect, bank, &ram[0xfc00] );
	}
	return 0;
}

/************************************************************************************/

VIDEO_START( sgladiat )
{
	dirtybuffer = auto_malloc( MAX_VRAM_SIZE );
	tmpbitmap = auto_bitmap_alloc( 512, 256, machine->screen[0].format );
	memset( dirtybuffer, 0xff, MAX_VRAM_SIZE );
}

static void sgladiat_draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly )
{
	const gfx_element *gfx = machine->gfx[1];

	int tile_number, color, sx, sy;
	int offs, x, y;

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++)
	{
		offs = (x<<5)+y;
		tile_number = videoram[offs];

		if(tile_number != dirtybuffer[offs])
		{
			dirtybuffer[offs] = tile_number;

			color = 0;
			sx = x << 3;
			sy = y << 3;

			drawgfx(tmpbitmap,gfx,tile_number,color,0,0,sx,sy,0,TRANSPARENCY_NONE,0);
		}
	}
	copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,1,&scrolly,cliprect,TRANSPARENCY_NONE,0);
}

VIDEO_UPDATE( sgladiat )
{
	UINT8 *pMem = snk_rambase - 0xd000;
	int attributes, scrollx, scrolly;

	attributes = pMem[0xd300];

	scrollx = -pMem[0xd700] + ((attributes & 2) ? 256:0);
	scrolly = -pMem[0xd600];
	scrollx += 15;
	scrolly += 8;
	sgladiat_draw_background( machine, bitmap, cliprect, scrollx, scrolly );

	scrollx = pMem[0xd500] + ((attributes & 1) ? 256:0);
	scrolly = pMem[0xd400];
	scrollx += 29;
	scrolly += 9;
	tnk3_draw_sprites( machine, bitmap, cliprect, scrollx, scrolly );

	tnk3_draw_text( machine, bitmap, cliprect, 0, &pMem[0xf000] );
	return 0;
}

/**************************************************************************************/

static void ikari_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int start, int xscroll, int yscroll,
				UINT8 *source, int mode )
{
	gfx_element *gfx = machine->gfx[mode];
	int tile_number, attributes, color, sx, sy;
	int which, finish;

	finish = (start+25)*4;

	for(which = start*4; which < finish; which+=4)
	{
		if(*(UINT32*)(source+which) == 0 || *(UINT32*)(source+which) == -1) continue;

		tile_number = source[which+1];
		attributes  = source[which+3];
		switch(mode)
		{
			case 2:
				tile_number |= (attributes & 0x60) << 3;
				break;
			case 3:
				if(attributes & 0x40) tile_number |= 256;
				break;
		}
		color = attributes & 0xf;
		sx =  xscroll - source[which+2];
		if(!(attributes & 0x80)) sx += 256;
		sy = -yscroll + source[which];
		if(attributes & 0x10) sy += 256;
		sx = (sx - 16) & 0x1ff;
		sy = (sy - 16) & 0x1ff;
		if (sx > 512-16) sx -= 512;
		if (sy > 512-16) sy -= 512;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN_TABLE,7);
	}
}

VIDEO_UPDATE( ikari )
{
	UINT8 *ram = snk_rambase - 0xd000;

	{
		int attributes = ram[0xc900];
		int scrolly =  8-ram[0xc800] - ((attributes & 0x01) ? 256:0);
		int scrollx = 13-ram[0xc880] - ((attributes & 0x02) ? 256:0);
		tnk3_draw_background( machine, bitmap, cliprect, scrollx, scrolly, 32, 32, 1 );
	}

	{
		int attributes = ram[0xcd00];

		int sp16_scrolly = -7 + ram[0xca00] + ((attributes & 0x04) ? 256:0);
		int sp16_scrollx = 44 + ram[0xca80] + ((attributes & 0x10) ? 256:0);

		int sp32_scrolly =  9 + ram[0xcb00] + ((attributes & 0x08) ? 256:0);
		int sp32_scrollx = 28 + ram[0xcb80] + ((attributes & 0x20) ? 256:0);

		ikari_draw_sprites( machine, bitmap, cliprect,  0, sp16_scrollx, sp16_scrolly, &ram[0xe800], 2 );
		ikari_draw_sprites( machine, bitmap, cliprect,  0, sp32_scrollx, sp32_scrolly, &ram[0xe000], 3 );
		ikari_draw_sprites( machine, bitmap, cliprect, 25, sp16_scrollx, sp16_scrolly, &ram[0xe800], 2 );
	}

	tnk3_draw_text( machine, bitmap, cliprect, -1, &ram[0xf800] );
	tnk3_draw_status( machine, bitmap, cliprect, -1, &ram[0xfc00] );
	return 0;
}

/**************************************************************/

static void tdfever_draw_bg(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int xscroll, int yscroll )
{
	const UINT8 *source = snk_rambase + 0x000;
	const gfx_element *gfx = machine->gfx[1];

	int tile_number, attributes, color, sx, sy;
	int offs, x, y;

	for(x = 0; x < 32; x++) for(y = 0; y < 32; y++)
	{
		offs = (x<<6)+(y<<1);
		tile_number = source[offs];
		attributes  = source[offs+1];

		if(tile_number != dirtybuffer[offs] || attributes != dirtybuffer[offs+1])
		{
			dirtybuffer[offs]   = tile_number;
			dirtybuffer[offs+1] = attributes;
			tile_number |= (attributes & 0xf) << 8;

			color = attributes >> 4;
			sx = x << 4;
			sy = y << 4;

			// intercept overflown tile indices
			if(tile_number >= gfx->total_elements)
				plot_box(tmpbitmap, sx, sy, gfx->width, gfx->height, get_black_pen(machine));
			else
				drawgfx(tmpbitmap,gfx,tile_number,color,0,0,sx,sy,0,TRANSPARENCY_NONE,0);
		}
	}
	copyscrollbitmap(bitmap,tmpbitmap,1,&xscroll,1,&yscroll,cliprect,TRANSPARENCY_NONE,0);
}

/*
Sprite Format
-------------
byte0: y offset
byte1: tile number
byte2: x offset
byte3: attributes

    mode 0/1 attributes:

    76543210
    ----xxxx (color)
    ---x---- (y offset bit8)
    -xx----- (bank number)
    x------- (x offset bit8)

    mode 2 attributes:

    76543210
    -----xxx (color)
    ---x---- (y offset bit8)
    -xx-x--- (bank number)
    x------- (x offset bit8)
*/
static void tdfever_draw_sp(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int xscroll, int yscroll, int mode )
{
	const UINT8 *source = snk_rambase + ((mode==2)?0x1800:0x1000);
	const gfx_element *gfx = machine->gfx[(mode==1)?3:2];
	int tile_number, attributes, sx, sy, color, pen_mode;
	int which, finish, sp_size;

	if(mode < 0 || mode > 2) return;

	pen_mode = (snk_gamegroup & 1) ? TRANSPARENCY_PEN_TABLE : TRANSPARENCY_PEN;

	if(mode == 2)
	{
		finish  = 64 * 4;
		sp_size = 16;
	}
	else
	{
		finish  = 32 * 4;
		sp_size = 32;
	}

	for(which = 0; which < finish; which+=4)
	{
		if(*(UINT32*)(source+which) == 0 || *(UINT32*)(source+which) == -1) continue;

		tile_number = source[which+1];
		attributes  = source[which+3];

		sx = xscroll + source[which+2]; if(mode==0) sx = 256-sx;
		sy = yscroll + source[which];
		sx += attributes<<1 & 0x100;
		sy += attributes<<4 & 0x100;
		sx &= 0x1ff; if(sx > 512-sp_size) sx -= 512;
		sy &= 0x1ff; if(sy > 512-sp_size) sy -= 512;

		switch(mode)
		{
			case 2:
				tile_number |= (attributes<<4 & 0x600) | (attributes<<5 & 0x100);
				color = attributes & 0x07;
			break;

			default:
				tile_number |= attributes<<3 & 0x300;
				color = attributes & 0x0f;
				if (snk_gamegroup == 7) // ftsoccer
					palette_set_shadow_mode(machine, ((attributes & 0x6f) == 0x60) ? 1 : 0);
		}

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,pen_mode,15);
	}
}

static void tdfever_draw_tx(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int attributes, int dx, int dy, int base )
{
	const UINT8 *source = snk_rambase - 0xd000 + base;
	const gfx_element *gfx = machine->gfx[0];

	int tile_high = (attributes & 0xf0) << 4;
	int color = attributes & 0xf;
	int tile_number, sx, sy;
	int x, y;

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(tile_number == 0x20) continue;

		sx = dx + x*8;
		sy = dy + y*8;

		drawgfx(bitmap,gfx,tile_high|tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN,15);
	}
}

/**************************************************************/

VIDEO_UPDATE( tdfever )
{
	const UINT8 *ram = snk_rambase - 0xd000;
	int i;

	UINT8 bg_attributes = ram[0xc880];
	UINT8 sp_attributes = ram[0xc900];
	UINT8 tx_attributes = ram[0xc8c0];
	int bg_scroll_x = -ram[0xc840] + ((bg_attributes & 0x02) ? 256:0);
	int bg_scroll_y = -ram[0xc800] + ((bg_attributes & 0x01) ? 256:0);
	int sp_scroll_x = -ram[0xc9c0] + ((sp_attributes & 0x40) ? 0:256);
	int sp_scroll_y = -ram[0xc980] + ((sp_attributes & 0x80) ? 256:0);

	if(snk_gamegroup == 3 || snk_gamegroup == 5) // tdfever, tdfeverj
	{
			bg_scroll_x += 143;
			bg_scroll_y += -32;
			sp_scroll_x += 135;
			sp_scroll_y += -65;
	}
	else if(snk_gamegroup == 7) // ftsoccer
	{
			bg_scroll_x += 16;
			bg_scroll_y += 0;
			sp_scroll_x += 40;
			sp_scroll_y += -31;
	}
	tdfever_draw_bg( machine, bitmap, cliprect, bg_scroll_x, bg_scroll_y );

	if (snk_gamegroup == 5) // tdfeverj
	{
		gfx_drawmode_table[13] = DRAWMODE_SHADOW;
		gfx_drawmode_table[14] = DRAWMODE_SOURCE;

		for (i=0x10e; i<0x200; i+=0x10) palette_set_color(machine,i,MAKE_RGB(snk_blink_parity,snk_blink_parity,snk_blink_parity));
		snk_blink_parity ^= 0x7f;
	}
	tdfever_draw_sp( machine, bitmap, cliprect, sp_scroll_x, sp_scroll_y, 0 );

	tdfever_draw_tx( machine, bitmap, cliprect, tx_attributes, 0, 0, 0xf800 );
	return 0;
}

VIDEO_UPDATE( gwar )
{
	const UINT8 *ram = snk_rambase - 0xd000;
	int gwar_sp_baseaddr, gwar_tx_baseaddr;
	UINT8 bg_attribute;

	if(snk_gamegroup == 4) // gwara
	{
		gwar_sp_baseaddr = 0xf000;
		gwar_tx_baseaddr = 0xc800;
	}
	else
	{
		gwar_sp_baseaddr = 0xc000;
		gwar_tx_baseaddr = 0xf800;
	}

	bg_attribute = ram[gwar_sp_baseaddr+0x880];

	{
		int bg_scroll_y, bg_scroll_x;

		bg_scroll_x = -ram[gwar_sp_baseaddr+0x840] + 16;
		bg_scroll_y = -ram[gwar_sp_baseaddr+0x800];

		bg_scroll_x += (bg_attribute & 2) ? 256:0;
 		bg_scroll_y += (bg_attribute & 1) ? 256:0;

		tdfever_draw_bg( machine, bitmap, cliprect, bg_scroll_x, bg_scroll_y );
	}

	{
		UINT8 sp_attribute = ram[gwar_sp_baseaddr+0xac0];
		int sp16_x = -ram[gwar_sp_baseaddr+0x940] - 9;
		int sp16_y = -ram[gwar_sp_baseaddr+0x900] - 15;
		int sp32_x = -ram[gwar_sp_baseaddr+0x9c0] - 9;
		int sp32_y = -ram[gwar_sp_baseaddr+0x980] - 31;

		if(snk_gamegroup == 2) // gwar, gwarj, gwarb, choppera
		{
			sp16_y += (bg_attribute & 0x10) ? 256:0;
			sp16_x += (bg_attribute & 0x40) ? 256:0;
			sp32_y += (bg_attribute & 0x20) ? 256:0;
			sp32_x += (bg_attribute & 0x80) ? 256:0;
		}
		else
		{
			UINT8 spp_attribute = ram[gwar_sp_baseaddr+0xa80];
			sp16_x += (spp_attribute & 0x10) ? 256:0;
			sp16_y += (spp_attribute & 0x04) ? 256:0;
			sp32_x += (spp_attribute & 0x20) ? 256:0;
			sp32_y += (spp_attribute & 0x08) ? 256:0;
		}

		if(sp_attribute & 0xf8) // improves priority
		{
			tdfever_draw_sp( machine, bitmap, cliprect, sp16_x, sp16_y, 2 );
			tdfever_draw_sp( machine, bitmap, cliprect, sp32_x, sp32_y, 1 );
		}
		else
		{
			tdfever_draw_sp( machine, bitmap, cliprect, sp32_x, sp32_y, 1 );
			tdfever_draw_sp( machine, bitmap, cliprect, sp16_x, sp16_y, 2 );
		}
	}

	{
		UINT8 text_attribute = ram[gwar_sp_baseaddr+0x8c0];
		tdfever_draw_tx( machine, bitmap, cliprect, text_attribute, 0, 0, gwar_tx_baseaddr );
	}
	return 0;
}
