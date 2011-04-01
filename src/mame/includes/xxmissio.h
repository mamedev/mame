class xxmissio_state : public driver_device
{
public:
	xxmissio_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_status;
	UINT8 *m_bgram;
	UINT8 *m_fgram;
	UINT8 *m_spriteram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_xscroll;
	UINT8 m_yscroll;
	UINT8 m_flipscreen;
};


/*----------- defined in video/xxmissio.c -----------*/

VIDEO_START( xxmissio );
SCREEN_UPDATE( xxmissio );

WRITE8_DEVICE_HANDLER( xxmissio_scroll_x_w );
WRITE8_DEVICE_HANDLER( xxmissio_scroll_y_w );
WRITE8_HANDLER( xxmissio_flipscreen_w );

READ8_HANDLER( xxmissio_bgram_r );
WRITE8_HANDLER( xxmissio_bgram_w );

WRITE8_HANDLER( xxmissio_paletteram_w );
