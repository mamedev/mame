class tsamurai_state : public driver_device
{
public:
	tsamurai_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int nmi_enabled;
	int sound_command1;
	int sound_command2;
	int sound_command3;
	int vsgongf_sound_nmi_enabled;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *bg_videoram;
	int bgcolor;
	int textbank1;
	int textbank2;
	tilemap_t *background;
	tilemap_t *foreground;
	int flicker;
	int vsgongf_color;
	int key_count;
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
VIDEO_UPDATE( tsamurai );

VIDEO_START( vsgongf );
VIDEO_UPDATE( vsgongf );
