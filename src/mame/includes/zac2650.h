class zac2650_state : public driver_device
{
public:
	zac2650_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/zac2650.c -----------*/

extern UINT8 *zac2650_s2636_0_ram;

WRITE8_HANDLER( tinvader_videoram_w );
READ8_HANDLER( zac_s2636_r );
WRITE8_HANDLER( zac_s2636_w );
READ8_HANDLER( tinvader_port_0_r );

VIDEO_START( tinvader );
VIDEO_UPDATE( tinvader );

