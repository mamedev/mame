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



static UINT8 ball_screen_collision_cause;
static UINT8 car_ball_collision_x;
static UINT8 car_ball_collision_y;
static UINT8 car_car_collision_cause;
static UINT8 car_goal_collision_cause;
static UINT8 car_ball_collision_cause;
static UINT8 car_border_collision_cause;
static UINT8 priority_0_extension;
static UINT8 last_wheel_value[4];

static running_device *ttl74148_3s;

static running_device *ttl74153_1k;

static running_device *ttl7474_2s_1;
static running_device *ttl7474_2s_2;
static running_device *ttl7474_2u_1;
static running_device *ttl7474_2u_2;
static running_device *ttl7474_1f_1;
static running_device *ttl7474_1f_2;
static running_device *ttl7474_1d_1;
static running_device *ttl7474_1d_2;
static running_device *ttl7474_1c_1;
static running_device *ttl7474_1c_2;
static running_device *ttl7474_1a_1;
static running_device *ttl7474_1a_2;


void carpolo_74148_3s_cb(running_device *device)
{
	cputag_set_input_line(device->machine, "maincpu", M6502_IRQ_LINE, ttl74148_output_valid_r(device) ? CLEAR_LINE : ASSERT_LINE);
}


/* the outputs of the flip-flops are connected to the priority encoder */
void carpolo_7474_2s_1_cb(running_device *device)
{
	ttl74148_input_line_w(ttl74148_3s, COIN1_PRIORITY_LINE, ttl7474_output_comp_r(device));
	ttl74148_update(ttl74148_3s);
}

void carpolo_7474_2s_2_cb(running_device *device)
{
	ttl74148_input_line_w(ttl74148_3s, COIN2_PRIORITY_LINE, ttl7474_output_comp_r(device));
	ttl74148_update(ttl74148_3s);
}

void carpolo_7474_2u_1_cb(running_device *device)
{
	ttl74148_input_line_w(ttl74148_3s, COIN3_PRIORITY_LINE, ttl7474_output_comp_r(device));
	ttl74148_update(ttl74148_3s);
}

void carpolo_7474_2u_2_cb(running_device *device)
{
	ttl74148_input_line_w(ttl74148_3s, COIN4_PRIORITY_LINE, ttl7474_output_comp_r(device));
	ttl74148_update(ttl74148_3s);
}


void carpolo_generate_ball_screen_interrupt(running_machine *machine, UINT8 cause)
{
	ball_screen_collision_cause = cause;

	ttl74148_input_line_w(ttl74148_3s, BALL_SCREEN_PRIORITY_LINE, 0);
	ttl74148_update(ttl74148_3s);
}

void carpolo_generate_car_car_interrupt(running_machine *machine, int car1, int car2)
{
	car_car_collision_cause = ~((1 << (3 - car1)) | (1 << (3 - car2)));

	ttl74148_input_line_w(ttl74148_3s, CAR_CAR_PRIORITY_LINE, 0);
	ttl74148_update(ttl74148_3s);
}

void carpolo_generate_car_goal_interrupt(running_machine *machine, int car, int right_goal)
{
	car_goal_collision_cause = car | (right_goal ? 0x08 : 0x00);

	ttl74148_input_line_w(ttl74148_3s, CAR_GOAL_PRIORITY_LINE, 0);
	ttl74148_update(ttl74148_3s);
}

void carpolo_generate_car_ball_interrupt(running_machine *machine, int car, int car_x, int car_y)
{
	car_ball_collision_cause = car;
	car_ball_collision_x = car_x;
	car_ball_collision_y = car_y;

	priority_0_extension = CAR_BALL_EXTRA_BITS;

	ttl74148_input_line_w(ttl74148_3s, PRI0_PRIORTITY_LINE, 0);
	ttl74148_update(ttl74148_3s);
}

void carpolo_generate_car_border_interrupt(running_machine *machine, int car, int horizontal_border)
{
	car_border_collision_cause = car | (horizontal_border ? 0x04 : 0x00);

	priority_0_extension = CAR_BORDER_EXTRA_BITS;

	ttl74148_input_line_w(ttl74148_3s, PRI0_PRIORTITY_LINE, 0);
	ttl74148_update(ttl74148_3s);
}


READ8_HANDLER( carpolo_ball_screen_collision_cause_r )
{
	/* bit 0 - 0=ball collided with border
       bit 1 - 0=ball collided with goal
       bit 2 - 0=ball collided with score area
       bit 3 - which goal/score collided (0=left, 1=right) */
	return ball_screen_collision_cause;
}

READ8_HANDLER( carpolo_car_ball_collision_x_r )
{
	/* the x coordinate of the colliding pixel */
	return car_ball_collision_x;
}

