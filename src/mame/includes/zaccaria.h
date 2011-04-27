class zaccaria_state : public driver_device
{
public:
	zaccaria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_dsw;
	int m_active_8910;
	int m_port0a;
	int m_acs;
	int m_last_port0b;
	int m_toggle;
	UINT8 *m_videoram;
	UINT8 *m_attributesram;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
};


/*----------- defined in video/zaccaria.c -----------*/

PALETTE_INIT( zaccaria );
VIDEO_START( zaccaria );
WRITE8_HANDLER( zaccaria_videoram_w );
WRITE8_HANDLER( zaccaria_attributes_w );
WRITE8_HANDLER( zaccaria_flip_screen_x_w );
WRITE8_HANDLER( zaccaria_flip_screen_y_w );
SCREEN_UPDATE( zaccaria );
