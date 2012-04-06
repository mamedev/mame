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
	bitmap_ind16 *m_bg_bitmap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(vigilant_bank_select_w);
	DECLARE_WRITE8_MEMBER(vigilant_out2_w);
	DECLARE_WRITE8_MEMBER(kikcubic_coin_w);
	DECLARE_WRITE8_MEMBER(vigilant_paletteram_w);
	DECLARE_WRITE8_MEMBER(vigilant_horiz_scroll_w);
	DECLARE_WRITE8_MEMBER(vigilant_rear_horiz_scroll_w);
	DECLARE_WRITE8_MEMBER(vigilant_rear_color_w);
};


/*----------- defined in video/vigilant.c -----------*/

VIDEO_START( vigilant );
VIDEO_RESET( vigilant );
SCREEN_UPDATE_IND16( vigilant );
SCREEN_UPDATE_IND16( kikcubic );
