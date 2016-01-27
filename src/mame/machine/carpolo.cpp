// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "includes/carpolo.h"


/*************************************
 *
 *  Interrupt system
 *
 *************************************/

/* the interrupt system consists of a 74148 priority encoder
   with the following interrupt priorites.  A lower number
   indicates a lower priority (coins handled first):

    7 - player 1 coin
    6 - player 2 coin
    5 - player 3 coin
    4 - player 4 coin
    3 - ball/screen object collision
    2 - car/car collision
    1 - car/goal collision
    0 - timer               (bit 4=0, bit 6=0)
    0 - car/ball collision  (bit 4=0, bit 6=1)
    0 - car/border          (bit 4=1, bit 6=1)

   After the interrupt is serviced, the code clears the
   priority encoder's appropriate line by pulling it HI.  This
   can trigger another interrupt immediately if there were
   lower priority lines LO.

   The four coin inputs are latched via 7474 flip-flop's. */

#define COIN1_PRIORITY_LINE         7
#define COIN2_PRIORITY_LINE         6
#define COIN3_PRIORITY_LINE         5
#define COIN4_PRIORITY_LINE         4
#define BALL_SCREEN_PRIORITY_LINE   3
#define CAR_CAR_PRIORITY_LINE       2
#define CAR_GOAL_PRIORITY_LINE      1
#define PRI0_PRIORTITY_LINE         0

/* priority 0 controls three different things */
#define TIMER_EXTRA_BITS            0x00
#define CAR_BALL_EXTRA_BITS         0x40
#define CAR_BORDER_EXTRA_BITS       0x50


TTL74148_OUTPUT_CB(carpolo_state::ttl74148_3s_cb)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_ttl74148_3s->output_valid_r() ? CLEAR_LINE : ASSERT_LINE);
}


/* the outputs of the flip-flops are connected to the priority encoder */
WRITE_LINE_MEMBER(carpolo_state::carpolo_7474_2s_1_q_cb)
{
	m_ttl74148_3s->input_line_w(COIN1_PRIORITY_LINE, state);
	m_ttl74148_3s->update();
}

WRITE_LINE_MEMBER(carpolo_state::carpolo_7474_2s_2_q_cb)
{
	m_ttl74148_3s->input_line_w(COIN2_PRIORITY_LINE, state);
	m_ttl74148_3s->update();
}

WRITE_LINE_MEMBER(carpolo_state::carpolo_7474_2u_1_q_cb)
{
	m_ttl74148_3s->input_line_w(COIN3_PRIORITY_LINE, state);
	m_ttl74148_3s->update();
}

WRITE_LINE_MEMBER(carpolo_state::carpolo_7474_2u_2_q_cb)
{
	m_ttl74148_3s->input_line_w(COIN4_PRIORITY_LINE, state);
	m_ttl74148_3s->update();
}


void carpolo_state::carpolo_generate_ball_screen_interrupt(UINT8 cause)
{
	m_ball_screen_collision_cause = cause;

	m_ttl74148_3s->input_line_w(BALL_SCREEN_PRIORITY_LINE, 0);
	m_ttl74148_3s->update();
}

void carpolo_state::carpolo_generate_car_car_interrupt(int car1, int car2)
{
	m_car_car_collision_cause = ~((1 << (3 - car1)) | (1 << (3 - car2)));

	m_ttl74148_3s->input_line_w(CAR_CAR_PRIORITY_LINE, 0);
	m_ttl74148_3s->update();
}

void carpolo_state::carpolo_generate_car_goal_interrupt(int car, int right_goal)
{
	m_car_goal_collision_cause = car | (right_goal ? 0x08 : 0x00);

	m_ttl74148_3s->input_line_w(CAR_GOAL_PRIORITY_LINE, 0);
	m_ttl74148_3s->update();
}

void carpolo_state::carpolo_generate_car_ball_interrupt(int car, int car_x, int car_y)
{
	m_car_ball_collision_cause = car;
	m_car_ball_collision_x = car_x;
	m_car_ball_collision_y = car_y;

	m_priority_0_extension = CAR_BALL_EXTRA_BITS;

	m_ttl74148_3s->input_line_w(PRI0_PRIORTITY_LINE, 0);
	m_ttl74148_3s->update();
}

