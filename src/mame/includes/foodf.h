/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "machine/x2212.h"

class foodf_state : public atarigen_state
{
public:
	foodf_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_device<x2212_device> m_nvram;

	double			m_rweights[3];
	double			m_gweights[3];
	double			m_bweights[2];
	UINT8			m_playfield_flip;

	UINT8			m_whichport;
	UINT16 *m_spriteram;
};


/*----------- defined in video/foodf.c -----------*/

WRITE16_HANDLER( foodf_paletteram_w );

void foodf_set_flip(foodf_state *state, int flip);
VIDEO_START( foodf );
SCREEN_UPDATE( foodf );
