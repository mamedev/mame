/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

/*----------- defined in machine/carpolo.c -----------*/

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

void carpolo_generate_car_car_interrupt(int car1, int car2);
void carpolo_generate_ball_screen_interrupt(UINT8 cause);
void carpolo_generate_car_goal_interrupt(int car, int right_goal);
void carpolo_generate_car_ball_interrupt(int car, int car_x, int car_y);
void carpolo_generate_car_border_interrupt(int car, int horizontal_border);


/*----------- defined in video/carpolo.c -----------*/

extern UINT8 *carpolo_alpharam;
extern UINT8 *carpolo_spriteram;

PALETTE_INIT( carpolo );
VIDEO_START( carpolo );
VIDEO_UPDATE( carpolo );
VIDEO_EOF( carpolo );
