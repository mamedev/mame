// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina
#include "sound/okim6295.h"
#include "audio/seibu.h"
#include "machine/raiden2cop.h"
#include "video/seibu_crtc.h"

class legionna_state : public driver_device
{
public:
	legionna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram"),
		/*m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),*/
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_wordswapram(*this, "wordswapram"),
		m_raiden2cop(*this, "raiden2cop")
	{
		memset(scrollvals, 0, sizeof(UINT16)*6);
	}

	required_shared_ptr<UINT16> m_spriteram;
	std::unique_ptr<UINT16[]> m_back_data;
	std::unique_ptr<UINT16[]> m_fore_data;
	std::unique_ptr<UINT16[]> m_mid_data;
	std::unique_ptr<UINT16[]> m_textram;
	std::unique_ptr<UINT16[]> m_scrollram16;
	UINT16 m_layer_disable;
	int m_sprite_xoffs;
	int m_sprite_yoffs;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	int m_has_extended_banking;
	int m_has_extended_priority;
	UINT16 m_back_gfx_bank;
	UINT16 m_fore_gfx_bank;
	UINT16 m_mid_gfx_bank;
	UINT16 scrollvals[6];
	DECLARE_WRITE16_MEMBER(tilemap_enable_w);
	DECLARE_WRITE16_MEMBER(tile_scroll_w);
	DECLARE_WRITE16_MEMBER(tile_scroll_base_w);
	DECLARE_WRITE16_MEMBER(tile_vreg_1a_w);
	DECLARE_WRITE16_MEMBER(videowrite_cb_w);
	DECLARE_WRITE16_MEMBER(wordswapram_w);
	DECLARE_WRITE16_MEMBER(legionna_background_w);
	DECLARE_WRITE16_MEMBER(legionna_midground_w);
	DECLARE_WRITE16_MEMBER(legionna_foreground_w);
	DECLARE_WRITE16_MEMBER(legionna_text_w);
	DECLARE_WRITE8_MEMBER(okim_rombank_w);
	DECLARE_READ16_MEMBER(sound_comms_r);
	DECLARE_WRITE16_MEMBER(sound_comms_w);
	DECLARE_WRITE16_MEMBER(denjinmk_setgfxbank);
	DECLARE_WRITE16_MEMBER(heatbrl_setgfxbank);

	DECLARE_DRIVER_INIT(legiongfx);
	DECLARE_DRIVER_INIT(cupsoc_debug);
	DECLARE_DRIVER_INIT(cupsoc);
	DECLARE_DRIVER_INIT(cupsocs);
	DECLARE_DRIVER_INIT(olysoc92);
	DECLARE_DRIVER_INIT(denjinmk);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info_denji);
	TILE_GET_INFO_MEMBER(get_mid_tile_info_cupsoc);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info_denji);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	DECLARE_VIDEO_START(legionna);
	DECLARE_VIDEO_START(godzilla);
	DECLARE_VIDEO_START(denjinmk);
	DECLARE_VIDEO_START(grainbow);
	DECLARE_VIDEO_START(cupsoc);
	UINT32 screen_update_legionna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_godzilla(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_grainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void descramble_legionnaire_gfx(UINT8* src);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT16> m_wordswapram;
	optional_device<raiden2cop_device> m_raiden2cop;

};
