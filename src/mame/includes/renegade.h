class renegade_state : public driver_device
{
public:
	renegade_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/renegade.c -----------*/

extern UINT8 *renegade_videoram2;

VIDEO_UPDATE( renegade );
VIDEO_START( renegade );
WRITE8_HANDLER( renegade_scroll0_w );
WRITE8_HANDLER( renegade_scroll1_w );
WRITE8_HANDLER( renegade_videoram_w );
WRITE8_HANDLER( renegade_videoram2_w );
WRITE8_HANDLER( renegade_flipscreen_w );
