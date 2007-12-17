/***************************************************************************

  liberator.c - 'video.c'

  Functions to emulate the video hardware of the machine.

   Liberator's screen is 256 pixels by 256 pixels.  The
     round planet in the middle of the screen is 128 pixels
     tall by 96 equivalent (192 at double pixel rate).  The
     emulator needs to account for the aspect ratio of 4/3
     from the arcade video system in order to make the planet
     appear round.

***************************************************************************/

#include "driver.h"
#include "liberatr.h"


#define NUM_PENS	(0x18)


UINT8 *liberatr_base_ram;
UINT8 *liberatr_planet_frame;
UINT8 *liberatr_planet_select;
UINT8 *liberatr_x;
UINT8 *liberatr_y;
UINT8 *liberatr_bitmapram;
UINT8 *liberatr_colorram;

static UINT8 *liberatr_videoram;



/*
    The following structure describes the (up to 32) line segments
    that make up one horizontal line (latitude) for one display frame of the planet.
    Note: this and the following structure is only used to collect the
    data before it is packed for actual use.
*/
typedef struct
{
	UINT8 segment_count;	/* the number of segments on this line */
	UINT8 max_x;			/* the maximum value of x_array for this line */
	UINT8 color_array[32];	/* the color values  */
	UINT8 x_array[32];		/* and maximum x values for each segment  */
} planet_frame_line;

/*
    The following structure describes the lines (latitudes)
    that make up one complete display frame of the planet.
    Note: this and the previous structure is only used to collect the
    data before it is packed for actual use.
*/
typedef struct
{
	planet_frame_line lines[0x80];
} planet_frame;


/*
    The following structure collects the 256 frames of the
    planet (one per value of longitude).
    The data is packed segment_count,segment_start,color,length,color,length,...  then
                       segment_count,segment_start,color,length,color,length...  for the next line, etc
    for the 128 lines.
*/
typedef struct
{
	UINT8 *frames[256];
} planet;


/*
    The following array collects the 2 different planet
    descriptions, which are selected by liberatr_planetbit
*/
static planet *liberatr_planets[2];


WRITE8_HANDLER( liberatr_bitmap_xy_w )
{
	liberatr_videoram[(*liberatr_y << 8) | *liberatr_x] = data & 0xe0;
}


READ8_HANDLER( liberatr_bitmap_xy_r )
{
	return liberatr_videoram[(*liberatr_y << 8) | *liberatr_x];
}


WRITE8_HANDLER( liberatr_bitmap_w )
{
	UINT8 x, y;

	liberatr_bitmapram[offset] = data;

	x = (offset & 0x3f) << 2;
	y = offset >> 6;

	data = data & 0xe0;

	liberatr_videoram[(y << 8) | x | 0] = data;
	liberatr_videoram[(y << 8) | x | 1] = data;
	liberatr_videoram[(y << 8) | x | 2] = data;
	liberatr_videoram[(y << 8) | x | 3] = data;
}



/********************************************************************************************
  liberatr_init_planet()

  The data for the planet is stored in ROM using a run-length type of encoding.  This
  function does the conversion to the above structures and then a smaller
  structure which is quicker to use in real time.

  Its a multi-step process, reflecting the history of the code.  Not quite as efficient
  as it might be, but this is not realtime stuff, so who cares...
 ********************************************************************************************/

