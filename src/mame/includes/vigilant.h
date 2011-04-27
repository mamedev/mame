class vigilant_state : public driver_device
{
public:
	vigilant_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_horiz_scroll_low;
	int m_horiz_scroll_high;
	int m_rear_horiz_scroll_low;
	int m_rear_horiz_scroll_high;
	int m_rear_color;
	int m_rear_disable;
	int m_rear_refresh;
	bitmap_t *m_bg_bitmap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
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
