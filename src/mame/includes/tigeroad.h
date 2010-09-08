class tigeroad_state : public driver_device
{
public:
	tigeroad_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/tigeroad.c -----------*/

WRITE16_HANDLER( tigeroad_videoram_w );
WRITE16_HANDLER( tigeroad_videoctrl_w );
WRITE16_HANDLER( tigeroad_scroll_w );
VIDEO_START( tigeroad );
VIDEO_UPDATE( tigeroad );
VIDEO_EOF( tigeroad );
