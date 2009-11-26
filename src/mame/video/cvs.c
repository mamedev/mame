/***************************************************************************

  video\cvs.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/s2636.h"
#include "cpu/s2650/s2650.h"
#include "cvs.h"


#define SPRITE_PEN_BASE		(0x820)
#define BULLET_STAR_PEN		(0x828)

#define MAX_STARS        	250
#define STARS_COLOR_BASE 	16


static s2636_t *s2636_0, *s2636_1, *s2636_2;
static bitmap_t *background_bitmap;
bitmap_t *cvs_collision_background;
static bitmap_t *scrolled_collision_background;

int cvs_collision_register;

struct star
{
	int x,y,code;
};


static struct star stars[MAX_STARS];
static int    total_stars;
static int    stars_on;
static UINT8  scroll_reg;
static int    stars_scroll;


/******************************************************
 * Convert Colour prom to format for Mame Colour Map  *
 *                                                    *
 * There is a prom used for colour mapping and plane  *
 * priority. This is converted to a colour table here *
 *                                                    *
 * colours are taken from SRAM and are programmable   *
 ******************************************************/

PALETTE_INIT( cvs )
{
	int i, attr;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x10);

	/* color mapping PROM */
	for (attr = 0; attr < 0x100; attr++)
	{
		for (i = 0; i < 8; i++)
		{
			UINT8 ctabentry = color_prom[(i << 8) | attr] & 0x07;

			/* bits 0 and 2 are swapped */
			ctabentry = BITSWAP8(ctabentry,7,6,5,4,3,0,1,2);

			colortable_entry_set_value(machine->colortable, (attr << 3) | i, ctabentry);
		}
	}

	/* background collision map */
	for (i = 0; i < 8; i++)
	{
		colortable_entry_set_value(machine->colortable, 0x800 + i, 0);
		colortable_entry_set_value(machine->colortable, 0x808 + i, i & 0x04);
		colortable_entry_set_value(machine->colortable, 0x810 + i, i & 0x02);
		colortable_entry_set_value(machine->colortable, 0x818 + i, i & 0x06);
	}

	/* sprites */
	for (i = 0; i < 8; i++)
		colortable_entry_set_value(machine->colortable, SPRITE_PEN_BASE + i, i | 0x08);

	/* bullet */
	colortable_entry_set_value(machine->colortable, BULLET_STAR_PEN, 7);
}


static void set_pens(colortable_t *colortable)
{
	int i;

	for (i = 0; i < 0x10; i++)
	{
		int r = pal2bit(~cvs_palette_ram[i] >> 0);
		int g = pal3bit(~cvs_palette_ram[i] >> 2);
		int b = pal3bit(~cvs_palette_ram[i] >> 5);

		colortable_palette_set_color(colortable, i, MAKE_RGB(r, g, b));
	}
}



WRITE8_HANDLER( cvs_video_fx_w )
{
	if (data & 0xce)
		logerror("%4x : CVS: Unimplemented CVS video fx = %2x\n",cpu_get_pc(space->cpu), data & 0xce);

    stars_on = data & 0x01;

    if (data & 0x02)   logerror("           SHADE BRIGHTER TO RIGHT\n");
    if (data & 0x04)   logerror("           SCREEN ROTATE\n");
    if (data & 0x08)   logerror("           SHADE BRIGHTER TO LEFT\n");

    set_led_status(space->machine, 1, data & 0x10);	/* lamp 1 */
    set_led_status(space->machine, 2, data & 0x20);	/* lamp 2 */

    if (data & 0x40)   logerror("           SHADE BRIGHTER TO BOTTOM\n");
    if (data & 0x80)   logerror("           SHADE BRIGHTER TO TOP\n");
}



READ8_HANDLER( cvs_collision_r )
{
	return cvs_collision_register;
}

READ8_HANDLER( cvs_collision_clear )
{
	cvs_collision_register = 0;

	return 0;
}


WRITE8_HANDLER( cvs_scroll_w )
{
	scroll_reg = 255 - data;
}


VIDEO_START( cvs )
{
	int generator = 0;
	int y, width, height;

	/* precalculate the star background */

	total_stars = 0;

	for (y = 255;y >= 0;y--)
	{
		int x;

		for (x = 511;x >= 0;x--)
		{
			int bit1,bit2;

			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2) generator |= 1;

			if (((~generator >> 16) & 1) && (generator & 0xfe) == 0xfe)
			{
            	if(((~(generator >> 12)) & 0x01) && ((~(generator >> 13)) & 0x01))
                {
				    if (total_stars < MAX_STARS)
				    {
					    stars[total_stars].x = x;
					    stars[total_stars].y = y;
					    stars[total_stars].code = 1;

					    total_stars++;
				    }
                }
			}
		}
	}

	/* configure the S2636 chips */
	width = video_screen_get_width(machine->primary_screen);
	height = video_screen_get_height(machine->primary_screen);

	s2636_0 = s2636_config(machine, cvs_s2636_0_ram, height, width, CVS_S2636_Y_OFFSET, CVS_S2636_X_OFFSET);
	s2636_1 = s2636_config(machine, cvs_s2636_1_ram, height, width, CVS_S2636_Y_OFFSET, CVS_S2636_X_OFFSET);
	s2636_2 = s2636_config(machine, cvs_s2636_2_ram, height, width, CVS_S2636_Y_OFFSET, CVS_S2636_X_OFFSET);

	/* create helper bitmaps */
	background_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	cvs_collision_background = video_screen_auto_bitmap_alloc(machine->primary_screen);
	scrolled_collision_background = video_screen_auto_bitmap_alloc(machine->primary_screen);
}



void cvs_scroll_stars(void)
{
	stars_scroll++;
}



