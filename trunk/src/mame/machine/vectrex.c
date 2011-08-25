#include "emu.h"
#include "video/vector.h"
#include "machine/6522via.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "image.h"

#include "includes/vectrex.h"


#define VC_RED      MAKE_RGB(0xff, 0x00, 0x00)
#define VC_GREEN    MAKE_RGB(0x00, 0xff, 0x00)
#define VC_BLUE     MAKE_RGB(0x00, 0x00, 0xff)
#define VC_DARKRED  MAKE_RGB(0x80, 0x00, 0x00)

#define DAMPC (-0.2)
#define MMI (5.0)

enum {
	PORTB = 0,
	PORTA
};


/*********************************************************************

   Global variables

*********************************************************************/



/*********************************************************************

   Local variables

*********************************************************************/

/* Colors for right and left eye */

/* Starting points of the three colors */
/* Values taken from J. Nelson's drawings*/

static const double minestorm_3d_angles[3] = {0, 0.1692, 0.2086};
static const double narrow_escape_angles[3] = {0, 0.1631, 0.3305};
static const double crazy_coaster_angles[3] = {0, 0.1631, 0.3305};


static const double unknown_game_angles[3] = {0,0.16666666, 0.33333333};


static int vectrex_verify_cart(char *data)
{
	/* Verify the file is accepted by the Vectrex bios */
	if (!memcmp(data,"g GCE", 5))
		return IMAGE_VERIFY_PASS;
	else
		return IMAGE_VERIFY_FAIL;
}


/*********************************************************************

   ROM load and id functions

*********************************************************************/

DEVICE_IMAGE_LOAD(vectrex_cart)
{
	vectrex_state *state = image.device().machine().driver_data<vectrex_state>();
	UINT8 *mem = image.device().machine().region("maincpu")->base();
	if (image.software_entry() == NULL)
	{
		image.fread( mem, 0x8000);
	} else {
		int size = image.get_software_region_length("rom");
		memcpy(mem, image.get_software_region("rom"), size);
	}

	/* check image! */
	if (vectrex_verify_cart((char*)mem) == IMAGE_VERIFY_FAIL)
	{
		logerror("Invalid image!\n");
		return IMAGE_INIT_FAIL;
	}

	/* If VIA T2 starts, reset refresh timer.
       This is the best strategy for most games. */
	state->m_reset_refresh = 1;

	state->m_imager_angles = narrow_escape_angles;

	/* let's do this 3D detection with a strcmp using data inside the cart images */
	/* slightly prettier than having to hardcode CRCs */

	/* handle 3D Narrow Escape but skip the 2-d hack of it from Fred Taft */
	if (!memcmp(mem + 0x11,"NARROW",6) && (((char*)mem)[0x39] == 0x0c))
	{
		state->m_imager_angles = narrow_escape_angles;
	}

	if (!memcmp(mem + 0x11,"CRAZY COASTER", 13))
	{
		state->m_imager_angles = crazy_coaster_angles;
	}

	if (!memcmp(mem + 0x11,"3D MINE STORM", 13))
	{
		state->m_imager_angles = minestorm_3d_angles;

		/* Don't reset T2 each time it's written.
           This would cause jerking in mine3. */
		state->m_reset_refresh = 0;
	}

	return IMAGE_INIT_PASS;
}


/*********************************************************************

   Vectrex configuration (mainly 3D Imager)

*********************************************************************/

