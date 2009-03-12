/**********************************************************************

    Signetics 2636 video chip

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.


    PVI REGISTER DESCRIPTION
    ------------------------

          |              bit              |R/W| description
    byte  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |   |
          |                               |   |
    FC0   | size 4| size 3| size 2| size 1| W | size of the 4 objects(=sprites)
          |                               |   |
    FC1   |       |C1 |C2 |C3 |C1 |C2 |C3 | W | colors of the 4 objects
          |       |  color 1  |  color 2  |   |
    FC2   |       |C1 |C2 |C3 |C1 |C2 |C3 | W |
          |       |  color 3  |  color 4  |   |
          |                               |   |
    FC3   |                       |sh |pos| W | 1=shape 0=position
          |                               |   | display format and position
    FC4   |            (free)             |   |
    FC5   |            (free)             |   |
          |                               |   |
    FC6   |   |C1 |C2 |C3 |BG |scrn colr  | W | background lock and color
          |   |backg colr |enb|C1 |C2 |C3 |   | 3="enable"
          |                               |   |
    FC7   |            sound              | W | squarewave output
          |                               |   |
    FC8   |       N1      |      N2       | W | range of the 4 display digits
    FC9   |       N3      |      N4       | W |
          |                               |   |
          |obj/backgrnd   |complete object| R |
    FCA   | 1 | 2 | 3 | 4 | 1 | 2 | 3 | 4 |   |
          |                               |   |
    FCB   |   |VR-|   object collisions   | R | Composition of object and back-
          |   |LE |1/2|1/3|1/3|1/4|2/4|3/4|   | ground,collision detection and
          |                               |   | object display as a state display
          |                               |   | for the status register.Set VRLE.
          |                               |   | wait for VRST.Read out or transmit
          |                               |   | [copy?] all bits until reset by
          |                               |   | VRST.
          |                               |   |
    FCC   |            PORT1              | R | PORT1 and PORT2 for the range of
    FCD   |            PORT2              |   | the A/D conversion.Cleared by VRST
    FCE   |            (free)             |   |
    FCF   |            (free)             |   |


    Size control by byte FC0

     bit  matrix
    |0|0|  8x10
    |0|1| 16x20
    |1|0| 32x40
    |1|1| 64x80

    CE1 and not-CE2 are outputs from the PVI.$E80..$EFF also controls the
    analog multiplexer.


    SPRITES
    -------

    each object field: (=sprite data structure)

    0 \ 10 bytes of bitmap (Each object is 8 pixels wide.)
    9 /
    A   HC  horizontal object coordinate
    B   HCB horizontal duplicate coordinate
    C   VC  vertical object coordinate
    D   VCB vertical duplicate coordinate

*************************************************************/

#include "driver.h"
#include "s2636.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define SPRITE_WIDTH	(8)
#define SPRITE_HEIGHT	(10)

static const int sprite_offsets[4] = { 0x00, 0x10, 0x20, 0x40 };



/*************************************
 *
 *  Internal S2636 data structure
 *
 *************************************/

struct _s2636_t
{
	UINT8 *work_ram;
	int y_offset;
	int x_offset;

	bitmap_t *bitmap;
	bitmap_t *collision_bitmap;
};



/*************************************
 *
 *  Configuration
 *
 *************************************/

s2636_t *s2636_config(UINT8 *work_ram, int screen_height, int screen_width, int y_offset, int x_offset)
{
	s2636_t *s2636;

	/* allocate the object that holds the state */
	s2636 = (s2636_t *)auto_malloc(sizeof(*s2636));

	s2636->work_ram = work_ram;
	s2636->y_offset = y_offset;
	s2636->x_offset = x_offset;

	s2636->bitmap = auto_bitmap_alloc(screen_width, screen_height, BITMAP_FORMAT_INDEXED16);
	s2636->collision_bitmap = auto_bitmap_alloc(screen_width, screen_height, BITMAP_FORMAT_INDEXED16);

	return s2636;
}



/*************************************
 *
 *  Draw a sprite
 *
 *************************************/

static void draw_sprite(UINT8 *gfx, int color, int y, int x, int expand,
						int or_mode, bitmap_t *bitmap, const rectangle *cliprect)
{
	int sy;

	/* for each row */
	for (sy = 0; sy < SPRITE_HEIGHT; sy++)
	{
		int sx;

		/* for each pixel on the row */
		for (sx = 0; sx < SPRITE_WIDTH; sx++)
		{
			int ey;

			/* each pixel can be expanded */
			for (ey = 0; ey <= expand; ey++)
			{
				int ex;

				for (ex = 0; ex <= expand; ex++)
				{
					/* compute effective destination pixel */
					int ty = y + sy * (expand + 1) + ey;
					int tx = x + sx * (expand + 1) + ex;

					/* get out if outside the drawing region */
					if ((tx < cliprect->min_x) ||
						(tx > cliprect->max_x) ||
						(ty < cliprect->min_y) ||
						(ty > cliprect->max_y))
						continue;

					/* get out if current image bit is transparent */
					if (((gfx[sy] << sx) & 0x80) == 0x00)
						continue;

					if (or_mode)
						*BITMAP_ADDR16(bitmap, ty, tx) = 0x08 | *BITMAP_ADDR16(bitmap, ty, tx) | color;
					else
						*BITMAP_ADDR16(bitmap, ty, tx) = 0x08 | color;
				}
			}
		}
	}
}