void carpolo_state::carpolo_generate_car_border_interrupt(int car, int horizontal_border)
{
	m_car_border_collision_cause = car | (horizontal_border ? 0x04 : 0x00);

	m_priority_0_extension = CAR_BORDER_EXTRA_BITS;

	m_ttl74148_3s->input_line_w(PRI0_PRIORTITY_LINE, 0);
	m_ttl74148_3s->update();
}


READ8_MEMBER(carpolo_state::carpolo_ball_screen_collision_cause_r)
{
	/* bit 0 - 0=ball collided with border
	   bit 1 - 0=ball collided with goal
	   bit 2 - 0=ball collided with score area
	   bit 3 - which goal/score collided (0=left, 1=right) */
	return m_ball_screen_collision_cause;
}

READ8_MEMBER(carpolo_state::carpolo_car_ball_collision_x_r)
{
	/* the x coordinate of the colliding pixel */
	return m_car_ball_collision_x;
}

READ8_MEMBER(carpolo_state::carpolo_car_ball_collision_y_r)
{
	/* the y coordinate of the colliding pixel */
	return m_car_ball_collision_y;
}

READ8_MEMBER(carpolo_state::carpolo_car_car_collision_cause_r)
{
	/* bit 0 - car 4 collided
	   bit 1 - car 3 collided
	   bit 2 - car 2 collided
	   bit 3 - car 1 collided */
	return m_car_car_collision_cause;
}

READ8_MEMBER(carpolo_state::carpolo_car_goal_collision_cause_r)
{
	/* bit 0-1 - which car collided
	   bit 2   - horizontal timing bit 1TEC4 (not accessed)
	   bit 3   - which goal collided (0=left, 1=right) */
	return m_car_goal_collision_cause;
}

READ8_MEMBER(carpolo_state::carpolo_car_ball_collision_cause_r)
{
	/* bit 0-1 - which car collided
	   bit 2-3 - unconnected */
	return m_car_ball_collision_cause;
}

READ8_MEMBER(carpolo_state::carpolo_car_border_collision_cause_r)
{
	/* bit 0-1 - which car collided
	   bit 2   - 0=vertical border, 1=horizontal border */
	return m_car_border_collision_cause;
}


READ8_MEMBER(carpolo_state::carpolo_interrupt_cause_r)
{
	/* the output of the 148 goes to bits 1-3 (which is priority ^ 7) */
	return (m_ttl74148_3s->output_r() << 1) | m_priority_0_extension;
}


INTERRUPT_GEN_MEMBER(carpolo_state::carpolo_timer_interrupt)
{
	UINT8 port_value;
	int player;


	/* cause the timer interrupt */
	m_ttl74148_3s->input_line_w(PRI0_PRIORTITY_LINE, 0);
	m_priority_0_extension = TIMER_EXTRA_BITS;

	m_ttl74148_3s->update();


	/* check the coins here as well - they drive the clock of the flip-flops */
	port_value = ioport("IN0")->read();

	m_ttl7474_2s_1->clock_w((port_value & 0x01) >> 0);
	m_ttl7474_2s_2->clock_w((port_value & 0x02) >> 1);
	m_ttl7474_2u_1->clock_w((port_value & 0x04) >> 2);
	m_ttl7474_2u_2->clock_w((port_value & 0x08) >> 3);

	/* read the steering controls */
	for (player = 0; player < 4; player++)
	{
		static const char *const portnames[] = { "DIAL0", "DIAL1", "DIAL2", "DIAL3" };
		ttl7474_device *movement_flip_flop;
		ttl7474_device *dir_flip_flop;

		switch (player)
		{
			default:
			case 0: movement_flip_flop = m_ttl7474_1f_1;    dir_flip_flop = m_ttl7474_1f_2; break;
			case 1: movement_flip_flop = m_ttl7474_1d_1;    dir_flip_flop = m_ttl7474_1d_2; break;
			case 2: movement_flip_flop = m_ttl7474_1c_1;    dir_flip_flop = m_ttl7474_1c_2; break;
			case 3: movement_flip_flop = m_ttl7474_1a_1;    dir_flip_flop = m_ttl7474_1a_2; break;
		}

		port_value = ioport(portnames[player])->read();

		if (port_value != m_last_wheel_value[player])
		{
			/* set the movement direction */
			dir_flip_flop->d_w(((port_value - m_last_wheel_value[player]) & 0x80) ? 1 : 0);

			m_last_wheel_value[player] = port_value;
		}

		/* as the wheel moves, both flip-flops are clocked */
		movement_flip_flop->clock_w(port_value & 0x01);
		dir_flip_flop->clock_w(     port_value & 0x01);
	}



	/* finally read the accelerator pedals */
	port_value = ioport("PEDALS")->read();

	for (player = 0; player < 4; player++)
	{
		/* one line indicates if the pedal is pressed and the other
		   how much, resulting in only two different possible levels */
		if (port_value & 0x01)
		{
			m_ttl74153_1k->input_line_w(0, player, 1);
			m_ttl74153_1k->input_line_w(1, player, 0);
		}
		else if (port_value & 0x02)
		{
			m_ttl74153_1k->input_line_w(0, player, 1);
			m_ttl74153_1k->input_line_w(1, player, 1);
		}
		else
		{
			m_ttl74153_1k->input_line_w(0, player, 0);
			/* the other line is irrelevant */
		}

		port_value >>= 2;
	}

	m_ttl74153_1k->update();
}

