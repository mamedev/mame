/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _foodf_state foodf_state;
struct _foodf_state
{
	atarigen_state	atarigen;

	double			rweights[3];
	double			gweights[3];
	double			bweights[2];
	UINT8			playfield_flip;

	UINT8			whichport;
};


/*----------- defined in video/foodf.c -----------*/

WRITE16_HANDLER( foodf_paletteram_w );

void foodf_set_flip(foodf_state *state, int flip);
VIDEO_START( foodf );
VIDEO_UPDATE( foodf );
