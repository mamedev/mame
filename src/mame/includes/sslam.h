// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Quench
#include "sound/okim6295.h"

class sslam_state : public driver_device
{
public:
	sslam_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg_tileram(*this, "bg_tileram"),
		m_md_tileram(*this, "md_tileram"),
		m_tx_tileram(*this, "tx_tileram"),
		m_regs(*this, "regs"),
		m_spriteram(*this, "spriteram") { }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_bg_tileram;
	optional_shared_ptr<UINT16> m_md_tileram;
	optional_shared_ptr<UINT16> m_tx_tileram;
	required_shared_ptr<UINT16> m_regs;
	required_shared_ptr<UINT16> m_spriteram;

	emu_timer *m_music_timer;

	int m_sound;
	int m_melody;
	int m_bar;
	int m_track;
	int m_snd_bank;

	UINT8 m_oki_control;
	UINT8 m_oki_command;
	UINT8 m_oki_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_md_tilemap;

	int m_sprites_x_offset;
	DECLARE_WRITE16_MEMBER(powerbls_sound_w);
	DECLARE_READ8_MEMBER(playmark_snd_command_r);
	DECLARE_WRITE8_MEMBER(playmark_oki_w);
	DECLARE_WRITE8_MEMBER(playmark_snd_control_w);
	DECLARE_WRITE16_MEMBER(sslam_tx_tileram_w);
	DECLARE_WRITE16_MEMBER(sslam_md_tileram_w);
	DECLARE_WRITE16_MEMBER(sslam_bg_tileram_w);
	DECLARE_WRITE16_MEMBER(powerbls_bg_tileram_w);
	DECLARE_WRITE8_MEMBER(sslam_snd_w);
	DECLARE_DRIVER_INIT(sslam);
	DECLARE_DRIVER_INIT(powerbls);
	TILE_GET_INFO_MEMBER(get_sslam_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_sslam_md_tile_info);
	TILE_GET_INFO_MEMBER(get_sslam_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_powerbls_bg_tile_info);
	DECLARE_VIDEO_START(sslam);
	DECLARE_VIDEO_START(powerbls);
	UINT32 screen_update_sslam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_powerbls(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(music_playback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sslam_play(int track, int data);
};
