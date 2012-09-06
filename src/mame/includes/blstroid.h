/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

#include "machine/atarigen.h"

class blstroid_state : public atarigen_state
{
public:
	blstroid_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_priorityram(*this, "priorityram") { }

	required_shared_ptr<UINT16>	m_priorityram;
	DECLARE_WRITE16_MEMBER(blstroid_halt_until_hblank_0_w);
	DECLARE_READ16_MEMBER(inputs_r);
	DECLARE_DRIVER_INIT(blstroid);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
};


/*----------- defined in video/blstroid.c -----------*/

VIDEO_START( blstroid );
SCREEN_UPDATE_IND16( blstroid );

void blstroid_scanline_update(screen_device &screen, int scanline);
