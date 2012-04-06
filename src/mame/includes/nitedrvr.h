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


class nitedrvr_state : public driver_device
{
public:
	nitedrvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_hvc;
	UINT8 *    m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* input */
	UINT8 m_gear;
	UINT8 m_track;
	INT32 m_steering_buf;
	INT32 m_steering_val;
	UINT8 m_crash_en;
	UINT8 m_crash_data;
	UINT8 m_crash_data_en;	// IC D8
	UINT8 m_ac_line;
	INT32 m_last_steering_val;

	/* devices */
	device_t *m_maincpu;
	device_t *m_discrete;
	DECLARE_READ8_MEMBER(nitedrvr_steering_reset_r);
	DECLARE_WRITE8_MEMBER(nitedrvr_steering_reset_w);
	DECLARE_READ8_MEMBER(nitedrvr_in0_r);
	DECLARE_READ8_MEMBER(nitedrvr_in1_r);
	DECLARE_WRITE8_MEMBER(nitedrvr_out0_w);
	DECLARE_WRITE8_MEMBER(nitedrvr_out1_w);
	DECLARE_WRITE8_MEMBER(nitedrvr_videoram_w);
	DECLARE_WRITE8_MEMBER(nitedrvr_hvc_w);
};


/*----------- defined in machine/nitedrvr.c -----------*/


TIMER_DEVICE_CALLBACK( nitedrvr_crash_toggle_callback );

MACHINE_RESET( nitedrvr );
MACHINE_START( nitedrvr );


/*----------- defined in audio/nitedrvr.c -----------*/

DISCRETE_SOUND_EXTERN( nitedrvr );


/*----------- defined in video/nitedrvr.c -----------*/


VIDEO_START( nitedrvr );
SCREEN_UPDATE_IND16( nitedrvr );
