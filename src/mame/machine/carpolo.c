/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/7474.h"
#include "machine/74148.h"
#include "machine/74153.h"
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

#define COIN1_PRIORITY_LINE			7
#define COIN2_PRIORITY_LINE			6
#define COIN3_PRIORITY_LINE			5
#define COIN4_PRIORITY_LINE			4
#define BALL_SCREEN_PRIORITY_LINE	3
#define CAR_CAR_PRIORITY_LINE		2
#define CAR_GOAL_PRIORITY_LINE		1
#define PRI0_PRIORTITY_LINE		    0

/* priority 0 controls three different things */
#define TIMER_EXTRA_BITS			0x00
#define CAR_BALL_EXTRA_BITS			0x40
#define CAR_BORDER_EXTRA_BITS		0x50


void carpolo_74148_3s_cb(device_t *device)
{
	cputag_set_input_line(device->machine(), "maincpu", M6502_IRQ_LINE, ttl74148_output_valid_r(device) ? CLEAR_LINE : ASSERT_LINE);
}


/* the outputs of the flip-flops are connected to the priority encoder */
WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2s_1_q_cb )
{
	ttl74148_input_line_w(device, COIN1_PRIORITY_LINE, state);
	ttl74148_update(device);
}

WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2s_2_q_cb )
{
	ttl74148_input_line_w(device, COIN2_PRIORITY_LINE, state);
	ttl74148_update(device);
}

WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2u_1_q_cb )
{
	ttl74148_input_line_w(device, COIN3_PRIORITY_LINE, state);
	ttl74148_update(device);
}

WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2u_2_q_cb )
{
	ttl74148_input_line_w(device, COIN4_PRIORITY_LINE, state);
	ttl74148_update(device);
}


void carpolo_generate_ball_screen_interrupt(running_machine &machine, UINT8 cause)
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	state->m_ball_screen_collision_cause = cause;

	ttl74148_input_line_w(state->m_ttl74148_3s, BALL_SCREEN_PRIORITY_LINE, 0);
	ttl74148_update(state->m_ttl74148_3s);
}

void carpolo_generate_car_car_interrupt(running_machine &machine, int car1, int car2)
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	state->m_car_car_collision_cause = ~((1 << (3 - car1)) | (1 << (3 - car2)));

	ttl74148_input_line_w(state->m_ttl74148_3s, CAR_CAR_PRIORITY_LINE, 0);
	ttl74148_update(state->m_ttl74148_3s);
}

void carpolo_generate_car_goal_interrupt(running_machine &machine, int car, int right_goal)
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	state->m_car_goal_collision_cause = car | (right_goal ? 0x08 : 0x00);

	ttl74148_input_line_w(state->m_ttl74148_3s, CAR_GOAL_PRIORITY_LINE, 0);
	ttl74148_update(state->m_ttl74148_3s);
}

void carpolo_generate_car_ball_interrupt(running_machine &machine, int car, int car_x, int car_y)
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	state->m_car_ball_collision_cause = car;
	state->m_car_ball_collision_x = car_x;
	state->m_car_ball_collision_y = car_y;

	state->m_priority_0_extension = CAR_BALL_EXTRA_BITS;

	ttl74148_input_line_w(state->m_ttl74148_3s, PRI0_PRIORTITY_LINE, 0);
	ttl74148_update(state->m_ttl74148_3s);
}

void carpolo_generate_car_border_interrupt(running_machine &machine, int car, int horizontal_border)
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	state->m_car_border_collision_cause = car | (horizontal_border ? 0x04 : 0x00);

	state->m_priority_0_extension = CAR_BORDER_EXTRA_BITS;

	ttl74148_input_line_w(state->m_ttl74148_3s, PRI0_PRIORTITY_LINE, 0);
	ttl74148_update(state->m_ttl74148_3s);
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
	return (ttl74148_output_r(m_ttl74148_3s) << 1) | m_priority_0_extension;
}


