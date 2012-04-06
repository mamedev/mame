#include <math.h>
#include "emu.h"
#include "includes/vectrex.h"
#include "video/vector.h"
#include "machine/6522via.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


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
	A_Y,
};


/*********************************************************************

   Prototypes

*********************************************************************/

static WRITE8_DEVICE_HANDLER (v_via_pa_w);
static WRITE8_DEVICE_HANDLER(v_via_pb_w);
static WRITE8_DEVICE_HANDLER (v_via_ca2_w);
static WRITE8_DEVICE_HANDLER (v_via_cb2_w);


/*********************************************************************

   Local variables

*********************************************************************/

const via6522_interface vectrex_via6522_interface =
{
	DEVCB_HANDLER(vectrex_via_pa_r), DEVCB_HANDLER(vectrex_via_pb_r),         /* read PA/B */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,                     /* read ca1, cb1, ca2, cb2 */
	DEVCB_HANDLER(v_via_pa_w), DEVCB_HANDLER(v_via_pb_w),         /* write PA/B */
	DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(v_via_ca2_w), DEVCB_HANDLER(v_via_cb2_w), /* write ca1, cb1, ca2, cb2 */
	DEVCB_LINE(vectrex_via_irq),                      /* IRQ */
};



/*********************************************************************

   Lightpen

*********************************************************************/

static TIMER_CALLBACK(lightpen_trigger)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	if (state->m_lightpen_port & 1)
	{
		via6522_device *via_0 = machine.device<via6522_device>("via6522_0");
		via_0->write_ca1(1);
		via_0->write_ca1(0);
	}

	if (state->m_lightpen_port & 2)
	{
		cputag_set_input_line(machine, "maincpu", M6809_FIRQ_LINE, PULSE_LINE);
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
	via6522_device *via = machine().device<via6522_device>("via6522_0");
	return via->read(space, offset);
}

WRITE8_MEMBER(vectrex_state::vectrex_via_w)
{
	via6522_device *via = machine().device<via6522_device>("via6522_0");
	attotime period;

	switch (offset)
	{
	case 8:
		m_via_timer2 = (m_via_timer2 & 0xff00) | data;
		break;

	case 9:
		m_via_timer2 = (m_via_timer2 & 0x00ff) | (data << 8);

		period = (attotime::from_hz(machine().device("maincpu")->unscaled_clock()) * m_via_timer2);

		if (m_reset_refresh)
			m_refresh->adjust(period, 0, period);
		else
			m_refresh->adjust(
								  min(period, m_refresh->remaining()),
								  0,
								  period);
		break;
	}
	via->write(space, offset, data);
}


/*********************************************************************

   Screen refresh

*********************************************************************/

static TIMER_CALLBACK(vectrex_refresh)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	/* Refresh only marks the range of vectors which will be drawn
     * during the next SCREEN_UPDATE_RGB32. */
	state->m_display_start = state->m_display_end;
	state->m_display_end = state->m_point_index;
}


SCREEN_UPDATE_RGB32(vectrex)
{
	vectrex_state *state = screen.machine().driver_data<vectrex_state>();
	int i;

	vectrex_configuration(screen.machine());

	/* start black */
	vector_add_point(screen.machine(),
					 state->m_points[state->m_display_start].x,
					 state->m_points[state->m_display_start].y,
					 state->m_points[state->m_display_start].col,
					 0);

	for (i = state->m_display_start; i != state->m_display_end; i = (i + 1) % NVECT)
	{
		vector_add_point(screen.machine(),
						 state->m_points[i].x,
						 state->m_points[i].y,
						 state->m_points[i].col,
						 state->m_points[i].intensity);
	}

	SCREEN_UPDATE32_CALL(vector);
	vector_clear_list();
	return 0;
}


/*********************************************************************

   Vector functions

*********************************************************************/

void vectrex_add_point(running_machine &machine, int x, int y, rgb_t color, int intensity)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	vectrex_point *newpoint;

	state->m_point_index = (state->m_point_index+1) % NVECT;
	newpoint = &state->m_points[state->m_point_index];

	newpoint->x = x;
	newpoint->y = y;
	newpoint->col = color;
	newpoint->intensity = intensity;
}


void vectrex_add_point_stereo(running_machine &machine, int x, int y, rgb_t color, int intensity)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	if (state->m_imager_status == 2) /* left = 1, right = 2 */
		vectrex_add_point(machine, (int)(y * M_SQRT1_2)+ state->m_x_center,
						   (int)(((state->m_x_max - x) * M_SQRT1_2)),
						   color,
						   intensity);
	else
		vectrex_add_point(machine, (int)(y * M_SQRT1_2),
						   (int)((state->m_x_max - x) * M_SQRT1_2),
						   color,
						   intensity);
}


