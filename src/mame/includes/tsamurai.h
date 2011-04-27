class tsamurai_state : public driver_device
{
public:
	tsamurai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_nmi_enabled;
	int m_sound_command1;
	int m_sound_command2;
	int m_sound_command3;
	int m_vsgongf_sound_nmi_enabled;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_bg_videoram;
	int m_bgcolor;
	int m_textbank1;
	int m_textbank2;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	int m_flicker;
	int m_vsgongf_color;
	int m_key_count;
	UINT8 *m_spriteram;
};


/*----------- defined in video/tsamurai.c -----------*/

WRITE8_HANDLER( vsgongf_color_w );

WRITE8_HANDLER( tsamurai_bgcolor_w );
WRITE8_HANDLER( tsamurai_textbank1_w );
WRITE8_HANDLER( tsamurai_textbank2_w );

WRITE8_HANDLER( tsamurai_scrolly_w );
WRITE8_HANDLER( tsamurai_scrollx_w );
WRITE8_HANDLER( tsamurai_bg_videoram_w );
WRITE8_HANDLER( tsamurai_fg_videoram_w );
WRITE8_HANDLER( tsamurai_fg_colorram_w );

VIDEO_START( tsamurai );
SCREEN_UPDATE( tsamurai );

VIDEO_START( vsgongf );
SCREEN_UPDATE( vsgongf );
