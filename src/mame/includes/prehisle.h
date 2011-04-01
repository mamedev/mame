class prehisle_state : public driver_device
{
public:
	prehisle_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }


	UINT16 *m_spriteram;
	UINT16 *m_videoram;
	UINT16 *m_bg_videoram16;
	UINT16 m_invert_controls;

	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
};


/*----------- defined in video/prehisle.c -----------*/

WRITE16_HANDLER( prehisle_bg_videoram16_w );
WRITE16_HANDLER( prehisle_fg_videoram16_w );
WRITE16_HANDLER( prehisle_control16_w );
READ16_HANDLER( prehisle_control16_r );

VIDEO_START( prehisle );
SCREEN_UPDATE( prehisle );
