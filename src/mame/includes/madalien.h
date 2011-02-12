/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "sound/discrete.h"


#define MADALIEN_MAIN_CLOCK		XTAL_10_595MHz


class madalien_state : public driver_device
{
public:
	madalien_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *shift_hi;
	UINT8 *shift_lo;
	UINT8 *videoram;
	UINT8 *charram;
	UINT8 *video_flags;
	UINT8 *video_control;
	UINT8 *scroll;
	UINT8 *edge1_pos;
	UINT8 *edge2_pos;
	UINT8 *headlight_pos;
	tilemap_t *tilemap_fg;
	tilemap_t *tilemap_edge1[4];
	tilemap_t *tilemap_edge2[4];
	bitmap_t *headlight_bitmap;
};


/*----------- defined in video/madalien.c -----------*/

MACHINE_CONFIG_EXTERN( madalien_video );

WRITE8_HANDLER( madalien_videoram_w );
WRITE8_HANDLER( madalien_charram_w );


/*----------- defined in audio/madalien.c -----------*/

DISCRETE_SOUND_EXTERN( madalien );

/* Discrete Sound Input Nodes */
#define MADALIEN_8910_PORTA			NODE_01
#define MADALIEN_8910_PORTB			NODE_02
