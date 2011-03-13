class timelimt_state : public driver_device
{
public:
	timelimt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int nmi_enabled;
	UINT8 *bg_videoram;
	size_t bg_videoram_size;
	int scrollx;
	int scrolly;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/timelimt.c -----------*/

VIDEO_START( timelimt );
PALETTE_INIT( timelimt );
SCREEN_UPDATE( timelimt );

WRITE8_HANDLER( timelimt_videoram_w );
WRITE8_HANDLER( timelimt_bg_videoram_w );
WRITE8_HANDLER( timelimt_scroll_y_w );
WRITE8_HANDLER( timelimt_scroll_x_msb_w );
WRITE8_HANDLER( timelimt_scroll_x_lsb_w );