void vectrex_configuration(running_machine &machine)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	unsigned char cport = input_port_read(machine, "3DCONF");

	/* Vectrex 'dipswitch' configuration */

	/* Imager control */
	if (cport & 0x01) /* Imager enabled */
	{
		if (state->m_imager_status == 0)
			state->m_imager_status = cport & 0x01;

		state->vector_add_point_function = cport & 0x02 ? vectrex_add_point_stereo: vectrex_add_point;

		switch ((cport >> 2) & 0x07)
		{
		case 0x00:
			state->m_imager_colors[0] = state->m_imager_colors[1] = state->m_imager_colors[2] = RGB_BLACK;
			break;
		case 0x01:
			state->m_imager_colors[0] = state->m_imager_colors[1] = state->m_imager_colors[2] = VC_DARKRED;
			break;
		case 0x02:
			state->m_imager_colors[0] = state->m_imager_colors[1] = state->m_imager_colors[2] = VC_GREEN;
			break;
		case 0x03:
			state->m_imager_colors[0] = state->m_imager_colors[1] = state->m_imager_colors[2] = VC_BLUE;
			break;
		case 0x04:
			/* mine3 has a different color sequence */
			if (state->m_imager_angles == minestorm_3d_angles)
			{
				state->m_imager_colors[0] = VC_GREEN;
				state->m_imager_colors[1] = VC_RED;
			}
			else
			{
				state->m_imager_colors[0] = VC_RED;
				state->m_imager_colors[1] = VC_GREEN;
			}
			state->m_imager_colors[2]=VC_BLUE;
			break;
		}

		switch ((cport >> 5) & 0x07)
		{
		case 0x00:
			state->m_imager_colors[3] = state->m_imager_colors[4] = state->m_imager_colors[5] = RGB_BLACK;
			break;
		case 0x01:
			state->m_imager_colors[3] = state->m_imager_colors[4] = state->m_imager_colors[5] = VC_DARKRED;
			break;
		case 0x02:
			state->m_imager_colors[3] = state->m_imager_colors[4] = state->m_imager_colors[5] = VC_GREEN;
			break;
		case 0x03:
			state->m_imager_colors[3] = state->m_imager_colors[4] = state->m_imager_colors[5] = VC_BLUE;
			break;
		case 0x04:
			if (state->m_imager_angles == minestorm_3d_angles)
			{
				state->m_imager_colors[3] = VC_GREEN;
				state->m_imager_colors[4] = VC_RED;
			}
			else
			{
				state->m_imager_colors[3] = VC_RED;
				state->m_imager_colors[4] = VC_GREEN;
			}
			state->m_imager_colors[5]=VC_BLUE;
			break;
		}
	}
	else
	{
		state->vector_add_point_function = vectrex_add_point;
		state->m_beam_color = RGB_WHITE;
		state->m_imager_colors[0] = state->m_imager_colors[1] = state->m_imager_colors[2] = state->m_imager_colors[3] = state->m_imager_colors[4] = state->m_imager_colors[5] = RGB_WHITE;
	}
	state->m_lightpen_port = input_port_read(machine, "LPENCONF") & 0x03;
}


/*********************************************************************

   VIA interface functions

*********************************************************************/

void vectrex_via_irq(device_t *device, int level)
{
	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, level);
}


READ8_DEVICE_HANDLER(vectrex_via_pb_r)
{
	vectrex_state *state = device->machine().driver_data<vectrex_state>();
	int pot;
	static const char *const ctrlnames[] = { "CONTR1X", "CONTR1Y", "CONTR2X", "CONTR2Y" };

	pot = input_port_read(device->machine(), ctrlnames[(state->m_via_out[PORTB] & 0x6) >> 1]) - 0x80;

	if (pot > (signed char)state->m_via_out[PORTA])
		state->m_via_out[PORTB] |= 0x20;
	else
		state->m_via_out[PORTB] &= ~0x20;

	return state->m_via_out[PORTB];
}


READ8_DEVICE_HANDLER(vectrex_via_pa_r)
{
	vectrex_state *state = device->machine().driver_data<vectrex_state>();
	if ((!(state->m_via_out[PORTB] & 0x10)) && (state->m_via_out[PORTB] & 0x08))
		/* BDIR inactive, we can read the PSG. BC1 has to be active. */
	{
		device_t *ay = device->machine().device("ay8912");

		state->m_via_out[PORTA] = ay8910_r(ay, 0)
			& ~(state->m_imager_pinlevel & 0x80);
	}
	return state->m_via_out[PORTA];
}


READ8_DEVICE_HANDLER(vectrex_s1_via_pb_r)
{
	vectrex_state *state = device->machine().driver_data<vectrex_state>();
	return (state->m_via_out[PORTB] & ~0x40) | (input_port_read(device->machine(), "COIN") & 0x40);
}


