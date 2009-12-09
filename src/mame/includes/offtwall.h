/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _offtwall_state offtwall_state;
struct _offtwall_state
{
	atarigen_state	atarigen;
};


/*----------- defined in video/offtwall.c -----------*/

VIDEO_START( offtwall );
VIDEO_UPDATE( offtwall );
