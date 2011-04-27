class tehkanwc_state : public driver_device
{
public:
	tehkanwc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_track0[2];
	int m_track1[2];
	int m_msm_data_offs;
	int m_toggle;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 m_scroll_x[2];
	UINT8 m_led0;
	UINT8 m_led1;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/tehkanwc.c -----------*/

extern WRITE8_HANDLER( tehkanwc_videoram_w );
extern WRITE8_HANDLER( tehkanwc_colorram_w );
extern WRITE8_HANDLER( tehkanwc_videoram2_w );
extern WRITE8_HANDLER( tehkanwc_scroll_x_w );
extern WRITE8_HANDLER( tehkanwc_scroll_y_w );
extern WRITE8_HANDLER( tehkanwc_flipscreen_x_w );
extern WRITE8_HANDLER( tehkanwc_flipscreen_y_w );
extern WRITE8_HANDLER( gridiron_led0_w );
extern WRITE8_HANDLER( gridiron_led1_w );

extern VIDEO_START( tehkanwc );
extern SCREEN_UPDATE( tehkanwc );
