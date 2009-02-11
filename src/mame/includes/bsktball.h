/*************************************************************************

    Atari Basketball hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define BSKTBALL_NOTE_DATA		NODE_01
#define BSKTBALL_CROWD_DATA		NODE_02
#define BSKTBALL_NOISE_EN		NODE_03
#define BSKTBALL_BOUNCE_EN		NODE_04


/*----------- defined in machine/bsktball.c -----------*/

WRITE8_HANDLER( bsktball_nmion_w );
INTERRUPT_GEN( bsktball_interrupt );
WRITE8_HANDLER( bsktball_ld1_w );
WRITE8_HANDLER( bsktball_ld2_w );
READ8_HANDLER( bsktball_in0_r );
WRITE8_HANDLER( bsktball_led1_w );
WRITE8_HANDLER( bsktball_led2_w );


/*----------- defined in audio/bsktball.c -----------*/

WRITE8_DEVICE_HANDLER( bsktball_bounce_w );
WRITE8_DEVICE_HANDLER( bsktball_note_w );
WRITE8_DEVICE_HANDLER( bsktball_noise_reset_w );

DISCRETE_SOUND_EXTERN( bsktball );

/*----------- defined in video/bsktball.c -----------*/

extern UINT8 *bsktball_motion;

VIDEO_START( bsktball );
VIDEO_UPDATE( bsktball );
WRITE8_HANDLER( bsktball_videoram_w );

