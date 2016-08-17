// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "audio/seibu.h"

class deadang_state : public driver_device
{
public:
	deadang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_seibu_sound(*this, "seibu_sound"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_video_data(*this, "video_data") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<seibu_adpcm_device> m_adpcm1;
	required_device<seibu_adpcm_device> m_adpcm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_scroll_ram;
	required_shared_ptr<UINT16> m_video_data;

	tilemap_t *m_pf3_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_layer;
	tilemap_t *m_text_layer;
	int m_tilebank;
	int m_oldtilebank;

	DECLARE_WRITE16_MEMBER(foreground_w);
	DECLARE_WRITE16_MEMBER(text_w);
	DECLARE_WRITE16_MEMBER(bank_w);
	DECLARE_READ16_MEMBER(ghunter_trackball_low_r);
	DECLARE_READ16_MEMBER(ghunter_trackball_high_r);

	DECLARE_DRIVER_INIT(deadang);
	DECLARE_DRIVER_INIT(ghunter);
	virtual void video_start() override;

	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILE_GET_INFO_MEMBER(get_pf3_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(main_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(sub_scanline);
};