INTERRUPT_GEN( carpolo_timer_interrupt )
{
	carpolo_state *state = device->machine().driver_data<carpolo_state>();
	UINT8 port_value;
	int player;


	/* cause the timer interrupt */
	ttl74148_input_line_w(state->m_ttl74148_3s, PRI0_PRIORTITY_LINE, 0);
	state->m_priority_0_extension = TIMER_EXTRA_BITS;

	ttl74148_update(state->m_ttl74148_3s);


	/* check the coins here as well - they drive the clock of the flip-flops */
	port_value = input_port_read(device->machine(), "IN0");

	state->m_ttl7474_2s_1->clock_w((port_value & 0x01) >> 0);
	state->m_ttl7474_2s_2->clock_w((port_value & 0x02) >> 1);
	state->m_ttl7474_2u_1->clock_w((port_value & 0x04) >> 2);
	state->m_ttl7474_2u_2->clock_w((port_value & 0x08) >> 3);

	/* read the steering controls */
	for (player = 0; player < 4; player++)
	{
		static const char *const portnames[] = { "DIAL0", "DIAL1", "DIAL2", "DIAL3" };
		ttl7474_device *movement_flip_flop;
		ttl7474_device *dir_flip_flop;

		switch (player)
		{
			default:
			case 0:	movement_flip_flop = state->m_ttl7474_1f_1;	dir_flip_flop = state->m_ttl7474_1f_2;	break;
			case 1:	movement_flip_flop = state->m_ttl7474_1d_1;	dir_flip_flop = state->m_ttl7474_1d_2;	break;
			case 2:	movement_flip_flop = state->m_ttl7474_1c_1;	dir_flip_flop = state->m_ttl7474_1c_2;	break;
			case 3:	movement_flip_flop = state->m_ttl7474_1a_1;	dir_flip_flop = state->m_ttl7474_1a_2;	break;
		}

		port_value = input_port_read(device->machine(), portnames[player]);

		if (port_value != state->m_last_wheel_value[player])
		{
			/* set the movement direction */
			dir_flip_flop->d_w(((port_value - state->m_last_wheel_value[player]) & 0x80) ? 1 : 0);

			state->m_last_wheel_value[player] = port_value;
		}

		/* as the wheel moves, both flip-flops are clocked */
		movement_flip_flop->clock_w(port_value & 0x01);
		dir_flip_flop->clock_w(     port_value & 0x01);
	}



	/* finally read the accelerator pedals */
	port_value = input_port_read(device->machine(), "PEDALS");

	for (player = 0; player < 4; player++)
	{
		/* one line indicates if the pedal is pressed and the other
           how much, resulting in only two different possible levels */
		if (port_value & 0x01)
		{
			ttl74153_input_line_w(state->m_ttl74153_1k, 0, player, 1);
			ttl74153_input_line_w(state->m_ttl74153_1k, 1, player, 0);
		}
		else if (port_value & 0x02)
		{
			ttl74153_input_line_w(state->m_ttl74153_1k, 0, player, 1);
			ttl74153_input_line_w(state->m_ttl74153_1k, 1, player, 1);
		}
		else
		{
			ttl74153_input_line_w(state->m_ttl74153_1k, 0, player, 0);
			/* the other line is irrelevant */
		}

		port_value >>= 2;
	}

	ttl74153_update(state->m_ttl74153_1k);
}

// FIXME: Remove trampolines

static WRITE_LINE_DEVICE_HANDLER( coin1_interrupt_clear_w )
{
	carpolo_state *drvstate = device->machine().driver_data<carpolo_state>();
	drvstate->m_ttl7474_2s_1->clear_w(state);
}

static WRITE_LINE_DEVICE_HANDLER( coin2_interrupt_clear_w )
{
	carpolo_state *drvstate = device->machine().driver_data<carpolo_state>();
	drvstate->m_ttl7474_2s_2->clear_w(state);
}

static WRITE_LINE_DEVICE_HANDLER( coin3_interrupt_clear_w )
{
	carpolo_state *drvstate = device->machine().driver_data<carpolo_state>();
	drvstate->m_ttl7474_2u_1->clear_w(state);
}

static WRITE_LINE_DEVICE_HANDLER( coin4_interrupt_clear_w )
{
	carpolo_state *drvstate = device->machine().driver_data<carpolo_state>();
	drvstate->m_ttl7474_2u_2->clear_w(state);
}

WRITE8_MEMBER(carpolo_state::carpolo_ball_screen_interrupt_clear_w)
{
	ttl74148_input_line_w(m_ttl74148_3s, BALL_SCREEN_PRIORITY_LINE, 1);
	ttl74148_update(m_ttl74148_3s);
}

WRITE8_MEMBER(carpolo_state::carpolo_car_car_interrupt_clear_w)
{
	ttl74148_input_line_w(m_ttl74148_3s, CAR_CAR_PRIORITY_LINE, 1);
	ttl74148_update(m_ttl74148_3s);
}

WRITE8_MEMBER(carpolo_state::carpolo_car_goal_interrupt_clear_w)
{
	ttl74148_input_line_w(m_ttl74148_3s, CAR_GOAL_PRIORITY_LINE, 1);
	ttl74148_update(m_ttl74148_3s);
}

