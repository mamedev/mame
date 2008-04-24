/***************************************************************************

    Tatsumi TX-1/Buggy Boy video hardware

****************************************************************************/
#include "driver.h"
#include "profiler.h"
#include "render.h"
#include "cpu/i86/i86.h"
#include "tx1.h"

#define PRINT_CRTC_DATA 1

/*
    HD46505S-2 CRT Controller
*/
READ16_HANDLER( tx1_crtc_r )
{
	return 0xffff;
}

WRITE16_HANDLER( tx1_crtc_w )
{
#if PRINT_CRTC_DATA
	data &= 0xff;
	if (offset == 0)
	{
		switch (data)
		{
			case 0x00: mame_printf_debug("Horizontal Total         "); break;
			case 0x01: mame_printf_debug("Horizontal displayed     "); break;
			case 0x02: mame_printf_debug("Horizontal sync position "); break;
			case 0x03: mame_printf_debug("Horizontal sync width    "); break;
			case 0x04: mame_printf_debug("Vertical total           "); break;
			case 0x05: mame_printf_debug("Vertical total adjust    "); break;
			case 0x06: mame_printf_debug("Vertical displayed       "); break;
        	case 0x07: mame_printf_debug("Vertical sync position   "); break;
        	case 0x08: mame_printf_debug("Interlace mode           "); break;
        	case 0x09: mame_printf_debug("Max. scan line address   "); break;
        	case 0x0a: mame_printf_debug("Cursror start            "); break;
        	case 0x0b: mame_printf_debug("Cursor end               "); break;
        	case 0x0c: mame_printf_debug("Start address (h)        "); break;
        	case 0x0d: mame_printf_debug("Start address (l)        "); break;
        	case 0x0e: mame_printf_debug("Cursor (h)               "); break;
        	case 0x0f: mame_printf_debug("Cursor (l)               "); break;
        	case 0x10: mame_printf_debug("Light pen (h))           "); break;
        	case 0x11: mame_printf_debug("Light pen (l)            "); break;
		}
	}
	else if (offset == 1)
	{
		mame_printf_debug("0x%.2x, (%d)\n",data, data);
	}
#endif
}


/***************************************************************************

  TX-1

***************************************************************************/
static struct
{
	UINT16	scol;		/* Road colours */
	UINT32  slock;		/* Scroll lock */
	UINT8	flags;		/* Road flags */

	UINT32	ba_val;		/* Accumulator */
	UINT32	ba_inc;

	UINT16	h_val;		/* Accumulator */
	UINT16	h_inc;

	UINT8	slin_val;	/* Accumulator */
	UINT8	slin_inc;
} tx1_vregs;

/* Offsets into the palette PROMs */
#define TX1_COLORS_CHAR	0x00
#define TX1_COLORS_OBJ	0x80
#define TX1_COLORS_ROAD	0xC0

UINT16 *tx1_vram;
UINT16 *tx1_objram;
UINT16 *tx1_rcram;
size_t tx1_objram_size;

static tilemap *tx1_tilemap;
static bitmap_t *tx1_bitmap;
static render_texture *tx1_texture;

WRITE16_HANDLER( tx1_vram_w )
{
	COMBINE_DATA(&tx1_vram[offset]);
	tilemap_mark_tile_dirty(tx1_tilemap, offset);
}

static TILE_GET_INFO( get_tx1_tile_info )
{
	int tilenum, color;

	color = (tx1_vram[tile_index] >> 10) & 0x3f;
	tilenum = (tx1_vram[tile_index]&0x03ff) | ((tx1_vram[tile_index] & 0x8000) >> 5);

	SET_TILE_INFO(0, tilenum, color, 0);
}

/***************************************************************************

  Palette initialisation

  TODO: Add some notes

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1.0kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( tx1 )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* TODO */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x600] & 0x0f) | 0x00;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE16_HANDLER( tx1_bankcs_w )
{
	// AAB2 = /BASET0
	// AAB3 = /BASET
	// AAB4 = /BSET
	// AAB5 = /HASET
	// AAB6 = /HSET

	offset <<= 1;

	if ( offset & 0x04 )
	{
		tx1_vregs.ba_inc &= ~0x0000ffff;
		tx1_vregs.ba_inc |= data;

		if ( !(offset & 2)  )
			tx1_vregs.ba_val &= ~0x000ffff;
	}
	if ( offset & 0x08 )
	{
		data &= 0xff;
		tx1_vregs.ba_inc &= ~0xffff0000;
		tx1_vregs.ba_inc |= data << 16;

		if ( !(offset & 2)  )
			tx1_vregs.ba_val &= ~0xffff0000;
	}
	if ( offset & 0x10 )
	{
		/* Ignore data */
		if ( offset & 2 )
		{
			tx1_vregs.ba_val = (tx1_vregs.ba_inc + tx1_vregs.ba_val) & 0x00ffffff;
		}
	}
	if ( offset & 0x20 )
	{
		tx1_vregs.h_inc = data;

		if ( !(offset & 2) )
			tx1_vregs.h_val = 0;
	}
	if ( !(offset & 0x40) )
	{
		/* Ignore data? */
		if ( offset & 2 )
			tx1_vregs.h_val += tx1_vregs.h_inc;
	}
}

WRITE16_HANDLER( tx1_slincs_w )
{
	if ( offset == 1 )
		tx1_vregs.slin_inc = data;
	else
		tx1_vregs.slin_inc = tx1_vregs.slin_val = 0;
}

WRITE16_HANDLER( tx1_slock_w )
{
	tx1_vregs.slock = data & 1;
}

WRITE16_HANDLER( tx1_scolst_w )
{
	tx1_vregs.scol = data & 0x0707;
}

WRITE16_HANDLER( tx1_flgcs_w )
{
	tx1_vregs.flags = data & 0xff;
}


