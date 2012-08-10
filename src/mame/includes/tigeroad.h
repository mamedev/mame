#include "video/bufsprite.h"

class tigeroad_state : public driver_device
{
public:
	tigeroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_ram16(*this, "ram16"){ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_ram16;
	int m_bgcharbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(f1dream_control_w);
	DECLARE_WRITE16_MEMBER(tigeroad_soundcmd_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoram_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoctrl_w);
	DECLARE_WRITE16_MEMBER(tigeroad_scroll_w);
	DECLARE_WRITE8_MEMBER(msm5205_w);
	DECLARE_DRIVER_INIT(f1dream);
	DECLARE_DRIVER_INIT(tigeroad);
};


/*----------- defined in video/tigeroad.c -----------*/

VIDEO_START( tigeroad );
SCREEN_UPDATE_IND16( tigeroad );