// FIXME: Remove trampolines

WRITE_LINE_MEMBER(carpolo_state::coin1_interrupt_clear_w)
{
	m_ttl7474_2s_1->clear_w(state);
}

WRITE_LINE_MEMBER(carpolo_state::coin2_interrupt_clear_w)
{
	m_ttl7474_2s_2->clear_w(state);
}

WRITE_LINE_MEMBER(carpolo_state::coin3_interrupt_clear_w)
{
	m_ttl7474_2u_1->clear_w(state);
}

WRITE_LINE_MEMBER(carpolo_state::coin4_interrupt_clear_w)
{
	m_ttl7474_2u_2->clear_w(state);
}

WRITE8_MEMBER(carpolo_state::carpolo_ball_screen_interrupt_clear_w)
{
	m_ttl74148_3s->input_line_w(BALL_SCREEN_PRIORITY_LINE, 1);
	m_ttl74148_3s->update();
}

WRITE8_MEMBER(carpolo_state::carpolo_car_car_interrupt_clear_w)
{
	m_ttl74148_3s->input_line_w(CAR_CAR_PRIORITY_LINE, 1);
	m_ttl74148_3s->update();
}

WRITE8_MEMBER(carpolo_state::carpolo_car_goal_interrupt_clear_w)
{
	m_ttl74148_3s->input_line_w(CAR_GOAL_PRIORITY_LINE, 1);
	m_ttl74148_3s->update();
}

WRITE8_MEMBER(carpolo_state::carpolo_car_ball_interrupt_clear_w)
{
	m_ttl74148_3s->input_line_w(PRI0_PRIORTITY_LINE, 1);
	m_ttl74148_3s->update();
}

WRITE8_MEMBER(carpolo_state::carpolo_car_border_interrupt_clear_w)
{
	m_ttl74148_3s->input_line_w(PRI0_PRIORTITY_LINE, 1);
	m_ttl74148_3s->update();
}

WRITE8_MEMBER(carpolo_state::carpolo_timer_interrupt_clear_w)
{
	m_ttl74148_3s->input_line_w(PRI0_PRIORTITY_LINE, 1);
	m_ttl74148_3s->update();
}


/*************************************
 *
 *  Input port handling
 *
 *************************************/

WRITE8_MEMBER(carpolo_state::pia_0_port_a_w)
{
	/* bit 0 - Coin counter
	   bit 1 - Player 4 crash sound
	   bit 2 - Player 3 crash sound
	   bit 3 - Clear steering wheel logic
	   bit 4 - Player 2 crash sound
	   bit 5 - Score pulse sound
	   bit 6 - Player 1 crash sound
	   bit 7 - Ball hit pulse sound */

	machine().bookkeeping().coin_counter_w(0, data & 0x01);


	m_ttl7474_1f_1->clear_w((data & 0x08) >> 3);
	m_ttl7474_1d_1->clear_w((data & 0x08) >> 3);
	m_ttl7474_1c_1->clear_w((data & 0x08) >> 3);
	m_ttl7474_1a_1->clear_w((data & 0x08) >> 3);
}


WRITE8_MEMBER(carpolo_state::pia_0_port_b_w)
{
	/* bit 0 - Strobe speed bits sound
	   bit 1 - Speed bit 0 sound
	   bit 2 - Speed bit 1 sound
	   bit 3 - Speed bit 2 sound
	   bit 6 - Select pedal 0
	   bit 7 - Select pdeal 1 */

	m_ttl74153_1k->a_w(data & 0x40);
	m_ttl74153_1k->b_w(data & 0x80);

	m_ttl74153_1k->update();
}

