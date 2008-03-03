/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

****************************************************************************/


typedef struct _stactics_state stactics_state;
struct _stactics_state
{
	/* machine state */
	int    vert_pos;
	int    horiz_pos;
	UINT8 *motor_on;

	/* video state */
	UINT8 *videoram_b;
	UINT8 *videoram_d;
	UINT8 *videoram_e;
	UINT8 *videoram_f;
	UINT8 *palette;
	UINT8 *display_buffer;
	UINT8 *lamps;

	UINT8  y_scroll_d;
	UINT8  y_scroll_e;
	UINT8  y_scroll_f;
	UINT8  frame_count;
	UINT8  shot_standby;
	UINT8  shot_arrive;
	UINT16 beam_state;
	UINT16 old_beam_state;
	UINT16 beam_states_per_frame;
};


/*----------- defined in video/stactics.c -----------*/

MACHINE_DRIVER_EXTERN( stactics_video );

WRITE8_HANDLER( stactics_palette_w );
WRITE8_HANDLER( stactics_scroll_ram_w );
WRITE8_HANDLER( stactics_speed_latch_w );
WRITE8_HANDLER( stactics_shot_trigger_w );
WRITE8_HANDLER( stactics_shot_flag_clear_w );
CUSTOM_INPUT( stactics_get_frame_count_d3 );
CUSTOM_INPUT( stactics_get_shot_standby );
CUSTOM_INPUT( stactics_get_not_shot_arrive );

