/***************************************************************************

 COSMIC.C

 emulation of video hardware of cosmic machines of 1979-1980(ish)

***************************************************************************/

#include "driver.h"


static pen_t (*map_color)(UINT8 x, UINT8 y);

static int color_registers[3];
static int background_enable;
static int magspot_pen_mask;



WRITE8_HANDLER( cosmic_color_register_w )
{
	color_registers[offset] = data ? 1 : 0;
}


static pen_t panic_map_color(UINT8 x, UINT8 y)
{
	offs_t offs;
	pen_t pen;


	offs = (color_registers[0] << 9) | (color_registers[2] << 10) | ((x >> 4) << 5) | (y >> 3);
	pen = memory_region(REGION_USER1)[offs];

	if (color_registers[1])
		pen >>= 4;

	return pen & 0x0f;
}

static pen_t cosmica_map_color(UINT8 x, UINT8 y)
{
	offs_t offs;
	pen_t pen;


	offs = (color_registers[0] << 9) | ((x >> 4) << 5) | (y >> 3);
	pen = memory_region(REGION_USER1)[offs];

	if (color_registers[0])		/* yes, 0 again according to the schematics */
		pen >>= 4;

	return pen & 0x07;
}

static pen_t cosmicg_map_color(UINT8 x, UINT8 y)
{
	offs_t offs;
	pen_t pen;


	offs = (color_registers[0] << 8) | (color_registers[1] << 9) | ((y >> 4) << 4) | (x >> 4);
	pen = memory_region(REGION_USER1)[offs];

	/* the upper 4 bits are for cocktail mode support */

	return pen & 0x0f;
}

static pen_t magspot2_map_color(UINT8 x, UINT8 y)
{
	offs_t offs;
	pen_t pen;


	offs = (color_registers[0] << 9) | ((x >> 3) << 4) | (y >> 4);
	pen = memory_region(REGION_USER1)[offs];

	if (color_registers[1])
		pen >>= 4;

	return pen & magspot_pen_mask;
}



/*
 * Panic Color table setup
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 * First 8 colors are normal intensities
 *
 * But, bit 3 can be used to pull Blue via a 2k resistor to 5v
 * (1k to ground) so second version of table has blue set to 2/3
 */

PALETTE_INIT( panic )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int r = 0xff * ((i >> 0) & 1);
		int g = 0xff * ((i >> 1) & 1);
		int b;
		if ((i & 0x0c) == 0x08)
			b = 0xaa;
		else
			b = 0xff * ((i >> 2) & 1);
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}


	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = *(color_prom++) & 0x07;


    map_color = panic_map_color;
}


/*
 * Cosmic Alien Color table setup
 *
 * 8 colors, 16 sprite color codes
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 */

PALETTE_INIT( cosmica )
{
	int i;

	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
		palette_set_color_rgb(machine,i,pal1bit(i >> 0),pal1bit(i >> 1),pal1bit(i >> 2));


	for (i = 0;i < TOTAL_COLORS(0)/2;i++)
	{
		COLOR(0,i)                     =  * color_prom          & 0x07;
		COLOR(0,i+(TOTAL_COLORS(0)/2)) = (*(color_prom++) >> 4) & 0x07;
	}


    map_color = cosmica_map_color;
}


/*
 * Cosmic guerilla table setup
 *
 * Use AA for normal, FF for Full Red
 * Bit 0 = R, bit 1 = G, bit 2 = B, bit 4 = High Red
 *
 * It's possible that the background is dark gray and not black, as the
 * resistor chain would never drop to zero, Anybody know ?
 */

PALETTE_INIT( cosmicg )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int r,g,b;

    	if (i > 8) r = 0xff;
        else r = 0xaa * ((i >> 0) & 1);

		g = 0xaa * ((i >> 1) & 1);
		b = 0xaa * ((i >> 2) & 1);
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}


    map_color = cosmicg_map_color;
}

PALETTE_INIT( magspot2 )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int r,g,b;

		if ((i & 0x09) == 0x08)
			r = 0xaa;
	 	else
			r = 0xff * ((i >> 0) & 1);

		g = 0xff * ((i >> 1) & 1);
		b = 0xff * ((i >> 2) & 1);
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}


	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		COLOR(0,i) = *(color_prom++) & 0x0f;
	}


    map_color = magspot2_map_color;
    magspot_pen_mask = 0x0f;
}


PALETTE_INIT( nomnlnd )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
		palette_set_color_rgb(machine,i,pal1bit(i >> 0),pal1bit(i >> 1),pal1bit(i >> 2));


	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		COLOR(0,i) = *(color_prom++) & 0x07;
	}


    map_color = magspot2_map_color;
    magspot_pen_mask = 0x07;
}


WRITE8_HANDLER( cosmic_background_enable_w )
{
	background_enable = data;
}


