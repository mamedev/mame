/*************************************************************************

    Atari Video Pinball hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define VIDEOPIN_OCTAVE_DATA	NODE_01
#define VIDEOPIN_NOTE_DATA		NODE_02
#define VIDEOPIN_BELL_EN		NODE_03
#define VIDEOPIN_BONG_EN		NODE_04
#define VIDEOPIN_ATTRACT_EN		NODE_05
#define VIDEOPIN_VOL_DATA		NODE_06


class videopin_state : public driver_device
{
public:
	videopin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	attotime m_time_pushed;
	attotime m_time_released;
	UINT8 m_prev;
	UINT8 m_mask;
	UINT8* m_video_ram;
	int m_ball_x;
	int m_ball_y;
	tilemap_t* m_bg_tilemap;
	DECLARE_READ8_MEMBER(videopin_misc_r);
	DECLARE_WRITE8_MEMBER(videopin_led_w);
	DECLARE_WRITE8_MEMBER(videopin_ball_w);
	DECLARE_WRITE8_MEMBER(videopin_video_ram_w);
};


/*----------- defined in audio/videopin.c -----------*/

DISCRETE_SOUND_EXTERN( videopin );


/*----------- defined in video/videopin.c -----------*/


VIDEO_START( videopin );
SCREEN_UPDATE_IND16( videopin );

