/*************************************************************************

    Atari Night Driver hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define NITEDRVR_BANG_DATA	NODE_01
#define NITEDRVR_SKID1_EN	NODE_02
#define NITEDRVR_SKID2_EN	NODE_03
#define NITEDRVR_MOTOR_DATA	NODE_04
#define NITEDRVR_CRASH_EN	NODE_05
#define NITEDRVR_ATTRACT_EN	NODE_06


class nitedrvr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, nitedrvr_state(machine)); }

	nitedrvr_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    hvc;
	UINT8 *    videoram;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* input */
	UINT8 gear;
	UINT8 track;
	INT32 steering_buf;
	INT32 steering_val;
	UINT8 crash_en;
	UINT8 crash_data;
	UINT8 crash_data_en;	// IC D8
	UINT8 ac_line;
	INT32 last_steering_val;

	/* devices */
	running_device *maincpu;
	running_device *discrete;
};


/*----------- defined in machine/nitedrvr.c -----------*/

READ8_HANDLER( nitedrvr_in0_r );
READ8_HANDLER( nitedrvr_in1_r );
READ8_HANDLER( nitedrvr_steering_reset_r );
WRITE8_HANDLER( nitedrvr_steering_reset_w );
WRITE8_HANDLER( nitedrvr_out0_w );
WRITE8_HANDLER( nitedrvr_out1_w );

TIMER_DEVICE_CALLBACK( nitedrvr_crash_toggle_callback );

MACHINE_RESET( nitedrvr );
MACHINE_START( nitedrvr );


/*----------- defined in audio/nitedrvr.c -----------*/

DISCRETE_SOUND_EXTERN( nitedrvr );


/*----------- defined in video/nitedrvr.c -----------*/

WRITE8_HANDLER( nitedrvr_hvc_w );
WRITE8_HANDLER( nitedrvr_videoram_w );

VIDEO_START( nitedrvr );
VIDEO_UPDATE( nitedrvr );
