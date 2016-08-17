// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
#include <math.h>
#include "emu.h"
#include "includes/vectrex.h"
#include "cpu/m6809/m6809.h"


#define ANALOG_DELAY 7800

#define INT_PER_CLOCK 550

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif

/*********************************************************************

   Enums and typedefs

*********************************************************************/

enum {
	PORTB = 0,
	PORTA
};

enum {
	A_X = 0,
	A_ZR,
	A_Z,
	A_AUDIO,
	A_Y
};

/*********************************************************************

   Lightpen

*********************************************************************/

TIMER_CALLBACK_MEMBER(vectrex_state::lightpen_trigger)
{
	if (m_lightpen_port & 1)
	{
		m_via6522_0->write_ca1(1);
		m_via6522_0->write_ca1(0);
	}

	if (m_lightpen_port & 2)
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, PULSE_LINE);
	}
}


/*********************************************************************

   VIA T2 configuration

   We need to snoop the frequency of VIA timer 2 here since most
   vectrex games use that timer for steady screen refresh. Even if the
   game stops T2 we continue refreshing the screen with the last known
   frequency. Note that we quickly get out of sync in this case and the
   screen will start flickering (see cut scenes in Spike).

   Note that the timer can be adjusted to the full period each time T2
   is restarted. This behaviour avoids flicker in most games. Some
   games like mine 3d don't work well with this scheme though and show
   severe jerking. So the second option is to leave the current period
   alone (if the new period isn't shorter) and change only the next
   full period.

*********************************************************************/

READ8_MEMBER(vectrex_state::vectrex_via_r)
{
	return m_via6522_0->read(space, offset);
}

WRITE8_MEMBER(vectrex_state::vectrex_via_w)
{
	attotime period;

	switch (offset)
	{
	case 8:
		m_via_timer2 = (m_via_timer2 & 0xff00) | data;
		break;

	case 9:
		m_via_timer2 = (m_via_timer2 & 0x00ff) | (data << 8);

		period = (attotime::from_hz(m_maincpu->unscaled_clock()) * m_via_timer2);

		if (m_reset_refresh)
			m_refresh->adjust(period, 0, period);
		else
			m_refresh->adjust(
									min(period, m_refresh->remaining()),
									0,
									period);
		break;
	}
	m_via6522_0->write(space, offset, data);
}


/*********************************************************************

   Screen refresh

*********************************************************************/

TIMER_CALLBACK_MEMBER(vectrex_state::vectrex_refresh)
{
	/* Refresh only marks the range of vectors which will be drawn
	 * during the next screen_update. */
	m_display_start = m_display_end;
	m_display_end = m_point_index;
}


UINT32 vectrex_state::screen_update_vectrex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i;

	vectrex_configuration();

	/* start black */
	m_vector->add_point(m_points[m_display_start].x,
						m_points[m_display_start].y,
						m_points[m_display_start].col,
						0);

	for (i = m_display_start; i != m_display_end; i = (i + 1) % NVECT)
	{
		m_vector->add_point(m_points[i].x,
							m_points[i].y,
							m_points[i].col,
							m_points[i].intensity);
	}

	m_vector->screen_update(screen, bitmap, cliprect);
	m_vector->clear_list();
	return 0;
}


/*********************************************************************

   Vector functions

*********************************************************************/

void vectrex_state::vectrex_add_point(int x, int y, rgb_t color, int intensity)
{
	vectrex_point *newpoint;

	m_point_index = (m_point_index+1) % NVECT;
	newpoint = &m_points[m_point_index];

	newpoint->x = x;
	newpoint->y = y;
	newpoint->col = color;
	newpoint->intensity = intensity;
}


void vectrex_state::vectrex_add_point_stereo(int x, int y, rgb_t color, int intensity)
{
	if (m_imager_status == 2) /* left = 1, right = 2 */
		vectrex_add_point((int)(y * M_SQRT1_2)+ m_x_center,
							(int)(((m_x_max - x) * M_SQRT1_2)),
							color,
							intensity);
	else
		vectrex_add_point((int)(y * M_SQRT1_2),
							(int)((m_x_max - x) * M_SQRT1_2),
							color,
							intensity);
}


