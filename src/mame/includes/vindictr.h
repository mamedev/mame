/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/

#include "machine/atarigen.h"

class vindictr_state : public atarigen_state
{
public:
	vindictr_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_playfield_tile_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;
	DECLARE_READ16_MEMBER(port1_r);
};


/*----------- defined in video/vindictr.c -----------*/

WRITE16_HANDLER( vindictr_paletteram_w );

VIDEO_START( vindictr );
SCREEN_UPDATE_IND16( vindictr );

void vindictr_scanline_update(screen_device &screen, int scanline);
