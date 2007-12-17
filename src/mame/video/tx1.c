/*====================================================================*/
/*               TX-1/Buggy Boy  (Tatsumi) Hardware                   */
/*                      Philip J Bennett 2005                         */
/*                                                                    */
/*                         Video Emulation                            */
/*====================================================================*/

#include "driver.h"

extern UINT16 *buggyb1_vram;
extern UINT16 *buggyboy_vram;
extern UINT16 *bb_objram;
extern UINT16 *bb_sky;

extern tilemap *buggyb1_tilemap;
extern tilemap *buggyboy_tilemap;
extern size_t bb_objectram_size;


extern UINT16 *tx1_vram;
extern UINT16 *tx1_object_ram;

extern tilemap *tx1_tilemap;
extern size_t tx1_objectram_size;

/*********/
/* TX-1 */
/********/

WRITE16_HANDLER( tx1_vram_w )
{
	COMBINE_DATA(&tx1_vram[offset]);
	tilemap_mark_tile_dirty(tx1_tilemap,offset);
}

static TILE_GET_INFO( get_tx1_tile_info )
{
	int tileno = (tx1_vram[tile_index]&0x03ff) | ((tx1_vram[tile_index] & 0x8000) >> 5);

	SET_TILE_INFO(0,tileno,0,0);
}

VIDEO_START( tx1 )
{
	tx1_tilemap = tilemap_create(get_tx1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,128,64);
	tilemap_set_transparent_pen(tx1_tilemap,0xff);
}

VIDEO_UPDATE( tx1 )
{
	/* the video hardware seems to use one large tilemap, scroll it to the right position for each screen */
	int xscrollamount = screen*256;
	tilemap_set_scrollx(tx1_tilemap,0,xscrollamount);


	tilemap_draw(bitmap,cliprect,tx1_tilemap,0,0);
	return 0;
}

/*************/
/* Buggy Boy */
/*************/

/***************************************************************************

  Convert the color PROMs into a more useable format.

  IC39, BB12 = Blue
  IC40, BB11 = Green
  IC41, BB10 = Red

  IC42, BB13 = Brightness

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1.0kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

  bit 0 -- 4.7kohm resistor  -- BLUE
  bit 1 -- 4.7kohm resistor  -- GREEN
  bit 2 -- 4.7kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( buggyboy )
{
        int i;

	for (i = 0; i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,bit4,r,g,b;

		bit0 = color_prom[i] & 1;
		bit1 = (color_prom[i] >> 1) & 1;
		bit2 = (color_prom[i] >> 2) & 1;
		bit3 = (color_prom[i] >> 3) & 1;
		bit4 = (color_prom[i+0x300] >> 2) & 1;
                r = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = color_prom[i+0x100] & 1;
		bit1 = (color_prom[i+0x100] >> 1) & 1;
		bit2 = (color_prom[i+0x100] >> 2) & 1;
		bit3 = (color_prom[i+0x100] >> 3) & 1;
		bit4 = (color_prom[i+0x300] >> 1) & 1;
                g = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = color_prom[i+0x200] & 1;
		bit1 = (color_prom[i+0x200] >> 1) & 1;
		bit2 = (color_prom[i+0x200] >> 2) & 1;
		bit3 = (color_prom[i+0x200] >> 3) & 1;
		bit4 = (color_prom[i+0x300]) & 1;
                b = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}


       /** Set up the colour lookups **/

       /* Objects use colours 0-63 */
       /* There are 2048 palette, however, this is expanded to 8192 */

	for (i = 0; i < 2048; i++)
		colortable[256+i] = ((0xf-color_prom[i + 0x500]) & 0xf) + 48;

	for (i = 0; i < 2048; i++)
	        colortable[256+2048+i] = ((0xf-color_prom[i + 0x500]) & 0xf) + 32;

	for (i = 0; i < 2048; i++)
        	colortable[256+2048+2048+i] = ((0xf-color_prom[i + 0x500]) & 0xf) + 16;

         /* Only this is used? */
	for (i = 0; i < 2048; i++)
		colortable[256+2048+2048+2048+i] = ((0xf-color_prom[i + 0x500]) & 0xf);

	/* Road uses 64-127 */
	/* Colour PROM only constitutes bits 0-3 - so expand 4-6*/
       	for (i = 0; i < 256; i++)
		colortable[256+8192+i] = (color_prom[i + 0x1500] & 0xf) + 64;

       	for (i = 0; i < 256; i++)
		colortable[256+8192+256+i] = (color_prom[i + 0x1500] & 0xf) + 64 + 16;

       	for (i = 0; i < 256; i++)
		colortable[256+8192+256*2+i] = (color_prom[i + 0x1500] & 0xf) + 64 + 32;

       	for (i = 0; i < 256; i++)
		colortable[256+8192+256*3+i] = (color_prom[i + 0x1500] & 0xf) + 64 + 48;

        /* Sky uses colours 128-191 directly - no lookup */

        /* Characters use colours 192-255 */
       	for (i = 0; i < 256; i++)
		colortable[i] = (color_prom[i + 0x400] & 0xf) + 192;

}