READ8_MEMBER(carpolo_state::pia_0_port_b_r)
{
	/* bit 4 - Pedal bit 0
	   bit 5 - Pedal bit 1 */

	return (m_ttl74153_1k->output_r(0) << 5) |
			(m_ttl74153_1k->output_r(1) << 4);
}


READ8_MEMBER(carpolo_state::pia_1_port_a_r)
{
	UINT8 ret;

	/* bit 0 - Player 4 steering input (left or right)
	   bit 1 - Player 3 steering input (left or right)
	   bit 2 - Player 2 steering input (left or right)
	   bit 3 - Player 1 steering input (left or right)
	   bit 4 - Player 4 forward/reverse input
	   bit 5 - Player 3 forward/reverse input
	   bit 6 - Player 2 forward/reverse input
	   bit 7 - Player 1 forward/reverse input */

	ret = (m_ttl7474_1a_2->output_r() ? 0x01 : 0x00) |
			(m_ttl7474_1c_2->output_r() ? 0x02 : 0x00) |
			(m_ttl7474_1d_2->output_r() ? 0x04 : 0x00) |
			(m_ttl7474_1f_2->output_r() ? 0x08 : 0x00) |
			(ioport("IN2")->read() & 0xf0);

	return ret;
}


READ8_MEMBER(carpolo_state::pia_1_port_b_r)
{
	UINT8 ret;

	/* bit 4 - Player 4 steering input (wheel moving or stopped)
	   bit 5 - Player 3 steering input (wheel moving or stopped)
	   bit 6 - Player 2 steering input (wheel moving or stopped)
	   bit 7 - Player 1 steering input (wheel moving or stopped) */

	ret = (m_ttl7474_1a_1->output_r() ? 0x10 : 0x00) |
			(m_ttl7474_1c_1->output_r() ? 0x20 : 0x00) |
			(m_ttl7474_1d_1->output_r() ? 0x40 : 0x00) |
			(m_ttl7474_1f_1->output_r() ? 0x80 : 0x00);

	return ret;
}

void carpolo_state::machine_start()
{
	save_item(NAME(m_ball_screen_collision_cause));
	save_item(NAME(m_car_ball_collision_x));
	save_item(NAME(m_car_ball_collision_y));
	save_item(NAME(m_car_car_collision_cause));
	save_item(NAME(m_car_goal_collision_cause));
	save_item(NAME(m_car_ball_collision_cause));
	save_item(NAME(m_car_border_collision_cause));
	save_item(NAME(m_priority_0_extension));
	save_item(NAME(m_last_wheel_value));
}

void carpolo_state::machine_reset()
{
	/* set up the priority encoder */
	m_ttl74148_3s->enable_input_w(0);  /* always enabled */

	/* set up the coin handling flip-flops */
	m_ttl7474_2s_1->d_w     (1);
	m_ttl7474_2s_1->preset_w(1);

	m_ttl7474_2s_2->d_w     (1);
	m_ttl7474_2s_2->preset_w(1);

	m_ttl7474_2u_1->d_w     (1);
	m_ttl7474_2u_1->preset_w(1);

	m_ttl7474_2u_2->d_w     (1);
	m_ttl7474_2u_2->preset_w(1);


	/* set up the steering handling flip-flops */
	m_ttl7474_1f_1->d_w     (1);
	m_ttl7474_1f_1->preset_w(1);

	m_ttl7474_1f_2->clear_w (1);
	m_ttl7474_1f_2->preset_w(1);

	m_ttl7474_1d_1->d_w     (1);
	m_ttl7474_1d_1->preset_w(1);

	m_ttl7474_1d_2->clear_w (1);
	m_ttl7474_1d_2->preset_w(1);

	m_ttl7474_1c_1->d_w     (1);
	m_ttl7474_1c_1->preset_w(1);

	m_ttl7474_1c_2->clear_w (1);
	m_ttl7474_1c_2->preset_w(1);

	m_ttl7474_1a_1->d_w     (1);
	m_ttl7474_1a_1->preset_w(1);

	m_ttl7474_1a_2->clear_w (1);
	m_ttl7474_1a_2->preset_w(1);


	/* set up the pedal handling chips */
	m_ttl74153_1k->enable_w(0, 0);
	m_ttl74153_1k->enable_w(1, 0);
}
