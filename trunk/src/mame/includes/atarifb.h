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
	atarifb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8 *  m_alphap1_videoram;
	UINT8 *  m_alphap2_videoram;
	UINT8 *  m_field_videoram;
	UINT8 *  m_spriteram;
	UINT8 *  m_scroll_register;
	size_t   m_spriteram_size;

	tilemap_t  *m_alpha1_tilemap;
	tilemap_t  *m_alpha2_tilemap;
	tilemap_t  *m_field_tilemap;

	/* sound-related */
	int m_CTRLD;
	int m_sign_x_1;
	int m_sign_y_1;
	int m_sign_x_2;
	int m_sign_y_2;
	int m_sign_x_3;
	int m_sign_y_3;
	int m_sign_x_4;
	int m_sign_y_4;
	int m_counter_x_in0;
	int m_counter_y_in0;
	int m_counter_x_in0b;
	int m_counter_y_in0b;
	int m_counter_x_in2;
	int m_counter_y_in2;
	int m_counter_x_in2b;
	int m_counter_y_in2b;

	/* devices */
	device_t *m_maincpu;
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
SCREEN_UPDATE( atarifb );
SCREEN_UPDATE( abaseb );
SCREEN_UPDATE( soccer );

WRITE8_HANDLER( atarifb_alpha1_videoram_w );
WRITE8_HANDLER( atarifb_alpha2_videoram_w );
WRITE8_HANDLER( atarifb_field_videoram_w );
