/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

#include "driver.h"
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
#define CAR_CAR_PRIORITY_LINE	 	2
#define CAR_GOAL_PRIORITY_LINE	 	1
#define PRI0_PRIORTITY_LINE		    0

/* priority 0 controls three different things */
#define TIMER_EXTRA_BITS			0x00
#define CAR_BALL_EXTRA_BITS			0x40
#define CAR_BORDER_EXTRA_BITS		0x50


#define TTL74148_3S					0

#define TTL74153_1K					0

#define TTL7474_2S_1				0
#define TTL7474_2S_2				1
#define TTL7474_2U_1				2
#define TTL7474_2U_2				3
#define TTL7474_1F_1				4
#define TTL7474_1F_2				5
#define TTL7474_1D_1				6
#define TTL7474_1D_2				7
#define TTL7474_1C_1				8
#define TTL7474_1C_2				9
#define TTL7474_1A_1				10
#define TTL7474_1A_2				11


static UINT8 ball_screen_collision_cause;
static UINT8 car_ball_collision_x;
static UINT8 car_ball_collision_y;
static UINT8 car_car_collision_cause;
static UINT8 car_goal_collision_cause;
static UINT8 car_ball_collision_cause;
static UINT8 car_border_collision_cause;
static UINT8 priority_0_extension;
static UINT8 last_wheel_value[4];


static void TTL74148_3S_cb(void)
{
	cpunum_set_input_line(0, M6502_IRQ_LINE, TTL74148_output_valid_r(TTL74148_3S) ? CLEAR_LINE : ASSERT_LINE);
}


/* the outputs of the flip-flops are connected to the priority encoder */
static void TTL7474_2S_1_cb(void)
{
	TTL74148_input_line_w(TTL74148_3S, COIN1_PRIORITY_LINE, TTL7474_output_comp_r(TTL7474_2S_1));
	TTL74148_update(TTL74148_3S);
}

static void TTL7474_2S_2_cb(void)
{
	TTL74148_input_line_w(TTL74148_3S, COIN2_PRIORITY_LINE, TTL7474_output_comp_r(TTL7474_2S_2));
	TTL74148_update(TTL74148_3S);
}

static void TTL7474_2U_1_cb(void)
{
	TTL74148_input_line_w(TTL74148_3S, COIN3_PRIORITY_LINE, TTL7474_output_comp_r(TTL7474_2U_1));
	TTL74148_update(TTL74148_3S);
}

static void TTL7474_2U_2_cb(void)
{
	TTL74148_input_line_w(TTL74148_3S, COIN4_PRIORITY_LINE, TTL7474_output_comp_r(TTL7474_2U_2));
	TTL74148_update(TTL74148_3S);
}


void carpolo_generate_ball_screen_interrupt(UINT8 cause)
{
	ball_screen_collision_cause = cause;

	TTL74148_input_line_w(TTL74148_3S, BALL_SCREEN_PRIORITY_LINE, 0);
	TTL74148_update(TTL74148_3S);
}

void carpolo_generate_car_car_interrupt(int car1, int car2)
{
	car_car_collision_cause = ~((1 << (3 - car1)) | (1 << (3 - car2)));

	TTL74148_input_line_w(TTL74148_3S, CAR_CAR_PRIORITY_LINE, 0);
	TTL74148_update(TTL74148_3S);
}

void carpolo_generate_car_goal_interrupt(int car, int right_goal)
{
	car_goal_collision_cause = car | (right_goal ? 0x08 : 0x00);

	TTL74148_input_line_w(TTL74148_3S, CAR_GOAL_PRIORITY_LINE, 0);
	TTL74148_update(TTL74148_3S);
}

void carpolo_generate_car_ball_interrupt(int car, int car_x, int car_y)
{
	car_ball_collision_cause = car;
	car_ball_collision_x = car_x;
	car_ball_collision_y = car_y;

	priority_0_extension = CAR_BALL_EXTRA_BITS;

	TTL74148_input_line_w(TTL74148_3S, PRI0_PRIORTITY_LINE, 0);
	TTL74148_update(TTL74148_3S);
}

