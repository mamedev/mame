// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "audio/m72.h"

class m90_state : public driver_device
{
public:
	m90_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_video_data(*this, "video_data"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_video_data;
	optional_shared_ptr<UINT16> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT16 m_video_control_data[8];
	tilemap_t *m_pf1_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_wide_layer;
	tilemap_t *m_pf2_wide_layer;
	UINT8 m_last_pf1;
	UINT8 m_last_pf2;
	DECLARE_WRITE16_MEMBER(m90_coincounter_w);
	DECLARE_WRITE16_MEMBER(quizf1_bankswitch_w);
	DECLARE_WRITE16_MEMBER(dynablsb_sound_command_w);
	DECLARE_WRITE16_MEMBER(m90_video_control_w);
	DECLARE_WRITE16_MEMBER(m90_video_w);
	DECLARE_DRIVER_INIT(bomblord);
	DECLARE_DRIVER_INIT(quizf1);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1w_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2w_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf1w_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(bomblord_get_pf2w_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf1w_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(dynablsb_get_pf2w_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(bomblord);
	DECLARE_VIDEO_START(dynablsb);
	UINT32 screen_update_m90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bomblord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dynablsb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(fake_nmi);
	INTERRUPT_GEN_MEMBER(bomblord_fake_nmi);
	INTERRUPT_GEN_MEMBER(m90_interrupt);
	INTERRUPT_GEN_MEMBER(dynablsb_interrupt);
	INTERRUPT_GEN_MEMBER(bomblord_interrupt);
	inline void get_tile_info(tile_data &tileinfo,int tile_index,int layer,int page_mask);
	inline void bomblord_get_tile_info(tile_data &tileinfo,int tile_index,int layer);
	inline void dynablsb_get_tile_info(tile_data &tileinfo,int tile_index,int layer);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void bomblord_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void dynablsb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void markdirty(tilemap_t *tmap,int page,offs_t offset);
};