static TIMER_CALLBACK(vectrex_zero_integrators)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	state->m_x_int = state->m_x_center + (state->m_analog[A_ZR] * INT_PER_CLOCK);
	state->m_y_int = state->m_y_center + (state->m_analog[A_ZR] * INT_PER_CLOCK);
	(*state->vector_add_point_function)(machine, state->m_x_int, state->m_y_int, state->m_beam_color, 0);
}


/*********************************************************************

   Delayed signals

   The RAMP signal is delayed wrt. beam blanking. Getting this right
   is important for text placement, the maze in Clean Sweep and many
   other games.

*********************************************************************/

static TIMER_CALLBACK(update_signal)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	int length;

	if (!state->m_ramp)
	{
		length = machine.device("maincpu")->unscaled_clock() * INT_PER_CLOCK
			* (machine.time() - state->m_vector_start_time).as_double();

		state->m_x_int += length * (state->m_analog[A_X] + state->m_analog[A_ZR]);
		state->m_y_int += length * (state->m_analog[A_Y] + state->m_analog[A_ZR]);

		(*state->vector_add_point_function)(machine, state->m_x_int, state->m_y_int, state->m_beam_color, 2 * state->m_analog[A_Z] * state->m_blank);
	}
	else
	{
		if (state->m_blank)
			(*state->vector_add_point_function)(machine, state->m_x_int, state->m_y_int, state->m_beam_color, 2 * state->m_analog[A_Z]);
	}

	state->m_vector_start_time = machine.time();

	if (ptr)
		* (UINT8 *) ptr = param;
}


/*********************************************************************

   Startup

*********************************************************************/

VIDEO_START(vectrex)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	screen_device *screen = machine.first_screen();
	const rectangle &visarea = screen->visible_area();

	state->m_x_center=(visarea.width() / 2) << 16;
	state->m_y_center=(visarea.height() / 2) << 16;
	state->m_x_max = visarea.max_x << 16;
	state->m_y_max = visarea.max_y << 16;

	state->m_imager_freq = 1;

	state->vector_add_point_function = vectrex_add_point;
	state->m_imager_timer = machine.scheduler().timer_alloc(FUNC(vectrex_imager_eye));
	state->m_imager_timer->adjust(
						  attotime::from_hz(state->m_imager_freq),
						  2,
						  attotime::from_hz(state->m_imager_freq));

	state->m_lp_t = machine.scheduler().timer_alloc(FUNC(lightpen_trigger));

	state->m_refresh = machine.scheduler().timer_alloc(FUNC(vectrex_refresh));

	VIDEO_START_CALL(vector);
}


/*********************************************************************

   VIA interface functions

*********************************************************************/

static void vectrex_multiplexer(running_machine &machine, int mux)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	device_t *dac_device = machine.device("dac");

	machine.scheduler().timer_set(attotime::from_nsec(ANALOG_DELAY), FUNC(update_signal), state->m_via_out[PORTA], &state->m_analog[mux]);

	if (mux == A_AUDIO)
		dac_data_w(dac_device, state->m_via_out[PORTA]);
}


static WRITE8_DEVICE_HANDLER(v_via_pb_w)
{
	vectrex_state *state = device->machine().driver_data<vectrex_state>();
	if (!(data & 0x80))
	{
		/* RAMP is active */
		if ((state->m_ramp & 0x80))
		{
			/* RAMP was inactive before */

			if (state->m_lightpen_down)
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
				ab = (state->m_pen_x - state->m_x_int) * state->m_analog[A_X]
					+(state->m_pen_y - state->m_y_int) * state->m_analog[A_Y];
				if (ab > 0)
				{
					a2 = (double)(state->m_analog[A_X] * state->m_analog[A_X]
								  +(double)state->m_analog[A_Y] * state->m_analog[A_Y]);
					b2 = (double)(state->m_pen_x - state->m_x_int) * (state->m_pen_x - state->m_x_int)
						+(double)(state->m_pen_y - state->m_y_int) * (state->m_pen_y - state->m_y_int);
					d2 = b2 - ab * ab / a2;
					if (d2 < 2e10 && state->m_analog[A_Z] * state->m_blank > 0)
						state->m_lp_t->adjust(attotime::from_double(ab / a2 / (device->machine().device("maincpu")->unscaled_clock() * INT_PER_CLOCK)));
				}
			}
		}

		if (!(data & 0x1) && (state->m_via_out[PORTB] & 0x1))
		{
			/* MUX has been enabled */
			device->machine().scheduler().timer_set(attotime::from_nsec(ANALOG_DELAY), FUNC(update_signal));
		}
	}
	else
	{
		/* RAMP is inactive */
		if (!(state->m_ramp & 0x80))
		{
			/* Cancel running timer, line already finished */
			if (state->m_lightpen_down)
				state->m_lp_t->adjust(attotime::never);
		}
	}

	/* Sound */
	if (data & 0x10)
	{
		device_t *ay8912 = device->machine().device("ay8912");

		if (data & 0x08) /* BC1 (do we select a reg or write it ?) */
			ay8910_address_w(ay8912, 0, state->m_via_out[PORTA]);
		else
			ay8910_data_w(ay8912, 0, state->m_via_out[PORTA]);
	}

	if (!(data & 0x1) && (state->m_via_out[PORTB] & 0x1))
		vectrex_multiplexer (device->machine(), (data >> 1) & 0x3);

	state->m_via_out[PORTB] = data;
	device->machine().scheduler().timer_set(attotime::from_nsec(ANALOG_DELAY), FUNC(update_signal), data & 0x80, &state->m_ramp);
}