static void draw_bitmap(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	offs_t offs;


	for (offs = 0; offs < videoram_size; offs++)
	{
		UINT8 data = videoram[offs];

		if (data != 0)	/* optimization, not absolutely neccessary */
		{
			int i;
			UINT8 x = offs << 3;
			UINT8 y = offs >> 5;

			pen_t pen = machine->pens[map_color(x, y)];


			for (i = 0; i < 8; i++)
			{
				if (data & 0x80)
				{
					if (flip_screen)
						*BITMAP_ADDR16(bitmap, 255-y, 255-x) = pen;
					else
						*BITMAP_ADDR16(bitmap, y, x) = pen;
				}

				x++;
				data <<= 1;
			}
		}
	}
}


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int color_mask, int extra_sprites)
{
	int offs;


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		if (spriteram[offs] != 0)
        {
			int code, color;

			code  = ~spriteram[offs  ] & 0x3f;
			color = ~spriteram[offs+3] & color_mask;

			if (extra_sprites)
			{
				code |= (spriteram[offs+3] & 0x08) << 3;
			}

            if (spriteram[offs] & 0x80)
            {
                /* 16x16 sprite */

			    drawgfx(bitmap,machine->gfx[0],
					    code, color,
					    0, ~spriteram[offs] & 0x40,
				    	256-spriteram[offs+2],spriteram[offs+1],
				        cliprect,TRANSPARENCY_PEN,0);
            }
            else
            {
                /* 32x32 sprite */

			    drawgfx(bitmap,machine->gfx[1],
					    code >> 2, color,
					    0, ~spriteram[offs] & 0x40,
				    	256-spriteram[offs+2],spriteram[offs+1],
				        cliprect,TRANSPARENCY_PEN,0);
            }
        }
	}
}


static void cosmica_draw_starfield(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT8 y = 0;
	UINT8 map = 0;
	UINT8 *PROM = memory_region(REGION_USER2);


	while (1)
	{
		int va  =  y       & 0x01;
		int vb  = (y >> 1) & 0x01;


		UINT8 x = 0;

		while (1)
		{
			UINT8 x1;
			int hc, hb_;


			if (flip_screen)
				x1 = x - cpu_getcurrentframe();
			else
				x1 = x + cpu_getcurrentframe();


			hc  = (x1 >> 2) & 0x01;
			hb_ = (x  >> 5) & 0x01;  /* not a bug, this one is the real x */


			if ((x1 & 0x1f) == 0)
			{
				// flip-flop at IC11 is clocked
				map = PROM[(x1 >> 5) | (y >> 1 << 3)];
			}


			if ((!(hc & va) & (vb ^ hb_)) &&			/* right network */
			    (((x1 ^ map) & (hc | 0x1e)) == 0x1e))	/* left network */
			{
				/* RGB order is reversed -- bit 7=R, 6=G, 5=B */
				int col = (map >> 7) | ((map >> 5) & 0x02) | ((map >> 3) & 0x04);

				*BITMAP_ADDR16(bitmap, y, x) = machine->pens[col];
			}


			x++;
			if (x == 0)  break;
		}


		y++;
		if (y == 0)  break;
	}
}


static void devzone_draw_grid(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT8 y;
	UINT8 *horz_PROM = memory_region(REGION_USER2);
	UINT8 *vert_PROM = memory_region(REGION_USER3);
	offs_t horz_addr = 0;

	UINT8 count = 0;
	UINT8 horz_data = 0;
	UINT8 vert_data;


	for (y = 32; y < 224; y++)
	{
		UINT8 x = 0;


		while (1)
		{
			int x1;


			/* for the vertical lines, each bit indicates
               if there should be a line at the x position */
			vert_data = vert_PROM[x >> 3];


			/* the horizontal (perspective) lines are RLE encoded.
               When the screen is flipped, the address should be
               decrementing.  But since it's just a mirrored image,
               this is easier. */
			if (count == 0)
			{
				count = horz_PROM[horz_addr++];
			}

			count++;

			if (count == 0)
			{
				horz_data = horz_PROM[horz_addr++];
			}


			for (x1 = 0; x1 < 8; x1++)
			{
				if (!(vert_data & horz_data & 0x80))	/* NAND gate */
				{
					pen_t pen = machine->pens[4];	/* blue */

					if (flip_screen)
						*BITMAP_ADDR16(bitmap, 255-y, 255-x) = pen;
					else
						*BITMAP_ADDR16(bitmap, y, x) = pen;
				}

				horz_data = (horz_data << 1) | 0x01;
				vert_data = (vert_data << 1) | 0x01;

				x++;
			}


			if (x == 0)  break;
		}
	}
}


