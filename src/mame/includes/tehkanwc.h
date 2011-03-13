class tehkanwc_state : public driver_device
{
public:
	tehkanwc_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int track0[2];
	int track1[2];
	int msm_data_offs;
	int toggle;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *videoram2;
	UINT8 scroll_x[2];
	UINT8 led0;
	UINT8 led1;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/tehkanwc.c -----------*/

extern WRITE8_HANDLER( tehkanwc_videoram_w );
extern WRITE8_HANDLER( tehkanwc_colorram_w );
extern WRITE8_HANDLER( tehkanwc_videoram2_w );
extern WRITE8_HANDLER( tehkanwc_scroll_x_w );
extern WRITE8_HANDLER( tehkanwc_scroll_y_w );
extern WRITE8_HANDLER( tehkanwc_flipscreen_x_w );
extern WRITE8_HANDLER( tehkanwc_flipscreen_y_w );
extern WRITE8_HANDLER( gridiron_led0_w );
extern WRITE8_HANDLER( gridiron_led1_w );

extern VIDEO_START( tehkanwc );
extern SCREEN_UPDATE( tehkanwc );