TIMER_CALLBACK_MEMBER(vectrex_state::vectrex_zero_integrators)
{
	m_x_int = m_x_center + (m_analog[A_ZR] * INT_PER_CLOCK);
	m_y_int = m_y_center + (m_analog[A_ZR] * INT_PER_CLOCK);
	(this->*vector_add_point_function)(m_x_int, m_y_int, m_beam_color, 0);
}


/*********************************************************************

   Delayed signals

   The RAMP signal is delayed wrt. beam blanking. Getting this right
   is important for text placement, the maze in Clean Sweep and many
   other games.

*********************************************************************/

TIMER_CALLBACK_MEMBER(vectrex_state::update_signal)
{
	int length;

	if (!m_ramp)
	{
		length = m_maincpu->unscaled_clock() * INT_PER_CLOCK
			* (machine().time() - m_vector_start_time).as_double();

		m_x_int += length * (m_analog[A_X] + m_analog[A_ZR]);
		m_y_int += length * (m_analog[A_Y] + m_analog[A_ZR]);

		(this->*vector_add_point_function)(m_x_int, m_y_int, m_beam_color, 2 * m_analog[A_Z] * m_blank);
	}
	else
	{
		if (m_blank)
			(this->*vector_add_point_function)(m_x_int, m_y_int, m_beam_color, 2 * m_analog[A_Z]);
	}

	m_vector_start_time = machine().time();

	if (ptr)
		* (UINT8 *) ptr = param;
}


/*********************************************************************

   Startup

*********************************************************************/

void vectrex_state::video_start()
{
	const rectangle &visarea = m_screen->visible_area();

	m_x_center=(visarea.width() / 2) << 16;
	m_y_center=(visarea.height() / 2) << 16;
	m_x_max = visarea.max_x << 16;
	m_y_max = visarea.max_y << 16;

	m_imager_freq = 1;

	vector_add_point_function = &vectrex_state::vectrex_add_point;
	m_imager_timer = timer_alloc(TIMER_VECTREX_IMAGER_EYE);
	m_imager_timer->adjust(attotime::from_hz(m_imager_freq), 2, attotime::from_hz(m_imager_freq));

	m_lp_t = timer_alloc(TIMER_LIGHTPEN_TRIGGER);

	m_refresh = timer_alloc(TIMER_VECTREX_REFRESH);
}


/*********************************************************************

   VIA interface functions

*********************************************************************/

void vectrex_state::vectrex_multiplexer(int mux)
{
	timer_set(attotime::from_nsec(ANALOG_DELAY), TIMER_UPDATE_SIGNAL, m_via_out[PORTA], &m_analog[mux]);

	if (mux == A_AUDIO)
		m_dac->write_unsigned8(m_via_out[PORTA]);
}


