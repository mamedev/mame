/***************************************************************************

    Cinematronics vector hardware

***************************************************************************/

#include "driver.h"
#include "video/vector.h"
#include "cpu/ccpu/ccpu.h"
#include "cinemat.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

enum
{
	COLOR_BILEVEL,
	COLOR_16LEVEL,
	COLOR_64LEVEL,
	COLOR_RGB,
	COLOR_QB3
};



/*************************************
 *
 *  Local variables
 *
 *************************************/

static int color_mode;
static rgb_t vector_color;
static INT16 lastx, lasty;
static UINT8 last_control;



/*************************************
 *
 *  Vector rendering
 *
 *************************************/

void cinemat_vector_callback(INT16 sx, INT16 sy, INT16 ex, INT16 ey, UINT8 shift)
{
	int intensity = 0xff;

	/* adjust for slop */
	sx = sx - Machine->screen[0].visarea.min_x;
	ex = ex - Machine->screen[0].visarea.min_x;
	sy = sy - Machine->screen[0].visarea.min_y;
	ey = ey - Machine->screen[0].visarea.min_y;

	/* point intensity is determined by the shift value */
	if (sx == ex && sy == ey)
		intensity = 0x1ff * shift / 8;

	/* move to the starting position if we're not there already */
	if (sx != lastx || sy != lasty)
		vector_add_point(sx << 16, sy << 16, 0, 0);

	/* draw the vector */
	vector_add_point(ex << 16, ey << 16, vector_color, intensity);

	/* remember the last point */
	lastx = ex;
	lasty = ey;
}



/*************************************
 *
 *  Vector color handling
 *
 *************************************/

WRITE8_HANDLER(cinemat_vector_control_w)
{
	int r, g, b, i;

	switch (color_mode)
	{
		case COLOR_BILEVEL:
			/* color is either bright or dim, selected by the value sent to the port */
			vector_color = (data & 1) ? MAKE_RGB(0x80,0x80,0x80) : MAKE_RGB(0xff,0xff,0xff);
			break;

		case COLOR_16LEVEL:
			/* on the rising edge of the data value, latch bits 0-3 of the */
			/* X register as the intensity */
			if (data != last_control && data)
			{
				int xval = cpunum_get_reg(0, CCPU_X) & 0x0f;
				i = (xval + 1) * 255 / 16;
				vector_color = MAKE_RGB(i,i,i);
			}
			break;

		case COLOR_64LEVEL:
			/* on the rising edge of the data value, latch bits 2-7 of the */
			/* X register as the intensity */
			if (data != last_control && data)
			{
				int xval = cpunum_get_reg(0, CCPU_X);
				xval = (~xval >> 2) & 0x3f;
				i = (xval + 1) * 255 / 64;
				vector_color = MAKE_RGB(i,i,i);
			}
			break;

		case COLOR_RGB:
			/* on the rising edge of the data value, latch the X register */
			/* as 4-4-4 BGR values */
			if (data != last_control && data)
			{
				int xval = cpunum_get_reg(0, CCPU_X);
				r = (~xval >> 0) & 0x0f;
				r = r * 255 / 15;
				g = (~xval >> 4) & 0x0f;
				g = g * 255 / 15;
				b = (~xval >> 8) & 0x0f;
				b = b * 255 / 15;
				vector_color = MAKE_RGB(r,g,b);
			}
			break;

		case COLOR_QB3:
			{
				static int lastx, lasty;

				/* on the falling edge of the data value, remember the original X,Y values */
				/* they will be restored on the rising edge; this is to simulate the fact */
				/* that the Rockola color hardware did not overwrite the beam X,Y position */
				/* on an IV instruction if data == 0 here */
				if (data != last_control && !data)
				{
					lastx = cpunum_get_reg(0, CCPU_X);
					lasty = cpunum_get_reg(0, CCPU_Y);
				}

				/* on the rising edge of the data value, latch the Y register */
				/* as 2-3-3 BGR values */
				if (data != last_control && data)
				{
					int yval = cpunum_get_reg(0, CCPU_Y);
					r = (~yval >> 0) & 0x07;
					r = r * 255 / 7;
					g = (~yval >> 3) & 0x07;
					g = g * 255 / 7;
					b = (~yval >> 6) & 0x03;
					b = b * 255 / 3;
					vector_color = MAKE_RGB(r,g,b);

					/* restore the original X,Y values */
					cpunum_set_reg(0, CCPU_X, lastx);
					cpunum_set_reg(0, CCPU_Y, lasty);
				}
			}
			break;
	}

	/* remember the last value */
	last_control = data;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( cinemat_bilevel )
{
	color_mode = COLOR_BILEVEL;
	video_start_vector(machine);
}


VIDEO_START( cinemat_16level )
{
	color_mode = COLOR_16LEVEL;
	video_start_vector(machine);
}


VIDEO_START( cinemat_64level )
{
	color_mode = COLOR_64LEVEL;
	video_start_vector(machine);
}


VIDEO_START( cinemat_color )
{
	color_mode = COLOR_RGB;
	video_start_vector(machine);
}


VIDEO_START( cinemat_qb3color )
{
	color_mode = COLOR_QB3;
	video_start_vector(machine);
}



/*************************************
 *
 *  End-of-frame
 *
 *************************************/

VIDEO_EOF( cinemat )
{
	vector_clear_list();
}



/*************************************
 *
 *  Space War update
 *
 *************************************/

VIDEO_UPDATE( spacewar )
{
	int sw_option = readinputportbytag("INPUTS");

	video_update_vector(machine, screen, bitmap, cliprect);

	/* set the state of the artwork */
	output_set_value("pressed3", (~sw_option >> 0) & 1);
	output_set_value("pressed8", (~sw_option >> 1) & 1);
	output_set_value("pressed4", (~sw_option >> 2) & 1);
	output_set_value("pressed9", (~sw_option >> 3) & 1);
	output_set_value("pressed1", (~sw_option >> 4) & 1);
	output_set_value("pressed6", (~sw_option >> 5) & 1);
	output_set_value("pressed2", (~sw_option >> 6) & 1);
	output_set_value("pressed7", (~sw_option >> 7) & 1);
	output_set_value("pressed5", (~sw_option >> 10) & 1);
	output_set_value("pressed0", (~sw_option >> 11) & 1);
	return 0;
}