void carpolo_generate_car_border_interrupt(int car, int horizontal_border)
{
	car_border_collision_cause = car | (horizontal_border ? 0x04 : 0x00);

	priority_0_extension = CAR_BORDER_EXTRA_BITS;

	TTL74148_input_line_w(TTL74148_3S, PRI0_PRIORTITY_LINE, 0);
	TTL74148_update(TTL74148_3S);
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
	return (TTL74148_output_r(TTL74148_3S) << 1) | priority_0_extension;
}


INTERRUPT_GEN( carpolo_timer_interrupt )
{
	UINT8 port_value;
	int player;


	/* cause the timer interrupt */
	TTL74148_input_line_w(TTL74148_3S, PRI0_PRIORTITY_LINE, 0);
	priority_0_extension = TIMER_EXTRA_BITS;

	TTL74148_update(TTL74148_3S);


	/* check the coins here as well - they drive the clock of the flip-flops */
	port_value = readinputport(0);

	TTL7474_clock_w(TTL7474_2S_1, port_value & 0x01);
	TTL7474_clock_w(TTL7474_2S_2, port_value & 0x02);
	TTL7474_clock_w(TTL7474_2U_1, port_value & 0x04);
	TTL7474_clock_w(TTL7474_2U_2, port_value & 0x08);

	TTL7474_update(TTL7474_2S_1);
	TTL7474_update(TTL7474_2S_2);
	TTL7474_update(TTL7474_2U_1);
	TTL7474_update(TTL7474_2U_2);


	/* read the steering controls */
	for (player = 0; player < 4; player++)
	{
		int movement_flip_flop = TTL7474_1F_1 + (2 * player);
		int dir_flip_flop      = movement_flip_flop + 1;


		port_value = readinputport(2 + player);

		if (port_value != last_wheel_value[player])
		{
			/* set the movement direction */
			TTL7474_d_w(dir_flip_flop, ((port_value - last_wheel_value[player]) & 0x80) ? 1 : 0);

			last_wheel_value[player] = port_value;
		}

		/* as the wheel moves, both flip-flops are clocked */
		TTL7474_clock_w(movement_flip_flop, port_value & 0x01);
		TTL7474_clock_w(dir_flip_flop,      port_value & 0x01);

		TTL7474_update(movement_flip_flop);
		TTL7474_update(dir_flip_flop);
	}



	/* finally read the accelerator pedals */
	port_value = readinputport(6);

	for (player = 0; player < 4; player++)
	{
		/* one line indicates if the pedal is pressed and the other
           how much, resulting in only two different possible levels */
		if (port_value & 0x01)
		{
			TTL74153_input_line_w(TTL74153_1K, 0, player, 1);
			TTL74153_input_line_w(TTL74153_1K, 1, player, 0);
		}
		else if (port_value & 0x02)
		{
			TTL74153_input_line_w(TTL74153_1K, 0, player, 1);
			TTL74153_input_line_w(TTL74153_1K, 1, player, 1);
		}
		else
		{
			TTL74153_input_line_w(TTL74153_1K, 0, player, 0);
			/* the other line is irrelevant */
		}

		port_value >>= 2;
	}

	TTL74153_update(TTL74153_1K);
}


static WRITE8_HANDLER( coin1_interrupt_clear_w )
{
	TTL7474_clear_w(TTL7474_2S_1, data);
	TTL7474_update(TTL7474_2S_1);
}

static WRITE8_HANDLER( coin2_interrupt_clear_w )
{
	TTL7474_clear_w(TTL7474_2S_2, data);
	TTL7474_update(TTL7474_2S_2);
}

static WRITE8_HANDLER( coin3_interrupt_clear_w )
{
	TTL7474_clear_w(TTL7474_2U_1, data);
	TTL7474_update(TTL7474_2U_1);
}

static WRITE8_HANDLER( coin4_interrupt_clear_w )
{
	TTL7474_clear_w(TTL7474_2U_2, data);
	TTL7474_update(TTL7474_2U_2);
}

WRITE8_HANDLER( carpolo_ball_screen_interrupt_clear_w )
{
	TTL74148_input_line_w(TTL74148_3S, BALL_SCREEN_PRIORITY_LINE, 1);
	TTL74148_update(TTL74148_3S);
}

