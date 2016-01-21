// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
#include "audio/seibu.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"

class toki_state : public driver_device
{
public:
	toki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_background1_videoram(*this, "bg1_vram"),
		m_background2_videoram(*this, "bg2_vram"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_background1_videoram;
	required_shared_ptr<UINT16> m_background2_videoram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_scrollram;

	int m_msm5205next;
	int m_toggle;

	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_text_layer;

	DECLARE_WRITE16_MEMBER(tokib_soundcommand_w);
	DECLARE_READ16_MEMBER(pip_r);
	DECLARE_WRITE16_MEMBER(toki_control_w);
	DECLARE_WRITE16_MEMBER(foreground_videoram_w);
	DECLARE_WRITE16_MEMBER(background1_videoram_w);
	DECLARE_WRITE16_MEMBER(background2_videoram_w);
	DECLARE_WRITE8_MEMBER(tokib_adpcm_control_w);
	DECLARE_WRITE8_MEMBER(tokib_adpcm_data_w);
	DECLARE_WRITE_LINE_MEMBER(tokib_adpcm_int);

	DECLARE_DRIVER_INIT(tokib);
	DECLARE_DRIVER_INIT(jujuba);
	DECLARE_DRIVER_INIT(toki);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);

	virtual void video_start() override;

	UINT32 screen_update_toki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tokib(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void toki_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void tokib_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