/*********************************************************************

   3D Imager support

*********************************************************************/

static TIMER_CALLBACK(vectrex_imager_change_color)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	state->m_beam_color = param;
}


static TIMER_CALLBACK(update_level)
{
	if (ptr)
		* (UINT8 *) ptr = param;
}


TIMER_CALLBACK(vectrex_imager_eye)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	via6522_device *via_0 = machine.device<via6522_device>("via6522_0");
	int coffset;
	double rtime = (1.0 / state->m_imager_freq);

	if (state->m_imager_status > 0)
	{
		state->m_imager_status = param;
		coffset = param > 1? 3: 0;
		machine.scheduler().timer_set (attotime::from_double(rtime * state->m_imager_angles[0]), FUNC(vectrex_imager_change_color), state->m_imager_colors[coffset+2]);
		machine.scheduler().timer_set (attotime::from_double(rtime * state->m_imager_angles[1]), FUNC(vectrex_imager_change_color), state->m_imager_colors[coffset+1]);
		machine.scheduler().timer_set (attotime::from_double(rtime * state->m_imager_angles[2]), FUNC(vectrex_imager_change_color), state->m_imager_colors[coffset]);

		if (param == 2)
		{
			machine.scheduler().timer_set (attotime::from_double(rtime * 0.50), FUNC(vectrex_imager_eye), 1);

			/* Index hole sensor is connected to IO7 which triggers also CA1 of VIA */
			via_0->write_ca1(1);
			via_0->write_ca1(0);
			state->m_imager_pinlevel |= 0x80;
			machine.scheduler().timer_set (attotime::from_double(rtime / 360.0), FUNC(update_level), 0, &state->m_imager_pinlevel);
		}
	}
}


WRITE8_HANDLER(vectrex_psg_port_w)
{
	vectrex_state *state = space->machine().driver_data<vectrex_state>();
	double wavel, ang_acc, tmp;
	int mcontrol;

	mcontrol = data & 0x40; /* IO6 controls the imager motor */

	if (!mcontrol && mcontrol ^ state->m_old_mcontrol)
	{
		state->m_old_mcontrol = mcontrol;
		tmp = space->machine().time().as_double();
		wavel = tmp - state->m_sl;
		state->m_sl = tmp;

		if (wavel < 1)
		{
			/* The Vectrex sends a stream of pulses which control the speed of
               the motor using Pulse Width Modulation. Guessed parameters are MMI
               (mass moment of inertia) of the color wheel, DAMPC (damping coefficient)
               of the whole thing and some constants of the motor's torque/speed curve.
               pwl is the negative pulse width and wavel is the whole wavelength. */

			ang_acc = (50.0 - 1.55 * state->m_imager_freq) / MMI;
			state->m_imager_freq += ang_acc * state->m_pwl + DAMPC * state->m_imager_freq / MMI * wavel;

			if (state->m_imager_freq > 1)
			{
				state->m_imager_timer->adjust(
									  attotime::from_double(MIN(1.0 / state->m_imager_freq, state->m_imager_timer->remaining().as_double())),
									  2,
									  attotime::from_double(1.0 / state->m_imager_freq));
			}
		}
	}
	if (mcontrol && mcontrol ^ state->m_old_mcontrol)
	{
		state->m_old_mcontrol = mcontrol;
		state->m_pwl = space->machine().time().as_double() - state->m_sl;
	}
}

DRIVER_INIT(vectrex)
{
	vectrex_state *state = machine.driver_data<vectrex_state>();
	int i;

	state->m_imager_angles = unknown_game_angles;
	state->m_beam_color = RGB_WHITE;
	for (i=0; i<ARRAY_LENGTH(state->m_imager_colors); i++)
		state->m_imager_colors[i] = RGB_WHITE;

	/*
     * Uninitialized RAM needs to return 0xff. Otherwise the mines in
     * the first level of Minestorm are not evenly distributed.
     */

	memset(state->m_gce_vectorram, 0xff, state->m_gce_vectorram_size);
}