WRITE16_HANDLER( buggyb1_vram_w )
{
	COMBINE_DATA(&buggyb1_vram[offset]);
	tilemap_mark_tile_dirty(buggyb1_tilemap,offset);
}

WRITE16_HANDLER( buggyboy_vram_w )
{
	COMBINE_DATA(&buggyboy_vram[offset]);
	tilemap_mark_tile_dirty(buggyboy_tilemap,offset);
}


static TILE_GET_INFO( get_buggyb1_tile_info )
{
	int color, tileno;

	color = ((buggyb1_vram[tile_index] >>10) & 0x3f);
	tileno = (buggyb1_vram[tile_index]&0x03ff) | ((buggyb1_vram[tile_index] & 0x8000) >> 5);

	SET_TILE_INFO(0,tileno,color,0);
}

static TILE_GET_INFO( get_buggyboy_tile_info )
{
	int color, tileno;

	color = ((buggyboy_vram[tile_index] >>10) & 0x3f);
	tileno = (buggyboy_vram[tile_index]&0x03ff) | ((buggyboy_vram[tile_index] & 0x8000) >> 5);

	SET_TILE_INFO(0,tileno,color,0);
}


/*

 Applies to both versions of Buggy Boy

 Each object entry occupies 16 bytes:

 Byte 0: Sprite code
 Byte 1: Y-Position (bit 15 has some significance)

 Byte 2: Scale
 Byte 3: Scale

 Byte 4: Scale     0=Tiny  0x7f=Normal   0xff=Huge   ?
 Byte 5: Bit 7 = X-Flip, Bit 4 = chunk bank, Bit 5-6 = palette select, Bit 0-1 = palette related (PC_TMP)

 Byte 6: Scale
 Byte 7: Scale

 Byte 8: X position bits 0-7
 Byte 9: X position bits 8-9

 Remaining bytes are unusued.

*/

