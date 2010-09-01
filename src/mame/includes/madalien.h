/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "sound/discrete.h"


#define MADALIEN_MAIN_CLOCK		XTAL_10_595MHz


/*----------- defined in video/madalien.c -----------*/

extern UINT8 *madalien_videoram;
extern UINT8 *madalien_charram;

extern UINT8 *madalien_video_flags;
extern UINT8 *madalien_video_control;
extern UINT8 *madalien_scroll;
extern UINT8 *madalien_edge1_pos;
extern UINT8 *madalien_edge2_pos;
extern UINT8 *madalien_headlight_pos;

MACHINE_CONFIG_EXTERN( madalien_video );

WRITE8_HANDLER( madalien_videoram_w );
WRITE8_HANDLER( madalien_charram_w );


/*----------- defined in audio/madalien.c -----------*/

DISCRETE_SOUND_EXTERN( madalien );

/* Discrete Sound Input Nodes */
#define MADALIEN_8910_PORTA			NODE_01
#define MADALIEN_8910_PORTB			NODE_02
