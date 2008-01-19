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

extern UINT8 *atarifb_alphap1_videoram;
extern UINT8 *atarifb_alphap2_videoram;
extern UINT8 *atarifb_field_videoram;
extern UINT8 *atarifb_scroll_register;

VIDEO_START( atarifb );
VIDEO_UPDATE( atarifb );
VIDEO_UPDATE( abaseb );
VIDEO_UPDATE( soccer );

WRITE8_HANDLER( atarifb_alpha1_videoram_w );
WRITE8_HANDLER( atarifb_alpha2_videoram_w );
WRITE8_HANDLER( atarifb_field_videoram_w );
