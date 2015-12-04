// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"

#include "includes/vectrex.h"


#define VC_RED      rgb_t(0xff, 0x00, 0x00)
#define VC_GREEN    rgb_t(0x00, 0xff, 0x00)
#define VC_BLUE     rgb_t(0x00, 0x00, 0xff)
#define VC_DARKRED  rgb_t(0x80, 0x00, 0x00)

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



void vectrex_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_VECTREX_IMAGER_CHANGE_COLOR:
		vectrex_imager_change_color(ptr, param);
		break;
	case TIMER_UPDATE_LEVEL:
		update_level(ptr, param);
		break;
	case TIMER_VECTREX_IMAGER_EYE:
		vectrex_imager_eye(ptr, param);
		break;
	case TIMER_LIGHTPEN_TRIGGER:
		lightpen_trigger(ptr, param);
		break;
	case TIMER_VECTREX_REFRESH:
		vectrex_refresh(ptr, param);
		break;
	case TIMER_VECTREX_ZERO_INTEGRATORS:
		vectrex_zero_integrators(ptr, param);
		break;
	case TIMER_UPDATE_SIGNAL:
		update_signal(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in vectrex_state::device_timer");
	}
}



/*********************************************************************

   Vectrex configuration (mainly 3D Imager)

*********************************************************************/

void vectrex_state::vectrex_configuration()
{
	unsigned char cport = m_io_3dconf->read();

	/* Vectrex 'dipswitch' configuration */

	/* Imager control */
	if (cport & 0x01) /* Imager enabled */
	{
		if (m_imager_status == 0)
			m_imager_status = cport & 0x01;

		vector_add_point_function = cport & 0x02 ? &vectrex_state::vectrex_add_point_stereo: &vectrex_state::vectrex_add_point;

		switch ((cport >> 2) & 0x07)
		{
		case 0x00:
			m_imager_colors[0] = m_imager_colors[1] = m_imager_colors[2] = rgb_t::black;
			break;
		case 0x01:
			m_imager_colors[0] = m_imager_colors[1] = m_imager_colors[2] = VC_DARKRED;
			break;
		case 0x02:
			m_imager_colors[0] = m_imager_colors[1] = m_imager_colors[2] = VC_GREEN;
			break;
		case 0x03:
			m_imager_colors[0] = m_imager_colors[1] = m_imager_colors[2] = VC_BLUE;
			break;
		case 0x04:
			/* mine3 has a different color sequence */
			if (m_imager_angles == minestorm_3d_angles)
			{
				m_imager_colors[0] = VC_GREEN;
				m_imager_colors[1] = VC_RED;
			}
			else
			{
				m_imager_colors[0] = VC_RED;
				m_imager_colors[1] = VC_GREEN;
			}
			m_imager_colors[2]=VC_BLUE;
			break;
		}

		switch ((cport >> 5) & 0x07)
		{
		case 0x00:
			m_imager_colors[3] = m_imager_colors[4] = m_imager_colors[5] = rgb_t::black;
			break;
		case 0x01:
			m_imager_colors[3] = m_imager_colors[4] = m_imager_colors[5] = VC_DARKRED;
			break;
		case 0x02:
			m_imager_colors[3] = m_imager_colors[4] = m_imager_colors[5] = VC_GREEN;
			break;
		case 0x03:
			m_imager_colors[3] = m_imager_colors[4] = m_imager_colors[5] = VC_BLUE;
			break;
		case 0x04:
			if (m_imager_angles == minestorm_3d_angles)
			{
				m_imager_colors[3] = VC_GREEN;
				m_imager_colors[4] = VC_RED;
			}
			else
			{
				m_imager_colors[3] = VC_RED;
				m_imager_colors[4] = VC_GREEN;
			}
			m_imager_colors[5]=VC_BLUE;
			break;
		}
	}
	else
	{
		vector_add_point_function = &vectrex_state::vectrex_add_point;
		m_beam_color = rgb_t::white;
		m_imager_colors[0] = m_imager_colors[1] = m_imager_colors[2] = m_imager_colors[3] = m_imager_colors[4] = m_imager_colors[5] = rgb_t::white;
	}
	m_lightpen_port = m_io_lpenconf->read() & 0x03;
}


/*********************************************************************

   VIA interface functions

*********************************************************************/

WRITE_LINE_MEMBER(vectrex_state::vectrex_via_irq)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, state);
}


READ8_MEMBER(vectrex_state::vectrex_via_pb_r)
{
	int pot;
	ioport_port *io_port[4] = { m_io_contr1x, m_io_contr1y, m_io_contr2x, m_io_contr2y };

	pot = io_port[(m_via_out[PORTB] & 0x6) >> 1]->read() - 0x80;

	if (pot > (signed char)m_via_out[PORTA])
		m_via_out[PORTB] |= 0x20;
	else
		m_via_out[PORTB] &= ~0x20;

	return m_via_out[PORTB];
}


READ8_MEMBER(vectrex_state::vectrex_via_pa_r)
{
	if ((!(m_via_out[PORTB] & 0x10)) && (m_via_out[PORTB] & 0x08))
		/* BDIR inactive, we can read the PSG. BC1 has to be active. */
	{
		m_via_out[PORTA] = m_ay8912->data_r(space, 0)
			& ~(m_imager_pinlevel & 0x80);
	}
	return m_via_out[PORTA];
}