/*************************************
 *
 *  Collision detection
 *
 *************************************/

static int check_collision(s2636_t *s2636, int spriteno1, int spriteno2, const rectangle *cliprect)
{
	int checksum = 0;

	UINT8* attr1 = &s2636->work_ram[sprite_offsets[spriteno1]];
	UINT8* attr2 = &s2636->work_ram[sprite_offsets[spriteno2]];

	/* TODO: does not check shadow sprites yet */

	bitmap_fill(s2636->collision_bitmap, cliprect, 0);

	if ((attr1[0x0a] != 0xff) && (attr2[0x0a] != 0xff))
	{
		int x, y;

		int x1 = attr1[0x0a] + s2636->x_offset;
		int y1 = attr1[0x0c] + s2636->y_offset;
		int x2 = attr2[0x0a] + s2636->x_offset;
		int y2 = attr2[0x0c] + s2636->y_offset;

		int expand1 = (s2636->work_ram[0xc0] >> (spriteno1 << 1)) & 0x03;
		int expand2 = (s2636->work_ram[0xc0] >> (spriteno2 << 1)) & 0x03;

		/* draw first sprite */
		draw_sprite(attr1, 1, y1, x1, expand1, FALSE, s2636->collision_bitmap, cliprect);

		/* get fingerprint */
		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
			{
				if ((x < cliprect->min_x) ||
					(x > cliprect->max_x) ||
					(y < cliprect->min_y) ||
					(y > cliprect->max_y))
					continue;

				checksum = checksum + *BITMAP_ADDR16(s2636->collision_bitmap, y, x);
			}

		/* black out second sprite */
		draw_sprite(attr2, 0, y2, x2, expand2, FALSE, s2636->collision_bitmap, cliprect);

		/* remove fingerprint */
		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
			{
				if ((x < cliprect->min_x) ||
					(x > cliprect->max_x) ||
					(y < cliprect->min_y) ||
					(y > cliprect->max_y))
					continue;

				checksum = checksum - *BITMAP_ADDR16(s2636->collision_bitmap, y, x);
			}
	}

	return (checksum != 0);
}



/*************************************
 *
 *  Main drawing
 *
 *************************************/

bitmap_t *s2636_update(s2636_t *s2636, const rectangle *cliprect)
{
	UINT8 collision = 0;
	int spriteno;

	bitmap_fill(s2636->bitmap, cliprect, 0);

	for (spriteno = 0; spriteno < 4; spriteno++)
	{
		int color, expand, x, y;

		UINT8* attr = &s2636->work_ram[sprite_offsets[spriteno]];

		/* get out if sprite is turned off */
		if (attr[0x0a] == 0xff)
			continue;

		x = attr[0x0a] + s2636->x_offset;
		y = attr[0x0c] + s2636->y_offset;

		color = (s2636->work_ram[0xc1 + (spriteno >> 1)] >> ((spriteno & 1) ? 0 : 3)) & 0x07;
		expand = (s2636->work_ram[0xc0] >> (spriteno << 1)) & 0x03;

		draw_sprite(attr, color, y, x, expand, TRUE, s2636->bitmap, cliprect);

		/* bail if no shadow sprites */
		if ((attr[0x0b] == 0xff) || (attr[0x0d] == 0xfe))
			continue;

		x = attr[0x0b] + s2636->x_offset;

		while (y < 0xff)
		{
			y = y + SPRITE_HEIGHT + attr[0x0d];

			draw_sprite(attr, color, y, x, expand, TRUE, s2636->bitmap, cliprect);
		}
	}

	/* collision detection */
	if (check_collision(s2636, 0, 1, cliprect))  collision |= 0x20;
	if (check_collision(s2636, 0, 2, cliprect))  collision |= 0x10;
	if (check_collision(s2636, 0, 3, cliprect))  collision |= 0x08;
	if (check_collision(s2636, 1, 2, cliprect))  collision |= 0x04;
	if (check_collision(s2636, 1, 3, cliprect))  collision |= 0x02;
	if (check_collision(s2636, 2, 3, cliprect))  collision |= 0x01;

	s2636->work_ram[0xcb] = collision;

	return s2636->bitmap;
}