READ8_HANDLER( carpolo_car_ball_collision_y_r )
{
	/* the y coordinate of the colliding pixel */
	return car_ball_collision_y;
}

READ8_HANDLER( carpolo_car_car_collision_cause_r )
{
	/* bit 0 - car 4 collided
       bit 1 - car 3 collided
       bit 2 - car 2 collided
       bit 3 - car 1 collided */
	return car_car_collision_cause;
}

READ8_HANDLER( carpolo_car_goal_collision_cause_r )
{
	/* bit 0-1 - which car collided
       bit 2   - horizontal timing bit 1TEC4 (not accessed)
       bit 3   - which goal collided (0=left, 1=right) */
	return car_goal_collision_cause;
}

READ8_HANDLER( carpolo_car_ball_collision_cause_r )
{
	/* bit 0-1 - which car collided
       bit 2-3 - unconnected */
	return car_ball_collision_cause;
}

READ8_HANDLER( carpolo_car_border_collision_cause_r )
{
	/* bit 0-1 - which car collided
       bit 2   - 0=vertical border, 1=horizontal border */
	return car_border_collision_cause;
}


READ8_HANDLER( carpolo_interrupt_cause_r )
{
	/* the output of the 148 goes to bits 1-3 (which is priority ^ 7) */
	return (ttl74148_output_r(ttl74148_3s) << 1) | priority_0_extension;
}


INTERRUPT_GEN( carpolo_timer_interrupt )
{
	UINT8 port_value;
	int player;


	/* cause the timer interrupt */
	ttl74148_input_line_w(ttl74148_3s, PRI0_PRIORTITY_LINE, 0);
	priority_0_extension = TIMER_EXTRA_BITS;

	ttl74148_update(ttl74148_3s);


	/* check the coins here as well - they drive the clock of the flip-flops */
	port_value = input_port_read(device->machine, "IN0");

	ttl7474_clock_w(ttl7474_2s_1, port_value & 0x01);
	ttl7474_clock_w(ttl7474_2s_2, port_value & 0x02);
	ttl7474_clock_w(ttl7474_2u_1, port_value & 0x04);
	ttl7474_clock_w(ttl7474_2u_2, port_value & 0x08);

	ttl7474_update(ttl7474_2s_1);
	ttl7474_update(ttl7474_2s_2);
	ttl7474_update(ttl7474_2u_1);
	ttl7474_update(ttl7474_2u_2);


	/* read the steering controls */
	for (player = 0; player < 4; player++)
	{
		static const char *const portnames[] = { "DIAL0", "DIAL1", "DIAL2", "DIAL3" };
		running_device *movement_flip_flop;
		running_device *dir_flip_flop;

		switch (player)
		{
			default:
			case 0:	movement_flip_flop = ttl7474_1f_1;	dir_flip_flop = ttl7474_1f_2;	break;
			case 1:	movement_flip_flop = ttl7474_1d_1;	dir_flip_flop = ttl7474_1d_2;	break;
			case 2:	movement_flip_flop = ttl7474_1c_1;	dir_flip_flop = ttl7474_1c_2;	break;
			case 3:	movement_flip_flop = ttl7474_1a_1;	dir_flip_flop = ttl7474_1a_2;	break;
		}

		port_value = input_port_read(device->machine, portnames[player]);

		if (port_value != last_wheel_value[player])
		{
			/* set the movement direction */
			ttl7474_d_w(dir_flip_flop, ((port_value - last_wheel_value[player]) & 0x80) ? 1 : 0);

			last_wheel_value[player] = port_value;
		}

		/* as the wheel moves, both flip-flops are clocked */
		ttl7474_clock_w(movement_flip_flop, port_value & 0x01);
		ttl7474_clock_w(dir_flip_flop,      port_value & 0x01);

		ttl7474_update(movement_flip_flop);
		ttl7474_update(dir_flip_flop);
	}



	/* finally read the accelerator pedals */
	port_value = input_port_read(device->machine, "PEDALS");

	for (player = 0; player < 4; player++)
	{
		/* one line indicates if the pedal is pressed and the other
           how much, resulting in only two different possible levels */
		if (port_value & 0x01)
		{
			ttl74153_input_line_w(ttl74153_1k, 0, player, 1);
			ttl74153_input_line_w(ttl74153_1k, 1, player, 0);
		}
		else if (port_value & 0x02)
		{
			ttl74153_input_line_w(ttl74153_1k, 0, player, 1);
			ttl74153_input_line_w(ttl74153_1k, 1, player, 1);
		}
		else
		{
			ttl74153_input_line_w(ttl74153_1k, 0, player, 0);
			/* the other line is irrelevant */
		}

		port_value >>= 2;
	}

	ttl74153_update(ttl74153_1k);
}


