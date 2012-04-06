#include "video/bufsprite.h"

class srumbler_state : public driver_device
{
public:
	srumbler_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram")
		{ }

	UINT8 *m_backgroundram;
	UINT8 *m_foregroundram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_scroll[4];

	required_device<cpu_device> m_maincpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_WRITE8_MEMBER(srumbler_bankswitch_w);
	DECLARE_WRITE8_MEMBER(srumbler_foreground_w);
	DECLARE_WRITE8_MEMBER(srumbler_background_w);
	DECLARE_WRITE8_MEMBER(srumbler_4009_w);
	DECLARE_WRITE8_MEMBER(srumbler_scroll_w);
};


/*----------- defined in video/srumbler.c -----------*/


VIDEO_START( srumbler );
SCREEN_UPDATE_IND16( srumbler );
SCREEN_VBLANK( srumbler );
