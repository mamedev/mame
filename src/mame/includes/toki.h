class toki_state : public driver_device
{
public:
	toki_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/toki.c -----------*/

extern UINT16 *toki_background1_videoram16;
extern UINT16 *toki_background2_videoram16;
extern UINT16 *toki_scrollram16;

VIDEO_START( toki );
VIDEO_EOF( toki );
VIDEO_EOF( tokib );
VIDEO_UPDATE( toki );
VIDEO_UPDATE( tokib );
WRITE16_HANDLER( toki_background1_videoram16_w );
WRITE16_HANDLER( toki_background2_videoram16_w );
WRITE16_HANDLER( toki_control_w );
WRITE16_HANDLER( toki_foreground_videoram16_w );