/* Rewrite once scale parameters etc. are discovered */
static void draw_objects(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int xdrawoffset)
{
	int offs;

	UINT8 PROM_lookup;
	UINT16 ROM_lookup;

	UINT8 *rom_lut  = (UINT8 *)memory_region(REGION_USER3);             /* Object index ROM */
	UINT8 *prom_lut = (UINT8 *)memory_region(REGION_PROMS)+0x1600;      /* Object index PROM */

	UINT8 *ROM_LUTA = (UINT8 *)memory_region(REGION_USER2);             /* Object LUT (lower byte) */
	UINT8 *ROM_LUTB = (UINT8 *)memory_region(REGION_USER2)+0x8000;      /* Object LUT (lower byte) */
	UINT8 *ROM_CLUT = (UINT8 *)memory_region(REGION_USER3)+0x2000;      /* Object palette LUT */

	for (offs = 0x0; offs <= (bb_objectram_size)/2; offs += 8)
	{
		int inc,last;
		int bit_12, PSA0_12, PSA, object_flip_x;
		int index_y,index_x,index=0;


		if((bb_objram[offs+0] >> 8) == 0xff)   /* End of object list marker? */
			return;

		/* The object code is fed into ROM and PROM to generate a lookup into a pair of ROMs */
		PROM_lookup = bb_objram[offs] & 0xff;
		ROM_lookup = ((bb_objram[offs] & 0xff) << 4)  | ((bb_objram[offs+1]>>11) & 0xf);

		if(rom_lut[ROM_lookup] == 0xff) /* Do not draw object  */
			continue;


		/* Calculate 13-bit index into object lookup ROMs that holds 8x8 chunk sequence */
		bit_12 = (((bb_objram[offs]>>7)&0x1) | ((bb_objram[offs]>>6)&0x1)) <<12;
		PSA0_12 = ( ( (prom_lut[PROM_lookup]&0xf)<<8) | rom_lut[ROM_lookup] | bit_12) & 0x1fff;

		PSA = (PSA0_12 << 2);

		object_flip_x = (bb_objram[offs+2]>>15)&0x1;

		for (index_y=0; index_y<16; index_y++)
		{
			if (object_flip_x)
			{
				index_x=16;
				inc=-1;
				last=0;
			}
			else
			{
				index_x=0;
				inc=1;
				last=16;
			}

			while(index_x!=last)
			{
				/* Bit 14 of chunk_number = data_end, related to end of line */
				int chunk_number = (ROM_LUTB[PSA+index]<<8) | ROM_LUTA[PSA+index];    // PSBB0-15

				int sx = (bb_objram[offs+4])+(index_x*8);
				int sy = (bb_objram[offs+0] >> 8)+(index_y*7);

				/* Calculate the 14-bit CLUT ROM address */
				int bit13 = (bb_objram[offs+2] & 0x1000) << 1;
				int bit12 = (chunk_number & 0x2000) >> 1;

				/* Tile Number bit 12 -> 1 = Bits 6-7 of BUG16s or bits 8-9 of PC_TMP */
				int bits6_and_7 = (chunk_number & 0x1000 ? chunk_number : bb_objram[offs+2] >> 2) & 0xc0;
				int CLUT_ROM_ADDR = (chunk_number & 0xf3f) + bits6_and_7 + bit12 + bit13;

				/* Now form the 12 bit OPCD */
				int bits10_and_11 = 0xc00 - (bb_objram[offs+2] & 0xc00);
				int OPCS = (bb_objram[offs+2] & 0x6000) >> 5;                     // bits 8 and 9
				int OPCD = (ROM_CLUT[CLUT_ROM_ADDR] + OPCS + bits10_and_11) & 0xfff;

				int tmp = (OPCD&0x7f);      // bits 0-6
				int tmp2= (OPCD&0x300)>>1;  // bits 9,8  (bit 7 is not there)
				int tmp3 = bits10_and_11 >> 1;

				int color = (tmp + tmp2 + tmp3);
				int trans;

				/* 8x8 chunk ROM bank (0-2) */
				int bank = (((bb_objram[offs+2]>>11)&0x2) | ((chunk_number>>13)&0x1))+1;
				int zoomx = 0xffff; //object_ram[offs+6]<<8;
				int zoomy = 0xffff; //object_ram[offs+4]<<8;
				int flipx = ((chunk_number>>15) & 0x1) ^ object_flip_x;
				int flipy = 0;

				const gfx_element *gfx = machine->gfx[bank];

				if(!(OPCD & 0x80))  /* Seems to work! */
					trans = TRANSPARENCY_PEN;
				else
					trans = TRANSPARENCY_NONE;

				index_x+=inc;
				index++;

				drawgfxzoom(bitmap, gfx,
					chunk_number,
					color,
					flipx,flipy,
					sx-xdrawoffset,sy,
					cliprect,trans,0,
					zoomx,zoomy);
                }
           }
    }
}


VIDEO_START( buggyb1 )
{
	buggyb1_tilemap = tilemap_create(get_buggyb1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
        tilemap_set_transparent_pen(buggyb1_tilemap, 0);
}


VIDEO_START( buggyboy )
{
	buggyboy_tilemap = tilemap_create(get_buggyboy_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,128,64);
        tilemap_set_transparent_pen(buggyboy_tilemap, 0);
}



/* Gradient sky - 'scrolls' up and down */
static void draw_sky(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int x,y,colour;
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
	        colour = (((*bb_sky & 0x7f) + y)>>2)&0x3f;
			*BITMAP_ADDR16(bitmap, y, x) = machine->pens[0x80 + colour];
		}
	}
}


/*
The current layer mixing implementation is incorrect.

On the actual PCB, the GAME OVER sign chains should be behind the text but the
objects are often in front of the characters.

See schematic page 11 for mixing logic.

*/


VIDEO_UPDATE( buggyb1 )
{
	if(*bb_sky & 0x80)
	{
		draw_sky(machine, bitmap, cliprect);
		tilemap_draw(bitmap,cliprect,buggyb1_tilemap,0,0);
		draw_objects(machine, bitmap,cliprect,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,buggyb1_tilemap,TILEMAP_DRAW_OPAQUE,0);
		draw_objects(machine, bitmap,cliprect,0);
	}
	return 0;
}

VIDEO_UPDATE( buggyboy )
{
	/* the video hardware seems to use one large tilemap, scroll it to the right position for each screen */
	int xscrollamount = screen*256;
	tilemap_set_scrollx(buggyboy_tilemap,0,xscrollamount);

	if(*bb_sky & 0x80)
	{
		draw_sky(machine, bitmap, cliprect);
		tilemap_draw(bitmap,cliprect,buggyboy_tilemap,0,0);
		draw_objects(machine, bitmap,cliprect,xscrollamount);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,buggyboy_tilemap,TILEMAP_DRAW_OPAQUE,0);
		draw_objects(machine, bitmap,cliprect,xscrollamount);
	}
	return 0;
}
