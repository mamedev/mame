// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cinematronics vector hardware

***************************************************************************/

#include "emu.h"
#include "cpu/ccpu/ccpu.h"
#include "includes/cinemat.h"


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
 *  Vector rendering
 *
 *************************************/

void cinemat_state::cinemat_vector_callback(INT16 sx, INT16 sy, INT16 ex, INT16 ey, UINT8 shift)
{
	const rectangle &visarea = m_screen->visible_area();
	int intensity = 0xff;

	/* adjust for slop */
	sx = sx - visarea.min_x;
	ex = ex - visarea.min_x;
	sy = sy - visarea.min_y;
	ey = ey - visarea.min_y;

	/* point intensity is determined by the shift value */
	if (sx == ex && sy == ey)
		intensity = 0x1ff * shift / 8;

	/* move to the starting position if we're not there already */
	if (sx != m_lastx || sy != m_lasty)
		m_vector->add_point(sx << 16, sy << 16, 0, 0);

	/* draw the vector */
	m_vector->add_point(ex << 16, ey << 16, m_vector_color, intensity);

	/* remember the last point */
	m_lastx = ex;
	m_lasty = ey;
}



/*************************************
 *
 *  Vector color handling
 *
 *************************************/

WRITE8_MEMBER(cinemat_state::cinemat_vector_control_w)
{
	int r, g, b, i;
	cpu_device *cpu = m_maincpu;

	switch (m_color_mode)
	{
		case COLOR_BILEVEL:
			/* color is either bright or dim, selected by the value sent to the port */
			m_vector_color = (data & 1) ? rgb_t(0x80,0x80,0x80) : rgb_t(0xff,0xff,0xff);
			break;

		case COLOR_16LEVEL:
			/* on the rising edge of the data value, latch bits 0-3 of the */
			/* X register as the intensity */
			if (data != m_last_control && data)
			{
				int xval = cpu->state_int(CCPU_X) & 0x0f;
				i = (xval + 1) * 255 / 16;
				m_vector_color = rgb_t(i,i,i);
			}
			break;

		case COLOR_64LEVEL:
			/* on the rising edge of the data value, latch bits 2-7 of the */
			/* X register as the intensity */
			if (data != m_last_control && data)
			{
				int xval = cpu->state_int(CCPU_X);
				xval = (~xval >> 2) & 0x3f;
				i = (xval + 1) * 255 / 64;
				m_vector_color = rgb_t(i,i,i);
			}
			break;

		case COLOR_RGB:
			/* on the rising edge of the data value, latch the X register */
			/* as 4-4-4 BGR values */
			if (data != m_last_control && data)
			{
				int xval = cpu->state_int(CCPU_X);
				r = (~xval >> 0) & 0x0f;
				r = r * 255 / 15;
				g = (~xval >> 4) & 0x0f;
				g = g * 255 / 15;
				b = (~xval >> 8) & 0x0f;
				b = b * 255 / 15;
				m_vector_color = rgb_t(r,g,b);
			}
			break;

		case COLOR_QB3:
			{
				/* on the falling edge of the data value, remember the original X,Y values */
				/* they will be restored on the rising edge; this is to simulate the fact */
				/* that the Rockola color hardware did not overwrite the beam X,Y position */
				/* on an IV instruction if data == 0 here */
				if (data != m_last_control && !data)
				{
					m_qb3_lastx = cpu->state_int(CCPU_X);
					m_qb3_lasty = cpu->state_int(CCPU_Y);
				}

				/* on the rising edge of the data value, latch the Y register */
				/* as 2-3-3 BGR values */
				if (data != m_last_control && data)
				{
					int yval = cpu->state_int(CCPU_Y);
					r = (~yval >> 0) & 0x07;
					r = r * 255 / 7;
					g = (~yval >> 3) & 0x07;
					g = g * 255 / 7;
					b = (~yval >> 6) & 0x03;
					b = b * 255 / 3;
					m_vector_color = rgb_t(r,g,b);

					/* restore the original X,Y values */
					cpu->set_state_int(CCPU_X, m_qb3_lastx);
					cpu->set_state_int(CCPU_Y, m_qb3_lasty);
				}
			}
			break;
	}

	/* remember the last value */
	m_last_control = data;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void cinemat_state::video_start()
{
	m_color_mode = COLOR_BILEVEL;
}


VIDEO_START_MEMBER(cinemat_state,cinemat_16level)
{
	m_color_mode = COLOR_16LEVEL;
}


VIDEO_START_MEMBER(cinemat_state,cinemat_64level)
{
	m_color_mode = COLOR_64LEVEL;
}


VIDEO_START_MEMBER(cinemat_state,cinemat_color)
{
	m_color_mode = COLOR_RGB;
}


VIDEO_START_MEMBER(cinemat_state,cinemat_qb3color)
{
	m_color_mode = COLOR_QB3;
}



/*************************************
 *
 *  End-of-frame
 *
 *************************************/

UINT32 cinemat_state::screen_update_cinemat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vector->screen_update(screen, bitmap, cliprect);
	m_vector->clear_list();

	m_maincpu->wdt_timer_trigger();

	return 0;
}



/*************************************
 *
 *  Space War update
 *
 *************************************/

UINT32 cinemat_state::screen_update_spacewar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int sw_option = ioport("INPUTS")->read();

	screen_update_cinemat(screen, bitmap, cliprect);

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
