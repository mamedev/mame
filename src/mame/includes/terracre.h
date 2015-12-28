// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
#include "video/bufsprite.h"

class terracre_state : public driver_device
{
public:
	terracre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_fg_videoram;

	const UINT16 *m_mpProtData;
	UINT8 m_mAmazonProtCmd;
	UINT8 m_mAmazonProtReg[6];
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	DECLARE_WRITE16_MEMBER(amazon_sound_w);
	DECLARE_READ8_MEMBER(soundlatch_clear_r);
	DECLARE_READ16_MEMBER(amazon_protection_r);
	DECLARE_WRITE16_MEMBER(amazon_protection_w);
	DECLARE_WRITE16_MEMBER(amazon_background_w);
	DECLARE_WRITE16_MEMBER(amazon_foreground_w);
	DECLARE_WRITE16_MEMBER(amazon_flipscreen_w);
	DECLARE_WRITE16_MEMBER(amazon_scrolly_w);
	DECLARE_WRITE16_MEMBER(amazon_scrollx_w);
	DECLARE_DRIVER_INIT(amazon);
	DECLARE_DRIVER_INIT(amatelas);
	DECLARE_DRIVER_INIT(horekid);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(terracre);
	DECLARE_MACHINE_START(amazon);
	UINT32 screen_update_amazon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
