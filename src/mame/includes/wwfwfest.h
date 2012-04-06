#include "video/bufsprite.h"

class wwfwfest_state : public driver_device
{
public:
	wwfwfest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	UINT16 *m_fg0_videoram;
	UINT16 *m_bg0_videoram;
	UINT16 *m_bg1_videoram;
	UINT16 m_pri;
	UINT16 m_bg0_scrollx;
	UINT16 m_bg0_scrolly;
	UINT16 m_bg1_scrollx;
	UINT16 m_bg1_scrolly;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;
	tilemap_t *m_bg1_tilemap;
	UINT16 m_sprite_xoff;
	UINT16 m_bg0_dx;
	UINT16 m_bg1_dx[2];
	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_WRITE16_MEMBER(wwfwfest_1410_write);
	DECLARE_WRITE16_MEMBER(wwfwfest_scroll_write);
	DECLARE_WRITE16_MEMBER(wwfwfest_irq_ack_w);
	DECLARE_WRITE16_MEMBER(wwfwfest_flipscreen_w);
	DECLARE_READ16_MEMBER(wwfwfest_paletteram16_xxxxBBBBGGGGRRRR_word_r);
	DECLARE_WRITE16_MEMBER(wwfwfest_paletteram16_xxxxBBBBGGGGRRRR_word_w);
	DECLARE_WRITE16_MEMBER(wwfwfest_soundwrite);
	DECLARE_WRITE16_MEMBER(wwfwfest_fg0_videoram_w);
	DECLARE_WRITE16_MEMBER(wwfwfest_bg0_videoram_w);
	DECLARE_WRITE16_MEMBER(wwfwfest_bg1_videoram_w);
};


/*----------- defined in video/wwfwfest.c -----------*/

VIDEO_START( wwfwfest );
VIDEO_START( wwfwfstb );
SCREEN_UPDATE_IND16( wwfwfest );