static WRITE_LINE_DEVICE_HANDLER( coin1_interrupt_clear_w )
{
	ttl7474_clear_w(ttl7474_2s_1, state);
	ttl7474_update(ttl7474_2s_1);
}

static WRITE_LINE_DEVICE_HANDLER( coin2_interrupt_clear_w )
{
	ttl7474_clear_w(ttl7474_2s_2, state);
	ttl7474_update(ttl7474_2s_2);
}

static WRITE_LINE_DEVICE_HANDLER( coin3_interrupt_clear_w )
{
	ttl7474_clear_w(ttl7474_2u_1, state);
	ttl7474_update(ttl7474_2u_1);
}

static WRITE_LINE_DEVICE_HANDLER( coin4_interrupt_clear_w )
{
	ttl7474_clear_w(ttl7474_2u_2, state);
	ttl7474_update(ttl7474_2u_2);
}

WRITE8_HANDLER( carpolo_ball_screen_interrupt_clear_w )
{
	ttl74148_input_line_w(ttl74148_3s, BALL_SCREEN_PRIORITY_LINE, 1);
	ttl74148_update(ttl74148_3s);
}

WRITE8_HANDLER( carpolo_car_car_interrupt_clear_w )
{
	ttl74148_input_line_w(ttl74148_3s, CAR_CAR_PRIORITY_LINE, 1);
	ttl74148_update(ttl74148_3s);
}

WRITE8_HANDLER( carpolo_car_goal_interrupt_clear_w )
{
	ttl74148_input_line_w(ttl74148_3s, CAR_GOAL_PRIORITY_LINE, 1);
	ttl74148_update(ttl74148_3s);
}

WRITE8_HANDLER( carpolo_car_ball_interrupt_clear_w )
{
	ttl74148_input_line_w(ttl74148_3s, PRI0_PRIORTITY_LINE, 1);
	ttl74148_update(ttl74148_3s);
}

WRITE8_HANDLER( carpolo_car_border_interrupt_clear_w )
{
	ttl74148_input_line_w(ttl74148_3s, PRI0_PRIORTITY_LINE, 1);
	ttl74148_update(ttl74148_3s);
}

WRITE8_HANDLER( carpolo_timer_interrupt_clear_w )
{
	ttl74148_input_line_w(ttl74148_3s, PRI0_PRIORTITY_LINE, 1);
	ttl74148_update(ttl74148_3s);
}


/*************************************
 *
 *  Input port handling
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( pia_0_port_a_w )
{
	/* bit 0 - Coin counter
       bit 1 - Player 4 crash sound
       bit 2 - Player 3 crash sound
       bit 3 - Clear steering wheel logic
       bit 4 - Player 2 crash sound
       bit 5 - Score pulse sound
       bit 6 - Player 1 crash sound
       bit 7 - Ball hit pulse sound */

	coin_counter_w(device->machine, 0, data & 0x01);


	ttl7474_clear_w(ttl7474_1f_1, data & 0x08);
	ttl7474_clear_w(ttl7474_1d_1, data & 0x08);
	ttl7474_clear_w(ttl7474_1c_1, data & 0x08);
	ttl7474_clear_w(ttl7474_1a_1, data & 0x08);

	ttl7474_update(ttl7474_1f_1);
	ttl7474_update(ttl7474_1d_1);
	ttl7474_update(ttl7474_1c_1);
	ttl7474_update(ttl7474_1a_1);
}


static WRITE8_DEVICE_HANDLER( pia_0_port_b_w )
{
	/* bit 0 - Strobe speed bits sound
       bit 1 - Speed bit 0 sound
       bit 2 - Speed bit 1 sound
       bit 3 - Speed bit 2 sound
       bit 6 - Select pedal 0
       bit 7 - Select pdeal 1 */

	ttl74153_a_w(ttl74153_1k, data & 0x40);
	ttl74153_b_w(ttl74153_1k, data & 0x80);

	ttl74153_update(ttl74153_1k);
}

static READ8_DEVICE_HANDLER( pia_0_port_b_r )
{
	/* bit 4 - Pedal bit 0
       bit 5 - Pedal bit 1 */

	return (ttl74153_output_r(ttl74153_1k, 0) << 5) |
		   (ttl74153_output_r(ttl74153_1k, 1) << 4);
}


static READ8_DEVICE_HANDLER( pia_1_port_a_r )
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

	ret = (ttl7474_output_r(ttl7474_1a_2) ? 0x01 : 0x00) |
		  (ttl7474_output_r(ttl7474_1c_2) ? 0x02 : 0x00) |
		  (ttl7474_output_r(ttl7474_1d_2) ? 0x04 : 0x00) |
		  (ttl7474_output_r(ttl7474_1f_2) ? 0x08 : 0x00) |
		  (input_port_read(device->machine, "IN2") & 0xf0);

	return ret;
}