READ8_MEMBER(vectrex_state::vectrex_s1_via_pb_r)
{
	return (m_via_out[PORTB] & ~0x40) | (m_io_coin->read() & 0x40);
}


/*********************************************************************

   3D Imager support

*********************************************************************/

TIMER_CALLBACK_MEMBER(vectrex_state::vectrex_imager_change_color)
{
	m_beam_color = param;
}


TIMER_CALLBACK_MEMBER(vectrex_state::update_level)
{
	if (ptr)
		* (UINT8 *) ptr = param;
}


TIMER_CALLBACK_MEMBER(vectrex_state::vectrex_imager_eye)
{
	int coffset;
	double rtime = (1.0 / m_imager_freq);

	if (m_imager_status > 0)
	{
		m_imager_status = param;
		coffset = param > 1? 3: 0;
		timer_set(attotime::from_double(rtime * m_imager_angles[0]), TIMER_VECTREX_IMAGER_CHANGE_COLOR, m_imager_colors[coffset+2]);
		timer_set(attotime::from_double(rtime * m_imager_angles[1]), TIMER_VECTREX_IMAGER_CHANGE_COLOR, m_imager_colors[coffset+1]);
		timer_set(attotime::from_double(rtime * m_imager_angles[2]), TIMER_VECTREX_IMAGER_CHANGE_COLOR, m_imager_colors[coffset]);

		if (param == 2)
		{
			timer_set(attotime::from_double(rtime * 0.50), TIMER_VECTREX_IMAGER_EYE, 1);

			/* Index hole sensor is connected to IO7 which triggers also CA1 of VIA */
			m_via6522_0->write_ca1(1);
			m_via6522_0->write_ca1(0);
			m_imager_pinlevel |= 0x80;
			timer_set(attotime::from_double(rtime / 360.0), TIMER_UPDATE_LEVEL, 0, &m_imager_pinlevel);
		}
	}
}


WRITE8_MEMBER(vectrex_state::vectrex_psg_port_w)
{
	double wavel, ang_acc, tmp;
	int mcontrol;

	mcontrol = data & 0x40; /* IO6 controls the imager motor */

	if (!mcontrol && mcontrol ^ m_old_mcontrol)
	{
		m_old_mcontrol = mcontrol;
		tmp = machine().time().as_double();
		wavel = tmp - m_sl;
		m_sl = tmp;

		if (wavel < 1)
		{
			/* The Vectrex sends a stream of pulses which control the speed of
			   the motor using Pulse Width Modulation. Guessed parameters are MMI
			   (mass moment of inertia) of the color wheel, DAMPC (damping coefficient)
			   of the whole thing and some constants of the motor's torque/speed curve.
			   pwl is the negative pulse width and wavel is the whole wavelength. */

			ang_acc = (50.0 - 1.55 * m_imager_freq) / MMI;
			m_imager_freq += ang_acc * m_pwl + DAMPC * m_imager_freq / MMI * wavel;

			if (m_imager_freq > 1)
			{
				m_imager_timer->adjust(
										attotime::from_double(MIN(1.0 / m_imager_freq, m_imager_timer->remaining().as_double())),
										2,
										attotime::from_double(1.0 / m_imager_freq));
			}
		}
	}
	if (mcontrol && mcontrol ^ m_old_mcontrol)
	{
		m_old_mcontrol = mcontrol;
		m_pwl = machine().time().as_double() - m_sl;
	}
}

DRIVER_INIT_MEMBER(vectrex_state,vectrex)
{
	m_imager_angles = unknown_game_angles;
	m_beam_color = rgb_t::white;
	for (auto & elem : m_imager_colors)
		elem = rgb_t::white;

	/*
	 * Minestorm's PRNG doesn't work with a 0 seed (mines in the first
	 * level are not randomly distributed then). Only patch the seed's
	 * location since initializing all RAM randomly causes problems
	 * with Berzerk.
	 */
	m_gce_vectorram[0x7e] = machine().rand() | 1;
	m_gce_vectorram[0x7f] = machine().rand() | 1;
}

void vectrex_state::machine_start()
{
	if (m_cart && m_cart->exists())
	{
		// install cart accesses
		if (m_cart->get_type() == VECTREX_SRAM)
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0000, 0x7fff, read8_delegate(FUNC(vectrex_cart_slot_device::read_rom),(vectrex_cart_slot_device*)m_cart), write8_delegate(FUNC(vectrex_cart_slot_device::write_ram),(vectrex_cart_slot_device*)m_cart));
		else
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x7fff, read8_delegate(FUNC(vectrex_cart_slot_device::read_rom),(vectrex_cart_slot_device*)m_cart));

		// setup 3d imager and refresh timer

		// If VIA T2 starts, reset refresh timer. This is the best strategy for most games.
		m_reset_refresh = 1;
		m_imager_angles = narrow_escape_angles;

		// let's do this 3D detection
		switch (m_cart->get_vec3d())
		{
			case VEC3D_MINEST:
				m_imager_angles = minestorm_3d_angles;
				// Don't reset T2 each time it's written. This would cause jerking in mine3.
				m_reset_refresh = 0;
				break;
			case VEC3D_CCOAST:
				m_imager_angles = crazy_coaster_angles;
				break;
			case VEC3D_NARROW:
				m_imager_angles = narrow_escape_angles;
				break;
			default:
				break;
		}
	}
}
