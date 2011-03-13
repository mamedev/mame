class vigilant_state : public driver_device
{
public:
	vigilant_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int horiz_scroll_low;
	int horiz_scroll_high;
	int rear_horiz_scroll_low;
	int rear_horiz_scroll_high;
	int rear_color;
	int rear_disable;
	int rear_refresh;
	bitmap_t *bg_bitmap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/vigilant.c -----------*/

VIDEO_START( vigilant );
VIDEO_RESET( vigilant );
WRITE8_HANDLER( vigilant_paletteram_w );
WRITE8_HANDLER( vigilant_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_color_w );
SCREEN_UPDATE( vigilant );
SCREEN_UPDATE( kikcubic );