WRITE8_MEMBER(vectrex_state::v_via_pb_w)
{
	if (!(data & 0x80))
	{
		/* RAMP is active */
		if ((m_ramp & 0x80))
		{
			/* RAMP was inactive before */

			if (m_lightpen_down)
			{
				/* Simple lin. algebra to check if pen is near
				 * the line defined by (A_X,A_Y).
				 * If that is the case, set a timer which goes
				 * off when the beam reaches the pen. Exact
				 * timing is important here.
				 *
				 *    lightpen
				 *       ^
				 *  _   /|
				 *  b  / |
				 *    /  |
				 *   /   |d
				 *  /    |
				 * /     |
				 * ------+---------> beam path
				 *    l  |    _
				 *            a
				 */
				double a2, b2, ab, d2;
				ab = (m_pen_x - m_x_int) * m_analog[A_X]
					+(m_pen_y - m_y_int) * m_analog[A_Y];
				if (ab > 0)
				{
					a2 = (double)(m_analog[A_X] * m_analog[A_X]
									+(double)m_analog[A_Y] * m_analog[A_Y]);
					b2 = (double)(m_pen_x - m_x_int) * (m_pen_x - m_x_int)
						+(double)(m_pen_y - m_y_int) * (m_pen_y - m_y_int);
					d2 = b2 - ab * ab / a2;
					if (d2 < 2e10 && m_analog[A_Z] * m_blank > 0)
						m_lp_t->adjust(attotime::from_double(ab / a2 / (m_maincpu->unscaled_clock() * INT_PER_CLOCK)));
				}
			}
		}

		if (!(data & 0x1) && (m_via_out[PORTB] & 0x1))
		{
			/* MUX has been enabled */
			timer_set(attotime::from_nsec(ANALOG_DELAY), TIMER_UPDATE_SIGNAL);
		}
	}
	else
	{
		/* RAMP is inactive */
		if (!(m_ramp & 0x80))
		{
			/* Cancel running timer, line already finished */
			if (m_lightpen_down)
				m_lp_t->adjust(attotime::never);
		}
	}

	/* Cartridge bank-switching */
	if (m_cart && ((data ^ m_via_out[PORTB]) & 0x40))
		m_cart->write_bank(space, 0, data);

	/* Sound */
	if (data & 0x10)
	{
		if (data & 0x08) /* BC1 (do we select a reg or write it ?) */
			m_ay8912->address_w(space, 0, m_via_out[PORTA]);
		else
			m_ay8912->data_w(space, 0, m_via_out[PORTA]);
	}

	if (!(data & 0x1) && (m_via_out[PORTB] & 0x1))
		vectrex_multiplexer((data >> 1) & 0x3);

	m_via_out[PORTB] = data;
	timer_set(attotime::from_nsec(ANALOG_DELAY), TIMER_UPDATE_SIGNAL, data & 0x80, &m_ramp);
}


WRITE8_MEMBER(vectrex_state::v_via_pa_w)
{
	/* DAC output always goes to Y integrator */
	m_via_out[PORTA] = data;
	timer_set(attotime::from_nsec(ANALOG_DELAY), TIMER_UPDATE_SIGNAL, data, &m_analog[A_Y]);

	if (!(m_via_out[PORTB] & 0x1))
		vectrex_multiplexer((m_via_out[PORTB] >> 1) & 0x3);
}


WRITE_LINE_MEMBER(vectrex_state::v_via_ca2_w)
{
	if (state == 0)
		timer_set(attotime::from_nsec(ANALOG_DELAY), TIMER_VECTREX_ZERO_INTEGRATORS);
}


WRITE_LINE_MEMBER(vectrex_state::v_via_cb2_w)
{
	int dx, dy;

	if (m_cb2 != state)
	{
		/* Check lightpen */
		if (m_lightpen_port != 0)
		{
			m_lightpen_down = ioport("LPENCONF")->read() & 0x10;

			if (m_lightpen_down)
			{
				m_pen_x = ioport("LPENX")->read() * (m_x_max / 0xff);
				m_pen_y = ioport("LPENY")->read() * (m_y_max / 0xff);

				dx = abs(m_pen_x - m_x_int);
				dy = abs(m_pen_y - m_y_int);
				if (dx < 500000 && dy < 500000 && state > 0)
					timer_set(attotime::zero, TIMER_LIGHTPEN_TRIGGER);
			}
		}

		timer_set(attotime::zero, TIMER_UPDATE_SIGNAL, state, &m_blank);
		m_cb2 = state;
	}
}


/*****************************************************************

   RA+A Spectrum I+

*****************************************************************/

WRITE8_MEMBER(vectrex_state::raaspec_led_w)
{
	logerror("Spectrum I+ LED: %i%i%i%i%i%i%i%i\n",
				(data>>7)&0x1, (data>>6)&0x1, (data>>5)&0x1, (data>>4)&0x1,
				(data>>3)&0x1, (data>>2)&0x1, (data>>1)&0x1, data&0x1);
}


VIDEO_START_MEMBER(vectrex_state,raaspec)
{
	const rectangle &visarea = m_screen->visible_area();

	m_x_center=(visarea.width() / 2) << 16;
	m_y_center=(visarea.height() / 2) << 16;
	m_x_max = visarea.max_x << 16;
	m_y_max = visarea.max_y << 16;

	vector_add_point_function = &vectrex_state::vectrex_add_point;
	m_refresh = timer_alloc(TIMER_VECTREX_REFRESH);
}
