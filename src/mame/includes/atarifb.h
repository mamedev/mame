/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define ATARIFB_WHISTLE_EN		NODE_01
#define ATARIFB_CROWD_DATA		NODE_02
#define ATARIFB_ATTRACT_EN		NODE_03
#define ATARIFB_NOISE_EN		NODE_04
#define ATARIFB_HIT_EN			NODE_05


class atarifb_state : public driver_device
{
public:
	atarifb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* video-related */
	UINT8 *  alphap1_videoram;
	UINT8 *  alphap2_videoram;
	UINT8 *  field_videoram;
	UINT8 *  spriteram;
	UINT8 *  scroll_register;
	size_t   spriteram_size;

	tilemap_t  *alpha1_tilemap;
	tilemap_t  *alpha2_tilemap;
	tilemap_t  *field_tilemap;

	/* sound-related */
	int CTRLD;
	int sign_x_1, sign_y_1;
	int sign_x_2, sign_y_2;
	int sign_x_3, sign_y_3;
	int sign_x_4, sign_y_4;
	int counter_x_in0, counter_y_in0;
	int counter_x_in0b, counter_y_in0b;
	int counter_x_in2, counter_y_in2;
	int counter_x_in2b, counter_y_in2b;

	/* devices */
	running_device *maincpu;
};


/*----------- defined in machine/atarifb.c -----------*/

WRITE8_HANDLER( atarifb_out1_w );
WRITE8_HANDLER( atarifb4_out1_w );
WRITE8_HANDLER( abaseb_out1_w );
WRITE8_HANDLER( soccer_out1_w );

WRITE8_HANDLER( atarifb_out2_w );
WRITE8_HANDLER( soccer_out2_w );

WRITE8_HANDLER( atarifb_out3_w );

READ8_HANDLER( atarifb_in0_r );
READ8_HANDLER( atarifb_in2_r );
READ8_HANDLER( atarifb4_in0_r );
READ8_HANDLER( atarifb4_in2_r );


/*----------- defined in audio/atarifb.c -----------*/

DISCRETE_SOUND_EXTERN( atarifb );
DISCRETE_SOUND_EXTERN( abaseb );


/*----------- defined in video/atarifb.c -----------*/

VIDEO_START( atarifb );
VIDEO_UPDATE( atarifb );
VIDEO_UPDATE( abaseb );
VIDEO_UPDATE( soccer );

WRITE8_HANDLER( atarifb_alpha1_videoram_w );
WRITE8_HANDLER( atarifb_alpha2_videoram_w );
WRITE8_HANDLER( atarifb_field_videoram_w );
