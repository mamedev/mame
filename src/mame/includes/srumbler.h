#include "video/bufsprite.h"

class srumbler_state : public driver_device
{
public:
	srumbler_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_backgroundram(*this, "backgroundram"),
		m_foregroundram(*this, "foregroundram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<UINT8> m_backgroundram;
	required_shared_ptr<UINT8> m_foregroundram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_scroll[4];

	DECLARE_WRITE8_MEMBER(srumbler_bankswitch_w);
	DECLARE_WRITE8_MEMBER(srumbler_foreground_w);
	DECLARE_WRITE8_MEMBER(srumbler_background_w);
	DECLARE_WRITE8_MEMBER(srumbler_4009_w);
	DECLARE_WRITE8_MEMBER(srumbler_scroll_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_srumbler(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
