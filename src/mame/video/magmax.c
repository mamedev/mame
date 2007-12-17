/***************************************************************************

Video Hardware for MAGMAX.

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/11/05 -
Additional tweaking by Jarek Burczynski

***************************************************************************/

#include "driver.h"

UINT16 *magmax_scroll_x;
UINT16 *magmax_scroll_y;
UINT16 magmax_vreg;
static int flipscreen = 0;

static UINT32 pens_line_tab[256];
static UINT32 *prom_tab = NULL;


static void blit_horiz_pixel_line(mame_bitmap *b,int x,int y,int w, UINT32* pens)
{
	UINT16* lineadr = BITMAP_ADDR16(b, y, x);
	while(w-->0)
	{
		*lineadr++ = (UINT16)(*pens);
		pens++;
	}
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mag Max has three 256x4 palette PROMs (one per gun), connected to the
  RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( magmax )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn, offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup tables */

	/* characters use colors 0-15 */
	for (i = 0; i < TOTAL_COLORS(0);i++)
		COLOR(0, i) = i;

	/*sprites use colors 16-32, color 31 being transparent*/
	for (i = 0; i < TOTAL_COLORS(1);i++)
	{
		COLOR(1, i) = *(color_prom++) + 16;
	}

}

VIDEO_START( magmax )
{
	int i,v;
	UINT8 * prom14D = memory_region(REGION_USER2);

	/* Set up save state */
	state_save_register_global(magmax_vreg);

	prom_tab = auto_malloc(256 * sizeof(UINT32));

	/* Allocate temporary bitmap */
 	tmpbitmap = auto_bitmap_alloc(256,256,machine->screen[0].format);

	for (i=0; i<256; i++)
	{
		v = (prom14D[i] << 4) + prom14D[i + 0x100];
		prom_tab[i] = ((v&0x1f)<<8) | ((v&0x10)<<10) | ((v&0xe0)>>1); /*convert data into more useful format*/
	}
}