/* Preliminary */
static void tx1_draw_objects(bitmap_t *bitmap, const rectangle *cliprect)
{
#define FRAC	16

	UINT32 offs;

	/* The many lookup table ROMs */
	const UINT8 *const ic48  = (UINT8*)memory_region(REGION_USER3);
	const UINT8 *const ic281 = (UINT8*)memory_region(REGION_USER3) + 0x2000;
	const UINT8 *const ic25  = (UINT8*)memory_region(REGION_PROMS) + 0x1000;

	const UINT8 *const ic106 = (UINT8*)memory_region(REGION_USER2);
	const UINT8 *const ic73  = (UINT8*)memory_region(REGION_USER2) + 0x4000;

	const UINT8 *const ic190 = (UINT8*)memory_region(REGION_PROMS) + 0xc00;
	const UINT8 *const ic162 = (UINT8*)memory_region(REGION_PROMS) + 0xe00;

	const UINT8 *const pixdata_rgn = (UINT8*)memory_region(REGION_GFX2);

	for (offs = 0x0; offs <= tx1_objram_size; offs += 8)
	{
		UINT32	x;
		UINT32	y;
		UINT32	gxflip;

		UINT32	x_scale;
		UINT32	x_step;
		UINT16	y_scale;
		UINT16	y_step;

		UINT8	pctmp0_7;
		UINT8	code;

		/* Check for end of object list */
		if ( (tx1_objram[offs] & 0xff00) == 0xff00 )
			break;

		/* X scale */
		x_scale = tx1_objram[offs + 2] & 0xff;

		/* TODO: Confirm against hardware? */
		if ( x_scale == 0 )
			continue;

		/* 16-bit y-scale accumulator */
		y_scale = tx1_objram[offs + 1];
		y_step  = tx1_objram[offs + 3];

		/* Object number */
		code = tx1_objram[offs] & 0xff;

		/* Attributes */
		pctmp0_7 = tx1_objram[offs + 2] >> 8;

		/* Global x-flip */
		gxflip = (pctmp0_7 & 0x80) >> 7;

		/* Add 1 to account for line buffering */
		y = (tx1_objram[offs] >> 8) + 1;

		for (; y <= cliprect->max_y; ++y)
		{
			UINT32	rom_addr2	= 0;
			UINT8	ic106_data	= 0;
			UINT8	ic73_data;

			/* Are we drawing on this line? */

			/* TODO: See big lampposts. */
			if ( y_scale & 0x8000 )
				break;

			{
				UINT32	psa0_11;
				UINT32	ic48_addr;
				UINT32	ic48_data;
				UINT32	rom_addr;
				UINT32	x_acc;
				UINT32	newtile = 1;
				UINT32	dataend = 0;
				UINT8	data1 = 0;
				UINT8	data2 = 0;
				UINT32	xflip = 0;
				UINT32	opcd0_7 = 0;
				UINT32	lasttile = 0;

				/* Use the object code to lookup the tile sequence data in ROM */
				ic48_addr = code << 4;
				ic48_addr |= ((y_scale >> 11) & 0xf);
				ic48_data = ic48[ic48_addr];

				/* Reached the bottom of the object? (/PASS2E) */
				if ( ic48_data == 0xff )
					break;

				/* Combine ROM and PROM data */
				psa0_11 = ((ic25[code] << 8) | ic48_data) & 0xfff;

				/* psa8_11 */
				rom_addr = (psa0_11 & ~0xff) << 2;

				/* Prepare the x-scaling */
				x_step = (128 << FRAC) / x_scale;
				x_acc = (psa0_11 & 0xff) << (FRAC + 5);

#define TX1_MASK	0xfff

				x = tx1_objram[offs + 4] & TX1_MASK;

				for (;;)
				{
					if (newtile)
					{
						UINT32	psbb0_12;
						UINT32	pscb0_14;
						UINT32	pscb11;
						UINT8	*romptr;
						UINT32	ic281_addr;
						UINT32  grom_addr;
						UINT32	lut_data;
						UINT32	low_addr = ((x_acc >> (FRAC + 3)) & TX1_MASK);

						if (gxflip)
						{
							UINT32 xor_mask;

							if ( BIT(psa0_11, 11) && BIT(psa0_11, 10) )
								xor_mask = 0xf;
							else if ( BIT(psa0_11, 11) || BIT(psa0_11, 10) || BIT(psa0_11, 9) )
								xor_mask = 0x7;
							else
								xor_mask = 0x3;

							rom_addr2 = rom_addr + (xor_mask ^ low_addr);
						}
						else
							rom_addr2 = rom_addr + low_addr;

						ic106_data = ic106[rom_addr2 & 0x3fff];

						if ( (ic106_data & 0x40) && dataend )
							lasttile = 1;

						dataend |= ic106_data & 0x40;

						/* Retrieve data for an 8x8 tile */
						ic73_data = ic73[rom_addr2];

						// This is the data from the LUT pair
						lut_data = (ic106_data << 8) | ic73_data;
						psbb0_12 = lut_data & 0x1fff;

						// 0000 1100 0011 1111
						pscb0_14 = (psbb0_12 & 0xc3f);

						/* Bits 9_6 are from PCTMP11-8 or PSBB9-6 */
						if ( BIT(psbb0_12, 12) )
							pscb0_14 |= psbb0_12 & 0x3c0;
						else
							pscb0_14 |= (pctmp0_7 & 0xf) << 6;

						if ( BIT(lut_data, 13) )
							pscb0_14 |= BIT(psbb0_12, 10) << 12;	// NOT USED
						else
							pscb0_14 |= ((pctmp0_7 & 0x70) << 8);

						/* Bit 12 is Bit 10 duplicated. */
						pscb0_14 &= ~(1 << 12);
						pscb0_14 |= BIT(psbb0_12, 10) << 12;

						pscb11 = BIT(pscb0_14, 11);

						/* TODO: Remove this - it's constant. */
						romptr = (UINT8*)(pixdata_rgn + pscb11 * (0x4000 * 2));

						grom_addr = ((pscb0_14 << 3) | ((y_scale >> 8) & 7)) & 0x3fff;

						/* Get raw 8x8 2bpp pixel row data */
						data1 = *(grom_addr + romptr);
						data2 = *(grom_addr + romptr + 0x4000);

						/* Determine flip state (global XOR local) */
						xflip = gxflip ^ !BIT(lut_data, 15);

						ic281_addr = pscb0_14 & 0x3ff;
						ic281_addr |= ((pscb0_14 & 0x7000) >> 2);
						ic281_addr |= pscb11 << 13;

						opcd0_7 = ic281[ic281_addr];

						newtile = 0;
					}

					/* Draw a pixel? */
					if ( x <= cliprect->max_x )
					{
						UINT8	pix;
						UINT8	bit;

						bit	= (x_acc >> FRAC) & 7;

						if ( xflip )
							bit ^= 7;

						pix = (((data1 >> bit) & 1) << 1) | ((data2 >> bit) & 1);

						/* Draw pixel, if not transparent */
						if ( !(!(opcd0_7 & 0x80) && !pix) )
						{
							UINT8 color;
							UINT32 prom_addr;

							prom_addr = ((opcd0_7 << 2) | pix) & 0x1ff;

							/* Inverted on schematic */
							if (x & 1)
								color = ~ic190[prom_addr] & 0x3f;
							else
								color = ~ic162[prom_addr] & 0x3f;

							*BITMAP_ADDR16(bitmap, y, x) = TX1_COLORS_OBJ + color;
						}
					}

					/* Check if we've stepped into a new 8x8 tile */
					/* TODO */
					if ( (((x_acc + x_step) >> (FRAC + 3)) & TX1_MASK) != ((x_acc >> (FRAC + 3)) & TX1_MASK) )
					{
						if (lasttile)
							break;

						newtile = 1;
					}

  					x = (x + 1) & TX1_MASK;
					x_acc += x_step;
				}
			}// if (yscale)
			y_scale += y_step;
		} /* for (y) */
	}/* for (offs) */
}


VIDEO_START( tx1 )
{
	tx1_tilemap = tilemap_create(get_tx1_tile_info, tilemap_scan_rows,  8, 8, 128, 64);
	tilemap_set_transparent_pen(tx1_tilemap, 0xff);

	/* Allocate a large bitmap that covers the three screens */
	tx1_bitmap = auto_bitmap_alloc(768, 256, BITMAP_FORMAT_INDEXED16);
	tx1_texture = render_texture_alloc(NULL, NULL);
}

VIDEO_EOF( tx1 )
{
	/* /VSYNC: Update TZ113 */
	tx1_vregs.slin_val += tx1_vregs.slin_inc;
}


