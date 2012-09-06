class tsamurai_state : public driver_device
{
public:
	tsamurai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"){ }

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
};


/*----------- defined in video/tsamurai.c -----------*/




VIDEO_START( tsamurai );
SCREEN_UPDATE_IND16( tsamurai );

VIDEO_START( vsgongf );
SCREEN_UPDATE_IND16( vsgongf );
