/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "8080bw.h"
#include "mw8080bw.h"


#define NUM_PENS	(8)


UINT8 *c8080bw_colorram;

static UINT8 c8080bw_flip_screen;
static UINT8 color_map;
static UINT8 screen_red;
static UINT8 schaser_background_disable;
static UINT8 schaser_background_select;



void c8080bw_flip_screen_w(int data)
{
	color_map = data;
	c8080bw_flip_screen = data && (readinputportbytag(CABINET_PORT_TAG) & 0x01);
}


void c8080bw_screen_red_w(int data)
{
	screen_red = data;
}


void schaser_background_control_w(int data)
{
	schaser_background_disable = (data >> 3) & 0x01;
	schaser_background_select = (data >> 4) & 0x01;
}


static void invadpt2_get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}
}


static void sflush_get_pens(pen_t *pens)
{
	offs_t i;

	pens[0] = MAKE_RGB(0x80, 0x80, 0xff);

	for (i = 1; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}
}


static void cosmo_get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}


INLINE void set_pixel(mame_bitmap *bitmap, UINT8 y, UINT8 x, pen_t *pens, UINT8 color)
{
	if (y >= MW8080BW_VCOUNTER_START_NO_VBLANK)
	{
		if (c8080bw_flip_screen)
			*BITMAP_ADDR32(bitmap, MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - x) = pens[color];
		else
			*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pens[color];
	}
}


INLINE void set_8_pixels(mame_bitmap *bitmap, UINT8 y, UINT8 x, UINT8 data,
						  pen_t *pens, UINT8 fore_color, UINT8 back_color)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		set_pixel(bitmap, y, x, pens, (data & 0x01) ? fore_color : back_color);

		x = x + 1;
		data = data >> 1;
	}
}


/* this is needed as this driver doesn't emulate the shift register like mw8080bw does */
static void clear_extra_columns(mame_bitmap *bitmap, pen_t *pens, UINT8 color)
{
	UINT8 x;

	for (x = 0; x < 4; x++)
	{
		UINT8 y;

		for (y = MW8080BW_VCOUNTER_START_NO_VBLANK; y != 0; y++)
		{
			if (c8080bw_flip_screen)
				*BITMAP_ADDR32(bitmap, MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - (256 + x)) = pens[color];
			else
				*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + x) = pens[color];
		}
	}
}


VIDEO_UPDATE( invadpt2 )
{
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;

	invadpt2_get_pens(pens);

	color_map_base = color_map ? &memory_region(REGION_PROMS)[0x0400] : memory_region(REGION_PROMS);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = screen_red ? 1 : color_map_base[color_address] & 0x07;

		set_8_pixels(bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( ballbomb )
{
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;

	invadpt2_get_pens(pens);

	color_map_base = color_map ? &memory_region(REGION_PROMS)[0x0400] : memory_region(REGION_PROMS);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = screen_red ? 1 : color_map_base[color_address] & 0x07;

		/* blue background */
		set_8_pixels(bitmap, y, x, data, pens, fore_color, 2);
	}

	clear_extra_columns(bitmap, pens, 2);

	return 0;
}


VIDEO_UPDATE( schaser )
{
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *background_map_base;

	invadpt2_get_pens(pens);

	background_map_base = memory_region(REGION_PROMS);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 back_color = 0;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = c8080bw_colorram[offs & 0x1f1f] & 0x07;

		if (!schaser_background_disable)
		{
			offs_t back_address = (offs >> 8 << 5) | (offs & 0x1f);

			UINT8 back_data = background_map_base[back_address];

			/* the equations derived from the schematics don't appear to produce
               the right colors, but this one does, at least for this PROM */
			back_color = (((back_data & 0x0c) == 0x0c) && schaser_background_select) ? 4 : 2;
		}

		set_8_pixels(bitmap, y, x, data, pens, fore_color, back_color);
	}

	clear_extra_columns(bitmap, pens, schaser_background_disable ? 0 : 2);

	return 0;
}


VIDEO_UPDATE( schasrcv )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	invadpt2_get_pens(pens);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = c8080bw_colorram[offs & 0x1f1f] & 0x07;

		/* blue background */
		set_8_pixels(bitmap, y, x, data, pens, fore_color, 2);
	}

	clear_extra_columns(bitmap, pens, 2);

	return 0;
}


VIDEO_UPDATE( rollingc )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	invadpt2_get_pens(pens);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = c8080bw_colorram[offs & 0x1f1f] & 0x07;

		set_8_pixels(bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( polaris )
{
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;
	UINT8 *cloud_gfx;

	invadpt2_get_pens(pens);

	color_map_base = memory_region(REGION_PROMS);
	cloud_gfx = memory_region(REGION_USER1);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		/* for the background color, bit 0 of the map PROM is connected to green gun.
           red is 0 and blue is 1, giving cyan and blue for the background.  This
           is different from what the schematics shows, but it's supported
           by screenshots.  Bit 3 is connected to cloud enable, while
           bits 1 and 2 are marked 'not use' (sic) */

		UINT8 back_color = (color_map_base[color_address] & 0x01) ? 6 : 2;
		UINT8 fore_color = ~c8080bw_colorram[offs & 0x1f1f] & 0x07;

		UINT8 cloud_y = y - polaris_get_cloud_pos();

		if ((color_map_base[color_address] & 0x08) || (cloud_y >= 64))
		{
			set_8_pixels(bitmap, y, x, data, pens, fore_color, back_color);
		}
		else
		{
			/* cloud appears in this part of the screen */
			int i;

			for (i = 0; i < 8; i++)
			{
				UINT8 color;

				if (data & 0x01)
				{
					color = fore_color;
				}
				else
				{
					int bit = 1 << (~x & 0x03);
					offs_t cloud_gfx_offs = ((x >> 2) & 0x03) | ((~cloud_y & 0x3f) << 2);

					color = (cloud_gfx[cloud_gfx_offs] & bit) ? 7 : back_color;
				}

				set_pixel(bitmap, y, x, pens, color);

				x = x + 1;
				data = data >> 1;
			}
		}
	}

	clear_extra_columns(bitmap, pens, 6);

	return 0;
}


VIDEO_UPDATE( lupin3 )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	invadpt2_get_pens(pens);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = ~c8080bw_colorram[offs & 0x1f1f] & 0x07;

		set_8_pixels(bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( cosmo )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	cosmo_get_pens(pens);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = c8080bw_colorram[color_address] & 0x07;

		set_8_pixels(bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( indianbt )
{
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;

	cosmo_get_pens(pens);

	color_map_base = color_map ? &memory_region(REGION_PROMS)[0x0400] : memory_region(REGION_PROMS);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = color_map_base[color_address] & 0x07;

		set_8_pixels(bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( shuttlei )
{
	pen_t pens[2] = { RGB_BLACK, RGB_WHITE };
	offs_t offs;

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
			data = data << 1;
		}
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( sflush )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	sflush_get_pens(pens);

	for (offs = 0; offs < mw8080bw_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = mw8080bw_ram[offs];
		UINT8 fore_color = c8080bw_colorram[offs & 0x1f1f] & 0x07;

		set_8_pixels(bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(bitmap, pens, 0);

	return 0;
}
