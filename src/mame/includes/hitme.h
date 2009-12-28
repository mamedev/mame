/*************************************************************************

    Hitme hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define HITME_DOWNCOUNT_VAL      NODE_01
#define HITME_OUT0               NODE_02
#define HITME_ENABLE_VAL         NODE_03
#define HITME_OUT1               NODE_04

typedef struct _hitme_state hitme_state;
struct _hitme_state
{
	/* memory pointers */
	UINT8 *  videoram;

	/* video-related */
	tilemap_t  *tilemap;

	/* misc */
	attotime timeout_time;
};


/*----------- defined in audio/hitme.c -----------*/

DISCRETE_SOUND_EXTERN( hitme );
