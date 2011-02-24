class speedatk_state : public driver_device
{
public:
	speedatk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 crtc_vreg[0x100],crtc_index;
	UINT8 flip_scr;

	UINT8 mux_data;
	UINT8 km_status;
	UINT8 coin_settings;
	UINT8 coin_impulse;
};


/*----------- defined in video/speedatk.c -----------*/

WRITE8_HANDLER( speedatk_videoram_w );
WRITE8_HANDLER( speedatk_colorram_w );
WRITE8_HANDLER( speedatk_6845_w );
PALETTE_INIT( speedatk );
VIDEO_START( speedatk );
SCREEN_UPDATE( speedatk );