VIDEO_UPDATE( cvs )
{
	static const int ram_based_char_start_indices[] = { 0xe0, 0xc0, 0x100, 0x80 };
	offs_t offs;
	int scroll[8];
	UINT8 character_banking_mode;
	bitmap_t *s2636_0_bitmap;
	bitmap_t *s2636_1_bitmap;
	bitmap_t *s2636_2_bitmap;

	set_pens(screen->machine->colortable);

	/* create our background character set, which is a software
       selectable mixture of RAM and ROM based tiles */

	character_banking_mode = cvs_get_character_banking_mode();

	/* draw the background */
	for (offs = 0; offs < 0x0400; offs++)
	{
		int collision_color = 0x100;

		UINT8 code = cvs_video_ram[offs];
		UINT8 color = cvs_color_ram[offs];

		UINT8 x = offs << 3;
		UINT8 y = offs >> 5 << 3;

		int gfxnum = (code < ram_based_char_start_indices[character_banking_mode]) ? 0 : 1;

		drawgfx_opaque(background_bitmap, 0, screen->machine->gfx[gfxnum],
				code, color,
				0, 0,
				x, y);

		/* foreground for collision detection */
		if (color & 0x80)
			collision_color = 0x103;
		else
		{
			if ((color & 0x03) == 0x03)
				collision_color = 0x101;
			else if ((color & 0x01) == 0)
				collision_color = 0x102;
		}

		drawgfx_opaque(cvs_collision_background, 0, screen->machine->gfx[gfxnum],
				code, collision_color,
				0, 0,
				x, y);
	}


    /* Update screen - 8 regions, fixed scrolling area */

    scroll[0] = 0;
    scroll[1] = scroll_reg;
    scroll[2] = scroll_reg;
    scroll[3] = scroll_reg;
    scroll[4] = scroll_reg;
    scroll[5] = scroll_reg;
    scroll[6] = 0;
    scroll[7] = 0;

	copyscrollbitmap(bitmap, background_bitmap, 0, 0, 8, scroll, cliprect);
	copyscrollbitmap(scrolled_collision_background, cvs_collision_background, 0, 0, 8, scroll, cliprect);

    /* update the S2636 chips */
	s2636_0_bitmap = s2636_update(s2636_0, cliprect);
	s2636_1_bitmap = s2636_update(s2636_1, cliprect);
	s2636_2_bitmap = s2636_update(s2636_2, cliprect);

    /* Bullet Hardware */
    for (offs = 8; offs < 256; offs++ )
    {
        if (cvs_bullet_ram[offs] != 0)
        {
        	int ct;
            for(ct=0;ct<4;ct++)
            {
            	int bx=255-7-cvs_bullet_ram[offs]-ct;

            	/* Bullet/Object Collision */
				if ((*BITMAP_ADDR16(s2636_0_bitmap, offs, bx) != 0) ||
					(*BITMAP_ADDR16(s2636_1_bitmap, offs, bx) != 0) ||
					(*BITMAP_ADDR16(s2636_2_bitmap, offs, bx) != 0))
					cvs_collision_register |= 0x08;

            	/* Bullet/Background Collision */
				if (colortable_entry_get_value(screen->machine->colortable, *BITMAP_ADDR16(scrolled_collision_background, offs, bx)))
                   	cvs_collision_register |= 0x80;

				*BITMAP_ADDR16(bitmap, offs, bx) = BULLET_STAR_PEN;
            }
        }
    }


	/* mix and copy the S2636 images into the main bitmap, also check for collision */
	{
		int y;

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			int x;

			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				int pixel0 = *BITMAP_ADDR16(s2636_0_bitmap, y, x);
				int pixel1 = *BITMAP_ADDR16(s2636_1_bitmap, y, x);
				int pixel2 = *BITMAP_ADDR16(s2636_2_bitmap, y, x);

				int pixel = pixel0 | pixel1 | pixel2;

				if (S2636_IS_PIXEL_DRAWN(pixel))
				{
					*BITMAP_ADDR16(bitmap, y, x) = SPRITE_PEN_BASE + S2636_PIXEL_COLOR(pixel);

					/* S2636 vs. S2636 collision detection */
					if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) cvs_collision_register |= 0x01;
					if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) cvs_collision_register |= 0x02;
					if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel2)) cvs_collision_register |= 0x04;

					/* S2636 vs. background collision detection */
					if (colortable_entry_get_value(screen->machine->colortable, *BITMAP_ADDR16(scrolled_collision_background, y, x)))
					{
						if (S2636_IS_PIXEL_DRAWN(pixel0)) cvs_collision_register |= 0x10;
						if (S2636_IS_PIXEL_DRAWN(pixel1)) cvs_collision_register |= 0x20;
						if (S2636_IS_PIXEL_DRAWN(pixel2)) cvs_collision_register |= 0x40;
					}
				}
			}
		}
	}


    /* stars circuit */

	if (stars_on)
	{
		for (offs = 0;offs < total_stars;offs++)
		{
			UINT8 x = (stars[offs].x + stars_scroll) >> 1;
			UINT8 y = stars[offs].y + ((stars_scroll + stars[offs].x) >> 9);

			if ((y & 1) ^ ((x >> 4) & 1))
			{
				if (flip_screen_x_get(screen->machine))
					x = ~x;

				if (flip_screen_y_get(screen->machine))
					y = ~y;

				if ((y >= cliprect->min_y) && (y <= cliprect->max_y) &&
					(colortable_entry_get_value(screen->machine->colortable, *BITMAP_ADDR16(bitmap, y, x)) == 0))
					*BITMAP_ADDR16(bitmap, y, x) = BULLET_STAR_PEN;
			}
		}
	}

	return 0;
}