static void nomnlnd_draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT8 y = 0;
	UINT8 water = cpu_getcurrentframe();
	UINT8 *PROM = memory_region(REGION_USER2);


	/* all positioning is via logic gates:

       tree is displayed where

       __          __
       HD' ^ HC' ^ HB'

       and
        __          __              __
       (VB' ^ VC' ^ VD')  X  (VB' ^ VC' ^ VD')


       water is displayed where
             __         __
       HD' ^ HC' ^ HB ^ HA'

       and vertically the same equation as the trees,
       but final result inverted.


       The colors are coded in logic gates:

       trees:
                                P1 P2  BGR
         R = Plane1 ^ Plane2     0  0  000
         G = Plane2              0  1  010
         B = Plane1 ^ ~Plane2    1  0  100
                                 1  1  011

       water:
                                P1 P2  BGR or
         R = Plane1 ^ Plane2     0  0  100 000
         G = Plane2 v Plane2     0  1  110 010
         B = ~Plane1 or          1  0  010 010
             0 based oh HD       1  1  011 011

         Not sure about B, the logic seems convulated for such
         a simple result.

    */

	while (1)
	{
		int vb_ = (y >> 5) & 0x01;
		int vc_ = (y >> 6) & 0x01;
		int vd_ =  y >> 7;

		UINT8 x = 0;


		while (1)
		{
			int color = 0;

			int hd  = (x >> 3) & 0x01;
			int ha_ = (x >> 4) & 0x01;
			int hb_ = (x >> 5) & 0x01;
			int hc_ = (x >> 6) & 0x01;
			int hd_ =  x >> 7;


			if ((!vb_ & vc_ & !vd_) ^ (vb_ & !vc_ & vd_))
			{
				/* tree */
				if (!hd_ & hc_ & !hb_)
				{
					offs_t offs = ((x >> 3) & 0x03) | ((y & 0x1f) << 2) |
					              (flip_screen ? 0x80 : 0);

					UINT8 plane1 = PROM[offs         ] << (x & 0x07);
					UINT8 plane2 = PROM[offs | 0x0400] << (x & 0x07);

					plane1 >>= 7;
					plane2 >>= 7;

					color = (plane1 & plane2)       |	// R
					        (plane2 		)  << 1 |	// G
					        (plane1 & !plane2) << 2; 	// B
				}
			}
			else
			{
				/* water */
				if (hd_ & !hc_ & hb_ & !ha_)
				{
					offs_t offs = hd | (water << 1) | 0x0200;

					UINT8 plane1 = PROM[offs         ] << (x & 0x07);
					UINT8 plane2 = PROM[offs | 0x0400] << (x & 0x07);

					plane1 >>= 7;
					plane2 >>= 7;

					color = ( plane1 & plane2)      |	// R
					        ( plane1 | plane2) << 1 |	// G
					        (!plane1 & hd)     << 2; 	// B - see above
				}
			}


			if (color != 0)
			{
				pen_t pen = machine->pens[color];

				if (flip_screen)
					*BITMAP_ADDR16(bitmap, 255-y, 255-x) = pen;
				else
					*BITMAP_ADDR16(bitmap, y, x) = pen;
			}


			x++;
			if (x == 0)  break;
		}


		// this is obviously wrong
//      if (vb_)
//      {
			water++;
//      }


		y++;
		if (y == 0)  break;
	}
}


VIDEO_UPDATE( cosmicg )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

	draw_bitmap(machine, bitmap, cliprect);
	return 0;
}


VIDEO_UPDATE( panic )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

	draw_bitmap(machine, bitmap, cliprect);

	draw_sprites(machine, bitmap, cliprect, 0x07, 1);
	return 0;
}


VIDEO_UPDATE( cosmica )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

	cosmica_draw_starfield(machine, bitmap, cliprect);

	draw_bitmap(machine, bitmap, cliprect);

	draw_sprites(machine, bitmap, cliprect, 0x0f, 0);
	return 0;
}


VIDEO_UPDATE( magspot2 )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

	draw_bitmap(machine, bitmap, cliprect);

	draw_sprites(machine, bitmap, cliprect, 0x07, 0);
	return 0;
}


VIDEO_UPDATE( devzone )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

    if (background_enable)
    {
    	devzone_draw_grid(machine, bitmap, cliprect);
	}

	draw_bitmap(machine, bitmap, cliprect);

	draw_sprites(machine, bitmap, cliprect, 0x07, 0);
	return 0;
}


VIDEO_UPDATE( nomnlnd )
{
	/* according to the video summation logic on pg4, the trees and river
       have the highest priority */

	fillbitmap(bitmap, machine->pens[0], cliprect);

	draw_bitmap(machine, bitmap, cliprect);

	draw_sprites(machine, bitmap, cliprect, 0x07, 0);

    if (background_enable)
    {
    	nomnlnd_draw_background(machine, bitmap, cliprect);
	}
	return 0;
}
