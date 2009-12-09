/*************************************************************************

    Atari Xybots hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _xybots_state xybots_state;
struct _xybots_state
{
	atarigen_state	atarigen;

	int 			h256;
};


/*----------- defined in video/xybots.c -----------*/

VIDEO_START( xybots );
VIDEO_UPDATE( xybots );
