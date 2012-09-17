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
	DECLARE_DRIVER_INIT(vindictr);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(vindictr);
	DECLARE_MACHINE_RESET(vindictr);
	DECLARE_VIDEO_START(vindictr);
};


/*----------- defined in video/vindictr.c -----------*/

DECLARE_WRITE16_HANDLER( vindictr_paletteram_w );


SCREEN_UPDATE_IND16( vindictr );

void vindictr_scanline_update(screen_device &screen, int scanline);