static WRITE8_DEVICE_HANDLER(v_via_pa_w)
{
	vectrex_state *state = device->machine().driver_data<vectrex_state>();
	/* DAC output always goes to Y integrator */
	state->m_via_out[PORTA] = data;
	device->machine().scheduler().timer_set(attotime::from_nsec(ANALOG_DELAY), FUNC(update_signal), data, &state->m_analog[A_Y]);

	if (!(state->m_via_out[PORTB] & 0x1))
		vectrex_multiplexer (device->machine(), (state->m_via_out[PORTB] >> 1) & 0x3);
}


static WRITE8_DEVICE_HANDLER(v_via_ca2_w)
{
	if (data == 0)
		device->machine().scheduler().timer_set(attotime::from_nsec(ANALOG_DELAY), FUNC(vectrex_zero_integrators));
}


static WRITE8_DEVICE_HANDLER(v_via_cb2_w)
{
	vectrex_state *state = device->machine().driver_data<vectrex_state>();
	int dx, dy;

	if (state->m_cb2 != data)
	{

		/* Check lightpen */
		if (state->m_lightpen_port != 0)
		{
			state->m_lightpen_down = input_port_read(device->machine(), "LPENCONF") & 0x10;

			if (state->m_lightpen_down)
			{
				state->m_pen_x = input_port_read(device->machine(), "LPENX") * (state->m_x_max / 0xff);
				state->m_pen_y = input_port_read(device->machine(), "LPENY") * (state->m_y_max / 0xff);

				dx = abs(state->m_pen_x - state->m_x_int);
				dy = abs(state->m_pen_y - state->m_y_int);
				if (dx < 500000 && dy < 500000 && data > 0)
					device->machine().scheduler().timer_set(attotime::zero, FUNC(lightpen_trigger));
			}
		}

		device->machine().scheduler().timer_set(attotime::zero, FUNC(update_signal), data, &state->m_blank);
		state->m_cb2 = data;
	}
}


/*****************************************************************

   RA+A Spectrum I+

*****************************************************************/

const via6522_interface spectrum1_via6522_interface =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_HANDLER(vectrex_via_pa_r), DEVCB_HANDLER(vectrex_s1_via_pb_r), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B1,CA/B2 */ DEVCB_HANDLER(v_via_pa_w), DEVCB_HANDLER(v_via_pb_w), DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(v_via_ca2_w), DEVCB_HANDLER(v_via_cb2_w),
	/*irq                      */ DEVCB_LINE(vectrex_via_irq),
};


WRITE8_MEMBER(vectrex_state::raaspec_led_w)
{
	logerror("Spectrum I+ LED: %i%i%i%i%i%i%i%i\n",
			 (data>>7)&0x1, (data>>6)&0x1, (data>>5)&0x1, (data>>4)&0x1,
			 (data>>3)&0x1, (data>>2)&0x1, (data>>1)&0x1, data&0x1);
}


VIDEO_START(raaspec)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	screen_device *screen = machine.first_screen();
	const rectangle &visarea = screen->visible_area();

	state->m_x_center=(visarea.width() / 2) << 16;
	state->m_y_center=(visarea.height() / 2) << 16;
	state->m_x_max = visarea.max_x << 16;
	state->m_y_max = visarea.max_y << 16;

	state->vector_add_point_function = vectrex_add_point;
	state->m_refresh = machine.scheduler().timer_alloc(FUNC(vectrex_refresh));

	VIDEO_START_CALL(vector);
}