WRITE8_MEMBER(carpolo_state::carpolo_car_ball_interrupt_clear_w)
{
	ttl74148_input_line_w(m_ttl74148_3s, PRI0_PRIORTITY_LINE, 1);
	ttl74148_update(m_ttl74148_3s);
}

WRITE8_MEMBER(carpolo_state::carpolo_car_border_interrupt_clear_w)
{
	ttl74148_input_line_w(m_ttl74148_3s, PRI0_PRIORTITY_LINE, 1);
	ttl74148_update(m_ttl74148_3s);
}

WRITE8_MEMBER(carpolo_state::carpolo_timer_interrupt_clear_w)
{
	ttl74148_input_line_w(m_ttl74148_3s, PRI0_PRIORTITY_LINE, 1);
	ttl74148_update(m_ttl74148_3s);
}


/*************************************
 *
 *  Input port handling
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( pia_0_port_a_w )
{
	carpolo_state *state = device->machine().driver_data<carpolo_state>();
	/* bit 0 - Coin counter
       bit 1 - Player 4 crash sound
       bit 2 - Player 3 crash sound
       bit 3 - Clear steering wheel logic
       bit 4 - Player 2 crash sound
       bit 5 - Score pulse sound
       bit 6 - Player 1 crash sound
       bit 7 - Ball hit pulse sound */

	coin_counter_w(device->machine(), 0, data & 0x01);


	state->m_ttl7474_1f_1->clear_w((data & 0x08) >> 3);
	state->m_ttl7474_1d_1->clear_w((data & 0x08) >> 3);
	state->m_ttl7474_1c_1->clear_w((data & 0x08) >> 3);
	state->m_ttl7474_1a_1->clear_w((data & 0x08) >> 3);
}


static WRITE8_DEVICE_HANDLER( pia_0_port_b_w )
{
	carpolo_state *state = device->machine().driver_data<carpolo_state>();
	/* bit 0 - Strobe speed bits sound
       bit 1 - Speed bit 0 sound
       bit 2 - Speed bit 1 sound
       bit 3 - Speed bit 2 sound
       bit 6 - Select pedal 0
       bit 7 - Select pdeal 1 */

	ttl74153_a_w(state->m_ttl74153_1k, data & 0x40);
	ttl74153_b_w(state->m_ttl74153_1k, data & 0x80);

	ttl74153_update(state->m_ttl74153_1k);
}

static READ8_DEVICE_HANDLER( pia_0_port_b_r )
{
	carpolo_state *state = device->machine().driver_data<carpolo_state>();
	/* bit 4 - Pedal bit 0
       bit 5 - Pedal bit 1 */

	return (ttl74153_output_r(state->m_ttl74153_1k, 0) << 5) |
		   (ttl74153_output_r(state->m_ttl74153_1k, 1) << 4);
}


static READ8_DEVICE_HANDLER( pia_1_port_a_r )
{
	carpolo_state *state = device->machine().driver_data<carpolo_state>();
	UINT8 ret;

	/* bit 0 - Player 4 steering input (left or right)
       bit 1 - Player 3 steering input (left or right)
       bit 2 - Player 2 steering input (left or right)
       bit 3 - Player 1 steering input (left or right)
       bit 4 - Player 4 forward/reverse input
       bit 5 - Player 3 forward/reverse input
       bit 6 - Player 2 forward/reverse input
       bit 7 - Player 1 forward/reverse input */

	ret = (state->m_ttl7474_1a_2->output_r() ? 0x01 : 0x00) |
		  (state->m_ttl7474_1c_2->output_r() ? 0x02 : 0x00) |
		  (state->m_ttl7474_1d_2->output_r() ? 0x04 : 0x00) |
		  (state->m_ttl7474_1f_2->output_r() ? 0x08 : 0x00) |
		  (input_port_read(device->machine(), "IN2") & 0xf0);

	return ret;
}


static READ8_DEVICE_HANDLER( pia_1_port_b_r )
{
	carpolo_state *state = device->machine().driver_data<carpolo_state>();
	UINT8 ret;

	/* bit 4 - Player 4 steering input (wheel moving or stopped)
       bit 5 - Player 3 steering input (wheel moving or stopped)
       bit 6 - Player 2 steering input (wheel moving or stopped)
       bit 7 - Player 1 steering input (wheel moving or stopped) */

	ret = (state->m_ttl7474_1a_1->output_r() ? 0x10 : 0x00) |
		  (state->m_ttl7474_1c_1->output_r() ? 0x20 : 0x00) |
		  (state->m_ttl7474_1d_1->output_r() ? 0x40 : 0x00) |
		  (state->m_ttl7474_1f_1->output_r() ? 0x80 : 0x00);

	return ret;
}


