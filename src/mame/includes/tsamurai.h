// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
class tsamurai_state : public driver_device
{
public:
	tsamurai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audio2(*this, "audio2"),
		m_audio3(*this, "audio3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_audio2;
	optional_device<cpu_device> m_audio3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	tilemap_t *m_background;
	tilemap_t *m_foreground;

	//common
	int m_flicker;
	int m_textbank1;
	int m_nmi_enabled;

	// tsamurai and m660 specific
	int m_bgcolor;
	int m_sound_command1;
	int m_sound_command2;

	//m660 specific
	int m_textbank2;
	int m_sound_command3;

	//vsgongf specific
	int m_vsgongf_sound_nmi_enabled;
	int m_vsgongf_color;
	int m_key_count; //debug only

	// common
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(textbank1_w);
	DECLARE_WRITE8_MEMBER(fg_videoram_w);

	// tsamurai and m660 specific
	DECLARE_WRITE8_MEMBER(bg_videoram_w);
	DECLARE_WRITE8_MEMBER(fg_colorram_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);
	DECLARE_WRITE8_MEMBER(scrollx_w);
	DECLARE_WRITE8_MEMBER(bgcolor_w);
	DECLARE_READ8_MEMBER(unknown_d806_r);
	DECLARE_READ8_MEMBER(unknown_d900_r);
	DECLARE_READ8_MEMBER(unknown_d938_r);
	DECLARE_WRITE8_MEMBER(sound_command1_w);
	DECLARE_WRITE8_MEMBER(sound_command2_w);
	DECLARE_READ8_MEMBER(sound_command1_r);
	DECLARE_READ8_MEMBER(sound_command2_r);

	// tsamurai specific
	DECLARE_READ8_MEMBER(tsamurai_unknown_d803_r);

	// m660 specific
	DECLARE_WRITE8_MEMBER(m660_textbank2_w);
	DECLARE_READ8_MEMBER(m660_unknown_d803_r);
	DECLARE_WRITE8_MEMBER(m660_sound_command3_w);
	DECLARE_READ8_MEMBER(m660_sound_command3_r);

	// vsgongf specific
	DECLARE_WRITE8_MEMBER(vsgongf_color_w);
	DECLARE_WRITE8_MEMBER(vsgongf_sound_nmi_enable_w);
	DECLARE_READ8_MEMBER(vsgongf_a006_r);
	DECLARE_READ8_MEMBER(vsgongf_a100_r);
	DECLARE_WRITE8_MEMBER(vsgongf_sound_command_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_vsgongf_tile_info);

	virtual void machine_start() override;
	DECLARE_MACHINE_START(m660);
	DECLARE_MACHINE_START(tsamurai);
	DECLARE_MACHINE_START(vsgongf);
	virtual void video_start() override;
	DECLARE_VIDEO_START(m660);
	DECLARE_VIDEO_START(tsamurai);
	DECLARE_VIDEO_START(vsgongf);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vsgongf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	INTERRUPT_GEN_MEMBER(interrupt);
	INTERRUPT_GEN_MEMBER(vsgongf_sound_interrupt);
};