/* Experimental :) */
VIDEO_UPDATE( tx1 )
{
	int y;

	const device_config *left_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "left");
	const device_config *center_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "center");
	const device_config *right_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "right");

	if ( screen == left_screen )
	{
		rectangle rect = { 0, 768 - 1, 0, 240 - 1 };

//      tilemap_set_scrollx(tx1_tilemap, 0, scroll);
		tilemap_draw(tx1_bitmap, &rect, tx1_tilemap, 0, 0);
//      tx1_draw_road(tx1_bitmap, &rect);
		tx1_draw_objects(tx1_bitmap, &rect);

		for (y = 0; y < 240; ++y)
			memcpy(BITMAP_ADDR16(bitmap, y, 0), BITMAP_ADDR16(tx1_bitmap, y, 0 * 256), sizeof(UINT16) * 256);
	}
	else if ( screen == center_screen )
		for (y = 0; y < 240; ++y)
			memcpy(BITMAP_ADDR16(bitmap, y, 0), BITMAP_ADDR16(tx1_bitmap, y, 1 * 256), sizeof(UINT16) * 256);
	else if ( screen == right_screen )
		for (y = 0; y < 240; ++y)
			memcpy(BITMAP_ADDR16(bitmap, y, 0), BITMAP_ADDR16(tx1_bitmap, y, 2 * 256), sizeof(UINT16) * 256);


	return 0;
}


/***************************************************************************

  Buggy Boy

***************************************************************************/

/* Road register bits */
#define BB_RDFLAG_WAVE1		7
#define BB_RDFLAG_WAVE0		6
#define BB_RDFLAG_TNLMD1	5
#define BB_RDFLAG_TNLMD0	4
#define BB_RDFLAG_TNLF		3
#define BB_RDFLAG_LINF		2
#define BB_RDFLAG_RVA7		1
#define BB_RDFLAG_WANGL		0

/* Video registers */
static struct
{
	UINT32	ba_val;
	UINT32	ba_inc;

	UINT32	bank_mode;

	UINT16	h_val;
	UINT16	h_inc;
	UINT16	h_init;

	UINT8	wa8;
	UINT8	wa4;

	UINT8	slin;
	UINT8	slin_inc;

	UINT16	wave_lfsr;
	UINT16	scol;
	UINT8	sky;
	UINT16	gas;
	UINT8	flags;
	UINT8	shift;
} vregs;


UINT16 *buggyboy_objram;
UINT16 *buggyboy_rcram;
UINT16 *buggyboy_vram;
UINT16 *buggybjr_vram;
size_t buggyboy_objram_size;
size_t buggyboy_rcram_size;

static tilemap *buggyboy_tilemap;

