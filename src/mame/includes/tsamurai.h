class tsamurai_state : public driver_device
{
public:
	tsamurai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audio2(*this, "audio2"),
		m_audio3(*this, "audio3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_nmi_enabled;
	int m_sound_command1;
	int m_sound_command2;
	int m_sound_command3;
	int m_vsgongf_sound_nmi_enabled;
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_bg_videoram;
	int m_bgcolor;
	int m_textbank1;
	int m_textbank2;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	int m_flicker;
	int m_vsgongf_color;
	int m_key_count;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(unknown_d803_r);
	DECLARE_READ8_MEMBER(unknown_d803_m660_r);
	DECLARE_READ8_MEMBER(unknown_d806_r);
	DECLARE_READ8_MEMBER(unknown_d900_r);
	DECLARE_READ8_MEMBER(unknown_d938_r);
	DECLARE_WRITE8_MEMBER(sound_command1_w);
	DECLARE_WRITE8_MEMBER(sound_command2_w);
	DECLARE_WRITE8_MEMBER(sound_command3_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(tsamurai_coin_counter_w);
	DECLARE_READ8_MEMBER(sound_command1_r);
	DECLARE_READ8_MEMBER(sound_command2_r);
	DECLARE_READ8_MEMBER(sound_command3_r);
	DECLARE_WRITE8_MEMBER(vsgongf_sound_nmi_enable_w);
	DECLARE_READ8_MEMBER(vsgongf_a006_r);
	DECLARE_READ8_MEMBER(vsgongf_a100_r);
	DECLARE_WRITE8_MEMBER(vsgongf_sound_command_w);
	DECLARE_WRITE8_MEMBER(tsamurai_scrolly_w);
	DECLARE_WRITE8_MEMBER(tsamurai_scrollx_w);
	DECLARE_WRITE8_MEMBER(tsamurai_bgcolor_w);
	DECLARE_WRITE8_MEMBER(tsamurai_textbank1_w);
	DECLARE_WRITE8_MEMBER(tsamurai_textbank2_w);
	DECLARE_WRITE8_MEMBER(tsamurai_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(tsamurai_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(tsamurai_fg_colorram_w);
	DECLARE_WRITE8_MEMBER(vsgongf_color_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_vsgongf_tile_info);
	DECLARE_VIDEO_START(tsamurai);
	DECLARE_VIDEO_START(vsgongf);
	UINT32 screen_update_tsamurai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vsgongf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(samurai_interrupt);
	INTERRUPT_GEN_MEMBER(vsgongf_sound_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_audio2;
	optional_device<cpu_device> m_audio3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