static READ8_DEVICE_HANDLER( pia_1_port_b_r )
{
	UINT8 ret;

	/* bit 4 - Player 4 steering input (wheel moving or stopped)
       bit 5 - Player 3 steering input (wheel moving or stopped)
       bit 6 - Player 2 steering input (wheel moving or stopped)
       bit 7 - Player 1 steering input (wheel moving or stopped) */

	ret = (ttl7474_output_r(ttl7474_1a_1) ? 0x10 : 0x00) |
		  (ttl7474_output_r(ttl7474_1c_1) ? 0x20 : 0x00) |
		  (ttl7474_output_r(ttl7474_1d_1) ? 0x40 : 0x00) |
		  (ttl7474_output_r(ttl7474_1f_1) ? 0x80 : 0x00);

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
	/* find flip-flops */
	ttl7474_2s_1 = devtag_get_device(machine, "7474_2s_1");
	ttl7474_2s_2 = devtag_get_device(machine, "7474_2s_2");
	ttl7474_2u_1 = devtag_get_device(machine, "7474_2u_1");
	ttl7474_2u_2 = devtag_get_device(machine, "7474_2u_2");
	ttl7474_1f_1 = devtag_get_device(machine, "7474_1f_1");
	ttl7474_1f_2 = devtag_get_device(machine, "7474_1f_2");
	ttl7474_1d_1 = devtag_get_device(machine, "7474_1d_1");
	ttl7474_1d_2 = devtag_get_device(machine, "7474_1d_2");
	ttl7474_1c_1 = devtag_get_device(machine, "7474_1c_1");
	ttl7474_1c_2 = devtag_get_device(machine, "7474_1c_2");
	ttl7474_1a_1 = devtag_get_device(machine, "7474_1a_1");
	ttl7474_1a_2 = devtag_get_device(machine, "7474_1a_2");

	ttl74148_3s = devtag_get_device(machine, "74148_3s");
	ttl74153_1k = devtag_get_device(machine, "74153_1k");

    state_save_register_global(machine, ball_screen_collision_cause);
    state_save_register_global(machine, car_ball_collision_x);
    state_save_register_global(machine, car_ball_collision_y);
    state_save_register_global(machine, car_car_collision_cause);
    state_save_register_global(machine, car_goal_collision_cause);
    state_save_register_global(machine, car_ball_collision_cause);
    state_save_register_global(machine, car_border_collision_cause);
    state_save_register_global(machine, priority_0_extension);
    state_save_register_global_array(machine, last_wheel_value);
}

MACHINE_RESET( carpolo )
{
	/* set up the priority encoder */
	ttl74148_enable_input_w(ttl74148_3s, 0);	/* always enabled */

	/* set up the coin handling flip-flops */
	ttl7474_d_w     (ttl7474_2s_1, 1);
	ttl7474_preset_w(ttl7474_2s_1, 1);

	ttl7474_d_w     (ttl7474_2s_2, 1);
	ttl7474_preset_w(ttl7474_2s_2, 1);

	ttl7474_d_w     (ttl7474_2u_1, 1);
	ttl7474_preset_w(ttl7474_2u_1, 1);

	ttl7474_d_w     (ttl7474_2u_2, 1);
	ttl7474_preset_w(ttl7474_2u_2, 1);


	/* set up the steering handling flip-flops */
	ttl7474_d_w     (ttl7474_1f_1, 1);
	ttl7474_preset_w(ttl7474_1f_1, 1);

	ttl7474_clear_w (ttl7474_1f_2, 1);
	ttl7474_preset_w(ttl7474_1f_2, 1);

	ttl7474_d_w     (ttl7474_1d_1, 1);
	ttl7474_preset_w(ttl7474_1d_1, 1);

	ttl7474_clear_w (ttl7474_1d_2, 1);
	ttl7474_preset_w(ttl7474_1d_2, 1);

	ttl7474_d_w     (ttl7474_1c_1, 1);
	ttl7474_preset_w(ttl7474_1c_1, 1);

	ttl7474_clear_w (ttl7474_1c_2, 1);
	ttl7474_preset_w(ttl7474_1c_2, 1);

	ttl7474_d_w     (ttl7474_1a_1, 1);
	ttl7474_preset_w(ttl7474_1a_1, 1);

	ttl7474_clear_w (ttl7474_1a_2, 1);
	ttl7474_preset_w(ttl7474_1a_2, 1);


	/* set up the pedal handling chips */
	ttl74153_enable_w(ttl74153_1k, 0, 0);
	ttl74153_enable_w(ttl74153_1k, 1, 0);
}
