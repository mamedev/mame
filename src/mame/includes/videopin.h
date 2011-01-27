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
	videopin_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	attotime time_pushed;
	attotime time_released;
	UINT8 prev;
	UINT8 mask;
	UINT8* video_ram;
	int ball_x;
	int ball_y;
	tilemap_t* bg_tilemap;
};


/*----------- defined in audio/videopin.c -----------*/

DISCRETE_SOUND_EXTERN( videopin );


/*----------- defined in video/videopin.c -----------*/

WRITE8_HANDLER( videopin_video_ram_w );
WRITE8_HANDLER( videopin_ball_w );

VIDEO_START( videopin );
VIDEO_UPDATE( videopin );

