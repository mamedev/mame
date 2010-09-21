/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "machine/x2212.h"

class foodf_state : public atarigen_state
{
public:
	foodf_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config),
		  m_nvram(*this, "nvram") { }

	required_device<x2212_device> m_nvram;

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
