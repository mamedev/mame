class vigilant_state : public driver_device
{
public:
	vigilant_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/vigilant.c -----------*/

VIDEO_START( vigilant );
VIDEO_RESET( vigilant );
WRITE8_HANDLER( vigilant_paletteram_w );
WRITE8_HANDLER( vigilant_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_color_w );
VIDEO_UPDATE( vigilant );
VIDEO_UPDATE( kikcubic );