static UINT8 *chr_bmp;
static UINT8 *obj_bmp;
static UINT8 *rod_bmp;

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
PALETTE_INIT( buggybjr )
{
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3, bit4;
		int r, g, b;

		bit0 = BIT(color_prom[i + 0x000], 0);
		bit1 = BIT(color_prom[i + 0x000], 1);
		bit2 = BIT(color_prom[i + 0x000], 2);
		bit3 = BIT(color_prom[i + 0x000], 3);
		bit4 = BIT(color_prom[i + 0x300], 2);
		r = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = BIT(color_prom[i + 0x100], 0);
		bit1 = BIT(color_prom[i + 0x100], 1);
		bit2 = BIT(color_prom[i + 0x100], 2);
		bit3 = BIT(color_prom[i + 0x100], 3);
		bit4 = BIT(color_prom[i + 0x300], 1);
		g = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		bit4 = BIT(color_prom[i + 0x300], 0);
		b = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

PALETTE_INIT( buggyboy )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3, bit4;
		int r, g, b;

		bit0 = BIT(color_prom[i + 0x000], 0);
		bit1 = BIT(color_prom[i + 0x000], 1);
		bit2 = BIT(color_prom[i + 0x000], 2);
		bit3 = BIT(color_prom[i + 0x000], 3);
		bit4 = BIT(color_prom[i + 0x300], 2);
		r = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = BIT(color_prom[i + 0x100], 0);
		bit1 = BIT(color_prom[i + 0x100], 1);
		bit2 = BIT(color_prom[i + 0x100], 2);
		bit3 = BIT(color_prom[i + 0x100], 3);
		bit4 = BIT(color_prom[i + 0x300], 1);
		g = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		bit4 = BIT(color_prom[i + 0x300], 0);
		b = 0x06 * bit4 + 0x0d * bit0 + 0x1e * bit1 + 0x41 * bit2 + 0x8a * bit3;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x400;

	/* Characters use colours 192-255 */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = 0xc0 | (((i & 0xc0) >> 2)) | (color_prom[i] & 0x0f);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE16_HANDLER( buggyboy_vram_w )
{
	COMBINE_DATA(&buggyboy_vram[offset]);
	tilemap_mark_tile_dirty(buggyboy_tilemap, offset);
}

WRITE16_HANDLER( buggybjr_vram_w )
{
	COMBINE_DATA(&buggybjr_vram[offset]);
}

static TILE_GET_INFO( get_buggyboy_tile_info )
{
	int color, tilenum;

	color = (buggyboy_vram[tile_index] >> 10) & 0x3f;
	tilenum = (buggyboy_vram[tile_index]&0x03ff) | ((buggyboy_vram[tile_index] & 0x8000) >> 5);

	SET_TILE_INFO(0, tilenum, color, 0);
}

/***************************************************************************

  Buggy Boy Road Hardware

  A mega-hack of TX-1 but without the second road.

  There are two lists in road/common RAM (double buffered) starting at 0x800
  and 0xa00:

  0x1800 - 0x18ff:    Road line horizontal position word (128 entries).
  0x19e0 - 0x19ef:    Vertical positions (starting line, water, tunnels etc)
  0x19f0 - 0x19ff:    Horizontal positions (walls and tunnels)

  Three TZ1113 accumulators are used to vary:
  * Road camber (update per pixel)
  * Road vertical scale/position (update per scanline)
  * Road 'speed' (update per frame)

  Road flags register (0x24E0):
  7 : Water sparkle control 1
  6 : Water sparkle control 0 ('WAVE0,1')
  5 : Tunnel mode 1
  4 : Tunnel mode 0 ('TNLMD0,1')
  3 : Tunnel flag ('TNLF')
  2 : Starting Line flag ('LINF')
  1 : Road list select
  0 : Wall angle enable ('WANGL')

  Road PAL equations:

  http://philwip.mameworld.info/buggyboy/PAL14H4.149.htm
  http://philwip.mameworld.info/buggyboy/PAL14L4.151.htm
  http://philwip.mameworld.info/buggyboy/PAL16H2.3.htm
  http://philwip.mameworld.info/buggyboy/PAL16L8.4.htm
  http://philwip.mameworld.info/buggyboy/PAL16L8.150.htm

***************************************************************************/
static void buggybjr_draw_road(UINT8 *bitmap)
{
#define X_ADJUST 384
#define LOAD_HPOS_COUNTER( NUM )												\
	ram_val = buggyboy_rcram[(rva_offs + 0x1f8 + (2*NUM)) >> 1];				\
	rcrs10 = ram_val & 0xfc00 ? 0x0400 : 0x0000;								\
	hp = vregs.wa8 + ((BIT(ram_val, 15) << 11) | rcrs10 | (ram_val & 0x03ff));	\
	hp##NUM = hp & 0xff;														\
	hp >>= 8;																	\
	hps##NUM##0 = (BIT(hp, 0) || BIT(hp, 2)) && !BIT(hp, 3);					\
	hps##NUM##1 = (BIT(hp, 1) || BIT(hp, 2)) && !BIT(hp, 3);					\
	hps##NUM##2 = BIT(hp, 2);													\

/* Check carry out calc */
#define UPDATE_HPOS( NUM )				\
	if (hp##NUM##_en)					\
	{									\
		if ((hp##NUM & 0xff) == 0xff)	\
			hp##NUM##_cy = 1;			\
		else							\
			hp##NUM = hp##NUM + 1;		\
	}									\

	INT32 x;
	UINT32 y;
	UINT16 rva_offs;
	UINT32 tnlmd0;
	UINT32 tnlmd1;
	UINT32 linf;
	UINT32 tnlf;
	UINT32 wangl;
	UINT32 tcmd;
	UINT32 wave0;
	UINT32 wave1;
	UINT32 rva20_6;

	/* ROM/PROM lookup tables */
	const UINT8 *rcols = (UINT8*)(memory_region(REGION_PROMS) + 0x1500);
	const UINT8 *rom   = memory_region(REGION_GFX6);
	const UINT8 *prom0 = rom + 0x4000;
	const UINT8 *prom1 = rom + 0x4200;
	const UINT8 *prom2 = rom + 0x4400;
	const UINT8 *vprom = rom + 0x4600;

	/* Extract constant values */
	tcmd	 = ((vregs.scol & 0xc000) >> 12) | ((vregs.scol & 0x00c0) >> 6);
	tnlmd0   = BIT(vregs.flags, BB_RDFLAG_TNLMD0);
	tnlmd1   = BIT(vregs.flags, BB_RDFLAG_TNLMD1);
	linf     = BIT(vregs.flags, BB_RDFLAG_LINF);
	tnlf     = BIT(vregs.flags, BB_RDFLAG_TNLF);
	wangl    = BIT(vregs.flags, BB_RDFLAG_WANGL);
	wave0    = BIT(vregs.flags, BB_RDFLAG_WAVE0);
	wave1    = BIT(vregs.flags, BB_RDFLAG_WAVE1);
	rva_offs = BIT(vregs.flags, BB_RDFLAG_RVA7) ? 0x800 : 0xc00;

	profiler_mark(PROFILER_USER1);

	for (y = 0; y < 240; ++y)
	{
		UINT8	rva0_6;
		UINT8	ram_addr;
		UINT16	rcrdb0_15;
		UINT16	rcrs10;
		UINT16	ls161_156_a;
		UINT16	ls161;
		UINT8	sld;
		UINT32	rva8;
		UINT32	rm0, rm1;
		UINT32	rcmd;
		UINT32	bnkcs = 1;

		UINT32	x_offs;

		/* Vertical positions shift register */
		UINT32	ram_val;
		UINT32	hp;
		UINT32	vp1, vp2, vp3, vp4, vp5, vp6, vp7;

		/* PAL outputs */
		UINT32	ic4_o12;
		UINT32	ic4_o13;
		UINT32	ic149_o15;
		UINT32	ic151_o14;

		/* Horizontal positions */
		UINT32	hp0, hp1, hp2, hp3;
		UINT8	hps00, hps01, hps02;
		UINT8	hps10, hps11, hps12;
		UINT8	hps20, hps21, hps22;
		UINT8	hps30, hps31, hps32;

		/* Road pixel data planes */
		UINT8	rc0 = 0, rc1 = 0, rc2 = 0, rc3 = 0;

		/* Horizontal position counter carry out */
		UINT8	hp0_cy = 0, hp1_cy = 0, hp2_cy = 0, hp3_cy = 0;

		UINT8	*bmpaddr = bitmap + (y * 256);

		UINT32	bank_cnt;
		UINT8	roadpix;

		rva8 = (vregs.h_val & 0x8000) || !(vregs.shift & 0x80);

		/* Get RVA0_6 from TZ113 accumulator chain @ 122/123 */
		rva0_6 = (vregs.h_val >> 7) & 0x7f;

		/* For /WAVE bit logic later */
		rva20_6 = ((rva0_6 >> 3) & 0xe) | ((rva0_6 & 2) >> 1);

		/* RVA is inverted! */
		ram_addr = (~rva0_6 & 0x7f) << 1;

		/* Get the road RAM data for this line */
		rcrdb0_15 = buggyboy_rcram[(rva_offs + ram_addr) >> 1];

		/* If 15-10 == 000000, then 0 */
		rcrs10 = rcrdb0_15 & 0xfc00 ? 0x0400 : 0x0000;

		/* If 15-10 == 111111, then 1 */
		ls161_156_a = (rcrdb0_15 & 0xfc00) == 0xfc00 ? 0x800 : 0x0000;

		/* LS161 15-bit counter chain - loaded with RAM data (bar bits 10-13) */
		ls161 =  ((rcrdb0_15 & 0x8000) >> 1) | ls161_156_a | rcrs10 | (rcrdb0_15 & 0x03ff);

		/* SLD */
		sld = (vprom[rva0_6] + vregs.slin) & 0x38;

		/* Determine the x-offset */
		x_offs = ls161 & 7;
		ls161 &= ~7;

		/* Fill vertical position shift register with bits for this line */
		/* TODO; cheated slightly to shift stuff up one pixel*/
		vp1 = buggyboy_rcram[(rva_offs + 0x1e2) >> 1] >= y ? 0 : 1;
		vp2 = buggyboy_rcram[(rva_offs + 0x1e4) >> 1] >= y ? 0 : 1;
		vp3 = buggyboy_rcram[(rva_offs + 0x1e6) >> 1] >= y ? 0 : 1;
		vp4 = buggyboy_rcram[(rva_offs + 0x1e8) >> 1] >= y ? 0 : 1;
		vp5 = buggyboy_rcram[(rva_offs + 0x1ea) >> 1] >= y ? 0 : 1;
		vp6 = buggyboy_rcram[(rva_offs + 0x1ec) >> 1] >= y ? 0 : 1;
		vp7 = buggyboy_rcram[(rva_offs + 0x1ee) >> 1] >= y ? 0 : 1;

		/* Stuff */
		rm0 = vp7 ? BIT(vregs.scol, 4) : BIT(vregs.scol, 12);
		rm1 = vp7 ? BIT(vregs.scol, 5) : BIT(vregs.scol, 13);

		/* Wall/tunnel control */
		rcmd = (vp7 ? vregs.scol : vregs.scol >> 8) & 0xf;

		/* Load 'em up */
		LOAD_HPOS_COUNTER(0);
		LOAD_HPOS_COUNTER(1);
		LOAD_HPOS_COUNTER(2);
		LOAD_HPOS_COUNTER(3);

		/* Some PAL equations that we can evaluate outside of the x-loop */
		ic4_o12 = (!vp1 && !vp2 && !vp6) || (!vp1 && !vp2 && vp7) || (vp4 && !vp6) || (vp4 && vp7);
		ic4_o13 = (!vp1 && !vp2 && !vp5) || (!vp1 && !vp2 && vp7) || (vp3 && !vp5) || (vp3 && vp7);
		ic149_o15 = (!vp5 && !vp6) || vp7 || !linf;
		ic151_o14 = !BIT(sld, 3) || tnlmd0 || tnlmd1 || ic149_o15;

		/* Load the bank counter with accumulator bits 14-5 */
		bank_cnt = (vregs.ba_val >> 5) & 0x3ff;

		bnkcs = 1;

		for (x = -x_offs; x < 256; ++x)
		{
			UINT16	ls283_159;
			UINT32	ls283_159_co;
			UINT16	rha;
			UINT32	rom_flip;
			UINT32	rom_en;
			UINT32	pix;
			UINT32	hp0_en, hp1_en, hp2_en, hp3_en;
			UINT32	_rorevcs = 0;

			/* The many PALs */

			UINT32	ic149_o16;

			UINT32	ic4_o18;
			UINT32	ic3_o15;
			UINT32	ic150_o12 = 0;
			UINT32	ic150_o16;
			UINT32	ic150_o17;
			UINT32	ic150_o18;
			UINT32	ic150_o19;
			UINT32	ic151_o15;
			UINT32	ic151_o16;
			UINT32	ic151_o17;

			UINT32	rcsd0_3 = 0;

			UINT32	sld5 = BIT(sld, 5);
			UINT32	sld4 = BIT(sld, 4);
			UINT32	mux;

			UINT32	cprom_addr;

			UINT8	px0, px1, px2, px3;

			/* Counter Q10-7 are added to 384 */
			ls283_159_co = (ls283_159 = (ls161 & 0x780) + X_ADJUST) & 0x800;
			rom_flip = ls283_159 & 0x200 ? 0 : 1;
			rom_en = !(ls283_159 & 0x400) && !(ls283_159_co ^ (ls161 & 0x800));

			/* Strip pixel number */
			pix = (ls161 & 7) ^ 7;

			/* Horizotnal position counter enables - also used as PAL inputs */
			hp0_en = !(hp0_cy || hps02);
			hp1_en = !(hp1_cy || hps12);
			hp2_en = !(hp2_cy || hps22);
			hp3_en = !(hp3_cy || hps32);

			_rorevcs = !( (rom_en && rom_flip) || (!rom_en && (ls161 & 0x4000)) );

			/* Load in a new road gfx strip */
			if ( (ls161 & 7) == 0 )
			{
				UINT8 d0 = 0;
				UINT8 d1 = 0;

				/* TODO: ROM data is 0xff if not enabled. */
				if (rom_en)
				{
					UINT8  rom_data;
					UINT16 prom_addr;

					/* 6 bit road horizontal address */
					rha = (ls283_159 & 0x180) | (ls161 & 0x78);

					if (rom_flip)
						rha ^= 0x1f8;

					/* Get road chunk first */
					rom_data = rom[(1 << 13) | (rha << 4) | rva0_6];
					prom_addr = (rom_flip ? 0x80 : 0) | (rom_data & 0x7f);

					rc0 = prom0[prom_addr];
					rc1 = prom1[prom_addr];
					rc2 = prom2[prom_addr];

					/* Now get the dirt chunk */
					rom_data = rom[(rha << 4) | rva0_6];
					prom_addr = 0x100 | rom_data;

					d0 = prom0[prom_addr];
					d1 = prom1[prom_addr];
				}
				else
				{
					/*
                        TODO: When ROM is not enabled, data = 0xff
                        But does anybody care?
                    */
					rc0 = rc1 = rc2 = rc3 = 0;
				}

				/* The data is mixed by two TZ0314 PALs */
				if (BIT(sld, 4))
				{
					if (BIT(sld, 5))
						d1 = ~d1;

					rc3 = d0 & d1;

					if (rom_flip)
						rc3 = BITSWAP8(rc3, 0, 1, 2, 3, 4, 5, 6, 7);
				}
				else
					rc3 = 0;
			}

			/* NEW!!!! Road camber */
			if (vregs.bank_mode == 0)
			{
				if ( BIT(vregs.ba_val, 23) )
					bnkcs = 1;
				else if (vregs.ba_val & 0x007f8000)
					bnkcs = 0;
				else
					bnkcs = bank_cnt < 0x300;
			}
			else
			{
				if ( BIT(vregs.ba_val, 23) )
					bnkcs = 0;
				else if (vregs.ba_val & 0x007f8000)
					bnkcs = 1;
				else
					bnkcs = bank_cnt >= 0x300;
			}

			px0 = BIT(rc0, pix);
			px1 = BIT(rc1, pix);
			px2 = BIT(rc2, pix);
			px3 = BIT(rc3, pix);

			/*
                Uh oh...
            */
			if (vp2)
				ic4_o18 = (hps00 && hps01 && hp3_en && !hps30)		||
						  (!hp0_en && hps01 && hp3_en && !hps30)	||
						  (hps00 && hps01 && !hps31)				||
						  (!hp0_en && hps01 && !hps31)				||
						  vp7;
			else
				ic4_o18 = !vp1;


			if (tnlf)
				ic3_o15 = (vp4 && !vp6 && !hp2_en && hps21)		||
						  (vp4 && !vp6 && hps20 && hps21)		||
						  (vp1 && !vp4 && !tnlmd1 && !tnlmd0)	||
						  (vp1 && !vp3 && !tnlmd1 && !tnlmd0)	||
						  (hp1_en && !hps10 && vp3 && !vp5)		||
						  (!hps11 && vp3 && !vp5);
			else
				ic3_o15 = !ic4_o18;

			ic151_o17 = (_rorevcs && !tnlmd1 && tnlmd0)		||
						(!_rorevcs && tnlmd1 && !tnlmd0)	||
						(_rorevcs && ic4_o12)				||
						(!_rorevcs && ic4_o13);

			if (!ic3_o15)
				ic151_o15 = (px0 && (bnkcs && wangl))	||
							(px1 && (bnkcs && wangl))	||
							ic151_o17					||
							px2							||
							!tnlf;
			else
				ic151_o15 =	!tnlf;

			ic151_o16 = (px1 && !px0 && tnlmd1 && !tnlmd0)	||
						(px2 && tnlmd1 && tnlmd0)			||
						ic149_o15;

			mux = BIT(tcmd, 3) ? ic149_o15 : ic151_o16;

			ic150_o19 = (px2 && !rva8)	||
						!bnkcs			||
						!mux			||
						!ic151_o15;

			/* Don't calculate the pixel colour if not visible */
			if (ic150_o19)
			{
				ic149_o16 = (_rorevcs && !px2 && ic151_o15)									||
							(tnlf && vp5 && !vp7 && px2 && !tnlmd0 && !tnlmd1 && ic151_o15)	||
							(tnlf && vp6 && !vp7 && px2 && !tnlmd0 && !tnlmd1 && ic151_o15)	||
							(tnlf && !ic4_o18);

				ic150_o16 = (px2 && mux && rm1)			||
							(mux && rva8 && ic151_o15)	||
							(!px0 && mux)				||
							!ic151_o15;

				{
					UINT32 a = mux && ic151_o15;

					ic150_o17 = (a && !rm0 && px0)	||
								(a && !px1)			||
								(rva8 && a);

					ic150_o18 = (a && !px2) ||
								(rva8 && a);
				}

				if (ic151_o14)
					ic150_o12 = rva8 || !mux || !ic151_o15				||
								(px2 && px1 && px0 && rm1 && !rm0)		||
								(!px2 && px1 && px0 && !sld4 && rm0)	||
								(px2 && px0 && !sld5 && !rm1 && !rm0)	||
								(px2 && !px1 && px0 && !sld5 && !rm1)	||
								(px2 && px1 && px0 && !sld5 && !sld4)	||
								(px2 && px1 && px0 && !sld4 && rm1)		||
								(!px2 && !px3 && !rm0)					||
								(!px1 && !px3 && rm1)					||
								(!px2 && !px1 && !px3)					||
								(!px0 && !px3);
				else
					ic150_o12 = 0;

				if (vp6 || ic151_o16)
				{
					UINT32 ic150_i5 = BIT(tcmd, 3) ? ic149_o15 : ic151_o16;

					if ( !(ic151_o15 && ic150_i5) )
						cprom_addr = (tcmd & 0x7) | (ic151_o16 ? 0x08 : 0);
					else
						cprom_addr = rcmd;

					/* Inverted! */
					cprom_addr = ((~cprom_addr) & 0xf) << 4;
				}
				else
					cprom_addr = 0xf0;

				cprom_addr |= (ic149_o16 ? 0x8 : 0) |
							  (ic150_o18 ? 0x4 : 0) |
							  (ic150_o17 ? 0x2 : 0) |
							  (ic150_o16 ? 0x1 : 0);

				/* Lower four bits of colour output come from PROM BB7 @ 188 */
				rcsd0_3 = rcols[cprom_addr] & 0xf;

				{
					UINT32 lfsr = vregs.wave_lfsr;
					UINT32 wave =
								(wave0 ^ BIT(lfsr, 0))	&&
								(wave1 ^ BIT(lfsr, 3))	&&
								BIT(lfsr, 5)			&&
								!BIT(lfsr, 15)			&&
								BIT(lfsr, 11)			&&
								BIT(lfsr, 13)			&&
								(rva20_6 < ((lfsr >> 8) & 0xf));

					roadpix = 0x40 | (wave ? 0 : 0x20) | (ic150_o12 ? 0x10 : 0) | rcsd0_3;
				}
			}
			else
				roadpix = 0;


			/* Horizontal position counters */
			if (x >= 0)
			{
				*bmpaddr++ = roadpix;

				UPDATE_HPOS(0);
				UPDATE_HPOS(1);
				UPDATE_HPOS(2);
				UPDATE_HPOS(3);

				/* Update the LFSR */
				vregs.wave_lfsr = (vregs.wave_lfsr << 1) | (BIT(vregs.wave_lfsr, 6) ^ !BIT(vregs.wave_lfsr, 15));

				/* Increment the bank counter */
				bank_cnt = (bank_cnt + 1) & 0x7ff;
			}

			/* X pos */
			ls161 = (ls161 + 1) & 0x7fff;
		}

		/* WANGL active? Update the 8-bit counter */
		if ( wangl )
		{
			if ( BIT(vregs.flags, BB_RDFLAG_TNLMD0) )
				vregs.wa8 -= 1;
			else
				vregs.wa8 += 1;
		}

		/* No carry out - just increment */
		if ( vregs.wa4 != 0xf )
			vregs.wa4 += 1;
		else
		{
			/* Carry out; increment again on /TMG2S rise */
			if ( wangl )
			{
				if ( BIT(vregs.flags, BB_RDFLAG_TNLMD0) )
					vregs.wa8 -= 1;
				else
					vregs.wa8 += 1;
			}
			vregs.wa4 = 1;
		}

		/* Update accumulator */
		vregs.h_val += vregs.h_inc;

		/* Seems correct */
		{
			UINT8 sf = vregs.shift;

			if ((vregs.shift & 0x80) == 0)
			{
				vregs.shift <<= 1;

				if ((sf & 0x08) == 0)
					vregs.shift |= BIT(vregs.h_val, 15);
			}

			if ((sf & 0x08) && !(vregs.shift & 0x08))
				vregs.h_inc = vregs.gas;
		}

		/* Finally, increment the banking accumulator */
		vregs.ba_val = (vregs.ba_val + vregs.ba_inc) & 0x00ffffff;
		}

	profiler_mark(PROFILER_END);
}


/***************************************************************************

    Buggy Boy Object Drawing

    X-scaling isn't quite right but you wouldn't notice...

    -------- xxxxxxxx       Object number
    xxxxxxxx --------       Y position

    xxxxxxxx xxxxxxxx       Y scale value

    -------- xxxxxxxx       X scale
                             00 = Invisible?
                             80 = 1:1
                             FF = Double size
    xxxxxxxx --------       Attributes

    xxxxxxxx xxxxxxxx       Y scale delta

    ------xx xxxxxxxx       X position

**************************************************************************/
static void buggyboy_draw_objs(UINT8 *bitmap)
{
#define FRAC	16

	UINT32 offs;

	/* The many lookup table ROMs */
	const UINT8 *const bug13  = (UINT8*)memory_region(REGION_USER3);
	const UINT8 *const bug18s = (UINT8*)memory_region(REGION_USER3) + 0x2000;
	const UINT8 *const bb8    = (UINT8*)memory_region(REGION_PROMS) + 0x1600;

	const UINT8 *const bug16s = (UINT8*)memory_region(REGION_USER2);
	const UINT8 *const bug17s = (UINT8*)memory_region(REGION_USER2) + 0x8000;

	const UINT8 *const bb9o = (UINT8*)memory_region(REGION_PROMS) + 0x500;
	const UINT8 *const bb9e = (UINT8*)memory_region(REGION_PROMS) + 0xd00;

	const UINT8 *const pixdata_rgn = (UINT8*)memory_region(REGION_GFX2);

	profiler_mark(PROFILER_USER1);

	for (offs = 0; offs <= buggyboy_objram_size; offs += 8)
	{
		UINT32	x;
		UINT32	y;
		UINT32	gxflip;

		UINT32	x_scale;
		UINT32	x_step;
		UINT16	y_scale;
		UINT16	y_step;

		UINT8	pctmp0_7;
		UINT8	code;

		/* Check for end of object list */
		if ( (buggyboy_objram[offs] & 0xff00) == 0xff00 )
			break;

		/* X scale */
		x_scale = buggyboy_objram[offs + 2] & 0xff;

		/* TODO: Confirm against hardware? */
		if ( x_scale == 0 )
			continue;

		/* 16-bit y-scale accumulator */
		y_scale = buggyboy_objram[offs + 1];
		y_step  = buggyboy_objram[offs + 3];

		/* Object number */
		code = buggyboy_objram[offs] & 0xff;

		/* Attributes */
		pctmp0_7 = buggyboy_objram[offs + 2] >> 8;

		/* Global x-flip */
		gxflip = (pctmp0_7 & 0x80) >> 7;

		/* Add 1 to account for line buffering */
		y = (buggyboy_objram[offs] >> 8) + 1;

		if (code == 0xa8)
			code = 0xa8;

		for (; y < 240; ++y)
		{
			UINT32	rom_addr2	= 0;
			UINT8	bug17s_data	= 0;
			UINT8	bug16s_data;

			/* Are we drawing on this line? */

			// TODO: See big lampposts.
			if ( y_scale & 0x8000 )
				break;

			{
				UINT32	psa0_12;
				UINT32	bug13_addr;
				UINT32	bug13_data;
				UINT32	rom_addr;
				UINT32	x_acc;
				UINT32	newtile = 1;
				UINT32	dataend = 0;
				UINT8	data1 = 0;
				UINT8	data2 = 0;
				UINT32	xflip = 0;
				UINT32	opcd10_11;
				UINT32	opcd8_9;
				UINT32	opcd0_11 = 0;
				UINT32	lasttile = 0;

				/* Use the object code to lookup the tile sequence data */
				bug13_addr = code << 4;
				bug13_addr |= ((y_scale >> 11) & 0xf);
				bug13_data = bug13[bug13_addr];

				/* Reached the bottom of the object */
				if (bug13_data == 0xff)
					break;

				psa0_12  = (((code & 0x80) << 5) | ((code & 0x40) << 6)) & 0x1000;
				psa0_12 |= ((bb8[code] << 8) | bug13_data) & 0x1fff;

				/* Static part of the BUG17S/BUG16S ROM address */
				rom_addr = (psa0_12 & ~0xff) << 2;

				/* Prepare the x-scaling */
				x_step = (128 << FRAC) / x_scale;
				x_acc = (psa0_12 & 0xff) << (FRAC + 5);

				/* TODO Add note */
				x = buggyboy_objram[offs + 4] & 0x3ff;

				for (;;)
				{
					#define MASK	0x3ff

					/* Get data and attributes for an 8x8 tile */
					if (newtile)
					{
						UINT32	pscb0_11;
						UINT32	psbb0_15;
						UINT32	psbb6_7;
						UINT32	rombank;
						UINT8	*romptr;
						UINT32	bug18s_data;
						UINT32	low_addr = ((x_acc >> (FRAC + 3)) & MASK);

						/*
                            Objects are grouped by width (either 16, 8 or 4 tiles) in
                            the LUT ROMs. The ROM address lines therefore indicate
                            width and are used to determine the correct scan order
                            when x-flip is set.
                        */
						if (gxflip)
						{
							UINT32	xor_mask;

							if	( BIT(psa0_12, 11) || !BIT(psa0_12, 12) )
								xor_mask = 0xf;
							else if	( !BIT(psa0_12, 9) )
								xor_mask = 0x7;
							else
								xor_mask = 0x3;

							rom_addr2 = rom_addr + (low_addr ^ xor_mask);
						}
						else
							rom_addr2 = rom_addr + low_addr;

						bug17s_data = bug17s[rom_addr2 & 0x7fff];

						if ((bug17s_data & 0x40) && dataend)
							lasttile = 1;

						dataend |= (bug17s_data & 0x40);

						/* Retrieve data for an 8x8 tile */
						bug16s_data = bug16s[rom_addr2];
						psbb0_15 = (bug17s_data << 8) | bug16s_data;
						psbb6_7 = (BIT(psbb0_15, 12) ? psbb0_15 : (pctmp0_7 << 6)) & 0xc0;

						/* Form the tile ROM address */
						pscb0_11 = ((((psbb0_15 & ~0xc0) | psbb6_7) << 3) | ((y_scale >> 8) & 7)) & 0x7fff;

						/* Choose from one of three banks */
						rombank = ((BIT(pctmp0_7, 4) << 1) | BIT(psbb0_15, 13)) & 3;

						/* TODO: Remember to put all the data into one GFX region */
						romptr = (UINT8*)(pixdata_rgn + rombank * (0x8000 * 2));

						/* Get raw 8x8 pixel row data */
						data1 = *(pscb0_11 + romptr);
						data2 = *(pscb0_11 + romptr + 0x8000);

						/* Determine flip state (global XOR local) */
						xflip = gxflip ^ !BIT(psbb0_15, 15);

						bug18s_data = bug18s[ (BIT(pctmp0_7, 4)  << 13)	|
											  (BIT(psbb0_15, 13) << 12)	|
											  (psbb0_15 & ~0xf0c0)		|
											  psbb6_7 ];

						/* Get the colour data. Note that bits 11 and 10 are inverted */
						opcd10_11 = ((pctmp0_7 << 8) & 0xc00) ^ 0xc00;
						opcd8_9 = ((pctmp0_7 & 0x60) << 3);
						opcd0_11 = (opcd10_11 | opcd8_9 | bug18s_data) & 0xfff;

						newtile = 0;
					}

					/* Draw a pixel? */
					if (x < 256)
					{
						UINT8	pix;
						UINT8	bit;

						bit	= (x_acc >> FRAC) & 7;

						if (xflip)
							bit ^= 7;

						pix = (((data1 >> bit) & 1) << 1) | ((data2 >> bit) & 1);

						/* Write the pixel if not transparent */
						if ( !(!(opcd0_11 & 0x80) && !pix) )
						{
							UINT8 color;
							UINT32 bb9_addr;

							bb9_addr = ((opcd0_11 << 1) & 0x600) | ((opcd0_11 & 0x7f) << 2) | pix;
							color = ((opcd0_11 >> 6) & 0x30);

							/* Inverted on schematic */
							if (x & 1)
								color = ~(color | bb9o[bb9_addr]) & 0x3f;
							else
								color = ~(color | bb9e[bb9_addr]) & 0x3f;

							*(bitmap + 256*y + x) = 0x40 | color;
						}
					}

					/* Check if we've stepped into a new 8x8 tile */
					if ( (((x_acc + x_step) >> (FRAC + 3)) & MASK) != ((x_acc >> (FRAC + 3)) & MASK) )
					{
						if (lasttile)
							break;

						newtile = 1;
					}

  					x = (x + 1) & 0x3ff;
					x_acc += x_step;
				}
			}// if (yscale)
			y_scale += y_step;
		} /* for (y) */
	}/* for (offs) */

	profiler_mark(PROFILER_END);
}


/*
    2400-24FF is road control (R/W)

    /GAS = 24XX:
    /BASET0 = 2400-F, 2410-F
    /BASET1 = 2420-F, 2430-F
    /BSET   = 2440-F, 2450-F
    /HASET  = 2460-F, 2470-F
    /HSET   = 2480-F, 2490-F
    /WASET  = 24A0-F, 24B0-F
    /FLAGS  = 24E0-F, 24F0-F
*/
WRITE16_HANDLER( buggyboy_gas_w )
{
	offset <<= 1;

	switch (offset & 0xe0)
	{
		case 0x00:
		{
			vregs.ba_inc &= ~0x0000ffff;
			vregs.ba_inc |= data;

			if ( !(offset & 2)  )
				vregs.ba_val &= ~0x0000ffff;

			break;
		}
		case 0x20:
		{
			data &= 0xff;
			vregs.ba_inc &= ~0xffff0000;
			vregs.ba_inc |= data << 16;

			vregs.bank_mode = data & 1;

			if ( !(offset & 2)  )
				vregs.ba_val &= ~0xffff0000;

			break;
		}
		case 0x40:
		{
			/* Ignore data? */
			if ( offset & 2 )
			{
				vregs.ba_val = (vregs.ba_inc + vregs.ba_val) & 0x00ffffff;
			}

			break;
		}
		case 0x60:
		{
			vregs.h_inc = data;
			vregs.shift = 0;

			if ( !(offset & 2)  )
				vregs.h_val = 0;

			break;
		}
		case 0x80:
		{
			/* Ignore data? */
			if ( offset & 2 )
				vregs.h_val += vregs.h_inc;
			break;
		}
		case 0xa0:
		{
			vregs.wa8 = data >> 8;
			vregs.wa4 = 0;
			break;
		}
		case 0xe0:
		{
			cpunum_set_input_line(machine, 1, INPUT_LINE_TEST, CLEAR_LINE);
			vregs.flags = data;
			break;
		}
	}

	/* Value is latched by LS373 76/77 */
	vregs.gas = data;
}


WRITE16_HANDLER( buggyboy_sky_w )
{
	vregs.sky = data;
}

WRITE16_HANDLER( buggyboy_slincs_w )
{
	if ( offset == 1 )
		vregs.slin_inc = data;
	else
		vregs.slin_inc = vregs.slin = 0;
}

WRITE16_HANDLER( buggyboy_scolst_w )
{
	vregs.scol = data;
}



VIDEO_START( buggyboy )
{
	buggyboy_tilemap = tilemap_create(get_buggyboy_tile_info, tilemap_scan_rows,  8, 8, 128, 64);
	tilemap_set_transparent_pen(buggyboy_tilemap, 0);
}



VIDEO_UPDATE( buggyboy )
{
	int xscrollamount = 0;

	const device_config *left_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "left");
	const device_config *center_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "center");
	const device_config *right_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "right");

	/* The video hardware seems to use one large tilemap, scroll it to the right position for each screen */
	if ( screen == left_screen )
		xscrollamount = 0 * 256;
	else if ( screen == center_screen )
		xscrollamount = 1 * 256;
	else if ( screen == right_screen )
		xscrollamount = 2 * 256;

	tilemap_set_scrollx(buggyboy_tilemap, 0, xscrollamount);
	tilemap_draw(bitmap, cliprect, buggyboy_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	return 0;
}


VIDEO_EOF( buggyboy )
{
	/* /VSYNC: Update TZ113 @ 219 */
	vregs.slin += vregs.slin_inc;

	/* /VSYNC: Clear wave LFSR */
	vregs.wave_lfsr = 0;
}


VIDEO_START( buggybjr )
{
	/* Allocate some bitmaps */
	chr_bmp = auto_malloc(sizeof(UINT8) * 256 * 240);
	obj_bmp = auto_malloc(sizeof(UINT8) * 256 * 240);
	rod_bmp = auto_malloc(sizeof(UINT8) * 256 * 240);
}

/*
    Draw the tilemap with scrolling
*/
static void buggyboy_draw_char(UINT8 *bitmap)
{
	INT32 x, y;
	UINT32 scroll_x, scroll_y;
	UINT8 *gfx1, *gfx2;

	profiler_mark(PROFILER_USER3);

	/* 2bpp characters */
	gfx1 = memory_region(REGION_GFX1);
	gfx2 = memory_region(REGION_GFX1) + 0x4000;

	/* X/Y scroll values are the last word in char RAM */
	scroll_y = (buggybjr_vram[0x7ff] >> 10) & 0x3f;
	scroll_x = buggybjr_vram[0x7ff] & 0x1ff;

	for (y = 0; y < 240; ++y)
	{
		UINT32 d0 = 0, d1 = 0;
		UINT32 colour = 0;
		UINT32 y_offs;
		UINT32 x_offs;
		UINT32 y_gran;

		/* There's no y-scrolling between scanlines 0 and 1 */
		if (y < 64)
			y_offs = y;
		else
		{
			y_offs = (y + (scroll_y | 0xc0) + 1) & 0xff;

			/* Clamp */
			if (y_offs < 64)
				y_offs |= 0xc0;
		}

		if ( (y_offs >= 64) && (y_offs < 128) )
			x_offs = scroll_x;
		else
			x_offs = 0;


		y_gran = y_offs & 7;

		if (x_offs & 7)
		{
			UINT32 tilenum;
			UINT16 ram_val = buggybjr_vram[((y_offs << 3) & 0x7c0) + ((x_offs >> 3) & 0x3f)];

			tilenum = (ram_val & 0x03ff) | ((ram_val & 0x8000) >> 5);
			colour = (ram_val & 0xfc00) >> 8;
			d0 = *(gfx2 + (tilenum << 3) + y_gran);
			d1 = *(gfx1 + (tilenum << 3) + y_gran);
		}

		for (x = 0; x < 256; ++x)
		{
			UINT32 x_gran = x_offs & 7;

			if (!x_gran)
			{
				UINT32 tilenum;
				UINT16 ram_val = buggybjr_vram[((y_offs << 3) & 0x7c0) + ((x_offs >> 3) & 0x3f)];

				tilenum = (ram_val & 0x03ff) | ((ram_val & 0x8000) >> 5);
				colour = (ram_val & 0xfc00) >> 8;
				d0 = *(gfx2 + (tilenum << 3) + y_gran);
				d1 = *(gfx1 + (tilenum << 3) + y_gran);
			}

			*bitmap++ = colour |
						(((d1 >> (7 ^ x_gran)) & 1) << 1) |
						((d0 >> (7 ^ x_gran)) & 1);

			x_offs = (x_offs + 1) & 0x1ff;
		}

	}

	profiler_mark(PROFILER_END);
}

VIDEO_UPDATE( buggybjr )
{
	int x, y;
	UINT8 *chr_pal = (memory_region(REGION_PROMS) + 0x400);

	memset(obj_bmp, 0, 256*240);

	buggyboy_draw_char(chr_bmp);
	buggybjr_draw_road(rod_bmp);
	buggyboy_draw_objs(obj_bmp);

	for (y = 0; y < 240; ++y)
	{
		UINT16 *bmp_addr = BITMAP_ADDR16(bitmap, y, 0);

		UINT8 *chr_addr = chr_bmp + (y * 256);
		UINT8 *rod_addr = rod_bmp + (y * 256);
		UINT8 *obj_addr = obj_bmp + (y * 256);

		UINT32 sky_en = BIT(vregs.sky, 7);
		UINT32 sky_val = (((vregs.sky & 0x7f) + y) >> 2) & 0x3f;

		for (x = 0; x < 256; ++x)
		{
			UINT32 out_val;

			UINT32 char_val = *chr_addr++;
			UINT32 char_6_7 = (char_val & 0xc0) >> 2;

			UINT32 obj_val = *obj_addr++;
			UINT32 obj6	= BIT(obj_val, 6);

			UINT32 rod_val = *rod_addr++;
			UINT32 rod6	= BIT(rod_val, 6);

			UINT32 chr = !(BIT(char_val, 7) && (char_val & 3) );

			UINT32 sel =
			(
				( BIT(obj_val, 6) && chr) ||
				( sky_en && !(char_val & 3) && (!obj6 && !rod6) )
			) ? 0 : 1;

			sel |= (!(obj6 || rod6) || !chr) ? 2 : 0;

			/* Select the layer */
			if		(sel == 0)	out_val = obj_val & 0x3f;
			else if (sel == 1)	out_val = rod_val & 0x3f;
			else if (sel == 2)	out_val = sky_val;
			else				out_val = char_6_7 + chr_pal[char_val];

			*bmp_addr++ = (sel << 6) + out_val;
		}
	}

	return 0;
}
