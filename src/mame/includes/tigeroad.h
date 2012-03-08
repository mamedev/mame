#include "video/bufsprite.h"

class tigeroad_state : public driver_device
{
public:
	tigeroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	UINT16 *m_videoram;
	UINT16 *m_ram16;
	int m_bgcharbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	required_device<buffered_spriteram16_device> m_spriteram;
};


/*----------- defined in video/tigeroad.c -----------*/

WRITE16_HANDLER( tigeroad_videoram_w );
WRITE16_HANDLER( tigeroad_videoctrl_w );
WRITE16_HANDLER( tigeroad_scroll_w );
VIDEO_START( tigeroad );
SCREEN_UPDATE_IND16( tigeroad );
