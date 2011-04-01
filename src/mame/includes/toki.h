class toki_state : public driver_device
{
public:
	toki_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_videoram;
	int m_msm5205next;
	int m_toggle;
	UINT16 *m_background1_videoram16;
	UINT16 *m_background2_videoram16;
	UINT16 *m_scrollram16;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_text_layer;
};


/*----------- defined in video/toki.c -----------*/

VIDEO_START( toki );
SCREEN_EOF( toki );
SCREEN_EOF( tokib );
SCREEN_UPDATE( toki );
SCREEN_UPDATE( tokib );
WRITE16_HANDLER( toki_background1_videoram16_w );
WRITE16_HANDLER( toki_background2_videoram16_w );
WRITE16_HANDLER( toki_control_w );
WRITE16_HANDLER( toki_foreground_videoram16_w );