WRITE8_HANDLER( carpolo_car_car_interrupt_clear_w )
{
	TTL74148_input_line_w(TTL74148_3S, CAR_CAR_PRIORITY_LINE, 1);
	TTL74148_update(TTL74148_3S);
}

WRITE8_HANDLER( carpolo_car_goal_interrupt_clear_w )
{
	TTL74148_input_line_w(TTL74148_3S, CAR_GOAL_PRIORITY_LINE, 1);
	TTL74148_update(TTL74148_3S);
}

WRITE8_HANDLER( carpolo_car_ball_interrupt_clear_w )
{
	TTL74148_input_line_w(TTL74148_3S, PRI0_PRIORTITY_LINE, 1);
	TTL74148_update(TTL74148_3S);
}

WRITE8_HANDLER( carpolo_car_border_interrupt_clear_w )
{
	TTL74148_input_line_w(TTL74148_3S, PRI0_PRIORTITY_LINE, 1);
	TTL74148_update(TTL74148_3S);
}

WRITE8_HANDLER( carpolo_timer_interrupt_clear_w )
{
	TTL74148_input_line_w(TTL74148_3S, PRI0_PRIORTITY_LINE, 1);
	TTL74148_update(TTL74148_3S);
}


/*************************************
 *
 *  Input port handling
 *
 *************************************/

static WRITE8_HANDLER( pia_0_port_a_w )
{
	/* bit 0 - Coin counter
       bit 1 - Player 4 crash sound
       bit 2 - Player 3 crash sound
       bit 3 - Clear steering wheel logic
       bit 4 - Player 2 crash sound
       bit 5 - Score pulse sound
       bit 6 - Player 1 crash sound
       bit 7 - Ball hit pulse sound */

	coin_counter_w(0, data & 0x01);


	TTL7474_clear_w(TTL7474_1F_1, data & 0x08);
	TTL7474_clear_w(TTL7474_1D_1, data & 0x08);
	TTL7474_clear_w(TTL7474_1C_1, data & 0x08);
	TTL7474_clear_w(TTL7474_1A_1, data & 0x08);

	TTL7474_update(TTL7474_1F_1);
	TTL7474_update(TTL7474_1D_1);
	TTL7474_update(TTL7474_1C_1);
	TTL7474_update(TTL7474_1A_1);
}


static WRITE8_HANDLER( pia_0_port_b_w )
{
	/* bit 0 - Strobe speed bits sound
       bit 1 - Speed bit 0 sound
       bit 2 - Speed bit 1 sound
       bit 3 - Speed bit 2 sound
       bit 6 - Select pedal 0
       bit 7 - Select pdeal 1 */

	TTL74153_a_w(TTL74153_1K, data & 0x40);
	TTL74153_b_w(TTL74153_1K, data & 0x80);

	TTL74153_update(TTL74153_1K);
}

static READ8_HANDLER( pia_0_port_b_r )
{
	/* bit 4 - Pedal bit 0
       bit 5 - Pedal bit 1 */

	return (TTL74153_output_r(TTL74153_1K, 0) << 5) |
		   (TTL74153_output_r(TTL74153_1K, 1) << 4);
}


static READ8_HANDLER( pia_1_port_a_r )
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

	ret = (TTL7474_output_r(TTL7474_1A_2) ? 0x01 : 0x00) |
		  (TTL7474_output_r(TTL7474_1C_2) ? 0x02 : 0x00) |
		  (TTL7474_output_r(TTL7474_1D_2) ? 0x04 : 0x00) |
		  (TTL7474_output_r(TTL7474_1F_2) ? 0x08 : 0x00) |
		  (readinputport(7) & 0xf0);

	return ret;
}


static READ8_HANDLER( pia_1_port_b_r )
{
	UINT8 ret;

	/* bit 4 - Player 4 steering input (wheel moving or stopped)
       bit 5 - Player 3 steering input (wheel moving or stopped)
       bit 6 - Player 2 steering input (wheel moving or stopped)
       bit 7 - Player 1 steering input (wheel moving or stopped) */

	ret = (TTL7474_output_r(TTL7474_1A_1) ? 0x10 : 0x00) |
		  (TTL7474_output_r(TTL7474_1C_1) ? 0x20 : 0x00) |
		  (TTL7474_output_r(TTL7474_1D_1) ? 0x40 : 0x00) |
		  (TTL7474_output_r(TTL7474_1F_1) ? 0x80 : 0x00);

	return ret;
}