static void liberatr_init_planet(running_machine *machine, planet *liberatr_planet, UINT8 *planet_rom)
{
	UINT16 longitude;

	const UINT8* latitude_scale = memory_region(REGION_USER1);
	const UINT8* longitude_scale = memory_region(REGION_USER2);

	/* for each starting longitude */
	for (longitude = 0; longitude < 0x100; longitude++)
	{
		UINT8 i, latitude, start_segment, segment_count;
		UINT8 *buffer;

		planet_frame frame;
		planet_frame_line *line = 0;

		UINT16 total_segment_count = 0;

		/* for each latitude */
		for (latitude = 0; latitude < 0x80; latitude++)
		{
			UINT8 segment, longitude_scale_factor, latitude_scale_factor, color, x=0;
			UINT8 x_array[32], color_array[32], visible_array[32];

			/* point to the structure which will hold the data for this line */
			line = &frame.lines[latitude];

			latitude_scale_factor = latitude_scale[latitude];

			/* for this latitude, load the 32 segments into the arrays */
			for (segment = 0; segment < 0x20; segment++)
			{
				UINT16 length, planet_data, address;

				/*
                   read the planet picture ROM and get the
                   latitude and longitude scaled from the scaling PROMS
                */
				address = (latitude << 5) + segment;
				planet_data = (planet_rom[address] << 8) | planet_rom[address + 0x1000];

				color  =  (planet_data >> 8) & 0x0f;
				length = ((planet_data << 1) & 0x1fe) + ((planet_data >> 15) & 0x01);


				/* scale the longitude limit (adding the starting longitude) */
				address = longitude + ( length >> 1 ) + ( length & 1 );		/* shift with rounding */
				visible_array[segment] = (( address & 0x100 ) ? 1 : 0);
				if (address & 0x80)
				{
					longitude_scale_factor = 0xff;
				}
				else
				{
					address = ((address & 0x7f) << 1) + (((length & 1) || visible_array[segment]) ? 0 : 1);
					longitude_scale_factor = longitude_scale[address];
				}

				x_array[segment] = (((UINT16)latitude_scale_factor * (UINT16)longitude_scale_factor) + 0x80) >> 8;	/* round it */
				color_array[segment] = color;
			}

			/*
               determine which segment is the western horizon and
                 leave 'segment' indexing it.
            */
			for (segment = 0; segment < 0x1f; segment++)	/* if not found, 'segment' = 0x1f */
				if (visible_array[segment]) break;

			/* transfer from the temporary arrays to the structure */
			line->max_x = (latitude_scale_factor * 0xc0) >> 8;
			if (line->max_x & 1)
				line->max_x += 1; 				/* make it even */

			/*
               as part of the quest to reduce memory usage (and to a lesser degree
                 execution time), stitch together segments that have the same color
            */
			segment_count = 0;
			i = 0;
			start_segment = segment;

			do
			{
				color = color_array[segment];
				while (color == color_array[segment])
				{
					x = x_array[segment];
					segment = (segment+1) & 0x1f;
					if (segment == start_segment)
						break;
				}

				line->color_array[i] = color;
				line->x_array[i] = (x > line->max_x) ? line->max_x : x;
				i++;
				segment_count++;
			} while ((i < 32) && (x <= line->max_x));

			total_segment_count += segment_count;
			line->segment_count = segment_count;
		}

		/* now that the all the lines have been processed, and we know how
           many segments it will take to store the description, allocate the
           space for it and copy the data to it.
        */
		buffer = auto_malloc(2*(128 + total_segment_count));

		liberatr_planet->frames[longitude] = buffer;

		for (latitude = 0; latitude < 0x80; latitude++)
		{
			UINT8 last_x;


			line = &frame.lines[latitude];
			segment_count = line->segment_count;
			*buffer++ = segment_count;
			last_x = 0;

			/* calculate the bitmap's x coordinate for the western horizon
               center of bitmap - (the number of planet pixels) / 4 */
			*buffer++ = machine->screen[0].width/2 - (line->max_x + 2) / 4;

			for (i = 0; i < segment_count; i++)
			{
				UINT8 current_x = (line->x_array[i] + 1) / 2;

				*buffer++ = line->color_array[i];
				*buffer++ = current_x - last_x;

				last_x = current_x;
			}
		}
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( liberatr )
{
	liberatr_videoram = auto_malloc(machine->screen[0].width * machine->screen[0].height);

	/* allocate the planet descriptor structure */
	liberatr_planets[0] = auto_malloc(sizeof(planet));
	liberatr_planets[1] = auto_malloc(sizeof(planet));

	/* for each planet in the planet ROMs */
	liberatr_init_planet(machine, liberatr_planets[0], &memory_region(REGION_GFX1)[0x2000]);
	liberatr_init_planet(machine, liberatr_planets[1], &memory_region(REGION_GFX1)[0x0000]);
}


static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		UINT8 r,g,b;

		/* handle the hardware flip of the bit order from 765 to 576 that
           hardware does between vram and color ram */
		static offs_t penmap[] = { 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
								   0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
								   0x10, 0x12, 0x14, 0x16, 0x11, 0x13, 0x15, 0x17 };

		UINT8 data = liberatr_colorram[i];

		/* scale it from 0x00-0xff */
		r = ((~data >> 3) & 0x07) * 0x24 + 3;  if (r == 3)  r = 0;
		g = ((~data >> 0) & 0x07) * 0x24 + 3;  if (g == 3)  g = 0;
		b = ((~data >> 5) & 0x06) * 0x24 + 3;  if (b == 3)  b = 0;

		pens[penmap[i]] = MAKE_RGB(r, g, b);
	}
}


static void liberatr_draw_planet(mame_bitmap *bitmap, pen_t *pens)
{
	UINT8 latitude;

	UINT8 *buffer = liberatr_planets[(*liberatr_planet_select >> 4) & 0x01]->frames[*liberatr_planet_frame];

	/* for each latitude */
	for (latitude = 0; latitude < 0x80; latitude++)
	{
		UINT8 segment;

		/* grab the color value for the base (if any) at this latitude */
		UINT8 base_color = liberatr_base_ram[latitude >> 3] ^ 0x0f;

		UINT8 segment_count = *buffer++;
		UINT8 x = *buffer++;
		UINT8 y = 64 + latitude;

		/* run through the segments, drawing its color until its x_array value comes up. */
		for (segment = 0; segment < segment_count; segment++)
		{
			UINT8 i;

			UINT8 color = *buffer++;
			UINT8 segment_length = *buffer++;

			if ((color & 0x0c) == 0x0c)
			{
				color = base_color;
			}

			for (i = 0; i < segment_length; i++, x++)
			{
				*BITMAP_ADDR32(bitmap, y, x) = pens[color];
			}
		}
	}
}


static void liberatr_draw_bitmap(running_machine *machine, mame_bitmap *bitmap, pen_t *pens)
{
	offs_t offs;

	for (offs = 0; offs < machine->screen[0].width * machine->screen[0].height; offs++)
	{
		UINT8 data = liberatr_videoram[offs];

		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		if (data)
			*BITMAP_ADDR32(bitmap, y, x) = pens[(data >> 5) | 0x10];
	}
}


VIDEO_UPDATE( liberatr )
{
	pen_t pens[NUM_PENS];
	get_pens(pens);

	fillbitmap(bitmap, RGB_BLACK, cliprect);

	liberatr_draw_planet(bitmap, pens);

	liberatr_draw_bitmap(machine, bitmap, pens);

	return 0;
}