const pia6821_interface carpolo_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_HANDLER(pia_0_port_b_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_0_port_a_w),		/* port A out */
	DEVCB_HANDLER(pia_0_port_b_w),		/* port B out */
	DEVCB_LINE(coin1_interrupt_clear_w),		/* line CA2 out */
	DEVCB_LINE(coin2_interrupt_clear_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


const pia6821_interface carpolo_pia1_intf =
{
	DEVCB_HANDLER(pia_1_port_a_r),		/* port A in */
	DEVCB_HANDLER(pia_1_port_b_r),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_LINE(coin3_interrupt_clear_w),		/* line CA2 out */
	DEVCB_LINE(coin4_interrupt_clear_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

MACHINE_START( carpolo )
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	/* find flip-flops */
	state->m_ttl7474_2s_1 = machine.device<ttl7474_device>("7474_2s_1");
	state->m_ttl7474_2s_2 = machine.device<ttl7474_device>("7474_2s_2");
	state->m_ttl7474_2u_1 = machine.device<ttl7474_device>("7474_2u_1");
	state->m_ttl7474_2u_2 = machine.device<ttl7474_device>("7474_2u_2");
	state->m_ttl7474_1f_1 = machine.device<ttl7474_device>("7474_1f_1");
	state->m_ttl7474_1f_2 = machine.device<ttl7474_device>("7474_1f_2");
	state->m_ttl7474_1d_1 = machine.device<ttl7474_device>("7474_1d_1");
	state->m_ttl7474_1d_2 = machine.device<ttl7474_device>("7474_1d_2");
	state->m_ttl7474_1c_1 = machine.device<ttl7474_device>("7474_1c_1");
	state->m_ttl7474_1c_2 = machine.device<ttl7474_device>("7474_1c_2");
	state->m_ttl7474_1a_1 = machine.device<ttl7474_device>("7474_1a_1");
	state->m_ttl7474_1a_2 = machine.device<ttl7474_device>("7474_1a_2");

	state->m_ttl74148_3s = machine.device("74148_3s");
	state->m_ttl74153_1k = machine.device("74153_1k");

    state_save_register_global(machine, state->m_ball_screen_collision_cause);
    state_save_register_global(machine, state->m_car_ball_collision_x);
    state_save_register_global(machine, state->m_car_ball_collision_y);
    state_save_register_global(machine, state->m_car_car_collision_cause);
    state_save_register_global(machine, state->m_car_goal_collision_cause);
    state_save_register_global(machine, state->m_car_ball_collision_cause);
    state_save_register_global(machine, state->m_car_border_collision_cause);
    state_save_register_global(machine, state->m_priority_0_extension);
    state_save_register_global_array(machine, state->m_last_wheel_value);
}

MACHINE_RESET( carpolo )
{
	carpolo_state *state = machine.driver_data<carpolo_state>();
	/* set up the priority encoder */
	ttl74148_enable_input_w(state->m_ttl74148_3s, 0);	/* always enabled */

	/* set up the coin handling flip-flops */
	state->m_ttl7474_2s_1->d_w     (1);
	state->m_ttl7474_2s_1->preset_w(1);

	state->m_ttl7474_2s_2->d_w     (1);
	state->m_ttl7474_2s_2->preset_w(1);

	state->m_ttl7474_2u_1->d_w     (1);
	state->m_ttl7474_2u_1->preset_w(1);

	state->m_ttl7474_2u_2->d_w     (1);
	state->m_ttl7474_2u_2->preset_w(1);


	/* set up the steering handling flip-flops */
	state->m_ttl7474_1f_1->d_w     (1);
	state->m_ttl7474_1f_1->preset_w(1);

	state->m_ttl7474_1f_2->clear_w (1);
	state->m_ttl7474_1f_2->preset_w(1);

	state->m_ttl7474_1d_1->d_w     (1);
	state->m_ttl7474_1d_1->preset_w(1);

	state->m_ttl7474_1d_2->clear_w (1);
	state->m_ttl7474_1d_2->preset_w(1);

	state->m_ttl7474_1c_1->d_w     (1);
	state->m_ttl7474_1c_1->preset_w(1);

	state->m_ttl7474_1c_2->clear_w (1);
	state->m_ttl7474_1c_2->preset_w(1);

	state->m_ttl7474_1a_1->d_w     (1);
	state->m_ttl7474_1a_1->preset_w(1);

	state->m_ttl7474_1a_2->clear_w (1);
	state->m_ttl7474_1a_2->preset_w(1);


	/* set up the pedal handling chips */
	ttl74153_enable_w(state->m_ttl74153_1k, 0, 0);
	ttl74153_enable_w(state->m_ttl74153_1k, 1, 0);
}
