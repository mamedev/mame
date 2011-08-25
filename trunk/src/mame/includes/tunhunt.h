class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_control;
	UINT8 *m_workram;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	tilemap_t *m_fg_tilemap;
};


/*----------- defined in video/tunhunt.c -----------*/

WRITE8_HANDLER( tunhunt_videoram_w );

PALETTE_INIT( tunhunt );
VIDEO_START( tunhunt );
SCREEN_UPDATE( tunhunt );
