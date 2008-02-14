/***************************************************************************

    Space Firebird hardware

****************************************************************************/

#include "driver.h"
#include "spacefb.h"
#include "video/resnet.h"


UINT8 *spacefb_videoram;
size_t spacefb_videoram_size;


/* bitmap that marks whether a given pixel has a sprite/bullet there */
static bitmap_t *object_present_map;

static UINT8 flipscreen;
static UINT8 gfx_bank;
static UINT8 palette_bank;
static UINT8 background_red;
static UINT8 background_blue;
static UINT8 disable_star_field;
static UINT8 color_contrast_r;
static UINT8 color_contrast_g;
static UINT8 color_contrast_b;
static UINT32 star_shift_reg;

static double color_weights_r[3], color_weights_g[3], color_weights_b[3];



/*************************************
 *
 *  Port bit setters
 *
 *************************************/

void spacefb_set_flip_screen(UINT8 data)
{
	flipscreen = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_gfx_bank(UINT8 data)
{
	gfx_bank = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_palette_bank(UINT8 data)
{
	palette_bank = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_background_red(UINT8 data)
{
	background_red = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_background_blue(UINT8 data)
{
	background_blue = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_disable_star_field(UINT8 data)
{
	disable_star_field = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_color_contrast_r(UINT8 data)
{
	color_contrast_r = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_color_contrast_g(UINT8 data)
{
	color_contrast_g = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}


void spacefb_set_color_contrast_b(UINT8 data)
{
	color_contrast_b = data;

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}



/*************************************
 *
 *  Video system start
 *
 *  The sprites use a 32 bytes palette PROM, connected to the RGB output this
 *  way:
 *
 *  bit 7 -- 220 ohm resistor  -- BLUE
 *        -- 470 ohm resistor  -- BLUE
 *        -- 220 ohm resistor  -- GREEN
 *        -- 470 ohm resistor  -- GREEN
 *        -- 1  kohm resistor  -- GREEN
 *        -- 220 ohm resistor  -- RED
 *        -- 470 ohm resistor  -- RED
 *  bit 0 -- 1  kohm resistor  -- RED
 *
 *
 *  The schematics show that the bullet color is connected this way,
 *  but this is impossible
 *
 *           860 ohm resistor  -- RED
 *            68 ohm resistor  -- GREEN
 *
 *
 *  The background color is connected this way:
 *
 *  Ra    -- 220 ohm resistor  -- BLUE
 *  Rb    -- 470 ohm resistor  -- BLUE
 *  Ga    -- 220 ohm resistor  -- GREEN
 *  Gb    -- 470 ohm resistor  -- GREEN
 *  Ba    -- 220 ohm resistor  -- RED
 *  Bb    -- 470 ohm resistor  -- RED
 *
 *************************************/

VIDEO_START( spacefb )
{
	/* compute the color gun weights */
	static const int resistances_rg[] = { 1000, 470, 220 };
	static const int resistances_b [] = {       470, 220 };

	compute_resistor_weights(0, 0xff, -1.0,
							 3, resistances_rg, color_weights_r, 470, 0,
							 3, resistances_rg, color_weights_g, 470, 0,
							 2, resistances_b,  color_weights_b, 470, 0);

	object_present_map = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, BITMAP_FORMAT_INDEXED8);

	/* this start value positions the stars to match the flyer screen shot,
       but most likely, the actual star position is random as the hardware
       uses whatever value is on the shift register on power-up */
	star_shift_reg = 0x18f89;
}



/*************************************
 *
 *  Star field generator
 *
 *************************************/

#define NUM_STARFIELD_PENS	(0x40)


INLINE void shift_star_generator(void)
{
	star_shift_reg = ((star_shift_reg << 1) | (((~star_shift_reg >> 16) & 0x01) ^ ((star_shift_reg >> 4) & 0x01))) & 0x1ffff;
}


static void get_starfield_pens(pen_t *pens)
{
	/* generate the pens based on the various enable bits */
	int i;

	for (i = 0; i < NUM_STARFIELD_PENS; i++)
	{
		UINT8 gb =  ((i >> 0) & 0x01) & color_contrast_g & !disable_star_field;
		UINT8 ga =  ((i >> 1) & 0x01) & !disable_star_field;
		UINT8 bb =  ((i >> 2) & 0x01) & color_contrast_b & !disable_star_field;
		UINT8 ba = (((i >> 3) & 0x01) | background_blue) & !disable_star_field;
		UINT8 ra = (((i >> 4) & 0x01) | background_red) & !disable_star_field;
		UINT8 rb =  ((i >> 5) & 0x01) & color_contrast_r & !disable_star_field;

		UINT8 r = combine_3_weights(color_weights_r, 0, rb, ra);
		UINT8 g = combine_3_weights(color_weights_g, 0, gb, ga);
		UINT8 b = combine_2_weights(color_weights_b,    bb, ba);

		pens[i] = MAKE_RGB(r, g, b);
	}

}


static void draw_starfield(running_machine *machine,
						   int scrnum,
						   mame_bitmap *bitmap,
						   const rectangle *cliprect)
{
	int y;
	pen_t pens[NUM_STARFIELD_PENS];

	get_starfield_pens(pens);

	/* the shift register is always shifting -- do the portion in the top VBLANK */
	if (cliprect->min_y == machine->screen[scrnum].visarea.min_y)
	{
		int i;

		/* one cycle delay introduced by IC10 D flip-flop at the end of the VBLANK */
		int clock_count = (SPACEFB_HBSTART - SPACEFB_HBEND) * SPACEFB_VBEND - 1;

		for (i = 0; i < clock_count; i++)
			shift_star_generator();
	}

	/* visible region of the screen */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;

		for (x = SPACEFB_HBEND; x < SPACEFB_HBSTART; x++)
		{
			if (!*BITMAP_ADDR8(object_present_map, y, x))
			{
				/* draw the star - the 4 possible values come from the effect of the two XOR gates */
				if (((star_shift_reg & 0x1c0ff) == 0x0c0b7) ||
					((star_shift_reg & 0x1c0ff) == 0x0c0d7) ||
					((star_shift_reg & 0x1c0ff) == 0x0c0bb) ||
					((star_shift_reg & 0x1c0ff) == 0x0c0db))
					*BITMAP_ADDR32(bitmap, y, x) = pens[(star_shift_reg >> 8) & 0x3f];
				else
					*BITMAP_ADDR32(bitmap, y, x) = pens[0];
			}

			shift_star_generator();
		}
	}

	/* do the shifting in the bottom VBLANK */
	if (cliprect->max_y == machine->screen[scrnum].visarea.max_y)
	{
		int i;
		int clock_count = (SPACEFB_HBSTART - SPACEFB_HBEND) * (SPACEFB_VTOTAL - SPACEFB_VBSTART);

		for (i = 0; i < clock_count; i++)
			shift_star_generator();
	}
}



/*************************************
 *
 *  Sprite/Bullet hardware
 *
 *  Sprites are opaque wrt. each other,
 *  but transparent wrt. to the
 *  star field.
 *
 *************************************/

#define NUM_SPRITE_PENS	(0x40)


static void get_sprite_pens(pen_t *pens)
{
	static const double fade_weights[] = { 1.0, 1.5, 2.5, 4.0 };

	int i;

	for (i = 0; i < NUM_SPRITE_PENS; i++)
	{
		UINT8 data = memory_region(REGION_PROMS)[(palette_bank << 4) | (i & 0x0f)];

		UINT8 r0 = (data >> 0) & 0x01;
		UINT8 r1 = (data >> 1) & 0x01;
		UINT8 r2 = (data >> 2) & 0x01;

		UINT8 g0 = (data >> 3) & 0x01;
		UINT8 g1 = (data >> 4) & 0x01;
		UINT8 g2 = (data >> 5) & 0x01;

		UINT8 b1 = (data >> 6) & 0x01;
		UINT8 b2 = (data >> 7) & 0x01;

		UINT8 r = combine_3_weights(color_weights_r, r0, r1, r2);
		UINT8 g = combine_3_weights(color_weights_g, g0, g1, g2);
		UINT8 b = combine_2_weights(color_weights_b,     b1, b2);

		if (i >> 4)
		{
			double fade_weight = fade_weights[i >> 4];

			/* faded pens */
			r = (r / fade_weight) + 0.5;
			g = (g / fade_weight) + 0.5;
			b = (b / fade_weight) + 0.5;
		}

		pens[i] = MAKE_RGB(r, g, b);
	}
}


static void draw_bullet(offs_t offs, pen_t pen, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT8 sy;

	UINT8 *gfx = memory_region(REGION_GFX2);

	UINT8 code = spacefb_videoram[offs + 0x0200] & 0x3f;
	UINT8 y = ~spacefb_videoram[offs + 0x0100] - 2;

	for (sy = 0; sy < 4; sy++)
	{
		UINT8 sx, dy;

		UINT8 data = gfx[(code << 2) | sy];
		UINT8 x = spacefb_videoram[offs + 0x0000];

		if (flipscreen)
			dy = ~y;
		else
			dy = y;

		if ((dy > cliprect->min_y) && (dy < cliprect->max_y))
		{
			for (sx = 0; sx < 4; sx++)
			{
				if (data & 0x01)
				{
					UINT16 dx;

					if (flipscreen)
						dx = (255 - x) * 2;
					else
						dx = x * 2;

					*BITMAP_ADDR32(bitmap, dy, dx + 0) = pen;
					*BITMAP_ADDR32(bitmap, dy, dx + 1) = pen;

					*BITMAP_ADDR8(object_present_map, dy, dx + 0) = 1;
					*BITMAP_ADDR8(object_present_map, dy, dx + 1) = 1;
				}

				x = x + 1;
				data = data >> 1;
			}
		}

		y = y + 1;
	}
}


static void draw_sprite(offs_t offs, pen_t* pens, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT8 sy;

	UINT8 *gfx = memory_region(REGION_GFX1);

	UINT8 code = ~spacefb_videoram[offs + 0x0200];
	UINT8 color_base = (~spacefb_videoram[offs + 0x0300] & 0x0f) << 2;
	UINT8 y = ~spacefb_videoram[offs + 0x0100] - 2;

	for (sy = 0; sy < 8; sy++)
	{
		UINT8 sx, dy;

		UINT8 data1 = gfx[0x0000 | (code << 3) | (sy ^ 0x07)];
		UINT8 data2 = gfx[0x0800 | (code << 3) | (sy ^ 0x07)];

		UINT8 x = spacefb_videoram[offs + 0x0000] - 3;

		if (flipscreen)
			dy = ~y;
		else
			dy = y;

		if ((dy > cliprect->min_y) && (dy < cliprect->max_y))
		{
			for (sx = 0; sx < 8; sx++)
			{
				UINT16 dx;
				UINT8 data;
				pen_t pen;

				if (flipscreen)
					dx = (255 - x) * 2;
				else
					dx = x * 2;

				data = ((data1 << 1) & 0x02) | (data2 & 0x01);
				pen = pens[color_base | data];

				*BITMAP_ADDR32(bitmap, dy, dx + 0) = pen;
				*BITMAP_ADDR32(bitmap, dy, dx + 1) = pen;

				*BITMAP_ADDR8(object_present_map, dy, dx + 0) = (data != 0);
				*BITMAP_ADDR8(object_present_map, dy, dx + 1) = (data != 0);

				x = x + 1;
				data1 = data1 >> 1;
				data2 = data2 >> 1;
			}
		}

		y = y + 1;
	}
}


static void draw_objects(mame_bitmap *bitmap, const rectangle *cliprect)
{
	pen_t sprite_pens[NUM_SPRITE_PENS];

	offs_t offs = gfx_bank ? 0x80 : 0x00;

	/* since the way the schematics show the bullet color
       connected is impossible, just use pure red for now */
	pen_t bullet_pen = MAKE_RGB(0xff, 0x00, 0x00);

	get_sprite_pens(sprite_pens);

	while (1)
	{
		if (spacefb_videoram[offs + 0x0300] & 0x20)
			draw_bullet(offs, bullet_pen, bitmap, cliprect);
		else if (spacefb_videoram[offs + 0x0300] & 0x40)
			draw_sprite(offs, sprite_pens, bitmap, cliprect);

		/* next object */
		offs = offs + 1;

		/* end of bank? */
		if ((offs & 0x7f) == 0)  break;
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( spacefb )
{
	fillbitmap(object_present_map, 0, cliprect);

	draw_objects(bitmap, cliprect);

	draw_starfield(machine, screen, bitmap, cliprect);

	return 0;
}
