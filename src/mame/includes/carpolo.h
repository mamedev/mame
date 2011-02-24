/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

#include "machine/6821pia.h"

class carpolo_state : public driver_device
{
public:
	carpolo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *alpharam;
	UINT8 *spriteram;
	UINT8 ball_screen_collision_cause;
	UINT8 car_ball_collision_x;
	UINT8 car_ball_collision_y;
	UINT8 car_car_collision_cause;
	UINT8 car_goal_collision_cause;
	UINT8 car_ball_collision_cause;
	UINT8 car_border_collision_cause;
	UINT8 priority_0_extension;
	UINT8 last_wheel_value[4];
	device_t *ttl74148_3s;
	device_t *ttl74153_1k;
	device_t *ttl7474_2s_1;
	device_t *ttl7474_2s_2;
	device_t *ttl7474_2u_1;
	device_t *ttl7474_2u_2;
	device_t *ttl7474_1f_1;
	device_t *ttl7474_1f_2;
	device_t *ttl7474_1d_1;
	device_t *ttl7474_1d_2;
	device_t *ttl7474_1c_1;
	device_t *ttl7474_1c_2;
	device_t *ttl7474_1a_1;
	device_t *ttl7474_1a_2;
	bitmap_t *sprite_sprite_collision_bitmap1;
	bitmap_t *sprite_sprite_collision_bitmap2;
	bitmap_t *sprite_goal_collision_bitmap1;
	bitmap_t *sprite_goal_collision_bitmap2;
	bitmap_t *sprite_border_collision_bitmap;
};


/*----------- defined in machine/carpolo.c -----------*/

extern const pia6821_interface carpolo_pia0_intf;
extern const pia6821_interface carpolo_pia1_intf;

void carpolo_74148_3s_cb(device_t *device);

WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2s_1_q_cb );
WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2s_2_q_cb );
WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2u_1_q_cb );
WRITE_LINE_DEVICE_HANDLER( carpolo_7474_2u_2_q_cb );

MACHINE_START( carpolo );
MACHINE_RESET( carpolo );

READ8_HANDLER( carpolo_interrupt_cause_r );

READ8_HANDLER( carpolo_ball_screen_collision_cause_r );
READ8_HANDLER( carpolo_car_ball_collision_x_r );
READ8_HANDLER( carpolo_car_ball_collision_y_r );
READ8_HANDLER( carpolo_car_car_collision_cause_r );
READ8_HANDLER( carpolo_car_goal_collision_cause_r );
READ8_HANDLER( carpolo_car_ball_collision_cause_r );
READ8_HANDLER( carpolo_car_border_collision_cause_r );

INTERRUPT_GEN( carpolo_timer_interrupt );

WRITE8_HANDLER( carpolo_ball_screen_interrupt_clear_w );
WRITE8_HANDLER( carpolo_car_car_interrupt_clear_w );
WRITE8_HANDLER( carpolo_car_goal_interrupt_clear_w );
WRITE8_HANDLER( carpolo_car_ball_interrupt_clear_w );
WRITE8_HANDLER( carpolo_car_border_interrupt_clear_w );
WRITE8_HANDLER( carpolo_timer_interrupt_clear_w );

void carpolo_generate_car_car_interrupt(running_machine *machine, int car1, int car2);
void carpolo_generate_ball_screen_interrupt(running_machine *machine, UINT8 cause);
void carpolo_generate_car_goal_interrupt(running_machine *machine, int car, int right_goal);
void carpolo_generate_car_ball_interrupt(running_machine *machine, int car, int car_x, int car_y);
void carpolo_generate_car_border_interrupt(running_machine *machine, int car, int horizontal_border);


/*----------- defined in video/carpolo.c -----------*/

PALETTE_INIT( carpolo );
VIDEO_START( carpolo );
SCREEN_UPDATE( carpolo );
SCREEN_EOF( carpolo );
