/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"

class batman_state : public atarigen_state
{
public:
	batman_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16			m_latch_data;

	UINT8			m_alpha_tile_bank;
	DECLARE_READ16_MEMBER(batman_atarivc_r);
	DECLARE_WRITE16_MEMBER(batman_atarivc_w);
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(latch_w);
};


/*----------- defined in video/batman.c -----------*/

VIDEO_START( batman );
SCREEN_UPDATE_IND16( batman );

void batman_scanline_update(screen_device &screen, int scanline);
