// license:BSD-3-Clause
// copyright-holders:Paul Leaman
#include "video/bufsprite.h"

class srumbler_state : public driver_device
{
public:
	srumbler_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_backgroundram(*this, "backgroundram"),
		m_foregroundram(*this, "foregroundram") { }

	required_device<cpu_device> m_maincpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_backgroundram;
	required_shared_ptr<UINT8> m_foregroundram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_scroll[4];

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(foreground_w);
	DECLARE_WRITE8_MEMBER(background_w);
	DECLARE_WRITE8_MEMBER(_4009_w);
	DECLARE_WRITE8_MEMBER(scroll_w);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
};