VIDEO_UPDATE( magmax )
{
	int offs;

	/* bit 2 flip screen */
	if (flipscreen != (magmax_vreg & 0x04))
	{
		flipscreen = magmax_vreg & 0x04;
	}

	/* copy the background graphics */
	if (magmax_vreg & 0x40)		/* background disable */
	{
		fillbitmap(bitmap, machine->pens[0], &machine->screen[0].visarea);
	}
	else
	{
		UINT32 h,v;
		UINT8 * rom18B = memory_region(REGION_USER1);
		UINT32 scroll_h = (*magmax_scroll_x) & 0x3fff;
		UINT32 scroll_v = (*magmax_scroll_y) & 0xff;

		/*clear background-over-sprites bitmap*/
		fillbitmap(tmpbitmap, 0, &machine->screen[0].visarea);

		for (v = 2*8; v < 30*8; v++) /*only for visible area*/
		{
			UINT32 map_v_scr_100 =   (scroll_v + v) & 0x100;
			UINT32 rom18D_addr   =  ((scroll_v + v) & 0xf8)     + (map_v_scr_100<<5);
			UINT32 rom15F_addr   = (((scroll_v + v) & 0x07)<<2) + (map_v_scr_100<<5);
			UINT32 map_v_scr_1fe_6 =((scroll_v + v) & 0x1fe)<<6;

			const pen_t *pens = &machine->pens[2*16 + (map_v_scr_100>>1)];

			if (!map_v_scr_100)
			{
				/* we are drawing surface */
				for (h = 0; h < 0x80; h++)
				{
					UINT32 graph_data;
					UINT32 graph_color;
					UINT32 LS283;
					UINT32 prom_data;

					LS283 =	scroll_h + h + rom18B[ map_v_scr_1fe_6 + h ] + 0xff01;

					prom_data = prom_tab[ (LS283 >> 6) & 0xff ];

					rom18D_addr &= 0x20f8;
					rom18D_addr += (prom_data & 0x1f00) + ((LS283 & 0x38) >>3);

					rom15F_addr &= 0x201c;
					rom15F_addr += (rom18B[0x4000 + rom18D_addr ]<<5) + ((LS283 & 0x6)>>1);
					rom15F_addr += (prom_data & 0x4000);

					graph_color = (prom_data & 0x0070);

					graph_data = rom18B[0x8000 + rom15F_addr];
					if ((LS283 & 1))
						graph_data >>= 4;
					graph_data &= 0x0f;

					pens_line_tab[h] = pens[graph_color + graph_data];

					/*priority: background over sprites*/
					/* not possible on the surface*/
					//if ((map_v_scr_100) && ((graph_data & 0x0c)==0x0c))
					//{
					//  *BITMAP_ADDR16(tmpbitmap, v, h) = pens[graph_color + graph_data];
					//}
				}
				for (h = 0x80; h < 0x100; h++)
				{
					UINT32 graph_data;
					UINT32 graph_color;
					UINT32 LS283;
					UINT32 prom_data;

					LS283 =	scroll_h + h + (rom18B[ (map_v_scr_1fe_6) + (h ^ 0xff) ] ^ 0xff);

					prom_data = prom_tab[ (LS283 >> 6) & 0xff ];

					rom18D_addr &= 0x20f8;
					rom18D_addr += (prom_data & 0x1f00) + ((LS283 & 0x38) >>3);

					rom15F_addr &= 0x201c;
					rom15F_addr += (rom18B[0x4000 + rom18D_addr ]<<5) + ((LS283 & 0x6)>>1);
					rom15F_addr += (prom_data & 0x4000);

					graph_color = (prom_data & 0x0070);

					graph_data = rom18B[0x8000 + rom15F_addr];
					if ((LS283 & 1))
						graph_data >>= 4;
					graph_data &= 0x0f;

					pens_line_tab[h] = pens[graph_color + graph_data];

					/*priority: background over sprites*/
					/* not possible on the surface*/
					//if ((map_v_scr_100) && ((graph_data & 0x0c)==0x0c))
					//{
					//  *BITMAP_ADDR16(tmpbitmap, v, h) = pens[graph_color + graph_data];
					//}
				}
			}
			else
			{
				/* we are drawing underground */
				for (h = 0; h < 0x80; h++)
				{
					UINT32 graph_data;
					UINT32 graph_color;
					UINT32 LS283;
					UINT32 prom_data;

					LS283 =	scroll_h + h;

					prom_data = prom_tab[ (LS283 >> 6) & 0xff ];

					rom18D_addr &= 0x20f8;
					rom18D_addr += (prom_data & 0x1f00) + ((LS283 & 0x38) >>3);

					rom15F_addr &= 0x201c;
					rom15F_addr += (rom18B[0x4000 + rom18D_addr ]<<5) + ((LS283 & 0x6)>>1);
					rom15F_addr += (prom_data & 0x4000);

					graph_color = (prom_data & 0x0070);

					graph_data = rom18B[0x8000 + rom15F_addr];
					if ((LS283 & 1))
						graph_data >>= 4;
					graph_data &= 0x0f;

					pens_line_tab[h] = pens[graph_color + graph_data];

					/*priority: background over sprites*/
					if (/*(map_v_scr_100) &&*/ ((graph_data & 0x0c)==0x0c))
					{
						*BITMAP_ADDR16(tmpbitmap, v, h) = pens[graph_color + graph_data];
					}
				}
				for (h = 0x80; h < 0x100; h++)
				{
					UINT32 graph_data;
					UINT32 graph_color;
					UINT32 LS283;
					UINT32 prom_data;

					LS283 =	scroll_h + h;

					prom_data = prom_tab[ (LS283 >> 6) & 0xff ];

					rom18D_addr &= 0x20f8;
					rom18D_addr += (prom_data & 0x1f00) + ((LS283 & 0x38) >>3);

					rom15F_addr &= 0x201c;
					rom15F_addr += (rom18B[0x4000 + rom18D_addr ]<<5) + ((LS283 & 0x6)>>1);
					rom15F_addr += (prom_data & 0x4000);

					graph_color = (prom_data & 0x0070);

					graph_data = rom18B[0x8000 + rom15F_addr];
					if ((LS283 & 1))
						graph_data >>= 4;
					graph_data &= 0x0f;

					pens_line_tab[h] = pens[graph_color + graph_data];

					/*priority: background over sprites*/
					if (/*(map_v_scr_100) &&*/ ((graph_data & 0x0c)==0x0c))
					{
						*BITMAP_ADDR16(tmpbitmap, v, h) = pens[graph_color + graph_data];
					}
				}
			}

			if (flipscreen)
			{
				int i;
				UINT32 pens_line_tab_flipped[256];
				for (i=0; i<256; i++)
					pens_line_tab_flipped[i] = pens_line_tab[255-i];
				blit_horiz_pixel_line(bitmap,0,255-v,256,pens_line_tab_flipped);
			}
			else
				blit_horiz_pixel_line(bitmap,0,    v,256,pens_line_tab);
		}
	}

	/* draw the sprites */
	for (offs = 0; offs < spriteram_size/2; offs += 4)
	{
		int sx, sy;

		sy = spriteram16[offs] & 0xff;
		if (sy)
		{
			int code = spriteram16[offs + 1] & 0xff;
			int attr = spriteram16[offs + 2] & 0xff;
			int color = (attr & 0xf0) >> 4;
			int flipx = attr & 0x04;
			int flipy = attr & 0x08;

			sx = (spriteram16[offs + 3] & 0xff) - 0x80 + 0x100 * (attr & 0x01);
			sy = 239 - sy;

			if (flipscreen)
			{
				sx = 255-16 - sx;
				sy = 239 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (code & 0x80)	/* sprite bankswitch */
			{
				code += (magmax_vreg & 0x30) * 0x8;
			}

			drawgfx(bitmap, machine->gfx[1],
					code,
					color,
					flipx, flipy,
					sx, sy,
					&machine->screen[0].visarea, TRANSPARENCY_COLOR, 31);
		}
	}
	if (!(magmax_vreg & 0x40))		/* background disable */
	{
		copybitmap(bitmap, tmpbitmap, flipscreen,flipscreen,0,0, &machine->screen[0].visarea, TRANSPARENCY_PEN, 0);
	}


	/* draw the foreground characters */
	for (offs = 32*32-1; offs >= 0; offs -= 1)
	{
		//int page = (magmax_vreg>>3) & 0x1;
		int code;

		code = videoram16[offs /*+ page*/] & 0xff;
		if (code)
		{
			int sx = (offs % 32);
			int sy = (offs / 32);

			if (flipscreen)
			{
				sx = 31 - sx;
				sy = 31 - sy;
			}

			drawgfx(bitmap, machine->gfx[0],
					code,
					0,
					flipscreen, flipscreen,
					8 * sx, 8 * sy,
					&machine->screen[0].visarea, TRANSPARENCY_PEN, 15);
		}
	}
	return 0;
}
