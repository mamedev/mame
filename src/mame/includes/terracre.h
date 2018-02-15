// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano

#include "machine/gen_latch.h"
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
		m_soundlatch(*this, "soundlatch"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;

	// move to 1412m2
	uint8_t m_mAmazonProtCmd;
	uint8_t m_mAmazonProtReg[6];

	uint16_t m_xscroll;
	uint16_t m_yscroll;
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
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(terracre);
	DECLARE_MACHINE_START(amazon);
	uint32_t screen_update_amazon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void amazon_base(machine_config &config);
	void amazon_1412m2(machine_config &config);
	void ym2203(machine_config &config);
	void ym3526(machine_config &config);
	void amazon_1412m2_map(address_map &map);
	void amazon_base_map(address_map &map);
	void sound_2203_io_map(address_map &map);
	void sound_3526_io_map(address_map &map);
	void sound_map(address_map &map);
	void terracre_map(address_map &map);
};
