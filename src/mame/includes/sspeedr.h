class sspeedr_state : public driver_device
{
public:
	sspeedr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 led_TIME[2];
	UINT8 led_SCORE[24];
	int toggle;
	unsigned driver_horz;
	unsigned driver_vert;
	unsigned driver_pic;
	unsigned drones_horz;
	unsigned drones_vert[3];
	unsigned drones_mask;
	unsigned track_horz;
	unsigned track_vert[2];
	unsigned track_ice;
};


/*----------- defined in video/sspeedr.c -----------*/

WRITE8_HANDLER( sspeedr_driver_horz_w );
WRITE8_HANDLER( sspeedr_driver_horz_2_w );
WRITE8_HANDLER( sspeedr_driver_vert_w );
WRITE8_HANDLER( sspeedr_driver_pic_w );

WRITE8_HANDLER( sspeedr_drones_horz_w );
WRITE8_HANDLER( sspeedr_drones_horz_2_w );
WRITE8_HANDLER( sspeedr_drones_vert_w );
WRITE8_HANDLER( sspeedr_drones_mask_w );

WRITE8_HANDLER( sspeedr_track_horz_w );
WRITE8_HANDLER( sspeedr_track_horz_2_w );
WRITE8_HANDLER( sspeedr_track_vert_w );
WRITE8_HANDLER( sspeedr_track_ice_w );

VIDEO_START( sspeedr );
SCREEN_UPDATE( sspeedr );
SCREEN_EOF( sspeedr );