static const pia6821_interface pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, pia_0_port_b_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_0_port_a_w, pia_0_port_b_w, coin1_interrupt_clear_w, coin2_interrupt_clear_w,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ pia_1_port_a_r, pia_1_port_b_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, coin3_interrupt_clear_w, coin4_interrupt_clear_w,
	/*irqs   : A/B             */ 0, 0
};

static struct TTL74148_interface TTL74148_3S_intf =
{
	TTL74148_3S_cb
};

static struct TTL7474_interface TTL7474_2S_1_intf =
{
	TTL7474_2S_1_cb
};

static struct TTL7474_interface TTL7474_2S_2_intf =
{
	TTL7474_2S_2_cb
};

static struct TTL7474_interface TTL7474_2U_1_intf =
{
	TTL7474_2U_1_cb
};

static struct TTL7474_interface TTL7474_2U_2_intf =
{
	TTL7474_2U_2_cb
};


MACHINE_START( carpolo )
{
	/* set up the PIA's */
	pia_config(0, &pia_0_intf);
	pia_config(1, &pia_1_intf);
}

MACHINE_RESET( carpolo )
{
	/* set up the priority encoder */
	TTL74148_config(TTL74148_3S, &TTL74148_3S_intf);
	TTL74148_enable_input_w(TTL74148_3S, 0);	/* always enabled */


	/* set up the coin handling flip-flops */
	TTL7474_config(TTL7474_2S_1, &TTL7474_2S_1_intf);
	TTL7474_config(TTL7474_2S_2, &TTL7474_2S_2_intf);
	TTL7474_config(TTL7474_2U_1, &TTL7474_2U_1_intf);
	TTL7474_config(TTL7474_2U_2, &TTL7474_2U_2_intf);

	TTL7474_d_w     (TTL7474_2S_1, 1);
	TTL7474_preset_w(TTL7474_2S_1, 1);

	TTL7474_d_w     (TTL7474_2S_2, 1);
	TTL7474_preset_w(TTL7474_2S_2, 1);

	TTL7474_d_w     (TTL7474_2U_1, 1);
	TTL7474_preset_w(TTL7474_2U_1, 1);

	TTL7474_d_w     (TTL7474_2U_2, 1);
	TTL7474_preset_w(TTL7474_2U_2, 1);


	/* set up the steering handling flip-flops */
	TTL7474_config(TTL7474_1F_1, 0);
	TTL7474_config(TTL7474_1F_2, 0);
	TTL7474_config(TTL7474_1D_1, 0);
	TTL7474_config(TTL7474_1D_2, 0);
	TTL7474_config(TTL7474_1C_1, 0);
	TTL7474_config(TTL7474_1C_2, 0);
	TTL7474_config(TTL7474_1A_1, 0);
	TTL7474_config(TTL7474_1A_2, 0);

	TTL7474_d_w     (TTL7474_1F_1, 1);
	TTL7474_preset_w(TTL7474_1F_1, 1);

	TTL7474_clear_w (TTL7474_1F_2, 1);
	TTL7474_preset_w(TTL7474_1F_2, 1);

	TTL7474_d_w     (TTL7474_1D_1, 1);
	TTL7474_preset_w(TTL7474_1D_1, 1);

	TTL7474_clear_w (TTL7474_1D_2, 1);
	TTL7474_preset_w(TTL7474_1D_2, 1);

	TTL7474_d_w     (TTL7474_1C_1, 1);
	TTL7474_preset_w(TTL7474_1C_1, 1);

	TTL7474_clear_w (TTL7474_1C_2, 1);
	TTL7474_preset_w(TTL7474_1C_2, 1);

	TTL7474_d_w     (TTL7474_1A_1, 1);
	TTL7474_preset_w(TTL7474_1A_1, 1);

	TTL7474_clear_w (TTL7474_1A_2, 1);
	TTL7474_preset_w(TTL7474_1A_2, 1);


	/* set up the pedal handling chips */
	TTL74153_config(TTL74153_1K, 0);

	TTL74153_enable_w(TTL74153_1K, 0, 0);
	TTL74153_enable_w(TTL74153_1K, 1, 0);
}
